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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AWRAPADO_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AWRAPADO_HXX

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/beans/PropertyAttribute.hpp>

// Includes for ADO
#include <oledb.h>
#include <ocidl.h>
#include <adoint.h>
#include <ado/adoimp.hxx>
#include <ado/Aolewrap.hxx>
#include <ado/Aolevariant.hxx>

namespace connectivity::ado
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
             bool PutConnectionString(std::u16string_view aCon) const;
             sal_Int32 GetCommandTimeout() const;
             void PutCommandTimeout(sal_Int32 nRet);
             sal_Int32 GetConnectionTimeout() const ;
             void PutConnectionTimeout(sal_Int32 nRet);

             bool Close( ) ;
             bool Execute(std::u16string_view CommandText,OLEVariant& RecordsAffected,long Options, WpADORecordset** ppiRset);
             bool BeginTrans();
             bool CommitTrans( ) ;
             bool RollbackTrans( );
             bool Open(std::u16string_view ConnectionString, std::u16string_view UserID,std::u16string_view Password,long Options);
             bool GetErrors(ADOErrors** pErrors);

             OUString GetDefaultDatabase() const;
             bool PutDefaultDatabase(std::u16string_view _bstr);

             IsolationLevelEnum get_IsolationLevel() const ;
             bool put_IsolationLevel(const IsolationLevelEnum& eNum) ;

             sal_Int32 get_Attributes() const;
             bool put_Attributes(sal_Int32 nRet);

             CursorLocationEnum get_CursorLocation() const;
             bool put_CursorLocation(const CursorLocationEnum &eNum) ;

             ConnectModeEnum get_Mode() const;
             bool put_Mode(const ConnectModeEnum &eNum) ;

             OUString get_Provider() const;
             bool put_Provider(std::u16string_view _bstr);

             sal_Int32 get_State() const;

             bool OpenSchema(SchemaEnum eNum,OLEVariant const & Restrictions,OLEVariant const & SchemaID,ADORecordset**pprset);

             OUString get_Version() const;

             // special methods
             ADORecordset* getExportedKeys( const css::uno::Any& catalog, const OUString& schema, std::u16string_view table );
             ADORecordset* getImportedKeys( const css::uno::Any& catalog, const OUString& schema, std::u16string_view table );
             ADORecordset* getPrimaryKeys( const css::uno::Any& catalog, const OUString& schema, std::u16string_view table );
             ADORecordset* getIndexInfo( const css::uno::Any& catalog, const OUString& schema, std::u16string_view table, bool unique, bool approximate );
             ADORecordset* getTablePrivileges( const css::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& tableNamePattern );
             ADORecordset* getCrossReference( const css::uno::Any& primaryCatalog,
                                                  const OUString& primarySchema,
                                                  std::u16string_view primaryTable,
                                                  const css::uno::Any& foreignCatalog,
                                                  const OUString& foreignSchema,
                                                  std::u16string_view foreignTable);
             ADORecordset* getProcedures( const css::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& procedureNamePattern );
             ADORecordset* getProcedureColumns( const css::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& procedureNamePattern,
                                                  const OUString& columnNamePattern );
             ADORecordset* getTables( const css::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& tableNamePattern,
                                                  const css::uno::Sequence< OUString >& types );
             ADORecordset* getColumns( const css::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  const OUString& tableNamePattern,
                                                  std::u16string_view columnNamePattern );
             ADORecordset* getColumnPrivileges( const css::uno::Any& catalog,
                                                  const OUString& schemaPattern,
                                                  std::u16string_view table,
                                                  std::u16string_view columnNamePattern );
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


            bool putref_ActiveConnection( WpADOConnection *pCon);

            void put_ActiveConnection(/* [in] */ const OLEVariant& vConn);
             void Create();
             sal_Int32 get_State() const;
             OUString get_CommandText() const;
             bool put_CommandText(std::u16string_view aCon) ;
             sal_Int32 get_CommandTimeout() const;
             void put_CommandTimeout(sal_Int32 nRet);
             bool get_Prepared() const;
             bool put_Prepared(VARIANT_BOOL bPrepared) const;
             bool Execute(OLEVariant& RecordsAffected,OLEVariant& Parameters,long Options, ADORecordset** ppiRset);
             ADOParameter* CreateParameter(std::u16string_view _bstr,DataTypeEnum Type,ParameterDirectionEnum Direction,long nSize,const OLEVariant &Value);

             ADOParameters* get_Parameters() const;
             bool put_CommandType( /* [in] */ CommandTypeEnum lCmdType);
             CommandTypeEnum get_CommandType( ) const ;
             // Returns the field's name
             OUString GetName() const ;
             bool put_Name(std::u16string_view Name);
             bool Cancel();
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
            WpADOField(ADOField* pInt=nullptr):WpOLEBase<ADOField>(pInt){}
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
             bool PutValue(const OLEVariant& aVariant);
             sal_Int32 GetPrecision() const ;
             sal_Int32 GetNumericScale() const ;
             bool AppendChunk(const OLEVariant& Variant);
             OLEVariant GetChunk(long Length) const;
             void GetChunk(long Length,OLEVariant &aValVar) const;
             OLEVariant GetOriginalValue() const;
             void GetOriginalValue(OLEVariant &aValVar) const;
             OLEVariant GetUnderlyingValue() const;

             void GetUnderlyingValue(OLEVariant &aValVar) const;

             bool PutPrecision(sal_Int8 _prec);

             bool PutNumericScale(sal_Int8 _prec);

             void PutADOType(DataTypeEnum eType) ;

             bool PutDefinedSize(sal_Int32 _nDefSize);

             bool PutAttributes(sal_Int32 _nDefSize);
        };


        class WpADOProperty: public WpOLEBase<ADOProperty>
        {
        public:
            // Ctors, operator=
            // They only call the superclass
            WpADOProperty(ADOProperty* pInt=nullptr):WpOLEBase<ADOProperty>(pInt){}
            WpADOProperty(const WpADOProperty& rhs) : WpOLEBase<ADOProperty>(rhs) {}
             WpADOProperty& operator=(const WpADOProperty& rhs)
                {WpOLEBase<ADOProperty>::operator=(rhs); return *this;}


             OLEVariant GetValue() const;
             void GetValue(OLEVariant &aValVar) const;
             bool PutValue(const OLEVariant &aValVar) ;
             OUString GetName() const ;
             DataTypeEnum GetADOType() const ;
             sal_Int32 GetAttributes() const ;
             bool PutAttributes(sal_Int32 _nDefSize);
        };


        class WpADORecordset : public WpOLEBase<ADORecordset>
        {

        public:
            // Ctors, operator=
            // They only call the superclass
            WpADORecordset(ADORecordset* pInt=nullptr):WpOLEBase<ADORecordset>(pInt){}
            WpADORecordset(const WpADORecordset& rhs) : WpOLEBase<ADORecordset>() {operator=(rhs);}
             WpADORecordset& operator=(const WpADORecordset& rhs)
            {
                WpOLEBase<ADORecordset>::operator=(rhs);
                return *this;
            }

             void Create();
             bool Open(
                    /* [optional][in] */ VARIANT Source,
                    /* [optional][in] */ VARIANT ActiveConnection,
                    /* [defaultvalue][in] */ CursorTypeEnum CursorType,
                    /* [defaultvalue][in] */ LockTypeEnum LockType,
                    /* [defaultvalue][in] */ sal_Int32 Options);
             LockTypeEnum GetLockType();
             void Close();
             bool Cancel() const;
             sal_Int32 get_State( );
             bool Supports( /* [in] */ CursorOptionEnum CursorOptions);
            PositionEnum_Param get_AbsolutePosition();
             void GetDataSource(IUnknown** pIUnknown) const ;
             void PutRefDataSource(IUnknown* pIUnknown);
             void GetBookmark(VARIANT& var);
             OLEVariant GetBookmark();
            CompareEnum CompareBookmarks(const OLEVariant& left,const OLEVariant& right);
             bool SetBookmark(const OLEVariant &pSafeAr);
             WpADOFields GetFields() const;
             bool Move(sal_Int32 nRows, VARIANT aBmk);
             bool MoveNext();
             bool MovePrevious();
             bool MoveFirst();
             bool MoveLast();

             bool IsAtBOF() const;
             bool IsAtEOF() const;
             bool Delete(AffectEnum eNum);
             bool AddNew(const OLEVariant &FieldList,const OLEVariant &Values);
             bool Update(const OLEVariant &FieldList,const OLEVariant &Values);
             bool CancelUpdate();
             WpADOProperties get_Properties() const;
             bool NextRecordset(OLEVariant& RecordsAffected,ADORecordset** ppiRset);
             bool get_RecordCount(ADO_LONGPTR &_nRet) const;
             bool get_MaxRecords(ADO_LONGPTR &_nRet) const;
             bool put_MaxRecords(ADO_LONGPTR _nRet);
             bool get_CursorType(CursorTypeEnum &_nRet) const;
             bool put_CursorType(CursorTypeEnum _nRet);
             bool get_LockType(LockTypeEnum &_nRet) const;
             bool put_LockType(LockTypeEnum _nRet);
             bool get_CacheSize(sal_Int32 &_nRet) const;
             bool put_CacheSize(sal_Int32 _nRet);
             bool UpdateBatch(AffectEnum AffectRecords);
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
             bool put_Size(sal_Int32 _nSize);
             sal_Int32 GetAttributes() const ;
             sal_Int32 GetPrecision() const ;
             sal_Int32 GetNumericScale() const ;
             ParameterDirectionEnum get_Direction() const;
             void GetValue(OLEVariant& aValVar) const ;
             OLEVariant GetValue() const;
             bool PutValue(const OLEVariant& aVariant);
             bool AppendChunk(const OLEVariant& aVariant);
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

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AWRAPADO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
