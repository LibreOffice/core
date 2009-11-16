/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unorefmk.cxx,v $
 * $Revision: 1.17 $
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
#include "precompiled_sw.hxx"


#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unoobj.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <hints.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

/******************************************************************
 * SwXReferenceMark
 ******************************************************************/

/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXReferenceMark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXReferenceMark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/* -----------------------------06.04.00 16:41--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXReferenceMark::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXReferenceMark");
}
/* -----------------------------06.04.00 16:41--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXReferenceMark::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.ReferenceMark")||
                !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}
/* -----------------------------06.04.00 16:41--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXReferenceMark::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ReferenceMark");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 10:28:32---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXReferenceMark, SwClient);

SwXReferenceMark::SwXReferenceMark(SwDoc* pDc, const SwFmtRefMark* pRefMark) :
    aLstnrCntnr( (text::XTextContent*)this),
    pDoc(pDc),
    pMark(pRefMark),
    m_bIsDescriptor(0 == pRefMark)
{
    if(pRefMark)
        sMarkName = pRefMark->GetRefName();
    if(pDoc)
        pDoc->GetUnoCallBack()->Add(this);
}
/*-- 11.12.98 10:28:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXReferenceMark::~SwXReferenceMark()
{

}
/* -----------------03.11.99 14:14-------------------

 --------------------------------------------------*/
void SwXReferenceMark::InsertRefMark(SwPaM& rPam, SwXTextCursor * pCursor)
{
    //! in some cases when this function is called the pDoc pointer member may have become
    //! invalid/deleted thus we obtain the document pointer from rPaM where it should always
    //! be valid.
    SwDoc *pDoc2 = rPam.GetDoc();

    UnoActionContext aCont(pDoc2);
    SwTxtAttr* pTxtAttr = 0;
    SwFmtRefMark aRefMark(sMarkName);
//  SfxItemSet  aSet(pDoc2->GetAttrPool(), RES_TXTATR_REFMARK, RES_TXTATR_REFMARK, 0L);
//  aSet.Put(aRefMark);
    sal_Bool bMark = *rPam.GetPoint() != *rPam.GetMark();
//    SwXTextCursor::SetCrsrAttr(rPam, aSet, 0);

    const bool bForceExpandHints( (!bMark && pCursor)
            ? pCursor->IsAtEndOfMeta() : false );
    const SetAttrMode nInsertFlags = (bForceExpandHints)
        ?   ( nsSetAttrMode::SETATTR_FORCEHINTEXPAND
            | nsSetAttrMode::SETATTR_DONTEXPAND)
        : nsSetAttrMode::SETATTR_DONTEXPAND;

    pDoc2->InsertPoolItem( rPam, aRefMark, nInsertFlags );

    if( bMark && *rPam.GetPoint() > *rPam.GetMark())
        rPam.Exchange();

    if( bMark )
        pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttr(
                rPam.GetPoint()->nContent, RES_TXTATR_REFMARK );
    else
    {
        pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
                rPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_REFMARK );
    }

    if(pTxtAttr)
        pMark = &pTxtAttr->GetRefMark();

    pDoc2->GetUnoCallBack()->Add(this);
}

/* -----------------18.02.99 13:33-------------------
 *
 * --------------------------------------------------*/
void SwXReferenceMark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }
    SwDoc* pDocument = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDocument)
    {
        SwUnoInternalPaM aPam(*pDocument);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        InsertRefMark(aPam, dynamic_cast<SwXTextCursor*>(pCursor));
        m_bIsDescriptor = sal_False;
        pDoc = pDocument;
        pDoc->GetUnoCallBack()->Add(this);
    }
    else
        throw lang::IllegalArgumentException();
}
/*-- 11.12.98 10:28:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::attach(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    attachToRange( xTextRange );
}
/*-- 11.12.98 10:28:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXReferenceMark::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    if(IsValid())
    {
        const SwFmtRefMark* pNewMark = pDoc->GetRefMark(sMarkName);
        if(pNewMark && pNewMark == pMark)
        {
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                SwPaM* pPam  = pTxtMark->GetEnd() ?
                                new SwPaM( rTxtNode, *pTxtMark->GetEnd(),
                                    rTxtNode, *pTxtMark->GetStart()) :
                                new SwPaM(  rTxtNode, *pTxtMark->GetStart());


                xRet = SwXTextRange::CreateTextRangeFromPosition(pDoc,
                        *pPam->Start(), pPam->End());
                delete pPam;
            }
        }
    }
    return xRet;
}
/*-- 11.12.98 10:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        const SwFmtRefMark* pNewMark = pDoc->GetRefMark(sMarkName);
        if(pNewMark && pNewMark == pMark)
        {
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                xub_StrLen nStt = *pTxtMark->GetStart(),
                           nEnd = pTxtMark->GetEnd() ? *pTxtMark->GetEnd()
                                                     : nStt + 1;

                SwPaM aPam( rTxtNode, nStt, rTxtNode, nEnd );
                pDoc->DeleteAndJoin( aPam );
            }
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 10:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 10:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 10:28:36---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXReferenceMark::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid() || !pDoc->GetRefMark(sMarkName))
    {
        throw uno::RuntimeException();
    }
    return sMarkName;
}
/*-- 11.12.98 10:28:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::setName(const OUString& Name_) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(m_bIsDescriptor)
        sMarkName = String(Name_);
    else
    {
        String sNewName(Name_);
        if(!IsValid() || !pDoc->GetRefMark(sMarkName) || pDoc->GetRefMark(sNewName))
        {
            throw uno::RuntimeException();
        }
        const SwFmtRefMark* pCurMark = pDoc->GetRefMark(sMarkName);
        if(sNewName != sMarkName && pCurMark && pCurMark == pMark)
        {
            UnoActionContext aCont(pDoc);
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                xub_StrLen nStt = *pTxtMark->GetStart(),
                           nEnd = pTxtMark->GetEnd() ? *pTxtMark->GetEnd()
                                                     : nStt + 1;

                SwPaM aPam( rTxtNode, nStt, rTxtNode, nEnd );
                pDoc->DeleteAndJoin( aPam );    //! deletes the pDoc member in the SwXReferenceMark
                                                //! The aPam will keep the correct and functional doc though

                sMarkName = sNewName;
                //create a new one
                InsertRefMark( aPam, 0 );
                pDoc = aPam.GetDoc();
            }
        }
    }
}
/* -----------------------------07.01.00 12:51--------------------------------

 ---------------------------------------------------------------------------*/
void    SwXReferenceMark::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        aLstnrCntnr.Disposing();
        pDoc = 0;
        pMark = 0;
    }
}
/*-- 11.12.98 10:28:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;
    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            Invalidate();
        break;
    case RES_REFMARK_DELETED:
        if( (void*)pMark == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;
    }
}

/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo > SwXReferenceMark::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH_EXTENSIONS)->getPropertySetInfo();
    return xRef;
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::setPropertyValue(
    const OUString& /*rPropertyName*/, const uno::Any& /*rValue*/ )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    throw lang::IllegalArgumentException();
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXReferenceMark::getPropertyValue( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
        throw beans::UnknownPropertyException();
    return aRet;
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::addVetoableChangeListener( const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
/*-- 12.09.00 12:58:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

#include <com/sun/star/lang/DisposedException.hpp>
#include <unometa.hxx>
#include <unoport.hxx>
#include <txtatr.hxx>
#include <fmtmeta.hxx>
#include <docsh.hxx>

//=============================================================================

/******************************************************************
 * SwXMetaText
 ******************************************************************/

class SwXMetaText
    : public SwXText
{
private:
    SwXMeta & m_rMeta;

    virtual void PrepareForAttach(uno::Reference< text::XTextRange > & xRange,
            const SwXTextRange* const pRange, const SwPaM * const pPam);

    virtual bool CheckForOwnMemberMeta(const SwXTextRange* const pRange,
            const SwPaM* const pPam, bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

protected:
    virtual const SwStartNode *GetStartNode() const;
    virtual uno::Reference< text::XTextCursor >
        createCursor() throw (uno::RuntimeException);

public:
    SwXMetaText(SwDoc & rDoc, SwXMeta & rMeta);

    // XInterface
    virtual void SAL_CALL acquire() throw()
        { OSL_ENSURE(false, "ERROR: SwXMetaText::acquire"); }
    virtual void SAL_CALL release() throw()
        { OSL_ENSURE(false, "ERROR: SwXMetaText::release"); }

    // XTypeProvider
    virtual uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId() throw (uno::RuntimeException);

    // XText
    virtual uno::Reference< text::XTextCursor >  SAL_CALL
        createTextCursor() throw (uno::RuntimeException);
    virtual uno::Reference< text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const uno::Reference< text::XTextRange > & xTextPosition)
        throw (uno::RuntimeException);

};

SwXMetaText::SwXMetaText(SwDoc & rDoc, SwXMeta & rMeta)
    : SwXText(&rDoc, CURSOR_META)
    , m_rMeta(rMeta)
{
}

const SwStartNode *SwXMetaText::GetStartNode() const
{
    SwXText const * const pParent(
            dynamic_cast<SwXText*>(m_rMeta.GetParentText().get()));
    return (pParent) ? pParent->GetStartNode() : 0;
}

void SwXMetaText::PrepareForAttach( uno::Reference<text::XTextRange> & xRange,
        const SwXTextRange* const pRange, const SwPaM * const pPam)
{
    SwPosition const* pPoint(0);
    SwPosition const* pMark (0);
    if (pRange)
    {
        ::sw::mark::IMark const& rIMark(*pRange->GetBookmark());
        pMark = &rIMark.GetMarkPos();
        if (rIMark.IsExpanded())
        {
            pMark = &rIMark.GetOtherMarkPos();
        }
    }
    else if (pPam)
    {
        pPoint = pPam->GetPoint();
        if (pPam->HasMark())
        {
            pMark = pPam->GetMark();
        }
    }
    // create a new cursor to prevent modifying SwXTextRange
    if (pPoint)
    {
        xRange = static_cast<text::XWordCursor*>(
            new SwXTextCursor(&m_rMeta, *pPoint, CURSOR_META, GetDoc(), pMark));
    }
}

bool SwXMetaText::CheckForOwnMemberMeta(const SwXTextRange* const pRange,
                const SwPaM* const pPam, bool bAbsorb)
    throw (::com::sun::star::lang::IllegalArgumentException,
           ::com::sun::star::uno::RuntimeException)
{
    return m_rMeta.CheckForOwnMemberMeta(pRange, pPam, bAbsorb);
}

uno::Reference< text::XTextCursor > SwXMetaText::createCursor()
throw (uno::RuntimeException)
{
    uno::Reference< text::XTextCursor > xRet;
    if (IsValid())
    {
        SwTxtNode * pTxtNode;
        xub_StrLen nMetaStart;
        xub_StrLen nMetaEnd;
        const bool bSuccess(
                m_rMeta.SetContentRange(pTxtNode, nMetaStart, nMetaEnd) );
        if (bSuccess)
        {
            SwPosition aPos(*pTxtNode, nMetaStart);
            xRet = static_cast<text::XWordCursor*>(
                    new SwXTextCursor(&m_rMeta, aPos, CURSOR_META, GetDoc()));
        }
    }
    return xRet;
}

uno::Sequence<sal_Int8> SAL_CALL
SwXMetaText::getImplementationId() throw (uno::RuntimeException)
{
    return m_rMeta.getImplementationId();
}

// XText
uno::Reference< text::XTextCursor > SAL_CALL
SwXMetaText::createTextCursor() throw (uno::RuntimeException)
{
    return createCursor();
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXMetaText::createTextCursorByRange(
        const uno::Reference<text::XTextRange> & xTextPosition)
    throw (uno::RuntimeException)
{
    const uno::Reference<text::XTextCursor> xCursor( createCursor() );
    xCursor->gotoRange(xTextPosition, sal_False);
    return xCursor;
}

/******************************************************************
 * SwXMeta
 ******************************************************************/

// the Meta has a cached list of text portions for its contents
// this list is created by SwXTextPortionEnumeration
// the Meta listens at the SwTxtNode and throws away the cache when it changes

struct SwXMeta::Impl
{
    SwEventListenerContainer m_ListenerContainer;
    ::std::auto_ptr<const TextRangeList_t> m_pTextPortions;
    // 3 possible states: not attached, attached, disposed
    bool m_bIsDisposed;
    bool m_bIsDescriptor;
    uno::Reference<text::XText> m_xParentText;
    SwXMetaText m_Text;

    Impl(SwXMeta & rThis, SwDoc & rDoc,
            uno::Reference<text::XText> const& xParentText,
            TextRangeList_t const * const pPortions,
            SwTxtMeta const * const pHint)
        : m_ListenerContainer(
                static_cast< ::cppu::OWeakObject* >(&rThis))
        , m_pTextPortions( pPortions )
        , m_bIsDisposed( false )
        , m_bIsDescriptor( 0 == pHint )
        , m_xParentText(xParentText)
        , m_Text(rDoc, rThis)
    {
    }
};

TYPEINIT1(SwXMeta, SwClient);

inline const ::sw::Meta * SwXMeta::GetMeta() const
{
    return static_cast< const ::sw::Meta * >(GetRegisteredIn());
}

uno::Reference<text::XText> SwXMeta::GetParentText() const
{
    return m_pImpl->m_xParentText;
}

bool SwXMeta::SetContentRange(
        SwTxtNode *& rpNode, xub_StrLen & rStart, xub_StrLen & rEnd ) const
{
    ::sw::Meta const * const pMeta( GetMeta() );
    if (pMeta)
    {
        SwTxtMeta const * const pTxtAttr( pMeta->GetTxtAttr() );
        if (pTxtAttr)
        {
            rpNode = pTxtAttr->GetTxtNode();
            if (rpNode)
            {
                // rStart points at the first position _within_ the meta!
                rStart = *pTxtAttr->GetStart() + 1;
                rEnd = *pTxtAttr->GetEnd();
                return true;
            }
        }
    }
    return false;
}

SwXMeta::SwXMeta(SwDoc *const pDoc,
        uno::Reference<text::XText> const& xParentText,
        TextRangeList_t * const pPortions, SwTxtMeta * const pHint)
    : m_pImpl( new SwXMeta::Impl(*this, *pDoc, xParentText, pPortions, pHint) )
{
    if (pHint)
    {
        ::sw::Meta * const pMeta(
            static_cast<SwFmtMeta&>(pHint->GetAttr()).GetMeta() );
        ASSERT(pMeta, "SwXMeta: no meta?")
        if (pMeta)
        {
            pMeta->Add(this);
        }
    }
}

bool SwXMeta::CheckForOwnMemberMeta(const SwXTextRange* const pRange,
                const SwPaM* const pPam, bool bAbsorb)
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    ASSERT((pPam && !pRange) || (!pPam && pRange), "ERROR: pam xor range");
    SwTxtNode * pTxtNode;
    xub_StrLen nMetaStart;
    xub_StrLen nMetaEnd;
    const bool bSuccess( SetContentRange(pTxtNode, nMetaStart, nMetaEnd) );
    ASSERT(bSuccess, "no pam?");
    if (!bSuccess)
        throw lang::DisposedException();
    SwPosition const * const pStartPos( (pPam)
        ? pPam->Start()
        : &pRange->GetBookmark()->GetMarkStart() );
    if (&pStartPos->nNode.GetNode() != pTxtNode)
    {
        throw lang::IllegalArgumentException(
            C2U("trying to insert into a nesting text content, but start "
                "of text range not in same paragraph as text content"),
                0, 0);
    }
    bool bForceExpandHints(false);
    const xub_StrLen nStartPos(pStartPos->nContent.GetIndex());
    // not <= but < because nMetaStart is behind dummy char!
    // not >= but > because == means insert at end!
    if ((nStartPos < nMetaStart) || (nStartPos > nMetaEnd))
    {
        throw lang::IllegalArgumentException(
            C2U("trying to insert into a nesting text content, but start "
                "of text range not inside text content"),
                0, 0);
    }
    else if (nStartPos == nMetaEnd)
    {
        bForceExpandHints = true;
    }
    const bool bHasEnd( (pPam)
        ? pPam->HasMark()
        : pRange->GetBookmark()->IsExpanded());
    if (bHasEnd && bAbsorb)
    {
        SwPosition const * const pEndPos( (pPam)
            ? pPam->End()
            : &pRange->GetBookmark()->GetMarkEnd() );
        if (&pEndPos->nNode.GetNode() != pTxtNode)
        {
            throw lang::IllegalArgumentException(
                C2U("trying to insert into a nesting text content, but end "
                    "of text range not in same paragraph as text content"),
                    0, 0);
        }
        const xub_StrLen nEndPos(pEndPos->nContent.GetIndex());
        // not <= but < because nMetaStart is behind dummy char!
        // not >= but > because == means insert at end!
        if ((nEndPos < nMetaStart) || (nEndPos > nMetaEnd))
        {
            throw lang::IllegalArgumentException(
                C2U("trying to insert into a nesting text content, but end "
                    "of text range not inside text content"),
                    0, 0);
        }
        else if (nEndPos == nMetaEnd)
        {
            bForceExpandHints = true;
        }
    }
    return bForceExpandHints;
}


SwXMeta::SwXMeta(SwDoc *const pDoc)
    : m_pImpl( new SwXMeta::Impl(*this, *pDoc, 0, 0, 0) )
{
}

SwXMeta::~SwXMeta()
{
}

const uno::Sequence< sal_Int8 > & SwXMeta::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq( ::CreateUnoTunnelId() );
    return aSeq;
}

// XUnoTunnel
sal_Int64 SAL_CALL
SwXMeta::getSomething( const uno::Sequence< sal_Int8 > & i_rId )
throw (uno::RuntimeException)
{
    if ( i_rId.getLength() == 16 &&
         0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                 i_rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >(
            reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

// XServiceInfo
::rtl::OUString SAL_CALL
SwXMeta::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXMeta");
}

sal_Bool SAL_CALL
SwXMeta::supportsService(const ::rtl::OUString& rServiceName)
throw (uno::RuntimeException)
{
    return rServiceName.equalsAscii("com.sun.star.text.TextContent")
        || rServiceName.equalsAscii("com.sun.star.text.InContentMetadata");
}

uno::Sequence< ::rtl::OUString > SAL_CALL
SwXMeta::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = C2U("com.sun.star.text.TextContent");
    aRet[1] = C2U("com.sun.star.text.InContentMetadata");
    return aRet;
}


// XComponent
void SAL_CALL
SwXMeta::addEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    m_pImpl->m_ListenerContainer.AddListener(xListener);
    if (m_pImpl->m_bIsDisposed)
    {
        m_pImpl->m_ListenerContainer.Disposing();
    }
}

void SAL_CALL
SwXMeta::removeEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!m_pImpl->m_bIsDisposed)
    {
        m_pImpl->m_ListenerContainer.RemoveListener(xListener);
    }
}

void SAL_CALL
SwXMeta::dispose() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_pTextPortions.reset();
        m_pImpl->m_ListenerContainer.Disposing();
        m_pImpl->m_bIsDisposed = true;
        m_pImpl->m_Text.Invalidate();
    }
    else if (!m_pImpl->m_bIsDisposed)
    {
        SwTxtNode * pTxtNode;
        xub_StrLen nMetaStart;
        xub_StrLen nMetaEnd;
        const bool bSuccess(SetContentRange(pTxtNode, nMetaStart, nMetaEnd));
        ASSERT(bSuccess, "no pam?");
        if (bSuccess)
        {
            // -1 because of CH_TXTATR
            SwPaM aPam( *pTxtNode, nMetaStart - 1, *pTxtNode, nMetaEnd );
            SwDoc * const pDoc( pTxtNode->GetDoc() );
            pDoc->DeleteAndJoin( aPam );

            // removal should call Modify and do the dispose
            OSL_ENSURE(m_pImpl->m_bIsDisposed, "zombie meta");
        }
    }
}


void SAL_CALL
SwXMeta::AttachImpl(const uno::Reference< text::XTextRange > & i_xTextRange,
        const USHORT i_nWhich)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException(
            C2S("SwXMeta::attach(): already attached"),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel(i_xTextRange, uno::UNO_QUERY);
    if (!xRangeTunnel.is())
    {
        throw lang::IllegalArgumentException(
            C2S("SwXMeta::attach(): argument is no XUnoTunnel"),
                static_cast< ::cppu::OWeakObject* >(this), 0);
    }
    SwXTextRange * const pRange(
        reinterpret_cast< SwXTextRange * >(
            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething(
                SwXTextRange::getUnoTunnelId() ))) );
    OTextCursorHelper * const pCursor( pRange ? 0 :
        reinterpret_cast< OTextCursorHelper * >(
            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething(
                OTextCursorHelper::getUnoTunnelId() ))) );
    if (!pRange && !pCursor)
    {
        throw lang::IllegalArgumentException(
            C2S("SwXMeta::attach(): argument not supported type"),
                static_cast< ::cppu::OWeakObject* >(this), 0);
    }

    SwDoc * const pDoc(
            pRange ? pRange->GetDoc() : pCursor ? pCursor->GetDoc() : 0 );
    if (!pDoc)
    {
        throw lang::IllegalArgumentException(
            C2S("SwXMeta::attach(): argument has no SwDoc"),
                static_cast< ::cppu::OWeakObject* >(this), 0);
    }

    SwUnoInternalPaM aPam(*pDoc);
    SwXTextRange::XTextRangeToSwPaM(aPam, i_xTextRange);

    UnoActionContext aContext(pDoc);

    SwXTextCursor const*const pTextCursor(
            dynamic_cast<SwXTextCursor*>(pCursor));
    const bool bForceExpandHints((pTextCursor)
            ? pTextCursor->IsAtEndOfMeta() : false);
    const SetAttrMode nInsertFlags( (bForceExpandHints)
        ?   ( nsSetAttrMode::SETATTR_FORCEHINTEXPAND
            | nsSetAttrMode::SETATTR_DONTEXPAND)
        : nsSetAttrMode::SETATTR_DONTEXPAND );

    const ::boost::shared_ptr< ::sw::Meta> pMeta( (RES_TXTATR_META == i_nWhich)
        ? ::boost::shared_ptr< ::sw::Meta>( new ::sw::Meta() )
        : ::boost::shared_ptr< ::sw::Meta>(
            pDoc->GetMetaFieldManager().makeMetaField()) );
    SwFmtMeta meta(pMeta, i_nWhich); // this is cloned by Insert!
    const bool bSuccess( pDoc->InsertPoolItem( aPam, meta, nInsertFlags ) );
    SwTxtAttr * const pTxtAttr( pMeta->GetTxtAttr() );
    if (!bSuccess)
    {
        throw lang::IllegalArgumentException(
            C2S("SwXMeta::attach(): cannot create meta: range invalid?"),
                static_cast< ::cppu::OWeakObject* >(this), 1);
    }
    if (!pTxtAttr)
    {
        ASSERT(false, "meta inserted, but has no text attribute?");
        throw uno::RuntimeException(
            C2S("SwXMeta::attach(): cannot create meta"),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    pMeta->Add(this);

    m_pImpl->m_xParentText =
        SwXTextRange::CreateParentXText(pDoc, *aPam.GetPoint());

    m_pImpl->m_bIsDescriptor = false;
}

// XTextContent
void SAL_CALL
SwXMeta::attach(const uno::Reference< text::XTextRange > & i_xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return SwXMeta::AttachImpl(i_xTextRange, RES_TXTATR_META);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMeta::getAnchor() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException(
                C2S("SwXMeta::getAnchor(): not inserted"),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    SwTxtNode * pTxtNode;
    xub_StrLen nMetaStart;
    xub_StrLen nMetaEnd;
    const bool bSuccess(SetContentRange(pTxtNode, nMetaStart, nMetaEnd));
    ASSERT(bSuccess, "no pam?");
    if (!bSuccess)
    {
        throw lang::DisposedException(
                C2S("SwXMeta::getAnchor(): not attached"),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    const SwPosition start(*pTxtNode, nMetaStart - 1); // -1 due to CH_TXTATR
    const SwPosition end(*pTxtNode, nMetaEnd);
    return SwXTextRange::CreateTextRangeFromPosition(
                pTxtNode->GetDoc(), start, &end);
}

// XTextRange
uno::Reference< text::XText > SAL_CALL
SwXMeta::getText() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    //TODO probably this should return outer meta in case there is nesting,
    // but currently that is not done; would need to change at least
    // SwXTextPortionEnumeration and SwXMeta::attach and other places where
    // SwXMeta is constructed
    return GetParentText();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMeta::getStart() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.getStart();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMeta::getEnd() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.getEnd();
}

rtl::OUString SAL_CALL
SwXMeta::getString() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.getString();
}

void SAL_CALL
SwXMeta::setString(const rtl::OUString& rString) throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.setString(rString);
}

// XSimpleText
uno::Reference< text::XTextCursor > SAL_CALL
SwXMeta::createTextCursor() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.createTextCursor();
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXMeta::createTextCursorByRange(
        const uno::Reference<text::XTextRange> & xTextPosition)
    throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.createTextCursorByRange(xTextPosition);
}

void SAL_CALL
SwXMeta::insertString(const uno::Reference<text::XTextRange> & xRange,
        const rtl::OUString& rString, sal_Bool bAbsorb)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.insertString(xRange, rString, bAbsorb);
}

void SAL_CALL
SwXMeta::insertControlCharacter(const uno::Reference<text::XTextRange> & xRange,
        sal_Int16 nControlCharacter, sal_Bool bAbsorb)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.insertControlCharacter(xRange, nControlCharacter,
                bAbsorb);
}

// XText
void SAL_CALL
SwXMeta::insertTextContent( const uno::Reference<text::XTextRange> & xRange,
        const uno::Reference<text::XTextContent> & xContent, sal_Bool bAbsorb)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.insertTextContent(xRange, xContent, bAbsorb);
}

void SAL_CALL
SwXMeta::removeTextContent(
        const uno::Reference< text::XTextContent > & xContent)
    throw (container::NoSuchElementException, uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());
    return m_pImpl->m_Text.removeTextContent(xContent);
}

// XElementAccess
uno::Type SAL_CALL
SwXMeta::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL
SwXMeta::hasElements() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    return GetRegisteredIn() ? sal_True : sal_False;
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL
SwXMeta::createEnumeration() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException(
                C2S("getAnchor(): not inserted"),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    SwTxtNode * pTxtNode;
    xub_StrLen nMetaStart;
    xub_StrLen nMetaEnd;
    const bool bSuccess(SetContentRange(pTxtNode, nMetaStart, nMetaEnd));
    ASSERT(bSuccess, "no pam?");
    if (!bSuccess)
        throw lang::DisposedException();

    SwPaM aPam(*pTxtNode, nMetaStart);

    if (!m_pImpl->m_pTextPortions.get())
    {
        return new SwXTextPortionEnumeration(
                    aPam, GetParentText(), nMetaStart, nMetaEnd);
    }
    else // cached!
    {
        return new SwXTextPortionEnumeration(aPam, *m_pImpl->m_pTextPortions);
    }
}


// MetadatableMixin
::sfx2::Metadatable* SwXMeta::GetCoreObject()
{
    return const_cast< ::sw::Meta * >(GetMeta());
}

uno::Reference<frame::XModel> SwXMeta::GetModel()
{
    ::sw::Meta const * const pMeta( GetMeta() );
    if (pMeta)
    {
        SwTxtNode const * const pTxtNode( pMeta->GetTxtNode() );
        if (pTxtNode)
        {
            SwDocShell const * const pShell(pTxtNode->GetDoc()->GetDocShell());
            return (pShell) ? pShell->GetModel() : 0;
        }
    }
    return 0;
}

// SwModify
void SwXMeta::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    m_pImpl->m_pTextPortions.reset(); // throw away cache (SwTxtNode changed)

    ClientModify(this, pOld, pNew);

    if (!GetRegisteredIn()) // removed => dispose
    {
        m_pImpl->m_ListenerContainer.Disposing();
        m_pImpl->m_bIsDisposed = true;
        m_pImpl->m_Text.Invalidate();
    }
}


/******************************************************************
 * SwXMetaField
 ******************************************************************/

inline const ::sw::MetaField * SwXMetaField::GetMetaField() const
{
    return static_cast< const ::sw::MetaField * >(GetRegisteredIn());
}

SwXMetaField::SwXMetaField(SwDoc *const pDoc,
        uno::Reference<text::XText> const& xParentText,
        TextRangeList_t * const pPortions, SwTxtMeta * const pHint)
    : SwXMetaFieldBaseClass(pDoc, xParentText, pPortions, pHint)
{
    ASSERT(!pHint || RES_TXTATR_METAFIELD == pHint->Which(),
        "SwXMetaField created for wrong hint!");
}

SwXMetaField::SwXMetaField(SwDoc *const pDoc)
    :  SwXMetaFieldBaseClass(pDoc)
{
}

SwXMetaField::~SwXMetaField()
{
}

// XServiceInfo
::rtl::OUString SAL_CALL
SwXMetaField::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXMetaField");
}

sal_Bool SAL_CALL
SwXMetaField::supportsService(const ::rtl::OUString& rServiceName)
throw (uno::RuntimeException)
{
    return rServiceName.equalsAscii("com.sun.star.text.TextContent")
        || rServiceName.equalsAscii("com.sun.star.text.TextField")
        || rServiceName.equalsAscii("com.sun.star.text.textfield.MetadataField");
}

uno::Sequence< ::rtl::OUString > SAL_CALL
SwXMetaField::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(3);
    aRet[0] = C2U("com.sun.star.text.TextContent");
    aRet[1] = C2U("com.sun.star.text.TextField");
    aRet[2] = C2U("com.sun.star.text.textfield.MetadataField");
    return aRet;
}

// XComponent
void SAL_CALL
SwXMetaField::addEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
throw (uno::RuntimeException)
{
    return SwXMeta::addEventListener(xListener);
}

void SAL_CALL
SwXMetaField::removeEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
throw (uno::RuntimeException)
{
    return SwXMeta::removeEventListener(xListener);
}

void SAL_CALL
SwXMetaField::dispose() throw (uno::RuntimeException)
{
    return SwXMeta::dispose();
}

// XTextContent
void SAL_CALL
SwXMetaField::attach(const uno::Reference< text::XTextRange > & i_xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return SwXMeta::AttachImpl(i_xTextRange, RES_TXTATR_METAFIELD);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMetaField::getAnchor() throw (uno::RuntimeException)
{
    return SwXMeta::getAnchor();
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXMetaField::getPropertySetInfo() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    static uno::Reference< beans::XPropertySetInfo > xRef(
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_METAFIELD)
            ->getPropertySetInfo() );
    return xRef;
}

void SAL_CALL
SwXMetaField::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    ::sw::MetaField * const pMeta(
            const_cast< ::sw::MetaField * >(GetMetaField()) );
    if (!pMeta)
        throw lang::DisposedException();

    if (rPropertyName.equalsAscii("NumberFormat"))
    {
        sal_Int32 nNumberFormat(0);
        if (rValue >>= nNumberFormat)
        {
            pMeta->SetNumberFormat(static_cast<sal_uInt32>(nNumberFormat));
        }
    }
    else if (rPropertyName.equalsAscii("IsFixedLanguage"))
    {
        bool b(false);
        if (rValue >>= b)
        {
            pMeta->SetIsFixedLanguage(b);
        }
    }
    else
    {
        throw beans::UnknownPropertyException();
    }
}

uno::Any SAL_CALL
SwXMetaField::getPropertyValue(const ::rtl::OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    ::sw::MetaField const * const pMeta( GetMetaField() );
    if (!pMeta)
        throw lang::DisposedException();

    uno::Any any;

    if (rPropertyName.equalsAscii("NumberFormat"))
    {
        const ::rtl::OUString text( getPresentation(sal_False) );
        any <<= static_cast<sal_Int32>(pMeta->GetNumberFormat(text));
    }
    else if (rPropertyName.equalsAscii("IsFixedLanguage"))
    {
        any <<= pMeta->IsFixedLanguage();
    }
    else
    {
        throw beans::UnknownPropertyException();
    }

    return any;
}

void SAL_CALL
SwXMetaField::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXMetaField::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXMetaField::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXMetaField::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXMetaField::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXMetaField::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXMetaField::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXMetaField::removeVetoableChangeListener(): not implemented");
}

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include <comphelper/processfactory.hxx>

static uno::Reference<rdf::XURI> const&
lcl_getURI(const bool bPrefix)
{
    static uno::Reference< uno::XComponentContext > xContext(
        ::comphelper::getProcessComponentContext());
    static uno::Reference< rdf::XURI > xOdfPrefix(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX),
        uno::UNO_SET_THROW);
    static uno::Reference< rdf::XURI > xOdfSuffix(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_SUFFIX),
        uno::UNO_SET_THROW);
    return (bPrefix) ? xOdfPrefix : xOdfSuffix;
}

static ::rtl::OUString
lcl_getPrefixOrSuffix(
    uno::Reference<rdf::XRepository> const & xRepository,
    uno::Reference<rdf::XResource> const & xMetaField,
    uno::Reference<rdf::XURI> const & xPredicate)
{
    const uno::Reference<container::XEnumeration> xEnum(
        xRepository->getStatements(xMetaField, xPredicate, 0),
        uno::UNO_SET_THROW);
    while (xEnum->hasMoreElements()) {
        rdf::Statement stmt;
        if (!(xEnum->nextElement() >>= stmt)) {
            throw uno::RuntimeException();
        }
        const uno::Reference<rdf::XLiteral> xObject(stmt.Object,
            uno::UNO_QUERY);
        if (!xObject.is()) continue;
        if (xEnum->hasMoreElements()) {
            OSL_TRACE("ignoring other odf:Prefix/odf:Suffix statements");
        }
        return xObject->getValue();
    }
    return ::rtl::OUString();
}

void
getPrefixAndSuffix(
        const uno::Reference<frame::XModel>& xModel,
        const uno::Reference<rdf::XMetadatable>& xMetaField,
        ::rtl::OUString *const o_pPrefix, ::rtl::OUString *const o_pSuffix)
{
    try {
        const uno::Reference<rdf::XRepositorySupplier> xRS(
                xModel, uno::UNO_QUERY_THROW);
        const uno::Reference<rdf::XRepository> xRepo(
                xRS->getRDFRepository(), uno::UNO_SET_THROW);
        const uno::Reference<rdf::XResource> xMeta(
                xMetaField, uno::UNO_QUERY_THROW);
        if (o_pPrefix)
        {
            *o_pPrefix = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(true));
        }
        if (o_pSuffix)
        {
            *o_pSuffix = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(false));
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString::createFromAscii("getPrefixAndSuffix: exception"),
            0, uno::makeAny(e));
    }
}

// XTextField
::rtl::OUString SAL_CALL
SwXMetaField::getPresentation(sal_Bool bShowCommand)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (bShowCommand)
    {
//FIXME ?
        return ::rtl::OUString();
    }
    else
    {
        // getString should check if this is invalid
        const ::rtl::OUString content( this->getString() );
        ::rtl::OUString prefix;
        ::rtl::OUString suffix;
        getPrefixAndSuffix(GetModel(), this, &prefix, &suffix);
        return prefix + content + suffix;
    }
}

