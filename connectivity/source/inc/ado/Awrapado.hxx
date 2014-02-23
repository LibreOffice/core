/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#define _CONNECTIVITY_ADO_AWRAPADO_HXX_

#include <com/sun/star/beans/PropertyAttribute.hpp>

// Includes for ADO
#include "ado_pre_sys_include.h"
#include <oledb.h>
#include <ocidl.h>
#include <adoint.h>
#include "ado_post_sys_include.h"
#include "ado/adoimp.hxx"
#include "ado/Aolewrap.hxx"
#include "ado/Aolevariant.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOConnection;
        class WpADOCommand;
        class WpADORecordset;
        class WpADOField;
        class WpADOParameter;
        class WpADOError;
        class WpADOProperty;

        typedef WpOLEAppendCollection<  ADOFields,      ADOField,       WpADOField>         WpADOFields;
        typedef WpOLECollection<        ADOProperties,  ADOProperty,    WpADOProperty>      WpADOProperties;


        class WpADOConnection : public WpOLEBase<ADOConnection>
        {
            friend class WpADOCommand;
        public:

            WpADOConnection(ADOConnection* pInt)    :   WpOLEBase<ADOConnection>(pInt){}

            WpADOConnection(const WpADOConnection& rhs) : WpOLEBase<ADOConnection>(rhs) {}

             WpADOConnection& operator=(const WpADOConnection& rhs)
                {WpOLEBase<ADOConnection>::operator=(rhs); return *this;}



            WpADOProperties get_Properties() const;

             OUString GetConnectionString() const;
             sal_Bool PutConnectionString(const OUString &aCon) const;
             sal_Int32 GetCommandTimeout() const;
             void PutCommandTimeout(sal_Int32 nRet);
             sal_Int32 GetConnectionTimeout() const ;
             void PutConnectionTimeout(sal_Int32 nRet);

             sal_Bool Close( ) ;
             sal_Bool Execute(const OUString& _CommandText,OLEVariant& RecordsAffected,long Options, WpADORecordset** ppiRset);
             sal_Bool BeginTrans();
             sal_Bool CommitTrans( ) ;
             sal_Bool RollbackTrans( );
             sal_Bool Open(const OUString& ConnectionString, const OUString& UserID,const OUString& Password,long Options);
             sal_Bool GetErrors(ADOErrors** pErrors);

             OUString GetDefaultDatabase() const;
             sal_Bool PutDefaultDatabase(const OUString& _bstr);

             IsolationLevelEnum get_IsolationLevel() const ;
             sal_Bool put_IsolationLevel(const IsolationLevelEnum& eNum) ;

             sal_Int32 get_Attributes() const;
             sal_Bool put_Attributes(sal_Int32 nRet);

             CursorLocationEnum get_CursorLocation() const;
             sal_Bool put_CursorLocation(const CursorLocationEnum &eNum) ;

             ConnectModeEnum get_Mode() const;
             sal_Bool put_Mode(const ConnectModeEnum &eNum) ;

             OUString get_Provider() const;
             sal_Bool put_Provider(const OUString& _bstr);

             sal_Int32 get_State() const;

             sal_Bool OpenSchema(SchemaEnum eNum,OLEVariant& Restrictions,OLEVariant& SchemaID,ADORecordset**pprset);

             OUString get_Version() const;

             // special methods
             ADORecordset* getExportedKeys( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table );
             ADORecordset* getImportedKeys( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table );
             ADORecordset* getPrimaryKeys( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table );
             ADORecordset* getIndexInfo( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate );
             ADORecordset* getTablePrivileges( const ::com::sun::star::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& tableNamePattern );
             ADORecordset* getCrossReference( const ::com::sun::star::uno::Any& primaryCatalog,
                                                  const OUString& primarySchema,
                                                  const OUString& primaryTable,
                                                  const ::com::sun::star::uno::Any& foreignCatalog,
                                                  const OUString& foreignSchema,
                                                  const OUString& foreignTable);
             ADORecordset* getProcedures( const ::com::sun::star::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& procedureNamePattern );
             ADORecordset* getProcedureColumns( const ::com::sun::star::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& procedureNamePattern,
                                                  const OUString& columnNamePattern );
             ADORecordset* getTables( const ::com::sun::star::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& tableNamePattern,
                                                  const ::com::sun::star::uno::Sequence< OUString >& types );
             ADORecordset* getColumns( const ::com::sun::star::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& tableNamePattern,
                                                  const OUString& columnNamePattern );
             ADORecordset* getColumnPrivileges( const ::com::sun::star::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& table,
                                                  const OUString& columnNamePattern );
             ADORecordset* getTypeInfo(DataTypeEnum _eType = adEmpty );
        };


        class WpADOCommand : public WpOLEBase<ADOCommand>
        {
        public:
            WpADOCommand(){}
            // Ctors, operator=
            // They only call the superclass
            WpADOCommand(ADOCommand* pInt)  :   WpOLEBase<ADOCommand>(pInt){}

            WpADOCommand(const WpADOCommand& rhs) : WpOLEBase<ADOCommand>(rhs) {}

             WpADOCommand& operator=(const WpADOCommand& rhs)
            {
                WpOLEBase<ADOCommand>::operator=(rhs); return *this;}



            sal_Bool putref_ActiveConnection( WpADOConnection *pCon);

            void put_ActiveConnection(/* [in] */ const OLEVariant& vConn);
             void Create();
             sal_Int32 get_State() const;
             OUString get_CommandText() const;
             sal_Bool put_CommandText(const OUString &aCon) ;
             sal_Int32 get_CommandTimeout() const;
             void put_CommandTimeout(sal_Int32 nRet);
             sal_Bool get_Prepared() const;
             sal_Bool put_Prepared(VARIANT_BOOL bPrepared) const;
             sal_Bool Execute(OLEVariant& RecordsAffected,OLEVariant& Parameters,long Options, ADORecordset** ppiRset);
             ADOParameter* CreateParameter(const OUString &_bstr,DataTypeEnum Type,ParameterDirectionEnum Direction,long nSize,const OLEVariant &Value);

             ADOParameters* get_Parameters() const;
             sal_Bool put_CommandType( /* [in] */ CommandTypeEnum lCmdType);
             CommandTypeEnum get_CommandType( ) const ;
             // Returns the field's name
             OUString GetName() const ;
             sal_Bool put_Name(const OUString& _Name);
             sal_Bool Cancel();
        };

        class WpADOError : public WpOLEBase<ADOError>
        {
        public:

            // Ctors, operator=
            // They only call the superclass
            WpADOError(ADOError* pInt):WpOLEBase<ADOError>(pInt){}

            WpADOError(const WpADOError& rhs) : WpOLEBase<ADOError>(rhs) {}

             WpADOError& operator=(const WpADOError& rhs)
                {WpOLEBase<ADOError>::operator=(rhs); return *this;}



             OUString GetDescription() const;
             OUString GetSource() const ;
             sal_Int32 GetNumber() const ;
             OUString GetSQLState() const ;
             sal_Int32 GetNativeError() const ;
        };




        class WpADOField : public WpOLEBase<ADOField>
        {
            //  friend class WpADOFields;
        public:

            // Ctors, operator=
            // They only call the superclass
            WpADOField(ADOField* pInt=NULL):WpOLEBase<ADOField>(pInt){}
            WpADOField(const WpADOField& rhs) : WpOLEBase<ADOField>(rhs) {}

             WpADOField& operator=(const WpADOField& rhs)
                {WpOLEBase<ADOField>::operator=(rhs); return *this;}


             WpADOProperties get_Properties();
             sal_Int32 GetActualSize() const ;
             sal_Int32 GetAttributes() const ;
             sal_Int32 GetStatus() const      ;
             sal_Int32 GetDefinedSize() const ;
             // Returns the field's name
             OUString GetName() const ;
             DataTypeEnum GetADOType() const  ;
             void get_Value(OLEVariant& aValVar) const ;
             OLEVariant get_Value() const;
             sal_Bool PutValue(const OLEVariant& aVariant);
             sal_Int32 GetPrecision() const ;
             sal_Int32 GetNumericScale() const ;
             sal_Bool AppendChunk(const OLEVariant& _Variant);
             OLEVariant GetChunk(long Length) const;
             void GetChunk(long Length,OLEVariant &aValVar) const;
             OLEVariant GetOriginalValue() const;
             void GetOriginalValue(OLEVariant &aValVar) const;
             OLEVariant GetUnderlyingValue() const;

             void GetUnderlyingValue(OLEVariant &aValVar) const;

             sal_Bool PutPrecision(sal_Int8 _prec);

             sal_Bool PutNumericScale(sal_Int8 _prec);

             void PutADOType(DataTypeEnum eType) ;

             sal_Bool PutDefinedSize(sal_Int32 _nDefSize);

             sal_Bool PutAttributes(sal_Int32 _nDefSize);
        };




        class WpADOProperty: public WpOLEBase<ADOProperty>
        {
        public:
            // Ctors, operator=
            // They only call the superclass
            WpADOProperty(ADOProperty* pInt=NULL):WpOLEBase<ADOProperty>(pInt){}
            WpADOProperty(const WpADOProperty& rhs) : WpOLEBase<ADOProperty>(rhs) {}
             WpADOProperty& operator=(const WpADOProperty& rhs)
                {WpOLEBase<ADOProperty>::operator=(rhs); return *this;}


             OLEVariant GetValue() const;
             void GetValue(OLEVariant &aValVar) const;
             sal_Bool PutValue(const OLEVariant &aValVar) ;
             OUString GetName() const ;
             DataTypeEnum GetADOType() const ;
             sal_Int32 GetAttributes() const ;
             sal_Bool PutAttributes(sal_Int32 _nDefSize);
        };


        class WpADORecordset : public WpOLEBase<ADORecordset>
        {

        public:
            // Ctors, operator=
            // They only call the superclass
            WpADORecordset(ADORecordset* pInt=NULL):WpOLEBase<ADORecordset>(pInt){}
            WpADORecordset(const WpADORecordset& rhs) : WpOLEBase<ADORecordset>() {operator=(rhs);}
             WpADORecordset& operator=(const WpADORecordset& rhs)
            {
                WpOLEBase<ADORecordset>::operator=(rhs);
                return *this;
            }
            ~WpADORecordset()
            {
                //  if(pInterface && get_State() == adStateOpen)
                    //  Close();
            }

             void Create();
             sal_Bool Open(
                    /* [optional][in] */ VARIANT Source,
                    /* [optional][in] */ VARIANT ActiveConnection,
                    /* [defaultvalue][in] */ CursorTypeEnum CursorType,
                    /* [defaultvalue][in] */ LockTypeEnum LockType,
                    /* [defaultvalue][in] */ sal_Int32 Options);
             LockTypeEnum GetLockType();
             void Close();
             sal_Bool Cancel() const;
             sal_Int32 get_State( );
             sal_Bool Supports( /* [in] */ CursorOptionEnum CursorOptions);
            PositionEnum_Param get_AbsolutePosition();
             void GetDataSource(IUnknown** pIUnknown) const ;
             void PutRefDataSource(IUnknown* pIUnknown);
             void GetBookmark(VARIANT& var);
             OLEVariant GetBookmark();
            CompareEnum CompareBookmarks(const OLEVariant& left,const OLEVariant& right);
             sal_Bool SetBookmark(const OLEVariant &pSafeAr);
             WpADOFields GetFields() const;
             sal_Bool Move(sal_Int32 nRows, VARIANT aBmk);
             sal_Bool MoveNext();
             sal_Bool MovePrevious();
             sal_Bool MoveFirst();
             sal_Bool MoveLast();

             sal_Bool IsAtBOF() const;
             sal_Bool IsAtEOF() const;
             sal_Bool Delete(AffectEnum eNum);
             sal_Bool AddNew(const OLEVariant &FieldList,const OLEVariant &Values);
             sal_Bool Update(const OLEVariant &FieldList,const OLEVariant &Values);
             sal_Bool CancelUpdate();
             WpADOProperties get_Properties() const;
             sal_Bool NextRecordset(OLEVariant& RecordsAffected,ADORecordset** ppiRset);
             sal_Bool get_RecordCount(ADO_LONGPTR &_nRet) const;
             sal_Bool get_MaxRecords(ADO_LONGPTR &_nRet) const;
             sal_Bool put_MaxRecords(ADO_LONGPTR _nRet);
             sal_Bool get_CursorType(CursorTypeEnum &_nRet) const;
             sal_Bool put_CursorType(CursorTypeEnum _nRet);
             sal_Bool get_LockType(LockTypeEnum &_nRet) const;
             sal_Bool put_LockType(LockTypeEnum _nRet);
             sal_Bool get_CacheSize(sal_Int32 &_nRet) const;
             sal_Bool put_CacheSize(sal_Int32 _nRet);
             sal_Bool UpdateBatch(AffectEnum AffectRecords);
        };


        class WpADOParameter:public WpOLEBase<ADOParameter>
        {
        public:
            // Ctors, operator=
            // They only call the superclass
            WpADOParameter(ADOParameter* pInt):WpOLEBase<ADOParameter>(pInt){}
            WpADOParameter(const WpADOParameter& rhs):WpOLEBase<ADOParameter>(rhs){}
             WpADOParameter& operator=(const WpADOParameter& rhs)
                {WpOLEBase<ADOParameter>::operator=(rhs); return *this;}


             OUString GetName() const ;
             DataTypeEnum GetADOType() const ;
             void put_Type(const DataTypeEnum& _eType);
             sal_Bool put_Size(const sal_Int32& _nSize);
             sal_Int32 GetAttributes() const ;
             sal_Int32 GetPrecision() const ;
             sal_Int32 GetNumericScale() const ;
             ParameterDirectionEnum get_Direction() const;
             void GetValue(OLEVariant& aValVar) const ;
             OLEVariant GetValue() const;
             sal_Bool PutValue(const OLEVariant& aVariant);
             sal_Bool AppendChunk(const OLEVariant& aVariant);
        };

        class OTools
        {
        public:
            /** putValue set the property value at the ado column
                @param  _rProps     the properties where to set
                @param  _aPosition  which property to set
                @param  _aValVar    the value to set
            */
            static void             putValue(const WpADOProperties& _rProps,const OLEVariant &_aPosition,const OLEVariant &_aValVar);

            /** getValue returns a specific property value
                @param  _rProps     the properties where to set
                @param  _aPosition  the property

                @return the property value
            */
            static OLEVariant       getValue(const WpADOProperties& _rProps,const OLEVariant &_aPosition);
        };
    }
}
#endif //_CONNECTIVITY_ADO_AWRAPADO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
