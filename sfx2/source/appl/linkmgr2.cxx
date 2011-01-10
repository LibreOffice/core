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

#include <sfx2/linkmgr.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <sfx2/objsh.hxx>
#include <svl/urihelper.hxx>
#include <sot/formats.hxx>
#include <tools/urlobj.hxx>
#include <sot/exchange.hxx>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/app.hxx>
#include <vcl/graph.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <unotools/localfilehelper.hxx>
#include <i18npool/mslangid.hxx>
#include <sfx2/request.hxx>

#include "fileobj.hxx"
#include "impldde.hxx"
#include "app.hrc"
#include "sfx2/sfxresid.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

namespace sfx2
{

class SvxInternalLink : public sfx2::SvLinkSource
{
public:
    SvxInternalLink() {}

    virtual BOOL Connect( sfx2::SvBaseLink* );
};


SV_IMPL_PTRARR( SvBaseLinks, SvBaseLinkRefPtr )

LinkManager::LinkManager(SfxObjectShell* p)
    : pPersist( p )
{
}


LinkManager::~LinkManager()
{
    SvBaseLinkRef** ppRef = (SvBaseLinkRef**)aLinkTbl.GetData();
    for( USHORT n = aLinkTbl.Count(); n; --n, ++ppRef )
    {
        if( (*ppRef)->Is() )
        {
            (*(*ppRef))->Disconnect();
            (*(*ppRef))->SetLinkManager( NULL );
        }
        delete *ppRef;
    }
}


/************************************************************************
|*    LinkManager::Remove()
|*
|*    Beschreibung
*************************************************************************/

void LinkManager::Remove( SvBaseLink *pLink )
{
    // keine Links doppelt einfuegen
    int bFound = FALSE;
    SvBaseLinkRef** ppRef = (SvBaseLinkRef**)aLinkTbl.GetData();
    for( USHORT n = aLinkTbl.Count(); n; --n, ++ppRef )
    {
        if( pLink == *(*ppRef) )
        {
            (*(*ppRef))->Disconnect();
            (*(*ppRef))->SetLinkManager( NULL );
            (*(*ppRef)).Clear();
            bFound = TRUE;
        }

        // falls noch leere rum stehen sollten, weg damit
        if( !(*ppRef)->Is() )
        {
            delete *ppRef;
            aLinkTbl.Remove( aLinkTbl.Count() - n, 1 );
            if( bFound )
                return ;
            --ppRef;
        }
    }
}


void LinkManager::Remove( USHORT nPos, USHORT nCnt )
{
    if( nCnt && nPos < aLinkTbl.Count() )
    {
        if( nPos + nCnt > aLinkTbl.Count() )
            nCnt = aLinkTbl.Count() - nPos;

        SvBaseLinkRef** ppRef = (SvBaseLinkRef**)aLinkTbl.GetData() + nPos;
        for( USHORT n = nCnt; n; --n, ++ppRef )
        {
            if( (*ppRef)->Is() )
            {
                (*(*ppRef))->Disconnect();
                (*(*ppRef))->SetLinkManager( NULL );
            }
            delete *ppRef;
        }
        aLinkTbl.Remove( nPos, nCnt );
    }
}


BOOL LinkManager::Insert( SvBaseLink* pLink )
{
    // keine Links doppelt einfuegen
    for( USHORT n = 0; n < aLinkTbl.Count(); ++n )
    {
        SvBaseLinkRef* pTmp = aLinkTbl[ n ];
        if( !pTmp->Is() )
            aLinkTbl.DeleteAndDestroy( n-- );

        if( pLink == *pTmp )
            return FALSE;
    }

    SvBaseLinkRef* pTmp = new SvBaseLinkRef( pLink );
    pLink->SetLinkManager( this );
    aLinkTbl.Insert( pTmp, aLinkTbl.Count() );
    return TRUE;
}


BOOL LinkManager::InsertLink( SvBaseLink * pLink,
                                USHORT nObjType,
                                USHORT nUpdateMode,
                                const String* pName )
{
    // unbedingt zuerst
    pLink->SetObjType( nObjType );
    if( pName )
        pLink->SetName( *pName );
    pLink->SetUpdateMode( nUpdateMode );
    return Insert( pLink );
}


BOOL LinkManager::InsertDDELink( SvBaseLink * pLink,
                                    const String& rServer,
                                    const String& rTopic,
                                    const String& rItem )
{
    if( !( OBJECT_CLIENT_SO & pLink->GetObjType() ) )
        return FALSE;

    String sCmd;
    ::sfx2::MakeLnkName( sCmd, &rServer, rTopic, rItem );

    pLink->SetObjType( OBJECT_CLIENT_DDE );
    pLink->SetName( sCmd );
    return Insert( pLink );
}


BOOL LinkManager::InsertDDELink( SvBaseLink * pLink )
{
    DBG_ASSERT( OBJECT_CLIENT_SO & pLink->GetObjType(), "no OBJECT_CLIENT_SO" );
    if( !( OBJECT_CLIENT_SO & pLink->GetObjType() ) )
        return FALSE;

    if( pLink->GetObjType() == OBJECT_CLIENT_SO )
        pLink->SetObjType( OBJECT_CLIENT_DDE );

    return Insert( pLink );
}


// erfrage die Strings fuer den Dialog
BOOL LinkManager::GetDisplayNames( const SvBaseLink * pLink,
                                        String* pType,
                                        String* pFile,
                                        String* pLinkStr,
                                        String* pFilter ) const
{
    BOOL bRet = FALSE;
    const String sLNm( pLink->GetLinkSourceName() );
    if( sLNm.Len() )
    {
        switch( pLink->GetObjType() )
        {
            case OBJECT_CLIENT_FILE:
            case OBJECT_CLIENT_GRF:
            case OBJECT_CLIENT_OLE:
                {
                    USHORT nPos = 0;
                    String sFile( sLNm.GetToken( 0, ::sfx2::cTokenSeperator, nPos ) );
                    String sRange( sLNm.GetToken( 0, ::sfx2::cTokenSeperator, nPos ) );

                    if( pFile )
                        *pFile = sFile;
                    if( pLinkStr )
                        *pLinkStr = sRange;
                    if( pFilter )
                        *pFilter = sLNm.Copy( nPos );

                    if( pType )
                    {
                        sal_uInt16 nObjType = pLink->GetObjType();
                        *pType = String( SfxResId(
                                    ( OBJECT_CLIENT_FILE == nObjType || OBJECT_CLIENT_OLE == nObjType )
                                            ? RID_SVXSTR_FILELINK
                                            : RID_SVXSTR_GRAFIKLINK ));
                    }
                    bRet = TRUE;
                }
                break;
            case OBJECT_CLIENT_DDE:
                {
                    USHORT nTmp = 0;
                    String sCmd( sLNm );
                    String sServer( sCmd.GetToken( 0, cTokenSeperator, nTmp ) );
                    String sTopic( sCmd.GetToken( 0, cTokenSeperator, nTmp ) );

                    if( pType )
                        *pType = sServer;
                    if( pFile )
                        *pFile = sTopic;
                    if( pLinkStr )
                        *pLinkStr = sCmd.Copy( nTmp );
                    bRet = TRUE;
                }
                break;
            default:
                break;
        }
    }

    return bRet;
}


void LinkManager::UpdateAllLinks(
    BOOL bAskUpdate,
    BOOL /*bCallErrHdl*/,
    BOOL bUpdateGrfLinks,
    Window* pParentWin )
{
    SvStringsDtor aApps, aTopics, aItems;
    String sApp, sTopic, sItem;

    // erstmal eine Kopie vom Array machen, damit sich updatende Links in
    // Links in ... nicht dazwischen funken!!
    SvPtrarr aTmpArr( 255, 50 );
    USHORT n;
    for( n = 0; n < aLinkTbl.Count(); ++n )
    {
        SvBaseLink* pLink = *aLinkTbl[ n ];
        if( !pLink )
        {
            Remove( n-- );
            continue;
        }
        aTmpArr.Insert( pLink, aTmpArr.Count() );
    }

    for( n = 0; n < aTmpArr.Count(); ++n )
    {
        SvBaseLink* pLink = (SvBaseLink*)aTmpArr[ n ];

        // suche erstmal im Array nach dem Eintrag
        USHORT nFndPos = USHRT_MAX;
        for( USHORT i = 0; i < aLinkTbl.Count(); ++i )
            if( pLink == *aLinkTbl[ i ] )
            {
                nFndPos = i;
                break;
            }

        if( USHRT_MAX == nFndPos )
            continue;                   // war noch nicht vorhanden!

        // Graphic-Links noch nicht updaten
        if( !pLink->IsVisible() ||
            ( !bUpdateGrfLinks && OBJECT_CLIENT_GRF == pLink->GetObjType() ))
            continue;

        if( bAskUpdate )
        {
            int nRet = QueryBox( pParentWin, WB_YES_NO | WB_DEF_YES, SfxResId( STR_QUERY_UPDATE_LINKS ) ).Execute();
            if( RET_YES != nRet )
                return ;        // es soll nichts geupdatet werden
            bAskUpdate = FALSE;     // einmal reicht
        }

        pLink->Update();
    }
}

/************************************************************************
|*    SvBaseLink::CreateObject()
|*
|*    Beschreibung
*************************************************************************/

SvLinkSourceRef LinkManager::CreateObj( SvBaseLink * pLink )
{
    switch( pLink->GetObjType() )
    {
        case OBJECT_CLIENT_FILE:
        case OBJECT_CLIENT_GRF:
        case OBJECT_CLIENT_OLE:
            return new SvFileObject;
        case OBJECT_INTERN:
            return new SvxInternalLink;
        case OBJECT_CLIENT_DDE:
            return new SvDDEObject;
        default:
            return SvLinkSourceRef();
       }
}

BOOL LinkManager::InsertServer( SvLinkSource* pObj )
{
    // keine doppelt einfuegen
    if( !pObj || USHRT_MAX != aServerTbl.GetPos( pObj ) )
        return FALSE;

    aServerTbl.Insert( pObj, aServerTbl.Count() );
    return TRUE;
}


void LinkManager::RemoveServer( SvLinkSource* pObj )
{
    USHORT nPos = aServerTbl.GetPos( pObj );
    if( USHRT_MAX != nPos )
        aServerTbl.Remove( nPos, 1 );
}


void MakeLnkName( String& rName, const String* pType, const String& rFile,
                    const String& rLink, const String* pFilter )
{
    if( pType )
        (rName = *pType).EraseLeadingChars().EraseTrailingChars() += cTokenSeperator;
    else if( rName.Len() )
        rName.Erase();

    ((rName += rFile).EraseLeadingChars().EraseTrailingChars() +=
        cTokenSeperator ).EraseLeadingChars().EraseTrailingChars() += rLink;
    if( pFilter )
        ((rName += cTokenSeperator ) += *pFilter).EraseLeadingChars().EraseTrailingChars();
}

BOOL LinkManager::InsertFileLink( sfx2::SvBaseLink& rLink,
                                    USHORT nFileType,
                                    const String& rFileNm,
                                    const String* pFilterNm,
                                    const String* pRange )
{
    if( !( OBJECT_CLIENT_SO & rLink.GetObjType() ))
        return FALSE;

    String sCmd( rFileNm );
    sCmd += ::sfx2::cTokenSeperator;
    if( pRange )
        sCmd += *pRange;
    if( pFilterNm )
        ( sCmd += ::sfx2::cTokenSeperator ) += *pFilterNm;

    return InsertLink( &rLink, nFileType, sfx2::LINKUPDATE_ONCALL, &sCmd );
}

BOOL LinkManager::InsertFileLink( sfx2::SvBaseLink& rLink )
{
    if( OBJECT_CLIENT_FILE == ( OBJECT_CLIENT_FILE & rLink.GetObjType() ))
        return InsertLink( &rLink, rLink.GetObjType(), sfx2::LINKUPDATE_ONCALL );
    return FALSE;
}

// eine Uebertragung wird abgebrochen, also alle DownloadMedien canceln
// (ist zur Zeit nur fuer die FileLinks interressant!)
void LinkManager::CancelTransfers()
{
    SvFileObject* pFileObj;
    sfx2::SvBaseLink* pLnk;

    const sfx2::SvBaseLinks& rLnks = GetLinks();
    for( USHORT n = rLnks.Count(); n; )
        if( 0 != ( pLnk = &(*rLnks[ --n ])) &&
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & pLnk->GetObjType()) &&
            0 != ( pFileObj = (SvFileObject*)pLnk->GetObj() ) )
//          0 != ( pFileObj = (SvFileObject*)SvFileObject::ClassFactory()->
//                                  CastAndAddRef( pLnk->GetObj() )) )
            pFileObj->CancelTransfers();
}

    // um Status Informationen aus dem FileObject an den BaseLink zu
    // senden, gibt es eine eigene ClipBoardId. Das SvData-Object hat
    // dann die entsprechenden Informationen als String.
    // Wird zur Zeit fuer FileObject in Verbindung mit JavaScript benoetigt
    // - das braucht Informationen ueber Load/Abort/Error
ULONG LinkManager::RegisterStatusInfoId()
{
    static ULONG nFormat = 0;

    if( !nFormat )
    {
// wie sieht die neue Schnittstelle aus?
//      nFormat = Exchange::RegisterFormatName( "StatusInfo vom SvxInternalLink" );
        nFormat = SotExchange::RegisterFormatName(
                    String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                                "StatusInfo vom SvxInternalLink" )));
    }
    return nFormat;
}

// ----------------------------------------------------------------------

BOOL LinkManager::GetGraphicFromAny( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue,
                                Graphic& rGrf )
{
    BOOL bRet = FALSE;
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    if( rValue.hasValue() && ( rValue >>= aSeq ) )
    {
        SvMemoryStream aMemStm( (void*)aSeq.getConstArray(), aSeq.getLength(),
                                STREAM_READ );
        aMemStm.Seek( 0 );

        switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
        {
        case SOT_FORMATSTR_ID_SVXB:
            {
                aMemStm >> rGrf;
                bRet = TRUE;
            }
            break;
        case FORMAT_GDIMETAFILE:
            {
                GDIMetaFile aMtf;
                aMtf.Read( aMemStm );
                rGrf = aMtf;
                bRet = TRUE;
            }
            break;
        case FORMAT_BITMAP:
            {
                Bitmap aBmp;
                aMemStm >> aBmp;
                rGrf = aBmp;
                bRet = TRUE;
            }
            break;
        }
    }
    return bRet;
}


// ----------------------------------------------------------------------
String lcl_DDE_RelToAbs( const String& rTopic, const String& rBaseURL )
{
    String sRet;
    INetURLObject aURL( rTopic );
    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        utl::LocalFileHelper::ConvertSystemPathToURL( rTopic, rBaseURL, sRet );
    if( !sRet.Len() )
        sRet = URIHelper::SmartRel2Abs( INetURLObject(rBaseURL), rTopic, URIHelper::GetMaybeFileHdl(), true );
    return sRet;
}

BOOL SvxInternalLink::Connect( sfx2::SvBaseLink* pLink )
{
    SfxObjectShell* pFndShell = 0;
    USHORT nUpdateMode = com::sun::star::document::UpdateDocMode::NO_UPDATE;
    String sTopic, sItem, sReferer;
    if( pLink->GetLinkManager() &&
        pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sTopic, &sItem )
        && sTopic.Len() )
    {
        // erstmal nur ueber die DocumentShells laufen und die mit dem
        // Namen heraussuchen:

        com::sun::star::lang::Locale aLocale;
        MsLangId::convertLanguageToLocale( LANGUAGE_SYSTEM, aLocale );
        CharClass aCC( aLocale );

        String sNm( sTopic ), sTmp;
        aCC.toLower( sNm );

        TypeId aType( TYPE(SfxObjectShell) );

        BOOL bFirst = TRUE;
        SfxObjectShell* pShell = pLink->GetLinkManager()->GetPersist();
        if( pShell && pShell->GetMedium() )
        {
            sReferer = pShell->GetMedium()->GetBaseURL();
            SFX_ITEMSET_ARG( pShell->GetMedium()->GetItemSet(), pItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False );
            if ( pItem )
                nUpdateMode = pItem->GetValue();
        }

        String sNmURL( lcl_DDE_RelToAbs( sTopic, sReferer ) );
        aCC.toLower( sNmURL );

        if ( !pShell )
        {
            bFirst = FALSE;
            pShell = SfxObjectShell::GetFirst( &aType, sal_False );
        }

        while( pShell )
        {
            if( !sTmp.Len() )
            {
                sTmp = pShell->GetTitle( SFX_TITLE_FULLNAME );
                sTmp = lcl_DDE_RelToAbs(sTmp, sReferer );
            }


            aCC.toLower( sTmp );
            if( sTmp == sNmURL )        // die wollen wir haben
            {
                pFndShell = pShell;
                break;
            }

            if( bFirst )
            {
                bFirst = FALSE;
                pShell = SfxObjectShell::GetFirst( &aType, sal_False );
            }
            else
                pShell = SfxObjectShell::GetNext( *pShell, &aType, sal_False );

            sTmp.Erase();
        }
    }

    // empty topics are not allowed - which document is it
    if( !sTopic.Len() )
        return FALSE;

    if( !pFndShell )
    {
        // dann versuche die Datei zu laden:
        INetURLObject aURL( sTopic );
        INetProtocol eOld = aURL.GetProtocol();
        aURL.SetURL( sTopic = lcl_DDE_RelToAbs( sTopic, sReferer ) );
        if( INET_PROT_NOT_VALID != eOld ||
            INET_PROT_HTTP != aURL.GetProtocol() )
        {
            SfxStringItem aName( SID_FILE_NAME, sTopic );
            SfxBoolItem aMinimized(SID_MINIMIZED, TRUE);
            SfxBoolItem aHidden(SID_HIDDEN, TRUE);
            SfxStringItem aTarget( SID_TARGETNAME, String::CreateFromAscii("_blank") );
            SfxStringItem aReferer( SID_REFERER, sReferer );
            SfxUInt16Item aUpdate( SID_UPDATEDOCMODE, nUpdateMode );
            SfxBoolItem aReadOnly(SID_DOC_READONLY, TRUE);

            // #i14200# (DDE-link crashes wordprocessor)
            SfxAllItemSet aArgs( SFX_APP()->GetPool() );
            aArgs.Put(aReferer);
            aArgs.Put(aTarget);
            aArgs.Put(aHidden);
            aArgs.Put(aMinimized);
            aArgs.Put(aName);
            aArgs.Put(aUpdate);
            aArgs.Put(aReadOnly);
            pFndShell = SfxObjectShell::CreateAndLoadObject( aArgs );
        }
    }

    BOOL bRet = FALSE;
    if( pFndShell )
    {
        sfx2::SvLinkSource* pNewSrc = pFndShell->DdeCreateLinkSource( sItem );
        if( pNewSrc )
        {
            bRet = TRUE;

            ::com::sun::star::datatransfer::DataFlavor aFl;
            SotExchange::GetFormatDataFlavor( pLink->GetContentType(), aFl );

            pLink->SetObj( pNewSrc );
            pNewSrc->AddDataAdvise( pLink, aFl.MimeType,
                                sfx2::LINKUPDATE_ONCALL == pLink->GetUpdateMode()
                                    ? ADVISEMODE_ONLYONCE
                                    : 0 );
        }
    }
    return bRet;
}


}



