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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#define _SVSTDARR_sal_uIt16S
#include <svl/svstdarr.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <rtl/instance.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/unofield.hxx>
#include <editeng/unotext.hxx>
#include <comphelper/serviceinfohelper.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        return uno::makeAny(uno::Reference< xint >(this))

// ====================================================================
// SvxUnoTextContentEnumeration
// ====================================================================

SvxUnoTextContentEnumeration::SvxUnoTextContentEnumeration( const SvxUnoTextBase& _rText ) throw()
: mrText( _rText )
{
    mxParentText = const_cast<SvxUnoTextBase*>(&_rText);
    if( mrText.GetEditSource() )
        mpEditSource = mrText.GetEditSource()->Clone();
    else
        mpEditSource = NULL;
    mnNextParagraph = 0;
}

SvxUnoTextContentEnumeration::~SvxUnoTextContentEnumeration() throw()
{
    delete mpEditSource;
}

// container::XEnumeration
sal_Bool SAL_CALL SvxUnoTextContentEnumeration::hasMoreElements(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if( mpEditSource && mpEditSource->GetTextForwarder() )
        return mnNextParagraph < mpEditSource->GetTextForwarder()->GetParagraphCount();
    else
        return sal_False;
}

uno::Any SvxUnoTextContentEnumeration::nextElement(void) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if(!hasMoreElements())
        throw container::NoSuchElementException();

    SvxUnoTextContent* pContent = 0;

    const SvxUnoTextRangeBaseList& rRanges( mpEditSource->getRanges() );
    SvxUnoTextRangeBaseList::const_iterator aIter;
    for( aIter = rRanges.begin(); (aIter != rRanges.end()) && (pContent == 0); ++aIter )
    {
        SvxUnoTextContent* pIterContent = dynamic_cast< SvxUnoTextContent* >( (*aIter ) );
        if( pIterContent && (pIterContent->mnParagraph == mnNextParagraph) )
            pContent = pIterContent;
    }

    if( pContent == 0 )
     pContent = new SvxUnoTextContent( mrText, mnNextParagraph );

    mnNextParagraph++;

    uno::Reference< text::XTextContent > xRef( pContent );
    return uno::makeAny( xRef );
}

// ====================================================================
// class SvxUnoTextContent
// ====================================================================
uno::Reference< text::XText > xDummyText;

static SvxUnoText* getDummyText() throw()
{
    if(!xDummyText.is())
        xDummyText = new SvxUnoText();

    return SvxUnoText::getImplementation( xDummyText );
}

SvxUnoTextContent::SvxUnoTextContent() throw()
:   SvxUnoTextRangeBase(*getDummyText())
,   mnParagraph(0)
,   mrParentText(*getDummyText())
,   maDisposeListeners(maDisposeContainerMutex)
,   mbDisposing( false )
{
}

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextBase& rText, sal_uInt16 nPara ) throw()
:   SvxUnoTextRangeBase(rText)
,   mnParagraph(nPara)
,   mrParentText(rText)
,   maDisposeListeners(maDisposeContainerMutex)
,   mbDisposing( false )
{
    mxParentText = const_cast<SvxUnoTextBase*>(&rText);
    if( GetEditSource() && GetEditSource()->GetTextForwarder() )
        SetSelection( ESelection( mnParagraph,0, mnParagraph, GetEditSource()->GetTextForwarder()->GetTextLen( mnParagraph ) ) );
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
uno::Any SAL_CALL SvxUnoTextContent::queryAggregation( const uno::Type & rType ) throw( uno::RuntimeException )
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

uno::Any SAL_CALL SvxUnoTextContent::queryInterface( const uno::Type & rType ) throw( uno::RuntimeException )
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

            *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertySet >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertyStates >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRangeCompare >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextContent >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< container::XEnumerationAccess >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);

            return aTypeSequence;
        }
    };
}

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextContent::getTypes()
    throw (uno::RuntimeException)
{
    return theSvxUnoTextContentTypes::get();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextContent::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// text::XTextRange

uno::Reference< text::XText > SAL_CALL SvxUnoTextContent::getText()
    throw(uno::RuntimeException)
{
    return mxParentText;
}

// text::XTextContent
void SAL_CALL SvxUnoTextContent::attach( const uno::Reference< text::XTextRange >& )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextContent::getAnchor() throw( uno::RuntimeException )
{
    return uno::Reference< text::XTextRange >::query( mxParentText );
}

// XComponent

void SAL_CALL SvxUnoTextContent::dispose()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mbDisposing )
        return; // catched a recursion

    mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *(OWeakAggObject*) this;
    maDisposeListeners.disposeAndClear(aEvt);

    if( mxParentText.is() )
        mxParentText->removeTextContent( this );
}

void SAL_CALL SvxUnoTextContent::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    maDisposeListeners.addInterface(xListener);
}

void SAL_CALL SvxUnoTextContent::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw(uno::RuntimeException)
{
   maDisposeListeners.removeInterface(aListener);
}

// XEnumerationAccess

uno::Reference< container::XEnumeration > SAL_CALL SvxUnoTextContent::createEnumeration(  )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return new SvxUnoTextRangeEnumeration( mrParentText, mnParagraph );
}

// XElementAccess ( container::XEnumerationAccess )

uno::Type SAL_CALL SvxUnoTextContent::getElementType()
    throw(uno::RuntimeException)
{
    return ::getCppuType((const uno::Reference< text::XTextRange >*)0);
}

sal_Bool SAL_CALL SvxUnoTextContent::hasElements()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SvxTextForwarder* pForwarder = GetEditSource() ? GetEditSource()->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        SvUShorts aPortions;
        pForwarder->GetPortions( mnParagraph, aPortions );
        return aPortions.Count() > 0;
    }
    else
    {
        return 0;
    }
}

// XPropertySet

void SAL_CALL SvxUnoTextContent::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    _setPropertyValue( aPropertyName, aValue, mnParagraph );
}

uno::Any SAL_CALL SvxUnoTextContent::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return _getPropertyValue( PropertyName, mnParagraph );
}

// XMultiPropertySet
void SAL_CALL SvxUnoTextContent::setPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues ) throw (beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    _setPropertyValues( aPropertyNames, aValues, mnParagraph );
}

uno::Sequence< uno::Any > SAL_CALL SvxUnoTextContent::getPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (uno::RuntimeException)
{
    return _getPropertyValues( aPropertyNames, mnParagraph );
}

/*// XTolerantMultiPropertySet
uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL SvxUnoTextContent::setPropertyValuesTolerant( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return _setPropertyValuesTolerant(aPropertyNames, aValues, mnParagraph);
}

uno::Sequence< beans::GetPropertyTolerantResult > SAL_CALL SvxUnoTextContent::getPropertyValuesTolerant( const uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (uno::RuntimeException)
{
    return _getPropertyValuesTolerant(aPropertyNames, mnParagraph);
}

uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL SvxUnoTextContent::getDirectPropertyValuesTolerant( const uno::Sequence< ::rtl::OUString >& aPropertyNames )
    throw (uno::RuntimeException)
{
    return _getDirectPropertyValuesTolerant(aPropertyNames, mnParagraph);
}*/

// beans::XPropertyState
beans::PropertyState SAL_CALL SvxUnoTextContent::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return _getPropertyState( PropertyName, mnParagraph );
}

uno::Sequence< beans::PropertyState > SAL_CALL SvxUnoTextContent::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return _getPropertyStates( aPropertyName, mnParagraph );
}

void SAL_CALL SvxUnoTextContent::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    _setPropertyToDefault( PropertyName, mnParagraph );
}

// lang::XServiceInfo

OUString SAL_CALL SvxUnoTextContent::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextContent") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextContent::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, 5, "com.sun.star.style.ParagraphProperties",
                                                  "com.sun.star.style.ParagraphPropertiesComplex",
                                                  "com.sun.star.style.ParagraphPropertiesAsian",
                                                  "com.sun.star.text.TextContent",
                                                  "com.sun.star.text.Paragraph");
    return aSeq;
}

// ====================================================================
//  class SvxUnoTextRangeEnumeration
// ====================================================================

SvxUnoTextRangeEnumeration::SvxUnoTextRangeEnumeration( const SvxUnoTextBase& rText, sal_uInt16 nPara ) throw()
:   mxParentText(  const_cast<SvxUnoTextBase*>(&rText) ),
    mrParentText( rText ),
    mnParagraph( nPara ),
    mnNextPortion( 0 )
{
    mpEditSource = rText.GetEditSource() ? rText.GetEditSource()->Clone() : NULL;

    if( mpEditSource && mpEditSource->GetTextForwarder() )
    {
        mpPortions = new SvUShorts;
        mpEditSource->GetTextForwarder()->GetPortions( nPara, *mpPortions );
    }
    else
    {
        mpPortions = NULL;
    }
}

SvxUnoTextRangeEnumeration::~SvxUnoTextRangeEnumeration() throw()
{
    delete mpEditSource;
    delete mpPortions;
}

// container::XEnumeration

sal_Bool SAL_CALL SvxUnoTextRangeEnumeration::hasMoreElements()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return mpPortions && mnNextPortion < mpPortions->Count();
}

uno::Any SAL_CALL SvxUnoTextRangeEnumeration::nextElement()
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpPortions == NULL || mnNextPortion >= mpPortions->Count() )
        throw container::NoSuchElementException();

    sal_uInt16 nStartPos = 0;
    if (mnNextPortion > 0)
        nStartPos = mpPortions->GetObject(mnNextPortion-1);
    sal_uInt16 nEndPos = mpPortions->GetObject(mnNextPortion);
    ESelection aSel( mnParagraph, nStartPos, mnParagraph, nEndPos );

    uno::Reference< text::XTextRange > xRange;

    const SvxUnoTextRangeBaseList& rRanges( mpEditSource->getRanges() );

    SvxUnoTextRange* pRange = 0;

    SvxUnoTextRangeBaseList::const_iterator aIter;
    for( aIter = rRanges.begin(); (aIter != rRanges.end()) && (pRange == 0); ++aIter )
    {
        SvxUnoTextRange* pIterRange = dynamic_cast< SvxUnoTextRange* >( (*aIter ) );
        if( pIterRange && pIterRange->mbPortion && (aSel.IsEqual( pIterRange->maSelection ) ) )
            pRange = pIterRange;
    }

    if( pRange == 0 )
    {
        pRange = new SvxUnoTextRange( mrParentText, sal_True );
        pRange->SetSelection(aSel);
    }

    xRange = pRange;

    mnNextPortion++;

    return uno::makeAny( xRange );
}

// ====================================================================
// class SvxUnoTextCursor
// ====================================================================

uno::Reference< uno::XInterface > SvxUnoTextCursor_NewInstance()
{
    SvxUnoText aText;
    uno::Reference< text::XText > xText( (text::XText*)new SvxUnoTextCursor( aText ) );
    uno::Reference< uno::XInterface > xInt( xText, uno::UNO_QUERY );
    return xInt;
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
    throw(uno::RuntimeException)
{
    if( rType == ::getCppuType((const uno::Reference< text::XTextRange >*)0) )
        return uno::makeAny(uno::Reference< text::XTextRange >((text::XText*)(this)));
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
    throw(uno::RuntimeException)
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

            *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextCursor >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertySet >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertyStates >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRangeCompare >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
            *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);

            return aTypeSequence;
        }
    };
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoTextCursor::getTypes()
    throw(uno::RuntimeException)
{
    return theSvxUnoTextCursorTypes::get();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextCursor::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// text::XTextCursor
void SAL_CALL SvxUnoTextCursor::collapseToStart()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    CollapseToStart();
}

void SAL_CALL SvxUnoTextCursor::collapseToEnd()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    CollapseToEnd();
}

sal_Bool SAL_CALL SvxUnoTextCursor::isCollapsed()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return IsCollapsed();
}

sal_Bool SAL_CALL SvxUnoTextCursor::goLeft( sal_Int16 nCount, sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GoLeft( nCount, bExpand );
}

sal_Bool SAL_CALL SvxUnoTextCursor::goRight( sal_Int16 nCount, sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GoRight( nCount, bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoStart( sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GotoStart( bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoEnd( sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GotoEnd( bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoRange( const uno::Reference< text::XTextRange >& xRange, sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    if( !xRange.is() )
        return;

    SvxUnoTextRangeBase* pRange = SvxUnoTextRangeBase::getImplementation( xRange );

    if( pRange )
    {
        ESelection aNewSel = pRange->GetSelection();

        if( bExpand )
        {
            const ESelection& rOldSel = GetSelection();
            aNewSel.nStartPara = rOldSel.nStartPara;
            aNewSel.nStartPos  = rOldSel.nStartPos;
        }

        SetSelection( aNewSel );
    }
}

// text::XTextRange (rest in SvxTextRange)
uno::Reference< text::XText > SAL_CALL SvxUnoTextCursor::getText(void) throw( uno::RuntimeException )
{
    return mxParentText;
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextCursor::getStart()
    throw(uno::RuntimeException)
{
    return SvxUnoTextRangeBase::getStart();
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextCursor::getEnd()
    throw(uno::RuntimeException)
{
    return SvxUnoTextRangeBase::getEnd();
}

OUString SAL_CALL SvxUnoTextCursor::getString() throw( uno::RuntimeException )
{
    return SvxUnoTextRangeBase::getString();
}

void SAL_CALL SvxUnoTextCursor::setString( const OUString& aString ) throw(uno::RuntimeException)
{
    SvxUnoTextRangeBase::setString(aString);
}
// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextCursor::getImplementationName() throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextCursor"));
}

sal_Bool SAL_CALL SvxUnoTextCursor::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextCursor::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, 4,"com.sun.star.style.ParagraphProperties",
                                                  "com.sun.star.style.ParagraphPropertiesComplex",
                                                  "com.sun.star.style.ParagraphPropertiesAsian",
                                                 "com.sun.star.text.TextCursor");
    return aSeq;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
