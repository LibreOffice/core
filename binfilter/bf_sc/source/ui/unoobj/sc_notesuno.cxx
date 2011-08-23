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


#include "notesuno.hxx"
#include "textuno.hxx"
#include "cellsuno.hxx"		// getParent
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "miscuno.hxx"

// setVisible:
#include <bf_svx/svdundo.hxx>
#include "drwlayer.hxx"
#include "detfunc.hxx"
#include "undocell.hxx"
#include <vcl/svapp.hxx>
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

//	keine Properties fuer Text in Notizen
const SfxItemPropertyMap* lcl_GetAnnotationPropertyMap()
{
    static SfxItemPropertyMap aAnnotationPropertyMap_Impl[] =
    {
        {0,0,0,0}
    };
    return aAnnotationPropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScAnnotationObj, "ScAnnotationObj", "com.sun.star.sheet.CellAnnotation" )

//------------------------------------------------------------------------

ScAnnotationObj::ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    pUnoText( NULL )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    //	pUnoText is allocated on demand (GetUnoText)
    //	can't be aggregated because getString/setString is handled here
}

SvxUnoText&	ScAnnotationObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertyMap(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

ScAnnotationObj::~ScAnnotationObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    if (pUnoText)
        pUnoText->release();
}

void ScAnnotationObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //!	Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}


// XChild

uno::Reference<uno::XInterface> SAL_CALL ScAnnotationObj::getParent() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //	Parent der Notiz ist die zugehoerige Zelle
    //!	existierendes Objekt finden und zurueckgeben ???

    if (pDocShell)
        return (cppu::OWeakObject*)new ScCellObj( pDocShell, aCellPos );

    return NULL;
}

void SAL_CALL ScAnnotationObj::setParent( const uno::Reference<uno::XInterface>& Parent )
                                    throw(lang::NoSupportException, uno::RuntimeException)
{
    //	hamma nich
    //!	Exception oder so ??!
}

// XSimpleText

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //	Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //	Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

::rtl::OUString SAL_CALL ScAnnotationObj::getString() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScPostIt aNote;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
    }
    return aNote.GetText();
}

void SAL_CALL ScAnnotationObj::setString( const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        String aString = aText;
        ScDocFunc aFunc(*pDocShell);
        aFunc.SetNoteText( aCellPos, aString, TRUE );

        // don't create pUnoText here if not there
        if (pUnoText)
            pUnoText->SetSelection(ESelection( 0,0, 0,aString.Len() ));
    }
}

void SAL_CALL ScAnnotationObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const ::rtl::OUString& aString, sal_Bool bAbsorb )
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
    aAdr.Sheet	= aCellPos.Tab();
    aAdr.Column	= aCellPos.Col();
    aAdr.Row	= aCellPos.Row();
    return aAdr;
}

::rtl::OUString SAL_CALL ScAnnotationObj::getAuthor() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScPostIt aNote;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
    }
    return aNote.GetAuthor();
}

::rtl::OUString SAL_CALL ScAnnotationObj::getDate() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScPostIt aNote;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
    }
    return aNote.GetDate();
}

sal_Bool SAL_CALL ScAnnotationObj::getIsVisible() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScPostIt aNote;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
    }
    return aNote.IsShown();
}

void SAL_CALL ScAnnotationObj::setIsVisible( sal_Bool bIsVisible ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    BOOL bDone = FALSE;
    if ( pDocShell )
    {
        //!	Funktion an docfunc oder so

        BOOL bSet = bIsVisible ? TRUE : FALSE;
        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nCol = aCellPos.Col();
        USHORT nRow = aCellPos.Row();
        USHORT nTab = aCellPos.Tab();
        ScPostIt aNote;
        if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) )
        {
            BOOL bHad = pDoc->HasNoteObject( nCol, nRow, nTab );
            if ( bHad != bSet )
            {
                pDocShell->MakeDrawLayer();
                ScDrawLayer* pModel = pDoc->GetDrawLayer();

                ScDetectiveFunc aFunc( pDoc,nTab );
                if ( bSet )
                    bDone = ( aFunc.ShowComment( nCol, nRow, FALSE ) != NULL );
                else
                    bDone = aFunc.HideComment( nCol, nRow );
                if (bDone)
                {
                    aNote.SetShown( bSet );
                    pDoc->SetNote( nCol, nRow, nTab, aNote );

                    pDocShell->SetDocumentModified();
                }
            }
        }
    }
}

//------------------------------------------------------------------------




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
