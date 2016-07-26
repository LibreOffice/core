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


class  ImplDdeItem;

struct BaseLink_Impl
{
    Link<SvBaseLink&,void> m_aEndEditLink;
    LinkManager*        m_pLinkMgr;
    VclPtr<vcl::Window> m_pParentWin;
    FileDialogHelper*   m_pFileDlg;
    bool                m_bIsConnect;

    BaseLink_Impl() :
          m_pLinkMgr( nullptr )
        , m_pParentWin( nullptr )
        , m_pFileDlg( nullptr )
        , m_bIsConnect( false )
        {}

    ~BaseLink_Impl()
        { delete m_pFileDlg; }
};

// only for internal management
struct ImplBaseLinkData
{
    struct tClientType
    {
        // applies for all links
        SotClipboardFormatId nCntntType; // Update Format
        // Not Ole-Links
        bool                 bIntrnlLnk;  // It is an internal link
        SfxLinkUpdateMode    nUpdateMode; // UpdateMode
    };

    struct tDDEType
    {
        ImplDdeItem* pItem;
    };

    union {
        tClientType ClientType;
        tDDEType DDEType;
    };
    ImplBaseLinkData()
    {
        ClientType.nCntntType = SotClipboardFormatId::NONE;
        ClientType.bIntrnlLnk = false;
        ClientType.nUpdateMode = SfxLinkUpdateMode::NONE;
        DDEType.pItem = nullptr;
    }
};


class ImplDdeItem : public DdeGetPutItem
{
    SvBaseLink* pLink;
    DdeData aData;
    Sequence< sal_Int8 > aSeq;  // Datacontainer for DdeData !!!
    bool bIsValidData : 1;
    bool bIsInDTOR : 1;
public:
#if defined(_WIN32)
    ImplDdeItem( SvBaseLink& rLink, const OUString& rStr )
        : DdeGetPutItem( rStr ), pLink( &rLink ), bIsValidData( false ),
        bIsInDTOR( false )
    {}
#endif
    virtual ~ImplDdeItem();

    virtual DdeData* Get( SotClipboardFormatId ) override;
    virtual bool     Put( const DdeData* ) override;
    virtual void     AdviseLoop( bool ) override;

    void Notify()
    {
        bIsValidData = false;
        DdeGetPutItem::NotifyClient();
    }

    bool IsInDTOR() const { return bIsInDTOR; }
};


SvBaseLink::SvBaseLink()
    : pImpl ( new BaseLink_Impl ),
      m_bIsReadOnly(false)
{
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = true;
    bWasLastEditOK = false;
}


SvBaseLink::SvBaseLink( SfxLinkUpdateMode nUpdateMode, SotClipboardFormatId nContentType )
   : pImpl( new BaseLink_Impl ),
     m_bIsReadOnly(false)
{
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = true;
    bWasLastEditOK = false;

    // It is going to be a Ole-Link,
    pImplData->ClientType.nUpdateMode = nUpdateMode;
    pImplData->ClientType.nCntntType = nContentType;
    pImplData->ClientType.bIntrnlLnk = false;
}

#if defined(_WIN32)

static DdeTopic* FindTopic( const OUString & rLinkName, sal_uInt16* pItemStt )
{
    if( rLinkName.isEmpty() )
        return 0;

    OUString sNm( rLinkName );
    sal_Int32 nTokenPos = 0;
    OUString sService( sNm.getToken( 0, cTokenSeparator, nTokenPos ) );

    DdeServices& rSvc = DdeService::GetServices();
    for (DdeServices::iterator aI = rSvc.begin(); aI != rSvc.end(); ++aI)
    {
        DdeService* pService = *aI;
        if( pService->GetName() == sService )
        {
            // then we search for the Topic
            OUString sTopic( sNm.getToken( 0, cTokenSeparator, nTokenPos ) );
            if( pItemStt )
                *pItemStt = nTokenPos;

            std::vector<DdeTopic*>& rTopics = pService->GetTopics();

            for( std::vector<DdeTopic*>::iterator iterTopic = rTopics.begin();
                 iterTopic != rTopics.end(); ++iterTopic )
                if( (*iterTopic)->GetName() == sTopic )
                    return *iterTopic;
            break;
        }
    }
    return 0;
}

SvBaseLink::SvBaseLink( const OUString& rLinkName, sal_uInt16 nObjectType, SvLinkSource* pObj )
    : pImpl()
    , m_bIsReadOnly(false)
{
    bVisible = bSynchron = bUseCache = true;
    bWasLastEditOK = false;
    aLinkName = rLinkName;
    pImplData = new ImplBaseLinkData;
    nObjType = nObjectType;

    if( !pObj )
    {
        DBG_ASSERT( pObj, "Where is my left-most object" );
        return;
    }

    if( OBJECT_DDE_EXTERN == nObjType )
    {
        sal_uInt16 nItemStt = 0;
        DdeTopic* pTopic = FindTopic( aLinkName, &nItemStt );
        if( pTopic )
        {
            // then we have it all together
            // MM_TODO how do I get the name
            OUString aStr = aLinkName; // xLinkName->GetDisplayName();
            aStr = aStr.copy( nItemStt );
            pImplData->DDEType.pItem = new ImplDdeItem( *this, aStr );
            pTopic->InsertItem( pImplData->DDEType.pItem );

            // store the Advice
            xObj = pObj;
        }
    }
    else if( pObj->Connect( this ) )
        xObj = pObj;
}

#endif

SvBaseLink::~SvBaseLink()
{
    Disconnect();

    switch( nObjType )
    {
    case OBJECT_DDE_EXTERN:
        if( !pImplData->DDEType.pItem->IsInDTOR() )
            delete pImplData->DDEType.pItem;
        break;
    }

    delete pImplData;
}

IMPL_LINK_TYPED( SvBaseLink, EndEditHdl, const OUString&, _rNewName, void )
{
    OUString sNewName = _rNewName;
    if ( !ExecuteEdit( sNewName ) )
        sNewName.clear();
    bWasLastEditOK = !sNewName.isEmpty();
    pImpl->m_aEndEditLink.Call( *this );
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
                pImplData->ClientType.bIntrnlLnk) ||
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
    GetRealObject_();
    ReleaseRef(); // should be superfluous
}


void SvBaseLink::SetUpdateMode( SfxLinkUpdateMode nMode )
{
    if( ( OBJECT_CLIENT_SO & nObjType ) &&
        pImplData->ClientType.nUpdateMode != nMode )
    {
        AddNextRef();
        Disconnect();

        pImplData->ClientType.nUpdateMode = nMode;
        GetRealObject_();
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

        GetRealObject_();
        ReleaseRef();
        if( xObj.Is() )
        {
            xObj->setStreamToLoadFrom(m_xInputStreamToLoadFrom,m_bIsReadOnly);
            OUString sMimeType( SotExchange::GetFormatMimeType(
                            pImplData->ClientType.nCntntType ));
            Any aData;

            if( xObj->GetData( aData, sMimeType ) )
            {
                UpdateResult eRes = DataChanged(sMimeType, aData);
                bool bSuccess = eRes == SUCCESS;
                //for manual Updates there is no need to hold the ServerObject
                if( OBJECT_CLIENT_DDE == nObjType &&
                    SfxLinkUpdateMode::ONCALL == GetUpdateMode() && xObj.Is() )
                    xObj->RemoveAllDataAdvise( this );
                return bSuccess;
            }
            if( xObj.Is() )
            {
                // should be asynchronous?
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


SfxLinkUpdateMode SvBaseLink::GetUpdateMode() const
{
    return ( OBJECT_CLIENT_SO & nObjType )
            ? pImplData->ClientType.nUpdateMode
            : SfxLinkUpdateMode::ONCALL;
}


void SvBaseLink::GetRealObject_( bool bConnect)
{
    if( !pImpl->m_pLinkMgr )
        return;

    DBG_ASSERT( !xObj.Is(), "object already exist" );

    if( OBJECT_CLIENT_DDE == nObjType )
    {
        OUString sServer;
        if( sfx2::LinkManager::GetDisplayNames( this, &sServer ) &&
            sServer == Application::GetAppName() )  // internal Link !!!
        {
            // so that the Internal link can be created!
            nObjType = OBJECT_INTERN;
            xObj = sfx2::LinkManager::CreateObj( this );

            pImplData->ClientType.bIntrnlLnk = true;
            nObjType = OBJECT_CLIENT_DDE;  // so we know what it once was!
        }
        else
        {
            pImplData->ClientType.bIntrnlLnk = false;
            xObj = sfx2::LinkManager::CreateObj( this );
        }
    }
    else if( (OBJECT_CLIENT_SO & nObjType) )
        xObj = sfx2::LinkManager::CreateObj( this );

    if( bConnect && ( !xObj.Is() || !xObj->Connect( this ) ) )
        Disconnect();
}

SotClipboardFormatId SvBaseLink::GetContentType() const
{
    if( OBJECT_CLIENT_SO & nObjType )
        return pImplData->ClientType.nCntntType;

    return SotClipboardFormatId::NONE;  // all Formats ?
}


void SvBaseLink::SetContentType( SotClipboardFormatId nType )
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        pImplData->ClientType.nCntntType = nType;
    }
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

SvBaseLink::UpdateResult SvBaseLink::DataChanged( const OUString &, const css::uno::Any & )
{
    switch( nObjType )
    {
    case OBJECT_DDE_EXTERN:
        if( pImplData->DDEType.pItem )
            pImplData->DDEType.pItem->Notify();
        break;
    }
    return SUCCESS;
}

void SvBaseLink::Edit( vcl::Window* pParent, const Link<SvBaseLink&,void>& rEndEditHdl )
{
    pImpl->m_pParentWin = pParent;
    pImpl->m_aEndEditLink = rEndEditHdl;
    pImpl->m_bIsConnect = xObj.Is();
    if( !pImpl->m_bIsConnect )
        GetRealObject_( xObj.Is() );

    bool bAsync = false;
    Link<const OUString&, void> aLink = LINK( this, SvBaseLink, EndEditHdl );

    if( OBJECT_CLIENT_SO & nObjType && pImplData->ClientType.bIntrnlLnk )
    {
        if( pImpl->m_pLinkMgr )
        {
            SvLinkSourceRef ref = sfx2::LinkManager::CreateObj( this );
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
        pImpl->m_aEndEditLink.Call( *this );
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
            sfx2::LinkManager::GetDisplayNames( this, &sApp, &sTopic, &sItem );
            if( nObjType == OBJECT_CLIENT_DDE )
            {
                sError = SFX2_RESSTR(STR_DDE_ERROR);

                sal_Int32 nFndPos = sError.indexOf( "%1" );
                if( -1 != nFndPos )
                {
                    sError = sError.replaceAt( nFndPos, 2, sApp );
                    nFndPos = nFndPos + sApp.getLength();

                    if( -1 != ( nFndPos = sError.indexOf( "%2", nFndPos )))
                    {
                        sError = sError.replaceAt( nFndPos, 2, sTopic );
                        nFndPos = nFndPos + sTopic.getLength();

                        if( -1 != ( nFndPos = sError.indexOf( "%3", nFndPos )))
                            sError = sError.replaceAt( nFndPos, 2, sItem );
                    }
                }
            }
            else
                return false;

            ScopedVclPtrInstance<MessageDialog>(pImpl->m_pParentWin, sError)->Execute();
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
            FileDialogFlags::Insert, rFactory);
    return *pImpl->m_pFileDlg;
}

ImplDdeItem::~ImplDdeItem()
{
    bIsInDTOR = true;
    // So that no-one gets the idea to delete the pointer when Disconnecting!
    tools::SvRef<SvBaseLink> aRef( pLink );
    aRef->Disconnect();
}

DdeData* ImplDdeItem::Get( SotClipboardFormatId nFormat )
{
    if( pLink->GetObj() )
    {
        // is it still valid?
        if( bIsValidData && nFormat == aData.GetFormat() )
            return &aData;

        Any aValue;
        OUString sMimeType( SotExchange::GetFormatMimeType( nFormat ));
        if( pLink->GetObj()->GetData( aValue, sMimeType ) )
        {
            if( aValue >>= aSeq )
            {
                aData = DdeData( aSeq.getConstArray(), aSeq.getLength(), nFormat );

                bIsValidData = true;
                return &aData;
            }
        }
    }
    aSeq.realloc( 0 );
    bIsValidData = false;
    return nullptr;
}


bool ImplDdeItem::Put( const DdeData*  )
{
    OSL_FAIL( "ImplDdeItem::Put not implemented" );
    return false;
}


void ImplDdeItem::AdviseLoop( bool bOpen )
{
    // Connection is closed, so also unsubscribe link
    if( pLink->GetObj() )
    {
        if( bOpen )
        {
            // A connection is re-established
            if( OBJECT_DDE_EXTERN == pLink->GetObjType() )
            {
                pLink->GetObj()->AddDataAdvise( pLink, "text/plain;charset=utf-16",  ADVISEMODE_NODATA );
                pLink->GetObj()->AddConnectAdvise( pLink );
            }
        }
        else
        {
            // So that no-one gets the idea to delete the pointer
            // when Disconnecting!
            tools::SvRef<SvBaseLink> aRef( pLink );
            aRef->Disconnect();
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
