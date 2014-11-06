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


#include <sfx2/lnkbase.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <vcl/layout.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/svapp.hxx>
#include "app.hrc"
#include <sfx2/sfxresid.hxx>
#include <sfx2/filedlghelper.hxx>
#include <tools/debug.hxx>
#include <svl/svdde.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sfx2
{

TYPEINIT0( SvBaseLink )

struct BaseLink_Impl
{
    Link                m_aEndEditLink;
    LinkManager*        m_pLinkMgr;
    vcl::Window*        m_pParentWin;
    FileDialogHelper*   m_pFileDlg;
    bool                m_bIsConnect;

    BaseLink_Impl() :
          m_pLinkMgr( NULL )
        , m_pParentWin( NULL )
        , m_pFileDlg( NULL )
        , m_bIsConnect( false )
        {}

    ~BaseLink_Impl()
        { delete m_pFileDlg; }
};

// only for internal management
struct ImplBaseLinkData
{
     // applies for all links
     sal_uIntPtr nCntntType; // Update Format
     // Not Ole-Links
     bool        bIntrnlLnk;  // It is an internal link
     sal_uInt16  nUpdateMode; // UpdateMode

    ImplBaseLinkData()
    {
        nCntntType = 0;
        bIntrnlLnk = false;
        nUpdateMode = 0;
    }
};


SvBaseLink::SvBaseLink()
    : m_bIsReadOnly(false)
{
    pImpl = new BaseLink_Impl();
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = true;
    bWasLastEditOK = false;
}



SvBaseLink::SvBaseLink( sal_uInt16 nUpdateMode, sal_uIntPtr nContentType )
    : m_bIsReadOnly(false)
{
    pImpl = new BaseLink_Impl();
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = true;
    bWasLastEditOK = false;

    // It it going to be a Ole-Link,
    pImplData->nUpdateMode = nUpdateMode;
    pImplData->nCntntType = nContentType;
    pImplData->bIntrnlLnk = false;
}



SvBaseLink::~SvBaseLink()
{
    Disconnect();

    delete pImplData;
    delete pImpl;
}

IMPL_LINK( SvBaseLink, EndEditHdl, OUString*, _pNewName )
{
    OUString sNewName;
    if ( _pNewName )
        sNewName = *_pNewName;
    if ( !ExecuteEdit( sNewName ) )
        sNewName = "";
    bWasLastEditOK = !sNewName.isEmpty();
    if ( pImpl->m_aEndEditLink.IsSet() )
        pImpl->m_aEndEditLink.Call( this );
    return 0;
}



void SvBaseLink::SetObjType( sal_uInt16 nObjTypeP )
{
    DBG_ASSERT( nObjType != OBJECT_CLIENT_DDE, "type already set" );
    DBG_ASSERT( !xObj.Is(), "object exist" );

    nObjType = nObjTypeP;
}



void SvBaseLink::SetName( const OUString & rNm )
{
    aLinkName = rNm;
}



void SvBaseLink::SetObj( SvLinkSource * pObj )
{
    DBG_ASSERT( (nObjType & OBJECT_CLIENT_SO &&
                pImplData->bIntrnlLnk) ||
                nObjType == OBJECT_CLIENT_GRF,
                "no intern link" );
    xObj = pObj;
}



void SvBaseLink::SetLinkSourceName( const OUString & rLnkNm )
{
    if( aLinkName == rLnkNm )
        return;

    AddNextRef(); // should be superfluous
    // remove old connection
    Disconnect();

    aLinkName = rLnkNm;

    // New Connection
    _GetRealObject();
    ReleaseRef(); // should be superfluous
}






void SvBaseLink::SetUpdateMode( sal_uInt16 nMode )
{
    if( ( OBJECT_CLIENT_SO & nObjType ) &&
        pImplData->nUpdateMode != nMode )
    {
        AddNextRef();
        Disconnect();

        pImplData->nUpdateMode = nMode;
        _GetRealObject();
        ReleaseRef();
    }
}

// #i88291#
void SvBaseLink::clearStreamToLoadFrom()
{
    m_xInputStreamToLoadFrom.clear();
    if( xObj.Is() )
    {
        xObj->clearStreamToLoadFrom();
    }
}

bool SvBaseLink::Update()
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        AddNextRef();
        Disconnect();

        _GetRealObject();
        ReleaseRef();
        if( xObj.Is() )
        {
            xObj->setStreamToLoadFrom(m_xInputStreamToLoadFrom,m_bIsReadOnly);
            OUString sMimeType( SotExchange::GetFormatMimeType(
                            pImplData->nCntntType ));
            Any aData;

            if( xObj->GetData( aData, sMimeType ) )
            {
                UpdateResult eRes = DataChanged(sMimeType, aData);
                bool bSuccess = eRes == SUCCESS;
                //for manual Updates there is no need to hold the ServerObject
                if( OBJECT_CLIENT_DDE == nObjType &&
                    LINKUPDATE_ONCALL == GetUpdateMode() && xObj.Is() )
                    xObj->RemoveAllDataAdvise( this );
                return bSuccess;
            }
            if( xObj.Is() )
            {
                // should be asynschron?
                if( xObj->IsPending() )
                    return true;

                // we do not need the object anymore
                AddNextRef();
                Disconnect();
                ReleaseRef();
            }
        }
    }
    return false;
}


sal_uInt16 SvBaseLink::GetUpdateMode() const
{
    return ( OBJECT_CLIENT_SO & nObjType )
            ? pImplData->nUpdateMode
            : sal::static_int_cast< sal_uInt16 >( LINKUPDATE_ONCALL );
}


void SvBaseLink::_GetRealObject( bool bConnect)
{
    if( !pImpl->m_pLinkMgr )
        return;

    DBG_ASSERT( !xObj.Is(), "object already exist" );

    if( OBJECT_CLIENT_DDE == nObjType )
    {
        OUString sServer;
        if( pImpl->m_pLinkMgr->GetDisplayNames( this, &sServer ) &&
            sServer == Application::GetAppName() )  // internal Link !!!
        {
            // so that the Internal link can be created!
            nObjType = OBJECT_INTERN;
            xObj = pImpl->m_pLinkMgr->CreateObj( this );

            pImplData->bIntrnlLnk = true;
            nObjType = OBJECT_CLIENT_DDE;  // so we know what it once was!
        }
        else
        {
            pImplData->bIntrnlLnk = false;
            xObj = pImpl->m_pLinkMgr->CreateObj( this );
        }
    }
    else if( (OBJECT_CLIENT_SO & nObjType) )
        xObj = pImpl->m_pLinkMgr->CreateObj( this );

    if( bConnect && ( !xObj.Is() || !xObj->Connect( this ) ) )
        Disconnect();
}

sal_uIntPtr SvBaseLink::GetContentType() const
{
    if( OBJECT_CLIENT_SO & nObjType )
        return pImplData->nCntntType;

    return 0;  // all Formats ?
}


bool SvBaseLink::SetContentType( sal_uIntPtr nType )
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        pImplData->nCntntType = nType;
        return true;
    }
    return false;
}

LinkManager* SvBaseLink::GetLinkManager()
{
    return pImpl->m_pLinkMgr;
}

const LinkManager* SvBaseLink::GetLinkManager() const
{
    return pImpl->m_pLinkMgr;
}

void SvBaseLink::SetLinkManager( LinkManager* _pMgr )
{
    pImpl->m_pLinkMgr = _pMgr;
}

void SvBaseLink::Disconnect()
{
    if( xObj.Is() )
    {
        xObj->RemoveAllDataAdvise( this );
        xObj->RemoveConnectAdvise( this );
        xObj.Clear();
    }
}

SvBaseLink::UpdateResult SvBaseLink::DataChanged( const OUString &, const ::com::sun::star::uno::Any & )
{
    return SUCCESS;
}

void SvBaseLink::Edit( vcl::Window* pParent, const Link& rEndEditHdl )
{
    pImpl->m_pParentWin = pParent;
    pImpl->m_aEndEditLink = rEndEditHdl;
    pImpl->m_bIsConnect = xObj.Is();
    if( !pImpl->m_bIsConnect )
        _GetRealObject( xObj.Is() );

    bool bAsync = false;
    Link aLink = LINK( this, SvBaseLink, EndEditHdl );

    if( OBJECT_CLIENT_SO & nObjType && pImplData->bIntrnlLnk )
    {
        if( pImpl->m_pLinkMgr )
        {
            SvLinkSourceRef ref = pImpl->m_pLinkMgr->CreateObj( this );
            if( ref.Is() )
            {
                ref->Edit( pParent, this, aLink );
                bAsync = true;
            }
        }
    }
    else
    {
        xObj->Edit( pParent, this, aLink );
        bAsync = true;
    }

    if ( !bAsync )
    {
        ExecuteEdit( OUString() );
        bWasLastEditOK = false;
        if ( pImpl->m_aEndEditLink.IsSet() )
            pImpl->m_aEndEditLink.Call( this );
    }
}

bool SvBaseLink::ExecuteEdit( const OUString& _rNewName )
{
    if( !_rNewName.isEmpty() )
    {
        SetLinkSourceName( _rNewName );
        if( !Update() )
        {
            OUString sApp, sTopic, sItem, sError;
            pImpl->m_pLinkMgr->GetDisplayNames( this, &sApp, &sTopic, &sItem );
            if( nObjType == OBJECT_CLIENT_DDE )
            {
                sError = SFX2_RESSTR(STR_DDE_ERROR);

                sal_Int32 nFndPos = sError.indexOf( '%' );
                if( -1 != nFndPos )
                {
                    sError = sError.replaceAt( nFndPos, 1, sApp );
                    nFndPos = nFndPos + sApp.getLength();

                    if( -1 != ( nFndPos = sError.indexOf( '%', nFndPos )))
                    {
                        sError = sError.replaceAt( nFndPos, 1, sTopic );
                        nFndPos = nFndPos + sTopic.getLength();

                        if( -1 != ( nFndPos = sError.indexOf( '%', nFndPos )))
                            sError = sError.replaceAt( nFndPos, 1, sItem );
                    }
                }
            }
            else
                return false;

            MessageDialog(pImpl->m_pParentWin, sError).Execute();
        }
    }
    else if( !pImpl->m_bIsConnect )
        Disconnect();
    pImpl->m_bIsConnect = false;
    return true;
}

void SvBaseLink::Closed()
{
    if( xObj.Is() )
        xObj->RemoveAllDataAdvise( this );
}

FileDialogHelper & SvBaseLink::GetInsertFileDialog(const OUString& rFactory) const
{
    if ( pImpl->m_pFileDlg )
        delete pImpl->m_pFileDlg;
    pImpl->m_pFileDlg = new FileDialogHelper(
            ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            SFXWB_INSERT, rFactory);
    return *pImpl->m_pFileDlg;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
