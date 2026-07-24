/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <initializer_list>

#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>

#include <editeng/unotext.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == cppu::UnoType<xint>::get() ) \
        return cpo::uno::Any(uno::Reference< xint >(this))


// SvxUnoTextContentEnumeration


SvxUnoTextContentEnumeration::SvxUnoTextContentEnumeration( const SvxUnoTextBase& rText, const ESelection& rSel ) noexcept
{
    mxParentText = const_cast<SvxUnoTextBase*>(&rText);
    if( rText.GetEditSource() )
        mpEditSource = rText.GetEditSource()->Clone();
    mnNextParagraph = 0;

    if (!mpEditSource)
        return;

    const SvxTextForwarder* pTextForwarder = rText.GetEditSource()->GetTextForwarder();
    const sal_Int32 maxParaIndex = std::min( rSel.end.nPara + 1, pTextForwarder->GetParagraphCount() );

    for (sal_Int32 currentPara = rSel.start.nPara; currentPara < maxParaIndex; currentPara++)
    {
        const SvxUnoTextRangeBaseVec& rRanges( mpEditSource->getRanges() );
        rtl::Reference<SvxUnoTextContent> pContent;
        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pTextForwarder->GetTextLen( currentPara );
        if (currentPara == rSel.start.nPara)
            nStartPos = std::max(nStartPos, rSel.start.nIndex);
        if (currentPara == rSel.end.nPara)
            nEndPos = std::min(nEndPos, rSel.end.nIndex);
        ESelection aCurrentParaSel( currentPara, nStartPos, currentPara, nEndPos );
        for (auto const& elemRange : rRanges)
        {
            if (pContent)
                break;
            SvxUnoTextContent* pIterContent = dynamic_cast< SvxUnoTextContent* >( elemRange );
            if( pIterContent && (pIterContent->mnParagraph == currentPara) )
            {
                ESelection aIterSel = pIterContent->GetSelection();
                if( aIterSel == aCurrentParaSel )
                {
                    pContent = pIterContent;
                    maContents.emplace_back(pContent );
                }
            }
        }
        if( pContent == nullptr )
        {
            pContent = new SvxUnoTextContent( rText, currentPara );
            pContent->SetSelection( aCurrentParaSel );
            maContents.emplace_back(pContent );
        }
    }
}

SvxUnoTextContentEnumeration::~SvxUnoTextContentEnumeration() noexcept
{
}

// container::XEnumeration
bool SvxUnoTextContentEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;
    if( mpEditSource && !maContents.empty() )
        return o3tl::make_unsigned(mnNextParagraph) < maContents.size();
    else
        return false;
}

cpo::uno::Any SvxUnoTextContentEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if(!hasMoreElements())
        throw container::NoSuchElementException();

    rtl::Reference< SvxUnoTextContent > xRef( maContents.at(mnNextParagraph) );
    mnNextParagraph++;
    return cpo::uno::Any( uno::Reference< text::XTextContent >(xRef) );
}




SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextBase& rText, sal_Int32 nPara ) noexcept
:   SvxUnoTextRangeBase(rText)
,   mnParagraph(nPara)
,   mrParentText(rText)
,   mbDisposing( false )
{
    mxParentText = const_cast<SvxUnoTextBase*>(&rText);
}

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextContent& rContent ) noexcept
:   SvxUnoTextRangeBase(rContent)
,   text::XTextContent()
,   container::XEnumerationAccess()
,   lang::XTypeProvider()
,   cppu::OWeakAggObject()
,   mrParentText(rContent.mrParentText)
,   mbDisposing( false )
{
    mxParentText = rContent.mxParentText;
    mnParagraph  = rContent.mnParagraph;
    SetSelection( rContent.GetSelection() );
}

SvxUnoTextContent::~SvxUnoTextContent() noexcept
{
}

// uno::XInterface
cpo::uno::Any SvxUnoTextContent::queryAggregation( const cpo::uno::Type & rType )
{
    QUERYINT( text::XTextRange );
    else QUERYINT( beans::XMultiPropertyStates );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XMultiPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( text::XTextContent );
    else QUERYINT( text::XTextRangeCompare );
    else QUERYINT( lang::XComponent );
    else QUERYINT( container::XEnumerationAccess );
    else QUERYINT( container::XElementAccess );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OWeakAggObject::queryAggregation( rType );
}

cpo::uno::Any SvxUnoTextContent::queryInterface( const cpo::uno::Type & rType )
{
    return OWeakAggObject::queryInterface(rType);
}

void SvxUnoTextContent::acquire() noexcept
{
    OWeakAggObject::acquire();
}

void SvxUnoTextContent::release() noexcept
{
    OWeakAggObject::release();
}

// XTypeProvider

cpo::uno::Sequence< cpo::uno::Type > SvxUnoTextContent::getTypes()
{
    static const cpo::uno::Sequence< cpo::uno::Type > TYPES {
            cppu::UnoType<text::XTextRange>::get(),
            cppu::UnoType<beans::XPropertySet>::get(),
            cppu::UnoType<beans::XMultiPropertySet>::get(),
            cppu::UnoType<beans::XMultiPropertyStates>::get(),
            cppu::UnoType<beans::XPropertyState>::get(),
            cppu::UnoType<text::XTextRangeCompare>::get(),
            cppu::UnoType<text::XTextContent>::get(),
            cppu::UnoType<container::XEnumerationAccess>::get(),
            cppu::UnoType<lang::XServiceInfo>::get(),
            cppu::UnoType<lang::XTypeProvider>::get(),
            cppu::UnoType<lang::XUnoTunnel>::get() };
    return TYPES;
}

cpo::uno::Sequence< sal_Int8 > SvxUnoTextContent::getImplementationId()
{
    return cpo::uno::Sequence<sal_Int8>();
}

// text::XTextRange

uno::Reference< text::XText > SvxUnoTextContent::getText()
{
    return mxParentText;
}

// text::XTextContent
void SvxUnoTextContent::attach( const uno::Reference< text::XTextRange >& )
{
}

uno::Reference< text::XTextRange > SvxUnoTextContent::getAnchor()
{
    return mxParentText;
}

// XComponent

void SvxUnoTextContent::dispose()
{
    SolarMutexGuard aGuard;

    if( mbDisposing )
        return; // caught a recursion

    mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *static_cast<OWeakAggObject*>(this);
    {
        std::unique_lock aMutexGuard(maDisposeContainerMutex);
        maDisposeListeners.disposeAndClear(aMutexGuard, aEvt);
    }

    if( mxParentText.is() )
    {
        mxParentText->removeTextContent( this );
        mxParentText.clear();
    }
}

void SvxUnoTextContent::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    std::unique_lock aGuard(maDisposeContainerMutex);
    maDisposeListeners.addInterface(aGuard, xListener);
}

void SvxUnoTextContent::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
{
   std::unique_lock aGuard(maDisposeContainerMutex);
   maDisposeListeners.removeInterface(aGuard, aListener);
}

// XEnumerationAccess

uno::Reference< container::XEnumeration > SvxUnoTextContent::createEnumeration()
{
    SolarMutexGuard aGuard;

    return new SvxUnoTextRangeEnumeration( mrParentText, mnParagraph, maSelection );
}

// XElementAccess ( container::XEnumerationAccess )

cpo::uno::Type SvxUnoTextContent::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

bool SvxUnoTextContent::hasElements()
{
    SolarMutexGuard aGuard;

    SvxTextForwarder* pForwarder = GetEditSource() ? GetEditSource()->GetTextForwarder() : nullptr;
    if( pForwarder )
    {
        std::vector<sal_Int32> aPortions;
        pForwarder->GetPortions( mnParagraph, aPortions );
        return !aPortions.empty();
    }
    else
    {
        return false;
    }
}

// XPropertySet

void SvxUnoTextContent::setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue )
{
    _setPropertyValue( aPropertyName, aValue, mnParagraph );
}

cpo::uno::Any SvxUnoTextContent::getPropertyValue( const OUString& PropertyName )
{
    return _getPropertyValue( PropertyName, mnParagraph );
}

// XMultiPropertySet
void SvxUnoTextContent::setPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Sequence< cpo::uno::Any >& aValues )
{
    _setPropertyValues( aPropertyNames, aValues, mnParagraph );
}

cpo::uno::Sequence< cpo::uno::Any > SvxUnoTextContent::getPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames )
{
    return _getPropertyValues( aPropertyNames, mnParagraph );
}

/*// XTolerantMultiPropertySet
cpo::uno::Sequence< beans::SetPropertyTolerantFailed > SvxUnoTextContent::setPropertyValuesTolerant( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Sequence< cpo::uno::Any >& aValues ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return _setPropertyValuesTolerant(aPropertyNames, aValues, mnParagraph);
}

cpo::uno::Sequence< beans::GetPropertyTolerantResult > SvxUnoTextContent::getPropertyValuesTolerant( const cpo::uno::Sequence< OUString >& aPropertyNames ) throw (uno::RuntimeException)
{
    return _getPropertyValuesTolerant(aPropertyNames, mnParagraph);
}

cpo::uno::Sequence< beans::GetDirectPropertyTolerantResult > SvxUnoTextContent::getDirectPropertyValuesTolerant( const cpo::uno::Sequence< OUString >& aPropertyNames )
    throw (uno::RuntimeException)
{
    return _getDirectPropertyValuesTolerant(aPropertyNames, mnParagraph);
}*/

// beans::XPropertyState
beans::PropertyState SvxUnoTextContent::getPropertyState( const OUString& PropertyName )
{
    return _getPropertyState( PropertyName, mnParagraph );
}

cpo::uno::Sequence< beans::PropertyState > SvxUnoTextContent::getPropertyStates( const cpo::uno::Sequence< OUString >& aPropertyName )
{
    return _getPropertyStates( aPropertyName, mnParagraph );
}

void SvxUnoTextContent::setPropertyToDefault( const OUString& PropertyName )
{
    _setPropertyToDefault( PropertyName, mnParagraph );
}

// lang::XServiceInfo

OUString SvxUnoTextContent::getImplementationName()
{
    return u"SvxUnoTextContent"_ustr;
}

cpo::uno::Sequence< OUString > SvxUnoTextContent::getSupportedServiceNames()
{
    return comphelper::concatSequences(
        SvxUnoTextRangeBase::getSupportedServiceNames(),
        std::initializer_list<OUString>{ u"com.sun.star.style.ParagraphProperties"_ustr,
                                         u"com.sun.star.style.ParagraphPropertiesComplex"_ustr,
                                         u"com.sun.star.style.ParagraphPropertiesAsian"_ustr,
                                         u"com.sun.star.text.TextContent"_ustr,
                                         u"com.sun.star.text.Paragraph"_ustr });
}




SvxUnoTextRangeEnumeration::SvxUnoTextRangeEnumeration(const SvxUnoTextBase& rParentText, sal_Int32 nParagraph, const ESelection& rSel)
:   mxParentText(  const_cast<SvxUnoTextBase*>(&rParentText) ),
    mnNextPortion( 0 )
{
    if (rParentText.GetEditSource())
        mpEditSource = rParentText.GetEditSource()->Clone();

    if( !(mpEditSource && mpEditSource->GetTextForwarder() && (nParagraph == rSel.start.nPara && nParagraph == rSel.end.nPara)) )
        return;

    std::vector<sal_Int32> aPortions;
    mpEditSource->GetTextForwarder()->GetPortions( nParagraph, aPortions );
    for( size_t aPortionIndex = 0; aPortionIndex < aPortions.size(); aPortionIndex++ )
    {
        sal_Int32 nStartPos = 0;
        if ( aPortionIndex > 0 )
            nStartPos = aPortions.at( aPortionIndex - 1 );
        if (nStartPos > rSel.end.nIndex)
            continue;
        sal_Int32 nEndPos = aPortions.at(aPortionIndex);
        if (nEndPos < rSel.start.nIndex)
            continue;

        nStartPos = std::max(nStartPos, rSel.start.nIndex);
        nEndPos = std::min(nEndPos, rSel.end.nIndex);
        ESelection aSel( nParagraph, nStartPos, nParagraph, nEndPos );

        const SvxUnoTextRangeBaseVec& rRanges( mpEditSource->getRanges() );
        rtl::Reference<SvxUnoTextRange> pRange;
        for (auto const& elemRange : rRanges)
        {
            if (pRange)
                break;
            SvxUnoTextRange* pIterRange = dynamic_cast< SvxUnoTextRange* >( elemRange );
            if( pIterRange && pIterRange->mbPortion && (aSel == pIterRange->maSelection) )
                pRange = pIterRange;
        }
        if( pRange == nullptr )
        {
            pRange = new SvxUnoTextRange( rParentText, true );
            pRange->SetSelection( aSel );
        }
        maPortions.emplace_back(pRange );
    }
}

SvxUnoTextRangeEnumeration::~SvxUnoTextRangeEnumeration() noexcept
{
}

// container::XEnumeration

bool SvxUnoTextRangeEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;

    return !maPortions.empty() && mnNextPortion < maPortions.size();
}

cpo::uno::Any SvxUnoTextRangeEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if( maPortions.empty() || mnNextPortion >= maPortions.size() )
        throw container::NoSuchElementException();

    rtl::Reference< SvxUnoTextRange > xRange = maPortions.at(mnNextPortion);
    mnNextPortion++;
    return cpo::uno::Any( uno::Reference< text::XTextRange >(xRange) );
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextBase& rText ) noexcept
:   SvxUnoTextRangeBase(rText),
    mxParentText( const_cast<SvxUnoTextBase*>(&rText) )
{
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextCursor& rCursor ) noexcept
:   SvxUnoTextRangeBase(rCursor)
,   text::XTextCursor()
,   lang::XTypeProvider()
,   cppu::OWeakAggObject()
,   mxParentText(rCursor.mxParentText)
{
}

SvxUnoTextCursor::~SvxUnoTextCursor() noexcept
{
}

// Comment out automatically - [getIdlClass(es) or queryInterface]
// Please use the XTypeProvider!
//bool SvxUnoTextCursor::queryInterface( uno::Uik aUIK, Reference< uno::XInterface > & xRef)
cpo::uno::Any SvxUnoTextCursor::queryAggregation( const cpo::uno::Type & rType )
{
    if( rType == cppu::UnoType<text::XTextRange>::get())
        return cpo::uno::Any(uno::Reference< text::XTextRange >(static_cast<SvxUnoTextRangeBase *>(this)));
    else QUERYINT( text::XTextCursor );
    else QUERYINT( beans::XMultiPropertyStates );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XMultiPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( text::XTextRangeCompare );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OWeakAggObject::queryAggregation( rType );
}

cpo::uno::Any SvxUnoTextCursor::queryInterface( const cpo::uno::Type & rType )
{
    return OWeakAggObject::queryInterface(rType);
}

void SvxUnoTextCursor::acquire() noexcept
{
    OWeakAggObject::acquire();
}

void SvxUnoTextCursor::release() noexcept
{
    OWeakAggObject::release();
}

// XTypeProvider
cpo::uno::Sequence< cpo::uno::Type > SvxUnoTextCursor::getTypes()
{
    static const cpo::uno::Sequence< cpo::uno::Type > TYPES {
            cppu::UnoType<text::XTextRange>::get(),
             cppu::UnoType<text::XTextCursor>::get(),
             cppu::UnoType<beans::XPropertySet>::get(),
             cppu::UnoType<beans::XMultiPropertySet>::get(),
             cppu::UnoType<beans::XMultiPropertyStates>::get(),
             cppu::UnoType<beans::XPropertyState>::get(),
             cppu::UnoType<text::XTextRangeCompare>::get(),
             cppu::UnoType<lang::XServiceInfo>::get(),
             cppu::UnoType<lang::XTypeProvider>::get(),
             cppu::UnoType<lang::XUnoTunnel>::get() };
    return TYPES;
}

cpo::uno::Sequence< sal_Int8 > SvxUnoTextCursor::getImplementationId()
{
    return cpo::uno::Sequence<sal_Int8>();
}

// text::XTextCursor
void SvxUnoTextCursor::collapseToStart()
{
    SolarMutexGuard aGuard;
    CollapseToStart();
}

void SvxUnoTextCursor::collapseToEnd()
{
    SolarMutexGuard aGuard;
    CollapseToEnd();
}

bool SvxUnoTextCursor::isCollapsed()
{
    SolarMutexGuard aGuard;
    return IsCollapsed();
}

bool SvxUnoTextCursor::goLeft( sal_Int16 nCount, bool bExpand )
{
    SolarMutexGuard aGuard;
    return GoLeft( nCount, bExpand );
}

bool SvxUnoTextCursor::goRight( sal_Int16 nCount, bool bExpand )
{
    SolarMutexGuard aGuard;
    return GoRight( nCount, bExpand );
}

void SvxUnoTextCursor::gotoStart( bool bExpand )
{
    SolarMutexGuard aGuard;
    GotoStart( bExpand );
}

void SvxUnoTextCursor::gotoEnd( bool bExpand )
{
    SolarMutexGuard aGuard;
    GotoEnd( bExpand );
}

void SvxUnoTextCursor::gotoRange( const uno::Reference< text::XTextRange >& xRange, bool bExpand )
{
    if( !xRange.is() )
        return;

    SvxUnoTextRangeBase* pRange = comphelper::getFromUnoTunnel<SvxUnoTextRangeBase>( xRange );

    if( !pRange )
        return;

    ESelection aNewSel = pRange->GetSelection();

    if( bExpand )
    {
        const ESelection& rOldSel = GetSelection();
        aNewSel.start = rOldSel.start;
    }

    SetSelection( aNewSel );
}

// text::XTextRange (rest in SvxTextRange)
uno::Reference< text::XText > SvxUnoTextCursor::getText()
{
    return mxParentText;
}

uno::Reference< text::XTextRange > SvxUnoTextCursor::getStart()
{
    return SvxUnoTextRangeBase::getStart();
}

uno::Reference< text::XTextRange > SvxUnoTextCursor::getEnd()
{
    return SvxUnoTextRangeBase::getEnd();
}

OUString SvxUnoTextCursor::getString()
{
    return SvxUnoTextRangeBase::getString();
}

void SvxUnoTextCursor::setString( const OUString& aString )
{
    SvxUnoTextRangeBase::setString(aString);
}
// lang::XServiceInfo
OUString SvxUnoTextCursor::getImplementationName()
{
    return u"SvxUnoTextCursor"_ustr;
}

bool SvxUnoTextCursor::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

cpo::uno::Sequence< OUString > SvxUnoTextCursor::getSupportedServiceNames()
{
    return comphelper::concatSequences(
        SvxUnoTextRangeBase::getSupportedServiceNames(),
        std::initializer_list<OUString>{ u"com.sun.star.style.ParagraphProperties"_ustr,
                                         u"com.sun.star.style.ParagraphPropertiesComplex"_ustr,
                                         u"com.sun.star.style.ParagraphPropertiesAsian"_ustr,
                                         u"com.sun.star.text.TextCursor"_ustr });
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
