/*************************************************************************
 *
 *  $RCSfile: editsrc.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: nn $ $Date: 2001-07-31 17:57:55 $
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

#include "scitems.hxx"
#include <svx/unofored.hxx>

#include "editsrc.hxx"
#include "textuno.hxx"
#include "editutil.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "patattr.hxx"
#include "unoguard.hxx"

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
    //  pTextData is held by the ScHeaderFooterTextObj.
    //  Text range and cursor keep a reference to their parent text, so the text object is
    //  always alive and the TextData is valid as long as there are children.
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

//  each ScHeaderFooterEditSource object has its own ScHeaderFooterTextData

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
    delete GetTextData();   // not accessed in ScSharedHeaderFooterEditSource dtor
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
    //  pCellTextData is part of the ScCellTextObj.
    //  Text range and cursor keep a reference to their parent text, so the text object is
    //  always alive and the CellTextData is valid as long as there are children.
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

//  each ScCellEditSource object has its own ScCellTextData

ScCellEditSource::ScCellEditSource( ScDocShell* pDocSh, const ScAddress& rP ) :
    ScSharedCellEditSource( new ScCellTextData( pDocSh, rP ) )
{
}

ScCellEditSource::~ScCellEditSource()
{
    delete GetCellTextData();   // not accessed in ScSharedCellEditSource dtor
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

//------------------------------------------------------------------------

ScSimpleEditSource::ScSimpleEditSource( SvxTextForwarder* pForw ) :
    pForwarder( pForw )
{
    //  The same forwarder (and EditEngine) is shared by all children of the same Text object.
    //  Text range and cursor keep a reference to their parent text, so the text object is
    //  always alive and the forwarder is valid as long as there are children.
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
    //  nothing
}



