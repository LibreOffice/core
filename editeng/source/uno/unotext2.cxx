/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <rtl/instance.hxx>

#include <editeng/unotext.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == cppu::UnoType<xint>::get() ) \
        return uno::makeAny(uno::Reference< xint >(this))


// SvxUnoTextContentEnumeration


SvxUnoTextContentEnumeration::SvxUnoTextContentEnumeration( const SvxUnoTextBase& rText, const ESelection& rSel ) throw()
{
    mxParentText = const_cast<SvxUnoTextBase*>(&rText);
    if( rText.GetEditSource() )
        mpEditSource = rText.GetEditSource()->Clone();
    mnNextParagraph = 0;

    const SvxTextForwarder* pTextForwarder = rText.GetEditSource()->GetTextForwarder();
    const sal_Int32 maxParaIndex = std::min( rSel.nEndPara + 1, pTextForwarder->GetParagraphCount() );

    for( sal_Int32 currentPara = rSel.nStartPara; currentPara < maxParaIndex; currentPara++ )
    {
        const SvxUnoTextRangeBaseVec& rRanges( mpEditSource->getRanges() );
        SvxUnoTextContent* pContent = nullptr;
        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pTextForwarder->GetTextLen( currentPara );
        if( currentPara == rSel.nStartPara )
            nStartPos = std::max(nStartPos, rSel.nStartPos);
        if( currentPara == rSel.nEndPara )
            nEndPos = std::min(nEndPos, rSel.nEndPos);
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

SvxUnoTextContentEnumeration::~SvxUnoTextContentEnumeration() throw()
{
}

// container::XEnumeration
sal_Bool SAL_CALL SvxUnoTextContentEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;
    if( mpEditSource && !maContents.empty() )
        return o3tl::make_unsigned(mnNextParagraph) < maContents.size();
    else
        return false;
}

uno::Any SvxUnoTextContentEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if(!hasMoreElements())
        throw container::NoSuchElementException();

    uno::Reference< text::XTextContent > xRef( maContents.at(mnNextParagraph).get() );
    mnNextParagraph++;
    return uno::makeAny( xRef );
}




SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextBase& rText, sal_Int32 nPara ) throw()
:   SvxUnoTextRangeBase(rText)
,   mnParagraph(nPara)
,   mrParentText(rText)
,   maDisposeListeners(maDisposeContainerMutex)
,   mbDisposing( false )
{
    mxParentText = const_cast<SvxUnoTextBase*>(&rText);
}

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextContent& rContent ) throw()
:   SvxUnoTextRangeBase(rContent)
,   text::XTextContent()
,   container::XEnumerationAccess()
,   lang::XTypeProvider()
,   cppu::OWeakAggObject()
,   mrParentText(rContent.mrParentText)
,   maDisposeListeners(maDisposeContainerMutex)
,   mbDisposing( false )
{
    mxParentText = rContent.mxParentText;
    mnParagraph  = rContent.mnParagraph;
    SetSelection( rContent.GetSelection() );
}

SvxUnoTextContent::~SvxUnoTextContent() throw()
{
}

// uno::XInterface
uno::Any SAL_CALL SvxUnoTextContent::queryAggregation( const uno::Type & rType )
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

uno::Any SAL_CALL SvxUnoTextContent::queryInterface( const uno::Type & rType )
{
    return OWeakAggObject::queryInterface(rType);
}

void SAL_CALL SvxUnoTextContent::acquire() throw( )
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoTextContent::release() throw( )
{
    OWeakAggObject::release();
}

// XTypeProvider

namespace
{
    struct theSvxUnoTextContentTypes :
        public rtl::StaticWithInit<uno::Sequence<uno::Type>, theSvxUnoTextContentTypes>
    {
        uno::Sequence<uno::Type> operator () ()
        {
            uno::Sequence< uno::Type > aTypeSequence;

            aTypeSequence.realloc( 11 ); // !DANGER! keep this updated
            uno::Type* pTypes = aTypeSequence.getArray();

            *pTypes++ = cppu::UnoType<text::XTextRange>::get();
            *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
            *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
            *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
            *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
            *pTypes++ = cppu::UnoType<text::XTextRangeCompare>::get();
            *pTypes++ = cppu::UnoType<text::XTextContent>::get();
            *pTypes++ = cppu::UnoType<container::XEnumerationAccess>::get();
            *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
            *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
            *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();

            return aTypeSequence;
        }
    };
}

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextContent::getTypes()
{
    return theSvxUnoTextContentTypes::get();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextContent::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// text::XTextRange

uno::Reference< text::XText > SAL_CALL SvxUnoTextContent::getText()
{
    return mxParentText;
}

// text::XTextContent
void SAL_CALL SvxUnoTextContent::attach( const uno::Reference< text::XTextRange >& )
{
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextContent::getAnchor()
{
    return uno::Reference< text::XTextRange >::query( mxParentText );
}

// XComponent

void SAL_CALL SvxUnoTextContent::dispose()
{
    SolarMutexGuard aGuard;

    if( mbDisposing )
        return; // caught a recursion

    mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *static_cast<OWeakAggObject*>(this);
    maDisposeListeners.disposeAndClear(aEvt);

    if( mxParentText.is() )
        mxParentText->removeTextContent( this );
}

void SAL_CALL SvxUnoTextContent::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    maDisposeListeners.addInterface(xListener);
}

void SAL_CALL SvxUnoTextContent::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
{
   maDisposeListeners.removeInterface(aListener);
}

// XEnumerationAccess

uno::Reference< container::XEnumeration > SAL_CALL SvxUnoTextContent::createEnumeration()
{
    SolarMutexGuard aGuard;

    return new SvxUnoTextRangeEnumeration( mrParentText, mnParagraph, maSelection );
}

// XElementAccess ( container::XEnumerationAccess )

uno::Type SAL_CALL SvxUnoTextContent::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SvxUnoTextContent::hasElements()
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

void SAL_CALL SvxUnoTextContent::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    _setPropertyValue( aPropertyName, aValue, mnParagraph );
}

uno::Any SAL_CALL SvxUnoTextContent::getPropertyValue( const OUString& PropertyName )
{
    return _getPropertyValue( PropertyName, mnParagraph );
}

// XMultiPropertySet
void SAL_CALL SvxUnoTextContent::setPropertyValues( const uno::Sequence< OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues )
{
    _setPropertyValues( aPropertyNames, aValues, mnParagraph );
}

uno::Sequence< uno::Any > SAL_CALL SvxUnoTextContent::getPropertyValues( const uno::Sequence< OUString >& aPropertyNames )
{
    return _getPropertyValues( aPropertyNames, mnParagraph );
}

/*// XTolerantMultiPropertySet
uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL SvxUnoTextContent::setPropertyValuesTolerant( const uno::Sequence< OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return _setPropertyValuesTolerant(aPropertyNames, aValues, mnParagraph);
}

uno::Sequence< beans::GetPropertyTolerantResult > SAL_CALL SvxUnoTextContent::getPropertyValuesTolerant( const uno::Sequence< OUString >& aPropertyNames ) throw (uno::RuntimeException)
{
    return _getPropertyValuesTolerant(aPropertyNames, mnParagraph);
}

uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL SvxUnoTextContent::getDirectPropertyValuesTolerant( const uno::Sequence< OUString >& aPropertyNames )
    throw (uno::RuntimeException)
{
    return _getDirectPropertyValuesTolerant(aPropertyNames, mnParagraph);
}*/

// beans::XPropertyState
beans::PropertyState SAL_CALL SvxUnoTextContent::getPropertyState( const OUString& PropertyName )
{
    return _getPropertyState( PropertyName, mnParagraph );
}

uno::Sequence< beans::PropertyState > SAL_CALL SvxUnoTextContent::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
{
    return _getPropertyStates( aPropertyName, mnParagraph );
}

void SAL_CALL SvxUnoTextContent::setPropertyToDefault( const OUString& PropertyName )
{
    _setPropertyToDefault( PropertyName, mnParagraph );
}

// lang::XServiceInfo

OUString SAL_CALL SvxUnoTextContent::getImplementationName()
{
    return "SvxUnoTextContent";
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextContent::getSupportedServiceNames()
{
    return comphelper::concatSequences(
        SvxUnoTextRangeBase::getSupportedServiceNames(),
        std::initializer_list<OUStringLiteral>{ u"com.sun.star.style.ParagraphProperties",
                                          u"com.sun.star.style.ParagraphPropertiesComplex",
                                          u"com.sun.star.style.ParagraphPropertiesAsian",
                                          u"com.sun.star.text.TextContent",
                                          u"com.sun.star.text.Paragraph" });
}




SvxUnoTextRangeEnumeration::SvxUnoTextRangeEnumeration(const SvxUnoTextBase& rParentText, sal_Int32 nParagraph, const ESelection& rSel)
:   mxParentText(  const_cast<SvxUnoTextBase*>(&rParentText) ),
    mnNextPortion( 0 )
{
    if (rParentText.GetEditSource())
        mpEditSource = rParentText.GetEditSource()->Clone();

    if( !(mpEditSource && mpEditSource->GetTextForwarder() && (nParagraph == rSel.nStartPara && nParagraph == rSel.nEndPara)) )
        return;

    std::vector<sal_Int32> aPortions;
    mpEditSource->GetTextForwarder()->GetPortions( nParagraph, aPortions );
    for( size_t aPortionIndex = 0; aPortionIndex < aPortions.size(); aPortionIndex++ )
    {
        sal_uInt16 nStartPos = 0;
        if ( aPortionIndex > 0 )
            nStartPos = aPortions.at( aPortionIndex - 1 );
        if( nStartPos > rSel.nEndPos )
            continue;
        sal_uInt16 nEndPos = aPortions.at( aPortionIndex );
        if( nEndPos < rSel.nStartPos )
            continue;

        nStartPos = std::max<int>(nStartPos, rSel.nStartPos);
        nEndPos = std::min<sal_uInt16>(nEndPos, rSel.nEndPos);
        ESelection aSel( nParagraph, nStartPos, nParagraph, nEndPos );

        const SvxUnoTextRangeBaseVec& rRanges( mpEditSource->getRanges() );
        SvxUnoTextRange* pRange = nullptr;
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

SvxUnoTextRangeEnumeration::~SvxUnoTextRangeEnumeration() throw()
{
}

// container::XEnumeration

sal_Bool SAL_CALL SvxUnoTextRangeEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;

    return !maPortions.empty() && mnNextPortion < maPortions.size();
}

uno::Any SAL_CALL SvxUnoTextRangeEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if( maPortions.empty() || mnNextPortion >= maPortions.size() )
        throw container::NoSuchElementException();

    uno::Reference< text::XTextRange > xRange = maPortions.at(mnNextPortion).get();
    mnNextPortion++;
    return uno::makeAny( xRange );
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextBase& rText ) throw()
:   SvxUnoTextRangeBase(rText),
    mxParentText( const_cast<SvxUnoTextBase*>(&rText) )
{
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextCursor& rCursor ) throw()
:   SvxUnoTextRangeBase(rCursor)
,   text::XTextCursor()
,   lang::XTypeProvider()
,   cppu::OWeakAggObject()
,   mxParentText(rCursor.mxParentText)
{
}

SvxUnoTextCursor::~SvxUnoTextCursor() throw()
{
}

// Comment out automatically - [getIdlClass(es) or queryInterface]
// Please use the XTypeProvider!
//sal_Bool SvxUnoTextCursor::queryInterface( uno::Uik aUIK, Reference< uno::XInterface > & xRef)
uno::Any SAL_CALL SvxUnoTextCursor::queryAggregation( const uno::Type & rType )
{
    if( rType == cppu::UnoType<text::XTextRange>::get())
        return uno::makeAny(uno::Reference< text::XTextRange >(static_cast<SvxUnoTextRangeBase *>(this)));
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

uno::Any SAL_CALL SvxUnoTextCursor::queryInterface( const uno::Type & rType )
{
    return OWeakAggObject::queryInterface(rType);
}

void SAL_CALL SvxUnoTextCursor::acquire() throw ( )
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoTextCursor::release() throw ( )
{
    OWeakAggObject::release();
}

namespace
{
    struct theSvxUnoTextCursorTypes :
        public rtl::StaticWithInit<uno::Sequence<uno::Type>, theSvxUnoTextCursorTypes>
    {
        uno::Sequence<uno::Type> operator () ()
        {
            uno::Sequence< uno::Type > aTypeSequence;

            aTypeSequence.realloc( 10 ); // !DANGER! keep this updated
            uno::Type* pTypes = aTypeSequence.getArray();

            *pTypes++ = cppu::UnoType<text::XTextRange>::get();
            *pTypes++ = cppu::UnoType<text::XTextCursor>::get();
            *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
            *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
            *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
            *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
            *pTypes++ = cppu::UnoType<text::XTextRangeCompare>::get();
            *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
            *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
            *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();

            return aTypeSequence;
        }
    };
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoTextCursor::getTypes()
{
    return theSvxUnoTextCursorTypes::get();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextCursor::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// text::XTextCursor
void SAL_CALL SvxUnoTextCursor::collapseToStart()
{
    SolarMutexGuard aGuard;
    CollapseToStart();
}

void SAL_CALL SvxUnoTextCursor::collapseToEnd()
{
    SolarMutexGuard aGuard;
    CollapseToEnd();
}

sal_Bool SAL_CALL SvxUnoTextCursor::isCollapsed()
{
    SolarMutexGuard aGuard;
    return IsCollapsed();
}

sal_Bool SAL_CALL SvxUnoTextCursor::goLeft( sal_Int16 nCount, sal_Bool bExpand )
{
    SolarMutexGuard aGuard;
    return GoLeft( nCount, bExpand );
}

sal_Bool SAL_CALL SvxUnoTextCursor::goRight( sal_Int16 nCount, sal_Bool bExpand )
{
    SolarMutexGuard aGuard;
    return GoRight( nCount, bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoStart( sal_Bool bExpand )
{
    SolarMutexGuard aGuard;
    GotoStart( bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoEnd( sal_Bool bExpand )
{
    SolarMutexGuard aGuard;
    GotoEnd( bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoRange( const uno::Reference< text::XTextRange >& xRange, sal_Bool bExpand )
{
    if( !xRange.is() )
        return;

    SvxUnoTextRangeBase* pRange = comphelper::getUnoTunnelImplementation<SvxUnoTextRangeBase>( xRange );

    if( !pRange )
        return;

    ESelection aNewSel = pRange->GetSelection();

    if( bExpand )
    {
        const ESelection& rOldSel = GetSelection();
        aNewSel.nStartPara = rOldSel.nStartPara;
        aNewSel.nStartPos  = rOldSel.nStartPos;
    }

    SetSelection( aNewSel );
}

// text::XTextRange (rest in SvxTextRange)
uno::Reference< text::XText > SAL_CALL SvxUnoTextCursor::getText()
{
    return mxParentText;
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextCursor::getStart()
{
    return SvxUnoTextRangeBase::getStart();
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextCursor::getEnd()
{
    return SvxUnoTextRangeBase::getEnd();
}

OUString SAL_CALL SvxUnoTextCursor::getString()
{
    return SvxUnoTextRangeBase::getString();
}

void SAL_CALL SvxUnoTextCursor::setString( const OUString& aString )
{
    SvxUnoTextRangeBase::setString(aString);
}
// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextCursor::getImplementationName()
{
    return "SvxUnoTextCursor";
}

sal_Bool SAL_CALL SvxUnoTextCursor::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextCursor::getSupportedServiceNames()
{
    return comphelper::concatSequences(
        SvxUnoTextRangeBase::getSupportedServiceNames(),
        std::initializer_list<OUStringLiteral>{ u"com.sun.star.style.ParagraphProperties",
                                          u"com.sun.star.style.ParagraphPropertiesComplex",
                                          u"com.sun.star.style.ParagraphPropertiesAsian",
                                          u"com.sun.star.text.TextCursor" });
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
