/*************************************************************************
 *
 *  $RCSfile: docdde.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <stdlib.h>

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _LINKNAME_HXX //autogen
#include <so3/linkname.hxx>
#endif
#ifndef SO2_DECL_SVLINKNAME_DEFINED
#define SO2_DECL_SVLINKNAME_DEFINED
SO2_DECL_REF(SvLinkName)
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>          // LinkManager
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>           // fuer Server-Funktionalitaet
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>          // fuer die Bookmarks
#endif
#ifndef _SECTION_HXX
#include <section.hxx>          // fuer SwSectionFmt
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>          // fuer SwTable
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

SO2_IMPL_REF( SwServerObject )

struct _FindItem
{
    const String& rItem;
    SwBookmark* pBkmk;
    SwTableNode* pTblNd;
    SwSectionNode* pSectNd;

    _FindItem( const String& rS )
        : rItem( rS ), pBkmk( 0 ), pTblNd( 0 ), pSectNd( 0 )
    {}

    void ClearObj()
    {
        if( pBkmk )
            pBkmk->SetRefObject( 0 );
        else if( pSectNd )
            pSectNd->GetSection().SetRefObject( 0 );
        else if( pTblNd )
            pTblNd->GetTable().SetRefObject( 0 );
    }
};


BOOL lcl_FindBookmark( const SwBookmarkPtr& rpBkmk, void* pArgs )
{
    BOOL bRet = TRUE;
    String sNm( GetAppCharClass().lower( rpBkmk->GetName() ));
    if( sNm.Equals( ((_FindItem*)pArgs)->rItem ) )
    {
        ((_FindItem*)pArgs)->pBkmk = rpBkmk;
        bRet = FALSE;
    }

    return bRet;
}



BOOL lcl_FindSection( const SwSectionFmtPtr& rpSectFmt, void* pArgs )
{
    SwSection* pSect = rpSectFmt->GetSection();
    if( pSect )
    {
        String sNm( GetAppCharClass().lower( pSect->GetName() ));
        if( sNm.Equals( ((_FindItem*)pArgs)->rItem ))
        {
            // gefunden, als erfrage die Daten
            const SwNodeIndex* pIdx;
            if( 0 != (pIdx = rpSectFmt->GetCntnt().GetCntntIdx() ) &&
                &rpSectFmt->GetDoc()->GetNodes() == &pIdx->GetNodes() )
            {
                // eine Tabelle im normalen NodesArr
                ((_FindItem*)pArgs)->pSectNd = pIdx->GetNode().GetSectionNode();
                return FALSE;
            }
//nein!!            // sollte der Namen schon passen, der Rest aber nicht, dann haben wir
            // sie nicht. Die Namen sind immer eindeutig.
        }
    }
    return TRUE;        // dann weiter
}



BOOL lcl_FindTable( const SwFrmFmtPtr& rpTableFmt, void* pArgs )
{
    String sNm( GetAppCharClass().lower( rpTableFmt->GetName() ));
    if( sNm.Equals( ((_FindItem*)pArgs)->rItem ))
    {
        SwTable* pTmpTbl;
        SwTableBox* pFBox;
        if( 0 != ( pTmpTbl = SwTable::FindTable( rpTableFmt ) ) &&
            0 != ( pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            &rpTableFmt->GetDoc()->GetNodes() == &pFBox->GetSttNd()->GetNodes() )
        {
            // eine Tabelle im normalen NodesArr
            ((_FindItem*)pArgs)->pTblNd = (SwTableNode*)
                                        pFBox->GetSttNd()->FindTableNode();
            return FALSE;
        }
//nein!     // sollte der Namen schon passen, der Rest aber nicht, dann haben wir
        // sie nicht. Die Namen sind immer eindeutig.
    }
    return TRUE;        // dann weiter
}



BOOL SwDoc::GetData( const String& rItem, SvData& rData ) const
{
    // haben wir ueberhaupt das Item vorraetig?
    String sItem( GetAppCharClass().lower( rItem ));
    _FindItem aPara( sItem );
    ((SwBookmarks&)*pBookmarkTbl).ForEach( 0, pBookmarkTbl->Count(),
                                            lcl_FindBookmark, &aPara );
    if( aPara.pBkmk )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pBkmk ).GetData( &rData );
    }

    ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                lcl_FindSection, &aPara );
    if( aPara.pSectNd )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pSectNd ).GetData( &rData );
    }

    ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                            lcl_FindTable, &aPara );
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).GetData( &rData );
    }

    return FALSE;
}



BOOL SwDoc::ChangeData( const String& rItem, const SvData& rData )
{
    // haben wir ueberhaupt das Item vorraetig?
    String sItem( GetAppCharClass().lower( rItem ));
    _FindItem aPara( sItem );
    pBookmarkTbl->ForEach( 0, pBookmarkTbl->Count(), lcl_FindBookmark, &aPara );
    if( aPara.pBkmk )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pBkmk ).ChangeData( (SvData&)rData );
    }

    pSectionFmtTbl->ForEach( 0, pSectionFmtTbl->Count(), lcl_FindSection, &aPara );
    if( aPara.pSectNd )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pSectNd ).ChangeData( (SvData&)rData );
    }

    pTblFrmFmtTbl->ForEach( 0, pTblFrmFmtTbl->Count(), lcl_FindTable, &aPara );
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).ChangeData( (SvData&)rData );
    }

    return FALSE;
}



SvPseudoObject* SwDoc::CreateHotLink( const String& rItem )
{
    // haben wir ueberhaupt das Item vorraetig?
    String sItem( GetAppCharClass().lower( rItem ));
    _FindItem aPara( sItem );

    SwServerObject* pObj;

    do {    // middle check Loop
        ((SwBookmarks&)*pBookmarkTbl).ForEach( 0, pBookmarkTbl->Count(),
                                                lcl_FindBookmark, &aPara );
        if( aPara.pBkmk && aPara.pBkmk->GetOtherPos() )
        {
            // gefunden, also Hotlink einrichten
            // sollten wir schon einer sein?
            if( 0 == (pObj = aPara.pBkmk->GetObject()) )
            {
                pObj = new SwServerObject( *aPara.pBkmk );
                aPara.pBkmk->SetRefObject( pObj );
            }
            else if( pObj->GetSelectorCount() )
                return pObj;
            break;
        }

        ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                    lcl_FindSection, &aPara );
        if( aPara.pSectNd )
        {
            // gefunden, also Hotlink einrichten
            // sollten wir schon einer sein?
            if( 0 == (pObj = aPara.pSectNd->GetSection().GetObject()) )
            {
                pObj = new SwServerObject( *aPara.pSectNd );
                aPara.pSectNd->GetSection().SetRefObject( pObj );
            }
            else if( pObj->GetSelectorCount() )
                return pObj;
            break;
        }

        ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                                lcl_FindTable, &aPara );
        if( aPara.pTblNd )
        {
            // gefunden, also Hotlink einrichten
            // sollten wir schon einer sein?
            if( 0 == (pObj = aPara.pTblNd->GetTable().GetObject()) )
            {
                pObj = new SwServerObject( *aPara.pTblNd );
                aPara.pTblNd->GetTable().SetRefObject( pObj );
            }
            else if( pObj->GetSelectorCount() )
                return pObj;
            break;
        }
        // bis hierhin, also nicht vorhanden
        return 0;
    } while( FALSE );

    // neu angelegt also ab in die Verwaltung
    GetLinkManager().InsertServer( pObj );
    return pObj;
}



BOOL SwDoc::SelectServerObj( const String& rStr, SwPaM*& rpPam,
                            SwNodeRange*& rpRange ) const
{
    // haben wir ueberhaupt das Item vorraetig?
    rpPam = 0;
    rpRange = 0;

    String sItem( INetURLObject::decode( rStr, INET_HEX_ESCAPE,
                                         INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));

    xub_StrLen nPos = sItem.Search( cMarkSeperator );

    const CharClass& rCC = GetAppCharClass();

    // Erweiterung fuer die Bereiche, nicht nur Bookmarks/Bereiche linken,
    // sondern auch Rahmen(Text!), Tabellen, Gliederungen:
    if( STRING_NOTFOUND != nPos )
    {
        BOOL bWeiter = FALSE;
        String sName( sItem.Copy( 0, nPos ) );
        String sCmp( sItem.Copy( nPos + 1 ));
        rCC.toLower( sItem );

        _FindItem aPara( sName );

        if( sCmp.EqualsAscii( pMarkToTable ) )
        {
            rCC.toLower( sName );
            ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                                    lcl_FindTable, &aPara );
            if( aPara.pTblNd )
            {
                rpRange = new SwNodeRange( *aPara.pTblNd, 0,
                                *aPara.pTblNd->EndOfSectionNode(), 1 );
                return TRUE;
            }
        }
        else if( sCmp.EqualsAscii( pMarkToFrame ) )
        {
            SwNodeIndex* pIdx;
            SwNode* pNd;
            const SwFlyFrmFmt* pFlyFmt = FindFlyByName( sName );
            if( pFlyFmt &&
                0 != ( pIdx = (SwNodeIndex*)pFlyFmt->GetCntnt().GetCntntIdx() ) &&
                !( pNd = &pIdx->GetNode())->IsNoTxtNode() )
            {
                rpRange = new SwNodeRange( *pNd, 1, *pNd->EndOfSectionNode() );
                return TRUE;
            }
        }
        else if( sCmp.EqualsAscii( pMarkToRegion ) )
        {
            sItem = sName;              // wird unten behandelt !
            bWeiter = TRUE;
        }
        else if( sCmp.EqualsAscii( pMarkToOutline ) )
        {
            SwPosition aPos( SwNodeIndex( (SwNodes&)GetNodes() ));
            if( GotoOutline( aPos, sName ))
            {
                SwNode* pNd = &aPos.nNode.GetNode();
                BYTE nLvl = pNd->GetTxtNode()->GetTxtColl()->GetOutlineLevel();

                const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
                USHORT nPos;
                rOutlNds.Seek_Entry( pNd, &nPos );
                rpRange = new SwNodeRange( aPos.nNode, 0, aPos.nNode );

                // dann suche jetzt noch das Ende vom Bereich
                for( ++nPos;
                        nPos < rOutlNds.Count() &&
                        nLvl < rOutlNds[ nPos ]->GetTxtNode()->
                                GetTxtColl()->GetOutlineLevel();
                    ++nPos )
                    ;       // es gibt keinen Block

                if( nPos < rOutlNds.Count() )
                    rpRange->aEnd = *rOutlNds[ nPos ];
                else
                    rpRange->aEnd = GetNodes().GetEndOfContent();
                return TRUE;
            }
        }

        if( !bWeiter )
            return FALSE;
    }

    // alte "Mechanik"
    rCC.toLower( sItem );
    _FindItem aPara( sItem );
    if( pBookmarkTbl->Count() )
    {
        ((SwBookmarks&)*pBookmarkTbl).ForEach( 0, pBookmarkTbl->Count(),
                                            lcl_FindBookmark, &aPara );
        if( aPara.pBkmk )
        {
            // gefunden, also erzeuge einen Bereich
            if( aPara.pBkmk->GetOtherPos() )
                // ein aufgespannter Bereich
                rpPam = new SwPaM( aPara.pBkmk->GetPos(),
                                    *aPara.pBkmk->GetOtherPos() );
            return 0 != rpPam;
        }
    }

    if( pSectionFmtTbl->Count() )
    {
        ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                lcl_FindSection, &aPara );
        if( aPara.pSectNd )
        {
            rpRange = new SwNodeRange( *aPara.pSectNd, 1,
                                    *aPara.pSectNd->EndOfSectionNode() );
            return TRUE;

        }
    }
    return FALSE;
}



extern "C" {
    int
#if defined( WNT )
     __cdecl
#endif
#if defined( ICC )
     _Optlink
#endif
        lcl_ServerNamesCmpNm( const void *pFirst, const void *pSecond)
    {
        const StringPtr pF = *(StringPtr*)pFirst;
        const StringPtr pS = *(StringPtr*)pSecond;
        ASSERT( pF && pS, "ungueltige Strings" );
        StringCompare eCmp = pF->CompareTo( *pS );
        return eCmp == COMPARE_EQUAL ? 0
                            : eCmp == COMPARE_LESS ? 1 : -1;
    }
}


USHORT SwDoc::GetServerObjects( SvStrings& rStrArr ) const
{
    USHORT n;
    for( n = pBookmarkTbl->Count(); n; )
    {
        SwBookmark* pBkmk = (*pBookmarkTbl)[ --n ];
        if( pBkmk->IsBookMark() && pBkmk->GetOtherPos() )
        {
            String* pNew = new String( pBkmk->GetName() );
            rStrArr.Insert( pNew, rStrArr.Count() );
        }
    }

    for( n = pSectionFmtTbl->Count(); n; )
    {
        SwSectionFmt* pFmt = (*pSectionFmtTbl)[ --n ];
        if( pFmt->IsInNodesArr() )
        {
            String* pNew = new String( pFmt->GetName() );
            rStrArr.Insert( pNew, rStrArr.Count() );
        }
    }

    // und nochmal nach Namen sortieren:
    if( 0 != ( n = rStrArr.Count() ) )
        qsort( (void*)rStrArr.GetData(), n, sizeof( StringPtr ),
                                                    lcl_ServerNamesCmpNm );

    return n;
}




