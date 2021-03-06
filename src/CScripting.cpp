#pragma once

#include "CScripting.h"
#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"
#include "CMySQLConnection.h"
#include "CCallback.h"
#include "COrm.h"
#include "CLog.h"

#include "misc.h"


//native ORM:orm_create(table[], connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::orm_create)
{
	const unsigned int connection_id = params[2];
	const char *table_name = NULL;
	amx_StrParam(amx, params[1], table_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_create", "table: \"%s\", connectionHandle: %d", table_name, connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("orm_create", connection_id);
	

	return static_cast<cell>(COrm::Create(table_name, CMySQLHandle::GetHandle(connection_id)));
}

//native orm_destroy(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_destroy)
{
	const unsigned int orm_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_destroy", "orm_id: %d", orm_id);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_destroy", orm_id);


	COrm::GetOrm(orm_id)->Destroy();
	return 1;
}

//native ORM_Error:orm_errno(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_errno)
{
	const unsigned int orm_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_errno", "orm_id: %d", orm_id);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_errno", orm_id);

	return static_cast<cell>(COrm::GetOrm(orm_id)->GetErrorID());
}

// native orm_apply_cache(ORM:id, row);
AMX_DECLARE_NATIVE(Native::orm_apply_cache)
{
	const unsigned int orm_id = params[1];
	const unsigned int row_idx = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_apply_cache", "orm_id: %d, row: %d", orm_id, row_idx);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_apply_cache", orm_id);

	return COrm::GetOrm(orm_id)->ApplyActiveResult(row_idx) ? 1 : 0;
}

//native orm_select(ORM:id, callback[], format[], {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_select)
{
	const int ConstParamCount = 3;
	const unsigned int orm_id = params[1];
	const char 
		*cb_format = NULL,
		*cb_name = NULL;
	amx_StrParam(amx, params[3], cb_format);
	amx_StrParam(amx, params[2], cb_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_select", "orm_id: %d, callback: \"%s\", format: \"%s\"", orm_id, cb_name, cb_format);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_select", orm_id);

	if(cb_format != NULL && strlen(cb_format) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_select", "callback parameter count does not match format specifier length");


	COrm *OrmObject = COrm::GetOrm(orm_id);
	CMySQLHandle *Handle = OrmObject->GetConnectionHandle();
	CMySQLQuery *query = new CMySQLQuery;

	if(OrmObject->GenerateSelectQuery(query->Query))
	{
		query->Callback.Name = (cb_name != NULL ? cb_name : string());
		if (cb_format != NULL)
			CCallback::Get()->FillCallbackParams(query->Callback.Params, cb_format, amx, params, ConstParamCount);

		query->Handle = Handle;
		query->Orm.Object = OrmObject;
		query->Orm.Type = ORM_QUERYTYPE_SELECT;
	
		Handle->QueueQuery(query);
		return 1;
	}
	else
	{
		delete query;
		return 0;
	}
}

//native orm_update(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_update)
{
	const unsigned int orm_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_update", "orm_id: %d", orm_id);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_update", orm_id);
	

	COrm *OrmObject = COrm::GetOrm(orm_id);
	CMySQLHandle *Handle = OrmObject->GetConnectionHandle();
	CMySQLQuery *query = new CMySQLQuery;

	if(OrmObject->GenerateUpdateQuery(query->Query))
	{
		query->Handle = Handle;
		query->Orm.Object = OrmObject;
		query->Orm.Type = ORM_QUERYTYPE_UPDATE;

		Handle->QueueQuery(query);
		return 1;
	}
	else
	{
		delete query;
		return 0;
	}
}

//native orm_insert(ORM:id, callback[]="", format[]="", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_insert)
{
	static const int ConstParamCount = 3;
	const unsigned int orm_id = params[1];
	const char 
		*cb_format = NULL,
		*cb_name = NULL;
	amx_StrParam(amx, params[3], cb_format);
	amx_StrParam(amx, params[2], cb_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_insert", "orm_id: %d, callback: \"%s\", format: \"%s\"", orm_id, cb_name, cb_format);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_insert", orm_id);

	if(cb_format != NULL && strlen(cb_format) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_insert", "callback parameter count does not match format specifier length");


	COrm *OrmObject = COrm::GetOrm(orm_id);
	CMySQLHandle *Handle = OrmObject->GetConnectionHandle();
	CMySQLQuery *query = new CMySQLQuery;

	if(OrmObject->GenerateInsertQuery(query->Query))
	{
		query->Callback.Name = (cb_name != NULL ? cb_name : string());
		if (cb_format != NULL)
			CCallback::Get()->FillCallbackParams(query->Callback.Params, cb_format, amx, params, ConstParamCount);
		
		query->Handle = Handle;
		query->Orm.Object = OrmObject;
		query->Orm.Type = ORM_QUERYTYPE_INSERT;

		Handle->QueueQuery(query);
		return 1;
	}
	else
	{
		delete query;
		return 0;
	}
}

//native orm_delete(ORM:id, bool:clearvars=true);
AMX_DECLARE_NATIVE(Native::orm_delete)
{
	const unsigned int orm_id = params[1];
	const bool clear_vars = (params[2] != 0);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_delete", "orm_id: %d, clearvars: %s", orm_id, clear_vars == true ? "true" : "false");

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_delete", orm_id);


	COrm *OrmObject = COrm::GetOrm(orm_id);
	CMySQLHandle *Handle = OrmObject->GetConnectionHandle();
	CMySQLQuery *query = new CMySQLQuery;

	if(OrmObject->GenerateDeleteQuery(query->Query))
	{
		query->Handle = Handle;
		query->Orm.Object = OrmObject;
		query->Orm.Type = ORM_QUERYTYPE_DELETE;

		Handle->QueueQuery(query);

		if(clear_vars == true)
			OrmObject->ClearVariableValues();
	
		return 1;
	}
	else
	{
		delete query;
		return 0;
	}
}

//native orm_save(ORM:id, callback[]="", format[]="", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_save)
{
	static const int ConstParamCount = 3;
	const unsigned int orm_id = params[1];
	const char 
		*cb_format = NULL,
		*cb_name = NULL;
	amx_StrParam(amx, params[3], cb_format);
	amx_StrParam(amx, params[2], cb_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_save", "orm_id: %d, callback: \"%s\", format: \"%s\"", orm_id, cb_name, cb_format);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_save", orm_id);

	if(cb_format != NULL && strlen(cb_format) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_save", "callback parameter count does not match format specifier length");


	COrm *OrmObject = COrm::GetOrm(orm_id);
	CMySQLHandle *Handle = OrmObject->GetConnectionHandle();
	CMySQLQuery *query = new CMySQLQuery;


	query->Orm.Type = OrmObject->GenerateSaveQuery(query->Query);
	if(query->Orm.Type != ORM_QUERYTYPE_INVALID)
	{
		query->Callback.Name = (cb_name != NULL ? cb_name : string());
		if (cb_format != NULL)
			CCallback::Get()->FillCallbackParams(query->Callback.Params, cb_format, amx, params, ConstParamCount);
	
		query->Handle = Handle;
		query->Orm.Object = OrmObject;

		Handle->QueueQuery(query);
		return 1;
	}
	else
	{
		delete query;
		return 0;
	}
}

//native orm_addvar(ORM:id, &{Float, _}:var, var_datatype:datatype, var_maxlen, varname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar)
{
	const char *var_name = NULL;
	cell *var_address = NULL;

	const unsigned int orm_id = params[1];
	amx_GetAddr(amx, params[2], &var_address);
	const unsigned short var_datatype = static_cast<unsigned short>(params[3]);
	const unsigned int var_maxlen = params[4];
	amx_StrParam(amx, params[5], var_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_addvar", "orm_id: %d, var: %p, datatype: %d, varname: \"%s\", var_maxlen: %d", orm_id, var_address, var_datatype, var_name, var_maxlen);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_addvar", orm_id);

	if(var_datatype != DATATYPE_INT && var_datatype != DATATYPE_FLOAT && var_datatype != DATATYPE_STRING)
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_addvar", "unknown datatype specified");

	if(var_maxlen <= 0)
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_addvar", "invalid variable length specified");


	return COrm::GetOrm(orm_id)->AddVariable(var_name, var_address, var_datatype, var_maxlen) ? 1 : 0;
}

//native orm_delvar(ORM:id, varname[]);
AMX_DECLARE_NATIVE(Native::orm_delvar)
{
	const unsigned int orm_id = params[1];
	const char *var_name = NULL;
	amx_StrParam(amx, params[2], var_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_delvar", "orm_id: %d, varname: \"%s\"", orm_id, var_name);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_delvar", orm_id);


	return COrm::GetOrm(orm_id)->RemoveVariable(var_name) ? 1 : 0;
}

//native orm_setkey(ORM:id, varname[]);
AMX_DECLARE_NATIVE(Native::orm_setkey)
{
	const unsigned int orm_id = params[1];
	const char *var_name = NULL;
	amx_StrParam(amx, params[2], var_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "orm_setkey", "orm_id: %d, varname: \"%s\"", orm_id, var_name);

	if(!COrm::IsValid(orm_id))
		return ERROR_INVALID_ORM_ID("orm_setkey", orm_id);

	if(var_name == NULL)
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_setkey", "empty variable name specified");
	

	return COrm::GetOrm(orm_id)->SetVariableAsKey(var_name) ? 1 : 0;
}


//native cache_affected_rows(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_affected_rows)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_affected_rows", "connection: %d", connection_id);

	if(CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_affected_rows", "no active cache");
	

	return static_cast<cell>(CMySQLHandle::GetActiveHandle()->GetActiveResult()->AffectedRows());
}

//native cache_warning_count(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_warning_count)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_warning_count", "connection: %d", connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_warning_count", "no active cache");
	

	return static_cast<cell>(CMySQLHandle::GetActiveHandle()->GetActiveResult()->WarningCount());
}

//native cache_insert_id(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_insert_id)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_insert_id", "connection: %d", connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_insert_id", "no active cache");
	

	return static_cast<cell>(CMySQLHandle::GetActiveHandle()->GetActiveResult()->InsertID());
}


// native Cache:cache_save(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_save)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_save", "connection: %d", connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_save", "no active cache");


	int cache_id = CMySQLHandle::GetActiveHandle()->SaveActiveResult();
	if(cache_id == 0)
		CLog::Get()->LogFunction(LOG_WARNING, "cache_save", "no active cache");

	return static_cast<cell>(cache_id);
}

// native cache_delete(Cache:id, connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_delete)
{
	const unsigned int connection_id = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_delete", "cache_id: %d, connection: %d", params[1], connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("cache_delete", connection_id);


	return static_cast<cell>(CMySQLHandle::GetHandle(connection_id)->DeleteSavedResult(params[1]));
}

// native cache_set_active(Cache:id, connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_set_active)
{
	const unsigned int connection_id = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_set_active", "cache_id: %d, connection: %d", params[1], connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("cache_set_active", connection_id);


	return static_cast<cell>(CMySQLHandle::GetHandle(connection_id)->SetActiveResult((int)params[1]) == true ? 1 : 0);
}

// native cache_is_valid(Cache:id, connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_is_valid)
{
	const unsigned int connection_id = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_is_valid", "cache_id: %d, connection: %d", params[1], connection_id);

	if (!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("cache_is_valid", connection_id);


	return static_cast<cell>(CMySQLHandle::GetHandle(connection_id)->IsValidResult((int)params[1]) == true ? 1 : 0);
}

// native cache_get_row_count(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_row_count)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_count", "connection: %d", connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_count", "no active cache");


	return static_cast<cell>(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowCount());
}

// native cache_get_field_count(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_field_count)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_count", "connection: %d", connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_count", "no active cache");


	return static_cast<cell>(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetFieldCount());
}

// native cache_get_data(&num_rows, &num_fields, connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_data)
{
	const unsigned int connection_id = params[3];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_data", "connection: %d", connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_data", "no active cache");


	const CMySQLResult *Result = CMySQLHandle::GetActiveHandle()->GetActiveResult();

	cell *amx_address;
	amx_GetAddr(amx, params[1], &amx_address);
	(*amx_address) = static_cast<cell>(Result->GetRowCount());
	amx_GetAddr(amx, params[2], &amx_address);
	(*amx_address) = static_cast<cell>(Result->GetFieldCount());
	return 1;
}

// native cache_get_field_name(field_index, dest[], connectionHandle = 1, max_len = sizeof(destination))
AMX_DECLARE_NATIVE(Native::cache_get_field_name)
{
	const unsigned int connection_id = params[3];
	const unsigned int
		field_idx = params[1],
		max_len = params[4];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_name", "field_index: %d, connection: %d, max_len: %d", field_idx, connection_id, max_len);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_name", "no active cache");

	
	const char *field_name = CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetFieldName(field_idx);
	amx_SetCString(amx, params[2], field_name == NULL ? "NULL" : field_name, params[4]);
	return 1;
}

// native cache_get_row(row, field_idx, destination[], connectionHandle = 1, max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_row)
{
	const unsigned int connection_id = params[4];
	const unsigned int 
		row_idx = params[1],
		field_idx = params[2],
		max_len = params[5];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row", "row: %d, field_idx: %d, connection: %d, max_len: %d", row_idx, field_idx, connection_id, max_len);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row", "no active cache");

	
	const char *row_data = CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowData(row_idx, field_idx);
	amx_SetCString(amx, params[3], row_data == NULL ? "NULL" : row_data, max_len);
	return 1;
}

// native cache_get_row_int(row, field_idx, connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_row_int)
{
	const unsigned int connection_id = params[3];
	const unsigned int
		row_idx = params[1],
		field_idx = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_int", "row: %d, field_idx: %d, connection: %d", row_idx, field_idx, connection_id);
	
	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_int", "no active cache");

	
	int return_val = 0;
	if (ConvertStrToInt(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowData(row_idx, field_idx), return_val) == false)
	{
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_int", "invalid datatype");
		return_val = 0;
	}

	return static_cast<cell>(return_val);
}

// native Float:cache_get_row_float(row, field_idx, connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_row_float)
{
	const unsigned int connection_id = params[3];
	const unsigned int
		row_idx = params[1],
		field_idx = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_float", "row: %d, field_idx: %d, connection: %d", row_idx, field_idx, connection_id);
	
	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_float", "no active cache");

	
	float return_val = 0.0f;
	if (ConvertStrToFloat(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowData(row_idx, field_idx), return_val) == false)
	{
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_float", "invalid datatype");
		return_val = 0.0f;
	}
	
	return amx_ftoc(return_val);
}

// native cache_get_field_content(row, const field_name[], destination[], connectionHandle = 1, max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_field_content)
{
	const unsigned int connection_id = params[4];
	const unsigned int 
		row_idx = params[1],
		max_len = params[5];
	const char *field_name = NULL;
	amx_StrParam(amx, params[2], field_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content", "row: %d, field_name: \"%s\", connection: %d, max_len: %d", row_idx, field_name, connection_id, max_len);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content", "no active cache");


	const char *field_data = CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowDataByName(row_idx, field_name);
	amx_SetCString(amx, params[3], field_data == NULL ? "NULL" : field_data, max_len);
	return 1;
}

// native cache_get_field_content_int(row, const field_name[], connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_int)
{
	const unsigned int connection_id = params[3];
	const unsigned int row_idx = params[1];
	const char *field_name = NULL;
	amx_StrParam(amx, params[2], field_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content_int", "row: %d, field_name: \"%s\", connection: %d", row_idx, field_name, connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content_int", "no active cache");


	int return_val = 0;
	if (ConvertStrToInt(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowDataByName(row_idx, field_name), return_val) == false)
	{
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_int", "invalid datatype");
		return_val = 0;
	}

	return static_cast<cell>(return_val);
}

// native Float:cache_get_field_content_float(row, const field_name[], connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_float)
{
	const unsigned int connection_id = params[3];
	const unsigned int row_idx = params[1];
	const char *field_name = NULL;
	amx_StrParam(amx, params[2], field_name);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content_float", "row: %d, field_name: \"%s\", connection: %d", row_idx, field_name, connection_id);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content_float", "no active cache");


	float return_val = 0.0f;
	if (ConvertStrToFloat(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetRowDataByName(row_idx, field_name), return_val) == false)
	{
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_float", "invalid datatype");
		return_val = 0.0f;
	}

	return amx_ftoc(return_val);
}

// native cache_get_query_exec_time(E_EXECTIME_UNIT:unit = UNIT_MICROSECONDS);
AMX_DECLARE_NATIVE(Native::cache_get_query_exec_time)
{
	const int time_unit = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_query_exec_time", "unit: %d", time_unit);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_query_exec_time", "no active cache");

	if(time_unit != UNIT_MILLISECONDS && time_unit != UNIT_MICROSECONDS)
		return CLog::Get()->LogFunction(LOG_ERROR, "cache_get_query_exec_time", "invalid unit");


	return static_cast<cell>(CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetQueryExecutionTime(time_unit));
}

// native cache_get_query_string(destination[], max_len = sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_query_string)
{
	const int max_len = static_cast<int>(params[2]);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_query_string", "max_len: %d", max_len);

	if (CMySQLHandle::GetActiveHandle() == NULL)
		return CLog::Get()->LogFunction(LOG_WARNING, "cache_get_query_string", "no active cache");


	amx_SetCString(amx, params[1], CMySQLHandle::GetActiveHandle()->GetActiveResult()->GetQueryString().c_str(), max_len);
	return 1;
}

//native mysql_connect(const host[], const user[], const database[], const password[], port = 3306, bool:autoreconnect = true, pool_size = 2);
AMX_DECLARE_NATIVE(Native::mysql_connect)
{
	const char
		*host = NULL, 
		*user = NULL, 
		*db = NULL, 
		*pass = NULL;

	amx_StrParam(amx, params[1], host);
	amx_StrParam(amx, params[2], user);
	amx_StrParam(amx, params[3], db);
	amx_StrParam(amx, params[4], pass);

	const size_t port = params[5];
	const bool auto_reconnect = !!(params[6]);
	const size_t pool_size = params[7];

	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_connect", "host: \"%s\", user: \"%s\", database: \"%s\", password: \"****\", port: %d, autoreconnect: %s, pool_size: %d", host, user, db, port, auto_reconnect == true ? "true" : "false", pool_size);

	if(host == NULL || user == NULL || db == NULL)
		return CLog::Get()->LogFunction(LOG_ERROR, "mysql_connect", "empty connection data specified");
	

	CMySQLHandle *Handle = CMySQLHandle::Create(host, user, pass != NULL ? pass : "", db, port, pool_size, auto_reconnect);
	Handle->ExecuteOnConnections(boost::bind(&CMySQLConnection::Connect, _1));

	return static_cast<cell>(Handle->GetID());
}

//native mysql_close(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::mysql_close)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_close", "connection: %d", connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_close", connection_id);
	

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);
	
	Handle->ExecuteOnConnections(boost::bind(&CMySQLConnection::Disconnect, _1));
	Handle->Destroy();

	CCallback::Get()->ClearByHandle(Handle);
	return 1;
}

//native mysql_reconnect(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::mysql_reconnect)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_reconnect", "connection: %d", connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_reconnect", connection_id);
	

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);

	Handle->ExecuteOnConnections(boost::bind(&CMySQLConnection::Disconnect, _1));
	Handle->ExecuteOnConnections(boost::bind(&CMySQLConnection::Connect, _1));
	return 1;
}

//native mysql_option(E_MYSQL_OPTION:type, value);
AMX_DECLARE_NATIVE(Native::mysql_option)
{
	const unsigned short option_type = params[1];
	const int option_value = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_option", "option: %d, value: %d", option_type, option_value);


	switch(option_type)
	{
		case DUPLICATE_CONNECTIONS:
			MySQLOptions.DuplicateConnections = (option_value != 0);
			break;
		default:
			return CLog::Get()->LogFunction(LOG_ERROR, "mysql_option", "invalid option");
	}

	return 1;
}

//native mysql_current_handle();
AMX_DECLARE_NATIVE(Native::mysql_current_handle)
{
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_current_handle", "");


	int connection_id = 0;
	if(CMySQLHandle::GetActiveHandle() != NULL)
		connection_id = CMySQLHandle::GetActiveHandle()->GetID();

	return static_cast<cell>(connection_id);
}

//native mysql_unprocessed_queries(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::mysql_unprocessed_queries)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_unprocessed_queries", "connection: %d", connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_unprocessed_queries", connection_id);


	return static_cast<cell>(CMySQLHandle::GetHandle(connection_id)->GetUnprocessedQueryCount());
}


//native mysql_pquery(conhandle, query[], callback[], format[], {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_pquery)
{
	static const int ConstParamCount = 4;
	const unsigned int connection_id = params[1];

	const char
		*query_str = NULL,
		*cb_name = NULL,
		*cb_format = NULL;
	amx_StrParam(amx, params[2], query_str);
	amx_StrParam(amx, params[3], cb_name);
	amx_StrParam(amx, params[4], cb_format);

	if (CLog::Get()->IsLogLevel(LOG_DEBUG))
	{
		string short_query(query_str == NULL ? "" : query_str);
		short_query.resize(64);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_pquery", "connection: %d, query: \"%s\", callback: \"%s\", format: \"%s\"", connection_id, short_query.c_str(), cb_name, cb_format);
	}

	if (!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_pquery", connection_id);

	if (cb_format != NULL && strlen(cb_format) != ((params[0] / 4) - ConstParamCount))
		return CLog::Get()->LogFunction(LOG_ERROR, "mysql_pquery", "callback parameter count does not match format specifier length");


	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);
	CMySQLQuery *query = new CMySQLQuery;

	query->Query = (query_str != NULL ? query_str : string());
	query->Callback.Name = (cb_name != NULL ? cb_name : string());
	if (cb_format != NULL)
		CCallback::Get()->FillCallbackParams(query->Callback.Params, cb_format, amx, params, ConstParamCount);

	query->Handle = Handle;
	
	Handle->QueueQuery(query, true);
	return 1;
}

//native mysql_tquery(conhandle, query[], callback[], format[], {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_tquery)
{
	static const int ConstParamCount = 4;
	const unsigned int connection_id = params[1];

	const char 
		*query_str = NULL,
		*cb_name = NULL,
		*cb_format = NULL;
	amx_StrParam(amx, params[2], query_str);
	amx_StrParam(amx, params[3], cb_name);
	amx_StrParam(amx, params[4], cb_format);

	if(CLog::Get()->IsLogLevel(LOG_DEBUG))
	{
		string short_query(query_str == NULL ? "" : query_str);
		short_query.resize(64);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", "connection: %d, query: \"%s\", callback: \"%s\", format: \"%s\"", connection_id, short_query.c_str(), cb_name, cb_format);
	}

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_tquery", connection_id);

	if (cb_format != NULL && strlen(cb_format) != ((params[0] / 4) - ConstParamCount))
		return CLog::Get()->LogFunction(LOG_ERROR, "mysql_tquery", "callback parameter count does not match format specifier length");


	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);
	CMySQLQuery *query = new CMySQLQuery;

	query->Query = (query_str != NULL ? query_str : string());
	query->Callback.Name = (cb_name != NULL ? cb_name : string());
	if (cb_format != NULL)
		CCallback::Get()->FillCallbackParams(query->Callback.Params, cb_format, amx, params, ConstParamCount);

	query->Handle = Handle;
	
	Handle->QueueQuery(query);
	return 1;
}


//native Cache:mysql_query(conhandle, query[], bool:use_cache = true);
AMX_DECLARE_NATIVE(Native::mysql_query)
{
	const unsigned int connection_id = params[1];
	const char *query_str = NULL;
	amx_StrParam(amx, params[2], query_str);
	const bool use_cache = (params[3] != 0);

	if(CLog::Get()->IsLogLevel(LOG_DEBUG))
	{
		string short_query(query_str == NULL ? "" : query_str);
		short_query.resize(64);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_query", "connection: %d, query: \"%s\", use_cache: %s", connection_id, short_query.c_str(), use_cache == true ? "true" : "false");
	}

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_query", connection_id);


	int stored_result_id = 0;
	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);
	CMySQLQuery query;
	query.Query = (query_str != NULL ? query_str : string());
	query.Handle = Handle;
	query.Unthreaded = true;

	query.Execute(Handle->GetMainConnection()->GetMysqlPtr());

	if(use_cache == true)
	{
		//first we set this result as active
		Handle->SetActiveResult(query.Result);
		//now we can save the result
		stored_result_id = Handle->SaveActiveResult();
		query.Result = NULL;
	}
	delete query.Result;

	return static_cast<cell>(stored_result_id);
}


// native mysql_format(connectionHandle, output[], len, format[], {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_format)
{
	const unsigned int connection_id = params[1];
	const size_t dest_len = (size_t)params[3];
	const char *format_str = NULL;
	amx_StrParam(amx, params[4], format_str);

	if(CLog::Get()->IsLogLevel(LOG_DEBUG))
	{
		string short_format(format_str == NULL ? "" : format_str);
		if(short_format.length() > 128)
		{
			short_format.erase(128, short_format.length());
			short_format.append("...");
		}
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_format", "connection: %d, len: %d, format: \"%s\"", connection_id, dest_len, short_format.c_str());
	}

	if(format_str == NULL)
		return 0;

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_format", connection_id);
	

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);

	char *output_str = (char *)calloc(dest_len * 2, sizeof(char)); //*2 just for safety, what if user specified wrong dest_len?
	char *org_output_str = output_str;

	const unsigned int 
		first_param_idx = 5,
		num_args = (params[0] / sizeof(cell)),
		num_dyn_args = num_args - (first_param_idx - 1);
	unsigned int param_counter = 0;

	for( ; *format_str != '\0'; ++format_str)
	{
		
		if(strlen(org_output_str) >= dest_len)
		{
			CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "destination size is too small");
			break;
		}
		
		if(*format_str == '%')
		{
			++format_str;

			if(*format_str == '%')
			{
				*output_str = '%';
				++output_str;
				continue;
			}

			if(param_counter >= num_dyn_args)
			{
				CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "no value for specifier \"%%%c\" available", *format_str);
				continue;
			}

			bool SpaceWidth = true;
			int Width = -1;
			int Precision = -1;
			
			if(*format_str == '0')
			{
				SpaceWidth = false;
				++format_str;
			}
			if(*format_str > '0' && *format_str <= '9')
			{
				Width = 0;
				while(*format_str >= '0' && *format_str <= '9')
				{
					Width *= 10;
					Width += *format_str - '0';
					++format_str;
				}
			}

			if(*format_str == '.')
			{
				++format_str;
				Precision = *format_str - '0';
				++format_str;
			}

			cell *amx_address = NULL;
			amx_GetAddr(amx, params[first_param_idx + param_counter], &amx_address);
			
			switch (*format_str)
			{
				case 'i': 
				case 'I':
				case 'd': 
				case 'D':
				{
					char int_str[13];
					ConvertIntToStr<10>(*amx_address, int_str);
					size_t int_str_len = strlen(int_str);
					for(int len = (int)int_str_len; Width > len; ++len)
					{
						if(SpaceWidth == true)
							*output_str = ' ';
						else
							*output_str = '0';
						++output_str;
					}
					
					for(size_t c=0; c < int_str_len; ++c)
					{
						*output_str = int_str[c];
						++output_str;
					}
					break;
				}
				case 'z': 
				case 'Z':
				case 's': 
				case 'S':
				{
					char *str_buf = NULL;
					amx_StrParam(amx, params[first_param_idx + param_counter], str_buf);
					if(str_buf != NULL)
					{
						for(size_t c=0, len = strlen(str_buf); c < len; ++c)
						{
							*output_str = str_buf[c];
							++output_str;
						}
					}
					
					break;
				}
				case 'f':
				case 'F':
				{
					float float_val = amx_ctof(*amx_address);
					char 
						float_str[84+1], 
						spec_buf[13];

					ConvertIntToStr<10>((int)floor(float_val), float_str);
					for(int len = (int)strlen(float_str); Width > len; ++len)
					{
						if(SpaceWidth == true)
							*output_str = ' ';
						else
							*output_str = '0';
						++output_str;
					}

					if(Precision <= 6 && Precision >= 0)
						sprintf(spec_buf, "%%.%df", Precision);
					else
						sprintf(spec_buf, "%%f");
					
					sprintf(float_str, spec_buf, float_val);

					for(size_t c=0, len = strlen(float_str); c < len; ++c)
					{
						*output_str = float_str[c];
						++output_str;
					}
					break;
				}
				case 'e': 
				case 'E':
				{
					char *str_buf = NULL;
					amx_StrParam(amx, params[first_param_idx + param_counter], str_buf);
					if(str_buf != NULL)
					{
						string escaped_str;
						Handle->GetMainConnection()->EscapeString(str_buf, escaped_str);

						for(size_t c=0, len = escaped_str.length(); c < len; ++c)
						{
							*output_str = escaped_str.at(c);
							++output_str;
						}
					}
					break;
				}
				case 'X':
				{
					char hex_str[17];
					memset(hex_str, 0, 17);
					ConvertIntToStr<16>(*amx_address, hex_str);

					for(size_t c=0, len = strlen(hex_str); c < len; ++c)
					{
						if(hex_str[c] >= 'a' && hex_str[c] <= 'f')
							hex_str[c] = toupper(hex_str[c]);

						*output_str = hex_str[c];
						++output_str;
					}

					break;
				}
				case 'x':
				{
					char hex_str[17];
					memset(hex_str, 0, 17);
					ConvertIntToStr<16>(*amx_address, hex_str);

					for(size_t c=0, len = strlen(hex_str); c < len; ++c)
					{
						*output_str = hex_str[c];
						++output_str;
					}
					break;
				}
				case 'b':
				case 'B':
				{
					char bin_str[33];
					memset(bin_str, 0, 33);
					ConvertIntToStr<2>(*amx_address, bin_str);

					for(size_t c=0, len = strlen(bin_str); c < len; ++c)
					{
						*output_str = bin_str[c];
						++output_str;
					}
					break;
				}
				default:
					CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "invalid format specifier \"%%%c\"", *format_str);

			}
			param_counter++;
		}
		else 
		{
			*output_str = *format_str;
			++output_str;
		}
	}
	
	*output_str = '\0';
	amx_SetCString(amx, params[2], org_output_str, dest_len);
	free(org_output_str);
	return static_cast<cell>(output_str-org_output_str);
}

//native mysql_set_charset(charset[], connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::mysql_set_charset)
{
	const unsigned int connection_id = params[2];
	const char *charset = NULL;
	amx_StrParam(amx, params[1], charset);
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_set_charset", "charset: \"%s\", connection: %d", charset, connection_id);

	if(charset == NULL)
		return 0;

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_set_charset", connection_id);


	CMySQLHandle::GetHandle(connection_id)->ExecuteOnConnections(boost::bind(&CMySQLConnection::SetCharset, _1, charset));
	return 1;
}

//native mysql_get_charset(destination[], connectionHandle = 1, max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::mysql_get_charset)
{
	const unsigned int connection_id = params[2];
	const unsigned int max_size = params[3];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_get_charset", "connection: %d, max_len: %d", connection_id, max_size);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_get_charset", connection_id);


	const char *charset = mysql_character_set_name(CMySQLHandle::GetHandle(connection_id)->GetMainConnection()->GetMysqlPtr());
	amx_SetCString(amx, params[1], charset == NULL ? "NULL" : charset, max_size);
	return 1;
}

//native mysql_escape_string(const source[], destination[], connectionHandle = 1, max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::mysql_escape_string)
{
	const unsigned int connection_id = params[3];
	const char *source_str = NULL;
	amx_StrParam(amx, params[1], source_str);
	const unsigned int max_size = params[4];

	if(CLog::Get()->IsLogLevel(LOG_DEBUG))
	{
		string short_src(source_str == NULL ? "" : source_str);
		if(short_src.length() > 128)
		{
			short_src.erase(128, short_src.length());
			short_src.append("...");
		}
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_escape_string", "source: \"%s\", connection: %d, max_len: %d", short_src.c_str(), connection_id, max_size);
	}

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_escape_string", connection_id);
	

	string escaped_str;
	if(source_str != NULL) 
	{
		if(strlen(source_str) >= max_size)
			return CLog::Get()->LogFunction(LOG_ERROR, "mysql_escape_string", "destination size is too small (must be at least as big as source)");
		
		CMySQLHandle::GetHandle(connection_id)->GetMainConnection()->EscapeString(source_str, escaped_str);
	}

	amx_SetCString(amx, params[2], escaped_str.c_str(), max_size);
	return static_cast<cell>(escaped_str.length());
}

//native mysql_stat(destination[], connectionHandle = 1, max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::mysql_stat)
{
	const unsigned int connection_id = params[2];
	const unsigned int max_size = params[3];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_stat", "connection: %d, max_len: %d", connection_id, max_size);

	if(!CMySQLHandle::IsValid(connection_id))
		return ERROR_INVALID_CONNECTION_HANDLE("mysql_stat", connection_id);
	

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(connection_id);
	const char *stat_str = mysql_stat(Handle->GetMainConnection()->GetMysqlPtr());

	amx_SetCString(amx, params[1], stat_str == NULL ? "NULL" : stat_str, max_size);
	return 1;
}

//native mysql_errno(connectionHandle = 1);
AMX_DECLARE_NATIVE(Native::mysql_errno)
{
	const unsigned int connection_id = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_errno", "connection: %d", connection_id);

	if(!CMySQLHandle::IsValid(connection_id))
	{
		ERROR_INVALID_CONNECTION_HANDLE("mysql_errno", connection_id);
		return -1; //don't return 0 since it means that there are no errors
	}


	return static_cast<cell>(mysql_errno(CMySQLHandle::GetHandle(connection_id)->GetMainConnection()->GetMysqlPtr()));
}

//native mysql_log(loglevel, logtype);
AMX_DECLARE_NATIVE(Native::mysql_log)
{
	if(params[1] < 0)
		return 0;

	CLog::Get()->SetLogLevel(params[1]);
	CLog::Get()->SetLogType(params[2]);
	return 1;
}
