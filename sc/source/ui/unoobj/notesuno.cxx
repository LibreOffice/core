/*************************************************************************
 *
 *  $RCSfile: notesuno.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-14 15:29:56 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#pragma hdrstop

#include "rangelst.hxx"
#include <svx/unotext.hxx>

#include "notesuno.hxx"
#include "textuno.hxx"
#include "cellsuno.hxx"     // getParent
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "miscuno.hxx"

// setVisible:
#include <svx/svdundo.hxx>
#include "drwlayer.hxx"
#include "detfunc.hxx"
#include "undocell.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  keine Properties fuer Text in Notizen
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

    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

SvxUnoText& ScAnnotationObj::GetUnoText()
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

void SAL_CALL ScAnnotationObj::setParent( const uno::Reference<uno::XInterface>& Parent )
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
    ScPostIt aNote;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
    }
    return aNote.GetText();
}

void SAL_CALL ScAnnotationObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
    ScPostIt aNote;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
    }
    return aNote.GetAuthor();
}

rtl::OUString SAL_CALL ScAnnotationObj::getDate() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    if ( pDocShell )
    {
        //! Funktion an docfunc oder so

        BOOL bSet = bIsVisible ? TRUE : FALSE;
        ScDocument* pDoc = pDocShell->GetDocument();
        BOOL bUndo(pDoc->IsUndoEnabled());
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

                if (bUndo)
                    pModel->BeginCalcUndo();
                ScDetectiveFunc aFunc( pDoc,nTab );
                if ( bSet )
                    bDone = ( aFunc.ShowComment( nCol, nRow, FALSE ) != NULL );
                else
                    bDone = aFunc.HideComment( nCol, nRow );
                SdrUndoGroup* pUndo = NULL;
                if (bUndo)
                    pUndo = pModel->GetCalcUndo();
                if (bDone)
                {
                    aNote.SetShown( bSet );
                    pDoc->SetNote( nCol, nRow, nTab, aNote );
                    if (pUndo)
                        pDocShell->GetUndoManager()->AddUndoAction(
                            new ScUndoNote( pDocShell, bSet, aCellPos, pUndo ) );

                    pDocShell->SetDocumentModified();
                }
                else
                    delete pUndo;
            }
        }
    }
}

//------------------------------------------------------------------------




