/*************************************************************************
 *
 *  $RCSfile: unotext2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-12 15:47:04 $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <eeitem.hxx>

#define ITEMID_FIELD EE_FEATURE_FIELD
#include "flditem.hxx"
#include "unofield.hxx"
#include "unotext.hxx"

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

// ====================================================================
// SvxUnoTextContentEnumeration
// ====================================================================

SvxUnoTextContentEnumeration::SvxUnoTextContentEnumeration( const SvxUnoText& _rText ) throw()
: rText( _rText )
{
    xParentText =  (text::XText*)&_rText;
    pEditSource = rText.GetEditSource()->Clone();
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
    OGuard aGuard( Application::GetSolarMutex() );
    return nNextParagraph < pEditSource->GetTextForwarder()->GetParagraphCount();
}

uno::Any SvxUnoTextContentEnumeration::nextElement(void) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

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

static SvxUnoText* getDummyText() throw()
{
    if(!xDummyText.is())
        xDummyText = new SvxUnoText();

    return SvxUnoText::getImplementation( xDummyText );
}

SvxUnoTextContent::SvxUnoTextContent() throw()
:   SvxUnoTextRangeBase(*getDummyText()),
    rParentText(*getDummyText()),
    aDisposeListeners(aDisposeContainerMutex),
    nParagraph(0)
{
}

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoText& rText, sal_uInt16 nPara ) throw()
:   SvxUnoTextRangeBase(rText),rParentText(rText),
    aDisposeListeners(aDisposeContainerMutex),
    nParagraph(nPara)
{
    xParentText =  (text::XText*)&rText;
    SetSelection( ESelection( nParagraph,0, nParagraph, GetEditSource()->GetTextForwarder()->GetTextLen( nParagraph ) ) );
}

SvxUnoTextContent::SvxUnoTextContent( const SvxUnoTextContent& rContent ) throw()
:   SvxUnoTextRangeBase(rContent),
    aDisposeListeners(aDisposeContainerMutex),
    rParentText(rContent.rParentText)
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

void SAL_CALL SvxUnoTextContent::acquire() throw( uno::RuntimeException )
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoTextContent::release() throw( uno::RuntimeException )
{
    OWeakAggObject::release();
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextContent::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 8 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
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
    return uno::Reference< text::XTextRange > ();
}

// XComponent

void SAL_CALL SvxUnoTextContent::dispose()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( bDisposing )
        return; // catched a recursion

    bDisposing = sal_True;

    lang::EventObject aEvt;
    aEvt.Source = *(OWeakAggObject*) this;
    aDisposeListeners.disposeAndClear(aEvt);

    // todo:: removemyself
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
    OGuard aGuard( Application::GetSolarMutex() );

    SvUShorts aPortions;
    SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
    pForwarder->GetPortions( nParagraph, aPortions );

    return aPortions.Count() > 0;
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
    SvxServiceInfoHelper::addToSequence( aSeq, 3, "com.sun.star.style.ParagraphProperties",
                                                  "com.sun.star.text.TextContent",
                                                  "com.sun.star.text.Paragraph");
    return aSeq;
}

// ====================================================================
//  class SvxUnoTextRangeEnumeration
// ====================================================================

SvxUnoTextRangeEnumeration::SvxUnoTextRangeEnumeration( const SvxUnoText& rText, sal_uInt16 nPara ) throw()
:   xParentText(  (text::XText*)&rText ),
    rParentText( rText ),
    nParagraph( nPara ),
    nNextPortion( 0 )
{
    pEditSource = rText.GetEditSource()->Clone();
    pPortions = new SvUShorts;
    pEditSource->GetTextForwarder()->GetPortions( nPara, *pPortions );
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
    OGuard aGuard( Application::GetSolarMutex() );

    if(pPortions == NULL)
        throw uno::RuntimeException();

    return nNextPortion < pPortions->Count();
}

uno::Any SAL_CALL SvxUnoTextRangeEnumeration::nextElement()
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( nNextPortion >= pPortions->Count() )
        throw container::NoSuchElementException();

    sal_uInt16 nStartPos = 0;
    if (nNextPortion > 0)
        nStartPos = pPortions->GetObject(nNextPortion-1);
    sal_uInt16 nEndPos = pPortions->GetObject(nNextPortion);
    ESelection aSel( nParagraph, nStartPos, nParagraph, nEndPos );

    uno::Reference< text::XTextRange > xRange;

    SvxTextForwarder* pForwarder = rParentText.GetEditSource()->GetTextForwarder();
    SfxItemSet aSet = pForwarder->GetAttribs( aSel );

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

uno::Reference< uno::XInterface > SvxUnoTextCursor_NewInstance()
{
    SvxUnoText aText;
    uno::Reference< text::XText > xText( (text::XText*)new SvxUnoTextCursor( aText ) );
    uno::Reference< uno::XInterface > xInt( xText, uno::UNO_QUERY );
    return xInt;
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoText& rText ) throw()
:   SvxUnoTextRangeBase(rText),
    xParentText( (text::XText*)&rText)
{
}

SvxUnoTextCursor::SvxUnoTextCursor( const SvxUnoTextCursor& rCursor ) throw()
:   SvxUnoTextRangeBase(rCursor),
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

void SAL_CALL SvxUnoTextCursor::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoTextCursor::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoTextCursor::getTypes()
    throw(uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 7 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextCursor >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
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
    OGuard aGuard( Application::GetSolarMutex() );
    CollapseToStart();
}

void SAL_CALL SvxUnoTextCursor::collapseToEnd()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    CollapseToEnd();
}

sal_Bool SAL_CALL SvxUnoTextCursor::isCollapsed()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return IsCollapsed();
}

sal_Bool SAL_CALL SvxUnoTextCursor::goLeft( sal_Int16 nCount, sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return GoLeft( nCount, bExpand );
}

sal_Bool SAL_CALL SvxUnoTextCursor::goRight( sal_Int16 nCount, sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return GoRight( nCount, bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoStart( sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    GotoStart( bExpand );
}

void SAL_CALL SvxUnoTextCursor::gotoEnd( sal_Bool bExpand )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextCursort"));
}

sal_Bool SAL_CALL SvxUnoTextCursor::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextCursor::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1,"com.sun.star.text.TextCursor");
    return aSeq;
}


