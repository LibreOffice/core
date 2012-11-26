/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "editsrc.hxx"

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/unofored.hxx>
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
#include "unoguard.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "postit.hxx"
#include "AccessibleText.hxx"

//------------------------------------------------------------------------

ScHeaderFooterChangedHint::ScHeaderFooterChangedHint(sal_uInt16 nP) :
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
                                                    sal_uInt16 nP ) :
    ScSharedHeaderFooterEditSource( new ScHeaderFooterTextData( *pContent, nP ) )
{
}

ScHeaderFooterEditSource::ScHeaderFooterEditSource( ScHeaderFooterContentObj& rContent,
                                                    sal_uInt16 nP ) :
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
    bDataValid( sal_False )
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

SdrObject* ScAnnotationEditSource::GetCaptionObj()
{
    ScPostIt* pNote = pDocShell->GetDocument()->GetNote( aCellPos );
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
        if ( ScPostIt* pNote = pDocShell->GetDocument()->GetNote( aCellPos ) )
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
            pObj->SetOutlinerParaObject( pOPO );
            pObj->ActionChanged();
        }

        //! Undo !!!

        aModificator.SetDocumentModified();

        // bDataValid wird bei SetDocumentModified zurueckgesetzt
    }
}

void ScAnnotationEditSource::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast< const ScUpdateRefHint* >(&rHint) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( dynamic_cast< const SfxSimpleHint* >(&rHint) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;                       // ungueltig geworden

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );     // EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
            bDataValid = sal_False;                     // Text muss neu geholt werden
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

