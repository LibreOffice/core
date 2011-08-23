/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <rtl/uuid.h>

#include <errhdl.hxx>

#include <unoobj.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ndtxt.hxx>
#include <unocrsr.hxx>
#include <hints.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

using rtl::OUString;

/******************************************************************
 *
 ******************************************************************/
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXFootnote::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXFootnote::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return SwXText::getSomething( rId );
}
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFootnote::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFootnote");
}
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXFootnote::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return  !rServiceName.compareToAscii("com.sun.star.text.Footnote") ||
            !rServiceName.compareToAscii("com.sun.star.text.TextContent") ||
            !rServiceName.compareToAscii("com.sun.star.text.Text") ||
            (m_bIsEndnote && !rServiceName.compareToAscii("com.sun.star.text.Endnote"));
;
}
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFootnote::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(m_bIsEndnote ? 4 : 3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Footnote");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    pArray[2] = C2U("com.sun.star.text.Text");
    if(m_bIsEndnote)
        pArray[3] = C2U("com.sun.star.text.Endnote");
    return aRet;
}
/*-- 10.12.98 15:31:44---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXFootnote, SwClient);

SwXFootnote::SwXFootnote(sal_Bool bEndnote) :
    aLstnrCntnr( (text::XTextContent*)this),
    SwXText(0, CURSOR_FOOTNOTE),
    pFmtFtn(0),
    m_bIsDescriptor(sal_True),
    m_bIsEndnote(bEndnote)
{

}
/*-- 10.12.98 15:31:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFootnote::SwXFootnote(SwDoc* pDoc, const SwFmtFtn& rFmt) :
    SwXText(pDoc, CURSOR_FOOTNOTE),
    aLstnrCntnr( (text::XTextContent*)this),
    pFmtFtn(&rFmt),
    m_bIsDescriptor(sal_False),
    m_bIsEndnote(rFmt.IsEndNote())
{
    GetDoc()->GetUnoCallBack()->Add(this);
}
/*-- 10.12.98 15:31:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFootnote::~SwXFootnote()
{

}
/* -----------------------------21.03.00 15:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL SwXFootnote::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aFtnTypes = SwXFootnoteBaseClass::getTypes();
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

    long nIndex = aFtnTypes.getLength();
    aFtnTypes.realloc(	aFtnTypes.getLength() + aTextTypes.getLength());

    uno::Type* pFtnTypes = aFtnTypes.getArray();
    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(long nPos = 0; nPos < aTextTypes.getLength(); nPos++)
        pFtnTypes[nIndex++] = pTextTypes[nPos];

    return aFtnTypes;
}

/* -----------------------------21.03.00 15:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXFootnote::getImplementationId(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/* -----------------------------21.03.00 15:46--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SAL_CALL SwXFootnote::queryInterface( const uno::Type& aType ) throw(uno::RuntimeException)
{
    uno::Any aRet = SwXFootnoteBaseClass::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType() )
        aRet = SwXText::queryInterface(aType);
    return aRet;
}

/*-- 10.12.98 15:31:47---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFootnote::getLabel(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    String sRet;
    const SwFmtFtn*  pFmt = SwXFootnote::FindFmt();
    if(pFmt)
        sRet = pFmt->GetNumStr();
    else if(m_bIsDescriptor)
        return m_sLabel;
    else
        throw uno::RuntimeException();
    return sRet;
}
/*-- 10.12.98 15:31:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::setLabel(const OUString& aLabel) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const SwFmtFtn*  pFmt = FindFmt();
    if(pFmt)
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
        DBG_ASSERT(pTxtFtn, "kein TextNode?");
        SwTxtNode& rTxtNode = (SwTxtNode&)pTxtFtn->GetTxtNode();

        SwPaM aPam(rTxtNode, *pTxtFtn->GetStart());
        GetDoc()->SetCurFtn(aPam, aLabel, pFmt->GetNumber(), pFmt->IsEndNote());
    }
    else if(m_bIsDescriptor)
        m_sLabel = String(aLabel);
    else
        throw uno::RuntimeException();

}
/* -----------------18.02.99 13:32-------------------
 *
 * --------------------------------------------------*/
void SwXFootnote::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
            throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }
    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc)
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);

        UnoActionContext aCont(pDoc);
        SwTxtAttr* pTxtAttr = 0;
        pDoc->DeleteAndJoin(aPam);
        aPam.DeleteMark();
        SwFmtFtn aFootNote(m_bIsEndnote);
        if(m_sLabel.Len())
            aFootNote.SetNumStr(m_sLabel);
        SfxItemSet  aSet(pDoc->GetAttrPool(), RES_TXTATR_FTN, RES_TXTATR_FTN, 0L);
        aSet.Put(aFootNote);
        SwXTextCursor::SetCrsrAttr(aPam, aSet, 0);

        pTxtAttr = aPam.GetNode()->GetTxtNode()->GetTxtAttr(
                    aPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_FTN );

        if(pTxtAttr)
        {
            const SwFmtFtn& rFtn = pTxtAttr->GetFtn();
            pFmtFtn = &rFtn;
            pDoc->GetUnoCallBack()->Add(this);
            //force creation of sequence id - is used for references
            if(pDoc->IsInReading())
                ((SwTxtFtn*)pTxtAttr)->SetSeqNo(pDoc->GetFtnIdxs().Count());
            else
                ((SwTxtFtn*)pTxtAttr)->SetSeqRefNo();
        }
        m_bIsDescriptor = sal_False;
        SetDoc(pDoc);
    }
    else
        throw lang::IllegalArgumentException();
}
/*-- 10.12.98 15:31:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::attach(const uno::Reference< text::XTextRange > & xTextRange)
            throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}
/*-- 10.12.98 15:31:48---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXFootnote::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    const SwFmtFtn*  pFmt = FindFmt();
    if(pFmt)
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
        SwPosition aPos( pTxtFtn->GetTxtNode() );
        aRef = SwXTextRange::CreateTextRangeFromPosition((SwDoc*)GetDoc(), aPos, 0);
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 10.12.98 15:31:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const SwFmtFtn*  pFmt = FindFmt();
    if(pFmt)
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
        DBG_ASSERT(pTxtFtn, "kein TextNode?");
        SwTxtNode& rTxtNode = (SwTxtNode&)pTxtFtn->GetTxtNode();
        xub_StrLen nPos = *pTxtFtn->GetStart();
        SwPaM aPam(rTxtNode, nPos, rTxtNode, nPos+1 );
        GetDoc()->DeleteAndJoin( aPam );
    }
    else
        throw uno::RuntimeException();

}
/*-- 10.12.98 15:31:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 10.12.98 15:31:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/* -----------------06.05.99 15:31-------------------
 *
 * --------------------------------------------------*/
const SwStartNode *SwXFootnote::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;
    const SwFmtFtn*  pFmt = FindFmt();
    if(pFmt)
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
        if( pTxtFtn )
            pSttNd = pTxtFtn->GetStartNode()->GetNode().GetStartNode();
    }
    return pSttNd;
}

uno::Reference< text::XTextCursor >   SwXFootnote::createCursor() throw ( ::com::sun::star::uno::RuntimeException)
{
    return createTextCursor();
}
/*-- 10.12.98 15:31:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXFootnote::createTextCursor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    const SwFmtFtn*  pFmt = FindFmt();
    if(pFmt)
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
        SwPosition aPos( *pTxtFtn->GetStartNode() );
        SwXTextCursor* pXCrsr = new SwXTextCursor(this, aPos, CURSOR_FOOTNOTE, GetDoc());
        aRef =  (text::XWordCursor*)pXCrsr;
        SwUnoCrsr*	pUnoCrsr = pXCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 10.12.98 15:31:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXFootnote::createTextCursorByRange(
    const uno::Reference< text::XTextRange > & aTextPosition)
            throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const SwFmtFtn*  pFmt = FindFmt();
    uno::Reference< text::XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if(pFmt && SwXTextRange::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();

        // skip section nodes to find 'true' start node
        const SwNode* pStart = aPam.GetNode()->FindFootnoteStartNode();
        while( pStart->IsSectionNode() )
            pStart = pStart->FindStartNode();

        if( pStart == &pTxtFtn->GetStartNode()->GetNode())
            aRef =  (text::XWordCursor*)new SwXTextCursor(this , *aPam.GetPoint(), CURSOR_FOOTNOTE, GetDoc(), aPam.GetMark());
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 13.06.00 14:28:23---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XEnumeration >  SwXFootnote::createEnumeration() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
     uno::XInterface* pRet = 0;
    const SwFmtFtn*  pFmt = FindFmt();
    if(pFmt)
    {
        const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
        SwPosition aPos( *pTxtFtn->GetStartNode() );
        SwXTextCursor* pXCrsr = new SwXTextCursor(this, aPos, CURSOR_FOOTNOTE, GetDoc());
        SwUnoCrsr*	pUnoCrsr = pXCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_FOOTNOTE);
    }
    return aRef;
}
/*-- 13.06.00 14:28:24---------------------------------------------------

  -----------------------------------------------------------------------*/
Type SwXFootnote::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((Reference<XTextRange>*)0);
}
/*-- 13.06.00 14:28:24---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXFootnote::hasElements(  ) throw(RuntimeException)
{
    return sal_True;
}
/* -----------------------------07.01.00 12:39--------------------------------

 ---------------------------------------------------------------------------*/
void SwXFootnote::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        pFmtFtn = 0;
        aLstnrCntnr.Disposing();
        SetDoc(0);
    }
}
/* -----------------18.01.99 09:12-------------------
 *
 * --------------------------------------------------*/
void SwXFootnote::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
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
    case RES_FOOTNOTE_DELETED:
        if( (void*)pFmtFtn == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;
    }
}
/*-- 11.09.00 13:12:03---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo > SwXFootnote::getPropertySetInfo(  )
    throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        new SfxItemPropertySetInfo(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_FOOTNOTE));
    return xRef;
}
/*-- 11.09.00 13:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::setPropertyValue( const ::rtl::OUString&,
    const Any& )
        throw(UnknownPropertyException, PropertyVetoException,
            IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    //no values to be set
    throw IllegalArgumentException();
}
/*-- 11.09.00 13:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXFootnote::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_START_REDLINE))||
                rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_END_REDLINE)))
        {
            //redline can only be returned if it's a living object
            if(!m_bIsDescriptor)
                aRet = SwXText::getPropertyValue(rPropertyName);
        }
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_REFERENCE_ID)))
        {
            const SwFmtFtn*  pFmt = FindFmt();
            if(pFmt)
            {
                const SwTxtFtn* pTxtFtn = pFmt->GetTxtFtn();
                DBG_ASSERT(pTxtFtn, "no TextNode?");
                aRet <<= (sal_Int16)pTxtFtn->GetSeqRefNo();
            }
        }
        else
        {
            UnknownPropertyException aExcept;
            aExcept.Message = rPropertyName;
            throw aExcept;
        }
    }
    return aRet;
}
/*-- 11.09.00 13:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::addPropertyChangeListener( const OUString& aPropertyName,
    const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.09.00 13:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::removePropertyChangeListener( const OUString& aPropertyName,
    const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.09.00 13:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::addVetoableChangeListener( const OUString& PropertyName,
    const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.09.00 13:12:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnote::removeVetoableChangeListener( const OUString& PropertyName,
    const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
