/*************************************************************************
 *
 *  $RCSfile: unoportenum.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-29 12:46:36 $
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

#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _NDHINTS_HXX
#include <ndhints.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTHBSH_HXX
#include <fmthbsh.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _UNOCLBCK_HXX
#include <unoclbck.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::rtl;

/******************************************************************
 *  SwXTextPortionEnumeration
 ******************************************************************/
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextPortionEnumeration::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextPortionEnumeration::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextPortionEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextPortionEnumeration");
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextPortionEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextPortionEnumeration") == rServiceName;
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextPortionEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextPortionEnumeration");
    return aRet;
}
/*-- 27.01.99 10:44:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortionEnumeration::SwXTextPortionEnumeration(SwPaM& rParaCrsr, uno::Reference< XText >  xParentText) :
    xParent(xParentText),
    bAtEnd(sal_False),
    bFirstPortion(sal_True)
{
    SwUnoCrsr* pUnoCrsr = rParaCrsr.GetDoc()->CreateUnoCrsr(*rParaCrsr.GetPoint(), sal_False);
    pUnoCrsr->Add(this);

    //alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // AM ZEICHEN gebunden sind
    ::CollectFrameAtNode( *this, pUnoCrsr->GetPoint()->nNode,
                            aFrameArr, TRUE );
#if 0
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    const SwNodeIndex& rOwnNode = pUnoCrsr->GetPoint()->nNode;
    const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
    sal_uInt16 nCount = rFmts.Count();
    const SwPosition* pAnchorPos;
    SvXub_StrLens aSortArr( 8, 8 );

    for( sal_uInt16 i = 0; i < nCount; i++)
    {
        SwFrmFmt* pFmt = rFmts[ i ];
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();

        //steht der Anker in diesem Node und ist er absatzgebunden?
        if( FLY_AUTO_CNTNT == rAnchor.GetAnchorId() &&
            0 != ( pAnchorPos = rAnchor.GetCntntAnchor() ) &&
            RES_FLYFRMFMT == pFmt->Which() &&
            pAnchorPos->nNode == rOwnNode )
        {
            //jetzt einen SwDepend anlegen und sortiert in das Array einfuegen
            SwDepend* pNewDepend = new SwDepend(this, pFmt);
            xub_StrLen nInsertIndex = pAnchorPos->nContent.GetIndex();

            USHORT nInsPos = 0, nEnd = aSortArr.Count();
            for( ; nInsPos < nEnd; ++nInsPos )
                if( aSortArr[ nInsPos ] > nInsertIndex )
                    break;

            aSortArr.Insert( nInsertIndex, nInsPos );
            aFrameArr.C40_INSERT( SwDepend, pNewDepend, nInsPos );
        }
    }
#endif
    CreatePortions();
}
/*-- 27.01.99 10:44:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortionEnumeration::~SwXTextPortionEnumeration()
{
    for(sal_uInt16 nFrame = aFrameArr.Count(); nFrame; )
        delete aFrameArr.GetObject( --nFrame );
    aFrameArr.Remove(0, aFrameArr.Count());

    if( aPortionArr.Count() )
        aPortionArr.DeleteAndDestroy(0, aPortionArr.Count() );

    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
}
/*-- 27.01.99 10:44:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextPortionEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return aPortionArr.Count() > 0;
}
/*-- 27.01.99 10:44:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortionEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!aPortionArr.Count())
        throw container::NoSuchElementException();
    XTextRangeRefPtr pPortion = aPortionArr.GetObject(0);
    Any aRet(pPortion, ::getCppuType((uno::Reference<XTextRange>*)0));
    aPortionArr.Remove(0);
    delete pPortion;
    return aRet;
}
/* -----------------------------31.08.00 14:28--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_InsertRefMarkPortion(
    XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwFmtRefMark& rRefMark = ((SwFmtRefMark&)pAttr->GetAttr());
    Reference<XTextContent> xContent = ((SwUnoCallBack*)pDoc->GetUnoCallBack())->GetRefMark(rRefMark);
    if(!xContent.is())
        xContent = new SwXReferenceMark(pDoc, &rRefMark);

    SwXTextPortion* pPortion;
    if(!bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_REFMARK_START)),
            rArr.Count());
        pPortion->SetRefMark(xContent);
        pPortion->SetCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
    }
    else
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_REFMARK_END)),
            rArr.Count());
        pPortion->SetRefMark(xContent);
    }
}
//-----------------------------------------------------------------------------
void lcl_InsertRubyPortion( XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr,
                        Reference<XText> xParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwXRubyPortion* pPortion;
    rArr.Insert(
        new Reference< XTextRange >(pPortion = new SwXRubyPortion(*pUnoCrsr, *(SwTxtRuby*)pAttr, xParent,
            bEnd)),
        rArr.Count());
    pPortion->SetCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
}
//-----------------------------------------------------------------------------
void lcl_InsertTOXMarkPortion(
    XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwTOXMark& rTOXMark = ((SwTOXMark&)pAttr->GetAttr());

    Reference<XTextContent> xContent =
        ((SwUnoCallBack*)pDoc->GetUnoCallBack())->GetTOXMark(rTOXMark);
    if(!xContent.is())
        xContent = new SwXDocumentIndexMark(rTOXMark.GetTOXType(), &rTOXMark, pDoc);

    SwXTextPortion* pPortion;
    if(!bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_TOXMARK_START)),
            rArr.Count());
        pPortion->SetTOXMark(xContent);
        pPortion->SetCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
    }
    if(bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_TOXMARK_END)),
            rArr.Count());
        pPortion->SetTOXMark(xContent);
    }
}
//-----------------------------------------------------------------------------
#define BKM_TYPE_START          0
#define BKM_TYPE_END            1
#define BKM_TYPE_START_END      2
struct SwXBookmarkPortion_Impl
{
    SwXBookmark*                pBookmark;
    Reference<XTextContent>     xBookmark;
    BYTE                        nBkmType;
    ULONG                       nIndex;

    SwXBookmarkPortion_Impl( SwXBookmark* pXMark, BYTE nType, ULONG nIdx) :
        pBookmark(pXMark),
        xBookmark(pXMark),
        nBkmType(nType),
        nIndex(nIdx){}

    // compare by nIndex
    BOOL operator < (const SwXBookmarkPortion_Impl &rCmp) const;
    BOOL operator ==(const SwXBookmarkPortion_Impl &rCmp) const;

};
BOOL SwXBookmarkPortion_Impl::operator ==(const SwXBookmarkPortion_Impl &rCmp) const
{
    return nIndex == rCmp.nIndex &&
        pBookmark == rCmp.pBookmark &&
            nBkmType == rCmp.nBkmType;
}

BOOL SwXBookmarkPortion_Impl::operator < (const SwXBookmarkPortion_Impl &rCmp) const
{
    return nIndex < rCmp.nIndex;
}

typedef SwXBookmarkPortion_Impl* SwXBookmarkPortion_ImplPtr;
SV_DECL_PTRARR_SORT(SwXBookmarkPortionArr, SwXBookmarkPortion_ImplPtr, 0, 2)
SV_IMPL_OP_PTRARR_SORT(SwXBookmarkPortionArr, SwXBookmarkPortion_ImplPtr)
//-----------------------------------------------------------------------------
void lcl_ExportBookmark(
    SwXBookmarkPortionArr& rBkmArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, XTextRangeArr& rPortionArr)
{
    if(!rBkmArr.Count())
        return;
    SwXBookmarkPortion_ImplPtr pPtr;
    while(rBkmArr.Count() && nIndex == (pPtr = rBkmArr.GetObject(0))->nIndex)
    {
        SwXTextPortion* pPortion;
        if(BKM_TYPE_START == pPtr->nBkmType || BKM_TYPE_START_END == pPtr->nBkmType)
        {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_BOOKMARK_START)),
                rPortionArr.Count());
            pPortion->SetBookmark(pPtr->xBookmark);
            pPortion->SetCollapsed(BKM_TYPE_START_END == pPtr->nBkmType ? TRUE : FALSE);

        }
        if(BKM_TYPE_END == pPtr->nBkmType)
        {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_BOOKMARK_END)),
                rPortionArr.Count());
            pPortion->SetBookmark(pPtr->xBookmark);
        }
        rBkmArr.Remove((USHORT)0);
        delete pPtr;
    }
}
/* -----------------------------18.12.00 14:51--------------------------------

 ---------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
#define REDLINE_PORTION_START_REMOVE 0//removed redlines are visible
#define REDLINE_PORTION_END_REMOVE   1//removed redlines are visible
#define REDLINE_PORTION_REMOVE       2//removed redlines are NOT visible
#define REDLINE_PORTION_INSERT_START 3
#define REDLINE_PORTION_INSERT_END   4

struct SwXRedlinePortion_Impl
{
    const SwRedline*    pRedline;
    sal_Bool            bStart;

    SwXRedlinePortion_Impl(const SwRedline* pRed, sal_Bool bIsStart) :
        pRedline(pRed),
        bStart(bIsStart)
        {}

    USHORT GetIndexPos()
    {
        const SwPosition* pOwnPos = bStart ?
                    pRedline->Start() : pRedline->End();
        return pOwnPos->nContent.GetIndex();
    }
    // compare by Position
    BOOL operator < (const SwXRedlinePortion_Impl &rCmp) const
        {   const SwPosition* pOwnPos = bStart ?
                    pRedline->Start() : pRedline->End();
            const SwPosition* pCmpPos = rCmp.bStart ?
                rCmp.pRedline->Start() : rCmp.pRedline->End();
            return *pOwnPos <= *pCmpPos;
        }

    BOOL operator ==(const SwXRedlinePortion_Impl &rCmp) const
        {return pRedline == rCmp.pRedline &&
                    bStart == rCmp.bStart;}
};
typedef SwXRedlinePortion_Impl* SwXRedlinePortion_ImplPtr;
SV_DECL_PTRARR_SORT(SwXRedlinePortionArr, SwXRedlinePortion_ImplPtr, 0, 2)
SV_IMPL_OP_PTRARR_SORT(SwXRedlinePortionArr, SwXRedlinePortion_ImplPtr)

//-----------------------------------------------------------------------------
Reference<XTextRange> lcl_ExportHints(SwpHints* pHints,
                                XTextRangeArr& rPortionArr,
                                SwUnoCrsr* pUnoCrsr,
                                Reference<XText> xParent,
                                const xub_StrLen nCurrentIndex,
                                SwTextPortionType& ePortionType,
                                const xub_StrLen& nFirstFrameIndex,
                                SwXBookmarkPortionArr& aBkmArr,
                                SwXRedlinePortionArr& aRedArr )
{
    Reference<XTextRange> xRef;
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    sal_Bool bAttrFound = sal_False;
    //search for special text attributes - first some ends
    sal_uInt16 nEndIndex = 0;
    sal_uInt16 nNextEnd = 0;
    while(nEndIndex < pHints->GetEndCount() &&
        (!pHints->GetEnd(nEndIndex)->GetEnd() ||
        nCurrentIndex >= (nNextEnd = (*pHints->GetEnd(nEndIndex)->GetEnd()))))
    {
        if(pHints->GetEnd(nEndIndex)->GetEnd())
        {
            SwTxtAttr* pAttr = pHints->GetEnd(nEndIndex);
            USHORT nAttrWhich = pAttr->Which();
            if(nNextEnd == nCurrentIndex &&
                ( RES_TXTATR_TOXMARK == nAttrWhich ||
                    RES_TXTATR_REFMARK == nAttrWhich ||
                        RES_TXTATR_CJK_RUBY == nAttrWhich))
            {
                switch( nAttrWhich )
                {
                    case RES_TXTATR_TOXMARK:
                        lcl_InsertTOXMarkPortion(
                            rPortionArr, pUnoCrsr, xParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;
                    case RES_TXTATR_REFMARK:
                        lcl_InsertRefMarkPortion(
                            rPortionArr, pUnoCrsr, xParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;
                    case RES_TXTATR_CJK_RUBY:
                        lcl_InsertRubyPortion(
                            rPortionArr, pUnoCrsr, xParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;
                }
            }
        }
        nEndIndex++;
    }
    //then som starts
    sal_uInt16 nStartIndex = 0;
    sal_uInt16 nNextStart = 0;
    while(nStartIndex < pHints->GetStartCount() &&
        nCurrentIndex >= (nNextStart = (*pHints->GetStart(nStartIndex)->GetStart())))
    {
        SwTxtAttr* pAttr = pHints->GetStart(nStartIndex);
        USHORT nAttrWhich = pAttr->Which();
        if(nNextStart == nCurrentIndex &&
            (!pAttr->GetEnd() ||
                RES_TXTATR_TOXMARK == nAttrWhich ||
                    RES_TXTATR_REFMARK == nAttrWhich||
                        RES_TXTATR_CJK_RUBY == nAttrWhich))
        {
            switch( nAttrWhich )
            {
                case RES_TXTATR_FIELD:
                    pUnoCrsr->Right(1);
                    bAttrFound = sal_True;
                    ePortionType = PORTION_FIELD;
                break;
                case RES_TXTATR_FLYCNT   :
                    pUnoCrsr->Right(1);
                    pUnoCrsr->Exchange();
                    bAttrFound = sal_True;
                    ePortionType = PORTION_FRAME;
                break;
                case RES_TXTATR_FTN      :
                {
                    pUnoCrsr->Right(1);
                    SwXTextPortion* pPortion;
                    xRef =  pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_FOOTNOTE);
                    Reference<XTextContent> xContent =
                        Reference<XTextContent>(
                        SwXFootnotes::GetObject(*pDoc, pAttr->SwTxtAttr::GetFtn()),
                        UNO_QUERY);
                    pPortion->SetFootnote(xContent);
                    bAttrFound = sal_True;
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_SOFTHYPH :
                {
                    SwXTextPortion* pPortion;
                    rPortionArr.Insert(
                        new Reference< XTextRange >(
                            pPortion = new SwXTextPortion(
                                *pUnoCrsr, xParent, PORTION_CONTROL_CHAR)),
                        rPortionArr.Count());
                    pPortion->SetControlChar(3);
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_HARDBLANK:
                {
                    ePortionType = PORTION_CONTROL_CHAR;
                    SwXTextPortion* pPortion;
                    rPortionArr.Insert(
                        new Reference< XTextRange >(
                            pPortion = new SwXTextPortion(
                                *pUnoCrsr, xParent, PORTION_CONTROL_CHAR)),
                        rPortionArr.Count());
                    const SwFmtHardBlank& rFmt = pAttr->GetHardBlank();
                    if(rFmt.GetChar() == '-')
                        pPortion->SetControlChar(2);//HARD_HYPHEN
                    else
                        pPortion->SetControlChar(4);//HARD_SPACE
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_TOXMARK:
                    lcl_InsertTOXMarkPortion(
                        rPortionArr, pUnoCrsr, xParent, pAttr, FALSE);
                    ePortionType = PORTION_TEXT;
                break;
                case RES_TXTATR_REFMARK:
                    if(!pAttr->GetEnd())
                    {
                        pUnoCrsr->Right(1);
                        bAttrFound = sal_True;
                    }
                    lcl_InsertRefMarkPortion(
                        rPortionArr, pUnoCrsr, xParent, pAttr, FALSE);
                    ePortionType = PORTION_TEXT;
                    if(!pAttr->GetEnd())
                    {
                        if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                                pUnoCrsr->Exchange();
                        pUnoCrsr->DeleteMark();
                    }
                break;
                case RES_TXTATR_CJK_RUBY:
                    if(pAttr->GetEnd())
                    {
                        lcl_InsertRubyPortion(
                            rPortionArr, pUnoCrsr, xParent, pAttr, FALSE);
                        ePortionType = PORTION_TEXT;
                    }
                break;
                default:
                    DBG_ERROR("was fuer ein Attribut?");
            }

        }
        nStartIndex++;
    }

    if(!bAttrFound)
    {
        // hier wird nach Uebergaengen zwischen Attributen gesucht, die nach der
        // aktuellen Cursorposition liegen
        // wenn dabei ein Rahmen 'ueberholt' wird, dann muss auch in der TextPortion unterbrochen werden

        nStartIndex = 0;
        nNextStart = 0;
        while(nStartIndex < pHints->GetStartCount() &&
            nCurrentIndex >= (nNextStart = (*pHints->GetStart(nStartIndex)->GetStart())))
            nStartIndex++;

        sal_uInt16 nEndIndex = 0;
        sal_uInt16 nNextEnd = 0;
        while(nEndIndex < pHints->GetEndCount() &&
            nCurrentIndex >= (nNextEnd = (*pHints->GetEnd(nEndIndex)->GetAnyEnd())))
            nEndIndex++;
        //nMovePos legt die neue EndPosition fest
        sal_uInt16 nMovePos = nNextStart > nCurrentIndex && nNextStart < nNextEnd ? nNextStart : nNextEnd;
        if (nMovePos <= nCurrentIndex)
            nMovePos = pUnoCrsr->GetCntntNode()->Len();

        if(aBkmArr.Count() && aBkmArr.GetObject(0)->nIndex < nMovePos)
        {
            DBG_ASSERT(aBkmArr.GetObject(0)->nIndex > nCurrentIndex,
                "forgotten bookmark(s)")
            nMovePos = aBkmArr.GetObject(0)->nIndex;
        }
        // break portions up for redlines
        if (aRedArr.Count() && aRedArr.GetObject(0)->GetIndexPos() < nMovePos)
        {
            nMovePos = aRedArr.GetObject(0)->GetIndexPos();
        }
        // liegt die Endposition nach dem naechsten Rahmen, dann aufbrechen
        if(nFirstFrameIndex != STRING_MAXLEN && nMovePos > nFirstFrameIndex)
            nMovePos = nFirstFrameIndex;

        if(nMovePos > nCurrentIndex)
            pUnoCrsr->Right(nMovePos - nCurrentIndex);
        else
        {
            // ensure proper exit: move to paragraph end
            // (this should not be necessary any more; we assert it only
            //  happens when the above would move to the end of the
            //  paragraph anyway)
            DBG_ASSERT(nMovePos == pUnoCrsr->GetCntntNode()->Len(),
                       "may only happen at paragraph end");
            pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
        }
    }
    return xRef;
}
//-----------------------------------------------------------------------------
void lcl_FillBookmarkArray(SwDoc& rDoc,SwUnoCrsr& rUnoCrsr, SwXBookmarkPortionArr& rBkmArr )
{
        const SwNodeIndex nOwnNode = rUnoCrsr.GetPoint()->nNode;
        //search for all bookmarks that start or end in this paragraph
        const SwBookmarks& rMarks = rDoc.GetBookmarks();
        sal_uInt16 nArrLen = rMarks.Count();
        for( sal_uInt16 n = 0; n < nArrLen; ++n )
        {
            SwBookmark* pMark = rMarks.GetObject( n );
            if( !pMark->IsBookMark() )
                continue;

            const SwPosition& rPos1 = pMark->GetPos();
            const SwPosition* pPos2 = pMark->GetOtherPos();
            BOOL bBackward = pPos2 ? rPos1 > *pPos2: FALSE;
            if(rPos1.nNode == nOwnNode)
            {
                BYTE nType = bBackward ? BKM_TYPE_END : BKM_TYPE_START;
                if(!pPos2)
                {
                    nType = BKM_TYPE_START_END;
                }
                SwXBookmarkPortion_ImplPtr pBkmPtr = new SwXBookmarkPortion_Impl(
                    SwXBookmarks::GetObject( *pMark, &rDoc ), nType, rPos1.nContent.GetIndex() );

                rBkmArr.Insert(pBkmPtr);

            }
            if(pPos2 && pPos2->nNode == nOwnNode)
            {
                BYTE nType = bBackward ? BKM_TYPE_START : BKM_TYPE_END;
                SwXBookmarkPortion_ImplPtr pBkmPtr = new SwXBookmarkPortion_Impl(
                        SwXBookmarks::GetObject( *pMark, &rDoc ), nType, pPos2->nContent.GetIndex() );
                rBkmArr.Insert(pBkmPtr);
            }
        }
}
//-----------------------------------------------------------------------------
void lcl_FillRedlineArray(SwDoc& rDoc,SwUnoCrsr& rUnoCrsr, SwXRedlinePortionArr& rRedArr )
{
    const SwRedlineTbl& rRedTbl = rDoc.GetRedlineTbl();
    const SwPosition* pStart = rUnoCrsr.GetPoint();
    const SwNodeIndex nOwnNode = pStart->nNode;
    SwRedlineMode eRedMode = rDoc.GetRedlineMode();

    for(USHORT nRed = 0; nRed < rRedTbl.Count(); nRed++)
    {
        const SwRedline* pRedline = rRedTbl[nRed];
        const SwPosition* pRedStart = pRedline->GetPoint();
        const SwNodeIndex nRedNode = pRedStart->nNode;
        SwRedlineType nType = pRedline->GetType();
        if(nOwnNode == nRedNode)
        {
            SwXRedlinePortion_ImplPtr pToInsert = new SwXRedlinePortion_Impl(pRedline, TRUE);
            rRedArr.Insert(pToInsert);
        }
        if(pRedline->HasMark() && pRedline->GetMark()->nNode == nOwnNode)
        {
            SwXRedlinePortion_ImplPtr pToInsert = new SwXRedlinePortion_Impl(pRedline, FALSE);
            rRedArr.Insert(pToInsert);
        }
    }
}
/* -----------------------------19.12.00 12:25--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_ExportRedline(
    SwXRedlinePortionArr& rRedlineArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, XTextRangeArr& rPortionArr)
{
    if(!rRedlineArr.Count())
        return;
    SwXRedlinePortion_ImplPtr pPtr;
    while(rRedlineArr.Count() &&  0 != (pPtr = rRedlineArr.GetObject(0)) &&
        ((pPtr->bStart && nIndex == pPtr->pRedline->Start()->nContent.GetIndex())||
            (!pPtr->bStart && nIndex == pPtr->pRedline->End()->nContent.GetIndex())))
    {
        SwXTextPortion* pPortion;

// ?????
//      if(pPtr->bStart )
//      {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXRedlinePortion(
                            pPtr->pRedline, *pUnoCrsr, xParent,
                            pPtr->bStart)),
                rPortionArr.Count());

//      }
        rRedlineArr.Remove((USHORT)0);
        delete pPtr;
    }
}
/* -----------------------------19.12.00 13:09--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_ExportBkmAndRedline(
    SwXBookmarkPortionArr& rBkmArr,
    SwXRedlinePortionArr& rRedlineArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, XTextRangeArr& rPortionArr)
{
    lcl_ExportBookmark(rBkmArr, nIndex, pUnoCrsr, xParent, rPortionArr);
    lcl_ExportRedline(rRedlineArr, nIndex, pUnoCrsr, xParent, rPortionArr);
}
//-----------------------------------------------------------------------------
sal_Int32 lcl_GetNextIndex(SwXBookmarkPortionArr& rBkmArr, SwXRedlinePortionArr& rRedlineArr)
{
    sal_Int32 nRet = -1;
    if(rBkmArr.Count())
    {
        SwXBookmarkPortion_ImplPtr pPtr = rBkmArr.GetObject(0);
        nRet = pPtr->nIndex;
    }
    if(rRedlineArr.Count())
    {
        SwXRedlinePortion_ImplPtr pPtr = rRedlineArr.GetObject(0);
        USHORT nTmp = pPtr->GetIndexPos();
        if(nRet < 0 || nTmp < nRet)
            nRet = nTmp;
    }
    return nRet;
};
//-----------------------------------------------------------------------------
void SwXTextPortionEnumeration::CreatePortions()
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr /*&& !bAtEnd*/)
    {
        SwXBookmarkPortionArr aBkmArr;
        SwXRedlinePortionArr aRedArr;

        SwDoc* pDoc = pUnoCrsr->GetDoc();
        lcl_FillRedlineArray(*pDoc, *pUnoCrsr, aRedArr);
        lcl_FillBookmarkArray(*pDoc, *pUnoCrsr, aBkmArr );
#ifdef DEBUG
        for(long i_debug = 0; i_debug <aBkmArr.Count(); i_debug++)
        {
            SwXBookmarkPortion_ImplPtr pPtr = aBkmArr.GetObject(i_debug);
        }

#endif
        while(!bAtEnd)
        {
            if(pUnoCrsr->HasMark())
            {
                if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                    pUnoCrsr->Exchange();
                pUnoCrsr->DeleteMark();
            }
            if(!bFirstPortion   && pUnoCrsr->GetCntntNode() &&
                    pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len())
            {
                //hier sollte man nie ankommen!
                bAtEnd = sal_True;
            }
            else
            {
                SwNode* pNode = pUnoCrsr->GetNode();
                if(ND_TEXTNODE == pNode->GetNodeType())
                {
                    SwTxtNode* pTxtNode = (SwTxtNode*)pNode;
                    SwpHints* pHints = pTxtNode->GetpSwpHints();
                    SwTextPortionType ePortionType = PORTION_TEXT;
                    xub_StrLen nCurrentIndex = pUnoCrsr->GetPoint()->nContent.GetIndex();
                    xub_StrLen nFirstFrameIndex = STRING_MAXLEN;
                    uno::Reference< XTextRange >  xRef;
                    if(!pUnoCrsr->GetCntntNode()->Len())
                    {
                        lcl_ExportBkmAndRedline(aBkmArr, aRedArr, 0, pUnoCrsr, xParent, aPortionArr);
                        // der Absatz ist leer, also nur Portion erzeugen und raus
                        xRef = new SwXTextPortion(*pUnoCrsr, xParent, ePortionType);
                    }
                    else
                    {
                        //falls schon Rahmen entsorgt wurden, dann raus hier
                        for(sal_uInt16 nFrame = aFrameArr.Count(); nFrame; nFrame--)
                        {
                            SwDepend* pCurDepend = aFrameArr.GetObject(nFrame - 1);
                            if(!pCurDepend->GetRegisteredIn())
                            {
                                delete pCurDepend;
                                aFrameArr.Remove(nFrame - 1);
                            }
                        }

                        //zunaechst den ersten Frame im aFrameArr finden (bezogen auf die Position im Absatz)
                        SwDepend* pFirstFrameDepend = 0;
                        //Eintraege im aFrameArr sind sortiert!
                        if(aFrameArr.Count())
                        {
                            SwDepend* pCurDepend = aFrameArr.GetObject(0);
                            SwFrmFmt* pFormat = (SwFrmFmt*)pCurDepend->GetRegisteredIn();
                            const SwFmtAnchor& rAnchor = pFormat->GetAnchor();
                            const SwPosition* pAnchorPos = rAnchor.GetCntntAnchor();
                            pFirstFrameDepend = pCurDepend;
                            nFirstFrameIndex = pAnchorPos->nContent.GetIndex();
                        }

                        SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);

                        //ist hier schon ein Rahmen faellig?
                        if(nCurrentIndex == nFirstFrameIndex)
                        {
                            xRef = new SwXTextPortion(*pUnoCrsr, xParent,
                                *(SwFrmFmt*)pFirstFrameDepend->GetRegisteredIn());
                            SwDepend* pCurDepend = aFrameArr.GetObject(0);
                            delete pCurDepend;
                            aFrameArr.Remove(0);
                        }
                    }
                    if(!xRef.is())
                    {
                        lcl_ExportBkmAndRedline(aBkmArr, aRedArr, nCurrentIndex, pUnoCrsr, xParent, aPortionArr);
                        if(pHints)
                        {
                            xRef = lcl_ExportHints(pHints,
                                aPortionArr,
                                pUnoCrsr,
                                xParent,
                                nCurrentIndex,
                                ePortionType,
                                nFirstFrameIndex,
                                aBkmArr,
                                aRedArr);

                        }
                        else if(USHRT_MAX != nFirstFrameIndex)
                        {
                            pUnoCrsr->Right(nFirstFrameIndex - nCurrentIndex);
                        }
                        else
                        {
//                          lcl_ExportBkmAndRedline(aBkmArr, aRedArr, nCurrentIndex, pUnoCrsr, xParent, aPortionArr);
                            sal_Int32 nNextIndex = lcl_GetNextIndex(aBkmArr, aRedArr);
                            if(nNextIndex < 0)
                                sal_Bool bMove = pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
                            else
                            {
                                DBG_ASSERT(nNextIndex > nCurrentIndex, "wrong move index")
                                pUnoCrsr->Right(nNextIndex - nCurrentIndex);
                            }
                        }
                    }
                    if(!xRef.is() && pUnoCrsr->HasMark() )
                        xRef = new SwXTextPortion(*pUnoCrsr, xParent, ePortionType);
                    if(xRef.is())
                        aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                }
                else
                {
                    DBG_ERROR("kein TextNode - was nun?")
                }
            }
            if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                    pUnoCrsr->Exchange();

            // Absatzende ?
            if(pUnoCrsr->GetCntntNode() &&
                    pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len())
            {
                bAtEnd = sal_True;
                lcl_ExportBkmAndRedline(aBkmArr, aRedArr, pUnoCrsr->GetCntntNode()->Len(),
                                            pUnoCrsr, xParent, aPortionArr);
                SwNode* pNode = pUnoCrsr->GetNode();
                if(ND_TEXTNODE == pNode->GetNodeType())
                {
                    SwTxtNode* pTxtNode = (SwTxtNode*)pNode;
                    SwpHints* pHints = pTxtNode->GetpSwpHints();
                    if(pHints)
                    {
                        SwTextPortionType ePortionType = PORTION_TEXT;
                        Reference<XTextRange> xRef = lcl_ExportHints(pHints,
                            aPortionArr,
                            pUnoCrsr,
                            xParent,
                            pUnoCrsr->GetCntntNode()->Len(),
                            ePortionType,
                            STRING_MAXLEN,
                            aBkmArr,
                            aRedArr);
                        if(xRef.is())
                            aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                    }
                }
            }
        }
    }
}
/*-- 27.01.99 10:44:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXTextPortionEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

