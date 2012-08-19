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
#include <comphelper/storagehelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templateinfodlg.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/templatelocalviewitem.hxx>
#include <sfx2/templateonlineview.hxx>
#include <sfx2/templateonlineviewitem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <sot/storage.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/PlaceEditDialog.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>

#include "doc.hrc"
#include "templatedlg.hrc"

#define INIT_FOLDER_COLS 3
#define INIT_FOLDER_LINES 2

#define PADDING_TOOLBAR_VIEW    15
#define PADDING_DLG_BORDER      10

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;

void lcl_createTemplate(uno::Reference<XComponentLoader> xDesktop, const FILTER_APPLICATION eApp);

bool lcl_getServiceName (const OUString &rFileURL, OUString &rName );

std::vector<OUString> lcl_getAllFactoryURLs ();

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
      maButtonSelMode(this,SfxResId(BTN_SELECTION_MODE)),
      mpSearchEdit(new Edit(this,WB_HIDE | WB_BORDER)),
      mpViewBar( new ToolBox(this, SfxResId(TBX_ACTION_VIEW))),
      mpActionBar( new ToolBox(this, SfxResId(TBX_ACTION_ACTION))),
      mpTemplateBar( new ToolBox(this, SfxResId(TBX_ACTION_TEMPLATES))),
      mpSearchView(new TemplateSearchView(this)),
      maView(new TemplateLocalView(this,SfxResId(TEMPLATE_VIEW))),
      mpOnlineView(new TemplateOnlineView(this, WB_VSCROLL,false)),
      mbIsSaveMode(false),
      mxDesktop(comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.Desktop" ),uno::UNO_QUERY )
{
    maButtonSelMode.SetStyle(maButtonSelMode.GetStyle() | WB_TOGGLE);

    // Create popup menus
    mpActionMenu = new PopupMenu;
    mpActionMenu->InsertItem(MNI_ACTION_SORT_NAME,SfxResId(STR_ACTION_SORT_NAME).toString(),SfxResId(IMG_ACTION_SORT));
    mpActionMenu->InsertItem(MNI_ACTION_REFRESH,SfxResId(STR_ACTION_REFRESH).toString(),SfxResId(IMG_ACTION_REFRESH));
    mpActionMenu->InsertItem(MNI_ACTION_DEFAULT,SfxResId(STR_ACTION_DEFAULT).toString());
    mpActionMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,MenuSelectHdl));

    mpRepositoryMenu = new PopupMenu;
    mpRepositoryMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,RepositoryMenuSelectHdl));

    mpTemplateDefaultMenu = new PopupMenu;
    mpTemplateDefaultMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,DefaultTemplateMenuSelectHdl));
    mpActionMenu->SetPopupMenu(MNI_ACTION_DEFAULT,mpTemplateDefaultMenu);

    Size aWinSize = GetOutputSize();

    // Calculate thumbnail view minimum size
    Size aThumbSize = maView->CalcWindowSizePixel(INIT_FOLDER_COLS,INIT_FOLDER_LINES,
                                                  TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_MAX_HEIGHT,TEMPLATE_ITEM_SPACE);

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
    maView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    maView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_PADDING);

    maView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVFolderStateHdl));
    maView->setOverlayItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));
    maView->setOverlayDblClickHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));
    maView->setOverlayCloseHdl(LINK(this,SfxTemplateManagerDlg,CloseOverlayHdl));

    // Set online view position and dimensions
    mpOnlineView->SetPosSizePixel(aViewPos,aThumbSize);
    mpOnlineView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpOnlineView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mpOnlineView->setOverlayItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));
    mpOnlineView->setOverlayDblClickHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));
    mpOnlineView->setOverlayCloseHdl(LINK(this,SfxTemplateManagerDlg,CloseOverlayHdl));
    mpOnlineView->setOverlayChangeNameHdl(LINK(this,SfxTemplateManagerDlg,RepositoryChangeNameHdl));

    mpSearchView->SetSizePixel(aThumbSize);
    mpSearchView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpSearchView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mpSearchView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));

    aButtonAll.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewAllHdl));
    aButtonDocs.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDocsHdl));
    aButtonPresents.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewPresentsHdl));
    aButtonSheets.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewSheetsHdl));
    aButtonDraws.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDrawsHdl));
    maButtonSelMode.SetClickHdl(LINK(this,SfxTemplateManagerDlg,OnClickSelectionMode));

    // Set dialog to correct dimensions
    SetSizePixel(aWinSize);

    centerTopButtons();

    mpViewBar->Show();
    mpActionBar->Show();

    switchMainView(true);

    mpOnlineView->Populate();

    createRepositoryMenu();
    createDefaultTemplateMenu();

    maView->Populate();
    maView->Show();

    FreeResource();
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg ()
{
    mpOnlineView->syncRepositories();

    delete mpSearchEdit;
    delete mpViewBar;
    delete mpActionBar;
    delete mpTemplateBar;
    delete mpSearchView;
    delete maView;
    delete mpOnlineView;
    delete mpActionMenu;
    delete mpRepositoryMenu;
    delete mpTemplateDefaultMenu;
}

void SfxTemplateManagerDlg::setSaveMode(bool bMode)
{
    mbIsSaveMode = bMode;

    if (bMode)
    {
        mpViewBar->ShowItem(TBI_TEMPLATE_SAVE);
        mpViewBar->HideItem(TBI_TEMPLATE_IMPORT);
        mpViewBar->HideItem(TBI_TEMPLATE_REPOSITORY);
    }
    else
    {
        mpViewBar->HideItem(TBI_TEMPLATE_SAVE);
        mpViewBar->ShowItem(TBI_TEMPLATE_IMPORT);
        mpViewBar->ShowItem(TBI_TEMPLATE_REPOSITORY);
    }
}

void SfxTemplateManagerDlg::setDocumentModel(const uno::Reference<frame::XModel> &rModel)
{
    m_xModel = rModel;
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

IMPL_LINK_NOARG(SfxTemplateManagerDlg, CloseOverlayHdl)
{
    maSelTemplates.clear();
    mpTemplateBar->Hide();
    mpViewBar->Show();
    mpActionBar->Show();

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
        if (mpCurView == maView)
            OnFolderDelete();
        else
            OnRepositoryDelete();
        break;
    case TBI_TEMPLATE_SAVE:
        OnTemplateSaveAs();
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
    case TBI_TEMPLATE_DEFAULT:
        OnTemplateAsDefault();
        break;
    case TBI_TEMPLATE_EXPORT:
        OnTemplateExport();
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
        if (maSelFolders.empty() && !mbIsSaveMode)
        {
            mpViewBar->ShowItem(TBI_TEMPLATE_IMPORT);
            mpViewBar->ShowItem(TBI_TEMPLATE_FOLDER_DEL);
        }

        maSelFolders.insert(pItem);
    }
    else
    {
        maSelFolders.erase(pItem);

        if (maSelFolders.empty() && !mbIsSaveMode)
        {
            mpViewBar->HideItem(TBI_TEMPLATE_IMPORT);
            mpViewBar->HideItem(TBI_TEMPLATE_FOLDER_DEL);
        }
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, TVTemplateStateHdl, const ThumbnailViewItem*, pItem)
{
    if (pItem->isSelected())
    {
        if (!mbIsSaveMode)
        {
            if (maSelTemplates.empty())
            {
                mpViewBar->Show(false);
                mpActionBar->Show(false);
                mpTemplateBar->Show();
            }
            else
            {
                mpTemplateBar->HideItem(TBI_TEMPLATE_EDIT);
                mpTemplateBar->HideItem(TBI_TEMPLATE_PROPERTIES);
                mpTemplateBar->HideItem(TBI_TEMPLATE_DEFAULT);
            }
        }

        maSelTemplates.insert(pItem);
    }
    else
    {
        if (maSelTemplates.find(pItem) != maSelTemplates.end())
        {
            maSelTemplates.erase(pItem);

            if (!mbIsSaveMode)
            {
                if (maSelTemplates.empty())
                {
                    mpTemplateBar->Show(false);
                    mpViewBar->Show();
                    mpActionBar->Show();
                }
                else if (maSelTemplates.size() == 1)
                {
                    mpTemplateBar->ShowItem(TBI_TEMPLATE_EDIT);
                    mpTemplateBar->ShowItem(TBI_TEMPLATE_PROPERTIES);
                    mpTemplateBar->ShowItem(TBI_TEMPLATE_DEFAULT);
                }
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
    case MNI_ACTION_SORT_NAME:
        if (maView->isOverlayVisible())
            maView->sortOverlayItems(SortView_Name());
        else
            maView->sortItems(SortView_Name());
        break;
    case MNI_ACTION_REFRESH:
        mpCurView->reload();
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

            if (mpOnlineView->insertRepository(pPlace->GetName(),pPlace->GetUrl()))
            {
                // update repository list menu.
                createRepositoryMenu();
            }
            else
            {
                OUString aMsg(SfxResId(STR_MSG_ERROR_REPOSITORY_NAME).toString());
                aMsg = aMsg.replaceFirst("$1",pPlace->GetName());
                ErrorBox(this,WB_OK,aMsg).Execute();
            }
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

IMPL_LINK(SfxTemplateManagerDlg, DefaultTemplateMenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    OUString aServiceName = SfxObjectShell::GetServiceNameFromFactory( mpTemplateDefaultMenu->GetItemCommand(nId));
    SfxObjectFactory::SetStandardTemplate( aServiceName, OUString() );

    createDefaultTemplateMenu();

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem)
{
    if (!mbIsSaveMode)
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
    }

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

IMPL_LINK_NOARG (SfxTemplateManagerDlg, RepositoryChangeNameHdl)
{
    createRepositoryMenu();
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
                OUString aTemplateList;
                TemplateLocalViewItem *pFolder = (TemplateLocalViewItem*)(*pIter);

                for (size_t i = 0, n = aFiles.getLength(); i < n; ++i)
                {
                    if(!maView->copyFrom(pFolder,aFiles[i]))
                    {
                        if (aTemplateList.isEmpty())
                            aTemplateList = aFiles[i];
                        else
                            aTemplateList = aTemplateList + "\n" + aFiles[i];
                    }
                }

                if (!aTemplateList.isEmpty())
                {
                    OUString aMsg(SfxResId(STR_MSG_ERROR_IMPORT).toString());
                    aMsg = aMsg.replaceFirst("$1",pFolder->maTitle);
                    ErrorBox(this,WB_OK,aMsg.replaceFirst("$2",aTemplateList));
                }
            }

            maView->Invalidate(INVALIDATE_NOERASE);
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateExport()
{
    uno::Reference<XMultiServiceFactory> xFactory(comphelper::getProcessServiceFactory());
    uno::Reference<XFolderPicker> xFolderPicker(xFactory->createInstance(FOLDER_PICKER_SERVICE_NAME),uno::UNO_QUERY);

    xFolderPicker->setDisplayDirectory(SvtPathOptions().GetWorkPath());

    sal_Int16 nResult = xFolderPicker->execute();

    if( nResult == ExecutableDialogResults::OK )
    {
        OUString aTemplateList;
        INetURLObject aPathObj(xFolderPicker->getDirectory());
        aPathObj.setFinalSlash();

        if (mpSearchView->IsVisible())
        {
            sal_uInt16 i = 1;

            std::set<const ThumbnailViewItem*>::const_iterator pIter = maSelTemplates.begin();
            for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end(); ++pIter, ++i)
            {
                const TemplateSearchViewItem *pItem = static_cast<const TemplateSearchViewItem*>(*pIter);

                INetURLObject aItemPath(pItem->getPath());

                if ( 1 == i )
                    aPathObj.Append(aItemPath.getName());
                else
                    aPathObj.setName(aItemPath.getName());

                OUString aPath = aPathObj.GetMainURL( INetURLObject::NO_DECODE );

                if (!maView->exportTo(pItem->mnIdx+1,pItem->mnRegionId+1,aPath))
                {
                    if (aTemplateList.isEmpty())
                        aTemplateList = pItem->maTitle;
                    else
                        aTemplateList = aTemplateList + "\n" + pItem->maTitle;
                }
            }

            mpSearchView->deselectItems();
        }
        else
        {
            // export templates from the current open overlay

            sal_uInt16 i = 1;
            sal_uInt16 nRegionItemId = maView->getOverlayRegionId() + 1;

            std::set<const ThumbnailViewItem*>::const_iterator pIter = maSelTemplates.begin();
            for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end(); ++pIter, ++i)
            {
                const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*pIter);

                INetURLObject aItemPath(pItem->getPath());

                if ( 1 == i )
                    aPathObj.Append(aItemPath.getName());
                else
                    aPathObj.setName(aItemPath.getName());

                OUString aPath = aPathObj.GetMainURL( INetURLObject::NO_DECODE );

                if (!maView->exportTo(pItem->mnId,nRegionItemId,aPath))
                {
                    if (aTemplateList.isEmpty())
                        aTemplateList = pItem->maTitle;
                    else
                        aTemplateList = aTemplateList + "\n" + pItem->maTitle;
                }
            }

            maView->deselectOverlayItems();
        }

        if (!aTemplateList.isEmpty())
        {
            OUString aText( SfxResId(STR_MSG_ERROR_EXPORT).toString() );
            ErrorBox(this, WB_OK,aText.replaceFirst("$1",aTemplateList)).Execute();
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateSearch ()
{
    Point aPos = maView->GetPosPixel();
    bool bVisible = mpSearchEdit->IsVisible();
    Size aWinSize = GetSizePixel();
    long nEditHeight = mpSearchEdit->GetSizePixel().getHeight();

    if (bVisible)
    {
        aWinSize.setHeight(aWinSize.getHeight() - nEditHeight );
        aPos.setY(aPos.getY() - nEditHeight );
        mpActionBar->SetItemState(TBI_TEMPLATE_SEARCH,STATE_NOCHECK);
    }
    else
    {
        aWinSize.setHeight(aWinSize.getHeight() + nEditHeight );
        aPos.setY(aPos.getY() + nEditHeight );
        mpActionBar->SetItemState(TBI_TEMPLATE_SEARCH,STATE_CHECK);
    }

    SetSizePixel(aWinSize);
    maView->SetPosPixel(aPos);
    mpOnlineView->SetPosPixel(aPos);
    mpSearchView->SetPosPixel(aPos);

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
    const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*maSelTemplates.begin());

    SfxTemplateInfoDlg aDlg;
    aDlg.loadDocument(pItem->getPath());
    aDlg.Execute();
}

void SfxTemplateManagerDlg::OnTemplateDelete ()
{
    OUString aTemplateList;

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
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + pItem->maTitle;

                ++pIter;
            }
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
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = (*pIter)->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + (*pIter)->maTitle;

                ++pIter;
            }
        }

        if (maSelTemplates.empty())
        {
            mpTemplateBar->SetItemDown(TBI_TEMPLATE_DELETE,false);
            mpTemplateBar->Show(false);
            mpViewBar->Show();
            mpActionBar->Show();
        }
    }

    if (!aTemplateList.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_TEMPLATE).toString() );
        ErrorBox(this, WB_OK,aMsg.replaceFirst("$1",aTemplateList)).Execute();
    }
}

void SfxTemplateManagerDlg::OnTemplateAsDefault ()
{
    assert(!maSelTemplates.empty());

    const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*(maSelTemplates.begin()));

    OUString aServiceName;
    if (lcl_getServiceName(pItem->getPath(),aServiceName))
    {
        SfxObjectFactory::SetStandardTemplate(aServiceName,pItem->getPath());

        createDefaultTemplateMenu();

        // clear selection and display view/action toolbars
        maView->deselectOverlayItem(pItem->mnId);
    }
}

void SfxTemplateManagerDlg::OnFolderDelete()
{
    QueryBox aQueryDlg(this, WB_YES_NO | WB_DEF_YES, SfxResId(STR_QMSG_SEL_FOLDER_DELETE).toString());

    if ( aQueryDlg.Execute() == RET_NO )
        return;

    OUString aFolderList;

    std::set<const ThumbnailViewItem*>::const_iterator pIter;
    for (pIter = maSelFolders.begin(); pIter != maSelFolders.end();)
    {
        if (maView->removeRegion((*pIter)->mnId))
            maSelFolders.erase(pIter++);
        else
        {
            if (aFolderList.isEmpty())
                aFolderList = (*pIter)->maTitle;
            else
                aFolderList = aFolderList + "\n" + (*pIter)->maTitle;

            ++pIter;
        }
    }

    if (maSelFolders.empty())
    {
        mpViewBar->HideItem(TBI_TEMPLATE_IMPORT);
        mpViewBar->HideItem(TBI_TEMPLATE_FOLDER_DEL);
    }

    if (!aFolderList.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_FOLDER).toString() );
        ErrorBox(this, WB_OK,aMsg.replaceFirst("$1",aFolderList)).Execute();
    }
}

void SfxTemplateManagerDlg::OnRepositoryDelete()
{
    if(mpOnlineView->deleteRepository(mpOnlineView->getOverlayRegionId()))
    {
        // close overlay and switch to local view
        switchMainView(true);

        createRepositoryMenu();
    }
}

void SfxTemplateManagerDlg::OnTemplateSaveAs()
{
    assert(m_xModel.is());

    if (!maView->isOverlayVisible() && maSelFolders.empty())
    {
        ErrorBox(this, WB_OK,SfxResId(STR_MSG_ERROR_SELECT_FOLDER).toString()).Execute();
        return;
    }

    InputDialog aDlg(SfxResId(STR_INPUT_TEMPLATE_NEW).toString(),this);

    if (aDlg.Execute())
    {
        OUString aName = aDlg.getEntryText();

        if (!aName.isEmpty())
        {
            OUString aFolderList;
            OUString aQMsg(SfxResId(STR_QMSG_TEMPLATE_OVERWRITE).toString());
            QueryBox aQueryDlg(this,WB_YES_NO | WB_DEF_YES, OUString());

            if (maView->isOverlayVisible())
            {
                sal_uInt16 nRegionItemId = maView->getOverlayRegionId()+1;

                if (!maView->isTemplateNameUnique(nRegionItemId,aName))
                {
                    aQMsg = aQMsg.replaceFirst("$1",aName);
                    aQueryDlg.SetMessText(aQMsg.replaceFirst("$2",maView->getOverlayName()));

                    if (aQueryDlg.Execute() == RET_NO)
                        return;
                }

                if (!maView->saveTemplateAs(nRegionItemId,m_xModel,aName))
                    aFolderList = maView->getOverlayName();
            }
            else
            {
                std::set<const ThumbnailViewItem*>::const_iterator pIter;
                for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
                {
                    TemplateLocalViewItem *pItem = (TemplateLocalViewItem*)(*pIter);

                    if (!maView->isTemplateNameUnique(pItem->mnId,aName))
                    {
                        OUString aDQMsg = aQMsg.replaceFirst("$1",aName);
                        aQueryDlg.SetMessText(aDQMsg.replaceFirst("$2",pItem->maTitle));

                        if (aQueryDlg.Execute() == RET_NO)
                            continue;
                    }

                    if (!maView->saveTemplateAs(pItem,m_xModel,aName))
                    {
                        if (aFolderList.isEmpty())
                            aFolderList = (*pIter)->maTitle;
                        else
                            aFolderList = aFolderList + "\n" + (*pIter)->maTitle;
                    }
                }
            }

            maView->reload();

            if (!aFolderList.isEmpty())
            {
            }
        }
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

void SfxTemplateManagerDlg::createDefaultTemplateMenu ()
{
    std::vector<OUString> aList = lcl_getAllFactoryURLs();

    if (!aList.empty())
    {
        mpTemplateDefaultMenu->Clear();

        sal_uInt16 nItemId = MNI_ACTION_DEFAULT + 1;
        for( std::vector<OUString>::const_iterator i = aList.begin(); i != aList.end(); ++i )
        {
            INetURLObject aObj(*i);
            OUString aTitle = SvFileInformationManager::GetDescription(aObj);
            mpTemplateDefaultMenu->InsertItem(nItemId, aTitle, SvFileInformationManager::GetImage(aObj, false));
            mpTemplateDefaultMenu->SetItemCommand(nItemId++, *i);
        }

        mpActionMenu->ShowItem(MNI_ACTION_DEFAULT);
    }
    else
        mpActionMenu->HideItem(MNI_ACTION_DEFAULT);
}

void SfxTemplateManagerDlg::switchMainView(bool bDisplayLocal)
{
    if (bDisplayLocal)
    {
        mpCurView = maView;

        mpViewBar->HideItem(TBI_TEMPLATE_FOLDER_DEL);

        // Enable deleting and exporting items from the filesystem
        mpTemplateBar->ShowItem(TBI_TEMPLATE_EXPORT);
        mpTemplateBar->ShowItem(TBI_TEMPLATE_DELETE);

        mpOnlineView->Hide();
        maView->Show();
    }
    else
    {
        mpCurView = mpOnlineView;

        mpViewBar->ShowItem(TBI_TEMPLATE_FOLDER_DEL);

        // Disable deleting and exporting items from remote repositories
        mpTemplateBar->HideItem(TBI_TEMPLATE_EXPORT);
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
            OUString aTemplateList;

            std::set<const ThumbnailViewItem*>::const_iterator pIter;
            for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = (*pIter)->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + (*pIter)->maTitle;
            }

            OUString aDst = maView->GetItemText(nItemId);
            OUString aMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE).toString());
            aMsg = aMsg.replaceFirst("$1",aDst);
            ErrorBox(this, WB_OK,aMsg.replaceFirst( "$2",aTemplateList)).Execute();
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
        OUString aTemplateList;

        std::set<const ThumbnailViewItem*>::const_iterator aIter;
        for (aIter = maSelTemplates.begin(); aIter != maSelTemplates.end(); ++aIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*aIter);

            if(!maView->copyFrom(nItemId,pItem->maPreview1,pItem->getPath()))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + pItem->maTitle;
            }
        }

        maView->Invalidate(INVALIDATE_NOERASE);

        if (!aTemplateList.isEmpty())
        {
            OUString aMsg(SfxResId(STR_MSG_ERROR_REMOTE_MOVE).toString());
            aMsg = aMsg.replaceFirst("$1",mpOnlineView->getOverlayName());
            aMsg = aMsg.replaceFirst("$2",maView->GetItemText(nItemId));
            ErrorBox(this,WB_OK,aMsg.replaceFirst("$1",aTemplateList)).Execute();
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
        OUString aTemplateList;

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
                if (aTemplateList.isEmpty())
                    aTemplateList = (*aIter)->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + (*aIter)->maTitle;
            }
        }

        if (!aTemplateList.isEmpty())
        {
            OUString aDst = maView->GetItemText(nItemId);
            OUString aMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE).toString());
            aMsg = aMsg.replaceFirst("$1",aDst);
            ErrorBox(this, WB_OK,aMsg.replaceFirst( "$2",aTemplateList)).Execute();
        }
    }

    // Deselect all items and update search results
    mpSearchView->deselectItems();

    SearchUpdateHdl(mpSearchEdit);
}

bool lcl_getServiceName ( const OUString &rFileURL, OUString &rName )
{
    bool bRet = false;

    if ( !rFileURL.isEmpty() )
    {
        try
        {
            uno::Reference< embed::XStorage > xStorage =
                    comphelper::OStorageHelper::GetStorageFromURL( rFileURL, embed::ElementModes::READ );

            sal_uIntPtr nFormat = SotStorage::GetFormatID( xStorage );

            const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( nFormat );

            if ( pFilter )
            {
                rName = pFilter->GetServiceName();
                bRet = true;
            }
        }
        catch( uno::Exception& )
        {}
    }

    return bRet;
}

std::vector<OUString> lcl_getAllFactoryURLs ()
{
    SvtModuleOptions aModOpt;
    std::vector<OUString> aList;
    const ::com::sun::star::uno::Sequence<OUString> &aServiceNames = aModOpt.GetAllServiceNames();

    for( sal_Int32 i=0, nCount = aServiceNames.getLength(); i < nCount; ++i )
    {
        if ( SfxObjectFactory::GetStandardTemplate( aServiceNames[i] ).Len() > 0 )
        {
            SvtModuleOptions::EFactory eFac = SvtModuleOptions::E_WRITER;
            SvtModuleOptions::ClassifyFactoryByName( aServiceNames[i], eFac );
            aList.push_back(aModOpt.GetFactoryEmptyDocumentURL(eFac));
        }
    }

    return aList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
