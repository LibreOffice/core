/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include <sal/macros.h>
#include "ado/Awrapado.hxx"
#include "ado/Awrapadox.hxx"
#include <comphelper/types.hxx>
#include <rtl/ustrbuf.hxx>
#include "diagnose_ex.h"

namespace connectivity
{
    namespace ado
    {
        sal_Int32 nAdoObjectCounter = 0;
    }
}

using namespace connectivity::ado;

void WpADOCatalog::Create()
{
    HRESULT         hr = -1;
    _ADOCatalog* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOCATALOG_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOCATALOG_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
        setWithOutAddRef(pCommand);
}


WpADOProperties WpADOConnection::get_Properties() const
{
    ADOProperties* pProps=NULL;
    pInterface->get_Properties(&pProps);
    WpADOProperties aProps;
    aProps.setWithOutAddRef(pProps);
    return aProps;
}

rtl::OUString WpADOConnection::GetConnectionString() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_ConnectionString(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOConnection::PutConnectionString(const ::rtl::OUString &aCon) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(aCon);
    sal_Bool bErg = SUCCEEDED(pInterface->put_ConnectionString(bstr));

    return bErg;
}

sal_Int32 WpADOConnection::GetCommandTimeout() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nRet=0;
    pInterface->get_CommandTimeout(&nRet);
    return nRet;
}

void WpADOConnection::PutCommandTimeout(sal_Int32 nRet)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_CommandTimeout(nRet);
}

sal_Int32 WpADOConnection::GetConnectionTimeout() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nRet=0;
    pInterface->get_ConnectionTimeout(&nRet);
    return nRet;
}

void WpADOConnection::PutConnectionTimeout(sal_Int32 nRet)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_ConnectionTimeout(nRet);
}

sal_Bool WpADOConnection::Close( )
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->Close()));
}

sal_Bool WpADOConnection::Execute(const ::rtl::OUString& _CommandText,OLEVariant& RecordsAffected,long Options, WpADORecordset** ppiRset)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString sStr1(_CommandText);
    sal_Bool bErg = SUCCEEDED(pInterface->Execute(sStr1,&RecordsAffected,Options,(_ADORecordset**)ppiRset));
    return bErg;
}

sal_Bool WpADOConnection::BeginTrans()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nIso=0;
    return SUCCEEDED(pInterface->BeginTrans(&nIso));
}

sal_Bool WpADOConnection::CommitTrans( )
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->CommitTrans());
}

sal_Bool WpADOConnection::RollbackTrans( )
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->RollbackTrans());
}

sal_Bool WpADOConnection::Open(const ::rtl::OUString& ConnectionString, const ::rtl::OUString& UserID,const ::rtl::OUString& Password,long Options)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString sStr1(ConnectionString);
    OLEString sStr2(UserID);
    OLEString sStr3(Password);
    sal_Bool bErg = SUCCEEDED(pInterface->Open(sStr1,sStr2,sStr3,Options));
    return bErg;
}

sal_Bool WpADOConnection::GetErrors(ADOErrors** pErrors)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->get_Errors(pErrors));
}

::rtl::OUString WpADOConnection::GetDefaultDatabase() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR; pInterface->get_DefaultDatabase(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOConnection::PutDefaultDatabase(const ::rtl::OUString& _bstr)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_bstr);
    sal_Bool bErg = SUCCEEDED(pInterface->put_DefaultDatabase(bstr));

    return bErg;
}

IsolationLevelEnum WpADOConnection::get_IsolationLevel() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    IsolationLevelEnum eNum=adXactUnspecified;
    pInterface->get_IsolationLevel(&eNum);
    return eNum;
}

sal_Bool WpADOConnection::put_IsolationLevel(const IsolationLevelEnum& eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_IsolationLevel(eNum));
}

sal_Int32 WpADOConnection::get_Attributes() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nRet=0;
    pInterface->get_Attributes(&nRet);
    return nRet;
}

sal_Bool WpADOConnection::put_Attributes(sal_Int32 nRet)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_Attributes(nRet));
}

CursorLocationEnum WpADOConnection::get_CursorLocation() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    CursorLocationEnum eNum=adUseNone;
    pInterface->get_CursorLocation(&eNum);
    return eNum;
}

sal_Bool WpADOConnection::put_CursorLocation(const CursorLocationEnum &eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_CursorLocation(eNum));
}

ConnectModeEnum WpADOConnection::get_Mode() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ConnectModeEnum eNum=adModeUnknown;
    pInterface->get_Mode(&eNum);
    return eNum;
}

sal_Bool WpADOConnection::put_Mode(const ConnectModeEnum &eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_Mode(eNum));
}

::rtl::OUString WpADOConnection::get_Provider() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR; pInterface->get_Provider(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOConnection::put_Provider(const ::rtl::OUString& _bstr)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_bstr);
    return SUCCEEDED(pInterface->put_Provider(bstr));
}

sal_Int32 WpADOConnection::get_State() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nRet=0;
    pInterface->get_State(&nRet);
    return nRet;
}

sal_Bool WpADOConnection::OpenSchema(SchemaEnum eNum,OLEVariant& Restrictions,OLEVariant& SchemaID,ADORecordset**pprset)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->OpenSchema(eNum,Restrictions,SchemaID,pprset));
}

::rtl::OUString WpADOConnection::get_Version() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Version(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOCommand::putref_ActiveConnection( WpADOConnection *pCon)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    if(pCon)
        return SUCCEEDED(pInterface->putref_ActiveConnection(pCon->pInterface));
    else
        return SUCCEEDED(pInterface->putref_ActiveConnection(NULL));
}

void WpADOCommand::put_ActiveConnection(/* [in] */ const OLEVariant& vConn)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_ActiveConnection(vConn);
}

void WpADOCommand::Create()
{
    IClassFactory2* pInterface2 = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr;
    hr = CoGetClassObject( ADOS::CLSID_ADOCOMMAND_21,
                          CLSCTX_INPROC_SERVER,
                          NULL,
                          IID_IClassFactory2,
                          (void**)&pInterface2 );

    if( !FAILED( hr ) )
    {
        ADOCommand* pCommand=NULL;

        hr = pInterface2->CreateInstanceLic(  pOuter,
                                            NULL,
                                            ADOS::IID_ADOCOMMAND_21,
                                            ADOS::GetKeyStr(),
                                            (void**) &pCommand);

        if( !FAILED( hr ) )
        {
            operator=(pCommand);
            pCommand->Release();
        }

        pInterface2->Release();
    }
}

sal_Int32 WpADOCommand::get_State() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nRet=0;
    pInterface->get_State(&nRet);
    return nRet;
}

::rtl::OUString WpADOCommand::get_CommandText() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_CommandText(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOCommand::put_CommandText(const ::rtl::OUString &aCon)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(aCon);
    sal_Bool bErg = SUCCEEDED(pInterface->put_CommandText(bstr));

    return bErg;
}

sal_Int32 WpADOCommand::get_CommandTimeout() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nRet=0;
    pInterface->get_CommandTimeout(&nRet);
    return nRet;
}

void WpADOCommand::put_CommandTimeout(sal_Int32 nRet)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_CommandTimeout(nRet);
}

sal_Bool WpADOCommand::get_Prepared() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL bPrepared = VARIANT_FALSE;
    pInterface->get_Prepared(&bPrepared);
    return bPrepared == VARIANT_TRUE;
}

sal_Bool WpADOCommand::put_Prepared(VARIANT_BOOL bPrepared) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_Prepared(bPrepared));
}

sal_Bool WpADOCommand::Execute(OLEVariant& RecordsAffected,OLEVariant& Parameters,long Options, ADORecordset** ppiRset)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->Execute(&RecordsAffected,&Parameters,Options,ppiRset));
}

ADOParameter* WpADOCommand::CreateParameter(const ::rtl::OUString &_bstr,DataTypeEnum Type,ParameterDirectionEnum Direction,long nSize,const OLEVariant &Value)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOParameter* pPara = NULL;
    OLEString bstr(_bstr);
    sal_Bool bErg = SUCCEEDED(pInterface->CreateParameter(bstr,Type,Direction,nSize,Value,&pPara));

    return bErg ? pPara : NULL;
}

ADOParameters* WpADOCommand::get_Parameters() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOParameters* pPara=NULL;
    pInterface->get_Parameters(&pPara);
    return pPara;
}

sal_Bool WpADOCommand::put_CommandType( /* [in] */ CommandTypeEnum lCmdType)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_CommandType(lCmdType));
}

CommandTypeEnum WpADOCommand::get_CommandType( ) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    CommandTypeEnum eNum=adCmdUnspecified;
    pInterface->get_CommandType(&eNum);
    return eNum;
}

// returns the name of the field
::rtl::OUString WpADOCommand::GetName() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOCommand::put_Name(const ::rtl::OUString& _Name)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_Name);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

    return bErg;
}
sal_Bool WpADOCommand::Cancel()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->Cancel());
}

::rtl::OUString WpADOError::GetDescription() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Description(&aBSTR);
    return aBSTR;
}

 ::rtl::OUString WpADOError::GetSource() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Source(&aBSTR);
    return aBSTR;
}

 sal_Int32 WpADOError::GetNumber() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nErrNr=0;
    pInterface->get_Number(&nErrNr);
    return nErrNr;
}

 ::rtl::OUString WpADOError::GetSQLState() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_SQLState(&aBSTR);
    return aBSTR;
}

 sal_Int32 WpADOError::GetNativeError() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nErrNr=0;
    pInterface->get_NativeError(&nErrNr);
    return nErrNr;
}
WpADOProperties WpADOField::get_Properties()
{
     OSL_ENSURE(pInterface,"Interface is null!");
    ADOProperties* pProps = NULL;
    pInterface->get_Properties(&pProps);
    WpADOProperties aProps;

    aProps.setWithOutAddRef(pProps);
    return aProps;
}

 sal_Int32 WpADOField::GetActualSize() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    ADO_LONGPTR nActualSize=0;
    pInterface->get_ActualSize(&nActualSize);
    return nActualSize;
}

 sal_Int32 WpADOField::GetAttributes() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 eADOSFieldAttributes=0;
    pInterface->get_Attributes(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

sal_Int32 WpADOField::GetStatus() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 eADOSFieldAttributes=0;
    //  pInterface->get_Status(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

sal_Int32 WpADOField::GetDefinedSize() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADO_LONGPTR nDefinedSize=0;
    pInterface->get_DefinedSize(&nDefinedSize);
    return nDefinedSize;
}

// returns the name of the field
::rtl::OUString WpADOField::GetName() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

 DataTypeEnum WpADOField::GetADOType() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    DataTypeEnum eType=adEmpty;
    pInterface->get_Type(&eType);
    return eType;
}

 void WpADOField::get_Value(OLEVariant& aValVar) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    aValVar.setEmpty();
    sal_Bool bOk = SUCCEEDED(pInterface->get_Value(&aValVar));
    (void)bOk;
}

 OLEVariant WpADOField::get_Value() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEVariant aValVar;
    pInterface->get_Value(&aValVar);
    return aValVar;
}

 sal_Bool WpADOField::PutValue(const OLEVariant& aVariant)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Value(aVariant)));
}

sal_Int32 WpADOField::GetPrecision() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_uInt8 eType=0;
    pInterface->get_Precision(&eType);
    return eType;
}

 sal_Int32 WpADOField::GetNumericScale() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_uInt8 eType=0;
    pInterface->get_NumericScale(&eType);
    return eType;
}

 sal_Bool WpADOField::AppendChunk(const OLEVariant& _Variant)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->AppendChunk(_Variant)));
}

OLEVariant WpADOField::GetChunk(long Length) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEVariant aValVar;
    pInterface->GetChunk(Length,&aValVar);
    return aValVar;
}

void WpADOField::GetChunk(long Length,OLEVariant &aValVar) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->GetChunk(Length,&aValVar);
}

OLEVariant WpADOField::GetOriginalValue() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEVariant aValVar;
    pInterface->get_OriginalValue(&aValVar);
    return aValVar;
}

void WpADOField::GetOriginalValue(OLEVariant &aValVar) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->get_OriginalValue(&aValVar);
}

OLEVariant WpADOField::GetUnderlyingValue() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEVariant aValVar;
    pInterface->get_UnderlyingValue(&aValVar);
    return aValVar;
}

 void WpADOField::GetUnderlyingValue(OLEVariant &aValVar) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->get_UnderlyingValue(&aValVar);
}

 sal_Bool WpADOField::PutPrecision(sal_Int8 _prec)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Precision(_prec)));
}

 sal_Bool WpADOField::PutNumericScale(sal_Int8 _prec)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_NumericScale(_prec)));
}

 void WpADOField::PutADOType(DataTypeEnum eType)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Type(eType);
}

 sal_Bool WpADOField::PutDefinedSize(sal_Int32 _nDefSize)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_DefinedSize(_nDefSize)));
}

 sal_Bool WpADOField::PutAttributes(sal_Int32 _nDefSize)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Attributes(_nDefSize)));
}

OLEVariant WpADOProperty::GetValue() const
{
    OLEVariant aValVar;
    if(pInterface)
        pInterface->get_Value(&aValVar);
    return aValVar;
}

void WpADOProperty::GetValue(OLEVariant &aValVar) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    if(pInterface)
        pInterface->get_Value(&aValVar);
}

sal_Bool WpADOProperty::PutValue(const OLEVariant &aValVar)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Value(aValVar)));
}

 ::rtl::OUString WpADOProperty::GetName() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

 DataTypeEnum WpADOProperty::GetADOType() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    DataTypeEnum eType=adEmpty;
    pInterface->get_Type(&eType);
    return eType;
}

 sal_Int32 WpADOProperty::GetAttributes() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 eADOSFieldAttributes=0;
    pInterface->get_Attributes(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

 sal_Bool WpADOProperty::PutAttributes(sal_Int32 _nDefSize)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Attributes(_nDefSize)));
}
 void WpADORecordset::Create()
{
    IClassFactory2* pInterface2 = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr;
    hr = CoGetClassObject( ADOS::CLSID_ADORECORDSET_21,
                          CLSCTX_INPROC_SERVER,
                          NULL,
                          IID_IClassFactory2,
                          (void**)&pInterface2 );

    if( !FAILED( hr ) )
    {
        ADORecordset *pRec = NULL;
        hr = pInterface2->CreateInstanceLic(  pOuter,
                                            NULL,
                                            ADOS::IID_ADORECORDSET_21,
                                            ADOS::GetKeyStr(),
                                            (void**) &pRec);

        if( !FAILED( hr ) )
        {
            operator=(pRec);
            pRec->Release();
        }

        pInterface2->Release();
    }
}

 sal_Bool WpADORecordset::Open(
        /* [optional][in] */ VARIANT Source,
        /* [optional][in] */ VARIANT ActiveConnection,
        /* [defaultvalue][in] */ CursorTypeEnum CursorType,
        /* [defaultvalue][in] */ LockTypeEnum LockType,
        /* [defaultvalue][in] */ sal_Int32 Options)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->Open(Source,ActiveConnection,CursorType,LockType,Options)));
}


LockTypeEnum WpADORecordset::GetLockType()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    LockTypeEnum eType=adLockUnspecified;
    pInterface->get_LockType(&eType);
    return eType;
}

void WpADORecordset::Close()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->Close();
}

 sal_Bool WpADORecordset::Cancel() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->Cancel()));
}

 sal_Int32 WpADORecordset::get_State( )
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nState = 0;
    pInterface->get_State(&nState);
    return nState;
}

 sal_Bool WpADORecordset::Supports( /* [in] */ CursorOptionEnum CursorOptions)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL bSupports=VARIANT_FALSE;
    pInterface->Supports(CursorOptions,&bSupports);
    return bSupports == VARIANT_TRUE;
}

PositionEnum_Param WpADORecordset::get_AbsolutePosition()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    PositionEnum_Param aTemp=adPosUnknown;
    pInterface->get_AbsolutePosition(&aTemp);
    return aTemp;
}

 void WpADORecordset::GetDataSource(IUnknown** _pInterface) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->get_DataSource(_pInterface);
}

 void WpADORecordset::PutRefDataSource(IUnknown* _pInterface)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->putref_DataSource(_pInterface);
}

 void WpADORecordset::GetBookmark(VARIANT& var)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->get_Bookmark(&var);
}

 OLEVariant WpADORecordset::GetBookmark()
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEVariant var;
    pInterface->get_Bookmark(&var);
    return var;
}

CompareEnum WpADORecordset::CompareBookmarks(const OLEVariant& left,const OLEVariant& right)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    CompareEnum eNum=adCompareNotComparable;
    pInterface->CompareBookmarks(left,right,&eNum);
    return eNum;
}

 sal_Bool WpADORecordset::SetBookmark(const OLEVariant &pSafeAr)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_Bookmark(pSafeAr));
}


WpADOFields WpADORecordset::GetFields() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    ADOFields* pFields=NULL;
    pInterface->get_Fields(&pFields);
    WpADOFields aFields;
    aFields.setWithOutAddRef(pFields);
    return aFields;
}


 sal_Bool WpADORecordset::Move(sal_Int32 nRows, VARIANT aBmk)   {return pInterface && SUCCEEDED(pInterface->Move(nRows, aBmk));}
 sal_Bool WpADORecordset::MoveNext() {return pInterface && SUCCEEDED(pInterface->MoveNext());}
 sal_Bool WpADORecordset::MovePrevious() {return pInterface && SUCCEEDED(pInterface->MovePrevious());}
 sal_Bool WpADORecordset::MoveFirst() {return pInterface && SUCCEEDED(pInterface->MoveFirst());}
 sal_Bool WpADORecordset::MoveLast()    {return pInterface && SUCCEEDED(pInterface->MoveLast());}

 sal_Bool WpADORecordset::IsAtBOF() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL bIsAtBOF=VARIANT_FALSE;
    pInterface->get_BOF(&bIsAtBOF);
    return bIsAtBOF == VARIANT_TRUE;
}

 sal_Bool WpADORecordset::IsAtEOF() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL bIsAtEOF=VARIANT_FALSE;
    pInterface->get_EOF(&bIsAtEOF);
    return bIsAtEOF == VARIANT_TRUE;
}

 sal_Bool WpADORecordset::Delete(AffectEnum eNum)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->Delete(eNum));
}

 sal_Bool WpADORecordset::AddNew(const OLEVariant &FieldList,const OLEVariant &Values)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->AddNew(FieldList,Values));
}

 sal_Bool WpADORecordset::Update(const OLEVariant &FieldList,const OLEVariant &Values)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->Update(FieldList,Values));
}

 sal_Bool WpADORecordset::CancelUpdate()
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->CancelUpdate());
}

WpADOProperties WpADORecordset::get_Properties() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    ADOProperties* pProps=NULL;
    pInterface->get_Properties(&pProps);
    WpADOProperties aProps;
    aProps.setWithOutAddRef(pProps);
    return aProps;
}

 sal_Bool WpADORecordset::NextRecordset(OLEVariant& RecordsAffected,ADORecordset** ppiRset)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->NextRecordset(&RecordsAffected,ppiRset));
}

 sal_Bool WpADORecordset::get_RecordCount(ADO_LONGPTR &_nRet) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->get_RecordCount(&_nRet));
}

 sal_Bool WpADORecordset::get_MaxRecords(ADO_LONGPTR &_nRet) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->get_MaxRecords(&_nRet));
}

 sal_Bool WpADORecordset::put_MaxRecords(ADO_LONGPTR _nRet)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_MaxRecords(_nRet));
}

 sal_Bool WpADORecordset::get_CursorType(CursorTypeEnum &_nRet) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->get_CursorType(&_nRet));
}

 sal_Bool WpADORecordset::put_CursorType(CursorTypeEnum _nRet)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_CursorType(_nRet));
}

 sal_Bool WpADORecordset::get_LockType(LockTypeEnum &_nRet) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->get_LockType(&_nRet));
}

 sal_Bool WpADORecordset::put_LockType(LockTypeEnum _nRet)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_LockType(_nRet));
}

 sal_Bool WpADORecordset::get_CacheSize(sal_Int32 &_nRet) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->get_CacheSize(&_nRet));
}

 sal_Bool WpADORecordset::put_CacheSize(sal_Int32 _nRet)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_CacheSize(_nRet));
}

 sal_Bool WpADORecordset::UpdateBatch(AffectEnum AffectRecords)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->UpdateBatch(AffectRecords));
}

 ::rtl::OUString WpADOParameter::GetName() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

 DataTypeEnum WpADOParameter::GetADOType() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    DataTypeEnum eType=adEmpty;
    pInterface->get_Type(&eType);
    return eType;
}

void WpADOParameter::put_Type(const DataTypeEnum& _eType)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Type(_eType);
}

 sal_Int32 WpADOParameter::GetAttributes() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 eADOSFieldAttributes=0;
    pInterface->get_Attributes(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

 sal_Int32 WpADOParameter::GetPrecision() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_uInt8 eType=0;
    pInterface->get_Precision(&eType);
    return eType;
}

 sal_Int32 WpADOParameter::GetNumericScale() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    sal_uInt8 eType=0;
    pInterface->get_NumericScale(&eType);
    return eType;
}

 ParameterDirectionEnum WpADOParameter::get_Direction() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    ParameterDirectionEnum alParmDirection=adParamUnknown;
    pInterface->get_Direction(&alParmDirection);
    return alParmDirection;
}

 void WpADOParameter::GetValue(OLEVariant& aValVar) const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->get_Value(&aValVar);
}

 OLEVariant WpADOParameter::GetValue() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEVariant aValVar;
    pInterface->get_Value(&aValVar);
    return aValVar;
}

sal_Bool WpADOParameter::PutValue(const OLEVariant& aVariant)
{
     OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Value(aVariant)));
}
sal_Bool WpADOParameter::AppendChunk(const OLEVariant& aVariant)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->AppendChunk(aVariant)));
}
sal_Bool WpADOParameter::put_Size(const sal_Int32& _nSize)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return (SUCCEEDED(pInterface->put_Size(_nSize)));
}

 ::rtl::OUString WpADOColumn::get_Name() const
{
     OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

::rtl::OUString WpADOColumn::get_RelatedColumn() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_RelatedColumn(&aBSTR);
    return aBSTR;
}

void WpADOColumn::put_Name(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
    (void)bErg;
}
void WpADOColumn::put_RelatedColumn(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_RelatedColumn(bstr));
    (void)bErg;
}

DataTypeEnum WpADOColumn::get_Type() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    DataTypeEnum eNum = adVarChar;
    pInterface->get_Type(&eNum);
    return eNum;
}

void WpADOColumn::put_Type(const DataTypeEnum& _eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Type(_eNum);
}

sal_Int32 WpADOColumn::get_Precision() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nPrec=0;
    pInterface->get_Precision(&nPrec);
    return nPrec;
}

void WpADOColumn::put_Precision(sal_Int32 _nPre)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Precision(_nPre);
}

sal_Int32 WpADOColumn::get_DefinedSize() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Int32 nPrec=0;
    pInterface->get_DefinedSize(&nPrec);
    return nPrec;
}
sal_uInt8 WpADOColumn::get_NumericScale() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_uInt8 nPrec=0;
    pInterface->get_NumericScale(&nPrec);
    return nPrec;
}

void WpADOColumn::put_NumericScale(sal_Int8 _nScale)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_NumericScale(_nScale);
}

SortOrderEnum WpADOColumn::get_SortOrder() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    SortOrderEnum nPrec=adSortAscending;
    pInterface->get_SortOrder(&nPrec);
    return nPrec;
}

void WpADOColumn::put_SortOrder(SortOrderEnum _nScale)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_SortOrder(_nScale);
}

ColumnAttributesEnum WpADOColumn::get_Attributes() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ColumnAttributesEnum eNum=adColNullable;
    pInterface->get_Attributes(&eNum);
    return eNum;
}

sal_Bool WpADOColumn::put_Attributes(const ColumnAttributesEnum& _eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    return SUCCEEDED(pInterface->put_Attributes(_eNum));
}

WpADOProperties WpADOColumn::get_Properties() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOProperties* pProps = NULL;
    pInterface->get_Properties(&pProps);
    WpADOProperties aProps;

    aProps.setWithOutAddRef(pProps);
    return aProps;
}

::rtl::OUString WpADOKey::get_Name() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOKey::put_Name(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
    (void)bErg;
}

KeyTypeEnum WpADOKey::get_Type() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    KeyTypeEnum eNum=adKeyPrimary;
    pInterface->get_Type(&eNum);
    return eNum;
}

void WpADOKey::put_Type(const KeyTypeEnum& _eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Type(_eNum);
}

::rtl::OUString WpADOKey::get_RelatedTable() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_RelatedTable(&aBSTR);
    return aBSTR;
}

void WpADOKey::put_RelatedTable(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_RelatedTable(bstr));
    (void)bErg;
}

RuleEnum WpADOKey::get_DeleteRule() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    RuleEnum eNum = adRINone;
    pInterface->get_DeleteRule(&eNum);
    return eNum;
}

void WpADOKey::put_DeleteRule(const RuleEnum& _eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_DeleteRule(_eNum);
}

RuleEnum WpADOKey::get_UpdateRule() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    RuleEnum eNum = adRINone;
    pInterface->get_UpdateRule(&eNum);
    return eNum;
}

void WpADOKey::put_UpdateRule(const RuleEnum& _eNum)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_UpdateRule(_eNum);
}

WpADOColumns WpADOKey::get_Columns() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOColumns* pCols = NULL;
    pInterface->get_Columns(&pCols);
    WpADOColumns aCols;
    aCols.setWithOutAddRef(pCols);
    return aCols;
}

::rtl::OUString WpADOIndex::get_Name() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOIndex::put_Name(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
    (void)bErg;
}

sal_Bool WpADOIndex::get_Clustered() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL eNum = VARIANT_FALSE;
    pInterface->get_Clustered(&eNum);
    return eNum == VARIANT_TRUE;
}

void WpADOIndex::put_Clustered(sal_Bool _b)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Clustered(_b ? VARIANT_TRUE : VARIANT_FALSE);
}

sal_Bool WpADOIndex::get_Unique() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL eNum = VARIANT_FALSE;
    pInterface->get_Unique(&eNum);
    return eNum == VARIANT_TRUE;
}

void WpADOIndex::put_Unique(sal_Bool _b)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_Unique(_b ? VARIANT_TRUE : VARIANT_FALSE);
}

sal_Bool WpADOIndex::get_PrimaryKey() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    VARIANT_BOOL eNum = VARIANT_FALSE;
    pInterface->get_PrimaryKey(&eNum);
    return eNum == VARIANT_TRUE;
}

void WpADOIndex::put_PrimaryKey(sal_Bool _b)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->put_PrimaryKey(_b ? VARIANT_TRUE : VARIANT_FALSE);
}

WpADOColumns WpADOIndex::get_Columns() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOColumns* pCols = NULL;
    pInterface->get_Columns(&pCols);
    WpADOColumns aCols;
    aCols.setWithOutAddRef(pCols);
    return aCols;
}

void WpADOCatalog::putref_ActiveConnection(IDispatch* pCon)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->putref_ActiveConnection(pCon);
}

WpADOTables WpADOCatalog::get_Tables()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOTables* pRet = NULL;
    pInterface->get_Tables(&pRet);
    WpADOTables aRet;
    aRet.setWithOutAddRef(pRet);
    return aRet;
}

WpADOViews WpADOCatalog::get_Views()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOViews* pRet = NULL;
    pInterface->get_Views(&pRet);
    WpADOViews aRet;
    aRet.setWithOutAddRef(pRet);
    return aRet;
}

WpADOGroups WpADOCatalog::get_Groups()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOGroups* pRet = NULL;
    pInterface->get_Groups(&pRet);
    WpADOGroups aRet;
    aRet.setWithOutAddRef(pRet);
    return aRet;
}

WpADOUsers WpADOCatalog::get_Users()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOUsers* pRet = NULL;
    pInterface->get_Users(&pRet);
    WpADOUsers aRet;
    aRet.setWithOutAddRef(pRet);
    return aRet;
}

ADOProcedures* WpADOCatalog::get_Procedures()
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOProcedures* pRet = NULL;
    pInterface->get_Procedures(&pRet);
    return pRet;
}

::rtl::OUString WpADOTable::get_Name() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOTable::put_Name(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
    (void)bErg;
}

::rtl::OUString WpADOTable::get_Type() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Type(&aBSTR);
    return aBSTR;
}

WpADOColumns WpADOTable::get_Columns() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOColumns* pCols = NULL;
    pInterface->get_Columns(&pCols);
    WpADOColumns aCols;
    aCols.setWithOutAddRef(pCols);
    return aCols;
}

WpADOIndexes WpADOTable::get_Indexes() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOIndexes* pCols = NULL;
    pInterface->get_Indexes(&pCols);
    WpADOIndexes aRet;
    aRet.setWithOutAddRef(pCols);
    return aRet;
}

WpADOKeys WpADOTable::get_Keys() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOKeys* pCols = NULL;
    pInterface->get_Keys(&pCols);
    WpADOKeys aRet;
    aRet.setWithOutAddRef(pCols);
    return aRet;
}

WpADOCatalog WpADOTable::get_ParentCatalog() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOCatalog* pCat = NULL;
    pInterface->get_ParentCatalog(&pCat);
    WpADOCatalog aRet;
    aRet.setWithOutAddRef(pCat);
    return aRet;
}

WpADOProperties WpADOTable::get_Properties() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    ADOProperties* pProps = NULL;
    pInterface->get_Properties(&pProps);
    WpADOProperties aProps;
    aProps.setWithOutAddRef(pProps);
    return aProps;
}

::rtl::OUString WpADOView::get_Name() const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOView::get_Command(OLEVariant& _rVar) const
{
    OSL_ENSURE(pInterface,"Interface is null!");
    pInterface->get_Command(&_rVar);
}

void WpADOView::put_Command(OLEVariant& _rVar)
{
    pInterface->put_Command(_rVar);
}

::rtl::OUString WpADOGroup::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOGroup::put_Name(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
    (void)bErg;
}

RightsEnum WpADOGroup::GetPermissions(
    /* [in] */ const OLEVariant& Name,
    /* [in] */ ObjectTypeEnum ObjectType)
{
    RightsEnum Rights=adRightNone;
    OLEVariant ObjectTypeId;
    ObjectTypeId.setNoArg();
    pInterface->GetPermissions(Name,ObjectType,ObjectTypeId,&Rights);
    return Rights;
}

sal_Bool WpADOGroup::SetPermissions(
    /* [in] */ const OLEVariant& Name,
    /* [in] */ ObjectTypeEnum ObjectType,
    /* [in] */ ActionEnum Action,
    /* [in] */ RightsEnum Rights)
{
    OLEVariant ObjectTypeId;
    ObjectTypeId.setNoArg();
    return SUCCEEDED(pInterface->SetPermissions(Name,ObjectType,Action,Rights,adInheritNone,ObjectTypeId));
}

WpADOUsers WpADOGroup::get_Users( )
{
    ADOUsers* pRet = NULL;
    pInterface->get_Users( &pRet);
    WpADOUsers aRet;
    aRet.setWithOutAddRef(pRet);
    return aRet;
}

::rtl::OUString WpADOUser::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOUser::put_Name(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
    (void)bErg;
}

sal_Bool WpADOUser::ChangePassword(const ::rtl::OUString& _rPwd,const ::rtl::OUString& _rNewPwd)
{
    OLEString sStr1(_rPwd);
    OLEString sStr2(_rNewPwd);
    sal_Bool bErg = SUCCEEDED(pInterface->ChangePassword(sStr1,sStr2));
    return bErg;
}

WpADOGroups WpADOUser::get_Groups()
{
    ADOGroups* pRet = NULL;
    pInterface->get_Groups(&pRet);
    WpADOGroups aRet;
    aRet.setWithOutAddRef(pRet);
    return aRet;
}

RightsEnum WpADOUser::GetPermissions(
    /* [in] */ const OLEVariant& Name,
    /* [in] */ ObjectTypeEnum ObjectType)
{
    RightsEnum Rights=adRightNone;
    OLEVariant ObjectTypeId;
    ObjectTypeId.setNoArg();
    pInterface->GetPermissions(Name,ObjectType,ObjectTypeId,&Rights);
    return Rights;
}

sal_Bool WpADOUser::SetPermissions(
    /* [in] */ const OLEVariant& Name,
    /* [in] */ ObjectTypeEnum ObjectType,
    /* [in] */ ActionEnum Action,
    /* [in] */ RightsEnum Rights)
{
    OLEVariant ObjectTypeId;
    ObjectTypeId.setNoArg();
    return SUCCEEDED(pInterface->SetPermissions(Name,ObjectType,Action,Rights,adInheritNone,ObjectTypeId));
}

WpBase::WpBase() : pIUnknown(NULL)
{
}
WpBase::WpBase(IDispatch* pInt)
    :pIUnknown(pInt)
{
    if (pIUnknown)
    {
        ULONG nCount = pIUnknown->AddRef();
        (void)nCount;
        //  OSL_ENSURE(nCount == 1,"Count is greater than 1");
    }
}

//inline
WpBase& WpBase::operator=(const WpBase& rhs)
{
    if (rhs.pIUnknown != pIUnknown)
    {
        if (pIUnknown)
            pIUnknown->Release();
        pIUnknown = rhs.pIUnknown;
        if (pIUnknown)
            pIUnknown->AddRef();
    }
    return *this;
};

WpBase& WpBase::operator=(IDispatch* rhs)
{
    if (pIUnknown != rhs)
    {
        if (pIUnknown)
            pIUnknown->Release();
        pIUnknown = rhs;
        if (pIUnknown)
            pIUnknown->AddRef();
    }
    return *this;
}

WpBase::WpBase(const WpBase& aWrapper)
{
    operator=(aWrapper);
}

WpBase::~WpBase()
{
    if (pIUnknown)
    {
        pIUnknown->Release();
        pIUnknown = NULL;
    }
}

void WpBase::clear()
{
    if (pIUnknown)
    {
        pIUnknown->Release();
        pIUnknown = NULL;
    }
}


sal_Bool WpBase::IsValid() const
{
    return pIUnknown != NULL;
}
WpBase::operator IDispatch*()
{
    return pIUnknown;
}

ADORecordset* WpADOConnection::getExportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[6];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schema.getLength() && schema.toChar() != '%')
        varCriteria[nPos].setString(schema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    varCriteria[nPos].setString(table);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaForeignKeys,vsa,vtEmpty,&pRecordset);
    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getImportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[6];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schema.getLength() && schema.toChar() != '%')
        varCriteria[nPos].setString(schema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    varCriteria[nPos].setString(table);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaForeignKeys,vsa,vtEmpty,&pRecordset);

    return pRecordset;

}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getPrimaryKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[3];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schema.getLength() && schema.toChar() != '%')
        varCriteria[nPos].setString(schema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    varCriteria[nPos].setString(table);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME


    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaPrimaryKeys,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getIndexInfo(
    const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    sal_Bool /*unique*/, sal_Bool /*approximate*/ )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[5];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schema.getLength() && schema.toChar() != '%')
        varCriteria[nPos].setString(schema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// INDEX_NAME

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TYPE

    varCriteria[nPos].setString(table);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaIndexes,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getTablePrivileges( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& tableNamePattern )
{
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[5];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schemaPattern.getLength() && schemaPattern.toChar() != '%')
        varCriteria[nPos].setString(schemaPattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    if(tableNamePattern.toChar() != '%')
        varCriteria[nPos].setString(tableNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// GRANTOR
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// GRANTEE

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaTablePrivileges,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getCrossReference( const ::com::sun::star::uno::Any& primaryCatalog,
                                                  const ::rtl::OUString& primarySchema,
                                                  const ::rtl::OUString& primaryTable,
                                                  const ::com::sun::star::uno::Any& foreignCatalog,
                                                  const ::rtl::OUString& foreignSchema,
                                                  const ::rtl::OUString& foreignTable)
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[6];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(primaryCatalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(primaryCatalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(primarySchema.getLength() && primarySchema.toChar() != '%')
        varCriteria[nPos].setString(primarySchema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    varCriteria[nPos].setString(primaryTable);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    if(foreignCatalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(foreignCatalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(foreignSchema.getLength() && foreignSchema.toChar() != '%')
        varCriteria[nPos].setString(foreignSchema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    varCriteria[nPos].setString(foreignTable);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaForeignKeys,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getProcedures( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& procedureNamePattern )
{
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[3];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schemaPattern.getLength() && schemaPattern.toChar() != '%')
        varCriteria[nPos].setString(schemaPattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    if(procedureNamePattern.toChar() != '%')
        varCriteria[nPos].setString(procedureNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaProcedures,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getProcedureColumns( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& procedureNamePattern,
                                                  const ::rtl::OUString& columnNamePattern )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[4];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schemaPattern.getLength() && schemaPattern.toChar() != '%')
        varCriteria[nPos].setString(schemaPattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    if(procedureNamePattern.toChar() != '%')
        varCriteria[nPos].setString(procedureNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    if(columnNamePattern.toChar() != '%')
        varCriteria[nPos].setString(columnNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// COLUMN_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaProcedureParameters,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getTables( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& tableNamePattern,
                                                  const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    OLEVariant varCriteria[4];

    sal_Int32 nPos=0;
    ::rtl::OUString sCatalog;
    if ( catalog.hasValue() && (catalog >>= sCatalog) )
        varCriteria[nPos].setString(sCatalog);

    ++nPos;
    if(schemaPattern.getLength() && schemaPattern.toChar() != '%')
        varCriteria[nPos].setString(schemaPattern);

    ++nPos;
    if(tableNamePattern.toChar() != '%')
        varCriteria[nPos].setString(tableNamePattern);

    ++nPos;
    ::rtl::OUStringBuffer aTypes;
    ::rtl::OUString aComma( RTL_CONSTASCII_USTRINGPARAM( "," ));
    const ::rtl::OUString* pIter = types.getConstArray();
    const ::rtl::OUString* pEnd = pIter + types.getLength();
    for( ; pIter != pEnd ; ++pIter)
    {
        if ( aTypes.getLength() )
            aTypes.append(aComma);
        aTypes.append(*pIter);
    }

    ::rtl::OUString sTypeNames = aTypes.makeStringAndClear();
    if ( sTypeNames.getLength() )
        varCriteria[nPos].setString(sTypeNames);

    // Create SafeArray Bounds and initialize the array
    const sal_Int32 nCrit = SAL_N_ELEMENTS( varCriteria );
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = nCrit;
    SAFEARRAY *psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    // Set the values for each element of the array
    for( long i = 0 ; i < nCrit && SUCCEEDED( hr );i++)
    {
        hr  = SafeArrayPutElement(psa, &i,&varCriteria[i]);
    }

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaTables,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getColumns( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schemaPattern,
                                                  const ::rtl::OUString& tableNamePattern,
                                                  const ::rtl::OUString& columnNamePattern )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[4];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schemaPattern.getLength() && schemaPattern.toChar() != '%')
        varCriteria[nPos].setString(schemaPattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    if(tableNamePattern.toChar() != '%')
        varCriteria[nPos].setString(tableNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    varCriteria[nPos].setString(columnNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// COLUMN_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaColumns,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getColumnPrivileges( const ::com::sun::star::uno::Any& catalog,
                                                  const ::rtl::OUString& schema,
                                                  const ::rtl::OUString& table,
                                                  const ::rtl::OUString& columnNamePattern )
{
    // Create elements used in the array
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa = NULL;
    OLEVariant varCriteria[4];

    // Create SafeArray Bounds and initialize the array
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = SAL_N_ELEMENTS( varCriteria );
    psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos=0;
    if(catalog.hasValue())
        varCriteria[nPos].setString(::comphelper::getString(catalog));

    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_CATALOG
    if(schema.getLength() && schema.toChar() != '%')
        varCriteria[nPos].setString(schema);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_SCHEMA

    varCriteria[nPos].setString(table);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// TABLE_NAME

    varCriteria[nPos].setString(columnNamePattern);
    hr = SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;// COLUMN_NAME

    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    ADORecordset *pRecordset = NULL;
    OpenSchema(adSchemaColumnPrivileges,vsa,vtEmpty,&pRecordset);

    return pRecordset;
}
// -----------------------------------------------------------------------------
ADORecordset* WpADOConnection::getTypeInfo(DataTypeEnum /*_eType*/)
{
    // Create elements used in the array
    OLEVariant varCriteria[2];
    const int nCrit = SAL_N_ELEMENTS( varCriteria );
    // Create SafeArray Bounds and initialize the array
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = nCrit;
    SAFEARRAY *psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    sal_Int32 nPos = 0;
    SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;
    SafeArrayPutElement(psa,&nPos,&varCriteria[nPos]);nPos++;

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    OLEVariant aEmpty;
    aEmpty.setNoArg();

    ADORecordset *pRec=NULL;
    OpenSchema(adSchemaProviderTypes,vsa,aEmpty,&pRec);

    return pRec;
}
// -----------------------------------------------------------------------------
void WpADOColumn::put_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Bool bRet = SUCCEEDED(pInterface->put_ParentCatalog(ppvObject));
    OSL_ENSURE(bRet,"Could not set ParentCatalog!");
    OSL_UNUSED(bRet);
}
// -----------------------------------------------------------------------------
void WpADOTable::putref_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject)
{
    OSL_ENSURE(pInterface,"Interface is null!");
    sal_Bool bRet = SUCCEEDED(pInterface->putref_ParentCatalog(ppvObject));
    OSL_ENSURE(bRet,"Could not set ParentCatalog!");
    OSL_UNUSED(bRet);
}
// -----------------------------------------------------------------------------
void WpBase::setIDispatch(IDispatch* _pIUnknown)
{
    pIUnknown = _pIUnknown;
}
// -----------------------------------------------------------------------------
void OTools::putValue(const WpADOProperties& _rProps,const OLEVariant &_aPosition,const OLEVariant &_aValVar)
{
    OSL_ENSURE(_rProps.IsValid(),"Properties are not valid!");
    WpADOProperty aProp(_rProps.GetItem(_aPosition));
    if ( aProp.IsValid() )
    {
        sal_Bool bRet = aProp.PutValue(_aValVar);
        OSL_ENSURE(bRet,"Could not put value!");
        OSL_UNUSED(bRet);
    }
}
// -----------------------------------------------------------------------------
OLEVariant OTools::getValue(const WpADOProperties& _rProps,const OLEVariant &_aPosition)
{
    WpADOProperty aProp(_rProps.GetItem(_aPosition));
    return aProp.GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
