/*************************************************************************
 *
 *  $RCSfile: linkmgr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:16:11 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#pragma hdrstop

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#include <sot/dtrans.hxx>
#include <sot/formats.hxx>
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_INTERNO_HXX //autogen
#include <sfx2/interno.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <com/sun/star/datatransfer/DataFlavor.hpp>

#include "linkmgr.hxx"
#include "fileobj.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "unolingu.hxx"

class SvxInternalLink : public so3::SvLinkSource
{
public:
    SvxInternalLink() {}

    virtual BOOL Connect( so3::SvBaseLink* );
};


SvxLinkManager::SvxLinkManager( SvPersist * pPersist )
{
    SvLinkManager::SetPersist( pPersist );
}

so3::SvLinkSourceRef SvxLinkManager::CreateObj( so3::SvBaseLink * pLink )
{
    switch( pLink->GetObjType() )
    {
    case OBJECT_CLIENT_FILE:
    case OBJECT_CLIENT_GRF:
        return new SvFileObject;

    case OBJECT_INTERN:
        return new SvxInternalLink();
    }
    return SvLinkManager::CreateObj( pLink );
}


BOOL SvxLinkManager::InsertFileLink( so3::SvBaseLink& rLink,
                                    USHORT nFileType,
                                    const String& rFileNm,
                                    const String* pFilterNm,
                                    const String* pRange )
{
    if( !( OBJECT_CLIENT_SO & rLink.GetObjType() ))
        return FALSE;

    String sCmd( rFileNm );
    sCmd += ::so3::cTokenSeperator;
    if( pRange )
        sCmd += *pRange;
    if( pFilterNm )
        ( sCmd += ::so3::cTokenSeperator ) += *pFilterNm;

    return SvLinkManager::InsertLink( &rLink, nFileType,
                                    LINKUPDATE_ONCALL, &sCmd );
}

BOOL SvxLinkManager::InsertFileLink( so3::SvBaseLink& rLink )
{
    if( OBJECT_CLIENT_FILE == ( OBJECT_CLIENT_FILE & rLink.GetObjType() ))
        return SvLinkManager::InsertLink( &rLink, rLink.GetObjType(),
                                            LINKUPDATE_ONCALL );
    return FALSE;
}

        // erfrage die Strings fuer den Dialog
BOOL SvxLinkManager::GetDisplayNames( const so3::SvBaseLink* pBaseLink,
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
            {
                USHORT nPos = 0;
                String sFile( sLNm.GetToken( 0, ::so3::cTokenSeperator, nPos ) );
                String sRange( sLNm.GetToken( 0, ::so3::cTokenSeperator, nPos ) );

                if( pFile )
                    *pFile = sFile;
                if( pLink )
                    *pLink = sRange;
                if( pFilter )
                    *pFilter = sLNm.Copy( nPos );

                if( pType )
                    *pType = String( ResId(
                                OBJECT_CLIENT_FILE == pBaseLink->GetObjType()
                                        ? RID_SVXSTR_FILELINK
                                        : RID_SVXSTR_GRAFIKLINK
                                        , DIALOG_MGR() ));

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
    so3::SvBaseLink* pLnk;

    const so3::SvBaseLinks& rLnks = GetLinks();
    for( USHORT n = rLnks.Count(); n; )
        if( 0 != ( pLnk = &(*rLnks[ --n ])) &&
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & pLnk->GetObjType()) &&
            0 != ( pFileObj = (SvFileObject*)pLnk->GetObj() ) )
//          0 != ( pFileObj = (SvFileObject*)SvFileObject::ClassFactory()->
//                                  CastAndAddRef( pLnk->GetObj() )) )
            pFileObj->CancelTransfers();
}

void SvxLinkManager::SetTransferPriority( so3::SvBaseLink& rLink, USHORT nPrio )
{
    SvFileObject* pFileObj =
//      (SvFileObject*)SvFileObject::ClassFactory()->
//                                  CastAndAddRef( rLink.GetObj() );
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & rLink.GetObjType()) ?
                (SvFileObject*)rLink.GetObj() : 0;

    if( pFileObj )
        pFileObj->SetTransferPriority( nPrio );
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
        nFormat = Exchange::RegisterFormatName(
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

BOOL SvxInternalLink::Connect( so3::SvBaseLink* pLink )
{
    SfxObjectShell* pFndShell = 0;
    String sTopic, sItem, sReferer;
    if( pLink->GetLinkManager() &&
        pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sTopic, &sItem )
        && sTopic.Len() )
    {
        // erstmal nur ueber die DocumentShells laufen und die mit dem
        // Namen heraussuchen:

        CharClass aCC( SvxCreateLocale( LANGUAGE_SYSTEM ));
        String sNm( sTopic ), sNmURL( URIHelper::SmartRelToAbs( sTopic ) ),
               sTmp;
        aCC.toLower( sNm );
        aCC.toLower( sNmURL );

        TypeId aType( TYPE(SfxObjectShell) );

        BOOL bFirst = TRUE;
        SfxObjectShell* pShell = 0;
        SvInPlaceObjectRef aRef( pLink->GetLinkManager()->GetPersist());
        if( aRef.Is() )
        {
            // sch... SFX: das gerade gelesen Doc hat noch keinen Namen und
            // steht noch nicht in der Doc. Liste
            pShell = ((SfxInPlaceObject*)&aRef)->GetObjectShell();

            if( pShell && pShell->GetMedium() )
            {
                sReferer = pShell->GetMedium()->GetName();
                if( !pShell->HasName()  )
                    sTmp = sReferer;
            }
        }

        if ( !pShell )
        {
            bFirst = FALSE;
            pShell = SfxObjectShell::GetFirst( &aType );
        }

        while( pShell )
        {
            if( !sTmp.Len() )
                sTmp = pShell->GetTitle( SFX_TITLE_FULLNAME );

            aCC.toLower( sTmp );
            if( sTmp == sNm || sTmp == sNmURL )     // die wollen wir haben
            {
                pFndShell = pShell;
                break;
            }

            if( bFirst )
            {
                bFirst = FALSE;
                pShell = SfxObjectShell::GetFirst( &aType );
            }
            else
                pShell = SfxObjectShell::GetNext( *pShell, &aType );

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
        aURL.SetURL( sTopic = URIHelper::SmartRelToAbs( sTopic ) );
        if( INET_PROT_NOT_VALID != eOld ||
            INET_PROT_HTTP != aURL.GetProtocol() )
        {
            SfxStringItem aName( SID_FILE_NAME, sTopic );
            SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, TRUE);
//          SfxBoolItem aHidden(SID_HIDDEN, TRUE);
                // minimiert!

            SfxUInt16Item aViewStat( SID_VIEW_ZOOM_MODE, 0 );
            SfxRectangleItem aRectItem( SID_VIEW_POS_SIZE, Rectangle() );
            SfxStringItem aReferer( SID_REFERER, sReferer );

            SfxBoolItem aSilent(SID_SILENT, TRUE);
            const SfxPoolItem* pRet = SfxViewFrame::Current()->GetDispatcher()->
                Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                        &aName, &aNewView,
                        &aViewStat,&aRectItem/*aHidden*/,
                        &aSilent, &aReferer, 0L );

            if( pRet && pRet->ISA( SfxViewFrameItem ) &&
                ((SfxViewFrameItem*)pRet)->GetFrame() )
                pFndShell = ((SfxViewFrameItem*)pRet)->
                                                GetFrame()->GetObjectShell();
        }
    }

    BOOL bRet = FALSE;
    if( pFndShell )
    {
        so3::SvLinkSource* pNewSrc = pFndShell->DdeCreateLinkSource( sItem );
        if( pNewSrc )
        {
            bRet = TRUE;

            ::com::sun::star::datatransfer::DataFlavor aFl;
            SotExchange::GetFormatDataFlavor( pLink->GetContentType(), aFl );

            pLink->SetObj( pNewSrc );
            pNewSrc->AddDataAdvise( pLink, aFl.MimeType,
                                LINKUPDATE_ONCALL == pLink->GetUpdateMode()
                                    ? ADVISEMODE_ONLYONCE
                                    : 0 );
        }
    }
    return bRet;
}


