/*************************************************************************
 *
 *  $RCSfile: Awrapado.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-17 07:26:59 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#include "ado/Awrapado.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADOX_HXX_
#include "ado/Awrapadox.hxx"
#endif

using namespace connectivity::ado;

ADOProperties* WpADOConnection::get_Properties() const
{
    ADOProperties* pProps=NULL;
    pInterface->get_Properties(&pProps);
    return pProps;
}

rtl::OUString WpADOConnection::GetConnectionString() const
{
    OLEString aBSTR;
    pInterface->get_ConnectionString(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOConnection::PutConnectionString(const ::rtl::OUString &aCon) const
{
    OLEString bstr(aCon);
    sal_Bool bErg = SUCCEEDED(pInterface->put_ConnectionString(bstr));

    return bErg;
}

sal_Int32 WpADOConnection::GetCommandTimeout() const
{
    sal_Int32 nRet=0;
    pInterface->get_CommandTimeout(&nRet);
    return nRet;
}

void WpADOConnection::PutCommandTimeout(sal_Int32 nRet)
{
    pInterface->put_CommandTimeout(nRet);
}

sal_Int32 WpADOConnection::GetConnectionTimeout() const
{
    sal_Int32 nRet=0;
    pInterface->get_ConnectionTimeout(&nRet);
    return nRet;
}

void WpADOConnection::PutConnectionTimeout(sal_Int32 nRet)
{
    pInterface->put_ConnectionTimeout(nRet);
}

sal_Bool WpADOConnection::Close( )
{
    return (SUCCEEDED(pInterface->Close()));
}

sal_Bool WpADOConnection::Execute(const ::rtl::OUString& _CommandText,OLEVariant& RecordsAffected,long Options, WpADORecordset** ppiRset)
{
    OLEString sStr1(_CommandText);
    sal_Bool bErg = SUCCEEDED(pInterface->Execute(sStr1,&RecordsAffected,Options,(_ADORecordset**)ppiRset));
    return bErg;
}

sal_Bool WpADOConnection::BeginTrans()
{
    sal_Int32 nIso=0;
    return SUCCEEDED(pInterface->BeginTrans(&nIso));
}

sal_Bool WpADOConnection::CommitTrans( )
{
    return SUCCEEDED(pInterface->CommitTrans());
}

sal_Bool WpADOConnection::RollbackTrans( )
{
    return SUCCEEDED(pInterface->RollbackTrans());
}

sal_Bool WpADOConnection::Open(const ::rtl::OUString& ConnectionString, const ::rtl::OUString& UserID,const ::rtl::OUString& Password,long Options)
{
    OLEString sStr1(ConnectionString);
    OLEString sStr2(UserID);
    OLEString sStr3(Password);
    sal_Bool bErg = SUCCEEDED(pInterface->Open(sStr1,sStr2,sStr3,Options));
    return bErg;
}

sal_Bool WpADOConnection::GetErrors(ADOErrors** pErrors)
{
    return SUCCEEDED(pInterface->get_Errors(pErrors));
}

::rtl::OUString WpADOConnection::GetDefaultDatabase() const
{
    OLEString aBSTR; pInterface->get_DefaultDatabase(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOConnection::PutDefaultDatabase(const ::rtl::OUString& _bstr)
{
    OLEString bstr(_bstr);
    sal_Bool bErg = SUCCEEDED(pInterface->put_DefaultDatabase(bstr));

    return bErg;
}

IsolationLevelEnum WpADOConnection::get_IsolationLevel() const
{
    IsolationLevelEnum eNum=adXactUnspecified;
    pInterface->get_IsolationLevel(&eNum);
    return eNum;
}

sal_Bool WpADOConnection::put_IsolationLevel(const IsolationLevelEnum& eNum)
{
    return SUCCEEDED(pInterface->put_IsolationLevel(eNum));
}

sal_Int32 WpADOConnection::get_Attributes() const
{
    sal_Int32 nRet=0;
    pInterface->get_Attributes(&nRet);
    return nRet;
}

sal_Bool WpADOConnection::put_Attributes(sal_Int32 nRet)
{
    return SUCCEEDED(pInterface->put_Attributes(nRet));
}

CursorLocationEnum WpADOConnection::get_CursorLocation() const
{
    CursorLocationEnum eNum=adUseNone;
    pInterface->get_CursorLocation(&eNum);
    return eNum;
}

sal_Bool WpADOConnection::put_CursorLocation(const CursorLocationEnum &eNum)
{
    return SUCCEEDED(pInterface->put_CursorLocation(eNum));
}

ConnectModeEnum WpADOConnection::get_Mode() const
{
    ConnectModeEnum eNum=adModeUnknown;
    pInterface->get_Mode(&eNum);
    return eNum;
}

sal_Bool WpADOConnection::put_Mode(const ConnectModeEnum &eNum)
{

    return SUCCEEDED(pInterface->put_Mode(eNum));
}

::rtl::OUString WpADOConnection::get_Provider() const
{
    OLEString aBSTR; pInterface->get_Provider(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOConnection::put_Provider(const ::rtl::OUString& _bstr)
{
    OLEString bstr(_bstr);
    return SUCCEEDED(pInterface->put_Provider(bstr));
}

sal_Int32 WpADOConnection::get_State() const
{
    sal_Int32 nRet=0;
    pInterface->get_State(&nRet);
    return nRet;
}

sal_Bool WpADOConnection::OpenSchema(SchemaEnum eNum,OLEVariant& Restrictions,OLEVariant& SchemaID,ADORecordset**pprset)
{
    return SUCCEEDED(pInterface->OpenSchema(eNum,Restrictions,SchemaID,pprset));
}

::rtl::OUString WpADOConnection::get_Version() const
{
    OLEString aBSTR;
    pInterface->get_Version(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOCommand::putref_ActiveConnection( WpADOConnection *pCon)
{
    return SUCCEEDED(pInterface->putref_ActiveConnection(pCon->pInterface));
}

void WpADOCommand::put_ActiveConnection(/* [in] */ const OLEVariant& vConn)
{
    pInterface->put_ActiveConnection(vConn);
}

void WpADOCommand::Create()
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
        ADOCommand* pCommand=NULL;

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

sal_Int32 WpADOCommand::get_State() const
{
    sal_Int32 nRet=0;
    pInterface->get_State(&nRet);
    return nRet;
}

::rtl::OUString WpADOCommand::get_CommandText() const
{
    OLEString aBSTR; pInterface->get_CommandText(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOCommand::put_CommandText(const ::rtl::OUString &aCon)
{
    OLEString bstr(aCon);
    sal_Bool bErg = SUCCEEDED(pInterface->put_CommandText(bstr));

    return bErg;
}

sal_Int32 WpADOCommand::get_CommandTimeout() const
{
    sal_Int32 nRet=0;
    pInterface->get_CommandTimeout(&nRet);
    return nRet;
}

void WpADOCommand::put_CommandTimeout(sal_Int32 nRet)
{
    pInterface->put_CommandTimeout(nRet);
}

sal_Bool WpADOCommand::get_Prepared() const
{
    VARIANT_BOOL bPrepared = VARIANT_FALSE;
    pInterface->get_Prepared(&bPrepared);
    return bPrepared == VARIANT_TRUE;
}

sal_Bool WpADOCommand::put_Prepared(VARIANT_BOOL bPrepared) const
{
    return SUCCEEDED(pInterface->put_Prepared(bPrepared));
}

sal_Bool WpADOCommand::Execute(OLEVariant& RecordsAffected,OLEVariant& Parameters,long Options, ADORecordset** ppiRset)
{
    return SUCCEEDED(pInterface->Execute(&RecordsAffected,&Parameters,Options,ppiRset));
}

ADOParameter* WpADOCommand::CreateParameter(const ::rtl::OUString &_bstr,DataTypeEnum Type,ParameterDirectionEnum Direction,long nSize,const OLEVariant &Value)
{
    ADOParameter* pPara = NULL;
    OLEString bstr(_bstr);
    sal_Bool bErg = SUCCEEDED(pInterface->CreateParameter(bstr,Type,Direction,nSize,Value,&pPara));

    return bErg ? pPara : NULL;
}

ADOParameters* WpADOCommand::get_Parameters() const
{
    ADOParameters* pPara=NULL;
    pInterface->get_Parameters(&pPara);
    return pPara;
}

sal_Bool WpADOCommand::put_CommandType( /* [in] */ CommandTypeEnum lCmdType)
{
    return SUCCEEDED(pInterface->put_CommandType(lCmdType));
}

CommandTypeEnum WpADOCommand::get_CommandType( ) const
{
    CommandTypeEnum eNum=adCmdUnspecified;
    pInterface->get_CommandType(&eNum);
    return eNum;
}

// gibt den Namen des Feldes zur"ueck
::rtl::OUString WpADOCommand::GetName() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

sal_Bool WpADOCommand::put_Name(const ::rtl::OUString& _Name)
{
    OLEString bstr(_Name);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

    return bErg;
}
sal_Bool WpADOCommand::Cancel()
{
    return SUCCEEDED(pInterface->Cancel());
}

::rtl::OUString WpADOError::GetDescription() const
{
    OLEString aBSTR;
    pInterface->get_Description(&aBSTR);
    return aBSTR;
}

 ::rtl::OUString WpADOError::GetSource() const
{
    OLEString aBSTR;
    pInterface->get_Source(&aBSTR);
    return aBSTR;
}

 sal_Int32 WpADOError::GetNumber() const
{
    sal_Int32 nErrNr=0;
    pInterface->get_Number(&nErrNr);
    return nErrNr;
}

 ::rtl::OUString WpADOError::GetSQLState() const
{
    OLEString aBSTR;
    pInterface->get_SQLState(&aBSTR);
    return aBSTR;
}

 sal_Int32 WpADOError::GetNativeError() const
{
    sal_Int32 nErrNr=0;
    pInterface->get_NativeError(&nErrNr);
    return nErrNr;
}
 ADOProperties* WpADOField::get_Properties()
{
    ADOProperties* pProps = NULL;
    pInterface->get_Properties(&pProps);
    return pProps;
}

 sal_Int32 WpADOField::GetActualSize() const
{
    sal_Int32 nActualSize=0;
    pInterface->get_ActualSize(&nActualSize);
    return nActualSize;
}

 sal_Int32 WpADOField::GetAttributes() const
{
    sal_Int32 eADOSFieldAttributes=0;
    pInterface->get_Attributes(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

 sal_Int32 WpADOField::GetStatus() const
{
    sal_Int32 eADOSFieldAttributes=0;
    //  pInterface->get_Status(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

 sal_Int32 WpADOField::GetDefinedSize() const
{
    sal_Int32 nDefinedSize=0;
    pInterface->get_DefinedSize(&nDefinedSize);
    return nDefinedSize;
}

// gibt den Namen des Feldes zur"ueck
 ::rtl::OUString WpADOField::GetName() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

 DataTypeEnum WpADOField::GetADOType() const
{
    DataTypeEnum eType=adEmpty;
    pInterface->get_Type(&eType);
    return eType;
}

 void WpADOField::get_Value(OLEVariant& aValVar) const
{
    aValVar.setEmpty();
    pInterface->get_Value(&aValVar);
}

 OLEVariant WpADOField::get_Value() const
{
    OLEVariant aValVar;
    pInterface->get_Value(&aValVar);
    return aValVar;
}

 sal_Bool WpADOField::PutValue(const OLEVariant& aVariant)
{
    return (SUCCEEDED(pInterface->put_Value(aVariant)));
}

 sal_Int32 WpADOField::GetPrecision() const
{
    sal_uInt8 eType=0;
    pInterface->get_Precision(&eType);
    return eType;
}

 sal_Int32 WpADOField::GetNumericScale() const
{
    sal_uInt8 eType=0;
    pInterface->get_NumericScale(&eType);
    return eType;
}

 sal_Bool WpADOField::AppendChunk(const OLEVariant& _Variant)
{
    return (SUCCEEDED(pInterface->AppendChunk(_Variant)));
}

 OLEVariant WpADOField::GetChunk(long Length) const
{
    OLEVariant aValVar;
    pInterface->GetChunk(Length,&aValVar);
    return aValVar;
}

 void WpADOField::GetChunk(long Length,OLEVariant &aValVar) const
{
    pInterface->GetChunk(Length,&aValVar);
}

 OLEVariant WpADOField::GetOriginalValue() const
{
    OLEVariant aValVar;
    pInterface->get_OriginalValue(&aValVar);
    return aValVar;
}

 void WpADOField::GetOriginalValue(OLEVariant &aValVar) const
{
    pInterface->get_OriginalValue(&aValVar);
}

 OLEVariant WpADOField::GetUnderlyingValue() const
{
    OLEVariant aValVar;
    pInterface->get_UnderlyingValue(&aValVar);
    return aValVar;
}

 void WpADOField::GetUnderlyingValue(OLEVariant &aValVar) const
{
    pInterface->get_UnderlyingValue(&aValVar);
}

 sal_Bool WpADOField::PutPrecision(sal_Int8 _prec)
{
    return (SUCCEEDED(pInterface->put_Precision(_prec)));
}

 sal_Bool WpADOField::PutNumericScale(sal_Int8 _prec)
{
    return (SUCCEEDED(pInterface->put_NumericScale(_prec)));
}

 void WpADOField::PutADOType(DataTypeEnum eType)
{
    pInterface->put_Type(eType);
}

 sal_Bool WpADOField::PutDefinedSize(sal_Int32 _nDefSize)
{
    return (SUCCEEDED(pInterface->put_DefinedSize(_nDefSize)));
}

 sal_Bool WpADOField::PutAttributes(sal_Int32 _nDefSize)
{
    return (SUCCEEDED(pInterface->put_Attributes(_nDefSize)));
}

 OLEVariant WpADOProperty::GetValue() const
{
    OLEVariant aValVar;
    pInterface->get_Value(&aValVar);
    return aValVar;
}

 void WpADOProperty::GetValue(OLEVariant &aValVar) const
{
    pInterface->get_Value(&aValVar);
}

 sal_Bool WpADOProperty::PutValue(const OLEVariant &aValVar)
{
    return (SUCCEEDED(pInterface->put_Value(aValVar)));
}

 ::rtl::OUString WpADOProperty::GetName() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

 DataTypeEnum WpADOProperty::GetADOType() const
{
    DataTypeEnum eType=adEmpty;
    pInterface->get_Type(&eType);
    return eType;
}

 sal_Int32 WpADOProperty::GetAttributes() const
{
    sal_Int32 eADOSFieldAttributes=0;
    pInterface->get_Attributes(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

 sal_Bool WpADOProperty::PutAttributes(sal_Int32 _nDefSize)
{
    return (SUCCEEDED(pInterface->put_Attributes(_nDefSize)));
}
 void WpADORecordset::Create()
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

 sal_Bool WpADORecordset::Open(
        /* [optional][in] */ VARIANT Source,
        /* [optional][in] */ VARIANT ActiveConnection,
        /* [defaultvalue][in] */ CursorTypeEnum CursorType,
        /* [defaultvalue][in] */ LockTypeEnum LockType,
        /* [defaultvalue][in] */ sal_Int32 Options)
{
    return (SUCCEEDED(pInterface->Open(Source,ActiveConnection,CursorType,LockType,Options)));
}


 LockTypeEnum WpADORecordset::GetLockType()
{
    LockTypeEnum eType=adLockUnspecified;
    pInterface->get_LockType(&eType);
    return eType;
}

 void WpADORecordset::Close()
{
    pInterface->Close();
}

 sal_Bool WpADORecordset::Cancel() const
{
    return (SUCCEEDED(pInterface->Cancel()));
}

 sal_Int32 WpADORecordset::get_State( )
{
    sal_Int32 nState = 0;
    pInterface->get_State(&nState);
    return nState;
}

 sal_Bool WpADORecordset::Supports( /* [in] */ CursorOptionEnum CursorOptions)
{
    VARIANT_BOOL bSupports=VARIANT_FALSE;
    pInterface->Supports(CursorOptions,&bSupports);
    return bSupports == VARIANT_TRUE;
}

PositionEnum WpADORecordset::get_AbsolutePosition()
{
    PositionEnum aTemp=adPosUnknown;
    pInterface->get_AbsolutePosition(&aTemp);
    return aTemp;
}

 void WpADORecordset::GetDataSource(IUnknown** pIUnknown) const
{
    pInterface->get_DataSource(pIUnknown);
}

 void WpADORecordset::PutRefDataSource(IUnknown* pIUnknown)
{
    pInterface->putref_DataSource(pIUnknown);
}

 void WpADORecordset::GetBookmark(VARIANT& var)
{
    pInterface->get_Bookmark(&var);
}

 OLEVariant WpADORecordset::GetBookmark()
{
    OLEVariant var;
    pInterface->get_Bookmark(&var);
    return var;
}

CompareEnum WpADORecordset::CompareBookmarks(const OLEVariant& left,const OLEVariant& right)
{
    CompareEnum eNum=adCompareNotComparable;
    pInterface->CompareBookmarks(left,right,&eNum);
    return eNum;
}

 sal_Bool WpADORecordset::SetBookmark(const OLEVariant &pSafeAr)
{
    return SUCCEEDED(pInterface->put_Bookmark(pSafeAr));
}


 ADOFields* WpADORecordset::GetFields() const
{
    ADOFields* pFields=NULL;
    pInterface->get_Fields(&pFields);
    return pFields;
}


 sal_Bool WpADORecordset::Move(sal_Int32 nRows, VARIANT aBmk)   {return SUCCEEDED(pInterface->Move(nRows, aBmk));}
 sal_Bool WpADORecordset::MoveNext() {return SUCCEEDED(pInterface->MoveNext());}
 sal_Bool WpADORecordset::MovePrevious() {return SUCCEEDED(pInterface->MovePrevious());}
 sal_Bool WpADORecordset::MoveFirst() {return SUCCEEDED(pInterface->MoveFirst());}
 sal_Bool WpADORecordset::MoveLast()    {return SUCCEEDED(pInterface->MoveLast());}

 sal_Bool WpADORecordset::IsAtBOF() const
{
    VARIANT_BOOL bIsAtBOF=VARIANT_FALSE;
    pInterface->get_BOF(&bIsAtBOF);
    return bIsAtBOF == VARIANT_TRUE;
}

 sal_Bool WpADORecordset::IsAtEOF() const
{
    VARIANT_BOOL bIsAtEOF=VARIANT_FALSE;
    pInterface->get_EOF(&bIsAtEOF);
    return bIsAtEOF == VARIANT_TRUE;
}

 sal_Bool WpADORecordset::Delete(AffectEnum eNum)
{
    return SUCCEEDED(pInterface->Delete(eNum));
}

 sal_Bool WpADORecordset::AddNew(const OLEVariant &FieldList,const OLEVariant &Values)
{
    return SUCCEEDED(pInterface->AddNew(FieldList,Values));
}

 sal_Bool WpADORecordset::Update(const OLEVariant &FieldList,const OLEVariant &Values)
{
    return SUCCEEDED(pInterface->Update(FieldList,Values));
}

 sal_Bool WpADORecordset::CancelUpdate()
{
    return SUCCEEDED(pInterface->CancelUpdate());
}

 ADOProperties* WpADORecordset::get_Properties() const
{
    ADOProperties* pProps=NULL;
    pInterface->get_Properties(&pProps);
    return pProps;
}

 sal_Bool WpADORecordset::NextRecordset(OLEVariant& RecordsAffected,ADORecordset** ppiRset)
{
    return SUCCEEDED(pInterface->NextRecordset(&RecordsAffected,ppiRset));
}

 sal_Bool WpADORecordset::get_RecordCount(sal_Int32 &_nRet) const
{
    return SUCCEEDED(pInterface->get_RecordCount(&_nRet));
}

 sal_Bool WpADORecordset::get_MaxRecords(sal_Int32 &_nRet) const
{
    return SUCCEEDED(pInterface->get_MaxRecords(&_nRet));
}

 sal_Bool WpADORecordset::put_MaxRecords(sal_Int32 _nRet)
{
    return SUCCEEDED(pInterface->put_MaxRecords(_nRet));
}

 sal_Bool WpADORecordset::get_CursorType(CursorTypeEnum &_nRet) const
{
    return SUCCEEDED(pInterface->get_CursorType(&_nRet));
}

 sal_Bool WpADORecordset::put_CursorType(CursorTypeEnum _nRet)
{
    return SUCCEEDED(pInterface->put_CursorType(_nRet));
}

 sal_Bool WpADORecordset::get_LockType(LockTypeEnum &_nRet) const
{
    return SUCCEEDED(pInterface->get_LockType(&_nRet));
}

 sal_Bool WpADORecordset::put_LockType(LockTypeEnum _nRet)
{
    return SUCCEEDED(pInterface->put_LockType(_nRet));
}

 sal_Bool WpADORecordset::get_CacheSize(sal_Int32 &_nRet) const
{
    return SUCCEEDED(pInterface->get_CacheSize(&_nRet));
}

 sal_Bool WpADORecordset::put_CacheSize(sal_Int32 _nRet)
{
    return SUCCEEDED(pInterface->put_CacheSize(_nRet));
}

 sal_Bool WpADORecordset::UpdateBatch(AffectEnum AffectRecords)
{
    return SUCCEEDED(pInterface->UpdateBatch(AffectRecords));
}

 ::rtl::OUString WpADOParameter::GetName() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

 DataTypeEnum WpADOParameter::GetADOType() const
{
    DataTypeEnum eType=adEmpty;
    pInterface->get_Type(&eType);
    return eType;
}

 sal_Int32 WpADOParameter::GetAttributes() const
{
    sal_Int32 eADOSFieldAttributes=0;
    pInterface->get_Attributes(&eADOSFieldAttributes);
    return eADOSFieldAttributes;
}

 sal_Int32 WpADOParameter::GetPrecision() const
{
    sal_uInt8 eType=0;
    pInterface->get_Precision(&eType);
    return eType;
}

 sal_Int32 WpADOParameter::GetNumericScale() const
{
    sal_uInt8 eType=0;
    pInterface->get_NumericScale(&eType);
    return eType;
}

 ParameterDirectionEnum WpADOParameter::get_Direction() const
{
    ParameterDirectionEnum alParmDirection=adParamUnknown;
    pInterface->get_Direction(&alParmDirection);
    return alParmDirection;
}

 void WpADOParameter::GetValue(OLEVariant& aValVar) const
{
    pInterface->get_Value(&aValVar);
}

 OLEVariant WpADOParameter::GetValue() const
{
    OLEVariant aValVar;
    pInterface->get_Value(&aValVar);
    return aValVar;
}

 sal_Bool WpADOParameter::PutValue(const OLEVariant& aVariant)
{
    return (SUCCEEDED(pInterface->put_Value(aVariant)));
}

 ::rtl::OUString WpADOColumn::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}
::rtl::OUString WpADOColumn::get_RelatedColumn() const
{
    OLEString aBSTR;
    pInterface->get_RelatedColumn(&aBSTR);
    return aBSTR;
}

void WpADOColumn::put_Name(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
}
void WpADOColumn::put_RelatedColumn(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_RelatedColumn(bstr));
}

DataTypeEnum WpADOColumn::get_Type() const
{
    DataTypeEnum eNum = adVarChar;
    pInterface->get_Type(&eNum);
    return eNum;
}

void WpADOColumn::put_Type(const DataTypeEnum& _eNum)
{
    pInterface->put_Type(_eNum);
}

sal_Int32 WpADOColumn::get_Precision() const
{
    sal_Int32 nPrec=0;
    pInterface->get_Precision(&nPrec);
    return nPrec;
}

void WpADOColumn::put_Precision(sal_Int32 _nPre)
{
    pInterface->put_Precision(_nPre);
}

sal_Int32 WpADOColumn::get_NumericScale() const
{
    sal_uInt8 nPrec=0;
    pInterface->get_NumericScale(&nPrec);
    return nPrec;
}

void WpADOColumn::put_NumericScale(sal_Int8 _nScale)
{
    pInterface->put_NumericScale(_nScale);
}

SortOrderEnum WpADOColumn::get_SortOrder() const
{
    SortOrderEnum nPrec=adSortAscending;
    pInterface->get_SortOrder(&nPrec);
    return nPrec;
}

void WpADOColumn::put_SortOrder(SortOrderEnum _nScale)
{
    pInterface->put_SortOrder(_nScale);
}

ColumnAttributesEnum WpADOColumn::get_Attributes() const
{
    ColumnAttributesEnum eNum=adColNullable;
    pInterface->get_Attributes(&eNum);
    return eNum;
}

void WpADOColumn::put_Attributes(const ColumnAttributesEnum& _eNum)
{
    pInterface->put_Attributes(_eNum);
}

ADOProperties* WpADOColumn::get_Properties() const
{
    ADOProperties* pProps = NULL;
    pInterface->get_Properties(&pProps);
    return pProps;
}

::rtl::OUString WpADOKey::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOKey::put_Name(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

}

KeyTypeEnum WpADOKey::get_Type() const
{
    KeyTypeEnum eNum=adKeyPrimary;
    pInterface->get_Type(&eNum);
    return eNum;
}

void WpADOKey::put_Type(const KeyTypeEnum& _eNum)
{
    pInterface->put_Type(_eNum);
}

::rtl::OUString WpADOKey::get_RelatedTable() const
{
    OLEString aBSTR;
    pInterface->get_RelatedTable(&aBSTR);
    return aBSTR;
}

void WpADOKey::put_RelatedTable(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_RelatedTable(bstr));

}

RuleEnum WpADOKey::get_DeleteRule() const
{
    RuleEnum eNum = adRINone;
    pInterface->get_DeleteRule(&eNum);
    return eNum;
}

void WpADOKey::put_DeleteRule(const RuleEnum& _eNum)
{
    pInterface->put_DeleteRule(_eNum);
}

RuleEnum WpADOKey::get_UpdateRule() const
{
    RuleEnum eNum = adRINone;
    pInterface->get_UpdateRule(&eNum);
    return eNum;
}

void WpADOKey::put_UpdateRule(const RuleEnum& _eNum)
{
    pInterface->put_UpdateRule(_eNum);
}

ADOColumns* WpADOKey::get_Columns() const
{
    ADOColumns* pCols = NULL;
    pInterface->get_Columns(&pCols);
    return pCols;
}

::rtl::OUString WpADOIndex::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOIndex::put_Name(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

}

sal_Bool WpADOIndex::get_Clustered() const
{
    VARIANT_BOOL eNum = VARIANT_FALSE;
    pInterface->get_Clustered(&eNum);
    return eNum == VARIANT_TRUE;
}

void WpADOIndex::put_Clustered(sal_Bool _b)
{
    pInterface->put_Clustered(_b ? VARIANT_TRUE : VARIANT_FALSE);
}

sal_Bool WpADOIndex::get_Unique() const
{
    VARIANT_BOOL eNum = VARIANT_FALSE;
    pInterface->get_Unique(&eNum);
    return eNum == VARIANT_TRUE;
}

void WpADOIndex::put_Unique(sal_Bool _b)
{
    pInterface->put_Unique(_b ? VARIANT_TRUE : VARIANT_FALSE);
}

sal_Bool WpADOIndex::get_PrimaryKey() const
{
    VARIANT_BOOL eNum = VARIANT_FALSE;
    pInterface->get_PrimaryKey(&eNum);
    return eNum == VARIANT_TRUE;
}

void WpADOIndex::put_PrimaryKey(sal_Bool _b)
{
    pInterface->put_PrimaryKey(_b ? VARIANT_TRUE : VARIANT_FALSE);
}

ADOColumns* WpADOIndex::get_Columns() const
{
    ADOColumns* pCols = NULL;
    pInterface->get_Columns(&pCols);
    return pCols;
}

void WpADOCatalog::putref_ActiveConnection(IDispatch* pCon)
{
    pInterface->putref_ActiveConnection(pCon);
}

ADOTables* WpADOCatalog::get_Tables()
{
    ADOTables* pRet = NULL;
    pInterface->get_Tables(&pRet);
    return pRet;
}

ADOViews* WpADOCatalog::get_Views()
{
    ADOViews* pRet = NULL;
    pInterface->get_Views(&pRet);
    return pRet;
}

ADOGroups* WpADOCatalog::get_Groups()
{
    ADOGroups* pRet = NULL;
    pInterface->get_Groups(&pRet);
    return pRet;
}

ADOUsers* WpADOCatalog::get_Users()
{
    ADOUsers* pRet = NULL;
    pInterface->get_Users(&pRet);
    return pRet;
}

ADOProcedures* WpADOCatalog::get_Procedures()
{
    ADOProcedures* pRet = NULL;
    pInterface->get_Procedures(&pRet);
    return pRet;
}

::rtl::OUString WpADOTable::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOTable::put_Name(const ::rtl::OUString& _rName)
{
    OLEString bstr(_rName);
    sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

}

::rtl::OUString WpADOTable::get_Type() const
{
    OLEString aBSTR;
    pInterface->get_Type(&aBSTR);
    return aBSTR;
}

ADOColumns* WpADOTable::get_Columns() const
{
    ADOColumns* pCols = NULL;
    pInterface->get_Columns(&pCols);
    return pCols;
}

ADOIndexes* WpADOTable::get_Indexes() const
{
    ADOIndexes* pCols = NULL;
    pInterface->get_Indexes(&pCols);
    return pCols;
}

ADOKeys* WpADOTable::get_Keys() const
{
    ADOKeys* pCols = NULL;
    pInterface->get_Keys(&pCols);
    return pCols;
}

WpADOCatalog WpADOTable::get_ParentCatalog() const
{
    ADOCatalog* pCat = NULL;
    pInterface->get_ParentCatalog(&pCat);
    return WpADOCatalog(pCat);
}

ADOProperties* WpADOTable::get_Properties() const
{
    ADOProperties* pProps = NULL;
    pInterface->get_Properties(&pProps);
    return pProps;
}

::rtl::OUString WpADOView::get_Name() const
{
    OLEString aBSTR;
    pInterface->get_Name(&aBSTR);
    return aBSTR;
}

void WpADOView::get_Command(OLEVariant& _rVar) const
{
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

ADOUsers* WpADOGroup::get_Users( )
{
    ADOUsers* pRet = NULL;
    pInterface->get_Users( &pRet);
    return pRet;
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

}

sal_Bool WpADOUser::ChangePassword(const ::rtl::OUString& _rPwd,const ::rtl::OUString& _rNewPwd)
{
    OLEString sStr1(_rPwd);
    OLEString sStr2(_rNewPwd);
    sal_Bool bErg = SUCCEEDED(pInterface->ChangePassword(sStr1,sStr2));
    return bErg;
}

ADOGroups* WpADOUser::get_Groups()
{
    ADOGroups* pRet = NULL;
    pInterface->get_Groups(&pRet);
    return pRet;
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