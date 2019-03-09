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

#include <DrawDocShell.hxx>

#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>

#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>
#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svditer.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/eeitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/printer.hxx>
#include <svx/drawitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <comphelper/classids.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/visitem.hxx>

#include <app.hrc>
#include <sdmod.hxx>
#include <View.hxx>
#include <drawdoc.hxx>

#include <fupoor.hxx>
#include <fusearch.hxx>
#include <ViewShell.hxx>
#include <unomodel.hxx>
#include <undo/undomanager.hxx>
#include <undo/undofactory.hxx>
#include <OutlineView.hxx>
#include <ViewShellBase.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>

using namespace sd;
#define ShellClass_DrawDocShell
#include <sdslots.hxx>

SFX_IMPL_SUPERCLASS_INTERFACE(DrawDocShell, SfxObjectShell);

void DrawDocShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterChildWindow(SvxSearchDialogWrapper::GetChildWindowId());
}

namespace sd {

/**
 * slotmaps and definitions of SFX
 */

SFX_IMPL_OBJECTFACTORY(
    DrawDocShell,
    SvGlobalName(SO3_SIMPRESS_CLASSID),
    "simpress" )

void DrawDocShell::Construct( bool bClipboard )
{
    mbInDestruction = false;
    SetSlotFilter();     // resets the filter

    mbOwnDocument = mpDoc == nullptr;
    if( mbOwnDocument )
        mpDoc = new SdDrawDocument(meDocType, this);

    // The document has been created so we can call UpdateRefDevice() to set
    // the document's ref device.
    UpdateRefDevice();

    SetBaseModel( new SdXImpressDocument( this, bClipboard ) );
    SetPool( &mpDoc->GetItemPool() );
    std::unique_ptr<sd::UndoManager> pUndoManager(new sd::UndoManager);
    pUndoManager->SetDocShell(this);
    mpUndoManager = std::move(pUndoManager);

    if (!utl::ConfigManager::IsFuzzing()
        && officecfg::Office::Common::Undo::Steps::get() < 1)
    {
        mpUndoManager->EnableUndo(false); // tdf#108863 disable if 0 steps
    }
    mpDoc->SetSdrUndoManager( mpUndoManager.get() );
    mpDoc->SetSdrUndoFactory( new sd::UndoFactory );
    UpdateTablePointers();
    SetStyleFamily(SfxStyleFamily::Pseudo);
}

DrawDocShell::DrawDocShell(SfxObjectCreateMode eMode,
                               bool bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell( eMode == SfxObjectCreateMode::INTERNAL ?  SfxObjectCreateMode::EMBEDDED : eMode),
    mpDoc(nullptr),
    mpPrinter(nullptr),
    mpViewShell(nullptr),
    meDocType(eDocumentType),
    mbSdDataObj(bDataObject),
    mbOwnPrinter(false)
{
    Construct( eMode == SfxObjectCreateMode::INTERNAL );
}

DrawDocShell::DrawDocShell( SfxModelFlags nModelCreationFlags, bool bDataObject, DocumentType eDocumentType ) :
    SfxObjectShell( nModelCreationFlags ),
    mpDoc(nullptr),
    mpPrinter(nullptr),
    mpViewShell(nullptr),
    meDocType(eDocumentType),
    mbSdDataObj(bDataObject),
    mbOwnPrinter(false)
{
    Construct( false );
}

DrawDocShell::DrawDocShell(SdDrawDocument* pDoc, SfxObjectCreateMode eMode,
                               bool bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell(eMode == SfxObjectCreateMode::INTERNAL ?  SfxObjectCreateMode::EMBEDDED : eMode),
    mpDoc(pDoc),
    mpPrinter(nullptr),
    mpViewShell(nullptr),
    meDocType(eDocumentType),
    mbSdDataObj(bDataObject),
    mbOwnPrinter(false)
{
    Construct( eMode == SfxObjectCreateMode::INTERNAL );
}

DrawDocShell::~DrawDocShell()
{
    // Tell all listeners that the doc shell is about to be
    // destroyed.  This has been introduced for the PreviewRenderer to
    // free its view (that uses the item poll of the doc shell) but
    // may be useful in other places as well.
    Broadcast(SfxHint(SfxHintId::Dying));

    mbInDestruction = true;

    SetDocShellFunction(nullptr);

    mpFontList.reset();

    if( mpDoc )
        mpDoc->SetSdrUndoManager( nullptr );
    mpUndoManager.reset();

    if (mbOwnPrinter)
        mpPrinter.disposeAndClear();

    if( mbOwnDocument )
        delete mpDoc;

    // that the navigator get informed about the disappearance of the document
    SfxBoolItem     aItem(SID_NAVIGATOR_INIT, true);
    SfxViewFrame*   pFrame = mpViewShell ? mpViewShell->GetFrame() : GetFrame();

    if( !pFrame )
        pFrame = SfxViewFrame::GetFirst( this );

    if( pFrame )
    {
        pFrame->GetDispatcher()->ExecuteList(
            SID_NAVIGATOR_INIT, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aItem });
    }
}

void DrawDocShell::GetState(SfxItemSet &rSet)
{

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;

        switch ( nSlotId )
        {
            case SID_ATTR_CHAR_FONTLIST:
                rSet.Put( SvxFontListItem( mpFontList.get(), nSlotId ) );
            break;

            case SID_SEARCH_ITEM:
            {
                rSet.Put( *SD_MOD()->GetSearchItem() );
            }
            break;

            case SID_CLOSEDOC:
                GetSlotState(SID_CLOSEDOC, SfxObjectShell::GetInterface(), &rSet);
            break;

            case SID_SEARCH_OPTIONS:
            {
                SearchOptionFlags nOpt = SearchOptionFlags::SEARCH |
                              SearchOptionFlags::WHOLE_WORDS |
                              SearchOptionFlags::BACKWARDS   |
                              SearchOptionFlags::REG_EXP     |
                              SearchOptionFlags::EXACT       |
                              SearchOptionFlags::SIMILARITY  |
                              SearchOptionFlags::SELECTION;

                if (!IsReadOnly())
                {
                    nOpt |= SearchOptionFlags::REPLACE;
                    nOpt |= SearchOptionFlags::REPLACE_ALL;
                }

                rSet.Put(SfxUInt16Item(nWhich, static_cast<sal_uInt16>(nOpt)));
            }
            break;

            case SID_VERSION:
            {
                GetSlotState( SID_VERSION, SfxObjectShell::GetInterface(), &rSet );
            }
            break;

            case SID_CHINESE_CONVERSION:
            case SID_HANGUL_HANJA_CONVERSION:
            {
                rSet.Put(SfxVisibilityItem(nWhich, SvtCJKOptions().IsAnyEnabled()));
            }
            break;
            case SID_LANGUAGE_STATUS:
            {
                SdrObject* pObj = nullptr;
                bool bLanguageFound = false;
                OutlinerParaObject* pParaObj = nullptr;
                LanguageType eLanguage( LANGUAGE_DONTKNOW );
                sal_uInt16 nCount = mpDoc->GetPageCount();
                for ( sal_uInt16 itPage = 0; itPage < nCount && !bLanguageFound; itPage++ )
                {
                    SdrObjListIter aListIter(mpDoc->GetPage(itPage), SdrIterMode::DeepWithGroups);
                    while ( aListIter.IsMore() && !bLanguageFound )
                    {
                        pObj = aListIter.Next();
                        if ( pObj )
                        {
                            pParaObj = pObj->GetOutlinerParaObject();
                            if ( pParaObj )
                            {
                                SdrOutliner aOutliner(&mpDoc->GetPool(), OutlinerMode::TextObject);
                                aOutliner.SetText(*pParaObj);
                                eLanguage = aOutliner.GetLanguage(0, 0);
                                bLanguageFound = eLanguage != LANGUAGE_DONTKNOW;
                            }
                        }
                    }
                }

                if ( eLanguage == LANGUAGE_DONTKNOW )
                {
                    eLanguage = mpDoc->GetLanguage( EE_CHAR_LANGUAGE );
                }

                rSet.Put(SfxStringItem(nWhich, SvtLanguageTable::GetLanguageString(eLanguage)));
            }
            break;

            case SID_NOTEBOOKBAR:
            {
                if (mpViewShell)
                {
                    bool bImpress = mpDoc->GetDocumentType() == DocumentType::Impress;
                    bool bVisible = false;
                    if(bImpress)
                    {
                        bVisible = sfx2::SfxNotebookBar::StateMethod(mpViewShell->GetFrame()->GetBindings(),
                                                                      "modules/simpress/ui/");
                    }
                    else
                    {
                        bVisible = sfx2::SfxNotebookBar::StateMethod(mpViewShell->GetFrame()->GetBindings(),
                                                                      "modules/sdraw/ui/");
                    }
                    rSet.Put( SfxBoolItem( SID_NOTEBOOKBAR, bVisible ) );
                }
            }
            break;

            default:
            break;
        }
        nWhich = aIter.NextWhich();
    }

    SfxViewFrame* pFrame = SfxViewFrame::Current();

    if (pFrame)
    {
        if (rSet.GetItemState(SID_RELOAD) != SfxItemState::UNKNOWN)
        {
            pFrame->GetSlotState(SID_RELOAD,
                                 pFrame->GetInterface(), &rSet);
        }
    }
}

void DrawDocShell::Activate( bool bMDI)
{
    if (bMDI)
    {
        ApplySlotFilter();
        mpDoc->StartOnlineSpelling();
    }
}

void DrawDocShell::Deactivate( bool )
{
}

SfxUndoManager* DrawDocShell::GetUndoManager()
{
    return mpUndoManager.get();
}

void DrawDocShell::UpdateTablePointers()
{
    PutItem( SvxColorListItem( mpDoc->GetColorList(), SID_COLOR_TABLE ) );
    PutItem( SvxGradientListItem( mpDoc->GetGradientList(), SID_GRADIENT_LIST ) );
    PutItem( SvxHatchListItem( mpDoc->GetHatchList(), SID_HATCH_LIST ) );
    PutItem( SvxBitmapListItem( mpDoc->GetBitmapList(), SID_BITMAP_LIST ) );
    PutItem( SvxPatternListItem( mpDoc->GetPatternList(), SID_PATTERN_LIST ) );
    PutItem( SvxDashListItem( mpDoc->GetDashList(), SID_DASH_LIST ) );
    PutItem( SvxLineEndListItem( mpDoc->GetLineEndList(), SID_LINEEND_LIST ) );

    UpdateFontList();
}

void DrawDocShell::CancelSearching()
{
    if( dynamic_cast<FuSearch*>( mxDocShellFunction.get() ) )
    {
        SetDocShellFunction(nullptr);
    }
}

/**
 * apply configured slot filters
 */
void DrawDocShell::ApplySlotFilter() const
{
    SfxViewShell* pTestViewShell = SfxViewShell::GetFirst();

    while( pTestViewShell )
    {
        if( pTestViewShell->GetObjectShell()
            == this
            && pTestViewShell->GetViewFrame()
            && pTestViewShell->GetViewFrame()->GetDispatcher() )
        {
            SfxDispatcher* pDispatcher = pTestViewShell->GetViewFrame()->GetDispatcher();

            if( !mpFilterSIDs.empty() )
                pDispatcher->SetSlotFilter( mbFilterEnable ? SfxSlotFilterState::ENABLED : SfxSlotFilterState::DISABLED, mpFilterSIDs );
            else
                pDispatcher->SetSlotFilter();

            if( pDispatcher->GetBindings() )
                pDispatcher->GetBindings()->InvalidateAll( true );
        }

        pTestViewShell = SfxViewShell::GetNext( *pTestViewShell );
    }
}

void DrawDocShell::SetModified( bool bSet /* = true */ )
{
    SfxObjectShell::SetModified( bSet );

    // change model state, too
    // only set the changed state if modification is enabled
    if( IsEnableSetModified() )
    {
        if ( mpDoc )
            mpDoc->NbcSetChanged( bSet );

        Broadcast( SfxHint( SfxHintId::DocChanged ) );
    }
}

/**
 * Callback for ExecuteSpellPopup()
 */
// ExecuteSpellPopup now handled by DrawDocShell. This is necessary
// to get hands on the outliner and the text object.
IMPL_LINK(DrawDocShell, OnlineSpellCallback, SpellCallbackInfo&, rInfo, void)
{
    SdrObject* pObj = nullptr;
    SdrOutliner* pOutl = nullptr;

    if(GetViewShell())
    {
        pOutl = GetViewShell()->GetView()->GetTextEditOutliner();
        pObj = GetViewShell()->GetView()->GetTextEditObject();
    }

    mpDoc->ImpOnlineSpellCallback(&rInfo, pObj, pOutl);
}

void DrawDocShell::ClearUndoBuffer()
{
    // clear possible undo buffers of outliners
    SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, false);
    while(pSfxViewFrame)
    {
        ViewShellBase* pViewShellBase = dynamic_cast< ViewShellBase* >( pSfxViewFrame->GetViewShell() );
        if( pViewShellBase )
        {
            std::shared_ptr<ViewShell> pViewSh( pViewShellBase->GetMainViewShell() );
            if( pViewSh.get() )
            {
                ::sd::View* pView = pViewSh->GetView();
                if( pView )
                {
                    pView->SdrEndTextEdit();
                    sd::OutlineView* pOutlView = dynamic_cast< sd::OutlineView* >( pView );
                    if( pOutlView )
                    {
                        pOutlView->GetOutliner().GetUndoManager().Clear();
                    }
                }
            }
        }
        pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, false);
    }

    SfxUndoManager* pUndoManager = GetUndoManager();
    if(pUndoManager && pUndoManager->GetUndoActionCount())
        pUndoManager->Clear();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
