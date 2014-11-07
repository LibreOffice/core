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

static DdeTopic* FindTopic( const OUString &, sal_uInt16* = 0 );

class  ImplDdeItem;

struct BaseLink_Impl
{
    Link                m_aEndEditLink;
    LinkManager*      m_pLinkMgr;
    vcl::Window*             m_pParentWin;
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
    struct tClientType
    {
        // applies for all links
        sal_uIntPtr nCntntType; // Update Format
        // Not Ole-Links
        bool    bIntrnlLnk;  // It is an internal link
        sal_uInt16  nUpdateMode; // UpdateMode
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
        ClientType.nCntntType = 0;
        ClientType.bIntrnlLnk = false;
        ClientType.nUpdateMode = 0;
        DDEType.pItem = NULL;
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
    ImplDdeItem( SvBaseLink& rLink, const OUString& rStr )
        : DdeGetPutItem( rStr ), pLink( &rLink ), bIsValidData( false ),
        bIsInDTOR( false )
    {}
    virtual ~ImplDdeItem();

    virtual DdeData* Get( sal_uIntPtr ) SAL_OVERRIDE;
    virtual bool     Put( const DdeData* ) SAL_OVERRIDE;
    virtual void     AdviseLoop( bool ) SAL_OVERRIDE;

    void Notify()
    {
        bIsValidData = false;
        DdeGetPutItem::NotifyClient();
    }

    bool IsInDTOR() const { return bIsInDTOR; }
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

    // It is going to be a Ole-Link,
    pImplData->ClientType.nUpdateMode = nUpdateMode;
    pImplData->ClientType.nCntntType = nContentType;
    pImplData->ClientType.bIntrnlLnk = false;
}



SvBaseLink::SvBaseLink( const OUString& rLinkName, sal_uInt16 nObjectType, SvLinkSource* pObj )
    : pImpl(0)
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
    _GetRealObject();
    ReleaseRef(); // should be superfluous
}






void SvBaseLink::SetUpdateMode( sal_uInt16 nMode )
{
    if( ( OBJECT_CLIENT_SO & nObjType ) &&
        pImplData->ClientType.nUpdateMode != nMode )
    {
        AddNextRef();
        Disconnect();

        pImplData->ClientType.nUpdateMode = nMode;
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
                            pImplData->ClientType.nCntntType ));
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
            ? pImplData->ClientType.nUpdateMode
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

            pImplData->ClientType.bIntrnlLnk = true;
            nObjType = OBJECT_CLIENT_DDE;  // so we know what it once was!
        }
        else
        {
            pImplData->ClientType.bIntrnlLnk = false;
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
        return pImplData->ClientType.nCntntType;

    return 0;  // all Formats ?
}


bool SvBaseLink::SetContentType( sal_uIntPtr nType )
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        pImplData->ClientType.nCntntType = nType;
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
    switch( nObjType )
    {
    case OBJECT_DDE_EXTERN:
        if( pImplData->DDEType.pItem )
            pImplData->DDEType.pItem->Notify();
        break;
    }
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

    if( OBJECT_CLIENT_SO & nObjType && pImplData->ClientType.bIntrnlLnk )
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

ImplDdeItem::~ImplDdeItem()
{
    bIsInDTOR = true;
    // So that no-one gets the idea to delete the pointer when Disconnecting!
    SvBaseLinkRef aRef( pLink );
    aRef->Disconnect();
}

DdeData* ImplDdeItem::Get( sal_uIntPtr nFormat )
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
                aData = DdeData( (const char *)aSeq.getConstArray(), aSeq.getLength(), nFormat );

                bIsValidData = true;
                return &aData;
            }
        }
    }
    aSeq.realloc( 0 );
    bIsValidData = false;
    return 0;
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
                pLink->GetObj()->AddDataAdvise( pLink, OUString("text/plain;charset=utf-16"),  ADVISEMODE_NODATA );
                pLink->GetObj()->AddConnectAdvise( pLink );
            }
        }
        else
        {
            // So that no-one gets the idea to delete the pointer
            // when Disconnecting!
            SvBaseLinkRef aRef( pLink );
            aRef->Disconnect();
        }
    }
}


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

            for( int i = 0; i < 2; ++i )
            {
                for( std::vector<DdeTopic*>::iterator iterTopic = rTopics.begin();
                     iterTopic != rTopics.end(); ++iterTopic )
                    if( (*iterTopic)->GetName() == sTopic )
                        return *iterTopic;

                // Topic not found?
                // then we try once to create it
                if( i || !pService->MakeTopic( sTopic ) )
                    break;  // did not work, exiting
            }
            break;
        }
    }
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
