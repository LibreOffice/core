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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/datetime.hxx>
#include <svx/svdotext.hxx>
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx>
#include <editeng/editdata.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <tools/tenccvt.hxx>


class ImpSdrObjTextLink: public ::sfx2::SvBaseLink
{
    SdrTextObj*                 pSdrObj;

public:
    ImpSdrObjTextLink( SdrTextObj* pObj1 )
        : ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, FORMAT_FILE ),
            pSdrObj( pObj1 )
    {}
    virtual ~ImpSdrObjTextLink();

    virtual void Closed();
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );

    sal_Bool Connect() { return 0 != SvBaseLink::GetRealObject(); }
};

ImpSdrObjTextLink::~ImpSdrObjTextLink()
{
}

void ImpSdrObjTextLink::Closed()
{
    if (pSdrObj )
    {
        // set pLink of the object to NULL, because we are destroying the link instance now
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if (pData!=NULL) pData->pLink=NULL;
        pSdrObj->ReleaseTextLink();
    }
    SvBaseLink::Closed();
}


::sfx2::SvBaseLink::UpdateResult ImpSdrObjTextLink::DataChanged(
    const String& /*rMimeType*/, const ::com::sun::star::uno::Any & /*rValue */)
{
    bool bForceReload = false;
    SdrModel* pModel = pSdrObj ? pSdrObj->GetModel() : 0;
    sfx2::LinkManager* pLinkManager= pModel ? pModel->GetLinkManager() : 0;
    if( pLinkManager )
    {
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if( pData )
        {
            String aFile;
            String aFilter;
            pLinkManager->GetDisplayNames( this, 0,&aFile, 0, &aFilter );

            if( !pData->aFileName.Equals( aFile ) ||
                !pData->aFilterName.Equals( aFilter ))
            {
                pData->aFileName = aFile;
                pData->aFilterName = aFilter;
                pSdrObj->SetChanged();
                bForceReload = true;
            }
        }
    }
    if (pSdrObj )
        pSdrObj->ReloadLinkedText( bForceReload );

    return SUCCESS;
}


TYPEINIT1(ImpSdrObjTextLinkUserData,SdrObjUserData);

ImpSdrObjTextLinkUserData::ImpSdrObjTextLinkUserData(SdrTextObj* pObj1):
    SdrObjUserData(SdrInventor,SDRUSERDATA_OBJTEXTLINK,0),
    pObj(pObj1),
    aFileDate0( DateTime::EMPTY ),
    pLink(NULL),
    eCharSet(RTL_TEXTENCODING_DONTKNOW)
{
}

ImpSdrObjTextLinkUserData::~ImpSdrObjTextLinkUserData()
{
    delete pLink;
}

SdrObjUserData* ImpSdrObjTextLinkUserData::Clone(SdrObject* pObj1) const
{
    ImpSdrObjTextLinkUserData* pData=new ImpSdrObjTextLinkUserData((SdrTextObj*)pObj1);
    pData->aFileName  =aFileName;
    pData->aFilterName=aFilterName;
    pData->aFileDate0 =aFileDate0;
    pData->eCharSet   =eCharSet;
    pData->pLink=NULL;
    return pData;
}


void SdrTextObj::SetTextLink(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet)
{
    if(eCharSet == RTL_TEXTENCODING_DONTKNOW)
        eCharSet = osl_getThreadTextEncoding();

    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    if (pData!=NULL) {
        ReleaseTextLink();
    }
    pData=new ImpSdrObjTextLinkUserData(this);
    pData->aFileName=rFileName;
    pData->aFilterName=rFilterName;
    pData->eCharSet=eCharSet;
    AppendUserData(pData);
    ImpLinkAnmeldung();
}

void SdrTextObj::ReleaseTextLink()
{
    ImpLinkAbmeldung();
    sal_uInt16 nAnz=GetUserDataCount();
    for (sal_uInt16 nNum=nAnz; nNum>0;) {
        nNum--;
        SdrObjUserData* pData=GetUserData(nNum);
        if (pData->GetInventor()==SdrInventor && pData->GetId()==SDRUSERDATA_OBJTEXTLINK) {
            DeleteUserData(nNum);
        }
    }
}

bool SdrTextObj::ReloadLinkedText( bool bForceLoad)
{
    ImpSdrObjTextLinkUserData*  pData = GetLinkUserData();
    bool                        bRet = true;

    if( pData )
    {
        DateTime                    aFileDT( DateTime::EMPTY );
        sal_Bool                        bExists = sal_True, bLoad = sal_False;

        try
        {
            INetURLObject aURL( pData->aFileName );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            ::ucbhelper::Content aCnt( aURL.GetMainURL( INetURLObject::NO_DECODE ), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            ::com::sun::star::uno::Any aAny( aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ) ) );
            ::com::sun::star::util::DateTime aDateTime;

            aAny >>= aDateTime;
            ::utl::typeConvert( aDateTime, aFileDT );
        }
        catch( ... )
        {
            bExists = sal_False;
        }

        if( bExists )
        {
            if( bForceLoad )
                bLoad = sal_True;
            else
                bLoad = ( aFileDT > pData->aFileDate0 );

            if( bLoad )
            {
                bRet = LoadText( pData->aFileName, pData->aFilterName, pData->eCharSet );
            }

            pData->aFileDate0 = aFileDT;
        }
    }

    return bRet;
}

bool SdrTextObj::LoadText(const String& rFileName, const String& /*rFilterName*/, rtl_TextEncoding eCharSet)
{
    INetURLObject   aFileURL( rFileName );
    sal_Bool            bRet = sal_False;

    if( aFileURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        rtl::OUString aFileURLStr;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aFileURLStr ) )
            aFileURL = INetURLObject( aFileURLStr );
        else
            aFileURL.SetSmartURL( rFileName );
    }

    DBG_ASSERT( aFileURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        pIStm->SetStreamCharSet(GetSOLoadTextEncoding(eCharSet));

        char cRTF[5];
        cRTF[4] = 0;
        pIStm->Read(cRTF, 5);

        sal_Bool bRTF = cRTF[0] == '{' && cRTF[1] == '\\' && cRTF[2] == 'r' && cRTF[3] == 't' && cRTF[4] == 'f';

        pIStm->Seek(0);

        if( !pIStm->GetError() )
        {
            SetText( *pIStm, aFileURL.GetMainURL( INetURLObject::NO_DECODE ), sal::static_int_cast< sal_uInt16 >( bRTF ? EE_FORMAT_RTF : EE_FORMAT_TEXT ) );
            bRet = sal_True;
        }

        delete pIStm;
    }

    return bRet;
}

ImpSdrObjTextLinkUserData* SdrTextObj::GetLinkUserData() const
{
    ImpSdrObjTextLinkUserData* pData=NULL;
    sal_uInt16 nAnz=GetUserDataCount();
    for (sal_uInt16 nNum=nAnz; nNum>0 && pData==NULL;) {
        nNum--;
        pData=(ImpSdrObjTextLinkUserData*)GetUserData(nNum);
        if (pData->GetInventor()!=SdrInventor || pData->GetId()!=SDRUSERDATA_OBJTEXTLINK) {
            pData=NULL;
        }
    }
    return pData;
}

void SdrTextObj::ImpLinkAnmeldung()
{
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    sfx2::LinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
    if (pLinkManager!=NULL && pData!=NULL && pData->pLink==NULL) { // don't register twice
        pData->pLink=new ImpSdrObjTextLink(this);
#ifdef GCC
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     pData->aFilterName.Len() ?
                                      &pData->aFilterName : (const String *)NULL,
                                     (const String *)NULL);
#else
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     pData->aFilterName.Len() ? &pData->aFilterName : NULL,NULL);
#endif
        pData->pLink->Connect();
    }
}

void SdrTextObj::ImpLinkAbmeldung()
{
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    sfx2::LinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
    if (pLinkManager!=NULL && pData!=NULL && pData->pLink!=NULL) { // don't register twice
        // when doing Remove, *pLink is deleted implicitly
        pLinkManager->Remove( pData->pLink );
        pData->pLink=NULL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
