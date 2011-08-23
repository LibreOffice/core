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
#include <unoprnms.hxx>
#include <bookmrk.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

/******************************************************************
 * SwXBookmark
 ******************************************************************/
TYPEINIT1(SwXBookmark, SwClient)

const uno::Sequence< sal_Int8 > & SwXBookmark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXBookmark::getSomething( const uno::Sequence< sal_Int8 >& rId )
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

SwXBookmark::SwXBookmark(SwBookmark* pBkm, SwDoc* pDc) :
        pDoc(pDc),
        bIsDescriptor(0 == pBkm),
        aLstnrCntnr( (text::XTextContent*)this)
{
    if(pBkm)
        pBkm->Add(this);
}

SwXBookmark::~SwXBookmark()
{
}

void SwXBookmark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
                                        throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!bIsDescriptor)
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

    SwDoc* pDc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ?
        (SwDoc*)pCursor->GetDoc() : 0;
    if(pDc)
    {
        pDoc = pDc;
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        UnoActionContext aCont(pDoc);
        SwBookmark* pBkm = 0;
        {
            if(!m_aName.Len())
                 m_aName =  C2S("Bookmark");
            if( USHRT_MAX != pDoc->FindBookmark(m_aName) )
                pDoc->MakeUniqueBookmarkName( m_aName );
            KeyCode aCode;
            pBkm = pDoc->MakeBookmark( aPam, aCode,
                                                m_aName, aEmptyStr, BOOKMARK);
            pBkm->Add(this);
            bIsDescriptor = sal_False;
        }
    }
    else
        throw lang::IllegalArgumentException();
}

void SwXBookmark::attach(const uno::Reference< text::XTextRange > & xTextRange)
                            throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange >  SwXBookmark::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRet;
    SwBookmark* pBkm = GetBookmark();

    if(pBkm)
    {
        const SwPosition& rPos = pBkm->GetPos();
        const SwPosition* pMarkPos = pBkm->GetOtherPos();

        aRet = SwXTextRange::CreateTextRangeFromPosition(pDoc, rPos, pMarkPos);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXBookmark::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
        GetDoc()->DelBookmark(getName());
    else
        throw uno::RuntimeException();
}

void SwXBookmark::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXBookmark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
    throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}

OUString SwXBookmark::getName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwBookmark* pBkm = GetBookmark();
    OUString sRet;
    if(pBkm)
        sRet = pBkm->GetName();
    else if(bIsDescriptor)
        sRet = m_aName;
    else
        throw uno::RuntimeException();
    return sRet;
}

void SwXBookmark::setName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwBookmark* pBkm = GetBookmark();
    String sBkName(rName);
    String sOldName = getName();
    if(sOldName != sBkName && pBkm && USHRT_MAX == pDoc->FindBookmark(sBkName))
    {
        KeyCode aCode;
        String sShortName;
        SwPaM aPam(pBkm->GetPos());
        if(pBkm->GetOtherPos())
        {
            aPam.SetMark();
            *aPam.GetMark() = *pBkm->GetOtherPos();
        }

        SwBookmark* pMark = pDoc->MakeBookmark(aPam, aCode,
                    sBkName, sShortName, BOOKMARK);
        pMark->Add(this);
        GetDoc()->DelBookmark( sOldName );

    }
    else if(bIsDescriptor)
        m_aName = sBkName;
    else
        throw uno::RuntimeException();
}

OUString SwXBookmark::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXBookmark");
}

sal_Bool SwXBookmark::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.Bookmark") ||
                !rServiceName.compareToAscii("com.sun.star.document.LinkTarget") ||
                    !rServiceName.compareToAscii("com.sun.star.text.TextContent");
;
}

uno::Sequence< OUString > SwXBookmark::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.text.Bookmark");
    pArr[1] = C2U("com.sun.star.document.LinkTarget");
    pArr[2] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

void 	SwXBookmark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetBookmark())
    {
        pDoc = 0;
        aLstnrCntnr.Disposing();
    }
}

uno::Reference< beans::XPropertySetInfo >  SwXBookmark::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef;
    if(!aRef.is())
    {
        const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_BOOKMARK);
        uno::Reference< beans::XPropertySetInfo >  xInfo = new SfxItemPropertySetInfo(pMap);
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        aRef = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    return aRef;
}

void SwXBookmark::setPropertyValue(const OUString& PropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    throw IllegalArgumentException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + PropertyName, static_cast < cppu::OWeakObject * > ( this ), 0 );
    //hier gibt es nichts zu setzen
}

uno::Any SwXBookmark::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_NAME)))
            aRet <<= getName();
    }
    return aRet;
}

void SwXBookmark::addPropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}

void SwXBookmark::removePropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XPropertyChangeListener > & aListener)
            throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}

void SwXBookmark::addVetoableChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener > & aListener)
            throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}

void SwXBookmark::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
