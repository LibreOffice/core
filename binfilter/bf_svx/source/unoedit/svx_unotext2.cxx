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

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <eeitem.hxx>

#define ITEMID_FIELD EE_FEATURE_FIELD


#include <bf_svtools/poolitem.hxx>

#include <bf_svx/itemdata.hxx>

#include "unotext.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

// ====================================================================
// SvxUnoTextContentEnumeration
// ====================================================================

SvxUnoTextContentEnumeration::SvxUnoTextContentEnumeration( const SvxUnoTextBase& _rText ) throw()
: rText( _rText )
{
    xParentText =  const_cast<SvxUnoTextBase*>(&rText);
    if( rText.GetEditSource() )
        pEditSource = rText.GetEditSource()->Clone();
    else
        pEditSource = NULL;
    nNextParagraph = 0;
}

SvxUnoTextContentEnumeration::~SvxUnoTextContentEnumeration() throw()
{
    delete pEditSource;
}

// container::XEnumeration
sal_Bool SAL_CALL SvxUnoTextContentEnumeration::hasMoreElements(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return nNextParagraph < pEditSource->GetTextForwarder()->GetParagraphCount();
}

uno::Any SvxUnoTextContentEnumeration::nextElement(void) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if(!hasMoreElements())
        throw container::NoSuchElementException();

    SvxUnoTextContent* pContent = new SvxUnoTextContent( rText, nNextParagraph++ );
    uno::Reference< text::XTextContent > xRef( pContent );
    return uno::Any( &xRef, ::getCppuType((const uno::Reference< text::XTextContent>*)0) );
}

// ====================================================================
// class SvxUnoTextContent
// ====================================================================
uno::Reference< text::XText > xDummyText;
uno::Sequence< uno::Type > SvxUnoTextContent::maTypeSequence;

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextBase& rText, sal_uInt16 nPara ) throw()
:	SvxUnoTextRangeBase(rText),rParentText(rText),
    aDisposeListeners(aDisposeContainerMutex),
    nParagraph(nPara),
    bDisposing( sal_False )
{
    xParentText =  const_cast<SvxUnoTextBase*>(&rText);
    SetSelection( ESelection( nParagraph,0, nParagraph, GetEditSource()->GetTextForwarder()->GetTextLen( nParagraph ) ) );
}

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextContent& rContent ) throw()
:	SvxUnoTextRangeBase(rContent),
    aDisposeListeners(aDisposeContainerMutex),
    rParentText(rContent.rParentText),
    bDisposing( sal_False )
{
    xParentText = rContent.xParentText;
    nParagraph  = rContent.nParagraph;
    SetSelection( rContent.GetSelection() );
}

SvxUnoTextContent::~SvxUnoTextContent() throw()
{
}

// uno::XInterface
uno::Any SAL_CALL SvxUnoTextContent::queryAggregation( const uno::Type & rType ) throw( uno::RuntimeException )
{
    uno::Any aAny;

    QUERYINT( text::XTextRange );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XMultiPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( text::XTextContent );
    else QUERYINT( lang::XComponent );
    else QUERYINT( container::XEnumerationAccess );
    else QUERYINT( container::XElementAccess );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OWeakAggObject::queryAggregation( rType );

    return aAny;
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

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextContent::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 9 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextContent >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< container::XEnumerationAccess >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);
    }
    return maTypeSequence;
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
    return xParentText;
}

// text::XTextContent
void SAL_CALL SvxUnoTextContent::attach( const uno::Reference< text::XTextRange >& xTextRange )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextContent::getAnchor() throw( uno::RuntimeException )
{
    return uno::Reference< text::XTextRange >::query( xParentText );
}

// XComponent

void SAL_CALL SvxUnoTextContent::dispose()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        return;	// catched a recursion

    bDisposing = sal_True;

    lang::EventObject aEvt;
    aEvt.Source = *(OWeakAggObject*) this;
    aDisposeListeners.disposeAndClear(aEvt);

    if( xParentText.is() )
        xParentText->removeTextContent( this );
}

void SAL_CALL SvxUnoTextContent::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    aDisposeListeners.addInterface(xListener);
}

void SAL_CALL SvxUnoTextContent::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw(uno::RuntimeException)
{
   aDisposeListeners.removeInterface(aListener);
}

// XEnumerationAccess

uno::Reference< container::XEnumeration > SAL_CALL SvxUnoTextContent::createEnumeration(  )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return new SvxUnoTextRangeEnumeration( rParentText, nParagraph );
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
        pForwarder->GetPortions( nParagraph, aPortions );
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
    _setPropertyValue( aPropertyName, aValue, nParagraph );
}

uno::Any SAL_CALL SvxUnoTextContent::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return _getPropertyValue( PropertyName, nParagraph );
}

// XMultiPropertySet
void SAL_CALL SvxUnoTextContent::setPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues ) throw (beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    _setPropertyValues( aPropertyNames, aValues, nParagraph );
}

uno::Sequence< uno::Any > SAL_CALL SvxUnoTextContent::getPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (uno::RuntimeException)
{
    return _getPropertyValues( aPropertyNames, nParagraph );
}

// beans::XPropertyState
beans::PropertyState SAL_CALL SvxUnoTextContent::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return _getPropertyState( PropertyName, nParagraph );
}

uno::Sequence< beans::PropertyState > SAL_CALL SvxUnoTextContent::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return _getPropertyStates( aPropertyName, nParagraph );
}

void SAL_CALL SvxUnoTextContent::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    _setPropertyToDefault( PropertyName, nParagraph );
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
    SvxServiceInfoHelper::addToSequence( aSeq, 5, "com.sun.star.style.ParagraphProperties",
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
:	xParentText(  const_cast<SvxUnoTextBase*>(&rText) ),
    rParentText( rText ),
    nParagraph( nPara ),
    nNextPortion( 0 )
{
    pEditSource = rText.GetEditSource() ? rText.GetEditSource()->Clone() : NULL;

    if( pEditSource && pEditSource->GetTextForwarder() )
    {
        pPortions = new SvUShorts;
        pEditSource->GetTextForwarder()->GetPortions( nPara, *pPortions );
    }
    else
    {
        pPortions = NULL;
    }
}

SvxUnoTextRangeEnumeration::~SvxUnoTextRangeEnumeration() throw()
{
    delete pEditSource;
    delete pPortions;
}

// container::XEnumeration

sal_Bool SAL_CALL SvxUnoTextRangeEnumeration::hasMoreElements()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return pPortions ? nNextPortion < pPortions->Count() : 0;
}

uno::Any SAL_CALL SvxUnoTextRangeEnumeration::nextElement()
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( pPortions == NULL || nNextPortion >= pPortions->Count() )
        throw container::NoSuchElementException();

    sal_uInt16 nStartPos = 0;
    if (nNextPortion > 0)
        nStartPos = pPortions->GetObject(nNextPortion-1);
    sal_uInt16 nEndPos = pPortions->GetObject(nNextPortion);
    ESelection aSel( nParagraph, nStartPos, nParagraph, nEndPos );

    uno::Reference< text::XTextRange > xRange;

    SvxTextForwarder* pForwarder = rParentText.GetEditSource()->GetTextForwarder();

    SvxUnoTextRange* pRange = new SvxUnoTextRange( rParentText, sal_True );
    xRange = pRange;

    pRange->SetSelection(aSel);
    nNextPortion++;

    return uno::Any( &xRange, ::getCppuType((const uno::Reference< text::XTextRange >*)0) );
}

// ====================================================================
// class SvxUnoTextCursor
// ====================================================================

uno::Sequence< uno::Type > SvxUnoTextCursor::maTypeSequence;

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextBase& rText ) throw()
:	SvxUnoTextRangeBase(rText),
    xParentText( const_cast<SvxUnoTextBase*>(&rText) )
{
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextCursor& rCursor ) throw()
:	SvxUnoTextRangeBase(rCursor),
    xParentText(rCursor.xParentText)
{
}

SvxUnoTextCursor::~SvxUnoTextCursor() throw()
{
}

// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//sal_Bool SvxUnoTextCursor::queryInterface( uno::Uik aUIK, Reference< uno::XInterface > & xRef)
uno::Any SAL_CALL SvxUnoTextCursor::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    if( rType == ::getCppuType((const uno::Reference< text::XTextRange >*)0) )
        aAny <<= uno::Reference< text::XTextRange >((text::XText*)(this));
    else QUERYINT( text::XTextCursor );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XMultiPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OWeakAggObject::queryAggregation( rType );

    return aAny;
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

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoTextCursor::getTypes()
    throw(uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 8 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextCursor >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);
    }
    return maTypeSequence;
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
    return xParentText;
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
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextCursor::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 4,"com.sun.star.style.ParagraphProperties",
                                                  "com.sun.star.style.ParagraphPropertiesComplex",
                                                  "com.sun.star.style.ParagraphPropertiesAsian",
                                                 "com.sun.star.text.TextCursor");
    return aSeq;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
