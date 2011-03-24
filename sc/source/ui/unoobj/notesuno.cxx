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
#include "precompiled_sc.hxx"

#include "notesuno.hxx"

#include <svl/smplhint.hxx>
#include <editeng/unotext.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdocapt.hxx>

#include "postit.hxx"
#include "cellsuno.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "miscuno.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  keine Properties fuer Text in Notizen
const SvxItemPropertySet* lcl_GetAnnotationPropertySet()
{
    static SfxItemPropertyMapEntry aAnnotationPropertyMap_Impl[] =
    {
        {0,0,0,0,0,0}
    };
    static SvxItemPropertySet aAnnotationPropertySet_Impl( aAnnotationPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aAnnotationPropertySet_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScAnnotationObj, "ScAnnotationObj", "com.sun.star.sheet.CellAnnotation" )
//SC_SIMPLE_SERVICE_INFO( ScAnnotationShapeObj, "ScAnnotationShapeObj", "com.sun.star.sheet.CellAnnotationShape" )

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
    ScUnoGuard aGuard;

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
    ScUnoGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

rtl::OUString SAL_CALL ScAnnotationObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getString();
}

void SAL_CALL ScAnnotationObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().setString(aText);
}

void SAL_CALL ScAnnotationObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScAnnotationObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

uno::Reference<text::XText> SAL_CALL ScAnnotationObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getEnd();
}

// XSheetAnnotation

table::CellAddress SAL_CALL ScAnnotationObj::getPosition() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet  = aCellPos.Tab();
    aAdr.Column = aCellPos.Col();
    aAdr.Row    = aCellPos.Row();
    return aAdr;
}

rtl::OUString SAL_CALL ScAnnotationObj::getAuthor() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetAuthor() : rtl::OUString();
}

rtl::OUString SAL_CALL ScAnnotationObj::getDate() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetDate() : rtl::OUString();
}

sal_Bool SAL_CALL ScAnnotationObj::getIsVisible() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote && pNote->IsCaptionShown();
}

void SAL_CALL ScAnnotationObj::setIsVisible( sal_Bool bIsVisible ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    // show/hide note with undo action
    if( pDocShell )
        pDocShell->GetDocFunc().ShowNote( aCellPos, bIsVisible );
}

// XSheetAnnotationShapeSupplier
uno::Reference < drawing::XShape > SAL_CALL ScAnnotationObj::getAnnotationShape()
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < drawing::XShape > xShape;
    if( const ScPostIt* pNote = ImplGetNote() )
        if( SdrObject* pCaption = pNote->GetOrCreateCaption( aCellPos ) )
            xShape.set( pCaption->getUnoShape(), uno::UNO_QUERY );
    return xShape;
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
    return pDocShell ? pDocShell->GetDocument()->GetNote( aCellPos ) : 0;
}

//------------------------------------------------------------------------
