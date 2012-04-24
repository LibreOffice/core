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


#include "editsrc.hxx"

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/unofored.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outliner.hxx>
#include "textuno.hxx"
#include "editutil.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "patattr.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "postit.hxx"
#include "AccessibleText.hxx"

ScHeaderFooterEditSource::ScHeaderFooterEditSource(ScHeaderFooterTextData& rData) :
    mrTextData(rData) {}

ScHeaderFooterEditSource::~ScHeaderFooterEditSource() {}

ScEditEngineDefaulter* ScHeaderFooterEditSource::GetEditEngine()
{
    return mrTextData.GetEditEngine();
}

SvxEditSource* ScHeaderFooterEditSource::Clone() const
{
    return new ScHeaderFooterEditSource(mrTextData);
}

SvxTextForwarder* ScHeaderFooterEditSource::GetTextForwarder()
{
    return mrTextData.GetTextForwarder();
}

void ScHeaderFooterEditSource::UpdateData()
{
    mrTextData.UpdateData();
}

//------------------------------------------------------------------------

ScCellEditSource::ScCellEditSource(ScDocShell* pDocSh, const ScAddress& rP) :
    pCellTextData(new ScCellTextData(pDocSh, rP)) {}

ScCellEditSource::~ScCellEditSource()
{
    delete pCellTextData;
}

SvxEditSource* ScCellEditSource::Clone() const
{
    return new ScCellEditSource(pCellTextData->GetDocShell(), pCellTextData->GetCellPos());
}

SvxTextForwarder* ScCellEditSource::GetTextForwarder()
{
    return pCellTextData->GetTextForwarder();
}

void ScCellEditSource::UpdateData()
{
    pCellTextData->UpdateData();
}

void ScCellEditSource::SetDoUpdateData(bool bValue)
{
    pCellTextData->SetDoUpdate(bValue);
}

bool ScCellEditSource::IsDirty() const
{
    return pCellTextData->IsDirty();
}

ScEditEngineDefaulter* ScCellEditSource::GetEditEngine()
{
    return pCellTextData->GetEditEngine();
}

//------------------------------------------------------------------------

ScAnnotationEditSource::ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    bDataValid( false )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAnnotationEditSource::~ScAnnotationEditSource()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pForwarder;
    delete pEditEngine;
}

SvxEditSource* ScAnnotationEditSource::Clone() const
{
    return new ScAnnotationEditSource( pDocShell, aCellPos );
}

SdrObject* ScAnnotationEditSource::GetCaptionObj()
{
    ScPostIt* pNote = pDocShell->GetDocument()->GetNotes( aCellPos.Tab() )->findByAddress(aCellPos);
    return pNote ? pNote->GetOrCreateCaption( aCellPos ) : 0;
}

SvxTextForwarder* ScAnnotationEditSource::GetTextForwarder()
{
    if (!pEditEngine)
    {
        // Notizen haben keine Felder
        if ( pDocShell )
        {
            pEditEngine = new ScNoteEditEngine( pDocShell->GetDocument()->GetNoteEngine() );
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScEditEngineDefaulter( pEnginePool, sal_True );
        }
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    if ( pDocShell )
        if ( ScPostIt* pNote = pDocShell->GetDocument()->GetNotes( aCellPos.Tab() )->findByAddress(aCellPos) )
            if ( const EditTextObject* pEditObj = pNote->GetEditTextObject() )
                pEditEngine->SetText( *pEditObj );      // incl. Umbrueche

    bDataValid = sal_True;
    return pForwarder;
}

void ScAnnotationEditSource::UpdateData()
{
    if ( pDocShell && pEditEngine )
    {
        ScDocShellModificator aModificator( *pDocShell );

        if( SdrObject* pObj = GetCaptionObj() )
        {
            EditTextObject* pEditObj = pEditEngine->CreateTextObject();
            OutlinerParaObject* pOPO = new OutlinerParaObject( *pEditObj );
            delete pEditObj;
            pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
            pObj->NbcSetOutlinerParaObject( pOPO );
            pObj->ActionChanged();
        }

        //! Undo !!!

        aModificator.SetDocumentModified();

        // bDataValid wird bei SetDocumentModified zurueckgesetzt
    }
}

void ScAnnotationEditSource::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;                       // ungueltig geworden

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );     // EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
            bDataValid = false;                     // Text muss neu geholt werden
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

//------------------------------------------------------------------------

SAL_WNODEPRECATED_DECLARATIONS_PUSH
ScAccessibilityEditSource::ScAccessibilityEditSource( ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData )
    : mpAccessibleTextData(pAccessibleCellTextData)
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP

ScAccessibilityEditSource::~ScAccessibilityEditSource()
{
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
SvxEditSource* ScAccessibilityEditSource::Clone() const
{
    return new ScAccessibilityEditSource(::std::auto_ptr < ScAccessibleTextData > (mpAccessibleTextData->Clone()));
}
SAL_WNODEPRECATED_DECLARATIONS_POP

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
