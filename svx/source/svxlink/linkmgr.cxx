/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: linkmgr.cxx,v $
 * $Revision: 1.34 $
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
#include "precompiled_svx.hxx"
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <vcl/graph.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/urlobj.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/rectitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#ifndef _SFX_INTERNO_HXX //autogen
//#include <sfx2/interno.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localfilehelper.hxx>
#include <svl/itemset.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>

#include "linkmgr.hxx"
#include "fileobj.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include "unolingu.hxx"

class SvxInternalLink : public sfx2::SvLinkSource
{
public:
    SvxInternalLink() {}

    virtual BOOL Connect( sfx2::SvBaseLink* );
};

SvxLinkManager::SvxLinkManager( SfxObjectShell* _pPersist )
{
    SvLinkManager::SetPersist( _pPersist );
}

sfx2::SvLinkSourceRef SvxLinkManager::CreateObj( sfx2::SvBaseLink * pLink )
{
    switch( pLink->GetObjType() )
    {
    case OBJECT_CLIENT_FILE:
    case OBJECT_CLIENT_GRF:
    case OBJECT_CLIENT_OLE:
        return new SvFileObject;

    case OBJECT_INTERN:
        return new SvxInternalLink();
    }
    return SvLinkManager::CreateObj( pLink );
}


BOOL SvxLinkManager::InsertFileLink( sfx2::SvBaseLink& rLink,
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

    return SvLinkManager::InsertLink( &rLink, nFileType,
                                    sfx2::LINKUPDATE_ONCALL, &sCmd );
}

BOOL SvxLinkManager::InsertFileLink( sfx2::SvBaseLink& rLink )
{
    if( OBJECT_CLIENT_FILE == ( OBJECT_CLIENT_FILE & rLink.GetObjType() ))
        return SvLinkManager::InsertLink( &rLink, rLink.GetObjType(),
                                            sfx2::LINKUPDATE_ONCALL );
    return FALSE;
}

        // erfrage die Strings fuer den Dialog
BOOL SvxLinkManager::GetDisplayNames( const sfx2::SvBaseLink* pBaseLink,
                                        String* pType,
                                        String* pFile,
                                        String* pLink,
                                        String* pFilter ) const
{
    BOOL bRet = FALSE;
    const String sLNm( pBaseLink->GetLinkSourceName() );
    if( sLNm.Len() )
        switch( pBaseLink->GetObjType() )
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
                if( pLink )
                    *pLink = sRange;
                if( pFilter )
                    *pFilter = sLNm.Copy( nPos );

                if( pType )
                {
                    sal_uInt16 nObjType = pBaseLink->GetObjType();
                    *pType = String( ResId(
                                ( OBJECT_CLIENT_FILE == nObjType || OBJECT_CLIENT_OLE == nObjType )
                                        ? RID_SVXSTR_FILELINK
                                        : RID_SVXSTR_GRAFIKLINK
                                        , DIALOG_MGR() ));
                }
                bRet = TRUE;
            }
            break;
        default:
            bRet = SvLinkManager::GetDisplayNames( pBaseLink, pType, pFile,
                                                    pLink, pFilter );
            break;
        }
    return bRet;
}

// eine Uebertragung wird abgebrochen, also alle DownloadMedien canceln
// (ist zur Zeit nur fuer die FileLinks interressant!)
void SvxLinkManager::CancelTransfers()
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

void SvxLinkManager::SetTransferPriority( sfx2::SvBaseLink& /*rLink*/, USHORT /*nPrio*/ )
{
//  SvFileObject* pFileObj =
//      (SvFileObject*)SvFileObject::ClassFactory()->
//                                  CastAndAddRef( rLink.GetObj() );
//          OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & rLink.GetObjType()) ?
//              (SvFileObject*)rLink.GetObj() : 0;
}


    // um Status Informationen aus dem FileObject an den BaseLink zu
    // senden, gibt es eine eigene ClipBoardId. Das SvData-Object hat
    // dann die entsprechenden Informationen als String.
    // Wird zur Zeit fuer FileObject in Verbindung mit JavaScript benoetigt
    // - das braucht Informationen ueber Load/Abort/Error
ULONG SvxLinkManager::RegisterStatusInfoId()
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

BOOL SvxLinkManager::GetGraphicFromAny( const String& rMimeType,
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

        CharClass aCC( SvxCreateLocale( LANGUAGE_SYSTEM ));
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


