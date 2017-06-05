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
#include <osl/file.hxx>
#include <osl/thread.h>
#include <unotools/ucbstreamhelper.hxx>
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
#include <memory>

class ImpSdrObjTextLink: public ::sfx2::SvBaseLink
{
    SdrTextObj*                 pSdrObj;

public:
    explicit ImpSdrObjTextLink( SdrTextObj* pObj1 )
        : ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SIMPLE_FILE ),
            pSdrObj( pObj1 )
    {}

    virtual void Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;
};

void ImpSdrObjTextLink::Closed()
{
    if (pSdrObj )
    {
        // set pLink of the object to NULL, because we are destroying the link instance now
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if (pData!=nullptr) pData->pLink=nullptr;
        pSdrObj->ReleaseTextLink();
    }
    SvBaseLink::Closed();
}


::sfx2::SvBaseLink::UpdateResult ImpSdrObjTextLink::DataChanged(
    const OUString& /*rMimeType*/, const css::uno::Any & /*rValue */)
{
    bool bForceReload = false;
    SdrModel* pModel = pSdrObj ? pSdrObj->GetModel() : nullptr;
    sfx2::LinkManager* pLinkManager= pModel ? pModel->GetLinkManager() : nullptr;
    if( pLinkManager )
    {
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if( pData )
        {
            OUString aFile;
            OUString aFilter;
            sfx2::LinkManager::GetDisplayNames( this, nullptr,&aFile, nullptr, &aFilter );

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


ImpSdrObjTextLinkUserData::ImpSdrObjTextLinkUserData():
    SdrObjUserData(SdrInventor::Default,SDRUSERDATA_OBJTEXTLINK),
    aFileDate0( DateTime::EMPTY ),
    pLink(nullptr),
    eCharSet(RTL_TEXTENCODING_DONTKNOW)
{
}

ImpSdrObjTextLinkUserData::~ImpSdrObjTextLinkUserData()
{
}

SdrObjUserData* ImpSdrObjTextLinkUserData::Clone(SdrObject* ) const
{
    ImpSdrObjTextLinkUserData* pData=new ImpSdrObjTextLinkUserData;
    pData->aFileName  =aFileName;
    pData->aFilterName=aFilterName;
    pData->aFileDate0 =aFileDate0;
    pData->eCharSet   =eCharSet;
    pData->pLink=nullptr;
    return pData;
}


void SdrTextObj::SetTextLink(const OUString& rFileName, const OUString& rFilterName)
{
    rtl_TextEncoding eCharSet = osl_getThreadTextEncoding();

    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    if (pData!=nullptr) {
        ReleaseTextLink();
    }
    pData=new ImpSdrObjTextLinkUserData;
    pData->aFileName=rFileName;
    pData->aFilterName=rFilterName;
    pData->eCharSet=eCharSet;
    AppendUserData(pData);
    ImpRegisterLink();
}

void SdrTextObj::ReleaseTextLink()
{
    ImpDeregisterLink();
    sal_uInt16 nCount=GetUserDataCount();
    for (sal_uInt16 nNum=nCount; nNum>0;) {
        nNum--;
        SdrObjUserData* pData=GetUserData(nNum);
        if (pData->GetInventor()==SdrInventor::Default && pData->GetId()==SDRUSERDATA_OBJTEXTLINK) {
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
        bool                        bExists = true;

        try
        {
            INetURLObject aURL( pData->aFileName );
            DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

            ::ucbhelper::Content aCnt( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            css::uno::Any aAny( aCnt.getPropertyValue("DateModified") );
            css::util::DateTime aDateTime;

            aAny >>= aDateTime;
            ::utl::typeConvert( aDateTime, aFileDT );
        }
        catch( ... )
        {
            bExists = false;
        }

        if( bExists )
        {
            bool bLoad = false;
            if( bForceLoad )
                bLoad = true;
            else
                bLoad = ( aFileDT > pData->aFileDate0 );

            if( bLoad )
            {
                bRet = LoadText( pData->aFileName, pData->eCharSet );
            }

            pData->aFileDate0 = aFileDT;
        }
    }

    return bRet;
}

bool SdrTextObj::LoadText(const OUString& rFileName, rtl_TextEncoding eCharSet)
{
    INetURLObject   aFileURL( rFileName );
    bool            bRet = false;

    if( aFileURL.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aFileURLStr;

        if( osl::FileBase::getFileURLFromSystemPath( rFileName, aFileURLStr ) == osl::FileBase::E_None )
            aFileURL = INetURLObject( aFileURLStr );
        else
            aFileURL.SetSmartURL( rFileName );
    }

    DBG_ASSERT( aFileURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aFileURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));

    if( pIStm )
    {
        pIStm->SetStreamCharSet(GetSOLoadTextEncoding(eCharSet));

        char cRTF[5];
        cRTF[4] = 0;
        pIStm->ReadBytes(cRTF, 5);

        bool bRTF = cRTF[0] == '{' && cRTF[1] == '\\' && cRTF[2] == 'r' && cRTF[3] == 't' && cRTF[4] == 'f';

        pIStm->Seek(0);

        if( !pIStm->GetError() )
        {
            SetText( *pIStm, aFileURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), sal::static_int_cast< sal_uInt16 >( bRTF ? EE_FORMAT_RTF : EE_FORMAT_TEXT ) );
            bRet = true;
        }
    }

    return bRet;
}

ImpSdrObjTextLinkUserData* SdrTextObj::GetLinkUserData() const
{
    sal_uInt16 nCount=GetUserDataCount();
    for (sal_uInt16 nNum=nCount; nNum>0;) {
        nNum--;
        SdrObjUserData * pData=GetUserData(nNum);
        if (pData->GetInventor() == SdrInventor::Default
            && pData->GetId() == SDRUSERDATA_OBJTEXTLINK)
        {
            return static_cast<ImpSdrObjTextLinkUserData *>(pData);
        }
    }
    return nullptr;
}

void SdrTextObj::ImpRegisterLink()
{
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    sfx2::LinkManager* pLinkManager=pModel!=nullptr ? pModel->GetLinkManager() : nullptr;
    if (pLinkManager!=nullptr && pData!=nullptr && pData->pLink==nullptr) { // don't register twice
        pData->pLink = new ImpSdrObjTextLink(this);
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     !pData->aFilterName.isEmpty() ?
                                      &pData->aFilterName : nullptr);
    }
}

void SdrTextObj::ImpDeregisterLink()
{
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    sfx2::LinkManager* pLinkManager=pModel!=nullptr ? pModel->GetLinkManager() : nullptr;
    if (pLinkManager!=nullptr && pData!=nullptr && pData->pLink!=nullptr) { // don't register twice
        // when doing Remove, *pLink is deleted implicitly
        pLinkManager->Remove( pData->pLink.get() );
        pData->pLink=nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
