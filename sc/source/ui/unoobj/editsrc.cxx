/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <utility>

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

ScAnnotationEditSource::ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( nullptr ),
    pForwarder( nullptr ),
    bDataValid( false )
{
    if (pDocShell)
        pDocShell->GetDocument().AddUnoObject(*this);
}

ScAnnotationEditSource::~ScAnnotationEditSource()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);

    delete pForwarder;
    delete pEditEngine;
}

SvxEditSource* ScAnnotationEditSource::Clone() const
{
    return new ScAnnotationEditSource( pDocShell, aCellPos );
}

SdrObject* ScAnnotationEditSource::GetCaptionObj()
{
    ScPostIt* pNote = pDocShell->GetDocument().GetNote(aCellPos);
    return pNote ? pNote->GetOrCreateCaption( aCellPos ) : nullptr;
}

SvxTextForwarder* ScAnnotationEditSource::GetTextForwarder()
{
    if (!pEditEngine)
    {
        // Notizen haben keine Felder
        if ( pDocShell )
        {
            pEditEngine = new ScNoteEditEngine( pDocShell->GetDocument().GetNoteEngine() );
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScEditEngineDefaulter( pEnginePool, true );
        }
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    if ( pDocShell )
        if ( ScPostIt* pNote = pDocShell->GetDocument().GetNote(aCellPos) )
            if ( const EditTextObject* pEditObj = pNote->GetEditTextObject() )
                pEditEngine->SetText( *pEditObj );      // incl. Umbrueche

    bDataValid = true;
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
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! Ref-Update
    }
    else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) )
    {
        sal_uLong nId = static_cast<const SfxSimpleHint&>(rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = nullptr;                       // ungueltig geworden

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );     // EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
            bDataValid = false;                     // Text muss neu geholt werden
    }
}

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

ScAccessibilityEditSource::ScAccessibilityEditSource( ::std::unique_ptr < ScAccessibleTextData > && pAccessibleCellTextData )
    : mpAccessibleTextData(std::move(pAccessibleCellTextData))
{
}

ScAccessibilityEditSource::~ScAccessibilityEditSource()
{
}

SvxEditSource* ScAccessibilityEditSource::Clone() const
{
    return new ScAccessibilityEditSource(::std::unique_ptr < ScAccessibleTextData > (mpAccessibleTextData->Clone()));
}

SvxTextForwarder* ScAccessibilityEditSource::GetTextForwarder()
{
    return mpAccessibleTextData->GetTextForwarder();
}

SvxViewForwarder* ScAccessibilityEditSource::GetViewForwarder()
{
    return mpAccessibleTextData->GetViewForwarder();
}

SvxEditViewForwarder* ScAccessibilityEditSource::GetEditViewForwarder( bool bCreate )
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
