/*************************************************************************
 *
 *  $RCSfile: content.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:30 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CONTENT_HXX
#include <content.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#define NAVIPI_CXX
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _NAVICONT_HXX
#include <navicont.hxx>
#endif
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif
#ifndef _SWCONT_HXX
#include <swcont.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _NAVIPI_HRC
#include <navipi.hrc>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTIONSSUPPLIER_HPP_
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLESSUPPLIER_HPP_
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEXESSUPPLIER_HPP_
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XBOOKMARKSSUPPLIER_HPP_
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTEMBEDDEDOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAMESSUPPLIER_HPP_
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#endif


#define CTYPE_CNT   0
#define CTYPE_CTT   1

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

#define CP2S(cChar) UniString::CreateFromAscii(cChar)
#define C2S(cChar)  UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))

#define NAVI_BOOKMARK_DELIM     (sal_Unicode)1


/***************************************************************************

***************************************************************************/

typedef SwContent* SwContentPtr;
SV_DECL_PTRARR_SORT_DEL( SwContentArr, SwContentPtr, 0,4)
SV_IMPL_OP_PTRARR_SORT(SwContentArr, SwContentPtr)

sal_Bool SwContentTree::bIsInDrag = sal_False;


/***************************************************************************
    Beschreibung: Hilfsmethoden
***************************************************************************/


sal_Bool lcl_IsContent(SvLBoxEntry* pEntry)
{
    return ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId() == CTYPE_CNT;
}


sal_Bool lcl_IsContentType(SvLBoxEntry* pEntry)
{
    return ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId() == CTYPE_CTT;
}


sal_Bool lcl_FindShell(SwWrtShell* pShell)
{
    sal_Bool bFound = sal_False;
    SwView *pView = SwModule::GetFirstView();
    while (pView)
    {
        if(pShell == &pView->GetWrtShell())
        {
            bFound = sal_True;
            break;
        }
        pView = SwModule::GetNextView(pView);
    }
    return bFound;
}

/***************************************************************************
    Beschreibung: Inhalt, enthaelt Namen und Verweis auf den Inhalstyp
***************************************************************************/


SwContent::SwContent(const SwContentType* pCnt, const String& rName, long nYPos) :
    pParent(pCnt),
    sContentName(rName),
    SwTypeNumber(CTYPE_CNT),
    nYPosition(nYPos),
    bInvisible(sal_False)
{
}


sal_uInt8   SwTypeNumber::GetTypeId()
{
    return nTypeId;
}

SwTypeNumber::~SwTypeNumber()
{
}

sal_Bool SwContent::IsProtect() const
{
    return sal_False;
}

sal_Bool SwPostItContent::IsProtect() const
{
    return pFld->IsProtect();
}

sal_Bool SwURLFieldContent::IsProtect() const
{
    return pINetAttr->IsProtect();
}

SwGraphicContent::~SwGraphicContent()
{
}
SwTOXBaseContent::~SwTOXBaseContent()
{
}

/***************************************************************************
    Beschreibung:   Inhaltstyp, kennt seine Inhalte und die WrtShell
***************************************************************************/


SwContentType::SwContentType(SwWrtShell* pShell, sal_uInt16 nType, sal_uInt8 nLevel) :
    SwTypeNumber(CTYPE_CTT),
    pWrtShell(pShell),
    nContentType(nType),
    nOutlineLevel(nLevel),
    bDataValid(sal_False),
    bMemberFilled(sal_False),
    sContentTypeName(SW_RES(STR_CONTENT_TYPE_FIRST + nType)),
    sSingleContentTypeName(SW_RES(STR_CONTENT_TYPE_SINGLE_FIRST + nType)),
    pMember(0),
    bEdit(sal_False),
    bDelete(sal_True),
    nMemberCount(0)
{
    Init();
}

/***************************************************************************
    Beschreibung:   Initialisierung
***************************************************************************/


void SwContentType::Init(sal_Bool* pbInvalidateWindow)
{
    // wenn sich der MemberCount aendert ...
    sal_uInt16 nOldMemberCount = nMemberCount;
    nMemberCount = 0;
    switch(nContentType)
    {
        case CONTENT_TYPE_OUTLINE   :
        {
            sTypeToken = CP2S(pMarkToOutline);
            sal_uInt16 nOutlineCount = nMemberCount = pWrtShell->GetOutlineCnt();
            if(nOutlineLevel < MAXLEVEL)
            {
                for(sal_uInt16 j = 0; j < nOutlineCount; j++)
                {
                    if(pWrtShell->GetOutlineLevel(j) > nOutlineLevel )
                        nMemberCount --;
                }
            }
            bDelete = sal_False;
        }
        break;

        case CONTENT_TYPE_TABLE     :
            sTypeToken = CP2S(pMarkToTable);
            nMemberCount = pWrtShell->GetTblFrmFmtCount(sal_True);
            bEdit = sal_True;
        break;

        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_GRAPHIC   :
        case CONTENT_TYPE_OLE       :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            sTypeToken = CP2S(pMarkToFrame);
            if(nContentType == CONTENT_TYPE_OLE)
            {
                eType = FLYCNTTYPE_OLE;
                sTypeToken = CP2S(pMarkToOLE);
            }
            else if(nContentType == CONTENT_TYPE_GRAPHIC)
            {
                eType = FLYCNTTYPE_GRF;
                sTypeToken = CP2S(pMarkToGraphic);
            }
            nMemberCount = pWrtShell->GetFlyCount(eType);
            bEdit = sal_True;
        }
        break;
        case CONTENT_TYPE_BOOKMARK  :
        {
            sal_uInt16 nCount = nMemberCount = pWrtShell->GetBookmarkCnt(sal_True);
            sTypeToken = aEmptyStr;
            bEdit = sal_True;
        }
        break;
        case CONTENT_TYPE_REGION :
        {
            SwContentArr*   pOldMember = 0;
            sal_uInt16 nOldRegionCount = 0;
            sal_Bool bInvalidate = sal_False;
            if(!pMember)
                pMember = new SwContentArr;
            else if(pMember->Count())
            {
                pOldMember = pMember;
                nOldRegionCount = pOldMember->Count();
                pMember = new SwContentArr;
            }
            const Point aNullPt;
            nMemberCount = pWrtShell->GetSectionFmtCount();
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwSectionFmt* pFmt;
                SectionType eTmpType;
                if( (pFmt = &pWrtShell->GetSectionFmt(i))->IsInNodesArr() &&
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
                {
                    const String& rSectionName = pFmt->GetSection()->GetName();
                    sal_uInt16 nLevel = 0;
                    SwSectionFmt* pParentFmt = pFmt->GetParent();
                    while(pParentFmt)
                    {
                        nLevel++;
                        pParentFmt = pParentFmt->GetParent();
                    }

                    SwContent* pCnt = new SwRegionContent(this, rSectionName,
                            nLevel,
                            pFmt->FindLayoutRect( sal_False, &aNullPt ).Top());

                    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, 0 );
                    if( !pFmt->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    pMember->Insert(pCnt);//, pMember->Count());

                    sal_uInt16 nPos = pMember->Count() - 1;
                    if(nOldRegionCount > nPos &&
                        (pOldMember->GetObject(nPos))->IsInvisible()
                                != pCnt->IsInvisible())
                            bInvalidate = sal_True;
                }
            }
            nMemberCount = pMember->Count();
            sTypeToken = CP2S(pMarkToRegion);
            bEdit = sal_True;
            bDelete = sal_False;
            if(pOldMember)
            {
                pOldMember->DeleteAndDestroy(0, pOldMember->Count());
                delete pOldMember;
                if(pbInvalidateWindow && bInvalidate)
                    *pbInvalidateWindow = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_INDEX:
        {
            nMemberCount = pWrtShell->GetTOXCount();
            bEdit = sal_True;
            bDelete = sal_False;
        }
        break;
        case CONTENT_TYPE_REFERENCE:
        {
            nMemberCount = pWrtShell->GetRefMarks( 0 );
            bDelete = sal_False;
        }
        break;
        case CONTENT_TYPE_URLFIELD:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(pMember->Count())
                pMember->DeleteAndDestroy(0, pMember->Count());

            SwGetINetAttrs aArr;
            nMemberCount = pWrtShell->GetINetAttrs( aArr );
            for( sal_uInt16 n = 0; n < nMemberCount; ++n )
            {
                SwGetINetAttr* p = aArr[ n ];
                SwURLFieldContent* pCnt = new SwURLFieldContent(
                                    this,
                                    p->sText,
                                    INetURLObject::decode(
                                        p->rINetAttr.GetINetFmt().GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ),
                                    &p->rINetAttr,
                                    n );
                pMember->Insert( pCnt );//, n );
            }
            bEdit = sal_True;
            nOldMemberCount = nMemberCount;
            bDelete = sal_False;
        }
        break;
        case CONTENT_TYPE_POSTIT:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(pMember->Count())
                pMember->DeleteAndDestroy(0, pMember->Count());
            SwFieldType* pType = pWrtShell->GetFldType(
                                    RES_POSTITFLD, aEmptyStr);
            SwClientIter aIter( *pType );
            SwClient * pFirst = aIter.GoStart();
            while(pFirst)
            {
                if(((SwFmtFld*)pFirst)->GetTxtFld() &&
                        ((SwFmtFld*)pFirst)->IsFldInDoc())
                {
                    SwField* pField = (SwField*)((SwFmtFld*)pFirst)
                                                                    ->GetFld();

                    String sEntry = pField->GetPar2();
                    RemoveNewline(sEntry);
                    SwPostItContent* pCnt = new SwPostItContent(
                                        this,
                                        sEntry, // hier steht der Text
                                        (const SwFmtFld*)pFirst,
                                        nMemberCount);
                    pMember->Insert(pCnt);//, nMemberCount);
                    nMemberCount++;
                }
                pFirst = aIter++;
            }
            sTypeToken = aEmptyStr;
            bEdit = sal_True;
            nOldMemberCount = nMemberCount;
        }
        break;
        case CONTENT_TYPE_DRAWOBJECT:
            sTypeToken = aEmptyStr;
        break;
    }
    // ... dann koennen die Daten auch nicht mehr gueltig sein
    // abgesehen von denen, die schon korrigiert wurden, dann ist
    // nOldMemberCount doch nicht so old
    if( nOldMemberCount != nMemberCount )
        bDataValid = sal_False;
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


SwContentType::~SwContentType()
{
    delete pMember;
}

/***************************************************************************
    Beschreibung:    Inhalt liefern, dazu gfs. die Liste fuellen
***************************************************************************/


const SwContent* SwContentType::GetMember(sal_uInt16 nIndex)
{
    if(!bDataValid || !pMember)
    {
        FillMemberList();
    }
    if(nIndex < pMember->Count())
        return pMember->GetObject(nIndex);
    else
        return 0;

}


/***************************************************************************
    Beschreibung:
***************************************************************************/


void    SwContentType::Invalidate()
{
    bDataValid = sal_False;
}

/***************************************************************************
    Beschreibung: Liste der Inhalte fuellen
***************************************************************************/


void    SwContentType::FillMemberList(sal_Bool* pbLevelOrVisibiblityChanged)
{
    SwContentArr*   pOldMember = 0;
    int nOldMemberCount = -1;
    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, 0 );
    if(pMember && pbLevelOrVisibiblityChanged)
    {
        pOldMember = pMember;
        nOldMemberCount = pOldMember->Count();
        pMember = new SwContentArr;
        *pbLevelOrVisibiblityChanged = sal_False;
    }
    else if(!pMember)
        pMember = new SwContentArr;
    else if(pMember->Count())
        pMember->DeleteAndDestroy(0, pMember->Count());
    switch(nContentType)
    {
        case CONTENT_TYPE_OUTLINE   :
        {
            sal_uInt16 nOutlineCount = nMemberCount = pWrtShell->GetOutlineCnt();

            sal_uInt16 nPos = 0;
            for (sal_uInt16 i = 0; i < nOutlineCount; ++i)
            {
                const sal_Int8 nLevel = (sal_Int8)pWrtShell->GetOutlineLevel(i);
                if(nLevel >= nOutlineLevel )
                    nMemberCount--;
                else
                {
                    String aEntry(pWrtShell->GetOutlineText(i));
                    aEntry.EraseLeadingChars();
                    SwNavigationPI::CleanEntry( aEntry );
                    SwOutlineContent* pCnt = new SwOutlineContent(this, aEntry, i, nLevel,
                                                        pWrtShell->IsOutlineMovable( i ), nPos );
                    pMember->Insert(pCnt);//, nPos);
                    // bei gleicher Anzahl und vorhandenem pOldMember wird die
                    // alte mit der neuen OutlinePos verglichen
                    // cast fuer Win16
                    if(nOldMemberCount > (int)nPos &&
                        ((SwOutlineContent*)pOldMember->GetObject(nPos))->GetOutlineLevel() != nLevel)
                        *pbLevelOrVisibiblityChanged = sal_True;

                    nPos++;
                }
            }

        }
        break;

        case CONTENT_TYPE_TABLE     :
        {
            DBG_ASSERT(nMemberCount ==
                    pWrtShell->GetTblFrmFmtCount(sal_True),
                    "MemberCount differiert")
            Point aNullPt;
            nMemberCount =  pWrtShell->GetTblFrmFmtCount(sal_True);
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwFrmFmt& rTblFmt = pWrtShell->GetTblFrmFmt(i, sal_True);
                String sTblName( rTblFmt.GetName() );

                SwContent* pCnt = new SwContent(this, sTblName,
                        rTblFmt.FindLayoutRect(sal_False, &aNullPt).Top() );
                if( !rTblFmt.GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                pMember->Insert(pCnt);//, i);

                if(nOldMemberCount > (int)i &&
                    (pOldMember->GetObject(i))->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibiblityChanged = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_OLE       :
        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_GRAPHIC   :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            if(nContentType == CONTENT_TYPE_OLE)
                eType = FLYCNTTYPE_OLE;
            else if(nContentType == CONTENT_TYPE_GRAPHIC)
                eType = FLYCNTTYPE_GRF;
            DBG_ASSERT(nMemberCount ==  pWrtShell->GetFlyCount(eType),
                    "MemberCount differiert");
            Point aNullPt;
            nMemberCount = pWrtShell->GetFlyCount(eType);
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwFrmFmt* pFrmFmt = pWrtShell->GetFlyNum(i,eType);
                String sFrmName = pFrmFmt->GetName();

                SwContent* pCnt;
                if(CONTENT_TYPE_GRAPHIC == nContentType)
                {
                    String sLink;
                    pWrtShell->GetGrfNms( &sLink, 0, (SwFlyFrmFmt*) pFrmFmt);
                    pCnt = new SwGraphicContent(this, sFrmName,
                                INetURLObject::decode( sLink, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ),
                                pFrmFmt->FindLayoutRect(sal_False, &aNullPt).Top());
                }
                else
                {
                    pCnt = new SwContent(this, sFrmName,
                            pFrmFmt->FindLayoutRect(sal_False, &aNullPt).Top() );
                }
                if( !pFrmFmt->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();
                pMember->Insert(pCnt);//, i);
                if(nOldMemberCount > (int)i &&
                    (pOldMember->GetObject(i))->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibiblityChanged = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_BOOKMARK  :
        {
            nMemberCount = pWrtShell->GetBookmarkCnt(sal_True);
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                SwBookmark& rBkmk = pWrtShell->GetBookmark( i, sal_True );
                    const String& rBkmName = rBkmk.GetName();
                    //nYPos von 0 -> ::com::sun::star::text::Bookmarks werden nach Alphabet sortiert
                    SwContent* pCnt = new SwContent(this, rBkmName, 0);
                    pMember->Insert(pCnt);//, pMember->Count());
            }
        }
        break;
        case CONTENT_TYPE_REGION    :
        {
            const Point aNullPt;
            nMemberCount = pWrtShell->GetSectionFmtCount();
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwSectionFmt* pFmt;
                SectionType eTmpType;
                if( (pFmt = &pWrtShell->GetSectionFmt(i))->IsInNodesArr() &&
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
                {
                    String sSectionName = pFmt->GetSection()->GetName();

                    sal_uInt16 nLevel = 0;
                    SwSectionFmt* pParentFmt = pFmt->GetParent();
                    while(pParentFmt)
                    {
                        nLevel++;
                        pParentFmt = pParentFmt->GetParent();
                    }

                    SwContent* pCnt = new SwRegionContent(this, sSectionName,
                            nLevel,
                            pFmt->FindLayoutRect( sal_False, &aNullPt ).Top());
                    if( !pFmt->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    pMember->Insert(pCnt);//, pMember->Count());

                    sal_uInt16 nPos = pMember->Count() - 1;
                    if(nOldMemberCount > nPos &&
                        (pOldMember->GetObject(nPos))->IsInvisible()
                                != pCnt->IsInvisible())
                            *pbLevelOrVisibiblityChanged = sal_True;
                }
            }
            nMemberCount = pMember->Count();
        }
        break;
        case CONTENT_TYPE_REFERENCE:
        {
            SvStringsDtor aRefMarks;
            nMemberCount = pWrtShell->GetRefMarks( &aRefMarks );

            for(sal_uInt16 i=0; i<nMemberCount; i++)
            {
                //Referenzen nach Alphabet sortiert
                SwContent* pCnt = new SwContent(
                            this, *aRefMarks.GetObject(i), 0);
                pMember->Insert(pCnt);//, i);
            }
        }
        break;
        case CONTENT_TYPE_URLFIELD:
        {
            SwGetINetAttrs aArr;
            nMemberCount = pWrtShell->GetINetAttrs( aArr );
            for( sal_uInt16 n = 0; n < nMemberCount; ++n )
            {
                SwGetINetAttr* p = aArr[ n ];
                SwURLFieldContent* pCnt = new SwURLFieldContent(
                                    this,
                                    p->sText,
                                    INetURLObject::decode(
                                        p->rINetAttr.GetINetFmt().GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ),
                                    &p->rINetAttr,
                                    n );
                pMember->Insert( pCnt );//, n );
            }
        }
        break;
        case CONTENT_TYPE_INDEX:
        {

            sal_uInt16 nCount = nMemberCount = pWrtShell->GetTOXCount();
            for ( sal_uInt16 nTox = 0; nTox < nCount; nTox++ )
            {
                const SwTOXBase* pBase = pWrtShell->GetTOX( nTox );
                String sTOXNm( pBase->GetTOXName() );

                SwContent* pCnt = new SwTOXBaseContent(
                        this, sTOXNm, nTox, *pBase);

                if( !pBase->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                pMember->Insert( pCnt );//, nTox );
                sal_uInt16 nPos = pMember->Count() - 1;
                if(nOldMemberCount > nPos &&
                    (pOldMember->GetObject(nPos))->IsInvisible()
                            != pCnt->IsInvisible())
                        *pbLevelOrVisibiblityChanged = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_POSTIT:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(pMember->Count())
                pMember->DeleteAndDestroy(0, pMember->Count());
            SwFieldType* pType = pWrtShell->GetFldType(
                                    RES_POSTITFLD, aEmptyStr);
            SwClientIter aIter( *pType );
            SwClient * pFirst = aIter.GoStart();
            while(pFirst)
            {
                if(((SwFmtFld*)pFirst)->GetTxtFld() &&
                        ((SwFmtFld*)pFirst)->IsFldInDoc())
                {
                    SwField* pField = (SwField*)((SwFmtFld*)pFirst)
                                                                    ->GetFld();
                    String sEntry = pField->GetPar2();
                    RemoveNewline(sEntry);
                    SwPostItContent* pCnt = new SwPostItContent(
                                        this,
                                        sEntry, // hier steht der Text
                                        (const SwFmtFld*)pFirst,
                                        nMemberCount);
                    pMember->Insert(pCnt);//, nMemberCount);
                    nMemberCount++;
                }
                pFirst = aIter++;
            }
        }
        break;
        case CONTENT_TYPE_DRAWOBJECT:
        break;
    }
    bDataValid = sal_True;
    if(pOldMember)
        pOldMember->DeleteAndDestroy(0, pOldMember->Count());

}

/***************************************************************************
    Beschreibung: TreeListBox fuer Inhaltsanzeige
***************************************************************************/


SwContentTree::SwContentTree(Window* pParent, const ResId& rResId) :
        SvTreeListBox( pParent, rResId ),
        pHiddenShell(0),
        pActiveShell(0),
        pConfig(SW_MOD()->GetNavigationConfig()),
        sSpace(C2S("                    ")),
        sRemoveIdx(SW_RES(ST_REMOVE_INDEX)),
        sUpdateIdx(SW_RES(ST_UPDATE)),
        sRename(SW_RES(ST_RENAME)),
        sReadonlyIdx(SW_RES(ST_READONLY_IDX)),
        sUnprotTbl(SW_RES(ST_REMOVE_TBL_PROTECTION)),
        sInvisible(SW_RES(ST_INVISIBLE)),
        nActiveBlock(0),
        nHiddenBlock(0),
        nOutlineLevel(MAXLEVEL),
        nRootType(USHRT_MAX),
        nLastSelType(USHRT_MAX),
        bIsActive(sal_True),
        bIsConstant(sal_False),
        bIsHidden(sal_False),
        bIsInternalDrag(sal_False),
        bDocChgdInDragging(sal_False),
        bIsRoot(sal_False),
        bIsIdleClear(sal_False),
        bIsLastReadOnly(sal_False),
        bIsOutlineMoveable(sal_True),
        bViewHasChanged(sal_False),
        bIsImageListInitialized(sal_False)
{
    SetHelpId(HID_NAVIGATOR_TREELIST);

    Bitmap aRootClosed      (SW_RES(BMP_ROOT_CLOSED));
    Bitmap aRootOpened      (SW_RES(BMP_ROOT_OPENED));
    SetNodeBitmaps( aRootClosed, aRootOpened );
    SetDoubleClickHdl(LINK(this, SwContentTree, ContentDoubleClickHdl));
    SetDragDropMode(SV_DRAGDROP_APP_COPY);
    for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
    {
        aActiveContentArr[i]    = 0;
        aHiddenContentArr[i]    = 0;
    }
    for( i = 0; i < CONTEXT_COUNT; i++  )
    {
        aContextStrings[i] = SW_RESSTR(i+ST_CONTEXT_FIRST);
    }
    nActiveBlock = pConfig->GetActiveBlock();
    aUpdTimer.SetTimeoutHdl(LINK(this, SwContentTree, TimerUpdate));
    aUpdTimer.SetTimeout(1000);
    Clear();
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


SwContentTree::~SwContentTree()
{
    Clear(); // vorher gfs. Inhaltstypen loeschen
    bIsInDrag = sal_False;
}

/***************************************************************************
    Beschreibung:   Drop wird im Navigator ausgefuehrt
***************************************************************************/


sal_Bool     SwContentTree::Drop( const DropEvent& rEvt)
{
    if(bIsRoot)
    {
        return SvTreeListBox::Drop(rEvt);
    }
    else
        return bIsInDrag ? sal_False : GetParentWindow()->Drop(rEvt);
}

/***************************************************************************
    Beschreibung:   QueryDrop wird im Navigator ausgefuehrt
***************************************************************************/


sal_Bool     SwContentTree::QueryDrop( DropEvent& rEvt)
{
    if(bIsRoot)
    {
        return bIsOutlineMoveable && SvTreeListBox::QueryDrop(rEvt);
    }
    else
        return bIsInDrag ? sal_False : GetParentWindow()->QueryDrop(rEvt);
}

/***************************************************************************
    Beschreibung:   Handler fuer Dragging und ContextMenu
***************************************************************************/


void  SwContentTree::Command( const CommandEvent& rCEvt )
{
    sal_Bool bParent = sal_False;
    switch( rCEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
        {
            if(!bIsRoot || nRootType != CONTENT_TYPE_OUTLINE)
            {
                ReleaseMouse();
                StartExecuteDrag();
            }
            else
                bParent = sal_True;
        }
        break;
        case COMMAND_CONTEXTMENU:
        {
            PopupMenu aPop;
            PopupMenu aSubPop1;
            PopupMenu aSubPop2;
            PopupMenu aSubPop3;
            PopupMenu aSubPop4; // Bearbeiten

            for(sal_uInt16 i = 1; i <= MAXLEVEL; i++ )
            {
                aSubPop1.InsertItem( i + 100, String::CreateFromInt32(i));
            }
            aSubPop1.CheckItem(100 + nOutlineLevel);
            for(i=0; i < 3; i++ )
            {
                aSubPop2.InsertItem( i + 201, aContextStrings[
                        ST_HYPERLINK - ST_CONTEXT_FIRST + i]);
            }
            aSubPop2.CheckItem( 201 +
                            GetParentWindow()->GetRegionDropMode());
            //Liste der offenen Dateien einfuegen
            sal_uInt16 nId = 301;
            const SwView* pActiveView = ::GetActiveView();
            SwView *pView = SwModule::GetFirstView();
            while (pView)
            {
                String sInsert = pView->GetDocShell()->GetTitle();
                if(pView == pActiveView)
                {
                    sInsert += '(';
                    sInsert += aContextStrings[ ST_ACTIVE - ST_CONTEXT_FIRST];
                    sInsert += ')';
                }
                aSubPop3.InsertItem(nId, sInsert);
                if(bIsConstant && pActiveShell == &pView->GetWrtShell())
                    aSubPop3.CheckItem(nId);
                pView = SwModule::GetNextView(pView);
                nId++;
            }
            aSubPop3.InsertItem(nId++, aContextStrings[ST_ACTIVE_VIEW - ST_CONTEXT_FIRST]);
            if(pHiddenShell)
            {
                String sHiddenEntry = pHiddenShell->GetView().GetDocShell()->GetTitle();
                sHiddenEntry += C2S(" ( ");
                sHiddenEntry += aContextStrings[ ST_HIDDEN - ST_CONTEXT_FIRST];
                sHiddenEntry += C2S(" )");
                aSubPop3.InsertItem(nId, sHiddenEntry);
            }

            if(bIsActive)
                aSubPop3.CheckItem( --nId );
            else if(bIsHidden)
                aSubPop3.CheckItem( nId );

            aPop.InsertItem( 1, aContextStrings[
                                        ST_OUTLINE_LEVEL - ST_CONTEXT_FIRST]);
            aPop.InsertItem(2, aContextStrings[ST_DRAGMODE - ST_CONTEXT_FIRST]);
            aPop.InsertItem(3, aContextStrings[ST_DISPLAY - ST_CONTEXT_FIRST]);
            Link aSelLk = LINK(this, SwContentTree, PopupHdl );
            //jetzt noch bearbeiten
            SvLBoxEntry* pEntry;
            //Bearbeiten nur, wenn die angezeigten Inhalte aus der aktiven View kommen
            if((bIsActive || pActiveShell == pActiveView->GetWrtShellPtr())
                    && 0 != (pEntry = FirstSelected()) && lcl_IsContent(pEntry))
            {
                const SwContentType* pContType = ((SwContent*)pEntry->GetUserData())->GetParent();
                const sal_uInt16 nContentType = pContType->GetType();
                sal_Bool bReadonly = pActiveShell->GetView().GetDocShell()->IsReadOnly();
                sal_Bool bEditable = pContType->IsEditable();
                sal_Bool bDeletable = pContType->IsDeletable();
                sal_Bool bRenamable = bEditable && !bReadonly &&
                    (CONTENT_TYPE_TABLE == nContentType ||
                        CONTENT_TYPE_FRAME == nContentType ||
                        CONTENT_TYPE_GRAPHIC == nContentType ||
                        CONTENT_TYPE_OLE == nContentType ||
                        CONTENT_TYPE_BOOKMARK == nContentType ||
                        CONTENT_TYPE_REGION == nContentType||
                        CONTENT_TYPE_INDEX == nContentType);

                if(!bReadonly && (bEditable || bDeletable))
                {
                    sal_Bool bSubPop4 = sal_False;
                    if(CONTENT_TYPE_INDEX == nContentType)
                    {
                        bSubPop4 = sal_True;
                        aSubPop4.InsertItem(401, sRemoveIdx);
                        aSubPop4.InsertItem(402, sUpdateIdx);
                        aSubPop4.InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                        aSubPop4.InsertItem(405, sReadonlyIdx);

                        const SwTOXBase* pBase = ((SwTOXBaseContent*)pEntry->GetUserData())->GetTOXBase();
                        aSubPop4.CheckItem( 405, pActiveShell->IsTOXBaseReadonly(*pBase));
                        aSubPop4.InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
                    }
                    else if(CONTENT_TYPE_TABLE == nContentType && !bReadonly)
                    {
                        bSubPop4 = sal_True;
                        aSubPop4.InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                        aSubPop4.InsertItem(404, sUnprotTbl);
                        sal_Bool bFull = sal_False;
                        String sTblName = ((SwContent*)pEntry->GetUserData())->GetName();
                        sal_Bool bProt =pActiveShell->HasTblAnyProtection( &sTblName, &bFull );
                        aSubPop4.EnableItem(403, !bFull );
                        aSubPop4.EnableItem(404, bProt );
                        aSubPop4.InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
                    }
                    else if(!((SwContent*)pEntry->GetUserData())->IsProtect())
                    {
                        if(bEditable && bDeletable)
                        {
                            aSubPop4.InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                            aSubPop4.InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
                            bSubPop4 = sal_True;
                        }
                        else if(bEditable)
                            aPop.InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                        else if(bDeletable)
                            aPop.InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
                    }
                    //Rename object
                    if(bRenamable)
                    {
                        if(bSubPop4)
                            aSubPop4.InsertItem(502, sRename);
                        else
                            aPop.InsertItem(502, sRename);
                    }

                    if(bSubPop4)
                    {
                        aPop.InsertItem(4, pContType->GetSingleName());
                        aPop.SetPopupMenu(4, &aSubPop4);
                        aSubPop4.SetSelectHdl(aSelLk);
                    }
                }
            }


            aPop.SetPopupMenu( 1, &aSubPop1 );
            aPop.SetPopupMenu( 2, &aSubPop2 );
            aPop.SetPopupMenu( 3, &aSubPop3 );
            aPop.SetSelectHdl(aSelLk);
            aSubPop1.SetSelectHdl(aSelLk);
            aSubPop2.SetSelectHdl(aSelLk);
            aSubPop3.SetSelectHdl(aSelLk);

            aPop.Execute( this, rCEvt.GetMousePosPixel() );
        }
        break;
        default: bParent = sal_True;
    }
    if(bParent)
        SvTreeListBox::Command(rCEvt);
}

/***************************************************************************
    Beschreibung:   Einrueckung fuer outlines (und sections)
***************************************************************************/


long    SwContentTree::GetTabPos( SvLBoxEntry* pEntry, SvLBoxTab* pTab)
{
    sal_uInt16 nLevel = 0;
    if(lcl_IsContent(pEntry))
    {
        nLevel++;
        SwContent* pCnt = (SwContent *) pEntry->GetUserData();
        const SwContentType*    pParent;
        if(pCnt &&  0 != (pParent = pCnt->GetParent()))
        {
            if(pParent->GetType() == CONTENT_TYPE_OUTLINE)
                nLevel += ((SwOutlineContent*)pCnt)->GetOutlineLevel();
            else if(pParent->GetType() == CONTENT_TYPE_REGION)
                nLevel += ((SwRegionContent*)pCnt)->GetRegionLevel();
        }
    }
    sal_uInt16 nBasis = bIsRoot ? 0 : 5;
    return nLevel * 10 + nBasis + pTab->GetPos();  //empirisch ermittelt
}

/***************************************************************************
    Beschreibung:   Inhalte werden erst auf Anforderung in die Box eingefuegt
***************************************************************************/


void  SwContentTree::RequestingChilds( SvLBoxEntry* pParent )
{
    // ist es ein Inhaltstyp?
    if(lcl_IsContentType(pParent))
    {
        if(!pParent->HasChilds())
        {
            DBG_ASSERT(pParent->GetUserData(), "keine UserData?")
            SwContentType* pCntType = (SwContentType*)pParent->GetUserData();

            sal_uInt16 nCount = pCntType->GetMemberCount();
            for(sal_uInt16 i = 0; i < nCount; i++)
            {
                const SwContent* pCnt = pCntType->GetMember(i);
                if(pCnt)
                {
                    String sEntry = pCnt->GetName();
                    if(!sEntry.Len())
                        sEntry = sSpace;
                    SvLBoxEntry* pChild = InsertEntry(sEntry, pParent,
                            sal_False, LIST_APPEND, (void*)pCnt);
                }

            }
        }
    }
}
/***************************************************************************
    Beschreibung:   Expand - Zustand fuer Inhaltstypen merken
***************************************************************************/


sal_Bool  SwContentTree::Expand( SvLBoxEntry* pParent )
{
    if(!bIsRoot)
    {
        if(lcl_IsContentType(pParent))
        {
            SwContentType* pCntType = (SwContentType*)pParent->GetUserData();
            sal_uInt16 nOr = 1 << pCntType->GetType(); //linear -> Bitposition
            if(bIsActive || bIsConstant)
            {
                nActiveBlock |= nOr;
                pConfig->SetActiveBlock(nActiveBlock);
            }
            else
                nHiddenBlock |= nOr;
        }
    }
    return SvTreeListBox::Expand(pParent);
}
/***************************************************************************
    Beschreibung:   Collapse - Zustand fuer Inhaltstypen merken
***************************************************************************/


sal_Bool  SwContentTree::Collapse( SvLBoxEntry* pParent )
{
    sal_Bool bRet;
    if(!bIsRoot)
    {
        if(lcl_IsContentType(pParent))
        {
            SwContentType* pCntType = (SwContentType*)pParent->GetUserData();
            sal_uInt16 nAnd = 1 << pCntType->GetType();
            nAnd = ~nAnd;
            if(bIsActive || bIsConstant)
            {
                nActiveBlock &= nAnd;
                pConfig->SetActiveBlock(nActiveBlock);
            }
            else
                nHiddenBlock &= nAnd;
        }
            bRet = SvTreeListBox::Collapse(pParent);
    }
    else
        bRet = sal_False;
    return bRet;
}


/***************************************************************************
    Beschreibung:   Auch auf Doppelclick wird zunaechst nur aufgeklappt
***************************************************************************/


IMPL_LINK( SwContentTree, ContentDoubleClickHdl, SwContentTree *, EMPTYARG )
{
    SvLBoxEntry* pEntry = GetCurEntry();
    // ist es ein Inhaltstyp?
    DBG_ASSERT(pEntry, "kein aktueller Eintrag!")
    if(pEntry)
    {
        if(lcl_IsContentType(pEntry))
            RequestingChilds(pEntry);
        else if(bIsActive || bIsConstant)
        {
            if(bIsConstant)
            {
                pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
            }
            //Inhaltstyp anspringen:
            SwContent* pCnt = (SwContent*)pEntry->GetUserData();
            DBG_ASSERT( pCnt, "keine UserData")
            GotoContent(pCnt);
            if(pCnt->GetParent()->GetType() == CONTENT_TYPE_FRAME)
                pActiveShell->EnterStdMode();
        }
    }
    return 0;
}

/***************************************************************************
    Beschreibung:   Anzeigen der Datei
***************************************************************************/


void SwContentTree::Display( sal_Bool bActive )
{
    if(!bIsImageListInitialized)
    {
        aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));
        bIsImageListInitialized = sal_True;
    }
    // erst den selektierten Eintrag auslesen, um ihn spaeter evtl. wieder
    // zu selektieren -> die UserDaten sind hier nicht mehr gueltig!
    SvLBoxEntry* pOldSelEntry = FirstSelected();
    String sEntryName;  // Name des Eintrags
    sal_uInt16 nEntryRelPos = 0; // rel. Pos zu seinem Parent
    if(pOldSelEntry)
    {
        sEntryName = GetEntryText(pOldSelEntry);
        if(GetParent(pOldSelEntry))
        {
            nEntryRelPos = (sal_uInt16)(GetModel()->GetAbsPos(pOldSelEntry) - GetModel()->GetAbsPos(GetParent(pOldSelEntry)));
        }
    }
    Clear();
    SetUpdateMode( sal_False );
    if(bActive && !bIsConstant && !bIsActive)
        bIsActive = bActive;
    bIsHidden = !bActive;
    SwWrtShell* pShell = GetWrtShell();
    sal_Bool bReadOnly = pShell ? pShell->GetView().GetDocShell()->IsReadOnly() : sal_True;
    if(bReadOnly != bIsLastReadOnly)
    {
        bIsLastReadOnly = bReadOnly;
        sal_Bool bDisable =  pShell == 0 || bReadOnly;
        SwNavigationPI* pNavi = GetParentWindow();
        pNavi->aContentToolBox.EnableItem(FN_ITEM_UP , !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_ITEM_DOWN, !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_ITEM_LEFT, !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_ITEM_RIGHT, !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_SELECT_SET_AUTO_BOOKMARK, !bDisable);
    }
    if(pShell)
    {
        SvLBoxEntry* pSelEntry = 0;
        if(nRootType == USHRT_MAX)
        {
            for(sal_uInt16 nCntType = CONTENT_TYPE_OUTLINE;
                        nCntType <= CONTENT_TYPE_POSTIT; nCntType++ )
            {
                SwContentType** ppContentT = bActive ?
                                &aActiveContentArr[nCntType] :
                                    &aHiddenContentArr[nCntType];
                if(!*ppContentT)
                    (*ppContentT) = new SwContentType(pShell, nCntType, nOutlineLevel );


                String sEntry = (*ppContentT)->GetName();
                SvLBoxEntry* pEntry;
                const Image& rImage = aEntryImages.GetImage(SID_SW_START + nCntType);
                sal_Bool bChOnDemand = 0 != (*ppContentT)->GetMemberCount();
                pEntry = InsertEntry(sEntry, rImage, rImage,
                                0, bChOnDemand, LIST_APPEND, (*ppContentT));
                if(nCntType == nLastSelType)
                    pSelEntry = pEntry;
                sal_uInt16 nExpandOptions = bIsActive || bIsConstant ?
                                            nActiveBlock :
                                                nHiddenBlock;
                if(nExpandOptions & (1 << nCntType))
                {
                    Expand(pEntry);
                    if(nEntryRelPos && nCntType == nLastSelType)
                    {
                        // jetzt vielleicht noch ein Child selektieren
                        SvLBoxEntry* pChild = pEntry;
                        SvLBoxEntry* pTemp = 0;
                        sal_uInt16 nPos = 1;
                        while(0 != (pChild = Next(pChild)))
                        {
                            // der alte Text wird leicht bevorzugt
                            if(sEntryName == GetEntryText(pChild) ||
                                nPos == nEntryRelPos )
                            {
                                pSelEntry = pChild;
                                break;
                            }
                            pTemp = pChild;
                            nPos++;
                        }
                        if(!pSelEntry || lcl_IsContentType(pSelEntry))
                            pSelEntry = pTemp;
                    }

                }
            }
            if(pSelEntry)
            {
                MakeVisible(pSelEntry);
                Select(pSelEntry);
            }
        }
        else
        {
            SwContentType** ppRootContentT = bActive ?
                                &aActiveContentArr[nRootType] :
                                    &aHiddenContentArr[nRootType];
            if(!(*ppRootContentT))
                (*ppRootContentT) = new SwContentType(pShell, nRootType, nOutlineLevel );
            const Image& rImage = aEntryImages.GetImage(20000 + nRootType);
            SvLBoxEntry* pParent = InsertEntry(
                    (*ppRootContentT)->GetName(), rImage, rImage,
                        0, sal_False, LIST_APPEND, *ppRootContentT);
            SvLBoxEntry* pEntry;
            for(sal_uInt16 i = 0; i < (*ppRootContentT)->GetMemberCount(); i++ )
            {
                const SwContent* pCnt = (*ppRootContentT)->GetMember(i);
                if(pCnt)
                {
                    String sEntry = pCnt->GetName();
                    if(!sEntry.Len())
                        sEntry = sSpace;
                    pEntry = InsertEntry( sEntry, pParent,
                                sal_False, LIST_APPEND, (void*)pCnt);
                }
            }
            Expand(pParent);
            if( nRootType == CONTENT_TYPE_OUTLINE && bIsActive )
            {
                //feststellen, wo der Cursor steht
                const sal_uInt16 nActPos = pShell->GetOutlinePos(MAXLEVEL);
                SvLBoxEntry* pEntry = First();

                while( 0 != (pEntry = Next(pEntry)) )
                {
                    if(((SwOutlineContent*)pEntry->GetUserData())->GetPos() == nActPos)
                    {
                        MakeVisible(pEntry);
                        Select(pEntry);
                    }
                }

            }
            else
            {
                // jetzt vielleicht noch ein Child selektieren
                SvLBoxEntry* pChild = pParent;
                SvLBoxEntry* pTemp = 0;
                sal_uInt16 nPos = 1;
                while(0 != (pChild = Next(pChild)))
                {
                    // der alte Text wird leicht bevorzugt
                    if(sEntryName == GetEntryText(pChild) ||
                        nPos == nEntryRelPos )
                    {
                        pSelEntry = pChild;
                        break;
                    }
                    pTemp = pChild;
                    nPos++;
                }
                if(!pSelEntry)
                    pSelEntry = pTemp;
                if(pSelEntry)
                {
                    MakeVisible(pSelEntry);
                    Select(pSelEntry);
                }
            }
        }
    }
    SetUpdateMode( sal_True );

}

/***************************************************************************
    Beschreibung:   Im Clear muessen auch die ContentTypes geloescht werden
***************************************************************************/


void SwContentTree::Clear()
{
    SetUpdateMode(sal_False);
    SvTreeListBox::Clear();
    SetUpdateMode(sal_True);
}

/***************************************************************************
    Beschreibung:   Inhalt eintueten und abschicken
***************************************************************************/


void SwContentTree::StartExecuteDrag()
{
    DragServer::Clear();
    nDragMode = DRAG_MOVEABLE|DRAG_COPYABLE|DRAG_LINKABLE;
    if(!FillDragServer(nDragMode))
        return;

    Application::PostUserEvent( STATIC_LINK( this, SwContentTree, ExecDragHdl ) );

}

/***************************************************************************
    Beschreibung:
***************************************************************************/


sal_Bool SwContentTree::FillDragServer(sal_uInt16& nDragMode)
{
    SwWrtShell* pWrtShell = GetWrtShell();
    DBG_ASSERT(pWrtShell, "keine Shell!")
    SvLBoxEntry* pEntry = GetCurEntry();
    if(!pEntry || lcl_IsContentType(pEntry) || !pWrtShell)
        return sal_False;
    String sEntry;
    SwContent* pCnt = ((SwContent*)pEntry->GetUserData());

    sal_uInt16 nActType = pCnt->GetParent()->GetType();
    String sUrl;
    sal_Bool bOutline = sal_False;
    String sOutlineText;
    switch( nActType )
    {
        case CONTENT_TYPE_OUTLINE:
        {
            sal_uInt16 nPos = ((SwOutlineContent*)pCnt)->GetPos();
            DBG_ASSERT(nPos < pWrtShell->GetOutlineCnt(),
            "outlinecnt veraendert")
            const SwNumRule* pOutlRule = pWrtShell->GetOutlineNumRule();
            const SwNodeNum* pNum = pWrtShell->GetOutlineNum(nPos);
            if( pNum && pOutlRule && MAXLEVEL >= pNum->GetLevel())
                for(sal_Int8 nLevel = 0; nLevel <= pNum->GetLevel(); nLevel++)
                {
                    sal_uInt16 nVal = pNum->GetLevelVal()[nLevel];
                    nVal ++;
                    nVal -= pOutlRule->Get(nLevel).GetStart();
                    sEntry += String::CreateFromInt32( nVal );
                    sEntry += '.';
                }
            sEntry += pWrtShell->GetOutlineText(nPos, sal_False);
            sOutlineText = pWrtShell->GetOutlineText(nPos, sal_True);
            bIsOutlineMoveable = ((SwOutlineContent*)pCnt)->IsMoveable();
            bOutline = sal_True;
        }
        break;
        case CONTENT_TYPE_POSTIT:
        case CONTENT_TYPE_INDEX:
        case CONTENT_TYPE_REFERENCE :
            // koennen weder als URL noch als Bereich eingefuegt werden
        break;
        case CONTENT_TYPE_URLFIELD:
            sUrl = ((SwURLFieldContent*)pCnt)->GetURL();
        // kein break;
        case CONTENT_TYPE_OLE:
        case CONTENT_TYPE_GRAPHIC:
            if(GetParentWindow()->GetRegionDropMode() != REGION_MODE_NONE)
                break;
            else
                nDragMode &= ~(DRAG_LINKABLE|DRAG_MOVEABLE);
        default:
            sEntry = GetEntryText(pEntry);
    }

    sal_Bool bRet = sal_False;
    if(sEntry.Len())
    {
        const SwDocShell* pDocShell = pWrtShell->GetView().GetDocShell();
        if(!sUrl.Len())
        {
            if(pDocShell->HasName())
            {
                SfxMedium* pMedium = pDocShell->GetMedium();
                sUrl = pMedium->GetURLObject().GetURLNoMark();
                // nur, wenn primaer ein Link eingefuegt werden soll
                bRet = sal_True;
            }
            else if(    nActType == CONTENT_TYPE_REGION ||
                        nActType == CONTENT_TYPE_BOOKMARK )
            {
                // fuer Bereich und Textmarken ist ein Link auch ohne
                // Dateiname ins eigene Dokument erlaubt
                bRet = sal_True;
            }
            else if(bIsConstant &&
                    ( !::GetActiveView() ||
                        pActiveShell != ::GetActiveView()->GetWrtShellPtr()))
            {
                // Urls von inaktiven Views ohne Dateinamen koennen auch nicht
                // gedraggt werden
                bRet = sal_False;
            }
            else
            {
                bRet = GetParentWindow()->GetRegionDropMode() == REGION_MODE_NONE;
                nDragMode = DRAG_MOVEABLE;
            }

            const String& rToken = pCnt->GetParent()->GetTypeToken();
            sUrl += '#';
            sUrl += sEntry;
            if(rToken.Len())
            {
                sUrl += cMarkSeperator;
                sUrl += rToken;
            }
        }
        else
            bRet = sal_True;
        if(bRet)
        {
            DragServer::NewItem();
            //fuer Outlines muss in die Description der Ueberschrifttext mit der echten Nummer
            if(bOutline)
                sEntry = sOutlineText;
            {
                NaviContentBookmark aBmk( sUrl, sEntry,
                                    GetParentWindow()->GetRegionDropMode(),
                                    pDocShell);
                aBmk.CopyDragServer();
            }

            // fuer fremde DocShells muss eine INetBookmark
            // dazugeliefert werden
            if(pDocShell->HasName())
            {
                INetBookmark aBmk( sUrl, sEntry );
                aBmk.CopyDragServer();
            }
        }
    }
    return bRet;
}
/***************************************************************************
    Beschreibung:   Umschalten der Anzeige auf Root
***************************************************************************/


sal_Bool SwContentTree::ToggleToRoot()
{
    if(!bIsRoot)
    {
        SvLBoxEntry* pEntry = GetCurEntry();
        const SwContentType* pCntType;
        if(pEntry)
        {
            if(lcl_IsContentType(pEntry))
                pCntType = (SwContentType*)pEntry->GetUserData();
            else
                pCntType = ((SwContent*)pEntry->GetUserData())->GetParent();
            nRootType = pCntType->GetType();
            bIsRoot = sal_True;
            Display(bIsActive || bIsConstant);
        }
    }
    else
    {
        nRootType = USHRT_MAX;
        bIsRoot = sal_False;
        FindActiveTypeAndRemoveUserData();
        Display(bIsActive || bIsConstant);
    }
    pConfig->SetRootType( nRootType );
    GetParentWindow()->aContentToolBox.CheckItem(FN_SHOW_ROOT, bIsRoot);
    return bIsRoot;
}

/***************************************************************************
    Beschreibung:   Angezeigten Inhalt auf Gueltigkeit pruefen
***************************************************************************/


sal_Bool SwContentTree::HasContentChanged()
{
/*
    -Parallel durch das lokale Array und die Treelistbox laufen.
    -Sind die Eintraege nicht expandiert, werden sie nur im Array verworfen
    und der Contenttype wird als UserData neu gesetzt.
    - ist der Root-Modus aktiv, wird nur dieser aktualisiert,
    fuer die nicht angezeigten Inhaltstypen gilt:
        die Memberliste wird geloescht und der Membercount aktualisiert
    Wenn Inhalte ueberprueft werden, werden gleichzeitig die vorhanden
    Memberlisten aufgefuellt. Sobald ein Unterschied auftritt wird nur noch
    gefuellt und nicht mehr ueberprueft. Abschliessend wird die Box neu gefuellt.

*/

    sal_Bool bRepaint = sal_False;
    sal_Bool bInvalidate = sal_False;

    if(!bIsActive && ! bIsConstant)
    {
        for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
        {
            if(aActiveContentArr[i])
                aActiveContentArr[i]->Invalidate();
        }
    }
    else if(bIsRoot)
    {
        sal_Bool bOutline = sal_False;
        SvLBoxEntry* pEntry = First();
        if(!pEntry)
            bRepaint = sal_True;
        else
        {
            sal_uInt16 nType = ((SwContentType*)pEntry->GetUserData())->GetType();
            bOutline = nRootType == CONTENT_TYPE_OUTLINE;
            SwContentType* pArrType = aActiveContentArr[nType];
            if(!pArrType)
                bRepaint = sal_True;
            else
            {
                sal_uInt16 nSelLevel = USHRT_MAX;

                SvLBoxEntry* pFirstSel;
                if(bOutline &&
                        0 != ( pFirstSel = FirstSelected()) &&
                            lcl_IsContent(pFirstSel))
                {
                    nSelLevel = ((SwOutlineContent*)pFirstSel->GetUserData())->GetOutlineLevel();
                    SwWrtShell* pSh = GetWrtShell();
                    sal_uInt16 nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
                    bRepaint |= nOutlinePos != USHRT_MAX && pSh->GetOutlineLevel(nOutlinePos) != nSelLevel;
                }

                pArrType->Init(&bInvalidate);
                pArrType->FillMemberList();
                pEntry->SetUserData((void*)pArrType);
                if(!bRepaint)
                {
                    if(GetChildCount(pEntry) != pArrType->GetMemberCount())
                            bRepaint = sal_True;
                    else
                    {
                        sal_uInt16 nChildCount = (sal_uInt16)GetChildCount(pEntry);
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            pEntry = Next(pEntry);
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pEntry->SetUserData((void*)pCnt);
                            String sEntryText = GetEntryText(pEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == sSpace && !pCnt->GetName().Len()))
                                bRepaint = sal_True;
                        }
                    }
                }
            }
        }
        if( !bRepaint && bOutline )
        {
            //feststellen, wo der Cursor steht
            const sal_uInt16 nActPos = GetWrtShell()->GetOutlinePos(MAXLEVEL);
            SvLBoxEntry* pEntry = First();

            while( 0 != (pEntry = Next(pEntry)) )
            {
                if(((SwOutlineContent*)pEntry->GetUserData())->GetPos() == nActPos)
                {
                    if(FirstSelected() != pEntry)
                    {
                        Select(pEntry);
                        MakeVisible(pEntry);
                    }
                }
            }

        }

    }
    else
    {
        SvLBoxEntry* pEntry = First();
        while ( pEntry )
        {
            sal_Bool bNext = sal_True; // mindestens ein Next muss sein
            SwContentType* pTreeType = (SwContentType*)pEntry->GetUserData();
            sal_uInt16 nType = pTreeType->GetType();
            sal_uInt16 nTreeCount = pTreeType->GetMemberCount();
            SwContentType* pArrType = aActiveContentArr[nType];
            if(!pArrType)
                bRepaint = sal_True;
            else
            {
                pArrType->Init(&bInvalidate);
                pEntry->SetUserData((void*)pArrType);
                if(IsExpanded(pEntry))
                {
                    sal_Bool bLevelOrVisibiblityChanged = sal_False;
                    // bLevelOrVisibiblityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibiblityChanged);
                    if(bLevelOrVisibiblityChanged)
                        bInvalidate = sal_True;
                    sal_uInt16 nChildCount = (sal_uInt16)GetChildCount(pEntry);
                    if(bLevelOrVisibiblityChanged)
                        bInvalidate = sal_True;

                    if(nChildCount != pArrType->GetMemberCount())
                        bRepaint = sal_True;
                    else
                    {
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            pEntry = Next(pEntry);
                            bNext = sal_False;
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pEntry->SetUserData((void*)pCnt);
                            String sEntryText = GetEntryText(pEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == sSpace && !pCnt->GetName().Len()))
                                bRepaint = sal_True;
                        }
                    }

                }
                else if(pEntry->HasChilds())
                {
                    //war der Eintrag einmal aufgeklappt, dann muessen auch
                    // die unsichtbaren Eintraege geprueft werden.
                    // zumindest muessen die Userdaten aktualisiert werden
                    sal_Bool bLevelOrVisibiblityChanged = sal_False;
                    // bLevelOrVisibiblityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibiblityChanged);
                    sal_Bool bRemoveChildren = sal_False;
                    sal_uInt16 nChildCount = (sal_uInt16)GetChildCount(pEntry);
                    if( nChildCount != pArrType->GetMemberCount() )
                    {
                        bRemoveChildren = sal_True;
                    }
                    else
                    {
                        SvLBoxEntry* pChild = FirstChild(pEntry);
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pChild->SetUserData((void*)pCnt);
                            String sEntryText = GetEntryText(pChild);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == sSpace && !pCnt->GetName().Len()))
                                bRemoveChildren = sal_True;
                            pChild = Next(pChild);
                        }
                    }
                    if(bRemoveChildren)
                    {
                        SvLBoxEntry* pChild = FirstChild(pEntry);
                        SvLBoxEntry* pRemove = pChild;
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            pChild = Next(pRemove);
                            GetModel()->Remove(pRemove);
                            pRemove = pChild;
                        }
                    }
                    if(!nChildCount)
                    {
                        pEntry->EnableChildsOnDemand(sal_False);
                        InvalidateEntry(pEntry);
                    }

                }
                else if((nTreeCount != 0)
                            != (pArrType->GetMemberCount()!=0))
                {
                    bRepaint = sal_True;
                }
            }
            //hier muss noch der naechste Root-Entry gefunden werden
            while( pEntry && (bNext || GetParent(pEntry ) ))
            {
                pEntry = Next(pEntry);
                bNext = sal_False;
            }
        }
    }
    if(!bRepaint && bInvalidate)
        Invalidate();
    return bRepaint;
}

/***************************************************************************
    Beschreibung:   Bevor alle Daten geloescht werden, soll noch der letzte
 *                  aktive Eintrag festgestellt werden. Dann werden die
 *                  UserData geloescht
***************************************************************************/
void SwContentTree::FindActiveTypeAndRemoveUserData()
{
    SvLBoxEntry* pEntry = FirstSelected();
    if(pEntry)
    {
        // wird Clear ueber TimerUpdate gerufen, kann nur fuer die Root
        // die Gueltigkeit der UserData garantiert werden
        SvLBoxEntry* pParent;
        while(0 != (pParent = GetParent(pEntry)))
            pEntry = pParent;
        if(pEntry->GetUserData() && lcl_IsContentType(pEntry))
            nLastSelType = ((SwContentType*)pEntry->GetUserData())->GetType();
    }
//  else
//      nLastSelType = USHRT_MAX;
    pEntry = First();
    while(pEntry)
    {
        pEntry->SetUserData(0);
        pEntry = Next(pEntry);
    }
}

/***************************************************************************
    Beschreibung:   Nachdem ein File auf den Navigator gedroppt wurde,
                    wird die neue Shell gesetzt
***************************************************************************/


void SwContentTree::SetHiddenShell(SwWrtShell* pSh)
{
    pHiddenShell = pSh;
    bIsHidden = sal_True;
    bIsActive = bIsConstant = sal_False;
    FindActiveTypeAndRemoveUserData();
    for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
    {
        DELETEZ(aHiddenContentArr[i]);
    }
    Display(bIsActive);

    GetParentWindow()->UpdateListBox();
}
/***************************************************************************
    Beschreibung:   Dokumentwechsel - neue Shell setzen
***************************************************************************/


void SwContentTree::SetActiveShell(SwWrtShell* pSh)
{
    if(bIsInternalDrag)
        bDocChgdInDragging = sal_True;
    sal_Bool bClear = pActiveShell != pSh;
    if(bIsActive && bClear)
    {
        pActiveShell = pSh;
        FindActiveTypeAndRemoveUserData();
        Clear();
    }
    else if(bIsConstant)
    {
        if(!lcl_FindShell(pActiveShell))
        {
            pActiveShell = pSh;
            bIsActive = sal_True;
            bIsConstant = sal_False;
            bClear = sal_True;
        }
    }
    // nur wenn es die aktive View ist, wird das Array geloescht und
    // die Anzeige neu gefuellt
    if(bIsActive && bClear)
    {
        FindActiveTypeAndRemoveUserData();
        for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
        {
            DELETEZ(aActiveContentArr[i]);
        }
        Display(sal_True);
    }
}

/***************************************************************************
    Beschreibung:   Eine offene View als aktiv festlegen
***************************************************************************/


void SwContentTree::SetConstantShell(SwWrtShell* pSh)
{
    pActiveShell = pSh;
    bIsActive       = sal_False;
    bIsConstant     = sal_True;
    FindActiveTypeAndRemoveUserData();
    for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
    {
        DELETEZ(aActiveContentArr[i]);
    }
    Display(sal_True);
}
/***************************************************************************
    Beschreibung:   Kommandos des Navigators ausfuehren
***************************************************************************/


void SwContentTree::ExecCommand(sal_uInt16 nCmd, sal_Bool bModifier)
{
    sal_Bool nMove = sal_False;
    switch( nCmd )
    {
        case FN_ITEM_DOWN:
        case FN_ITEM_UP:   nMove = sal_True;
        case FN_ITEM_LEFT:
        case FN_ITEM_RIGHT:
        if( !GetWrtShell()->GetView().GetDocShell()->IsReadOnly() &&
                (bIsActive ||
                    (bIsConstant && pActiveShell == ::GetActiveView()->GetWrtShellPtr())))
        {
            SwWrtShell* pShell = GetWrtShell();
            sal_Int8 nActOutlineLevel = nOutlineLevel;
            sal_uInt16 nActPos = pShell->GetOutlinePos(nActOutlineLevel);
            SvLBoxEntry* pFirstEntry = FirstSelected();
            if (pFirstEntry && lcl_IsContent(pFirstEntry))
            {
                if(bIsRoot && nRootType == CONTENT_TYPE_OUTLINE ||
                    ((SwContent*)pFirstEntry->GetUserData())->GetParent()->GetType()
                                                ==  CONTENT_TYPE_OUTLINE)
                    nActPos = ((SwOutlineContent*)pFirstEntry->GetUserData())->GetPos();
            }
            if ( nActPos < USHRT_MAX &&
                    ( !nMove || pShell->IsOutlineMovable( nActPos )) )
            {
                pShell->StartAllAction();
                pShell->GotoOutline( nActPos); // Falls Textselektion != BoxSelektion
                pShell->Push();
                pShell->MakeOutlineSel( nActPos, nActPos,
                                    bModifier);
                if( nMove )
                {
                    short nDir = nCmd == FN_ITEM_UP ? -1 : 1;
                    if( !bModifier && (nDir == -1 && nActPos > 0 ||
                        nDir == 1 && nActPos < GetEntryCount() - 2 ) )
                    {
                        pShell->MoveOutlinePara( nDir );
                        //Cursor wieder an die aktuelle Position setzen
                        pShell->GotoOutline( nActPos + nDir);
                    }
                    else if(bModifier)
                    {
                        sal_uInt16 nActEndPos = nActPos;
                        SvLBoxEntry* pEntry = pFirstEntry;
                        sal_uInt16 nActLevel = ((SwOutlineContent*)
                                pFirstEntry->GetUserData())->GetOutlineLevel();
                        pEntry = Next(pEntry);
                        while( pEntry && CONTENT_TYPE_OUTLINE ==
                            ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId() )
                        {
                            if(nActLevel >= ((SwOutlineContent*)
                                pEntry->GetUserData())->GetOutlineLevel())
                                break;
                            pEntry = Next(pEntry);
                            nActEndPos++;
                        }
                        sal_uInt16 nDest;
                        if(nDir == 1)
                        {
                            //Wenn der letzte Eintrag bewegt werden soll
                            //ist Schluss
                            if(pEntry && CONTENT_TYPE_OUTLINE ==
                                ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId())
                            {
                                // pEntry zeigt jetzt auf den
                                // dem letzten sel. Eintrag folgenden E.
                                nDest = nActEndPos;
                                nDest++;
                                //hier muss der uebernaechste Eintrag
                                //gefunden werden. Die Selektion muss davor eingefuegt
                                //werden
                                while(pEntry )
                                {
                                    pEntry = Next(pEntry);
                                    // nDest++ darf nur ausgefuehrt werden,
                                    // wenn pEntry != 0
                                    if(pEntry && nDest++ &&
                                    ( nActLevel >= ((SwOutlineContent*)pEntry->GetUserData())->GetOutlineLevel()||
                                     CONTENT_TYPE_OUTLINE != ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId()))
                                    {
                                        nDest--;
                                        break;
                                    }
                                }
                                nDir = nDest - nActEndPos;
                                //wenn kein Eintrag gefunden wurde, der der Bedingung
                                //fuer das zuvor Einfuegen entspricht, muss etwas weniger
                                //geschoben werden
                            }
                            else
                                nDir = 0;
                        }
                        else
                        {
                            nDest = nActPos;
                            pEntry = pFirstEntry;
                            while(pEntry && nDest )
                            {
                                nDest--;
                                pEntry = Prev(pEntry);
                                if(pEntry &&
                                    (nActLevel >= ((SwOutlineContent*)pEntry->GetUserData())->GetOutlineLevel()||
                                    CONTENT_TYPE_OUTLINE !=
                                ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId()))
                                {
                                    break;
                                }
                            }
                            nDir = nDest - nActPos;
                        }
                        if(nDir)
                        {
                            pShell->MoveOutlinePara( nDir );
                            //Cursor wieder an die aktuelle Position setzen
                            pShell->GotoOutline( nActPos + nDir);
                        }
                    }
                }
                else
                {
                    if( pShell->IsProtectedOutlinePara() )
                        Sound::Beep(); //konnte nicht umgestuft werden
                    else
                        pShell->OutlineUpDown( nCmd == FN_ITEM_LEFT ? -1 : 1 );
                }

                pShell->ClearMark();
                pShell->Pop(sal_False); //Cursor steht jetzt wieder an der akt. Ueberschrift
                pShell->EndAllAction();
                if(aActiveContentArr[CONTENT_TYPE_OUTLINE])
                    aActiveContentArr[CONTENT_TYPE_OUTLINE]->Invalidate();
                Display(sal_True);
                if(!bIsRoot)
                {
                    const sal_uInt16 nActPos = pShell->GetOutlinePos(MAXLEVEL);
                    SvLBoxEntry* pEntry = First();

                    while( 0 != (pEntry = Next(pEntry)) && lcl_IsContent(pEntry))
                    {
                        if(((SwOutlineContent*)pEntry->GetUserData())->GetPos() == nActPos)
                        {
                            Select(pEntry);
                            MakeVisible(pEntry);
                        }
                    }
                }
            }
            else
                Sound::Beep(); //konnte nicht verschoben werden
        }
    }
}
/***************************************************************************
    Beschreibung:
***************************************************************************/


void    SwContentTree::Show()
{
    aUpdTimer.Start();
    SvTreeListBox::Show();
}

/***************************************************************************
    Beschreibung:   zusammengefaltet wird nicht geidlet
***************************************************************************/


void    SwContentTree::Hide()
{
    aUpdTimer.Stop();
    SvTreeListBox::Hide();
}

/***************************************************************************
    Beschreibung:   Kein Idle mit Focus oder waehrend des Dragging
***************************************************************************/


IMPL_LINK( SwContentTree, TimerUpdate, Timer*, EMPTYARG)
{
    // kein Update waehrend D&D
    // Viewabfrage, da der Navigator zu spaet abgeraeumt wird
    SwView* pView = ::GetActiveView();
    if( (!HasFocus() || bViewHasChanged) &&
         !bIsInDrag && !bIsInternalDrag && pView &&
         pView->GetWrtShellPtr() && !pView->GetWrtShellPtr()->ActionPend() )
    {
        bViewHasChanged = sal_False;
        bIsIdleClear = sal_False;
        SwWrtShell* pActShell = pView->GetWrtShellPtr();
        if( bIsConstant && !lcl_FindShell( pActiveShell ) )
        {
            SetActiveShell(pActShell);
            GetParentWindow()->UpdateListBox();
        }

        if(bIsActive && pActShell != GetWrtShell())
            SetActiveShell(pActShell);
        else if( (bIsActive || (bIsConstant && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            FindActiveTypeAndRemoveUserData();
            Display(sal_True);
        }
    }
    else if(!pView && bIsActive && !bIsIdleClear)
    {
        if(pActiveShell)
            SetActiveShell(0);
        Clear();
        bIsIdleClear = sal_True;
    }
    return 0;
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


DragDropMode  SwContentTree::NotifyBeginDrag( SvLBoxEntry* pEntry )
{
    DragDropMode eMode = (DragDropMode)0;
    if(bIsActive && nRootType == CONTENT_TYPE_OUTLINE &&
        GetModel()->GetAbsPos( pEntry ) > 0
        && !GetWrtShell()->GetView().GetDocShell()->IsReadOnly())
        eMode =  GetDragDropMode();
    else if(!bIsActive && GetWrtShell()->GetView().GetDocShell()->HasName())
        eMode = SV_DRAGDROP_APP_COPY;

    sal_uInt16 nDrgMode;
    FillDragServer(nDrgMode);
    bDocChgdInDragging = sal_False;
    return eMode;
}


/***************************************************************************
    Beschreibung :  Nach dem Drag wird der aktuelle Absatz m i t
                    Childs verschoben
***************************************************************************/


sal_Bool  SwContentTree::NotifyMoving( SvLBoxEntry*  pTarget,
        SvLBoxEntry*  pEntry, SvLBoxEntry*& , sal_uInt32& )
{
    if(!bDocChgdInDragging)
    {
        sal_uInt16 nTargetPos = 0;
        sal_uInt16 nSourcePos = (( SwOutlineContent* )pEntry->GetUserData())->GetPos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = USHRT_MAX;
        else
            nTargetPos = (( SwOutlineContent* )pTarget->GetUserData())->GetPos();
        if( MAXLEVEL > nOutlineLevel && // werden nicht alle Ebenen angezeigt
                        nTargetPos != USHRT_MAX)
        {
            SvLBoxEntry* pNext = Next(pTarget);
            if(pNext)
                nTargetPos = (( SwOutlineContent* )pNext->GetUserData())->GetPos() -1;
            else
                nTargetPos = GetWrtShell()->GetOutlineCnt() - 1;

        }

        DBG_ASSERT( pEntry &&
            lcl_IsContent(pEntry),"Source == 0 oder Source hat keinen Content" )
        GetParentWindow()->MoveOutline( nSourcePos,
                                    nTargetPos,
                                    sal_True);

        aActiveContentArr[CONTENT_TYPE_OUTLINE]->Invalidate();
        Display(sal_True);
    }
    //TreeListBox wird aus dem Dokument neu geladen
    return sal_False;
}
/***************************************************************************
    Beschreibung :  Nach dem Drag wird der aktuelle Absatz o h n e
                    Childs verschoben
***************************************************************************/


sal_Bool  SwContentTree::NotifyCopying( SvLBoxEntry*  pTarget,
        SvLBoxEntry*  pEntry, SvLBoxEntry*& , sal_uInt32& )
{
    if(!bDocChgdInDragging)
    {
        sal_uInt16 nTargetPos = 0;
        sal_uInt16 nSourcePos = (( SwOutlineContent* )pEntry->GetUserData())->GetPos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = USHRT_MAX;
        else
            nTargetPos = (( SwOutlineContent* )pTarget->GetUserData())->GetPos();

        if( MAXLEVEL > nOutlineLevel && // werden nicht alle Ebenen angezeigt
                        nTargetPos != USHRT_MAX)
        {
            SvLBoxEntry* pNext = Next(pTarget);
            if(pNext)
                nTargetPos = (( SwOutlineContent* )pNext->GetUserData())->GetPos() - 1;
            else
                nTargetPos = GetWrtShell()->GetOutlineCnt() - 1;

        }


        DBG_ASSERT( pEntry &&
            lcl_IsContent(pEntry),"Source == 0 oder Source hat keinen Content" )
        GetParentWindow()->MoveOutline( nSourcePos, nTargetPos, sal_False);

        //TreeListBox wird aus dem Dokument neu geladen
        aActiveContentArr[CONTENT_TYPE_OUTLINE]->Invalidate();
        Display(sal_True);
    }
    return sal_False;
}
/***************************************************************************
    Beschreibung:   Kein Drop vor den ersten Eintrag - es ist ein SwContentType
***************************************************************************/


sal_Bool  SwContentTree::NotifyQueryDrop( SvLBoxEntry* pEntry)
{
    return pEntry != 0;
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


void  SwContentTree::BeginDrag( const Point& rPt)
{
    bIsInternalDrag = sal_True;
    SvTreeListBox::BeginDrag(rPt);
}
/***************************************************************************
    Beschreibung:
***************************************************************************/


void  SwContentTree::EndDrag()
{
    bIsInternalDrag = sal_False;
    SvTreeListBox::EndDrag();
}
/***************************************************************************
    Beschreibung:   Wird ein Ctrl+DoubleClick in einen freien Bereich ausgefuehrt,
 *                  dann soll die Basisfunktion des Controls gerufen werden
***************************************************************************/
void  SwContentTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos( rMEvt.GetPosPixel());
    SvLBoxEntry* pEntry = GetEntry( aPos, sal_True );
    if( !pEntry && rMEvt.IsLeft() && rMEvt.IsMod1() && (rMEvt.GetClicks() % 2) == 0)
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

/***************************************************************************
    Beschreibung:   sofort aktualisieren
***************************************************************************/


void  SwContentTree::GetFocus()
{
    SwView* pActView = ::GetActiveView();
    if(pActView)
    {
        SwWrtShell* pActShell = pActView->GetWrtShellPtr();
        if(bIsConstant && !lcl_FindShell(pActiveShell))
        {
            SetActiveShell(pActShell);
        }

        if(bIsActive && pActShell != GetWrtShell())
            SetActiveShell(pActShell);
        else if( (bIsActive || (bIsConstant && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            Display(sal_True);
        }
    }
    else if(bIsActive)
        Clear();
    SvTreeListBox::GetFocus();
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


void  SwContentTree::KeyInput(const KeyEvent& rEvent)
{
    const KeyCode aCode = rEvent.GetKeyCode();
    if(aCode.GetCode() == KEY_RETURN)
    {
        SvLBoxEntry* pEntry = FirstSelected();
        if ( pEntry )
        {
            switch(aCode.GetModifier())
            {
                case KEY_MOD2:
                    // Boxen umschalten
                    GetParentWindow()->ToggleTree();
                break;
                case KEY_MOD1:
                    // RootModus umschalten
                    ToggleToRoot();
                break;
                case 0:
                    if(lcl_IsContentType(pEntry))
                    {
                        IsExpanded(pEntry) ?
                            Collapse(pEntry) :
                                Expand(pEntry);
                    }
                    else
                        ContentDoubleClickHdl(0);
                break;
            }
        }
    }
    else if(aCode.GetCode() == KEY_DELETE && 0 == aCode.GetModifier())
    {
        SvLBoxEntry* pEntry = FirstSelected();
        if(pEntry &&
            lcl_IsContent(pEntry) &&
                ((SwContent*)pEntry->GetUserData())->GetParent()->IsDeletable() &&
                    !pActiveShell->GetView().GetDocShell()->IsReadOnly())
        {
            EditEntry(pEntry, EDIT_MODE_DELETE);
            GrabFocus();
        }
    }
    else
        SvTreeListBox::KeyInput(rEvent);

}

/***************************************************************************
    Beschreibung:
***************************************************************************/


void  SwContentTree::RequestHelp( const HelpEvent& rHEvt )
{
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            sal_uInt16 nType;
            sal_Bool bBalloon = sal_False;
            sal_Bool bContent = sal_False;
            void* pUserData = pEntry->GetUserData();
            if(lcl_IsContentType(pEntry))
                nType = ((SwContentType*)pUserData)->GetType();
            else
            {
                nType = ((SwContent*)pUserData)->GetParent()->GetType();
                bContent = sal_True;
            }
            String sEntry;
            sal_Bool bRet = sal_False;
            if(bContent)
            {
                switch( nType )
                {
                    case CONTENT_TYPE_URLFIELD:
                        sEntry = ((SwURLFieldContent*)pUserData)->GetURL();
                        bRet = sal_True;
                    break;

                    case CONTENT_TYPE_POSTIT:
                        sEntry = ((SwPostItContent*)pUserData)->GetName();
                        bRet = sal_True;
                        if(Help::IsBalloonHelpEnabled())
                            bBalloon = sal_True;
                    break;
                    case CONTENT_TYPE_OUTLINE:
                        sEntry = ((SwOutlineContent*)pUserData)->GetName();
                        bRet = sal_True;
                    break;
                    case CONTENT_TYPE_GRAPHIC:
                        sEntry = ((SwGraphicContent*)pUserData)->GetLink();
#ifdef DEBUG
                        sEntry += ' ';
                        sEntry += String::CreateFromInt32(
                                    ((SwGraphicContent*)pUserData)->GetYPos());
#endif
                        bRet = sal_True;
                    break;
#ifdef DEBUG
                    case CONTENT_TYPE_TABLE:
                    case CONTENT_TYPE_FRAME:
                        sEntry = String::CreateFromInt32(
                                        ((SwContent*)pUserData)->GetYPos() );
                        bRet = sal_True;
                    break;
#endif
                }
                if(((SwContent*)pUserData)->IsInvisible())
                {
                    if(sEntry.Len())
                        sEntry += C2S(", ");
                    sEntry += sInvisible;
                    bRet = sal_True;
                }
            }
            else
            {
                sal_uInt16 nMemberCount = ((SwContentType*)pUserData)->GetMemberCount();
                sEntry = String::CreateFromInt32(nMemberCount);
                sEntry += ' ';
                sEntry += nMemberCount == 1
                            ? ((SwContentType*)pUserData)->GetSingleName()
                            : ((SwContentType*)pUserData)->GetName();
                bRet = sal_True;
            }
            if(bRet)
            {
                SvLBoxTab* pTab;
                SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
                if( pItem && SV_ITEM_ID_LBOXSTRING == pItem->IsA())
                {
                    aPos = GetEntryPos( pEntry );

                    aPos.X() = GetTabPos( pEntry, pTab );
                    Size aSize( pItem->GetSize( this, pEntry ) );

                    if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                        aSize.Width() = GetSizePixel().Width() - aPos.X();

                    aPos = OutputToScreenPixel(aPos);
                    Rectangle aItemRect( aPos, aSize );
                    if(bBalloon)
                    {
                        aPos.X() += aSize.Width();
                        Help::ShowBalloon( this, aPos, aItemRect, sEntry );
                    }
                    else
                        Help::ShowQuickHelp( this, aItemRect, sEntry,
                            QUICKHELP_LEFT|QUICKHELP_VCENTER );
                }
            }
            else
                Help::ShowQuickHelp( this, Rectangle(), aEmptyStr, 0 );
        }
    }
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


IMPL_LINK(SwContentTree, PopupHdl, Menu*, pMenu)
{
    sal_uInt16 nId = pMenu->GetCurItemId();
    SvLBoxEntry* pFirst = FirstSelected();
    switch( nId )
    {
        //Outlinelevel
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
            nId -= 100;
            if(nOutlineLevel != nId )
                SetOutlineLevel((sal_Int8)nId);
        break;
        case 201:
        case 202:
        case 203:
            GetParentWindow()->SetRegionDropMode(nId - 201);
        break;
        case 401:
        case 402:
            EditEntry(pFirst, nId == 401 ? EDIT_MODE_RMV_IDX : EDIT_MODE_UPD_IDX);
        break;
        // Eintrag bearbeiten
        case 403:
            EditEntry(pFirst);
        break;
        case 404:
            EditEntry(pFirst, EDIT_UNPROTECT_TABLE);
        break;
        case 405 :
        {
            const SwTOXBase* pBase = ((SwTOXBaseContent*)pFirst->GetUserData())
                                                                ->GetTOXBase();
            pActiveShell->SetTOXBaseReadonly(*pBase, !pMenu->IsItemChecked(nId));
        }
        break;
        case 4:
        break;
        case 501:
            EditEntry(pFirst, EDIT_MODE_DELETE);
        break;
        case 502 :
            EditEntry(pFirst, EDIT_MODE_RENAME);
        break;
        //Anzeige
        default: // nId > 300
        if(nId > 300 && nId < 400)
        {
            nId -= 300;
            SwView *pView = SwModule::GetFirstView();
            while (pView)
            {
                nId --;
                if(nId == 0)
                {
                    SetConstantShell(&pView->GetWrtShell());
                    break;
                }
                pView = SwModule::GetNextView(pView);
            }
            if(nId)
            {
                bViewHasChanged = bIsActive = nId==1;
                bIsConstant = sal_False;
                Display(nId == 1);
            }
        }
    }
    GetParentWindow()->UpdateListBox();
    return sal_True;
}

/***************************************************************************
    Beschreibung:
***************************************************************************/


void SwContentTree::SetOutlineLevel(sal_uInt8 nSet)
{
    nOutlineLevel = nSet;
    pConfig->SetOutlineLevel( nOutlineLevel );
    SwContentType** ppContentT = bIsActive ?
                    &aActiveContentArr[CONTENT_TYPE_OUTLINE] :
                        &aHiddenContentArr[CONTENT_TYPE_OUTLINE];
    if(*ppContentT)
    {
        (*ppContentT)->SetOutlineLevel(nOutlineLevel);
        (*ppContentT)->Init();
    }
    Display(bIsActive);
}

/***************************************************************************
    Beschreibung:   Moduswechsel: gedropptes Doc anzeigen
***************************************************************************/


void SwContentTree::ShowHiddenShell()
{
    if(pHiddenShell)
    {
        bIsConstant = sal_False;
        bIsActive = sal_False;
        Display(sal_False);
    }
}

/***************************************************************************
    Beschreibung:   Moduswechsel: aktive Sicht anzeigen
***************************************************************************/


void SwContentTree::ShowActualView()
{
    bIsActive = sal_True;
    bIsConstant = sal_False;
    Display(sal_True);
    GetParentWindow()->UpdateListBox();
}

/*-----------------20.11.96 13.34-------------------
    Beschreibung: Hier sollen die Buttons zum Verschieben von
                  Outlines en-/disabled werden
--------------------------------------------------*/

sal_Bool  SwContentTree::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    if(!pEntry)
        return sal_False;
    sal_Bool bEnable = sal_False;
    SvLBoxEntry* pParentEntry = GetParent(pEntry);
    if(!bIsLastReadOnly && (!IsVisible() ||
        (bIsRoot && nRootType == CONTENT_TYPE_OUTLINE && pParentEntry ||
            lcl_IsContent(pEntry) && ((SwContentType*)pParentEntry->GetUserData())->GetType() == CONTENT_TYPE_OUTLINE)))
        bEnable = sal_True;
    SwNavigationPI* pNavi = GetParentWindow();
    pNavi->aContentToolBox.EnableItem(FN_ITEM_UP ,  bEnable);
    pNavi->aContentToolBox.EnableItem(FN_ITEM_DOWN, bEnable);
    pNavi->aContentToolBox.EnableItem(FN_ITEM_LEFT, bEnable);
    pNavi->aContentToolBox.EnableItem(FN_ITEM_RIGHT,bEnable);

    return SvTreeListBox::Select(pEntry, bSelect);
}

/*-----------------27.11.96 12.56-------------------

--------------------------------------------------*/

void SwContentTree::SetRootType(sal_uInt16 nType)
{
    nRootType = nType;
    bIsRoot = sal_True;
    pConfig->SetRootType( nRootType );
}

/*-----------------10.01.97 12.19-------------------

--------------------------------------------------*/

void SwContentType::RemoveNewline(String& rEntry)
{
    sal_Unicode* pStr = rEntry.GetBufferAccess();
    for(xub_StrLen i = rEntry.Len(); i; --i, ++pStr )
    {
        if( *pStr == 10 || *pStr == 13 )
            *pStr = 0x20;
    }
}

/*-----------------14.01.97 16.38-------------------

--------------------------------------------------*/

void SwContentTree::EditEntry(SvLBoxEntry* pEntry, sal_uInt8 nMode)
{
    SwContent* pCnt = (SwContent*)pEntry->GetUserData();
    GotoContent(pCnt);
    sal_uInt16 nType = pCnt->GetParent()->GetType();
    sal_uInt16 nSlot = 0;

    uno::Reference< container::XNameAccess >  xNameAccess, xSecond, xThird;
    switch(nType)
    {
        case CONTENT_TYPE_TABLE     :
            if(nMode == EDIT_UNPROTECT_TABLE)
            {
                pActiveShell->GetView().GetDocShell()->
                        GetDoc()->UnProtectCells( pCnt->GetName());
            }
            else if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->StartAction();
                pActiveShell->StartUndo();
                pActiveShell->GetView().GetViewFrame()->GetDispatcher()->Execute(FN_TABLE_SELECT_ALL);
                pActiveShell->DeleteRow();
                pActiveShell->EndUndo();
                pActiveShell->EndAction();
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
                xNameAccess = xTables->getTextTables();
            }
            else
                nSlot = FN_FORMAT_TABLE_DLG;
        break;

        case CONTENT_TYPE_GRAPHIC   :
            if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->DelRight();
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
                xNameAccess = xGraphics->getGraphicObjects();
                uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
                xSecond = xFrms->getTextFrames();
                uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
                xThird = xObjs->getEmbeddedObjects();
            }
            else
                nSlot = FN_FORMAT_GRAFIC_DLG;
        break;

        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_OLE       :
            if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->DelRight();
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
                uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
                if(CONTENT_TYPE_FRAME == nType)
                {
                    xNameAccess = xFrms->getTextFrames();
                    xSecond = xObjs->getEmbeddedObjects();
                }
                else
                {
                    xNameAccess = xObjs->getEmbeddedObjects();
                    xSecond = xFrms->getTextFrames();
                }
                uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
                xThird = xGraphics->getGraphicObjects();
            }
            else
                nSlot = FN_FORMAT_FRAME_DLG;
        break;
        case CONTENT_TYPE_BOOKMARK  :
            if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->DelBookmark( pCnt->GetName() );
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XBookmarksSupplier >  xBkms(xModel, uno::UNO_QUERY);
                xNameAccess = xBkms->getBookmarks();
            }
            else
                nSlot = FN_INSERT_BOOKMARK;
        break;

        case CONTENT_TYPE_REGION    :
            if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextSectionsSupplier >  xSects(xModel, uno::UNO_QUERY);
                xNameAccess = xSects->getTextSections();
            }
            else
                nSlot = FN_EDIT_REGION;
        break;

        case CONTENT_TYPE_URLFIELD:
            nSlot = FN_EDIT_HYPERLINK;
        break;
        case CONTENT_TYPE_REFERENCE:
            nSlot = FN_EDIT_FIELD;
        break;

        case CONTENT_TYPE_POSTIT:
            if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->DelRight();
            }
            else
                nSlot = FN_POSTIT;
        break;
        case CONTENT_TYPE_INDEX:
        {
            const SwTOXBase* pBase = ((SwTOXBaseContent*)pCnt)->GetTOXBase();
            switch(nMode)
            {
                case EDIT_MODE_EDIT:
                    if(pBase)
                    {
                        SwPtrItem aPtrItem( FN_INSERT_MULTI_TOX, (void*)pBase);
                        pActiveShell->GetView().GetViewFrame()->
                            GetDispatcher()->Execute(FN_INSERT_MULTI_TOX,
                                            SFX_CALLMODE_ASYNCHRON, &aPtrItem, 0);

                    }
                break;
                case EDIT_MODE_RMV_IDX:
                case EDIT_MODE_DELETE:
                {
                    if( pBase )
                        pActiveShell->DeleteTOX(*pBase, EDIT_MODE_DELETE == nMode);
                }
                break;
                case EDIT_MODE_UPD_IDX:
                case EDIT_MODE_RENAME:
                {
                    Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                    Reference< XDocumentIndexesSupplier >  xIndexes(xModel, UNO_QUERY);
                    Reference< XIndexAccess> xIdxAcc(xIndexes->getDocumentIndexes());
                    Reference< XNameAccess >xLocalNameAccess(xIdxAcc, UNO_QUERY);
                    if(EDIT_MODE_RENAME == nMode)
                        xNameAccess = xLocalNameAccess;
                    else if(xLocalNameAccess.is() && xLocalNameAccess->hasByName(pBase->GetTOXName()))
                    {
                        Any aIdx = xLocalNameAccess->getByName(pBase->GetTOXName());
                        Reference< XDocumentIndex> xIdx;
                        if(aIdx >>= xIdx)
                            xIdx->update();
                    }
                }
                break;
            }
        }
        break;

    }
    if(nSlot)
        pActiveShell->GetView().GetViewFrame()->
                    GetDispatcher()->Execute(nSlot, SFX_CALLMODE_ASYNCHRON);
    else if(xNameAccess.is())
    {
        uno::Any aObj = xNameAccess->getByName(pCnt->GetName());
        uno::Reference< uno::XInterface >  xTmp = *(uno::Reference< uno::XInterface > *)aObj.getValue();
        uno::Reference< container::XNamed >  xNamed(xTmp, uno::UNO_QUERY);
        SwRenameXNamedDlg aDlg(this, xNamed, xNameAccess);
        if(xSecond.is())
            aDlg.SetAlternativeAccess( xSecond, xThird);

        String sForbiddenChars;
        if(CONTENT_TYPE_BOOKMARK == nType)
        {
            sForbiddenChars = C2S("/\\@:*?\";,.# ");
        }
        else if(CONTENT_TYPE_TABLE == nType)
        {
            sForbiddenChars = C2S(" .<>");
        }
        aDlg.SetForbiddenChars(sForbiddenChars);
        aDlg.Execute();
    }
}

/*-----------------14.01.97 16.53-------------------

--------------------------------------------------*/

void SwContentTree::GotoContent(SwContent* pCnt)
{
    pActiveShell->EnterStdMode();

    sal_Bool bSel = sal_False;
    sal_uInt16 nJumpType = pCnt->GetParent()->GetType();
    switch(nJumpType)
    {
        case CONTENT_TYPE_OUTLINE   :
        {
            pActiveShell->GotoOutline(((SwOutlineContent*)pCnt)->GetPos());
        }
        break;
        case CONTENT_TYPE_TABLE     :
        {
            pActiveShell->GotoTable(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_GRAPHIC   :
        case CONTENT_TYPE_OLE       :
        {
            if(pActiveShell->GotoFly(pCnt->GetName()))
                bSel = sal_True;
        }
        break;
        case CONTENT_TYPE_BOOKMARK  :
        {
            pActiveShell->GotoBookmark(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_REGION    :
        {
            pActiveShell->GotoRegion(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_URLFIELD:
        {
            if(pActiveShell->GotoINetAttr(
                            *((SwURLFieldContent*)pCnt)->GetINetAttr() ))
            {
                pActiveShell->Right(sal_True, 1, sal_False);
                pActiveShell->SwCrsrShell::SelectTxtAttr( RES_TXTATR_INETFMT, sal_True );
            }

        }
        break;
        case CONTENT_TYPE_REFERENCE:
        {
            pActiveShell->GotoRefMark(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_INDEX:
        {
            if (!pActiveShell->GotoNextTOXBase(&pCnt->GetName()))
                pActiveShell->GotoPrevTOXBase(&pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_POSTIT:
            pActiveShell->GotoFld(*((SwPostItContent*)pCnt)->GetPostIt());
        break;
        case CONTENT_TYPE_DRAWOBJECT:
            DBG_ERROR("unsupported format")
        break;
    }
    if(bSel)
    {
        pActiveShell->HideCrsr();
        pActiveShell->EnterSelFrmMode();
    }
    SwView& rView = pActiveShell->GetView();
    rView.StopShellTimer();
    rView.GetEditWin().GrabFocus();
}
/*-----------------06.02.97 19.14-------------------
    Jetzt nochtdie passende ::com::sun::star::text::Bookmark
--------------------------------------------------*/

NaviContentBookmark::NaviContentBookmark()
        :   nDefDrag( REGION_MODE_NONE ), nDocSh(0)
{
}

/*-----------------06.02.97 20.12-------------------

--------------------------------------------------*/

NaviContentBookmark::NaviContentBookmark( const String &rUrl,
                    const String& rDesc,
                    sal_uInt16 nDragType,
                    const SwDocShell* pDocSh ) :
    aUrl( rUrl ),
    aDescr(rDesc),
    nDefDrag( nDragType ),
    nDocSh((long)pDocSh)
{
}

/*-----------------06.02.97 19.17-------------------

--------------------------------------------------*/


sal_Bool    NaviContentBookmark::DragServerHasFormat( sal_uInt16 nItem,
                                                    const SwDocShell* pDocSh )
{
    sal_Bool bRet = sal_False;
    if(DragServer::HasFormat( nItem, SOT_FORMATSTR_ID_SONLK ))
    {
        long nDocSh = (long) pDocSh;
        NaviContentBookmark aTemp;
        aTemp.PasteDragServer(nItem);
        // steht vor dem # ein Dateiname?
        sal_uInt16 nFound = aTemp.GetURL().Search('#');
        // entweder die Quelle hatte einen Namen oder Quelle und Ziel sind gleich
        if(nFound != STRING_NOTFOUND &&
            (nFound > 0  ||
                nDocSh == aTemp.GetDocShell() ))
                bRet = sal_True;
    }
    return bRet;
}

/*-----------------06.02.97 19.17-------------------

--------------------------------------------------*/

sal_Bool NaviContentBookmark::CopyDragServer() const
{
     String aString( aUrl );
     aString += NAVI_BOOKMARK_DELIM;
     aString += aDescr;
     aString += NAVI_BOOKMARK_DELIM;
     aString += String::CreateFromInt32( nDefDrag );
     aString += NAVI_BOOKMARK_DELIM;
     aString += String::CreateFromInt32( nDocSh );

     return DragServer::CopyData( aString.GetBuffer(), (aString.Len() + 1) *2,
                                  SOT_FORMATSTR_ID_SONLK );
}
/*-----------------06.02.97 19.16-------------------

--------------------------------------------------*/

sal_Bool NaviContentBookmark::PasteDragServer( sal_uInt16 nItem )
{
    sal_Bool bRet = sal_False;
    if( DragServer::HasFormat( nItem, SOT_FORMATSTR_ID_SONLK ) )
    {
        sal_uInt32 nLen = DragServer::GetDataLen( nItem, SOT_FORMATSTR_ID_SONLK );
        String aString;
        DragServer::PasteData( nItem, aString.AllocBuffer(
                            nLen / 2 ), nLen, SOT_FORMATSTR_ID_SONLK );

        xub_StrLen nStrFndPos = 0;
        aUrl        = aString.GetToken(0, NAVI_BOOKMARK_DELIM, nStrFndPos );
        aDescr      = aString.GetToken(0, NAVI_BOOKMARK_DELIM, nStrFndPos );
        nDefDrag    = aString.GetToken(0, NAVI_BOOKMARK_DELIM,
                                        nStrFndPos ).ToInt32();
        nDocSh      = aString.GetToken(0, NAVI_BOOKMARK_DELIM,
                                        nStrFndPos ).ToInt32();
        bRet = sal_True;
    }
    return bRet;
}

/*-----------------06.02.97 20.43-------------------

--------------------------------------------------*/

sal_Bool NaviContentBookmark::Paste( SotDataObject& rObj, sal_uInt32 nFormat )
{
    sal_Bool bRet = sal_False;

    SvData aData( nFormat, MEDIUM_ALL );
    if( rObj.GetData( &aData ) )
    {
        void *pData;
        aData.GetData( &pData, TRANSFER_REFERENCE );

        if( nFormat == SOT_FORMATSTR_ID_SONLK )
        {
            String aString((sal_Unicode*)pData);
            xub_StrLen nStrFndPos = 0;
            aUrl        = aString.GetToken(0, NAVI_BOOKMARK_DELIM, nStrFndPos );
            aDescr      = aString.GetToken(0, NAVI_BOOKMARK_DELIM, nStrFndPos );
            nDefDrag    = aString.GetToken(0, NAVI_BOOKMARK_DELIM,
                                            nStrFndPos ).ToInt32();
            nDocSh      = aString.GetToken(0, NAVI_BOOKMARK_DELIM,
                                            nStrFndPos ).ToInt32();
            bRet = sal_True;
        }
    }
    return bRet;

}

/*-----------------07.02.97 08.23-------------------

--------------------------------------------------*/

sal_uInt32  NaviContentBookmark::HasFormat( SotDataObject& rObj )
{
    return rObj.GetTypeList().Get( SOT_FORMATSTR_ID_SONLK )
                ? SOT_FORMATSTR_ID_SONLK : 0;
}

/*-----------------20.02.97 15.37-------------------

--------------------------------------------------*/
IMPL_STATIC_LINK(SwContentTree, ExecDragHdl, SwContentTree*, EMPTYARG)
{
    SwContentTree::SetInDrag(sal_True);
    DropAction eDropAction = pThis->ExecuteDrag(Pointer(POINTER_MOVEDATA), Pointer(POINTER_COPYDATA), POINTER_LINKDATA, pThis->nDragMode );
    SwContentTree::SetInDrag(sal_False);
    return 0;
}
/* -----------------------------09.12.99 13:50--------------------------------

 ---------------------------------------------------------------------------*/
class SwContentLBoxString : public SvLBoxString
{
public:
    SwContentLBoxString( SvLBoxEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags,
        SvLBoxEntry* pEntry);
};

/* -----------------------------09.12.99 13:49--------------------------------

 ---------------------------------------------------------------------------*/
void SwContentTree::InitEntry(SvLBoxEntry* pEntry,
        const XubString& rStr ,const Image& rImg1,const Image& rImg2)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2 );
    SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nColToHilite );
    SwContentLBoxString* pStr = new SwContentLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}
/* -----------------------------09.12.99 13:49--------------------------------

 ---------------------------------------------------------------------------*/
void SwContentLBoxString::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags,
    SvLBoxEntry* pEntry )
{
    if(lcl_IsContent(pEntry) &&
            ((SwContent *)pEntry->GetUserData())->IsInvisible())
    {
        //* pCont = (SwContent*)pEntry->GetUserData();
        Font aOldFont( rDev.GetFont());
        Font aFont(aOldFont);
        Color aCol( COL_LIGHTGRAY );
        aFont.SetColor( aCol );
        rDev.SetFont( aFont );
        rDev.DrawText( rPos, GetText() );
        rDev.SetFont( aOldFont );
    }
    else
        SvLBoxString::Paint( rPos, rDev, nFlags, pEntry);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.4  2001/01/29 12:43:43  os
    #339# fixed: update/rename of indexes

    Revision 1.3  2000/11/03 11:32:31  os
    allow editing of indexes independent from the cursor position

    Revision 1.2  2000/10/20 13:42:18  jp
    use correct INetURL-Decode enum

    Revision 1.1.1.1  2000/09/18 17:14:50  hr
    initial import

    Revision 1.161  2000/09/18 16:06:17  willem.vandorp
    OpenOffice header added.

    Revision 1.160  2000/09/08 15:11:58  os
    use configuration service

    Revision 1.159  2000/09/07 15:59:34  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.158  2000/08/17 13:46:34  jp
    UI with decode URL; integer -> string bugs fixed

    Revision 1.157  2000/07/20 13:17:38  jp
    change old txtatr-character to the two new characters

    Revision 1.156  2000/07/03 08:54:55  jp
    must changes for VCL

    Revision 1.155  2000/06/30 10:25:49  os
    #63367# keep selected position in root mode, too

    Revision 1.154  2000/06/06 09:11:51  os
    76056# CreateFromInt32

    Revision 1.153  2000/05/23 19:54:16  jp
    Bugfixes for Unicode

    Revision 1.152  2000/04/26 15:03:20  os
    GetName() returns const String&

    Revision 1.151  2000/04/18 15:14:08  os
    UNICODE

    Revision 1.150  2000/03/23 07:51:10  os
    UNO III

    Revision 1.149  2000/03/03 15:17:05  os
    StarView remainders removed

    Revision 1.148  2000/02/22 16:56:35  jp
    Bug #73303#: new IsProtectedOutlinePara

    Revision 1.147  2000/02/11 15:00:33  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.146  2000/02/01 12:41:04  os
    #72443# visibility of section must be checked in SwContentType::Init

------------------------------------------------------------------------*/

