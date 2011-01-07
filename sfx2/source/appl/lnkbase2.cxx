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
#include "precompiled_sfx2.hxx"


#include <sfx2/lnkbase.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/svapp.hxx>
#include "app.hrc"
#include "sfx2/sfxresid.hxx"
#include <sfx2/filedlghelper.hxx>
#include <tools/debug.hxx>
#include <svl/svdde.hxx>

using namespace ::com::sun::star::uno;

namespace sfx2
{

TYPEINIT0( SvBaseLink )

static DdeTopic* FindTopic( const String &, sal_uInt16* = 0 );

class  ImplDdeItem;

struct BaseLink_Impl
{
    Link                m_aEndEditLink;
    LinkManager*      m_pLinkMgr;
    Window*             m_pParentWin;
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

// nur fuer die interne Verwaltung
struct ImplBaseLinkData
{
    struct tClientType
    {
        // gilt fuer alle Links
        sal_uIntPtr             nCntntType; // Update Format
        // nicht Ole-Links
        sal_Bool            bIntrnlLnk; // ist es ein interner Link
        sal_uInt16          nUpdateMode;// UpdateMode
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
        ClientType.bIntrnlLnk = sal_False;
        ClientType.nUpdateMode = 0;
        DDEType.pItem = NULL;
    }
};


class ImplDdeItem : public DdeGetPutItem
{
    SvBaseLink* pLink;
    DdeData aData;
    Sequence< sal_Int8 > aSeq;          // Datacontainer for DdeData !!!
    sal_Bool bIsValidData : 1;
    sal_Bool bIsInDTOR : 1;
public:
    ImplDdeItem( SvBaseLink& rLink, const String& rStr )
        : DdeGetPutItem( rStr ), pLink( &rLink ), bIsValidData( sal_False ),
        bIsInDTOR( sal_False )
    {}
    virtual ~ImplDdeItem();

    virtual DdeData* Get( sal_uIntPtr );
    virtual sal_Bool Put( const DdeData* );
    virtual void AdviseLoop( sal_Bool );

    void Notify()
    {
        bIsValidData = sal_False;
        DdeGetPutItem::NotifyClient();
    }

    sal_Bool IsInDTOR() const { return bIsInDTOR; }
};


/************************************************************************
|*    SvBaseLink::SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

SvBaseLink::SvBaseLink()
{
    pImpl = new BaseLink_Impl();
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = sal_True;
    bWasLastEditOK = sal_False;
}

/************************************************************************
|*    SvBaseLink::SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

SvBaseLink::SvBaseLink( sal_uInt16 nUpdateMode, sal_uIntPtr nContentType )
{
    pImpl = new BaseLink_Impl();
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = sal_True;
    bWasLastEditOK = sal_False;

    // falls es ein Ole-Link wird,
    pImplData->ClientType.nUpdateMode = nUpdateMode;
    pImplData->ClientType.nCntntType = nContentType;
    pImplData->ClientType.bIntrnlLnk = sal_False;
}

/************************************************************************
|*    SvBaseLink::SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

SvBaseLink::SvBaseLink( const String& rLinkName, sal_uInt16 nObjectType, SvLinkSource* pObj )
{
    bVisible = bSynchron = bUseCache = sal_True;
    bWasLastEditOK = sal_False;
    aLinkName = rLinkName;
    pImplData = new ImplBaseLinkData;
    nObjType = nObjectType;

    if( !pObj )
    {
        DBG_ASSERT( pObj, "Wo ist mein zu linkendes Object" );
        return;
    }

    if( OBJECT_DDE_EXTERN == nObjType )
    {
        sal_uInt16 nItemStt = 0;
        DdeTopic* pTopic = FindTopic( aLinkName, &nItemStt );
        if( pTopic )
        {
            // dann haben wir alles zusammen
            // MM hat gefummelt ???
            // MM_TODO wie kriege ich den Namen
            String aStr = aLinkName; // xLinkName->GetDisplayName();
            aStr = aStr.Copy( nItemStt );
            pImplData->DDEType.pItem = new ImplDdeItem( *this, aStr );
            pTopic->InsertItem( pImplData->DDEType.pItem );

            // dann koennen wir uns auch das Advise merken
            xObj = pObj;
        }
    }
    else if( pObj->Connect( this ) )
        xObj = pObj;
}

/************************************************************************
|*    SvBaseLink::~SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

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

IMPL_LINK( SvBaseLink, EndEditHdl, String*, _pNewName )
{
    String sNewName;
    if ( _pNewName )
        sNewName = *_pNewName;
    if ( !ExecuteEdit( sNewName ) )
        sNewName.Erase();
    bWasLastEditOK = ( sNewName.Len() > 0 );
    if ( pImpl->m_aEndEditLink.IsSet() )
        pImpl->m_aEndEditLink.Call( this );
    return 0;
}

/************************************************************************
|*    SvBaseLink::SetObjType()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetObjType( sal_uInt16 nObjTypeP )
{
    DBG_ASSERT( nObjType != OBJECT_CLIENT_DDE, "type already set" );
    DBG_ASSERT( !xObj.Is(), "object exist" );

    nObjType = nObjTypeP;
}

/************************************************************************
|*    SvBaseLink::SetName()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetName( const String & rNm )
{
    aLinkName = rNm;
}

/************************************************************************
|*    SvBaseLink::GetName()
|*
|*    Beschreibung
*************************************************************************/

String SvBaseLink::GetName() const
{
    return aLinkName;
}

/************************************************************************
|*    SvBaseLink::SetObj()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetObj( SvLinkSource * pObj )
{
    DBG_ASSERT( (nObjType & OBJECT_CLIENT_SO &&
                pImplData->ClientType.bIntrnlLnk) ||
                nObjType == OBJECT_CLIENT_GRF,
                "no intern link" );
    xObj = pObj;
}

/************************************************************************
|*    SvBaseLink::SetLinkSourceName()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetLinkSourceName( const String & rLnkNm )
{
    if( aLinkName == rLnkNm )
        return;

    AddNextRef(); // sollte ueberfluessig sein
    // Alte Verbindung weg
    Disconnect();

    aLinkName = rLnkNm;

    // Neu verbinden
    _GetRealObject();
    ReleaseRef(); // sollte ueberfluessig sein
}

/************************************************************************
|*    SvBaseLink::GetLinkSourceName()
|*
|*    Beschreibung
*************************************************************************/

String  SvBaseLink::GetLinkSourceName() const
{
    return aLinkName;
}


/************************************************************************
|*    SvBaseLink::SetUpdateMode()
|*
|*    Beschreibung
*************************************************************************/

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

// --> OD 2008-06-19 #i88291#
void SvBaseLink::clearStreamToLoadFrom()
{
    m_xInputStreamToLoadFrom.clear();
    if( xObj.Is() )
    {
        xObj->clearStreamToLoadFrom();
    }
}
// <--

sal_Bool SvBaseLink::Update()
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
            // m_xInputStreamToLoadFrom = 0;
            String sMimeType( SotExchange::GetFormatMimeType(
                            pImplData->ClientType.nCntntType ));
            Any aData;

            if( xObj->GetData( aData, sMimeType ) )
            {
                DataChanged( sMimeType, aData );
                //JP 13.07.00: Bug 76817 - for manual Updates there is no
                //              need to hold the ServerObject
                if( OBJECT_CLIENT_DDE == nObjType &&
                    LINKUPDATE_ONCALL == GetUpdateMode() && xObj.Is() )
                    xObj->RemoveAllDataAdvise( this );
                return sal_True;
            }
            if( xObj.Is() )
            {
                // sollten wir asynschron sein?
                if( xObj->IsPending() )
                    return sal_True;

                // dann brauchen wir das Object auch nicht mehr
                AddNextRef();
                Disconnect();
                ReleaseRef();
            }
        }
    }
    return sal_False;
}


sal_uInt16 SvBaseLink::GetUpdateMode() const
{
    return ( OBJECT_CLIENT_SO & nObjType )
            ? pImplData->ClientType.nUpdateMode
            : sal::static_int_cast< sal_uInt16 >( LINKUPDATE_ONCALL );
}


void SvBaseLink::_GetRealObject( sal_Bool bConnect)
{
    if( !pImpl->m_pLinkMgr )
        return;

    DBG_ASSERT( !xObj.Is(), "object already exist" );

    if( OBJECT_CLIENT_DDE == nObjType )
    {
        String sServer;
        if( pImpl->m_pLinkMgr->GetDisplayNames( this, &sServer ) &&
            sServer == GetpApp()->GetAppName() )        // interner Link !!!
        {
            // damit der Internal - Link erzeugt werden kann !!!
            nObjType = OBJECT_INTERN;
            xObj = pImpl->m_pLinkMgr->CreateObj( this );

            pImplData->ClientType.bIntrnlLnk = sal_True;
            nObjType = OBJECT_CLIENT_DDE;       // damit wir wissen was es mal war !!
        }
        else
        {
            pImplData->ClientType.bIntrnlLnk = sal_False;
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

    return 0;       // alle Formate ?
}


sal_Bool SvBaseLink::SetContentType( sal_uIntPtr nType )
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        pImplData->ClientType.nCntntType = nType;
        return sal_True;
    }
    return sal_False;
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

void SvBaseLink::DataChanged( const String &, const ::com::sun::star::uno::Any & )
{
    switch( nObjType )
    {
    case OBJECT_DDE_EXTERN:
        if( pImplData->DDEType.pItem )
            pImplData->DDEType.pItem->Notify();
        break;
    }
}

void SvBaseLink::Edit( Window* pParent, const Link& rEndEditHdl )
{
    pImpl->m_pParentWin = pParent;
    pImpl->m_aEndEditLink = rEndEditHdl;
    pImpl->m_bIsConnect = ( xObj.Is() != sal_False );
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
        ExecuteEdit( String() );
        bWasLastEditOK = sal_False;
        if ( pImpl->m_aEndEditLink.IsSet() )
            pImpl->m_aEndEditLink.Call( this );
    }
}

bool SvBaseLink::ExecuteEdit( const String& _rNewName )
{
    if( _rNewName.Len() != 0 )
    {
        SetLinkSourceName( _rNewName );
        if( !Update() )
        {
            String sApp, sTopic, sItem, sError;
            pImpl->m_pLinkMgr->GetDisplayNames( this, &sApp, &sTopic, &sItem );
            if( nObjType == OBJECT_CLIENT_DDE )
            {
                sError = SfxResId( STR_DDE_ERROR );

                sal_uInt16 nFndPos = sError.Search( '%' );
                if( STRING_NOTFOUND != nFndPos )
                {
                    sError.Erase( nFndPos, 1 ).Insert( sApp, nFndPos );
                    nFndPos = nFndPos + sApp.Len();
                }
                if( STRING_NOTFOUND != ( nFndPos = sError.Search( '%', nFndPos )))
                {
                    sError.Erase( nFndPos, 1 ).Insert( sTopic, nFndPos );
                    nFndPos = nFndPos + sTopic.Len();
                }
                if( STRING_NOTFOUND != ( nFndPos = sError.Search( '%', nFndPos )))
                    sError.Erase( nFndPos, 1 ).Insert( sItem, nFndPos );
            }
            else
                return false;

            ErrorBox( pImpl->m_pParentWin, WB_OK, sError ).Execute();
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
        // beim Advise Abmelden
        xObj->RemoveAllDataAdvise( this );
}

FileDialogHelper* SvBaseLink::GetFileDialog( sal_uInt32 nFlags, const String& rFactory ) const
{
    if ( pImpl->m_pFileDlg )
        delete pImpl->m_pFileDlg;
    pImpl->m_pFileDlg = new FileDialogHelper( nFlags, rFactory );
    return pImpl->m_pFileDlg;
}

ImplDdeItem::~ImplDdeItem()
{
    bIsInDTOR = sal_True;
    // damit im Disconnect nicht jemand auf die Idee kommt, den Pointer zu
    // loeschen!!
    SvBaseLinkRef aRef( pLink );
    aRef->Disconnect();
}

DdeData* ImplDdeItem::Get( sal_uIntPtr nFormat )
{
    if( pLink->GetObj() )
    {
        // ist das noch gueltig?
        if( bIsValidData && nFormat == aData.GetFormat() )
            return &aData;

        Any aValue;
        String sMimeType( SotExchange::GetFormatMimeType( nFormat ));
        if( pLink->GetObj()->GetData( aValue, sMimeType ) )
        {
            if( aValue >>= aSeq )
            {
                aData = DdeData( (const char *)aSeq.getConstArray(), aSeq.getLength(), nFormat );

                bIsValidData = sal_True;
                return &aData;
            }
        }
    }
    aSeq.realloc( 0 );
    bIsValidData = sal_False;
    return 0;
}


sal_Bool ImplDdeItem::Put( const DdeData*  )
{
    DBG_ERROR( "ImplDdeItem::Put not implemented" );
    return sal_False;
}


void ImplDdeItem::AdviseLoop( sal_Bool bOpen )
{
    // Verbindung wird geschlossen, also Link abmelden
    if( pLink->GetObj() )
    {
        if( bOpen )
        {
            // es wird wieder eine Verbindung hergestellt
            if( OBJECT_DDE_EXTERN == pLink->GetObjType() )
            {
                pLink->GetObj()->AddDataAdvise( pLink, String::CreateFromAscii( "text/plain;charset=utf-16" ),  ADVISEMODE_NODATA );
                pLink->GetObj()->AddConnectAdvise( pLink );
            }
        }
        else
        {
            // damit im Disconnect nicht jemand auf die Idee kommt,
            // den Pointer zu loeschen!!
            SvBaseLinkRef aRef( pLink );
            aRef->Disconnect();
        }
    }
}


static DdeTopic* FindTopic( const String & rLinkName, sal_uInt16* pItemStt )
{
    if( 0 == rLinkName.Len() )
        return 0;

    String sNm( rLinkName );
    sal_uInt16 nTokenPos = 0;
    String sService( sNm.GetToken( 0, cTokenSeperator, nTokenPos ) );

    DdeServices& rSvc = DdeService::GetServices();
    for( DdeService* pService = rSvc.First(); pService;
                                                pService = rSvc.Next() )
        if( pService->GetName() == sService )
        {
            // dann suchen wir uns das Topic
            String sTopic( sNm.GetToken( 0, cTokenSeperator, nTokenPos ) );
            if( pItemStt )
                *pItemStt = nTokenPos;

            DdeTopics& rTopics = pService->GetTopics();

            for( int i = 0; i < 2; ++i )
            {
                for( DdeTopic* pTopic = rTopics.First(); pTopic;
                                                pTopic = rTopics.Next() )
                    if( pTopic->GetName() == sTopic )
                        return pTopic;

                // Topic nicht gefunden ?
                // dann versuchen wir ihn mal anzulegen
                if( i || !pService->MakeTopic( sTopic ) )
                    break;  // hat nicht geklappt, also raus
            }
            break;
        }
    return 0;
}

}
