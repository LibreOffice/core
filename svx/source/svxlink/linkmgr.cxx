/*************************************************************************
 *
 *  $RCSfile: linkmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
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

#include <sot/dtrans.hxx>
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
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

#include "linkmgr.hxx"
#include "fileobj.hxx"
#include "fobjcach.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "unolingu.hxx"

class SvxInternalLink : public SvPseudoObject
{
public:
    SvxInternalLink() {}

    virtual BOOL Connect( SvBaseLink& );
};

class ImplCastBaseLink : public SvBaseLink
{
public:
    ImplCastBaseLink() : SvBaseLink( 0, 0 ) {}
    void SetObject( SvPseudoObject* pObj ) { SetObj( pObj ); }
};


SvxLinkManager::SvxLinkManager( SvPersist * pCacheCont )
    : SvLinkManager( pCacheCont )
{
}

SvPseudoObjectRef SvxLinkManager::CreateObj( SvBaseLink & rLink )
{
    switch( rLink.GetObjType() )
    {
    case OBJECT_CLIENT_FILE:
    case OBJECT_CLIENT_GRF:
        return new SvFileObject();

    case OBJECT_INTERN:
        return new SvxInternalLink();
    }
    return SvLinkManager::CreateObj( rLink );
}


BOOL SvxLinkManager::InsertFileLink( SvBaseLink& rLink,
                                    USHORT nFileType,
                                    const String& rFileNm,
                                    const String* pFilterNm,
                                    const String* pRange )
{
    if( !( OBJECT_CLIENT_SO & rLink.GetObjectType() ))
        return FALSE;

    String sCmd( rFileNm );
    sCmd += cTokenSeperator;
    if( pRange )
        sCmd += *pRange;
    if( pFilterNm )
        ( sCmd += cTokenSeperator ) += *pFilterNm;

    return SvLinkManager::InsertLink( rLink, nFileType,
                                    LINKUPDATE_ONCALL, &sCmd );
}

BOOL SvxLinkManager::InsertFileLink( SvBaseLink& rLink )
{
    if( OBJECT_CLIENT_FILE == ( OBJECT_CLIENT_FILE & rLink.GetObjectType() ))
        return SvLinkManager::InsertLink( rLink, rLink.GetObjectType(),
                                            LINKUPDATE_ONCALL );
    return FALSE;
}

        // erfrage die Strings fuer den Dialog
BOOL SvxLinkManager::GetDisplayNames( const SvBaseLink& rLink,
                                        String* pType,
                                        String* pFile,
                                        String* pLink,
                                        String* pFilter ) const
{
    BOOL bRet = FALSE;
    const SvLinkName* pLNm = rLink.GetLinkSourceName();
    if( pLNm )
        switch( rLink.GetObjectType() )
        {
        case OBJECT_CLIENT_FILE:
        case OBJECT_CLIENT_GRF:
            {
                USHORT nPos = 0;
                String sCmd( pLNm->GetName() );
                String sFile( sCmd.GetToken( 0, cTokenSeperator, nPos ) );
                String sRange( sCmd.GetToken( 0, cTokenSeperator, nPos ) );

                if( pFile )
                    *pFile = sFile;
                if( pLink )
                    *pLink = sRange;
                if( pFilter )
                    *pFilter = sCmd.Copy( nPos );

                if( pType )
                    *pType = String( ResId(
                                OBJECT_CLIENT_FILE == rLink.GetObjectType()
                                        ? RID_SVXSTR_FILELINK
                                        : RID_SVXSTR_GRAFIKLINK
                                        , DIALOG_MGR() ));

                bRet = TRUE;
            }
            break;
        default:
            bRet = SvLinkManager::GetDisplayNames( rLink, pType, pFile,
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
    SvBaseLink* pLnk;

    const SvBaseLinks& rLnks = GetLinks();
    for( USHORT n = rLnks.Count(); n; )
        if( 0 != ( pLnk = &(*rLnks[ --n ])) &&
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & pLnk->GetObjType()) &&
            0 != ( pFileObj = (SvFileObject*)pLnk->GetObj() ) )
//          0 != ( pFileObj = (SvFileObject*)SvFileObject::ClassFactory()->
//                                  CastAndAddRef( pLnk->GetObj() )) )
            pFileObj->CancelTransfers();
}

void SvxLinkManager::SetTransferPriority( SvBaseLink& rLink, USHORT nPrio )
{
    SvFileObject* pFileObj =
//      (SvFileObject*)SvFileObject::ClassFactory()->
//                                  CastAndAddRef( rLink.GetObj() );
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & rLink.GetObjType()) ?
                (SvFileObject*)rLink.GetObj() : 0;

    if( pFileObj )
        pFileObj->SetTransferPriority( nPrio );
}


void SvxLinkManager::PrepareReload( SvBaseLink* pLnk )
{
    FileObjCache_Impl* pCache = ::GetCache();
    SvFileObject* pFileObj;

    if( pLnk )      // einen speziellen?
    {
        if( OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & pLnk->GetObjType()) &&
            0 != ( pFileObj = (SvFileObject*)pLnk->GetObj() ) )
        {
            pCache->Remove( *pFileObj );
            pLnk->SetUseCache( FALSE );
        }
        return ;
    }

    // dann eben alle
    const SvBaseLinks& rLnks = GetLinks();
    for( USHORT n = rLnks.Count(); n; )
        if( 0 != ( pLnk = &(*rLnks[ --n ])) &&
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & pLnk->GetObjType()) &&
            0 != ( pFileObj = (SvFileObject*)pLnk->GetObj() ) )
        {
            pCache->Remove( *pFileObj );
        }
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
                    String( RTL_CONSTASCII_STRINGPARAM(
                                "StatusInfo vom SvxInternalLink" ),
                            RTL_TEXTENCODING_MS_1252 ));
    }
    return nFormat;
}


BOOL SvxInternalLink::Connect( SvBaseLink& rLink )
{
    String sTopic, sItem, sReferer;
    if( rLink.GetLinkManager() &&
        rLink.GetLinkManager()->GetDisplayNames( rLink, 0, &sTopic, &sItem ) )
    {
        // erstmal nur ueber die DocumentShells laufen und die mit dem
        // Namen heraussuchen:

        CharClass aCC( SvxCreateLocale( LANGUAGE_SYSTEM ));
        String sNm( sTopic ), sTmp;
        aCC.toLower( sNm );

        TypeId aType( TYPE(SfxObjectShell) );

        BOOL bFirst = TRUE;
        SfxObjectShell* pShell = 0;
        SvPersist* pPersist = rLink.GetLinkManager()->GetCacheContainer();
        SvInPlaceObjectRef aRef( pPersist );
        if( aRef.Is() )
        {
            // sch... SFX das gerade gelesen Doc hat noch keinen Namen und
            // steht noch nicht in der Doc. Liste
            pShell = ((SfxInPlaceObject*)&aRef)->GetObjectShell();

            if( pShell && pShell->GetMedium() )
            {
                sReferer = pShell->GetMedium()->GetName();
                if( !pShell->HasName()  )
                {
                    sTmp = sReferer;
                    INetURLObject aURL( sTmp );
                    if ( aURL.GetProtocol() == INET_PROT_FILE )
                        sTmp = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
                }
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
            if( sTmp == sNm )       // die wollen wir haben
            {
                SvPseudoObject* pNewObj = pShell->DdeCreateHotLink( sItem );
                if( pNewObj )
                {
                    ((ImplCastBaseLink&)rLink).SetObject( pNewObj );
                    pNewObj->AddDataAdvise( &rLink, rLink.GetContentType(),
                                LINKUPDATE_ONCALL == rLink.GetUpdateMode()
                                    ? ADVISEMODE_ONLYONCE
                                    : 0 );
                }
                return 0 != pNewObj;
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

    DirEntry aFileNm( GUI2FSYS( sTopic ) );
    aFileNm.ToAbs();
    if( FSYS_KIND_FILE == FileStat( aFileNm ).GetKind() )
    {
        // File vorhanden

        // dann versuche die Datei zu laden:

        SfxStringItem aName( SID_FILE_NAME, aFileNm.GetFull() );
        SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, TRUE);
//          SfxBoolItem aHidden(SID_HIDDEN, TRUE);
        // minimiert!

        SfxUInt16Item aViewStat( SID_VIEW_ZOOM_MODE, 0 );
        SfxRectangleItem aRectItem( SID_VIEW_POS_SIZE, Rectangle() );
        SfxStringItem aReferer( SID_REFERER, sReferer );

        SfxBoolItem aSilent(SID_SILENT, TRUE);
        const SfxPoolItem* pRet = SFX_DISPATCHER().Execute(
                SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                &aName, &aNewView,
                &aViewStat,&aRectItem/*aHidden*/,
                &aSilent, &aReferer, 0L );

        SfxObjectShell* pShell;
        if( pRet && pRet->ISA( SfxViewFrameItem ) &&
            ((SfxViewFrameItem*)pRet)->GetFrame() &&
            0 != ( pShell = ((SfxViewFrameItem*)pRet)
                ->GetFrame()->GetObjectShell() ) )
        {
            SvPseudoObject* pNewObj = pShell->DdeCreateHotLink( sItem );
            if( pNewObj )
            {
                ((ImplCastBaseLink&)rLink).SetObject( pNewObj );
                pNewObj->AddDataAdvise( &rLink, rLink.GetContentType(),
                            LINKUPDATE_ONCALL == rLink.GetUpdateMode()
                                ? ADVISEMODE_ONLYONCE
                                : 0 );
//JP 13.04.96: interne Links sind nicht am Closed interresiert!
//              pNewObj->AddConnectAdvise( &rLink, ADVISE_CLOSED );
            }
            return 0 != pNewObj;
        }
    }

    return FALSE;
}



