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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <bf_svx/unofored.hxx>

#include "editsrc.hxx"
#include "editutil.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "unoguard.hxx"
#include "AccessibleText.hxx"
namespace binfilter {

//------------------------------------------------------------------------

TYPEINIT1( ScHeaderFooterChangedHint, SfxHint );

ScHeaderFooterChangedHint::ScHeaderFooterChangedHint(USHORT nP) :
    nPart( nP )
{
}

ScHeaderFooterChangedHint::~ScHeaderFooterChangedHint()
{
}

//------------------------------------------------------------------------

ScSharedHeaderFooterEditSource::ScSharedHeaderFooterEditSource( ScHeaderFooterTextData* pData ) :
    pTextData( pData )
{
    //	pTextData is held by the ScHeaderFooterTextObj.
    //	Text range and cursor keep a reference to their parent text, so the text object is
    //	always alive and the TextData is valid as long as there are children.
}

ScSharedHeaderFooterEditSource::~ScSharedHeaderFooterEditSource()
{
}

SvxEditSource* ScSharedHeaderFooterEditSource::Clone() const
{
    return new ScSharedHeaderFooterEditSource( pTextData );
}

SvxTextForwarder* ScSharedHeaderFooterEditSource::GetTextForwarder()
{
    return pTextData->GetTextForwarder();
}

void ScSharedHeaderFooterEditSource::UpdateData()
{
    pTextData->UpdateData();
}

ScEditEngineDefaulter* ScSharedHeaderFooterEditSource::GetEditEngine()
{
    return pTextData->GetEditEngine();
}

//------------------------------------------------------------------------

//	each ScHeaderFooterEditSource object has its own ScHeaderFooterTextData

ScHeaderFooterEditSource::ScHeaderFooterEditSource( ScHeaderFooterContentObj* pContent,
                                                    USHORT nP ) :
    ScSharedHeaderFooterEditSource( new ScHeaderFooterTextData( *pContent, nP ) )
{
}

ScHeaderFooterEditSource::ScHeaderFooterEditSource( ScHeaderFooterContentObj& rContent,
                                                    USHORT nP ) :
    ScSharedHeaderFooterEditSource( new ScHeaderFooterTextData( rContent, nP ) )
{
}

ScHeaderFooterEditSource::~ScHeaderFooterEditSource()
{
    delete GetTextData();	// not accessed in ScSharedHeaderFooterEditSource dtor
}

SvxEditSource* ScHeaderFooterEditSource::Clone() const
{
    const ScHeaderFooterTextData* pData = GetTextData();
    return new ScHeaderFooterEditSource( pData->GetContentObj(), pData->GetPart() );
}

//------------------------------------------------------------------------

ScSharedCellEditSource::ScSharedCellEditSource( ScCellTextData* pData ) :
    pCellTextData( pData )
{
    //	pCellTextData is part of the ScCellTextObj.
    //	Text range and cursor keep a reference to their parent text, so the text object is
    //	always alive and the CellTextData is valid as long as there are children.
}

ScSharedCellEditSource::~ScSharedCellEditSource()
{
}

SvxEditSource* ScSharedCellEditSource::Clone() const
{
    return new ScSharedCellEditSource( pCellTextData );
}

SvxTextForwarder* ScSharedCellEditSource::GetTextForwarder()
{
    return pCellTextData->GetTextForwarder();
}

void ScSharedCellEditSource::UpdateData()
{
    pCellTextData->UpdateData();
}

void ScSharedCellEditSource::SetDoUpdateData(sal_Bool bValue)
{
    pCellTextData->SetDoUpdate(bValue);
}

sal_Bool ScSharedCellEditSource::IsDirty() const
{
    return pCellTextData->IsDirty();
}

ScEditEngineDefaulter* ScSharedCellEditSource::GetEditEngine()
{
    return pCellTextData->GetEditEngine();
}

//------------------------------------------------------------------------

//	each ScCellEditSource object has its own ScCellTextData

ScCellEditSource::ScCellEditSource( ScDocShell* pDocSh, const ScAddress& rP ) :
    ScSharedCellEditSource( new ScCellTextData( pDocSh, rP ) )
{
}

ScCellEditSource::~ScCellEditSource()
{
    delete GetCellTextData();	// not accessed in ScSharedCellEditSource dtor
}

SvxEditSource* ScCellEditSource::Clone() const
{
    const ScCellTextData* pData = GetCellTextData();
    return new ScCellEditSource( pData->GetDocShell(), pData->GetCellPos() );
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
    ScUnoGuard aGuard;		//	needed for EditEngine dtor

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
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScEditEngineDefaulter( pEnginePool, TRUE );
        }
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    if ( pDocShell )
    {
        ScPostIt aNote;
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );

        pEditEngine->SetText( aNote.GetText() );		// incl. Umbrueche
    }

    bDataValid = TRUE;
    return pForwarder;
}

void ScAnnotationEditSource::UpdateData()
{
    if ( pDocShell && pEditEngine )
    {
        String aNewText = pEditEngine->GetText( LINEEND_LF );	// im SetNoteText passiert ConvertLineEnd
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

        //!	Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;						// ungueltig geworden

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );		// EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
            bDataValid = FALSE;						// Text muss neu geholt werden
    }
}

//------------------------------------------------------------------------

ScSimpleEditSource::ScSimpleEditSource( SvxTextForwarder* pForw ) :
    pForwarder( pForw )
{
    //	The same forwarder (and EditEngine) is shared by all children of the same Text object.
    //	Text range and cursor keep a reference to their parent text, so the text object is
    //	always alive and the forwarder is valid as long as there are children.
}

ScSimpleEditSource::~ScSimpleEditSource()
{
}

SvxEditSource* ScSimpleEditSource::Clone() const
{
    return new ScSimpleEditSource( pForwarder );
}

SvxTextForwarder* ScSimpleEditSource::GetTextForwarder()
{
    return pForwarder;
}

void ScSimpleEditSource::UpdateData()
{
    //	nothing
}

//------------------------------------------------------------------------

ScAccessibilityEditSource::ScAccessibilityEditSource( ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData )
    : mpAccessibleTextData(pAccessibleCellTextData)
{
}

ScAccessibilityEditSource::~ScAccessibilityEditSource()
{
}

SvxEditSource* ScAccessibilityEditSource::Clone() const
{
    return new ScAccessibilityEditSource(::std::auto_ptr < ScAccessibleTextData > (mpAccessibleTextData->Clone()));
}

SvxTextForwarder* ScAccessibilityEditSource::GetTextForwarder()
{
    return mpAccessibleTextData->GetTextForwarder();
}

SvxViewForwarder* ScAccessibilityEditSource::GetViewForwarder()
{
    return mpAccessibleTextData->GetViewForwarder();
}

SvxEditViewForwarder* ScAccessibilityEditSource::GetEditViewForwarder( sal_Bool bCreate )
{
    return mpAccessibleTextData->GetEditViewForwarder(bCreate);
}

void ScAccessibilityEditSource::UpdateData()
{
    mpAccessibleTextData->UpdateData();
}

SfxBroadcaster& ScAccessibilityEditSource::GetBroadcaster() const
{
    return mpAccessibleTextData->GetBroadcaster();
}

}
