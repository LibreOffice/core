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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/datetime.hxx>
#include <svx/svdotext.hxx>
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
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue );

    bool Connect() { return 0 != SvBaseLink::GetRealObject(); }
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
    const OUString& /*rMimeType*/, const ::com::sun::star::uno::Any & /*rValue */)
{
    bool bForceReload = false;
    SdrModel* pModel = pSdrObj ? pSdrObj->GetModel() : 0;
    sfx2::LinkManager* pLinkManager= pModel ? pModel->GetLinkManager() : 0;
    if( pLinkManager )
    {
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if( pData )
        {
            OUString aFile;
            OUString aFilter;
            pLinkManager->GetDisplayNames( this, 0,&aFile, 0, &aFilter );

            if( pData->aFileName != aFile ||
                pData->aFilterName != aFilter )
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


void SdrTextObj::SetTextLink(const OUString& rFileName, const OUString& rFilterName, rtl_TextEncoding eCharSet)
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
        bool                        bExists = true, bLoad = false;

        try
        {
            INetURLObject aURL( pData->aFileName );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            ::ucbhelper::Content aCnt( aURL.GetMainURL( INetURLObject::NO_DECODE ), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            ::com::sun::star::uno::Any aAny( aCnt.getPropertyValue("DateModified") );
            ::com::sun::star::util::DateTime aDateTime;

            aAny >>= aDateTime;
            ::utl::typeConvert( aDateTime, aFileDT );
        }
        catch( ... )
        {
            bExists = false;
        }

        if( bExists )
        {
            if( bForceLoad )
                bLoad = true;
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

bool SdrTextObj::LoadText(const OUString& rFileName, const OUString& /*rFilterName*/, rtl_TextEncoding eCharSet)
{
    INetURLObject   aFileURL( rFileName );
    bool            bRet = false;

    if( aFileURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        OUString aFileURLStr;

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

        bool bRTF = cRTF[0] == '{' && cRTF[1] == '\\' && cRTF[2] == 'r' && cRTF[3] == 't' && cRTF[4] == 'f';

        pIStm->Seek(0);

        if( !pIStm->GetError() )
        {
            SetText( *pIStm, aFileURL.GetMainURL( INetURLObject::NO_DECODE ), sal::static_int_cast< sal_uInt16 >( bRTF ? EE_FORMAT_RTF : EE_FORMAT_TEXT ) );
            bRet = true;
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
        pData->pLink = new ImpSdrObjTextLink(this);
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     !pData->aFilterName.isEmpty() ?
                                      &pData->aFilterName : NULL,
                                     NULL);
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
