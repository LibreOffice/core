/*************************************************************************
 *
 *  $RCSfile: Awrapado.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-04 09:08:48 $
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

#include <tools/prewin.h>
// Includes fuer ADO
#include <oledb.h>
//#include <objbase.h>
//#include <initguid.h>
//#include <mapinls.h>
#include <ocidl.h>

#ifndef _ADOINT_H_
#include <adoint.h>
#endif

#include <tools/postwin.h>

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
        class WpADOConnection;
        class WpADOCommand;
        class WpADORecordset;
        class WpADOField;
        class WpADOParameter;
        class WpADOError;
        class WpADOProperty;

        typedef WpOLEAppendCollection<ADOFields, ADOField, WpADOField> WpADOFields;

        //------------------------------------------------------------------------
        class WpADOConnection : public WpOLEBase<ADOConnection>
        {
            friend class WpADOCommand;
        public:

            WpADOConnection(ADOConnection* pInt)    :   WpOLEBase<ADOConnection>(pInt){}

            WpADOConnection(const WpADOConnection& rhs){operator=(rhs);}

            inline WpADOConnection& operator=(const WpADOConnection& rhs)
                {WpOLEBase<ADOConnection>::operator=(rhs); return *this;}

            //////////////////////////////////////////////////////////////////////

            inline ADOProperties* get_Properties() const
            {
                ADOProperties* pProps=NULL;
                pInterface->get_Properties(&pProps);
                return pProps;
            }

            inline rtl::OUString GetConnectionString() const
            {
                BSTR aBSTR; pInterface->get_ConnectionString(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            inline sal_Bool PutConnectionString(const ::rtl::OUString &aCon) const
            {
                BSTR bstr = SysAllocString(aCon.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->put_ConnectionString(bstr));
                SysFreeString(bstr);
                return bErg;
            }

            inline sal_Int32 GetCommandTimeout() const
            {
                sal_Int32 nRet;
                pInterface->get_CommandTimeout(&nRet);
                return nRet;
            }

            inline void PutCommandTimeout(sal_Int32 nRet)
            {
                pInterface->put_CommandTimeout(nRet);
            }

            inline sal_Int32 GetConnectionTimeout() const
            {
                sal_Int32 nRet;
                pInterface->get_ConnectionTimeout(&nRet);
                return nRet;
            }

            inline void PutConnectionTimeout(sal_Int32 nRet)
            {
                pInterface->put_ConnectionTimeout(nRet);
            }

            inline sal_Bool Close( )
            {
                return (SUCCEEDED(pInterface->Close()));
            }

            inline sal_Bool Execute(const ::rtl::OUString& _CommandText,OLEVariant& RecordsAffected,long Options, WpADORecordset** ppiRset)
            {
                BSTR sStr1 = SysAllocString(_CommandText.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->Execute(sStr1,&RecordsAffected,Options,(_ADORecordset**)ppiRset));
                SysFreeString(sStr1);
                return bErg;
            }

            inline sal_Bool BeginTrans()
            {
                sal_Int32 nIso;
                return SUCCEEDED(pInterface->BeginTrans(&nIso));
            }

            inline sal_Bool CommitTrans( )
            {
                return SUCCEEDED(pInterface->CommitTrans());
            }

            inline sal_Bool RollbackTrans( )
            {
                return SUCCEEDED(pInterface->RollbackTrans());
            }

            inline sal_Bool Open(const ::rtl::OUString& ConnectionString, const ::rtl::OUString& UserID,const ::rtl::OUString& Password,long Options)
            {
                BSTR sStr1 = SysAllocString(ConnectionString.getStr());
                BSTR sStr2 = SysAllocString(UserID.getStr());
                BSTR sStr3 = SysAllocString(Password.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->Open(sStr1,sStr2,sStr3,Options));
                SysFreeString(sStr1);
                SysFreeString(sStr2);
                SysFreeString(sStr3);
                return bErg;
            }

            inline sal_Bool GetErrors(ADOErrors** pErrors)
            {
                return SUCCEEDED(pInterface->get_Errors(pErrors));
            }

            inline ::rtl::OUString GetDefaultDatabase() const
            {
                BSTR aBSTR; pInterface->get_DefaultDatabase(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline sal_Bool PutDefaultDatabase(const ::rtl::OUString& _bstr)
            {
                BSTR bstr = SysAllocString(_bstr.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->put_DefaultDatabase(bstr));
                SysFreeString(bstr);
                return bErg;
            }

            inline IsolationLevelEnum get_IsolationLevel() const
            {
                IsolationLevelEnum eNum;
                pInterface->get_IsolationLevel(&eNum);
                return eNum;
            }

            inline sal_Bool put_IsolationLevel(const IsolationLevelEnum& eNum)
            {
                return SUCCEEDED(pInterface->put_IsolationLevel(eNum));
            }

            inline sal_Int32 get_Attributes() const
            {
                sal_Int32 nRet;
                pInterface->get_Attributes(&nRet);
                return nRet;
            }

            inline sal_Bool put_Attributes(sal_Int32 nRet)
            {
                return SUCCEEDED(pInterface->put_Attributes(nRet));
            }

            inline CursorLocationEnum get_CursorLocation() const
            {
                CursorLocationEnum eNum;
                pInterface->get_CursorLocation(&eNum);
                return eNum;
            }

            inline sal_Bool put_CursorLocation(const CursorLocationEnum &eNum)
            {
                return SUCCEEDED(pInterface->put_CursorLocation(eNum));
            }

            inline ConnectModeEnum get_Mode() const
            {
                ConnectModeEnum eNum;
                pInterface->get_Mode(&eNum);
                return eNum;
            }

            inline sal_Bool put_Mode(const ConnectModeEnum &eNum)
            {

                return SUCCEEDED(pInterface->put_Mode(eNum));
            }

            inline ::rtl::OUString get_Provider() const
            {
                BSTR aBSTR; pInterface->get_Provider(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline sal_Bool put_Provider(const ::rtl::OUString& _bstr)
            {
                BSTR bstr = SysAllocString(_bstr.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->put_Provider(bstr));
                SysFreeString(bstr);
                return bErg;
            }

            inline sal_Int32 get_State() const
            {
                sal_Int32 nRet;
                pInterface->get_State(&nRet);
                return nRet;
            }

            inline sal_Bool OpenSchema(SchemaEnum eNum,OLEVariant& Restrictions,OLEVariant& SchemaID,ADORecordset**pprset)
            {
                return SUCCEEDED(pInterface->OpenSchema(eNum,Restrictions,SchemaID,pprset));
            }

            inline ::rtl::OUString get_Version() const
            {
                BSTR aBSTR;
                pInterface->get_Version(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }
        };

        //------------------------------------------------------------------------
        class WpADOCommand : public WpOLEBase<ADOCommand>
        {
        public:
            WpADOCommand(){}
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOCommand(ADOCommand* pInt)  :   WpOLEBase<ADOCommand>(pInt){}

            WpADOCommand(const WpADOCommand& rhs){operator=(rhs);}

            inline WpADOCommand& operator=(const WpADOCommand& rhs)
            {
                WpOLEBase<ADOCommand>::operator=(rhs); return *this;}

            //////////////////////////////////////////////////////////////////////

            sal_Bool putref_ActiveConnection( WpADOConnection *pCon)
            {
                return SUCCEEDED(pInterface->putref_ActiveConnection(pCon->pInterface));
            }

            void put_ActiveConnection(/* [in] */ const OLEVariant& vConn)
            {
                pInterface->put_ActiveConnection(vConn);
            }

            inline void Create()
            {
                IClassFactory2* pIUnknown   = NULL;
                IUnknown        *pOuter     = NULL;
                HRESULT         hr;
                hr = CoGetClassObject( ADOS::CLSID_ADOCOMMAND_21,
                                      CLSCTX_INPROC_SERVER,
                                      NULL,
                                      IID_IClassFactory2,
                                      (void**)&pIUnknown );

                if( !FAILED( hr ) )
                {
                    pIUnknown->AddRef();
                    ADOCommand* pCommand;

                    hr = pIUnknown->CreateInstanceLic(  pOuter,
                                                        NULL,
                                                        ADOS::IID_ADOCOMMAND_21,
                                                        ADOS::GetKeyStr(),
                                                        (void**) &pCommand);

                    if( !FAILED( hr ) )
                    {
                        pInterface = pCommand;
                        pInterface->AddRef();
                    }
                    pIUnknown->Release();
                }
            }

            inline sal_Int32 get_State() const
            {
                sal_Int32 nRet;
                pInterface->get_State(&nRet);
                return nRet;
            }

            inline ::rtl::OUString get_CommandText() const
            {
                BSTR aBSTR; pInterface->get_CommandText(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline sal_Bool put_CommandText(const ::rtl::OUString &aCon)
            {
                BSTR bstr = SysAllocString(aCon.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->put_CommandText(bstr));
                SysFreeString(bstr);
                return bErg;
            }

            inline sal_Int32 get_CommandTimeout() const
            {
                sal_Int32 nRet;
                pInterface->get_CommandTimeout(&nRet);
                return nRet;
            }

            inline void put_CommandTimeout(sal_Int32 nRet)
            {
                pInterface->put_CommandTimeout(nRet);
            }

            inline sal_Bool get_Prepared() const
            {
                VARIANT_BOOL bPrepared;
                pInterface->get_Prepared(&bPrepared);
                return bPrepared == VARIANT_TRUE;
            }

            inline sal_Bool put_Prepared(VARIANT_BOOL bPrepared) const
            {
                return SUCCEEDED(pInterface->put_Prepared(bPrepared));
            }

            inline sal_Bool Execute(OLEVariant& RecordsAffected,OLEVariant& Parameters,long Options, ADORecordset** ppiRset)
            {
                return SUCCEEDED(pInterface->Execute(&RecordsAffected,&Parameters,Options,ppiRset));
            }

            inline ADOParameter* CreateParameter(const ::rtl::OUString &_bstr,DataTypeEnum Type,ParameterDirectionEnum Direction,long nSize,const OLEVariant &Value)
            {
                ADOParameter* pPara = NULL;
                BSTR bstr = SysAllocString(_bstr.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->CreateParameter(_bstr.getLength() ? bstr : NULL,Type,Direction,nSize,Value,&pPara));
                SysFreeString(bstr);
                return bErg ? pPara : NULL;
            }

            inline ADOParameters* get_Parameters() const
            {
                ADOParameters* pPara;
                pInterface->get_Parameters(&pPara);
                return pPara;
            }

            inline sal_Bool put_CommandType( /* [in] */ CommandTypeEnum lCmdType)
            {
                return SUCCEEDED(pInterface->put_CommandType(lCmdType));
            }

            inline CommandTypeEnum get_CommandType( ) const
            {
                CommandTypeEnum eNum;
                pInterface->get_CommandType(&eNum);
                return eNum;
            }

            // gibt den Namen des Feldes zur"ueck
            inline ::rtl::OUString GetName() const
            {
                BSTR aBSTR; pInterface->get_Name(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline sal_Bool put_Name(const ::rtl::OUString& _Name)
            {
                BSTR bstr = SysAllocString(_Name.getStr());
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
                SysFreeString(bstr);
                return bErg;
            }
            inline sal_Bool Cancel()
            {
                return SUCCEEDED(pInterface->Cancel());
            }
        };
        //------------------------------------------------------------------------
        class WpADOError:public WpOLEBase<ADOError>
        {
        public:

            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse

            WpADOError(ADOError* pInt):WpOLEBase<ADOError>(pInt){}

            WpADOError(const WpADOError& rhs){operator=(rhs);}

            inline WpADOError& operator=(const WpADOError& rhs)
                {WpOLEBase<ADOError>::operator=(rhs); return *this;}

            //////////////////////////////////////////////////////////////////////

            inline ::rtl::OUString GetDescription() const
            {
                BSTR aBSTR; pInterface->get_Description(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline ::rtl::OUString GetSource() const {BSTR aBSTR; pInterface->get_Source(&aBSTR);
                                ::rtl::OUString sRetStr(aBSTR);
                                SysFreeString(aBSTR); return sRetStr;}

            inline sal_Int32 GetNumber() const {sal_Int32 nErrNr; pInterface->get_Number(&nErrNr);
                                    return nErrNr;}

            inline ::rtl::OUString GetSQLState() const
            {
                BSTR aBSTR; pInterface->get_SQLState(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline sal_Int32 GetNativeError() const
            {
                sal_Int32 nErrNr; pInterface->get_NativeError(&nErrNr);
                return nErrNr;
            }
        };



        //------------------------------------------------------------------------
        class WpADOField : public WpOLEBase<ADOField>
        {
            //  friend class WpADOFields;
        public:

            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOField(ADOField* pInt=NULL):WpOLEBase<ADOField>(pInt){}
            WpADOField(const WpADOField& rhs){operator=(rhs);}

            inline WpADOField& operator=(const WpADOField& rhs)
                {WpOLEBase<ADOField>::operator=(rhs); return *this;}
            //////////////////////////////////////////////////////////////////////

            inline ADOProperties* get_Properties()
            {
                ADOProperties* pProps = NULL;
                pInterface->get_Properties(&pProps);
                return pProps;
            }

            inline sal_Int32 GetActualSize() const
            {
                sal_Int32 nActualSize; pInterface->get_ActualSize(&nActualSize);
                return nActualSize;
            }

            inline sal_Int32 GetAttributes() const
            {
                sal_Int32 eADOSFieldAttributes; pInterface->get_Attributes(&eADOSFieldAttributes);
                return eADOSFieldAttributes;
            }

            inline sal_Int32 GetStatus() const
            {
                sal_Int32 eADOSFieldAttributes=0;
                //  pInterface->get_Status(&eADOSFieldAttributes);
                return eADOSFieldAttributes;
            }

            inline sal_Int32 GetDefinedSize() const
            {
                sal_Int32 nDefinedSize;
                pInterface->get_DefinedSize(&nDefinedSize);
                return nDefinedSize;
            }

            // gibt den Namen des Feldes zur"ueck
            inline ::rtl::OUString GetName() const
            {
                BSTR aBSTR; pInterface->get_Name(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline DataTypeEnum GetADOType() const
            {
                DataTypeEnum eType; pInterface->get_Type(&eType);
                return eType;
            }

            inline void get_Value(OLEVariant& aValVar) const
            {
                aValVar.setEmpty();
                pInterface->get_Value(&aValVar);
            }

            inline OLEVariant get_Value() const
            {
                OLEVariant aValVar;
                pInterface->get_Value(&aValVar);
                return aValVar;
            }

            inline sal_Bool PutValue(const OLEVariant& aVariant)
            {
                return (SUCCEEDED(pInterface->put_Value(aVariant)));
            }

            inline sal_Int32 GetPrecision() const
            {
                sal_uInt8 eType; pInterface->get_Precision(&eType);
                return eType;
            }

            inline sal_Int32 GetNumericScale() const
            {
                sal_uInt8 eType; pInterface->get_NumericScale(&eType);
                return eType;
            }

            inline sal_Bool AppendChunk(const OLEVariant& _Variant)
            {
                return (SUCCEEDED(pInterface->AppendChunk(_Variant)));
            }

            inline OLEVariant GetChunk(long Length) const
            {
                OLEVariant aValVar;
                pInterface->GetChunk(Length,&aValVar);
                return aValVar;
            }

            inline void GetChunk(long Length,OLEVariant &aValVar) const
            {
                pInterface->GetChunk(Length,&aValVar);
            }

            inline OLEVariant GetOriginalValue() const
            {
                OLEVariant aValVar;
                pInterface->get_OriginalValue(&aValVar);
                return aValVar;
            }

            inline void GetOriginalValue(OLEVariant &aValVar) const
            {
                pInterface->get_OriginalValue(&aValVar);
            }

            inline OLEVariant GetUnderlyingValue() const
            {
                OLEVariant aValVar;
                pInterface->get_UnderlyingValue(&aValVar);
                return aValVar;
            }

            inline void GetUnderlyingValue(OLEVariant &aValVar) const
            {
                pInterface->get_UnderlyingValue(&aValVar);
            }

            inline sal_Bool PutPrecision(sal_Int8 _prec)
            {
                return (SUCCEEDED(pInterface->put_Precision(_prec)));
            }

            inline sal_Bool PutNumericScale(sal_Int8 _prec)
            {
                return (SUCCEEDED(pInterface->put_NumericScale(_prec)));
            }

            inline void PutADOType(DataTypeEnum eType)
            {
                pInterface->put_Type(eType);
            }

            inline sal_Bool PutDefinedSize(sal_Int32 _nDefSize)
            {
                return (SUCCEEDED(pInterface->put_DefinedSize(_nDefSize)));
            }

            inline sal_Bool PutAttributes(sal_Int32 _nDefSize)
            {
                return (SUCCEEDED(pInterface->put_Attributes(_nDefSize)));
            }
        };



        //------------------------------------------------------------------------
        class WpADOProperty: public WpOLEBase<ADOProperty>
        {
            friend class WpADOProperties;

        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOProperty(ADOProperty* pInt):WpOLEBase<ADOProperty>(pInt){}
            WpADOProperty(const WpADOProperty& rhs){operator=(rhs);}
            inline WpADOProperty& operator=(const WpADOProperty& rhs)
                {WpOLEBase<ADOProperty>::operator=(rhs); return *this;}
            //////////////////////////////////////////////////////////////////////

            inline OLEVariant GetValue() const
            {
                OLEVariant aValVar;
                pInterface->get_Value(&aValVar);
                return aValVar;
            }

            inline void GetValue(OLEVariant &aValVar) const
            {
                pInterface->get_Value(&aValVar);
            }

            inline sal_Bool PutValue(const OLEVariant &aValVar)
            {
                return (SUCCEEDED(pInterface->put_Value(aValVar)));
            }

            inline ::rtl::OUString GetName() const
            {
                BSTR aBSTR; pInterface->get_Name(&aBSTR);
                ::rtl::OUString sRetStr(aBSTR);
                SysFreeString(aBSTR); return sRetStr;
            }

            inline DataTypeEnum GetADOType() const
            {
                DataTypeEnum eType; pInterface->get_Type(&eType);
                return eType;
            }

            inline sal_Int32 GetAttributes() const
            {
                sal_Int32 eADOSFieldAttributes; pInterface->get_Attributes(&eADOSFieldAttributes);
                return eADOSFieldAttributes;
            }

            inline sal_Bool PutAttributes(sal_Int32 _nDefSize)
            {
                return (SUCCEEDED(pInterface->put_Attributes(_nDefSize)));
            }
        };


        class WpADORecordset:public WpOLEBase<ADORecordset>
        {

        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADORecordset(ADORecordset* pInt=NULL):WpOLEBase<ADORecordset>(pInt){}
            WpADORecordset(const WpADORecordset& rhs){operator=(rhs);}
            inline WpADORecordset& operator=(const WpADORecordset& rhs)
            {
                WpOLEBase<ADORecordset>::operator=(rhs);
                return *this;
            }
            ~WpADORecordset()
            {
                //  if(pInterface && get_State() == adStateOpen)
                    //  Close();
            }
            //////////////////////////////////////////////////////////////////////
            inline void Create()
            {
                IClassFactory2* pIUnknown   = NULL;
                IUnknown        *pOuter     = NULL;
                HRESULT         hr;
                hr = CoGetClassObject( ADOS::CLSID_ADORECORDSET_21,
                                      CLSCTX_INPROC_SERVER,
                                      NULL,
                                      IID_IClassFactory2,
                                      (void**)&pIUnknown );

                if( !FAILED( hr ) )
                {
                    pIUnknown->AddRef();

                    ADORecordset *pRec = NULL;
                    hr = pIUnknown->CreateInstanceLic(  pOuter,
                                                        NULL,
                                                        ADOS::IID_ADORECORDSET_21,
                                                        ADOS::GetKeyStr(),
                                                        (void**) &pRec);

                    if( !FAILED( hr ) )
                    {
                        pInterface = pRec;
                        pInterface->AddRef();
                    }
                    pIUnknown->Release();
                }
            }

            inline sal_Bool Open(
                    /* [optional][in] */ VARIANT Source,
                    /* [optional][in] */ VARIANT ActiveConnection,
                    /* [defaultvalue][in] */ CursorTypeEnum CursorType,
                    /* [defaultvalue][in] */ LockTypeEnum LockType,
                    /* [defaultvalue][in] */ sal_Int32 Options)
            {
                return (SUCCEEDED(pInterface->Open(Source,ActiveConnection,CursorType,LockType,Options)));
            }


            inline LockTypeEnum GetLockType()
            {
                LockTypeEnum eType;
                pInterface->get_LockType(&eType);
                return eType;
            }

            inline void Close()
            {
                pInterface->Close();
            }

            inline sal_Bool Cancel() const
            {
                return (SUCCEEDED(pInterface->Cancel()));
            }

            inline sal_Int32 get_State( )
            {
                sal_Int32 nState = 0;
                pInterface->get_State(&nState);
                return nState;
            }

            inline sal_Bool Supports( /* [in] */ CursorOptionEnum CursorOptions)
            {
                VARIANT_BOOL bSupports;
                pInterface->Supports(CursorOptions,&bSupports);
                return (sal_Bool) bSupports;
            }

            PositionEnum get_AbsolutePosition()
            {
                PositionEnum aTemp;
                pInterface->get_AbsolutePosition(&aTemp);
                return aTemp;
            }

            inline void GetDataSource(IUnknown** pIUnknown) const
            {
                pInterface->get_DataSource(pIUnknown);
            }

            inline void PutRefDataSource(IUnknown* pIUnknown)
            {
                pInterface->putref_DataSource(pIUnknown);
            }

            inline void GetBookmark(VARIANT& var)
            {
                pInterface->get_Bookmark(&var);
            }

            inline OLEVariant GetBookmark()
            {
                OLEVariant var;
                pInterface->get_Bookmark(&var);
                return var;
            }

            CompareEnum CompareBookmarks(const OLEVariant& left,const OLEVariant& right)
            {
                CompareEnum eNum;
                pInterface->CompareBookmarks(left,right,&eNum);
                return eNum;
            }

            inline sal_Bool SetBookmark(const OLEVariant &pSafeAr)
            {
                return SUCCEEDED(pInterface->put_Bookmark(pSafeAr));
            }


            inline ADOFields* GetFields() const
            {
                ADOFields* pFields;
                pInterface->get_Fields(&pFields);
                return pFields;
            }


            inline sal_Bool Move(sal_Int32 nRows, VARIANT aBmk) {return SUCCEEDED(pInterface->Move(nRows, aBmk));}
            inline sal_Bool MoveNext() {return SUCCEEDED(pInterface->MoveNext());}
            inline sal_Bool MovePrevious() {return SUCCEEDED(pInterface->MovePrevious());}
            inline sal_Bool MoveFirst() {return SUCCEEDED(pInterface->MoveFirst());}
            inline sal_Bool MoveLast()  {return SUCCEEDED(pInterface->MoveLast());}

            inline sal_Bool IsAtBOF() const
            {
                VARIANT_BOOL bIsAtBOF;
                pInterface->get_BOF(&bIsAtBOF);
                return (sal_Bool) bIsAtBOF;
            }

            inline sal_Bool IsAtEOF() const
            {
                VARIANT_BOOL bIsAtEOF;
                pInterface->get_EOF(&bIsAtEOF);
                return (sal_Bool) bIsAtEOF;
            }

            inline sal_Bool Delete(AffectEnum eNum)
            {
                return SUCCEEDED(pInterface->Delete(eNum));
            }

            inline sal_Bool AddNew(const OLEVariant &FieldList,const OLEVariant &Values)
            {
                return SUCCEEDED(pInterface->AddNew(FieldList,Values));
            }

            inline sal_Bool Update(const OLEVariant &FieldList,const OLEVariant &Values)
            {
                return SUCCEEDED(pInterface->Update(FieldList,Values));
            }

            inline sal_Bool CancelUpdate()
            {
                return SUCCEEDED(pInterface->CancelUpdate());
            }

            inline ADOProperties* get_Properties() const
            {
                ADOProperties* pProps=NULL;
                pInterface->get_Properties(&pProps);
                return pProps;
            }

            inline sal_Bool NextRecordset(OLEVariant& RecordsAffected,ADORecordset** ppiRset)
            {
                return SUCCEEDED(pInterface->NextRecordset(&RecordsAffected,ppiRset));
            }

            inline sal_Bool get_RecordCount(sal_Int32 &_nRet) const
            {
                return SUCCEEDED(pInterface->get_RecordCount(&_nRet));
            }

            inline sal_Bool get_MaxRecords(sal_Int32 &_nRet) const
            {
                return SUCCEEDED(pInterface->get_MaxRecords(&_nRet));
            }

            inline sal_Bool put_MaxRecords(sal_Int32 _nRet)
            {
                return SUCCEEDED(pInterface->put_MaxRecords(_nRet));
            }

            inline sal_Bool get_CursorType(CursorTypeEnum &_nRet) const
            {
                return SUCCEEDED(pInterface->get_CursorType(&_nRet));
            }

            inline sal_Bool put_CursorType(CursorTypeEnum _nRet)
            {
                return SUCCEEDED(pInterface->put_CursorType(_nRet));
            }

            inline sal_Bool get_LockType(LockTypeEnum &_nRet) const
            {
                return SUCCEEDED(pInterface->get_LockType(&_nRet));
            }

            inline sal_Bool put_LockType(LockTypeEnum _nRet)
            {
                return SUCCEEDED(pInterface->put_LockType(_nRet));
            }

            inline sal_Bool get_CacheSize(sal_Int32 &_nRet) const
            {
                return SUCCEEDED(pInterface->get_CacheSize(&_nRet));
            }

            inline sal_Bool put_CacheSize(sal_Int32 _nRet)
            {
                return SUCCEEDED(pInterface->put_CacheSize(_nRet));
            }

            inline sal_Bool UpdateBatch(AffectEnum AffectRecords)
            {
                return SUCCEEDED(pInterface->UpdateBatch(AffectRecords));
            }
        };

        //------------------------------------------------------------------------
        class WpADOParameter:public WpOLEBase<ADOParameter>
        {
        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpADOParameter(ADOParameter* pInt):WpOLEBase<ADOParameter>(pInt){}
            WpADOParameter(const WpADOParameter& rhs):WpOLEBase<ADOParameter>(rhs){}
            inline WpADOParameter& operator=(const WpADOParameter& rhs)
                {WpOLEBase<ADOParameter>::operator=(rhs); return *this;}
            //////////////////////////////////////////////////////////////////////

            inline ::rtl::OUString GetName() const {BSTR aBSTR; pInterface->get_Name(&aBSTR);
                                ::rtl::OUString sRetStr(aBSTR);
                                SysFreeString(aBSTR); return sRetStr;}

            inline DataTypeEnum GetADOType() const
            {
                DataTypeEnum eType; pInterface->get_Type(&eType);
                return eType;
            }

            inline sal_Int32 GetAttributes() const
            {
                sal_Int32 eADOSFieldAttributes; pInterface->get_Attributes(&eADOSFieldAttributes);
                return eADOSFieldAttributes;
            }

            inline sal_Int32 GetPrecision() const
            {
                sal_uInt8 eType; pInterface->get_Precision(&eType);
                return eType;
            }

            inline sal_Int32 GetNumericScale() const
            {
                sal_uInt8 eType; pInterface->get_NumericScale(&eType);
                return eType;
            }

            inline ParameterDirectionEnum get_Direction() const
            {
                ParameterDirectionEnum alParmDirection;
                pInterface->get_Direction(&alParmDirection);
                return alParmDirection;
            }

            inline void GetValue(OLEVariant& aValVar) const
            {
                pInterface->get_Value(&aValVar);
            }

            inline OLEVariant GetValue() const
            {
                OLEVariant aValVar;
                pInterface->get_Value(&aValVar);
                return aValVar;
            }

            inline sal_Bool PutValue(const OLEVariant& aVariant)
            {
                return (SUCCEEDED(pInterface->put_Value(aVariant)));
            }
        };
    }
}
#endif //_CONNECTIVITY_ADO_AWRAPADO_HXX_

