/*************************************************************************
 *
 *  $RCSfile: Awrapado.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: fs $ $Date: 2002-01-16 08:48:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#define _CONNECTIVITY_ADO_AWRAPADO_HXX_

// Includes fuer ADO
#include <oledb.h>
//#include <objbase.h>
//#include <initguid.h>
//#include <mapinls.h>
#include <ocidl.h>

#ifndef _ADOINT_H_
#include <adoint.h>
#endif


#ifndef _CONNECTIVITY_ADO_ADOIMP_HXX_
#include "ado/adoimp.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AOLEWRAP_HXX_
#include "ado/Aolewrap.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AOLEVARIANT_HXX_
#include "ado/Aolevariant.hxx"
#endif

namespace connectivity
{
    namespace ado
    {
#ifdef DBG_UTIL
        extern sal_Int32 nAdoObjectCounter;

        template <class IFACE>
        class WpCountedAdoObject : public WpOLEBase< IFACE >
        {
        protected:
            WpCountedAdoObject( IFACE* _pInt = NULL ) : WpOLEBase< IFACE >( _pInt )
            {
                ++nAdoObjectCounter;
            }
            ~WpCountedAdoObject()
            {
                --nAdoObjectCounter;
            }
        };
#endif

#ifdef DBG_UTIL
#define OBJECT_BASE     WpCountedAdoObject
#else
#define OBJECT_BASE     WpOLEBase
#endif

        class WpADOConnection;
        class WpADOCommand;
        class WpADORecordset;
        class WpADOField;
        class WpADOParameter;
        class WpADOError;
        class WpADOProperty;

        typedef WpOLEAppendCollection<ADOFields, ADOField, WpADOField> WpADOFields;

        //------------------------------------------------------------------------
        class WpADOConnection : public OBJECT_BASE<ADOConnection>
        {
            friend class WpADOCommand;
        public:

            WpADOConnection(ADOConnection* pInt)    :   OBJECT_BASE<ADOConnection>(pInt){}

            WpADOConnection(const WpADOConnection& rhs){operator=(rhs);}

             WpADOConnection& operator=(const WpADOConnection& rhs)
                {OBJECT_BASE<ADOConnection>::operator=(rhs); return *this;}

            //////////////////////////////////////////////////////////////////////

            ADOProperties* get_Properties() const;

             rtl::OUString GetConnectionString() const;

             sal_Bool PutConnectionString(const ::rtl::OUString &aCon) const;

             sal_Int32 GetCommandTimeout() const;

             void PutCommandTimeout(sal_Int32 nRet);

             sal_Int32 GetConnectionTimeout() const ;

             void PutConnectionTimeout(sal_Int32 nRet);

             sal_Bool Close( ) ;

             sal_Bool Execute(const ::rtl::OUString& _CommandText,OLEVariant& RecordsAffected,long Options, WpADORecordset** ppiRset);

             sal_Bool BeginTrans();

             sal_Bool CommitTrans( ) ;

             sal_Bool RollbackTrans( );

             sal_Bool Open(const ::rtl::OUString& ConnectionString, const ::rtl::OUString& UserID,const ::rtl::OUString& Password,long Options);

             sal_Bool GetErrors(ADOErrors** pErrors);

             ::rtl::OUString GetDefaultDatabase() const;

             sal_Bool PutDefaultDatabase(const ::rtl::OUString& _bstr);

             IsolationLevelEnum get_IsolationLevel() const ;

             sal_Bool put_IsolationLevel(const IsolationLevelEnum& eNum) ;

             sal_Int32 get_Attributes() const;

             sal_Bool put_Attributes(sal_Int32 nRet);

             CursorLocationEnum get_CursorLocation() const;

             sal_Bool put_CursorLocation(const CursorLocationEnum &eNum) ;

             ConnectModeEnum get_Mode() const;

             sal_Bool put_Mode(const ConnectModeEnum &eNum) ;

             ::rtl::OUString get_Provider() const;

             sal_Bool put_Provider(const ::rtl::OUString& _bstr);

             sal_Int32 get_State() const;

             sal_Bool OpenSchema(SchemaEnum eNum,OLEVariant& Restrictions,OLEVariant& SchemaID,ADORecordset**pprset);

             ::rtl::OUString get_Version() const;

             // special methods
             ADORecordset* getExportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table );
             ADORecordset* getImportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table );
             ADORecordset* getPrimaryKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table );
             ADORecordset* getIndexInfo( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Bool unique, sal_Bool approximate );
             ADORecordset* getTablePrivileges( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& tableNamePattern );
             ADORecordset* getCrossReference( const ::com::sun::star::uno::Any& primaryCatalog,
                                                  const ::rtl::OUString& primarySchema,
                                                  const ::rtl::OUString& primaryTable,
                                                  const ::com::sun::star::uno::Any& foreignCatalog,
                                                  const ::rtl::OUString& foreignSchema,
                                                  const ::rtl::OUString& foreignTable);
             ADORecordset* getProcedures( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& procedureNamePattern );
             ADORecordset* getProcedureColumns( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& procedureNamePattern,
                                                  const ::rtl::OUString& columnNamePattern );
             ADORecordset* getTables( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& tableNamePattern,
                                                  const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types );
             ADORecordset* getColumns( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& tableNamePattern,
                                                  const ::rtl::OUString& columnNamePattern );
             ADORecordset* getColumnPrivileges( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& table,
                                                  const ::rtl::OUString& columnNamePattern );
             ADORecordset* getTypeInfo( );
        };

        //------------------------------------------------------------------------
        class WpADOCommand : public OBJECT_BASE<ADOCommand>
        {
        public:
            WpADOCommand(){}
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOCommand(ADOCommand* pInt)  :   OBJECT_BASE<ADOCommand>(pInt){}

            WpADOCommand(const WpADOCommand& rhs){operator=(rhs);}

             WpADOCommand& operator=(const WpADOCommand& rhs)
            {
                OBJECT_BASE<ADOCommand>::operator=(rhs); return *this;}

            //////////////////////////////////////////////////////////////////////

            sal_Bool putref_ActiveConnection( WpADOConnection *pCon);

            void put_ActiveConnection(/* [in] */ const OLEVariant& vConn);
             void Create();
             sal_Int32 get_State() const;
             ::rtl::OUString get_CommandText() const;
             sal_Bool put_CommandText(const ::rtl::OUString &aCon) ;
             sal_Int32 get_CommandTimeout() const;
             void put_CommandTimeout(sal_Int32 nRet);
             sal_Bool get_Prepared() const;
             sal_Bool put_Prepared(VARIANT_BOOL bPrepared) const;
             sal_Bool Execute(OLEVariant& RecordsAffected,OLEVariant& Parameters,long Options, ADORecordset** ppiRset);
             ADOParameter* CreateParameter(const ::rtl::OUString &_bstr,DataTypeEnum Type,ParameterDirectionEnum Direction,long nSize,const OLEVariant &Value);

             ADOParameters* get_Parameters() const;
             sal_Bool put_CommandType( /* [in] */ CommandTypeEnum lCmdType);
             CommandTypeEnum get_CommandType( ) const ;
            // gibt den Namen des Feldes zur"ueck
             ::rtl::OUString GetName() const ;
             sal_Bool put_Name(const ::rtl::OUString& _Name);
             sal_Bool Cancel();
        };
        //------------------------------------------------------------------------
        class WpADOError : public OBJECT_BASE<ADOError>
        {
        public:

            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse

            WpADOError(ADOError* pInt):OBJECT_BASE<ADOError>(pInt){}

            WpADOError(const WpADOError& rhs){operator=(rhs);}

             WpADOError& operator=(const WpADOError& rhs)
                {OBJECT_BASE<ADOError>::operator=(rhs); return *this;}

            //////////////////////////////////////////////////////////////////////

             ::rtl::OUString GetDescription() const;

             ::rtl::OUString GetSource() const ;

             sal_Int32 GetNumber() const ;
             ::rtl::OUString GetSQLState() const ;
             sal_Int32 GetNativeError() const ;
        };



        //------------------------------------------------------------------------
        class WpADOField : public OBJECT_BASE<ADOField>
        {
            //  friend class WpADOFields;
        public:

            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOField(ADOField* pInt=NULL):OBJECT_BASE<ADOField>(pInt){}
            WpADOField(const WpADOField& rhs){operator=(rhs);}

             WpADOField& operator=(const WpADOField& rhs)
                {OBJECT_BASE<ADOField>::operator=(rhs); return *this;}
            //////////////////////////////////////////////////////////////////////

             ADOProperties* get_Properties();
             sal_Int32 GetActualSize() const ;
             sal_Int32 GetAttributes() const ;
             sal_Int32 GetStatus() const      ;
             sal_Int32 GetDefinedSize() const ;
            // gibt den Namen des Feldes zur"ueck
             ::rtl::OUString GetName() const ;
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



        //------------------------------------------------------------------------
        class WpADOProperty: public OBJECT_BASE<ADOProperty>
        {
            friend class WpADOProperties;

        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOProperty(ADOProperty* pInt):OBJECT_BASE<ADOProperty>(pInt){}
            WpADOProperty(const WpADOProperty& rhs){operator=(rhs);}
             WpADOProperty& operator=(const WpADOProperty& rhs)
                {OBJECT_BASE<ADOProperty>::operator=(rhs); return *this;}
            //////////////////////////////////////////////////////////////////////

             OLEVariant GetValue() const;
             void GetValue(OLEVariant &aValVar) const;
             sal_Bool PutValue(const OLEVariant &aValVar) ;
             ::rtl::OUString GetName() const ;
             DataTypeEnum GetADOType() const ;
             sal_Int32 GetAttributes() const ;
             sal_Bool PutAttributes(sal_Int32 _nDefSize);
        };


        class WpADORecordset : public OBJECT_BASE<ADORecordset>
        {

        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADORecordset(ADORecordset* pInt=NULL):OBJECT_BASE<ADORecordset>(pInt){}
            WpADORecordset(const WpADORecordset& rhs){operator=(rhs);}
             WpADORecordset& operator=(const WpADORecordset& rhs)
            {
                OBJECT_BASE<ADORecordset>::operator=(rhs);
                return *this;
            }
            ~WpADORecordset()
            {
                //  if(pInterface && get_State() == adStateOpen)
                    //  Close();
            }
            //////////////////////////////////////////////////////////////////////
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
            PositionEnum get_AbsolutePosition();
             void GetDataSource(IUnknown** pIUnknown) const ;
             void PutRefDataSource(IUnknown* pIUnknown);
             void GetBookmark(VARIANT& var);
             OLEVariant GetBookmark();
            CompareEnum CompareBookmarks(const OLEVariant& left,const OLEVariant& right);
             sal_Bool SetBookmark(const OLEVariant &pSafeAr);
             ADOFields* GetFields() const;
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
             ADOProperties* get_Properties() const;
             sal_Bool NextRecordset(OLEVariant& RecordsAffected,ADORecordset** ppiRset);
             sal_Bool get_RecordCount(sal_Int32 &_nRet) const;
             sal_Bool get_MaxRecords(sal_Int32 &_nRet) const;
             sal_Bool put_MaxRecords(sal_Int32 _nRet);
             sal_Bool get_CursorType(CursorTypeEnum &_nRet) const;
             sal_Bool put_CursorType(CursorTypeEnum _nRet);
             sal_Bool get_LockType(LockTypeEnum &_nRet) const;
             sal_Bool put_LockType(LockTypeEnum _nRet);
             sal_Bool get_CacheSize(sal_Int32 &_nRet) const;
             sal_Bool put_CacheSize(sal_Int32 _nRet);
             sal_Bool UpdateBatch(AffectEnum AffectRecords);
        };

        //------------------------------------------------------------------------
        class WpADOParameter:public OBJECT_BASE<ADOParameter>
        {
        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOParameter(ADOParameter* pInt):OBJECT_BASE<ADOParameter>(pInt){}
            WpADOParameter(const WpADOParameter& rhs):OBJECT_BASE<ADOParameter>(rhs){}
             WpADOParameter& operator=(const WpADOParameter& rhs)
                {OBJECT_BASE<ADOParameter>::operator=(rhs); return *this;}
            //////////////////////////////////////////////////////////////////////

             ::rtl::OUString GetName() const ;
             DataTypeEnum GetADOType() const ;
             sal_Int32 GetAttributes() const ;
             sal_Int32 GetPrecision() const ;
             sal_Int32 GetNumericScale() const ;
             ParameterDirectionEnum get_Direction() const;
             void GetValue(OLEVariant& aValVar) const ;
             OLEVariant GetValue() const;
             sal_Bool PutValue(const OLEVariant& aVariant);
        };
    }
}
#endif //_CONNECTIVITY_ADO_AWRAPADO_HXX_

