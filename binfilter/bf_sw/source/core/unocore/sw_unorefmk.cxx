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

#include <errhdl.hxx>

#include <unoobj.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
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
 * SwXReferenceMark
 ******************************************************************/
const uno::Sequence< sal_Int8 > & SwXReferenceMark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXReferenceMark::getSomething( const uno::Sequence< sal_Int8 >& rId )
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

OUString SwXReferenceMark::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXReferenceMark");
}

BOOL SwXReferenceMark::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.ReferenceMark")||
                !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}

Sequence< OUString > SwXReferenceMark::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ReferenceMark");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

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

SwXReferenceMark::~SwXReferenceMark()
{
}

void SwXReferenceMark::InsertRefMark(SwPaM& rPam, SwDoc* pDoc)
{
    UnoActionContext aCont(pDoc);
    SwTxtAttr* pTxtAttr = 0;
    SwFmtRefMark aRefMark(sMarkName);
    SfxItemSet  aSet(pDoc->GetAttrPool(), RES_TXTATR_REFMARK, RES_TXTATR_REFMARK, 0L);
    aSet.Put(aRefMark);
    sal_Bool bMark = *rPam.GetPoint() != *rPam.GetMark();
    SwXTextCursor::SetCrsrAttr(rPam, aSet, 0);

    if( bMark && *rPam.GetPoint() > *rPam.GetMark())
        rPam.Exchange();

    if( bMark )
        pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttr(
                rPam.GetPoint()->nContent, RES_TXTATR_REFMARK );
    else
        pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttr(
                rPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_REFMARK );

    if(pTxtAttr)
        pMark = &pTxtAttr->GetRefMark();

    pDoc->GetUnoCallBack()->Add(this);
}

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
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }
    SwDoc* pDocument = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDocument)
    {
        SwUnoInternalPaM aPam(*pDocument);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        InsertRefMark(aPam, pDocument);
        m_bIsDescriptor = sal_False;
        pDoc = pDocument;
        pDoc->GetUnoCallBack()->Add(this);
    }
    else
        throw lang::IllegalArgumentException();
}

void SwXReferenceMark::attach(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange >  SwXReferenceMark::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    if(IsValid())
    {
        const SwFmtRefMark*	pNewMark = pDoc->GetRefMark(sMarkName);
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
                                new SwPaM(	rTxtNode, *pTxtMark->GetStart());


                xRet = SwXTextRange::CreateTextRangeFromPosition(pDoc,
                        *pPam->Start(), pPam->End());
                delete pPam;
            }
        }
    }
    return xRet;
}

void SwXReferenceMark::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        const SwFmtRefMark*	pNewMark = pDoc->GetRefMark(sMarkName);
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

void SwXReferenceMark::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXReferenceMark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}

OUString SwXReferenceMark::getName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid() || !pDoc->GetRefMark(sMarkName))
    {
        throw uno::RuntimeException();
    }
    return sMarkName;
}

void SwXReferenceMark::setName(const OUString& Name_) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_bIsDescriptor)
        sMarkName = String(Name_);
    else
    {
        String sNewName(Name_);
        if(!IsValid() || !pDoc->GetRefMark(sMarkName) || pDoc->GetRefMark(sNewName))
        {
            throw uno::RuntimeException();
        }
        const SwFmtRefMark*	pCurMark = pDoc->GetRefMark(sMarkName);
        if(sNewName != sMarkName && pCurMark && pCurMark == pMark)
        {
            SwDoc* pTempDoc = pDoc;
            UnoActionContext aCont(pTempDoc);
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pTempDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                xub_StrLen nStt = *pTxtMark->GetStart(),
                           nEnd = pTxtMark->GetEnd() ? *pTxtMark->GetEnd()
                                                     : nStt + 1;

                SwPaM aPam( rTxtNode, nStt, rTxtNode, nEnd );
                pDoc->DeleteAndJoin( aPam );

                sMarkName = sNewName;
                //create a new one
                InsertRefMark( aPam, pTempDoc );
                pDoc = pTempDoc;
            }
        }
    }
}

void 	SwXReferenceMark::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        aLstnrCntnr.Disposing();
        pDoc = 0;
        pMark = 0;
    }
}

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

Reference< XPropertySetInfo > SwXReferenceMark::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        SfxItemPropertySet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS)).
            getPropertySetInfo();
    return xRef;
}

void SwXReferenceMark::setPropertyValue(
    const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw IllegalArgumentException();
}

Any SwXReferenceMark::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
        throw UnknownPropertyException();
    return aRet;
}

void SwXReferenceMark::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
            throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXReferenceMark::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
            throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXReferenceMark::addVetoableChangeListener( const OUString& PropertyName,
    const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXReferenceMark::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
