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
#include "precompiled_sd.hxx"

#ifndef _SVDLAYER
#include <svx/svdlayer.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdview.hxx>
#include "strings.hrc"
#include "glob.hxx"
#include "glob.hrc"         // STR_BCKGRND, STR_BCKGRNDOBJ
#include "app.hrc"          // SID_SWITCHPAGE
#include "unmodpg.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

ModifyPageUndoAction::ModifyPageUndoAction(
    SdDrawDocument* pTheDoc,
    SdPage* pThePage,
    String aTheNewName,
    AutoLayout  eTheNewAutoLayout,
    bool bTheNewBckgrndVisible,
    bool bTheNewBckgrndObjsVisible)
:   SdUndoAction(pTheDoc)
{
    DBG_ASSERT(pThePage, "Undo ohne Seite ???");

    mpPage                  = pThePage;
    maNewName               = aTheNewName;
    meNewAutoLayout         = eTheNewAutoLayout;
    mbNewBckgrndVisible     = bTheNewBckgrndVisible;
    mbNewBckgrndObjsVisible = bTheNewBckgrndObjsVisible;

    meOldAutoLayout         = mpPage->GetAutoLayout();

    if (!mpPage->IsMasterPage())
    {
        maOldName = mpPage->GetName();
        SdrLayerAdmin& rLayerAdmin = mpDoc->GetModelLayerAdmin();
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), false);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), false);
        SetOfByte aVisibleLayers = mpPage->TRG_GetMasterPageVisibleLayers();

        mbOldBckgrndVisible = aVisibleLayers.IsSet(aBckgrnd);
        mbOldBckgrndObjsVisible = aVisibleLayers.IsSet(aBckgrndObj);
    }

    maComment = String(SdResId(STR_UNDO_MODIFY_PAGE));
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void ModifyPageUndoAction::Undo()
{
    // #94637# invalidate Selection, there could be objects deleted in tis UNDO
    // which are no longer allowed to be selected then.
    const ::std::set< SdrView* > aAllSdrViews(mpPage->getSdrModelFromSdrPage().getSdrViews());

    for(::std::set< SdrView* >::const_iterator aLoopViews(aAllSdrViews.begin());
        aLoopViews != aAllSdrViews.end(); aLoopViews++)
    {
        (*aLoopViews)->UnmarkAll();
    }

    mpPage->SetAutoLayout( meOldAutoLayout );

    if (!mpPage->IsMasterPage())
    {
        if (mpPage->GetName() != maOldName)
        {
            mpPage->SetName(maOldName);

            if (mpPage->GetPageKind() == PK_STANDARD)
            {
                SdPage* pNotesPage = (SdPage*)mpDoc->GetPage(mpPage->GetPageNumber() + 1);
                pNotesPage->SetName(maOldName);
            }
        }

        SdrLayerAdmin& rLayerAdmin = mpDoc->GetModelLayerAdmin();
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), false);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), false);
        SetOfByte aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, mbOldBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, mbOldBckgrndObjsVisible);
        mpPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }

    // Redisplay
    SfxViewFrame::Current()->GetDispatcher()->Execute(
        SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void ModifyPageUndoAction::Redo()
{
    // #94637# invalidate Selection, there could be objects deleted in tis UNDO
    // which are no longer allowed to be selected then.
    const ::std::set< SdrView* > aAllSdrViews(mpPage->getSdrModelFromSdrPage().getSdrViews());

    for(::std::set< SdrView* >::const_iterator aLoopViews(aAllSdrViews.begin());
        aLoopViews != aAllSdrViews.end(); aLoopViews++)
    {
        (*aLoopViews)->UnmarkAll();
    }

    mpPage->meAutoLayout = meNewAutoLayout;

    if (!mpPage->IsMasterPage())
    {
        if (mpPage->GetName() != maNewName)
        {
            mpPage->SetName(maNewName);

            if (mpPage->GetPageKind() == PK_STANDARD)
            {
                SdPage* pNotesPage = (SdPage*)mpDoc->GetPage(mpPage->GetPageNumber() + 1);
                pNotesPage->SetName(maNewName);
            }
        }

        SdrLayerAdmin& rLayerAdmin = mpDoc->GetModelLayerAdmin();
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), false);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), false);
        SetOfByte aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, mbNewBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, mbNewBckgrndObjsVisible);
        mpPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }

    // Redisplay
    SfxViewFrame::Current()->GetDispatcher()->Execute(
        SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

ModifyPageUndoAction::~ModifyPageUndoAction()
{
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String ModifyPageUndoAction::GetComment() const
{
    return maComment;
}

// --------------------------------------------------------------------

RenameLayoutTemplateUndoAction::RenameLayoutTemplateUndoAction( SdDrawDocument* pDocument, const String& rOldLayoutName, const String& rNewLayoutName )
: SdUndoAction(pDocument)
, maOldName( rOldLayoutName )
, maNewName( rNewLayoutName )
, maComment(SdResId(STR_TITLE_RENAMESLIDE))
{
    sal_uInt16 nPos = maOldName.SearchAscii( SD_LT_SEPARATOR );
    if( nPos != (sal_uInt16)-1 )
        maOldName.Erase(nPos);
}

void RenameLayoutTemplateUndoAction::Undo()
{
    String aLayoutName( maNewName );
    aLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aLayoutName.Append( String(SdResId(STR_LAYOUT_OUTLINE))) ;

    mpDoc->RenameLayoutTemplate( aLayoutName, maOldName );
}

void RenameLayoutTemplateUndoAction::Redo()
{
    String aLayoutName( maOldName );
    aLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aLayoutName.Append( String(SdResId(STR_LAYOUT_OUTLINE))) ;

    mpDoc->RenameLayoutTemplate( aLayoutName, maNewName );
}

String RenameLayoutTemplateUndoAction::GetComment() const
{
    return  maComment;
}

