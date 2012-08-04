/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "templatedlg.hxx"

#include "inputdlg.hxx"
#include "templatesearchview.hxx"
#include "templatesearchviewitem.hxx"

#include <comphelper/processfactory.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/templatelocalviewitem.hxx>
#include <sfx2/templateonlineview.hxx>
#include <sfx2/templateonlineviewitem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <svtools/PlaceEditDialog.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include "doc.hrc"
#include "templatedlg.hrc"

#define ITEM_MAX_WIDTH 192
#define ITEM_MAX_HEIGHT 192
#define ITEM_PADDING 5
#define ITEM_SPACE 30
#define ITEM_MAX_TEXT_LENGTH 20
#define THUMBNAIL_MAX_HEIGHT 128

#define INIT_FOLDER_COLS 3
#define INIT_FOLDER_LINES 2

#define PADDING_TOOLBAR_VIEW    15
#define PADDING_DLG_BORDER      10

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

void lcl_createTemplate(uno::Reference<XComponentLoader> xDesktop, const FILTER_APPLICATION eApp);

// Sort by name in ascending order
class SortView_Name
{
public:

    bool operator() (const ThumbnailViewItem *pItem1, const ThumbnailViewItem *pItem2)
    {
        return (pItem1->maTitle.compareTo(pItem2->maTitle) < 0);
    }
};

class SearchView_Keyword
{
public:

    SearchView_Keyword (const rtl::OUString &rKeyword)
        : maKeyword(rKeyword)
    {}

    bool operator() (const TemplateItemProperties &rItem)
    {
        return rItem.aName.matchIgnoreAsciiCase(maKeyword);
    }

private:

    rtl::OUString maKeyword;
};

SfxTemplateManagerDlg::SfxTemplateManagerDlg (Window *parent)
    : ModelessDialog(parent, SfxResId(DLG_TEMPLATE_MANAGER)),
      aButtonAll(this,SfxResId(BTN_SELECT_ALL)),
      aButtonDocs(this,SfxResId(BTN_SELECT_DOCS)),
      aButtonPresents(this,SfxResId(BTN_SELECT_PRESENTATIONS)),
      aButtonSheets(this,SfxResId(BTN_SELECT_SHEETS)),
      aButtonDraws(this,SfxResId(BTN_SELECT_DRAWS)),
      maButtonClose(this,SfxResId(BTN_TEMPLATE_CLOSE)),
      maButtonSelMode(this,SfxResId(BTN_SELECTION_MODE)),
      mpSearchEdit(new Edit(this,WB_HIDE | WB_BORDER)),
      mpViewBar( new ToolBox(this, SfxResId(TBX_ACTION_VIEW))),
      mpActionBar( new ToolBox(this, SfxResId(TBX_ACTION_ACTION))),
      mpTemplateBar( new ToolBox(this, SfxResId(TBX_ACTION_TEMPLATES))),
      mpSearchView(new TemplateSearchView(this)),
      maView(new TemplateLocalView(this,SfxResId(TEMPLATE_VIEW))),
      mpOnlineView(new TemplateOnlineView(this, WB_VSCROLL,false)),
      mnSelectionCount(0),
      mxDesktop(comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.Desktop" ),uno::UNO_QUERY )
{
    maButtonSelMode.SetStyle(maButtonSelMode.GetStyle() | WB_TOGGLE);

    // Create popup menus
    mpCreateMenu = new PopupMenu;
    mpCreateMenu->InsertItem(MNI_CREATE_TEXT,SfxResId(STR_CREATE_TEXT).toString(),SfxResId(IMG_CREATE_TEXT));
    mpCreateMenu->InsertItem(MNI_CREATE_SHEET,SfxResId(STR_CREATE_SHEET).toString(),SfxResId(IMG_CREATE_SHEET));
    mpCreateMenu->InsertItem(MNI_CREATE_PRESENT,SfxResId(STR_CREATE_PRESENT).toString(),SfxResId(IMG_CREATE_PRESENT));
    mpCreateMenu->InsertItem(MNI_CREATE_DRAW,SfxResId(STR_CREATE_DRAW).toString(),SfxResId(IMG_CREATE_DRAW));
    mpCreateMenu->SetSelectHdl(LINK(this, SfxTemplateManagerDlg, MenuSelectHdl));

    mpActionMenu = new PopupMenu;
    mpActionMenu->InsertItem(MNI_ACTION_SORT_NAME,SfxResId(STR_ACTION_SORT_NAME).toString(),SfxResId(IMG_ACTION_SORT));
    mpActionMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,MenuSelectHdl));

    mpRepositoryMenu = new PopupMenu;
    mpRepositoryMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,RepositoryMenuSelectHdl));

    Size aWinSize = GetOutputSize();

    // Calculate thumbnail view minimum size
    Size aThumbSize = maView->CalcWindowSizePixel(INIT_FOLDER_COLS,INIT_FOLDER_LINES,
                                                  ITEM_MAX_WIDTH,ITEM_MAX_HEIGHT,ITEM_SPACE);

    if (aWinSize.getWidth() < aThumbSize.getWidth() + 2*PADDING_DLG_BORDER)
        aWinSize.setWidth(aThumbSize.getWidth() + 2*PADDING_DLG_BORDER);

    // Calculate toolboxs size and positions
    Size aViewSize = mpViewBar->CalcMinimumWindowSizePixel();
    Size aActionSize = mpActionBar->CalcMinimumWindowSizePixel();
    Size aTemplateSize = mpTemplateBar->CalcMinimumWindowSizePixel();

    aActionSize.setWidth(3*aActionSize.getWidth());
    aViewSize.setWidth(aWinSize.getWidth()-aActionSize.getWidth()-mpViewBar->GetPosPixel().X());
    aTemplateSize.setWidth(aWinSize.getWidth());

    Point aActionPos = mpActionBar->GetPosPixel();
    aActionPos.setX(aWinSize.getWidth() - aActionSize.getWidth());

    mpViewBar->SetSizePixel(aViewSize);
    mpActionBar->SetPosSizePixel(aActionPos,aActionSize);
    mpTemplateBar->SetSizePixel(aTemplateSize);

    // Set toolbox styles
    mpViewBar->SetButtonType(BUTTON_SYMBOLTEXT);
    mpTemplateBar->SetButtonType(BUTTON_SYMBOLTEXT);

    // Set toolbox button bits
    mpViewBar->EnableItem(TBI_TEMPLATE_IMPORT,false);
    mpViewBar->SetItemBits(TBI_TEMPLATE_CREATE, TIB_DROPDOWNONLY);
    mpViewBar->SetItemBits(TBI_TEMPLATE_REPOSITORY, TIB_DROPDOWNONLY);
    mpActionBar->SetItemBits(TBI_TEMPLATE_ACTION, TIB_DROPDOWNONLY);
    mpTemplateBar->SetItemBits(TBI_TEMPLATE_MOVE,TIB_DROPDOWNONLY);

    // Set toolbox handlers
    mpViewBar->SetClickHdl(LINK(this,SfxTemplateManagerDlg,TBXViewHdl));
    mpViewBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));
    mpActionBar->SetClickHdl(LINK(this,SfxTemplateManagerDlg,TBXActionHdl));
    mpActionBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));
    mpTemplateBar->SetClickHdl(LINK(this,SfxTemplateManagerDlg,TBXTemplateHdl));
    mpTemplateBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));

    // Set view position below toolbox
    Point aViewPos = maView->GetPosPixel();
    aViewPos.setY(aActionPos.Y() + aActionSize.getHeight() + PADDING_TOOLBAR_VIEW);
    aViewPos.setX((aWinSize.getWidth() - aThumbSize.getWidth())/2);     // Center the view
    maView->SetPosPixel(aViewPos);

    if (aWinSize.getHeight() < aViewPos.getY() + aThumbSize.getHeight() + PADDING_DLG_BORDER)
        aWinSize.setHeight(aViewPos.getY() + aThumbSize.getHeight() + PADDING_DLG_BORDER);

    // Set search box position and size
    Size aSearchSize = mpSearchEdit->CalcMinimumSize();
    aSearchSize.setWidth(aWinSize.getWidth() - 2*PADDING_DLG_BORDER);

    mpSearchEdit->SetSizePixel(aSearchSize);
    mpSearchEdit->SetPosPixel(Point(PADDING_DLG_BORDER,aActionPos.Y()+aActionSize.getHeight()));
    mpSearchEdit->SetUpdateDataHdl(LINK(this,SfxTemplateManagerDlg,SearchUpdateHdl));
    mpSearchEdit->EnableUpdateData();

    maView->SetStyle(WB_VSCROLL);
    maView->SetSizePixel(aThumbSize);
    maView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);

    maView->setItemDimensions(ITEM_MAX_WIDTH,THUMBNAIL_MAX_HEIGHT,
                              ITEM_MAX_HEIGHT-THUMBNAIL_MAX_HEIGHT,
                              ITEM_PADDING);

    maView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVFolderStateHdl));
    maView->setOverlayItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));
    maView->setOverlayDblClickHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));
    maView->setOverlayCloseHdl(LINK(this,SfxTemplateManagerDlg,CloseOverlayHdl));

    // Set online view position and dimensions
    mpOnlineView->SetPosSizePixel(aViewPos,aThumbSize);
    mpOnlineView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);

    mpOnlineView->setItemDimensions(ITEM_MAX_WIDTH,THUMBNAIL_MAX_HEIGHT,
                                    ITEM_MAX_HEIGHT-THUMBNAIL_MAX_HEIGHT,
                                    ITEM_PADDING);

    mpOnlineView->setOverlayItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));
    mpOnlineView->setOverlayDblClickHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));
    mpOnlineView->setOverlayCloseHdl(LINK(this,SfxTemplateManagerDlg,CloseOverlayHdl));

    mpSearchView->SetSizePixel(aThumbSize);
    mpSearchView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);

    mpSearchView->setItemDimensions(ITEM_MAX_WIDTH,THUMBNAIL_MAX_HEIGHT,
                                    ITEM_MAX_HEIGHT-THUMBNAIL_MAX_HEIGHT,
                                    ITEM_PADDING);

    mpSearchView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));

    // Set OK button position
    Point aBtnPos;
    Size aBtnSize = maButtonClose.GetSizePixel();
    aBtnPos.setX(aWinSize.getWidth() - PADDING_DLG_BORDER - aBtnSize.getWidth());
    aBtnPos.setY(aViewPos.getY()+aThumbSize.getHeight() + PADDING_TOOLBAR_VIEW);
    maButtonClose.SetPosPixel(aBtnPos);

    if (aWinSize.getHeight() != aBtnPos.getY() + aBtnSize.getHeight() + PADDING_DLG_BORDER )
        aWinSize.setHeight(aBtnPos.getY() + aBtnSize.getHeight() + PADDING_DLG_BORDER);

    aButtonAll.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewAllHdl));
    aButtonDocs.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDocsHdl));
    aButtonPresents.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewPresentsHdl));
    aButtonSheets.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewSheetsHdl));
    aButtonDraws.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDrawsHdl));
    maButtonClose.SetClickHdl(LINK(this,SfxTemplateManagerDlg,CloseHdl));
    maButtonSelMode.SetClickHdl(LINK(this,SfxTemplateManagerDlg,OnClickSelectionMode));

    // Set dialog to correct dimensions
    SetSizePixel(aWinSize);

    centerTopButtons();

    mpViewBar->Show();
    mpActionBar->Show();

    switchMainView(true);

    mpOnlineView->Populate();

    createRepositoryMenu();

    maView->Populate();
    maView->Show();

    FreeResource();
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg ()
{
    delete mpSearchEdit;
    delete mpViewBar;
    delete mpActionBar;
    delete mpTemplateBar;
    delete mpSearchView;
    delete maView;
    delete mpOnlineView;
    delete mpCreateMenu;
    delete mpActionMenu;
    delete mpRepositoryMenu;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewAllHdl)
{
    mpCurView->filterTemplatesByApp(FILTER_APP_NONE);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewDocsHdl)
{
    mpCurView->filterTemplatesByApp(FILTER_APP_WRITER);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewPresentsHdl)
{
    mpCurView->filterTemplatesByApp(FILTER_APP_IMPRESS);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewSheetsHdl)
{
    mpCurView->filterTemplatesByApp(FILTER_APP_CALC);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewDrawsHdl)
{
    mpCurView->filterTemplatesByApp(FILTER_APP_DRAW);
    return 0;
}

void SfxTemplateManagerDlg::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!maView->GetActiveClipRegion().IsInside(rMEvt.GetPosPixel()) && maView->isOverlayVisible())
    {
        maSelTemplates.clear();
        mpTemplateBar->Hide();
        mpViewBar->Show();

        maView->showOverlay(false);
    }
}

IMPL_LINK_NOARG (SfxTemplateManagerDlg, CloseHdl)
{
    Close();
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, CloseOverlayHdl)
{
    maSelTemplates.clear();
    mpTemplateBar->Hide();
    mpViewBar->Show();

    if (mpCurView == maView)
        mpCurView->showOverlay(false);
    else
        switchMainView(true);

    return 0;
}

IMPL_LINK (SfxTemplateManagerDlg, OnClickSelectionMode, ImageButton*, pButton)
{
    maView->setSelectionMode(pButton->GetState() == STATE_CHECK);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,TBXViewHdl)
{
    switch(mpViewBar->GetCurItemId())
    {
    case TBI_TEMPLATE_IMPORT:
        OnTemplateImport();
        break;
    case TBI_TEMPLATE_FOLDER_DEL:
        OnFolderDelete();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,TBXActionHdl)
{
    switch(mpActionBar->GetCurItemId())
    {
    case TBI_TEMPLATE_SEARCH:
        OnTemplateSearch();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,TBXTemplateHdl)
{
    switch(mpTemplateBar->GetCurItemId())
    {
    case TBI_TEMPLATE_EDIT:
        OnTemplateEdit();
        break;
    case TBI_TEMPLATE_PROPERTIES:
        OnTemplateProperties();
        break;
    case TBI_TEMPLATE_DELETE:
        OnTemplateDelete();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, TBXDropdownHdl, ToolBox*, pBox)
{
    const sal_uInt16 nCurItemId = pBox->GetCurItemId();

    switch(nCurItemId)
    {
    case TBI_TEMPLATE_CREATE:
        pBox->SetItemDown( nCurItemId, true );

        mpCreateMenu->Execute(pBox,pBox->GetItemRect(TBI_TEMPLATE_CREATE),
                              POPUPMENU_EXECUTE_DOWN);

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
        break;
    case TBI_TEMPLATE_ACTION:
        pBox->SetItemDown( nCurItemId, true );

        mpActionMenu->Execute(pBox,pBox->GetItemRect(TBI_TEMPLATE_ACTION),
                              POPUPMENU_EXECUTE_DOWN);

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
        break;
    case TBI_TEMPLATE_MOVE:
    {
        pBox->SetItemDown( nCurItemId, true );

        std::vector<rtl::OUString> aNames = maView->getFolderNames();

        PopupMenu *pMoveMenu = new PopupMenu;
        pMoveMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,MoveMenuSelectHdl));

        if (!aNames.empty())
        {
            for (size_t i = 0, n = aNames.size(); i < n; ++i)
                pMoveMenu->InsertItem(MNI_MOVE_FOLDER_BASE+i,aNames[i]);
        }

        pMoveMenu->InsertSeparator();

        pMoveMenu->InsertItem(MNI_MOVE_NEW,SfxResId(STR_MOVE_NEW).toString());

        pMoveMenu->Execute(pBox,pBox->GetItemRect(TBI_TEMPLATE_MOVE),
                            POPUPMENU_EXECUTE_DOWN);

        delete pMoveMenu;

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
        break;
    }
    case TBI_TEMPLATE_REPOSITORY:
        pBox->SetItemDown( nCurItemId, true );

        mpRepositoryMenu->Execute(pBox,pBox->GetItemRect(TBI_TEMPLATE_REPOSITORY),
                                  POPUPMENU_EXECUTE_DOWN);

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, TVFolderStateHdl, const ThumbnailViewItem*, pItem)
{
    if (pItem->isSelected())
    {
        if (maSelFolders.empty())
        {
            mpViewBar->EnableItem(TBI_TEMPLATE_IMPORT,true);
            mpViewBar->ShowItem(TBI_TEMPLATE_FOLDER_DEL);
        }

        maSelFolders.insert(pItem);
    }
    else
    {
        maSelFolders.erase(pItem);

        if (maSelFolders.empty())
        {
            mpViewBar->EnableItem(TBI_TEMPLATE_IMPORT,false);
            mpViewBar->HideItem(TBI_TEMPLATE_FOLDER_DEL);
        }
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, TVTemplateStateHdl, const ThumbnailViewItem*, pItem)
{
    if (pItem->isSelected())
    {
        if (maSelTemplates.empty())
        {
            mpViewBar->Show(false);
            mpActionBar->Show(false);
            mpTemplateBar->Show();
        }

        maSelTemplates.insert(pItem);
    }
    else
    {
        if (maSelTemplates.find(pItem) != maSelTemplates.end())
        {
            maSelTemplates.erase(pItem);

            if (maSelTemplates.empty())
            {
                mpTemplateBar->Show(false);
                mpViewBar->Show();
                mpActionBar->Show();
            }
        }
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, MenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_CREATE_TEXT:
        lcl_createTemplate(mxDesktop,FILTER_APP_WRITER);
        Close( );
        break;
    case MNI_CREATE_SHEET:
        lcl_createTemplate(mxDesktop,FILTER_APP_CALC);
        Close( );
        break;
    case MNI_CREATE_PRESENT:
        lcl_createTemplate(mxDesktop,FILTER_APP_IMPRESS);
        Close( );
        break;
    case MNI_CREATE_DRAW:
        lcl_createTemplate(mxDesktop,FILTER_APP_DRAW);
        Close( );
        break;
    case MNI_ACTION_SORT_NAME:
        if (maView->isOverlayVisible())
            maView->sortOverlayItems(SortView_Name());
        else
            maView->sortItems(SortView_Name());
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, MoveMenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    if (mpSearchView->IsVisible())
    {
        // Check if we are searching the local or remote templates
        if (mpCurView == maView)
            localSearchMoveTo(nMenuId);
    }
    else
    {
        // Check if we are displaying the local or remote templates
        if (mpCurView == maView)
            localMoveTo(nMenuId);
        else
            remoteMoveTo(nMenuId);
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, RepositoryMenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    if (nMenuId == MNI_REPOSITORY_LOCAL)
    {
        switchMainView(true);
    }
    else if (nMenuId == MNI_REPOSITORY_NEW)
    {
        PlaceEditDialog dlg(this);

        if (dlg.Execute())
        {
            boost::shared_ptr<Place> pPlace = dlg.GetPlace();

            mpOnlineView->insertRepository(pPlace->GetName(),pPlace->GetUrl());

            // update repository list menu.
            createRepositoryMenu();
        }
    }
    else
    {
        sal_uInt16 nRepoId = nMenuId - MNI_REPOSITORY_BASE;

        if (mpOnlineView->loadRepository(nRepoId,false))
        {
            switchMainView(false);
            mpOnlineView->showOverlay(true);
        }
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem)
{
    uno::Sequence< PropertyValue > aArgs(1);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= sal_True;

    TemplateViewItem *pTemplateItem = static_cast<TemplateViewItem*>(pItem);

    try
    {
        mxDesktop->loadComponentFromURL(pTemplateItem->getPath(),rtl::OUString("_default"), 0, aArgs );
    }
    catch( const uno::Exception& )
    {
    }

    Close();

    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SearchUpdateHdl)
{
    // if the search view is hidden, hide the folder view and display search one
    if (!mpCurView->isOverlayVisible() && !mpSearchView->IsVisible())
    {
        mpSearchView->Clear();
        mpSearchView->Show();
        mpCurView->Hide();
    }

    rtl::OUString aKeyword = mpSearchEdit->GetText();

    if (!aKeyword.isEmpty())
    {
        if (mpCurView->isOverlayVisible())
        {
            mpCurView->filterTemplatesByKeyword(aKeyword);
        }
        else
        {
            mpSearchView->Clear();

            std::vector<TemplateItemProperties> aItems =
                    maView->getFilteredItems(SearchView_Keyword(aKeyword));

            size_t nCounter = 0;
            for (size_t i = 0; i < aItems.size(); ++i)
            {
                TemplateItemProperties *pItem = &aItems[i];

                mpSearchView->AppendItem(++nCounter,pItem->nRegionId,
                                         pItem->nId-1,
                                         pItem->aName,
                                         maView->GetItemText(pItem->nRegionId+1),
                                         pItem->aPath,
                                         pItem->aThumbnail);
            }

            mpSearchView->Invalidate();
        }
    }
    else
    {
        if (mpCurView->isOverlayVisible())
        {
            mpCurView->filterTemplatesByApp(FILTER_APP_NONE);
        }
        else
        {
            mpSearchView->Hide();
            mpCurView->Show();
        }
    }

    return 0;
}

void SfxTemplateManagerDlg::OnTemplateImport ()
{
    sal_Int16 nDialogType =
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;

    sfx2::FileDialogHelper aFileDlg(nDialogType, SFXWB_MULTISELECTION);

    // add "All" filter
    aFileDlg.AddFilter( String(SfxResId( STR_SFX_FILTERNAME_ALL) ),
                        DEFINE_CONST_UNICODE(FILEDIALOG_FILTER_ALL) );

    // add template filter
    rtl::OUString sFilterExt;
    rtl::OUString sFilterName( SfxResId( STR_TEMPLATE_FILTER ).toString() );

    // add filters of modules which are installed
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        sFilterExt += "*.ott;*.stw;*.oth";

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.ots;*.stc";
    }

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.otp;*.sti";
    }

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.otg;*.std";
    }

    if ( !sFilterExt.isEmpty() )
        sFilterExt += ";";

    sFilterExt += "*.vor";

    sFilterName += " (";
    sFilterName += sFilterExt;
    sFilterName += ")";

    aFileDlg.AddFilter( sFilterName, sFilterExt );
    aFileDlg.SetCurrentFilter( sFilterName );

    ErrCode nCode = aFileDlg.Execute();

    if ( nCode == ERRCODE_NONE )
    {
        com::sun::star::uno::Sequence< ::rtl::OUString > aFiles = aFileDlg.GetSelectedFiles();

        if (aFiles.hasElements())
        {
            std::set<const ThumbnailViewItem*>::const_iterator pIter;
            for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
            {
                TemplateLocalViewItem *pFolder = (TemplateLocalViewItem*)(*pIter);

                for (size_t i = 0, n = aFiles.getLength(); i < n; ++i)
                    maView->copyFrom(pFolder,aFiles[i]);
            }
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateSearch ()
{
    Point aPos = maView->GetPosPixel();
    Point aClosePos = maButtonClose.GetPosPixel();
    bool bVisible = mpSearchEdit->IsVisible();
    Size aWinSize = GetSizePixel();
    long nEditHeight = mpSearchEdit->GetSizePixel().getHeight();

    if (bVisible)
    {
        aWinSize.setHeight(aWinSize.getHeight() - nEditHeight );
        aPos.setY(aPos.getY() - nEditHeight );
        aClosePos.setY(aClosePos.getY() - nEditHeight );
        mpActionBar->SetItemState(TBI_TEMPLATE_SEARCH,STATE_NOCHECK);
    }
    else
    {
        aWinSize.setHeight(aWinSize.getHeight() + nEditHeight );
        aPos.setY(aPos.getY() + nEditHeight );
        aClosePos.setY(aClosePos.getY() + nEditHeight );
        mpActionBar->SetItemState(TBI_TEMPLATE_SEARCH,STATE_CHECK);
    }

    SetSizePixel(aWinSize);
    maView->SetPosPixel(aPos);
    mpOnlineView->SetPosPixel(aPos);
    mpSearchView->SetPosPixel(aPos);
    maButtonClose.SetPosPixel(aClosePos);

    // Hide search view
    if (bVisible)
    {
        mpSearchView->Hide();
        mpCurView->Show();
    }

    mpSearchEdit->Show(!bVisible);
    mpSearchEdit->SetText(rtl::OUString());

    // display all templates if we hide the search bar
    if (bVisible && mpCurView->isOverlayVisible())
        mpCurView->filterTemplatesByApp(FILTER_APP_NONE);
}

void SfxTemplateManagerDlg::OnTemplateEdit ()
{
    uno::Sequence< PropertyValue > aArgs(1);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= sal_False;

    uno::Reference< XStorable > xStorable;
    std::set<const ThumbnailViewItem*>::const_iterator pIter;
    for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end(); ++pIter)
    {
        const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*pIter);

        try
        {
            xStorable = uno::Reference< XStorable >(
                        mxDesktop->loadComponentFromURL(pItem->getPath(),rtl::OUString("_blank"), 0, aArgs ),
                        uno::UNO_QUERY );
        }
        catch( const uno::Exception& )
        {
        }
    }

    Close();
}

void SfxTemplateManagerDlg::OnTemplateProperties ()
{
}

void SfxTemplateManagerDlg::OnTemplateDelete ()
{
    if (mpSearchView->IsVisible())
    {
        std::set<const ThumbnailViewItem*>::const_iterator pIter;
        for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end();)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*pIter);

            sal_uInt16 nItemId = pItem->mnIdx + 1;
            sal_uInt16 nItemRegionId = pItem->mnRegionId + 1;

            if (maView->removeTemplate(nItemId,nItemRegionId))
                maSelTemplates.erase(pIter++);
            else
                ++pIter;
        }

        // Update search results
        if (maSelTemplates.empty())
        {
            mpTemplateBar->Show(false);
            mpViewBar->Show();
            mpActionBar->Show();
        }

        SearchUpdateHdl(mpSearchEdit);
    }
    else
    {
        std::set<const ThumbnailViewItem*>::const_iterator pIter;
        for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end();)
        {
            if (maView->removeTemplate((*pIter)->mnId,maView->getOverlayRegionId()+1))
                maSelTemplates.erase(pIter++);
            else
                ++pIter;
        }
    }
}

void SfxTemplateManagerDlg::OnFolderDelete()
{
    std::set<const ThumbnailViewItem*>::const_iterator pIter;
    for (pIter = maSelFolders.begin(); pIter != maSelFolders.end();)
    {
        if (maView->removeRegion((*pIter)->mnId))
            maSelFolders.erase(pIter++);
        else
            ++pIter;
    }

    if (maSelFolders.empty())
    {
        mpViewBar->EnableItem(TBI_TEMPLATE_IMPORT,false);
        mpViewBar->HideItem(TBI_TEMPLATE_FOLDER_DEL);
    }
}

void SfxTemplateManagerDlg::centerTopButtons()
{
    Point aFirstBtnPos = aButtonAll.GetPosPixel();

    Size aSelBtnSize = maButtonSelMode.GetOutputSize(); // Last button in the list
    Size aBtnSize = aButtonAll.GetOutputSize();
    Size aWinSize = GetOutputSize();

    long nTotalWidth = aSelBtnSize.getWidth() + aBtnSize.getWidth()*5;
    long nSpace = (aWinSize.getWidth() - nTotalWidth)/2;

    Point aBtnPos(nSpace,aFirstBtnPos.getY());
    aButtonAll.SetPosPixel(aBtnPos);

    aBtnPos.setX(aBtnPos.getX() + aBtnSize.getWidth());
    aButtonDocs.SetPosPixel(aBtnPos);

    aBtnPos.setX(aBtnPos.getX() + aBtnSize.getWidth());
    aButtonPresents.SetPosPixel(aBtnPos);

    aBtnPos.setX(aBtnPos.getX() + aBtnSize.getWidth());
    aButtonSheets.SetPosPixel(aBtnPos);

    aBtnPos.setX(aBtnPos.getX() + aBtnSize.getWidth());
    aButtonDraws.SetPosPixel(aBtnPos);

    aBtnPos.setX(aBtnPos.getX() + aBtnSize.getWidth());
    maButtonSelMode.SetPosPixel(aBtnPos);
}

void SfxTemplateManagerDlg::createRepositoryMenu()
{
    mpRepositoryMenu->Clear();

    mpRepositoryMenu->InsertItem(MNI_REPOSITORY_LOCAL,SfxResId(STR_REPOSITORY_LOCAL).toString());

    const std::vector<TemplateOnlineViewItem*> &rRepos = mpOnlineView->getRepositories();

    for (size_t i = 0, n = rRepos.size(); i < n; ++i)
        mpRepositoryMenu->InsertItem(MNI_REPOSITORY_BASE+rRepos[i]->mnId,rRepos[i]->maTitle);

    mpRepositoryMenu->InsertSeparator();
    mpRepositoryMenu->InsertItem(MNI_REPOSITORY_NEW,SfxResId(STR_REPOSITORY_NEW).toString());
}

void SfxTemplateManagerDlg::switchMainView(bool bDisplayLocal)
{
    if (bDisplayLocal)
    {
        mpCurView = maView;

        // Enable deleting items from the filesystem
        mpTemplateBar->ShowItem(TBI_TEMPLATE_DELETE);

        mpOnlineView->Hide();
        maView->Show();
    }
    else
    {
        mpCurView = mpOnlineView;

        // Disable deleting items from remote repositories
        mpTemplateBar->HideItem(TBI_TEMPLATE_DELETE);

        maView->Hide();
        mpOnlineView->Show();
    }
}

void SfxTemplateManagerDlg::localMoveTo(sal_uInt16 nMenuId)
{
    sal_uInt16 nItemId = 0;

    if (nMenuId == MNI_MOVE_NEW)
    {
        InputDialog dlg(SfxResId(STR_INPUT_NEW).toString(),this);

        int ret = dlg.Execute();

        if (ret)
        {
            rtl::OUString aName = dlg.getEntryText();

            if (!aName.isEmpty())
                nItemId = maView->createRegion(aName);
        }
    }
    else
    {
        nItemId = maView->GetItemId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        if (!maView->moveTemplates(maSelTemplates,nItemId,false) &&
                !maView->moveTemplates(maSelTemplates,nItemId,true))
        {
        }
    }
}

void SfxTemplateManagerDlg::remoteMoveTo(const sal_uInt16 nMenuId)
{
    sal_uInt16 nItemId = 0;

    if (nMenuId == MNI_MOVE_NEW)
    {
        InputDialog dlg(SfxResId(STR_INPUT_NEW).toString(),this);

        int ret = dlg.Execute();

        if (ret)
        {
            rtl::OUString aName = dlg.getEntryText();

            if (!aName.isEmpty())
                nItemId = maView->createRegion(aName);
        }
    }
    else
    {
        nItemId = maView->GetItemId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        std::set<const ThumbnailViewItem*>::const_iterator aIter;
        for (aIter = maSelTemplates.begin(); aIter != maSelTemplates.end(); ++aIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*aIter);

            maView->copyFrom(nItemId,pItem->maPreview1,pItem->getPath());
        }
    }
}

void SfxTemplateManagerDlg::localSearchMoveTo(sal_uInt16 nMenuId)
{
    sal_uInt16 nItemId = 0;

    if (nMenuId == MNI_MOVE_NEW)
    {
        InputDialog dlg(SfxResId(STR_INPUT_NEW).toString(),this);

        int ret = dlg.Execute();

        if (ret)
        {
            rtl::OUString aName = dlg.getEntryText();

            if (!aName.isEmpty())
                nItemId = maView->createRegion(aName);
        }
    }
    else
    {
        nItemId = maView->GetItemId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        std::set<const ThumbnailViewItem*>::const_iterator aIter;
        for (aIter = maSelTemplates.begin(); aIter != maSelTemplates.end(); ++aIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*aIter);

            if(!maView->moveTemplate(pItem,pItem->mnRegionId+1,nItemId,false)
                    && !maView->moveTemplate(pItem,pItem->mnRegionId+1,nItemId,true))
            {
            }
        }
    }

    // Deselect all items and update search results
    mpSearchView->unselectItems();

    SearchUpdateHdl(mpSearchEdit);
}

void lcl_createTemplate(uno::Reference< com::sun::star::frame::XComponentLoader > xDesktop,
                        const FILTER_APPLICATION eApp)
{
    rtl::OUString aURL;

    switch(eApp)
    {
    case FILTER_APP_WRITER:
        aURL = "private:factory/swriter";
        break;
    case FILTER_APP_CALC:
        aURL = "private:factory/scalc";
        break;
    case FILTER_APP_IMPRESS:
        aURL = "private:factory/simpress";
        break;
    case FILTER_APP_DRAW:
        aURL = "private:factory/sdraw";
        break;
    default:
        break;
    }

    if (!aURL.isEmpty())
    {
        uno::Sequence<PropertyValue> aArgs;
        xDesktop->loadComponentFromURL(aURL,rtl::OUString("_default"), 0, aArgs );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
