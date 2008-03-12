/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoportenum.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:32:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
// --> OD 2007-10-23 #i81002#
#include <crossrefbookmark.hxx>
// <--
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
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include <set>
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using ::rtl::OUString;
using namespace ::std;
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
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ) );
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
SwXTextPortionEnumeration::SwXTextPortionEnumeration(
    SwPaM& rParaCrsr,
    uno::Reference< XText >  xParentText,
    sal_Int32 nStart,
    sal_Int32 nEnd
    ) :
    xParent(xParentText),
    bAtEnd(sal_False),
    bFirstPortion(sal_True),
    nStartPos(nStart),
    nEndPos(nEnd)
{
    SwUnoCrsr* pUnoCrsr = rParaCrsr.GetDoc()->CreateUnoCrsr(*rParaCrsr.GetPoint(), sal_False);
    pUnoCrsr->Add(this);

    DBG_ASSERT(nEnd == -1 || (nStart <= nEnd &&
        nEnd <= pUnoCrsr->Start()->nNode.GetNode().GetTxtNode()->GetTxt().Len()),
            "start or end value invalid!")
    //alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // AM ZEICHEN gebunden sind
    ::CollectFrameAtNode( *this, pUnoCrsr->GetPoint()->nNode,
                            aFrameArr, TRUE );
    CreatePortions();
}
/*-- 27.01.99 10:44:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortionEnumeration::~SwXTextPortionEnumeration()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr, Reference<XText>& rParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwFmtRefMark& rRefMark = ((SwFmtRefMark&)pAttr->GetAttr());
    Reference<XTextContent> xContent = ((SwUnoCallBack*)pDoc->GetUnoCallBack())->GetRefMark(rRefMark);
    if(!xContent.is())
        xContent = new SwXReferenceMark(pDoc, &rRefMark);

    SwXTextPortion* pPortion = 0;
    if(!bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_REFMARK_START)),
            rArr.Count());
        pPortion->SetRefMark(xContent);
        pPortion->SetCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
    }
    else
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_REFMARK_END)),
            rArr.Count());
        pPortion->SetRefMark(xContent);
    }
}
//-----------------------------------------------------------------------------
void lcl_InsertRubyPortion( XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr,
                        Reference<XText>& rParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwXRubyPortion* pPortion = 0;
    rArr.Insert(
        new Reference< XTextRange >(pPortion = new SwXRubyPortion(*pUnoCrsr, *(SwTxtRuby*)pAttr, rParent,
            bEnd)),
        rArr.Count());
    pPortion->SetCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
}
//-----------------------------------------------------------------------------
void lcl_InsertTOXMarkPortion(
    XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr, Reference<XText>& rParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwTOXMark& rTOXMark = ((SwTOXMark&)pAttr->GetAttr());

    Reference<XTextContent> xContent =
        ((SwUnoCallBack*)pDoc->GetUnoCallBack())->GetTOXMark(rTOXMark);
    if(!xContent.is())
        xContent = new SwXDocumentIndexMark(rTOXMark.GetTOXType(), &rTOXMark, pDoc);

    SwXTextPortion* pPortion = 0;
    if(!bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_TOXMARK_START)),
            rArr.Count());
        pPortion->SetTOXMark(xContent);
        pPortion->SetCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
    }
    if(bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_TOXMARK_END)),
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
    Reference<XTextContent>     xBookmark;
    BYTE                        nBkmType;
    const SwPosition            aPosition;

    SwXBookmarkPortion_Impl( SwXBookmark* pXMark, BYTE nType, const SwPosition &rPosition )
    : xBookmark ( pXMark )
    , nBkmType  ( nType )
    , aPosition ( rPosition )
    {
    }
    ULONG getIndex ()
    {
        return aPosition.nContent.GetIndex();
    }
};

typedef boost::shared_ptr < SwXBookmarkPortion_Impl > SwXBookmarkPortion_ImplSharedPtr;
struct BookmarkCompareStruct
{
    bool operator () ( const SwXBookmarkPortion_ImplSharedPtr &r1,
                       const SwXBookmarkPortion_ImplSharedPtr &r2 )
    {
        // #i16896# for bookmark portions at the same position, the start should
        // always precede the end. Hence compare positions, and use bookmark type
        // as tie-breaker for same position.
        // return ( r1->nIndex   == r2->nIndex )
        //   ? ( r1->nBkmType <  r2->nBkmType )
        //   : ( r1->nIndex   <  r2->nIndex );

        // MTG: 25/11/05: Note that the above code does not correctly handle
        // the case when one bookmark ends, and another begins in the same
        // position. When this occurs, the above code will return the
        // the start of the 2nd bookmark BEFORE the end of the first bookmark
        // See bug #i58438# for more details. The below code is correct and
        // fixes both #i58438 and #i16896#
        return r1->aPosition < r2->aPosition;
    }
};
typedef std::multiset < SwXBookmarkPortion_ImplSharedPtr, BookmarkCompareStruct > SwXBookmarkPortion_ImplList;

//-----------------------------------------------------------------------------
void lcl_ExportBookmark(
    SwXBookmarkPortion_ImplList& rBkmArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> & rParent, XTextRangeArr& rPortionArr)
{
    for ( SwXBookmarkPortion_ImplList::iterator aIter = rBkmArr.begin(), aEnd = rBkmArr.end();
          aIter != aEnd; )
    {
        SwXBookmarkPortion_ImplSharedPtr pPtr = (*aIter);
        if ( nIndex > pPtr->getIndex() )
        {
            rBkmArr.erase( aIter++ );
            continue;
        }
        if ( nIndex < pPtr->getIndex() )
            break;

        SwXTextPortion* pPortion = 0;
        if(BKM_TYPE_START == pPtr->nBkmType || BKM_TYPE_START_END == pPtr->nBkmType)
        {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_BOOKMARK_START)),
                rPortionArr.Count());
            pPortion->SetBookmark(pPtr->xBookmark);
            pPortion->SetCollapsed(BKM_TYPE_START_END == pPtr->nBkmType ? TRUE : FALSE);

        }
        if(BKM_TYPE_END == pPtr->nBkmType)
        {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_BOOKMARK_END)),
                rPortionArr.Count());
            pPortion->SetBookmark(pPtr->xBookmark);
        }
        rBkmArr.erase( aIter++ );
    }
}

void lcl_ExportSoftPageBreak(
    SwSoftPageBreakList& rBreakArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> & rParent, XTextRangeArr& rPortionArr)
{
    for ( SwSoftPageBreakList::iterator aIter = rBreakArr.begin(), aEnd = rBreakArr.end();
          aIter != aEnd; )
    {
        if ( nIndex > *aIter )
        {
            rBreakArr.erase( aIter++ );
            continue;
        }
        if ( nIndex < *aIter )
            break;

        rPortionArr.Insert(
            new Reference< XTextRange >(new SwXTextPortion(pUnoCrsr, rParent, PORTION_SOFT_PAGEBREAK)),
            rPortionArr.Count());
        rBreakArr.erase( aIter++ );
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

    SwXRedlinePortion_Impl ( const SwRedline* pRed, sal_Bool bIsStart )
    : pRedline(pRed)
    , bStart(bIsStart)
    {
    }
    ULONG getRealIndex ()
    {
        return bStart ? pRedline->Start()->nContent.GetIndex() :
                        pRedline->End()  ->nContent.GetIndex();
    }
};

typedef boost::shared_ptr < SwXRedlinePortion_Impl > SwXRedlinePortion_ImplSharedPtr;
struct RedlineCompareStruct
{
    const SwPosition& getPosition ( const SwXRedlinePortion_ImplSharedPtr &r )
    {
        return *(r->bStart ? r->pRedline->Start() : r->pRedline->End());
    }
    bool operator () ( const SwXRedlinePortion_ImplSharedPtr &r1,
                       const SwXRedlinePortion_ImplSharedPtr &r2 )
    {
        return getPosition ( r1 ) < getPosition ( r2 );
    }
};
typedef std::multiset < SwXRedlinePortion_ImplSharedPtr, RedlineCompareStruct > SwXRedlinePortion_ImplList;

//-----------------------------------------------------------------------------
Reference<XTextRange> lcl_ExportHints(SwpHints* pHints,
                                XTextRangeArr& rPortionArr,
                                SwUnoCrsr* pUnoCrsr,
                                Reference<XText> & rParent,
                                const xub_StrLen nCurrentIndex,
                                SwTextPortionType& ePortionType,
                                const xub_StrLen& nFirstFrameIndex,
                                SwXBookmarkPortion_ImplList& aBkmArr,
                                SwXRedlinePortion_ImplList& aRedArr,
                                SwSoftPageBreakList& aBreakArr,
                                sal_Int32 nEndPos )
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
                            rPortionArr, pUnoCrsr, rParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;
                    case RES_TXTATR_REFMARK:
                        lcl_InsertRefMarkPortion(
                            rPortionArr, pUnoCrsr, rParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;
                    case RES_TXTATR_CJK_RUBY:
                        lcl_InsertRubyPortion(
                            rPortionArr, pUnoCrsr, rParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;
                }
            }
        }
        nEndIndex++;
    }
    //#111716# the cursor must not move right at the end position of a selection!
    BOOL bRightMoveForbidden = FALSE;
    if(nEndPos > 0 && nCurrentIndex >= nEndPos)
        bRightMoveForbidden = TRUE;

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
                    if(!bRightMoveForbidden)
                    {
                        pUnoCrsr->Right(1,CRSR_SKIP_CHARS,FALSE,FALSE);
                        if( *pUnoCrsr->GetMark() == *pUnoCrsr->GetPoint() )
                            break;
                        bAttrFound = sal_True;
                        ePortionType = PORTION_FIELD;
                    }
                break;
                case RES_TXTATR_FLYCNT   :
                    if(!bRightMoveForbidden)
                    {
                        pUnoCrsr->Right(1,CRSR_SKIP_CHARS,FALSE,FALSE);
                        if( *pUnoCrsr->GetMark() == *pUnoCrsr->GetPoint() )
                            break; // Robust #i81708 content in covered cells
                        pUnoCrsr->Exchange();
                        bAttrFound = sal_True;
                        ePortionType = PORTION_FRAME;
                    }
                break;
                case RES_TXTATR_FTN      :
                {
                    if(!bRightMoveForbidden)
                    {
                        pUnoCrsr->Right(1,CRSR_SKIP_CHARS,FALSE,FALSE);
                        if( *pUnoCrsr->GetMark() == *pUnoCrsr->GetPoint() )
                            break;
                        SwXTextPortion* pPortion;
                        xRef =  pPortion = new SwXTextPortion(pUnoCrsr, rParent, PORTION_FOOTNOTE);
                        Reference<XTextContent> xContent =
                            Reference<XTextContent>(
                            SwXFootnotes::GetObject(*pDoc, pAttr->SwTxtAttr::GetFtn()),
                            UNO_QUERY);
                        pPortion->SetFootnote(xContent);
                        bAttrFound = sal_True;
                        ePortionType = PORTION_TEXT;
                    }
                }
                break;
                case RES_TXTATR_SOFTHYPH :
                {
                    SwXTextPortion* pPortion = 0;
                    rPortionArr.Insert(
                        new Reference< XTextRange >(
                            pPortion = new SwXTextPortion(
                                pUnoCrsr, rParent, PORTION_CONTROL_CHAR)),
                        rPortionArr.Count());
                    pPortion->SetControlChar(3);
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_HARDBLANK:
                {
                    ePortionType = PORTION_CONTROL_CHAR;
                    SwXTextPortion* pPortion = 0;
                    rPortionArr.Insert(
                        new Reference< XTextRange >(
                            pPortion = new SwXTextPortion(
                                pUnoCrsr, rParent, PORTION_CONTROL_CHAR)),
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
                        rPortionArr, pUnoCrsr, rParent, pAttr, FALSE);
                    ePortionType = PORTION_TEXT;
                break;
                case RES_TXTATR_REFMARK:

                    if(!bRightMoveForbidden || pAttr->GetEnd())
                    {
                        if(!pAttr->GetEnd())
                        {
                            pUnoCrsr->Right(1,CRSR_SKIP_CHARS,FALSE,FALSE);
                            bAttrFound = sal_True;
                        }
                        lcl_InsertRefMarkPortion(
                            rPortionArr, pUnoCrsr, rParent, pAttr, FALSE);
                        ePortionType = PORTION_TEXT;
                        if(!pAttr->GetEnd())
                        {
                            if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                                    pUnoCrsr->Exchange();
                            pUnoCrsr->DeleteMark();
                        }
                    }
                break;
                case RES_TXTATR_CJK_RUBY:
                    if(pAttr->GetEnd())
                    {
                        lcl_InsertRubyPortion(
                            rPortionArr, pUnoCrsr, rParent, pAttr, FALSE);
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
        // search for attribute changes behind the current cursor position
        // break up at frames, bookmarks, redlines

        nStartIndex = 0;
        nNextStart = 0;
        while(nStartIndex < pHints->GetStartCount() &&
            nCurrentIndex >= (nNextStart = (*pHints->GetStart(nStartIndex)->GetStart())))
            nStartIndex++;

        nEndIndex = 0;
        nNextEnd = 0;
        while(nEndIndex < pHints->GetEndCount() &&
            nCurrentIndex >= (nNextEnd = (*pHints->GetEnd(nEndIndex)->GetAnyEnd())))
            nEndIndex++;
        //nMovePos legt die neue EndPosition fest
        sal_uInt16 nMovePos = nNextStart > nCurrentIndex && nNextStart < nNextEnd ? nNextStart : nNextEnd;
        if (nMovePos <= nCurrentIndex)
            nMovePos = pUnoCrsr->GetCntntNode()->Len();

        if(nEndPos >= 0 && nMovePos > nEndPos)
            nMovePos = (USHORT)nEndPos;

        if(aBkmArr.size() && (*aBkmArr.begin())->getIndex() < nMovePos)
        {
            DBG_ASSERT((*aBkmArr.begin())->getIndex() > nCurrentIndex,
                "forgotten bookmark(s)")
            nMovePos = (sal_uInt16)(*aBkmArr.begin())->getIndex();
        }
        // break up portions for redlines
        if (aRedArr.size() && (*aRedArr.begin())->getRealIndex() < nMovePos)
        {
            nMovePos = (sal_uInt16)(*aRedArr.begin())->getRealIndex();
        }
        // break up portions for soft page breaks
        if (aBreakArr.size() && *aBreakArr.begin() < nMovePos)
        {
            nMovePos = *aBreakArr.begin();
        }
        // break up if the destination is behind a frame
        if(nFirstFrameIndex != STRING_MAXLEN && nMovePos > nFirstFrameIndex)
            nMovePos = nFirstFrameIndex;

        if(nMovePos > nCurrentIndex)
//          pUnoCrsr->Right(nMovePos - nCurrentIndex);
            pUnoCrsr->GetPoint()->nContent = nMovePos;
        else if(nEndPos < 0 || nCurrentIndex < nEndPos)
        {
            // ensure proper exit: move to paragraph end
            // (this should not be necessary any more; we assert it only
            //  happens when the above would move to the end of the
            //  paragraph anyway)
            DBG_ASSERT(nMovePos == pUnoCrsr->GetCntntNode()->Len()||
            (nEndPos > 0 && nMovePos == nEndPos),
                       "may only happen at paragraph end");
            pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
        }
    }
    return xRef;
}
//-----------------------------------------------------------------------------
void lcl_FillBookmarkArray(SwDoc& rDoc,SwUnoCrsr& rUnoCrsr, SwXBookmarkPortion_ImplList& rBkmArr )
{
    const SwBookmarks& rMarks = rDoc.getBookmarks();
    sal_uInt16 nArrLen = rMarks.Count();
    if ( nArrLen > 0 )
    {
        const SwNodeIndex nOwnNode = rUnoCrsr.GetPoint()->nNode;
        //search for all bookmarks that start or end in this paragraph
        for( sal_uInt16 n = 0; n < nArrLen; ++n )
        {
            SwBookmark* pMark = rMarks.GetObject( n );
            // --> OD 2007-10-23 #i81002#
            if ( !pMark->IsBookMark() &&
                 !dynamic_cast<SwCrossRefBookmark*>(pMark) )
                continue;

            const SwPosition& rPos1 = pMark->GetBookmarkPos();
            // --> OD 2007-10-23 #i81002#
//            const SwPosition* pPos2 = pMark->GetOtherBookmarkPos();
            const SwPosition* pPos2( 0 );
            SwPosition* pCrossRefBkmkPos2( 0 );
            if ( dynamic_cast<SwCrossRefBookmark*>(pMark) )
            {
                pCrossRefBkmkPos2 = new SwPosition( pMark->GetBookmarkPos() );
                pCrossRefBkmkPos2->nContent =
                        pCrossRefBkmkPos2->nNode.GetNode().GetTxtNode()->Len();
                pPos2 = pCrossRefBkmkPos2;
            }
            else
            {
                pPos2 = pMark->GetOtherBookmarkPos();
            }
            // <--
            BOOL bBackward = pPos2 ? rPos1 > *pPos2: FALSE;
            if(rPos1.nNode == nOwnNode)
            {
                BYTE nType = bBackward ? BKM_TYPE_END : BKM_TYPE_START;
                if(!pPos2)
                {
                    nType = BKM_TYPE_START_END;
                }

                rBkmArr.insert ( SwXBookmarkPortion_ImplSharedPtr (
                    new SwXBookmarkPortion_Impl ( SwXBookmarks::GetObject( *pMark, &rDoc ), nType, rPos1 )));

            }
            if(pPos2 && pPos2->nNode == nOwnNode)
            {
                BYTE nType = bBackward ? BKM_TYPE_START : BKM_TYPE_END;
                rBkmArr.insert( SwXBookmarkPortion_ImplSharedPtr (
                    new SwXBookmarkPortion_Impl( SwXBookmarks::GetObject( *pMark, &rDoc ), nType, *pPos2 ) ) );
            }
            // --> OD 2007-10-23 #i81002#
            delete pCrossRefBkmkPos2;
            // <--
        }
    }
}
//-----------------------------------------------------------------------------
void lcl_FillRedlineArray(SwDoc& rDoc,SwUnoCrsr& rUnoCrsr, SwXRedlinePortion_ImplList& rRedArr )
{
    const SwRedlineTbl& rRedTbl = rDoc.GetRedlineTbl();
    USHORT nRedTblCount = rRedTbl.Count();

    if ( nRedTblCount > 0 )
    {
        const SwPosition* pStart = rUnoCrsr.GetPoint();
        const SwNodeIndex nOwnNode = pStart->nNode;

        for(USHORT nRed = 0; nRed < nRedTblCount; nRed++)
        {
            const SwRedline* pRedline = rRedTbl[nRed];
            const SwPosition* pRedStart = pRedline->Start();
            const SwNodeIndex nRedNode = pRedStart->nNode;
            if ( nOwnNode == nRedNode )
                rRedArr.insert( SwXRedlinePortion_ImplSharedPtr (
                    new SwXRedlinePortion_Impl ( pRedline, TRUE) ) );
            if( pRedline->HasMark() && pRedline->End()->nNode == nOwnNode )
                rRedArr.insert( SwXRedlinePortion_ImplSharedPtr (
                    new SwXRedlinePortion_Impl ( pRedline, FALSE) ) );
       }
    }
}

//-----------------------------------------------------------------------------
void lcl_FillSoftPageBreakArray( SwUnoCrsr& rUnoCrsr, SwSoftPageBreakList& rBreakArr )
{
    const SwTxtNode *pTxtNode = rUnoCrsr.GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNode )
        pTxtNode->fillSoftPageBreakList( rBreakArr );
}

/* -----------------------------19.12.00 12:25--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_ExportRedline(
    SwXRedlinePortion_ImplList& rRedlineArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> & rParent, XTextRangeArr& rPortionArr)
{

    // MTG: 23/11/05: We want this loop to iterate over all red lines in this
    // array. We will only insert the ones with index matches
    for ( SwXRedlinePortion_ImplList::iterator aIter = rRedlineArr.begin(), aEnd = rRedlineArr.end();
          aIter != aEnd; )
    {
        SwXRedlinePortion_ImplSharedPtr pPtr = (*aIter );
        ULONG nRealIndex = pPtr->getRealIndex();
        // MTG: 23/11/05: If there are elements before nIndex, remove them
        if ( nIndex > nRealIndex )
            rRedlineArr.erase ( aIter++ );
        // MTG: 23/11/05: If the elements match, and them to the list
        else if ( nIndex == nRealIndex )
        {
            rPortionArr.Insert(
                new Reference< XTextRange >( new SwXRedlinePortion(
                            pPtr->pRedline, *pUnoCrsr, rParent,
                            pPtr->bStart)),
                rPortionArr.Count());
            rRedlineArr.erase ( aIter++ );
        }
        // MTG: 23/11/05: If we've iterated past nIndex, exit the loop
        else
            break;
    }
}
/* -----------------------------19.12.00 13:09--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_ExportBkmAndRedline(
    SwXBookmarkPortion_ImplList& rBkmArr,
    SwXRedlinePortion_ImplList& rRedlineArr,
    SwSoftPageBreakList& rBreakArr,
    ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> & rParent, XTextRangeArr& rPortionArr)
{
    if (rBkmArr.size())
        lcl_ExportBookmark(rBkmArr, nIndex, pUnoCrsr, rParent, rPortionArr);

    if (rRedlineArr.size())
        lcl_ExportRedline(rRedlineArr, nIndex, pUnoCrsr, rParent, rPortionArr);

    if (rBreakArr.size())
        lcl_ExportSoftPageBreak(rBreakArr, nIndex, pUnoCrsr, rParent, rPortionArr);
}
//-----------------------------------------------------------------------------
sal_Int32 lcl_GetNextIndex(SwXBookmarkPortion_ImplList& rBkmArr,
    SwXRedlinePortion_ImplList& rRedlineArr,
    SwSoftPageBreakList& rBreakArr )
{
    sal_Int32 nRet = -1;
    if(rBkmArr.size())
    {
        SwXBookmarkPortion_ImplSharedPtr pPtr = (*rBkmArr.begin());
        nRet = pPtr->getIndex();
    }
    if(rRedlineArr.size())
    {
        SwXRedlinePortion_ImplSharedPtr pPtr = (*rRedlineArr.begin());
        sal_Int32 nTmp = pPtr->getRealIndex();
        if(nRet < 0 || nTmp < nRet)
            nRet = nTmp;
    }
    if(rBreakArr.size())
    {
        if(nRet < 0 || *rBreakArr.begin() < static_cast<sal_uInt32>(nRet))
            nRet = *rBreakArr.begin();
    }
    return nRet;
};
//-----------------------------------------------------------------------------
void SwXTextPortionEnumeration::CreatePortions()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    // set the start if a selection should be exported
    if(nStartPos > 0 && pUnoCrsr->Start()->nContent.GetIndex() != nStartPos)
    {
        if(pUnoCrsr->HasMark())
            pUnoCrsr->DeleteMark();
        DBG_ASSERT(pUnoCrsr->Start()->nNode.GetNode().GetTxtNode() &&
            nStartPos <= pUnoCrsr->Start()->nNode.GetNode().GetTxtNode()->GetTxt().Len(),
                "Incorrect start position"  )
        pUnoCrsr->Right((xub_StrLen)nStartPos,CRSR_SKIP_CHARS,FALSE,FALSE);
    }
    if(pUnoCrsr /*&& !bAtEnd*/)
    {
        SwXBookmarkPortion_ImplList aBkmArr;
        SwXRedlinePortion_ImplList aRedArr;
        SwSoftPageBreakList aBreakArr;

        SwDoc* pDoc = pUnoCrsr->GetDoc();
        lcl_FillRedlineArray(*pDoc, *pUnoCrsr, aRedArr);
        lcl_FillBookmarkArray(*pDoc, *pUnoCrsr, aBkmArr );
        lcl_FillSoftPageBreakArray( *pUnoCrsr, aBreakArr );
#if OSL_DEBUG_LEVEL > 1
        for (SwXBookmarkPortion_ImplList::const_iterator aIter = aBkmArr.begin(), aEnd = aBkmArr.end();
             aIter != aEnd;
             ++aIter )
        {
            SwXBookmarkPortion_ImplSharedPtr pPtr = (*aIter);
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
            SwNode* pNode = pUnoCrsr->GetNode();
            SwCntntNode *pCNd = pNode->GetCntntNode();
            if(!bFirstPortion   && pCNd &&
                    pUnoCrsr->GetPoint()->nContent == pCNd->Len())
            {
                //hier sollte man nie ankommen!
                bAtEnd = sal_True;
            }
            else
            {
                if(ND_TEXTNODE == pNode->GetNodeType())
                {
                    SwTxtNode* pTxtNode = (SwTxtNode*)pNode;
                    SwpHints* pHints = pTxtNode->GetpSwpHints();
                    SwTextPortionType ePortionType = PORTION_TEXT;
                    xub_StrLen nCurrentIndex = pUnoCrsr->GetPoint()->nContent.GetIndex();
                    xub_StrLen nFirstFrameIndex = STRING_MAXLEN;
                    uno::Reference< XTextRange >  xRef;
                    if(!pCNd->Len())
                    {
                        lcl_ExportBkmAndRedline(aBkmArr, aRedArr, aBreakArr, 0, pUnoCrsr, xParent, aPortionArr);
                        // the paragraph is empty
                        xRef = new SwXTextPortion(pUnoCrsr, xParent, ePortionType);
                        // are there any frames?
                        while(aFrameArr.Count())
                        {
                            SwDepend* pCurDepend = aFrameArr.GetObject(0);
                            if(pCurDepend->GetRegisteredIn())
                            {
                                //the previously created portion has to be inserted here
                                aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                                xRef = new SwXTextPortion(pUnoCrsr, xParent,
                                    *(SwFrmFmt*)pCurDepend->GetRegisteredIn());
                            }
                            delete pCurDepend;
                            aFrameArr.Remove(0);
                        }
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
                            if(nEndPos >= 0 && nFirstFrameIndex >= nEndPos)
                                nFirstFrameIndex = USHRT_MAX;
                        }

                        SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);

                        //ist hier schon ein Rahmen faellig?
                        if(nCurrentIndex == nFirstFrameIndex)
                        {
                            xRef = new SwXTextPortion(pUnoCrsr, xParent,
                                *(SwFrmFmt*)pFirstFrameDepend->GetRegisteredIn());
                            SwDepend* pCurDepend = aFrameArr.GetObject(0);
                            delete pCurDepend;
                            aFrameArr.Remove(0);
                        }
                    }
                    if(!xRef.is())
                    {
                        lcl_ExportBkmAndRedline(aBkmArr, aRedArr, aBreakArr, nCurrentIndex, pUnoCrsr, xParent, aPortionArr);
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
                                aRedArr,
                                aBreakArr,
                                nEndPos);

                        }
                        else if(USHRT_MAX != nFirstFrameIndex)
                        {
                            pUnoCrsr->Right(nFirstFrameIndex - nCurrentIndex,CRSR_SKIP_CHARS,FALSE,FALSE);
                        }
                        else
                        {
                            sal_Int32 nNextIndex = lcl_GetNextIndex(aBkmArr, aRedArr, aBreakArr);
                            DBG_ASSERT( nNextIndex <= pCNd->Len(), "illegal next index" );
                            if( nNextIndex > pCNd->Len() )
                            {
                                nNextIndex = pCNd->Len();
                                bAtEnd = sal_True;
                            }
                            if(nEndPos >= 0 && (nNextIndex > nEndPos || nNextIndex < 0))
                            {
                                nNextIndex = nEndPos;
                                bAtEnd = sal_True;
                            }
                            if(nNextIndex < 0)
                            {
                                // a text portion should stay within it's paragraph (#i56165)
                                //sal_Bool bMove = pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
                                pUnoCrsr->GetPoint()->nContent = pCNd->Len();
                                bAtEnd = sal_True;
                            }
                            else
                            {
                                DBG_ASSERT(nNextIndex > nCurrentIndex || nNextIndex == nEndPos,
                                    "wrong move index")
                                pUnoCrsr->Right((sal_uInt16)(nNextIndex - nCurrentIndex),CRSR_SKIP_CHARS,FALSE,FALSE);
                            }
                        }
                    }
                    if(!xRef.is() && pUnoCrsr->HasMark() )
                        xRef = new SwXTextPortion(pUnoCrsr, xParent, ePortionType);
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
            pNode = pUnoCrsr->GetNode();
            pCNd = pNode->GetCntntNode();
            sal_Int32 nLocalEnd = nEndPos >= 0 ? nEndPos : pCNd->Len();
            if( pCNd && pUnoCrsr->GetPoint()->nContent >= (xub_StrLen)nLocalEnd)
            {
                bAtEnd = sal_True;
                lcl_ExportBkmAndRedline(aBkmArr, aRedArr, aBreakArr, nLocalEnd,
                                            pUnoCrsr, xParent, aPortionArr);
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
                            static_cast< xub_StrLen >(nLocalEnd),
                            ePortionType,
                            STRING_MAXLEN,
                            aBkmArr,
                            aRedArr,
                            aBreakArr,
                            nEndPos);
                        if(xRef.is())
                            aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                    }
                }
                while(aFrameArr.Count())
                {
                    SwDepend* pCurDepend = aFrameArr.GetObject(0);
                    if(pCurDepend->GetRegisteredIn())
                    {
                        Reference<XTextRange> xRef = new SwXTextPortion(pUnoCrsr, xParent,
                            *(SwFrmFmt*)pCurDepend->GetRegisteredIn());
                        aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                    }
                    delete pCurDepend;
                    aFrameArr.Remove(0);
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

