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

#include <svl/smplhint.hxx>


#include "rangelst.hxx"
#include <editeng/unotext.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <vcl/svapp.hxx>
#include "notesuno.hxx"
#include "textuno.hxx"
#include "cellsuno.hxx"     // getParent
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "miscuno.hxx"
#include "fielduno.hxx"

// setVisible:
#include <svx/svdundo.hxx>
#include "drwlayer.hxx"
#include "detfunc.hxx"
#include "undocell.hxx"
#include "userdat.hxx"
#include <editeng/outlobj.hxx>
#include "editeng/unofield.hxx"
#include <svx/unoshape.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <editeng/unoprnms.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------------

static const SvxItemPropertySet* lcl_GetAnnotationPropertySet()
{
    static SfxItemPropertyMapEntry aAnnotationPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_NUMBERING_PROPERTIE,    // for completeness of service ParagraphProperties
        {0,0,0,0,0,0}
    };
    static SvxItemPropertySet aAnnotationPropertySet_Impl( aAnnotationPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aAnnotationPropertySet_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScAnnotationObj, "ScAnnotationObj", "com.sun.star.sheet.CellAnnotation" )
SC_SIMPLE_SERVICE_INFO( ScAnnotationShapeObj, "ScAnnotationShapeObj", "com.sun.star.sheet.CellAnnotationShape" )

//------------------------------------------------------------------------

ScAnnotationObj::ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    pUnoText( NULL )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

ScAnnotationObj::~ScAnnotationObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    if (pUnoText)
        pUnoText->release();
}

void ScAnnotationObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}


// XChild

uno::Reference<uno::XInterface> SAL_CALL ScAnnotationObj::getParent() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //  Parent der Notiz ist die zugehoerige Zelle
    //! existierendes Objekt finden und zurueckgeben ???

    if (pDocShell)
        return (cppu::OWeakObject*)new ScCellObj( pDocShell, aCellPos );

    return NULL;
}

void SAL_CALL ScAnnotationObj::setParent( const uno::Reference<uno::XInterface>& /* Parent */ )
                                    throw(lang::NoSupportException, uno::RuntimeException)
{
    //  hamma nich
    //! Exception oder so ??!
}

// XSimpleText

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

rtl::OUString SAL_CALL ScAnnotationObj::getString() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getString();
}

void SAL_CALL ScAnnotationObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().setString(aText);
}

void SAL_CALL ScAnnotationObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScAnnotationObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

uno::Reference<text::XText> SAL_CALL ScAnnotationObj::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getEnd();
}

// XSheetAnnotation

table::CellAddress SAL_CALL ScAnnotationObj::getPosition() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet  = aCellPos.Tab();
    aAdr.Column = aCellPos.Col();
    aAdr.Row    = aCellPos.Row();
    return aAdr;
}

rtl::OUString SAL_CALL ScAnnotationObj::getAuthor() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetAuthor() : rtl::OUString();
}

rtl::OUString SAL_CALL ScAnnotationObj::getDate() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetDate() : rtl::OUString();
}

sal_Bool SAL_CALL ScAnnotationObj::getIsVisible() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote && pNote->IsCaptionShown();
}

void SAL_CALL ScAnnotationObj::setIsVisible( sal_Bool bIsVisible ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    // show/hide note with undo action
    if( pDocShell )
        pDocShell->GetDocFunc().ShowNote( aCellPos, bIsVisible );
}

// XSheetAnnotationShapeSupplier
uno::Reference < drawing::XShape > SAL_CALL ScAnnotationObj::getAnnotationShape()
                                throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScAnnotationShapeObj(pDocShell, aCellPos);
}

SvxUnoText& ScAnnotationObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertySet(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

const ScPostIt* ScAnnotationObj::ImplGetNote() const
{
    return pDocShell ? pDocShell->GetDocument()->GetNotes( aCellPos.Tab() )->findByAddress(aCellPos) : 0;
}
//------------------------------------------------------------------------

ScAnnotationShapeObj::ScAnnotationShapeObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    pUnoText( NULL )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

SvxUnoText& ScAnnotationShapeObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertySet(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

uno::Reference < drawing::XShape > ScAnnotationShapeObj::GetXShape()
{
    if (!xShape.is())
        if( ScPostIt* pNote = pDocShell->GetDocument()->GetNotes( aCellPos.Tab() )->findByAddress(aCellPos) )
            if( SdrObject* pCaption = pNote->GetOrCreateCaption( aCellPos ) )
                xShape.set( pCaption->getUnoShape(), uno::UNO_QUERY );
    return xShape;
}

ScAnnotationShapeObj::~ScAnnotationShapeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
    if (pUnoText)
        pUnoText->release();
}

void ScAnnotationShapeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}


// XChild

uno::Reference<uno::XInterface> SAL_CALL ScAnnotationShapeObj::getParent() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //  Parent der Notiz ist die zugehoerige Zelle
    //! existierendes Objekt finden und zurueckgeben ???

    if (pDocShell)
        return (cppu::OWeakObject*)new ScCellObj( pDocShell, aCellPos );

    return NULL;
}

void SAL_CALL ScAnnotationShapeObj::setParent( const uno::Reference<uno::XInterface>& /* Parent */ )
                                    throw(lang::NoSupportException, uno::RuntimeException)
{
    //  hamma nich
    //! Exception oder so ??!
}

// XElementAccess
uno::Type SAL_CALL ScAnnotationShapeObj::getElementType(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return GetUnoText().getElementType();
}

sal_Bool SAL_CALL ScAnnotationShapeObj::hasElements(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return GetUnoText().hasElements();
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL ScAnnotationShapeObj::createEnumeration(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return GetUnoText().createEnumeration();
}

// XTextRangeMover
void SAL_CALL ScAnnotationShapeObj::moveTextRange( const uno::Reference< text::XTextRange >& xRange, sal_Int16 nParagraphs )
                                throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetUnoText().moveTextRange( xRange, nParagraphs );
}

// XText
void SAL_CALL ScAnnotationShapeObj::insertTextContent( const uno::Reference< text::XTextRange >& xRange,
                                    const uno::Reference< text::XTextContent >& xContent, sal_Bool bAbsorb )
                                    throw (lang::IllegalArgumentException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().insertTextContent(xRange, xContent, bAbsorb);
}

void SAL_CALL ScAnnotationShapeObj::removeTextContent( const uno::Reference< text::XTextContent >& xContent )
                                    throw (container::NoSuchElementException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetUnoText().removeTextContent( xContent );
}

// XSimpleText

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationShapeObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationShapeObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

rtl::OUString SAL_CALL ScAnnotationShapeObj::getString() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getString();
}

void SAL_CALL ScAnnotationShapeObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().setString(aText);
}

void SAL_CALL ScAnnotationShapeObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScAnnotationShapeObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetUnoText().insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

uno::Reference<text::XText> SAL_CALL ScAnnotationShapeObj::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationShapeObj::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationShapeObj::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetUnoText().getEnd();
}

// XShapeDescriptor
::rtl::OUString SAL_CALL ScAnnotationShapeObj::getShapeType(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < drawing::XShapeDescriptor > xDesc(GetXShape(), uno::UNO_QUERY);
    if (xDesc.is())
        return xDesc->getShapeType();
    return rtl::OUString();
}

// XShape
awt::Point SAL_CALL ScAnnotationShapeObj::getPosition(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetXShape();
    return xShape.is() ? xShape->getPosition() : awt::Point();
}

void SAL_CALL ScAnnotationShapeObj::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetXShape();
    if( xShape.is() )
        xShape->setPosition(aPosition);
}

awt::Size SAL_CALL ScAnnotationShapeObj::getSize(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetXShape();
    return xShape.is() ? xShape->getSize() : awt::Size();
}

void SAL_CALL ScAnnotationShapeObj::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetXShape();
    if( xShape.is() )
        xShape->setSize(aSize);
}

// XPropertyState
beans::PropertyState SAL_CALL ScAnnotationShapeObj::getPropertyState( const rtl::OUString& PropertyName )
                                    throw (beans::UnknownPropertyException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        return xState->getPropertyState( PropertyName );
    return beans::PropertyState();
}

uno::Sequence< beans::PropertyState > SAL_CALL ScAnnotationShapeObj::getPropertyStates(
                                    const uno::Sequence< rtl::OUString >& aPropertyName )
                                    throw (beans::UnknownPropertyException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        return xState->getPropertyStates( aPropertyName );
    return uno::Sequence< beans::PropertyState >();
}

void SAL_CALL ScAnnotationShapeObj::setPropertyToDefault( const ::rtl::OUString& PropertyName )
                                    throw (::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        xState->setPropertyToDefault( PropertyName );
}

uno::Any SAL_CALL ScAnnotationShapeObj::getPropertyDefault( const rtl::OUString& aPropertyName )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        return xState->getPropertyDefault( aPropertyName );
    return uno::Any();
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScAnnotationShapeObj::getPropertySetInfo(  )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->getPropertySetInfo();
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL ScAnnotationShapeObj::setPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
                                    throw (beans::UnknownPropertyException,
                                        beans::PropertyVetoException,
                                        lang::IllegalArgumentException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL ScAnnotationShapeObj::getPropertyValue( const rtl::OUString& PropertyName )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->getPropertyValue( PropertyName );
    return uno::Any();
}

void SAL_CALL ScAnnotationShapeObj::addPropertyChangeListener( const rtl::OUString& aPropertyName,
                                    const uno::Reference< beans::XPropertyChangeListener >& xListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL ScAnnotationShapeObj::removePropertyChangeListener( const rtl::OUString& aPropertyName,
                                    const uno::Reference< beans::XPropertyChangeListener >& aListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScAnnotationShapeObj::addVetoableChangeListener( const rtl::OUString& PropertyName,
                                    const uno::Reference< beans::XVetoableChangeListener >& aListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL ScAnnotationShapeObj::removeVetoableChangeListener( const rtl::OUString& PropertyName,
                                    const uno::Reference< beans::XVetoableChangeListener >& aListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->removeVetoableChangeListener( PropertyName, aListener );
}

                            // XMultiPropertySet
void SAL_CALL ScAnnotationShapeObj::setPropertyValues( const uno::Sequence< rtl::OUString >& aPropertyNames,
                                    const uno::Sequence< uno::Any >& aValues )
                                    throw (beans::PropertyVetoException,
                                        lang::IllegalArgumentException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->setPropertyValues( aPropertyNames, aValues );
}

uno::Sequence< uno::Any > SAL_CALL ScAnnotationShapeObj::getPropertyValues(
                                    const uno::Sequence< rtl::OUString >& aPropertyNames )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->getPropertyValues( aPropertyNames );
    return uno::Sequence< uno::Any >();
}

void SAL_CALL ScAnnotationShapeObj::addPropertiesChangeListener( const uno::Sequence< rtl::OUString >& aPropertyNames,
                                    const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->addPropertiesChangeListener( aPropertyNames, xListener );
}

void SAL_CALL ScAnnotationShapeObj::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->removePropertiesChangeListener( xListener );
}

void SAL_CALL ScAnnotationShapeObj::firePropertiesChangeEvent( const uno::Sequence< rtl::OUString >& aPropertyNames,
                                    const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->firePropertiesChangeEvent( aPropertyNames, xListener );
}

                            // XComponent
void SAL_CALL ScAnnotationShapeObj::dispose(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < lang::XComponent > xComp (GetXShape(), uno::UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    if (xShape.is())
        xShape.clear();
}

void SAL_CALL ScAnnotationShapeObj::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < lang::XComponent > xComp (GetXShape(), uno::UNO_QUERY);
    if (xComp.is())
        xComp->addEventListener( xListener );
}

void SAL_CALL ScAnnotationShapeObj::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
                                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference < lang::XComponent > xComp (GetXShape(), uno::UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener( aListener );
}

//------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
