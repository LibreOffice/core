/*************************************************************************
 *
 *  $RCSfile: editsrc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-21 17:47:44 $
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

#pragma hdrstop

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#include "scitems.hxx"
#include <tools/debug.hxx>
#include <svx/editeng.hxx>
#include <svx/unofored.hxx>

#include "textuno.hxx"
#include "editsrc.hxx"
#include "editutil.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "cell.hxx"
#include "hints.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "unoguard.hxx"

//------------------------------------------------------------------------

ScHeaderFooterEditSource::ScHeaderFooterEditSource( ScHeaderFooterContentObj* pContent,
                                                        USHORT nP ) :
    pContentObj( pContent ),
    nPart( nP ),
    pEditEngine( NULL ),
    pForwarder( NULL )

{
    if (pContentObj)                // pContentObj kann 0 sein bei getReflection-Krempel
        pContentObj->acquire();     // darf nicht wegkommen

    //! bDataValid oder so...
}

ScHeaderFooterEditSource::~ScHeaderFooterEditSource()
{
    ScUnoGuard aGuard;      //  needed for EditEngine dtor

    delete pForwarder;
    delete pEditEngine;

    if (pContentObj)
        pContentObj->release();
}

SvxEditSource* ScHeaderFooterEditSource::Clone() const
{
    return new ScHeaderFooterEditSource( pContentObj, nPart );
}

SvxTextForwarder* ScHeaderFooterEditSource::GetTextForwarder()
{
    if (!pEditEngine)
    {
        ScHeaderEditEngine* pHdrEngine = new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE );

        pHdrEngine->EnableUndo( FALSE );
        pHdrEngine->SetRefMapMode( MAP_TWIP );

        //  default font must be set, independently of document
        //  -> use global pool from module

        Font aDefFont;
        ((const ScPatternAttr&)SC_MOD()->GetPool().GetDefaultItem(ATTR_PATTERN)).GetFont(aDefFont);
        SfxItemSet aDefaults( pHdrEngine->GetEmptyItemSet() );
        EditEngine::SetFontInfoInItemSet( aDefaults, aDefFont );
        pHdrEngine->SetDefaults( aDefaults );

        ScHeaderFieldData aData;
        ScHeaderFooterTextObj::FillDummyFieldData( aData );
        pHdrEngine->SetData( aData );

        pEditEngine = pHdrEngine;
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (pContentObj)
    {
        const EditTextObject* pData;
        if (nPart == SC_HDFT_LEFT)
            pData = pContentObj->GetLeftEditObject();
        else if (nPart == SC_HDFT_CENTER)
            pData = pContentObj->GetCenterEditObject();
        else
            pData = pContentObj->GetRightEditObject();

        if (pData)
            pEditEngine->SetText(*pData);
    }

    return pForwarder;
}

void ScHeaderFooterEditSource::UpdateData()
{
    if ( pContentObj && pEditEngine )
        pContentObj->UpdateText( nPart, *pEditEngine );
}

void ScHeaderFooterEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  spaeter...
}

//------------------------------------------------------------------------

ScCellEditSource::ScCellEditSource(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    bDataValid( FALSE ),
    bInUpdate( FALSE )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellEditSource::~ScCellEditSource()
{
    ScUnoGuard aGuard;      //  needed for EditEngine dtor

    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pForwarder;
    delete pEditEngine;
}

SvxEditSource* ScCellEditSource::Clone() const
{
    return new ScCellEditSource( pDocShell, aCellPos );
}

SvxTextForwarder* ScCellEditSource::GetTextForwarder()
{
    if (!pEditEngine)
    {
        if ( pDocShell )
        {
            const ScDocument* pDoc = pDocShell->GetDocument();
            pEditEngine = new ScFieldEditEngine( pDoc->GetEnginePool(),
                pDoc->GetEditPool(), FALSE );
        }
        else
            pEditEngine = new ScFieldEditEngine( EditEngine::CreatePool(),
                NULL, TRUE );
#if SUPD > 600
        //  currently, GetPortions doesn't work if UpdateMode is FALSE,
        //  this will be fixed (in EditEngine) by src600
//      pEditEngine->SetUpdateMode( FALSE );
#endif
        pEditEngine->EnableUndo( FALSE );
        pEditEngine->SetRefMapMode( MAP_100TH_MM );
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    BOOL bEditCell = FALSE;
    String aText;

    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        const ScBaseCell* pCell = pDoc->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_EDIT )
        {
            pEditEngine->SetText( *((const ScEditCell*)pCell)->GetData() );
            bEditCell = TRUE;
        }
        else
            pDoc->GetInputString( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aText );

        SfxItemSet aDefaults( pEditEngine->GetEmptyItemSet() );
        const ScPatternAttr* pPattern =
                pDoc->GetPattern( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab() );
        pPattern->FillEditItemSet( &aDefaults );
        pPattern->FillEditParaItems( &aDefaults );  // auch Ausrichtung etc. auslesbar
        pEditEngine->SetDefaults( aDefaults );
    }

    if (!bEditCell)
        pEditEngine->SetText( aText );

    bDataValid = TRUE;
    return pForwarder;
}

void ScCellEditSource::UpdateData()
{
    if ( pDocShell && pEditEngine )
    {
        //  beim eigenen Update darf bDataValid nicht zurueckgesetzt werden,
        //  damit z.B. Attribute hinter dem Text nicht verloren gehen
        //  (werden in die Zelle nicht uebernommen)

        bInUpdate = TRUE;   // damit wird bDataValid nicht zurueckgesetzt

        ScDocFunc aFunc(*pDocShell);
        aFunc.PutData( aCellPos, *pEditEngine, FALSE, TRUE );   // immer Text

        bInUpdate = FALSE;
    }
}

void ScCellEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;                       // ungueltig geworden

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );     // EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            if (!bInUpdate)                         // eigene Updates zaehlen nicht
                bDataValid = FALSE;                 // Text muss neu geholt werden
        }
    }
}

//------------------------------------------------------------------------

ScAnnotationEditSource::ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    bDataValid( FALSE )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAnnotationEditSource::~ScAnnotationEditSource()
{
    ScUnoGuard aGuard;      //  needed for EditEngine dtor

    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pForwarder;
    delete pEditEngine;
}

SvxEditSource* ScAnnotationEditSource::Clone() const
{
    return new ScAnnotationEditSource( pDocShell, aCellPos );
}

SvxTextForwarder* ScAnnotationEditSource::GetTextForwarder()
{
    if (!pEditEngine)
    {
        // Notizen haben keine Felder
        if ( pDocShell )
            pEditEngine = new ScEditEngineDefaulter(
                pDocShell->GetDocument()->GetEnginePool(), FALSE );
        else
            pEditEngine = new ScEditEngineDefaulter(
                EditEngine::CreatePool(), TRUE );
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    if ( pDocShell )
    {
        ScPostIt aNote;
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );

        pEditEngine->SetText( aNote.GetText() );        // incl. Umbrueche
    }

    bDataValid = TRUE;
    return pForwarder;
}

void ScAnnotationEditSource::UpdateData()
{
    if ( pDocShell && pEditEngine )
    {
        String aNewText = pEditEngine->GetText( LINEEND_LF );   // im SetNoteText passiert ConvertLineEnd
        ScDocFunc aFunc(*pDocShell);
        aFunc.SetNoteText( aCellPos, aNewText, TRUE );

        // bDataValid wird bei SetDocumentModified zurueckgesetzt
    }
}

void ScAnnotationEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;                       // ungueltig geworden

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );     // EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
            bDataValid = FALSE;                     // Text muss neu geholt werden
    }
}




