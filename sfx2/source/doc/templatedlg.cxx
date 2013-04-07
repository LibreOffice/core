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
#include <officecfg/Office/Common.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templateinfodlg.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateremoteview.hxx>
#include <sfx2/templaterepository.hxx>
#include <sfx2/templateviewitem.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <sot/storage.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/PlaceEditDialog.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>

#include "doc.hrc"
#include "templatedlg.hrc"

#define PADDING_DLG_BORDER      10

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::document;

static bool lcl_getServiceName (const OUString &rFileURL, OUString &rName );

static std::vector<OUString> lcl_getAllFactoryURLs ();

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

    SearchView_Keyword (const OUString &rKeyword, FILTER_APPLICATION App)
        : maKeyword(rKeyword), meApp(App)
    {}

    bool operator() (const TemplateItemProperties &rItem)
    {
        bool bRet = true;

        INetURLObject aUrl(rItem.aPath);
        OUString aExt = aUrl.getExtension();

        if (meApp == FILTER_APP_WRITER)
        {
            bRet = aExt == "ott" || aExt == "stw" || aExt == "oth" || aExt == "dot" || aExt == "dotx";
        }
        else if (meApp == FILTER_APP_CALC)
        {
            bRet = aExt == "ots" || aExt == "stc" || aExt == "xlt" || aExt == "xltm" || aExt == "xltx";
        }
        else if (meApp == FILTER_APP_IMPRESS)
        {
            bRet = aExt == "otp" || aExt == "sti" || aExt == "pot" || aExt == "potm" || aExt == "potx";
        }
        else if (meApp == FILTER_APP_DRAW)
        {
            bRet = aExt == "otg" || aExt == "std";
        }

        return bRet && rItem.aName.matchIgnoreAsciiCase(maKeyword);
    }

private:

    OUString maKeyword;
    FILTER_APPLICATION meApp;
};

/***
 *
 * Order items in ascending order (useful for the selection sets and move/copy operations since the associated ids
 * change when processed by the SfxDocumentTemplates class so we want to process to ones with higher id first)
 *
 ***/

static bool cmpSelectionItems (const ThumbnailViewItem *pItem1, const ThumbnailViewItem *pItem2)
{
    return pItem1->mnId > pItem2->mnId;
}

class TemplateManagerPage : public TabPage
{
    private:
        FixedText maFixedText;

    public:
        TemplateManagerPage( Window* pParent );
        ~TemplateManagerPage( ) { };
};

SfxTemplateManagerDlg::SfxTemplateManagerDlg (Window *parent)
    : ModelessDialog(parent, SfxResId(DLG_TEMPLATE_MANAGER)),
      maTabControl(this,SfxResId(TAB_CONTROL)),
      maTabPage(&maTabControl, SfxResId(TAB_TEMPLATE_MANAGER)),
      mpSearchEdit(new Edit(&maTabPage,WB_HIDE | WB_BORDER)),
      mpViewBar( new ToolBox(&maTabPage, SfxResId(TBX_ACTION_VIEW))),
      mpActionBar( new ToolBox(&maTabPage, SfxResId(TBX_ACTION_ACTION))),
      mpTemplateBar( new ToolBox(&maTabPage, SfxResId(TBX_ACTION_TEMPLATES))),
      mpSearchView(new TemplateSearchView(&maTabPage)),
      maView(new TemplateLocalView(&maTabPage,SfxResId(TEMPLATE_VIEW))),
      mpOnlineView(new TemplateRemoteView(&maTabPage, WB_VSCROLL,false)),
      maSelTemplates(cmpSelectionItems),
      maSelFolders(cmpSelectionItems),
      mbIsSaveMode(false),
      mxDesktop(comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.Desktop" ),uno::UNO_QUERY ),
      mbIsSynced(false),
      maRepositories()
{
    maTabControl.SetTabPage( FILTER_DOCS, &maTabPage );
    maTabControl.SetTabPage( FILTER_SHEETS, &maTabPage );
    maTabControl.SetTabPage( FILTER_PRESENTATIONS, &maTabPage );
    maTabControl.SetTabPage( FILTER_DRAWS, &maTabPage );

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
    mpSearchEdit->SetUpdateDataHdl(LINK(this,SfxTemplateManagerDlg,SearchUpdateHdl));
    mpSearchEdit->EnableUpdateData();

    maView->SetStyle(maView->GetStyle() | WB_VSCROLL);
    maView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    maView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_PADDING);

    maView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    maView->setOpenRegionHdl(LINK(this,SfxTemplateManagerDlg,OpenRegionHdl));
    maView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));

    // Set online view position and dimensions
    mpOnlineView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpOnlineView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mpOnlineView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mpOnlineView->setOpenRegionHdl(LINK(this,SfxTemplateManagerDlg,OpenRegionHdl));
    mpOnlineView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));

    mpSearchView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpSearchView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mpSearchView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));

    maTabControl.SetActivatePageHdl(LINK(this,SfxTemplateManagerDlg,ActivatePageHdl));

    SvtMiscOptions aMiscOptions;
    if ( !aMiscOptions.IsExperimentalMode() )
    {
        sal_uInt16 nPos = mpViewBar->GetItemId(TBI_TEMPLATE_REPOSITORY);
        mpViewBar->RemoveItem(nPos);
    }

    mpViewBar->Show();
    mpActionBar->Show();

    switchMainView(true);

    loadRepositories();

    createRepositoryMenu();
    createDefaultTemplateMenu();

    maView->Populate();
    maView->showRootRegion();
    maView->Show();

    mpCurView->filterItems(ViewFilter_Application(FILTER_APP_WRITER));

    FreeResource();
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg ()
{
    // Synchronize the config before deleting it
    syncRepositories();
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
        delete maRepositories[i];

    // Ignore view events since we are cleaning the object
    maView->setItemStateHdl(Link());
    maView->setOpenRegionHdl(Link());
    maView->setOpenTemplateHdl(Link());

    mpOnlineView->setItemStateHdl(Link());
    mpOnlineView->setOpenRegionHdl(Link());
    mpOnlineView->setOpenTemplateHdl(Link());

    mpSearchView->setItemStateHdl(Link());

    delete mpSearchEdit;
    delete mpViewBar;
    delete mpActionBar;
    delete mpTemplateBar;
    delete mpSearchView;
    delete maView;
    delete mpOnlineView;
    delete mpTemplateDefaultMenu;
    delete mpActionMenu;
    delete mpRepositoryMenu;
}

void SfxTemplateManagerDlg::setSaveMode(bool bMode)
{
    mbIsSaveMode = bMode;
    maTabControl.Clear();
    mpCurView->filterItems(ViewFilter_Application(FILTER_APP_NONE));

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

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ActivatePageHdl)
{
    FILTER_APPLICATION eFilter = FILTER_APP_NONE;
    switch (maTabControl.GetCurPageId())
    {
        case FILTER_DOCS:
            eFilter = FILTER_APP_WRITER;
            break;
        case FILTER_PRESENTATIONS:
            eFilter = FILTER_APP_IMPRESS;
            break;
        case FILTER_SHEETS:
            eFilter = FILTER_APP_CALC;
            break;
        case FILTER_DRAWS:
            eFilter = FILTER_APP_DRAW;
            break;
    }
    mpCurView->filterItems(ViewFilter_Application(eFilter));

    if (mpSearchView->IsVisible())
        SearchUpdateHdl(NULL);

    return 0;
}

void SfxTemplateManagerDlg::Resize()
{
    Size aWinSize = GetSizePixel();

    // Fit the tab page control and the toolbars
    Size aTabSize = maTabControl.GetSizePixel();
    aTabSize.setWidth(aWinSize.getWidth());
    maTabControl.SetSizePixel(aTabSize);
    maTabControl.SetTabPageSizePixel(aWinSize);

    // Calculate toolboxes size and positions
    Size aViewSize = mpViewBar->CalcMinimumWindowSizePixel();
    Size aActionSize = mpActionBar->CalcMinimumWindowSizePixel();
    Size aTemplateSize = mpTemplateBar->CalcMinimumWindowSizePixel();

    long nToolbarsHeight = std::max(std::max(aViewSize.getHeight(), aActionSize.getHeight()), aTemplateSize.getHeight());

    aActionSize.setWidth(3*aActionSize.getWidth());
    aViewSize.setWidth(aWinSize.getWidth()-aActionSize.getWidth()-mpViewBar->GetPosPixel().X());
    aTemplateSize.setWidth(aWinSize.getWidth());

    Point aActionPos = mpActionBar->GetPosPixel();
    aActionPos.setX(aWinSize.getWidth() - aActionSize.getWidth());

    mpViewBar->SetSizePixel(aViewSize);
    mpActionBar->SetPosSizePixel(aActionPos,aActionSize);
    mpTemplateBar->SetSizePixel(aTemplateSize);

    // Set view position below toolbox
    Point aViewPos = maView->GetPosPixel();
    aViewPos.setY(nToolbarsHeight);
    aViewPos.setX(0);
    Size aThumbSize(aWinSize.getWidth(), maTabControl.GetTabPageSizePixel().getHeight() - aViewPos.getY());
    maView->SetPosSizePixel(aViewPos, aThumbSize);

    if (aWinSize.getHeight() < aViewPos.getY() + aThumbSize.getHeight() + PADDING_DLG_BORDER)
        aWinSize.setHeight(aViewPos.getY() + aThumbSize.getHeight() + PADDING_DLG_BORDER);

    // Set search box position and size
    Size aSearchSize = mpSearchEdit->CalcMinimumSize();
    aSearchSize.setWidth(aWinSize.getWidth() - 2*PADDING_DLG_BORDER);

    mpSearchEdit->SetSizePixel(aSearchSize);
    mpSearchEdit->SetPosPixel(Point(PADDING_DLG_BORDER,aViewPos.Y()));

    maView->SetSizePixel(aThumbSize);
    mpOnlineView->SetPosSizePixel(aViewPos,aThumbSize);
    mpSearchView->SetSizePixel(aThumbSize);

    mpCurView->Resize();

    ModelessDialog::Resize();
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

        std::vector<OUString> aNames = maView->getFolderNames();

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

IMPL_LINK(SfxTemplateManagerDlg, TVItemStateHdl, const ThumbnailViewItem*, pItem)
{
    const TemplateContainerItem *pCntItem = dynamic_cast<const TemplateContainerItem*>(pItem);

    if (pCntItem)
        OnRegionState(pItem);
    else
        OnTemplateState(pItem);

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, MenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_ACTION_SORT_NAME:
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

            if (insertRepository(pPlace->GetName(),pPlace->GetUrl()))
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

        TemplateRepository *pRepository = NULL;

        for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
        {
            if (maRepositories[i]->mnId == nRepoId)
            {
                pRepository = maRepositories[i];
                break;
            }
        }

        if (mpOnlineView->loadRepository(pRepository,false))
            switchMainView(false);
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

IMPL_LINK_NOARG(SfxTemplateManagerDlg, OpenRegionHdl)
{
    maSelFolders.clear();
    maSelTemplates.clear();

    mpViewBar->ShowItem(TBI_TEMPLATE_IMPORT,mpCurView->isImportAllowed());

    mpTemplateBar->Hide();
    mpViewBar->Show();
    mpActionBar->Show();

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem)
{
    if (!mbIsSaveMode)
    {
        uno::Sequence< PropertyValue > aArgs(3);
        aArgs[0].Name = "AsTemplate";
        aArgs[0].Value <<= sal_True;
        aArgs[1].Name = "MacroExecutionMode";
        aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
        aArgs[2].Name = "UpdateDocMode";
        aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;

        TemplateViewItem *pTemplateItem = static_cast<TemplateViewItem*>(pItem);

        try
        {
            mxDesktop->loadComponentFromURL(pTemplateItem->getPath(),"_default", 0, aArgs );
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
    OUString aKeyword = mpSearchEdit->GetText();

    if (!aKeyword.isEmpty())
    {
        mpSearchView->Clear();

        // if the search view is hidden, hide the folder view and display search one
        if (!mpSearchView->IsVisible())
        {
            mpSearchView->Show();
            mpCurView->Hide();
        }

        bool bDisplayFolder = !mpCurView->isNonRootRegionVisible();

        FILTER_APPLICATION eFilter = FILTER_APP_NONE;
        switch (maTabControl.GetCurPageId())
        {
            case FILTER_DOCS:
                eFilter = FILTER_APP_WRITER;
                break;
            case FILTER_PRESENTATIONS:
                eFilter = FILTER_APP_IMPRESS;
                break;
            case FILTER_SHEETS:
                eFilter = FILTER_APP_CALC;
                break;
            case FILTER_DRAWS:
                eFilter = FILTER_APP_DRAW;
                break;
        }

        std::vector<TemplateItemProperties> aItems =
                maView->getFilteredItems(SearchView_Keyword(aKeyword,eFilter));

        for (size_t i = 0; i < aItems.size(); ++i)
        {
            TemplateItemProperties *pItem = &aItems[i];

            OUString aFolderName;

            if (bDisplayFolder)
                aFolderName = maView->getRegionName(pItem->nRegionId);

            mpSearchView->AppendItem(pItem->nId,maView->getRegionId(pItem->nRegionId),
                                     pItem->nDocId,
                                     pItem->aName,
                                     aFolderName,
                                     pItem->aPath,
                                     pItem->aThumbnail);
        }

        mpSearchView->Invalidate();
    }
    else
    {
        mpSearchView->Hide();
        mpCurView->Show();
    }

    return 0;
}

void SfxTemplateManagerDlg::OnRegionState (const ThumbnailViewItem *pItem)
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
}

void SfxTemplateManagerDlg::OnTemplateState (const ThumbnailViewItem *pItem)
{
    bool bInSelection = maSelTemplates.find(pItem) != maSelTemplates.end();

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
            else if (maSelTemplates.size() != 1 || !bInSelection)
            {
                mpTemplateBar->HideItem(TBI_TEMPLATE_EDIT);
                mpTemplateBar->HideItem(TBI_TEMPLATE_PROPERTIES);
                mpTemplateBar->HideItem(TBI_TEMPLATE_DEFAULT);
            }
        }

        if (!bInSelection)
            maSelTemplates.insert(pItem);
    }
    else
    {
        if (bInSelection)
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
    OUString sFilterExt;
    OUString sFilterName( SfxResId( STR_TEMPLATE_FILTER ).toString() );

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
        com::sun::star::uno::Sequence<OUString> aFiles = aFileDlg.GetSelectedFiles();

        if (aFiles.hasElements())
        {
            std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
            for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
            {
                OUString aTemplateList;
                TemplateContainerItem *pFolder = (TemplateContainerItem*)(*pIter);

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
    uno::Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<XFolderPicker2> xFolderPicker = FolderPicker::create(xContext);

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

            std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter = maSelTemplates.begin();
            for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end(); ++pIter, ++i)
            {
                const TemplateSearchViewItem *pItem = static_cast<const TemplateSearchViewItem*>(*pIter);

                INetURLObject aItemPath(pItem->getPath());

                if ( 1 == i )
                    aPathObj.Append(aItemPath.getName());
                else
                    aPathObj.setName(aItemPath.getName());

                OUString aPath = aPathObj.GetMainURL( INetURLObject::NO_DECODE );

                if (!maView->exportTo(pItem->mnAssocId,pItem->mnRegionId,aPath))
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
            // export templates from the current view

            sal_uInt16 i = 1;
            sal_uInt16 nRegionItemId = maView->getCurRegionItemId();

            std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter = maSelTemplates.begin();
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

            maView->deselectItems();
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

//    SetSizePixel(aWinSize);
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
    mpSearchEdit->SetText(OUString());
    if (!bVisible)
        mpSearchEdit->GrabFocus();

    // display all templates if we hide the search bar
    if (bVisible && mpCurView->isNonRootRegionVisible())
        mpCurView->filterItems(ViewFilter_Application(FILTER_APP_NONE));
}

void SfxTemplateManagerDlg::OnTemplateEdit ()
{
    uno::Sequence< PropertyValue > aArgs(3);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= sal_False;
    aArgs[1].Name = "MacroExecutionMode";
    aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
    aArgs[2].Name = "UpdateDocMode";
    aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;

    uno::Reference< XStorable > xStorable;
    std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
    for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end(); ++pIter)
    {
        const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*pIter);

        try
        {
            xStorable = uno::Reference< XStorable >(
                        mxDesktop->loadComponentFromURL(pItem->getPath(),"_default", 0, aArgs ),
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
        std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates; //Avoids invalid iterators

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
        for (pIter = aSelTemplates.begin(); pIter != aSelTemplates.end(); ++pIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*pIter);

            if (!maView->removeTemplate(pItem->mnAssocId,pItem->mnRegionId))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + pItem->maTitle;
            }
            else
                mpSearchView->RemoveItem(pItem->mnId);
        }
    }
    else
    {
        sal_uInt16 nRegionItemId = maView->getCurRegionItemId();
        std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates;  //Avoid invalid iterators

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
        for (pIter = aSelTemplates.begin(); pIter != aSelTemplates.end(); ++pIter)
        {
            if (!maView->removeTemplate((*pIter)->mnId,nRegionItemId))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = (*pIter)->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + (*pIter)->maTitle;
            }
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
    if (!maSelTemplates.empty())
    {
        const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*(maSelTemplates.begin()));

        OUString aServiceName;
        if (lcl_getServiceName(pItem->getPath(),aServiceName))
        {
            SfxObjectFactory::SetStandardTemplate(aServiceName,pItem->getPath());

            createDefaultTemplateMenu();
        }
    }
}

void SfxTemplateManagerDlg::OnFolderDelete()
{
    QueryBox aQueryDlg(this, WB_YES_NO | WB_DEF_YES, SfxResId(STR_QMSG_SEL_FOLDER_DELETE).toString());

    if ( aQueryDlg.Execute() == RET_NO )
        return;

    OUString aFolderList;

    std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
    std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelFolders = maSelFolders; //Copy to avoid invalidating an iterator

    for (pIter = aSelFolders.begin(); pIter != aSelFolders.end(); ++pIter)
    {
        if (!maView->removeRegion((*pIter)->mnId))
        {
            if (aFolderList.isEmpty())
                aFolderList = (*pIter)->maTitle;
            else
                aFolderList = aFolderList + "\n" + (*pIter)->maTitle;

            ++pIter;
        }
    }

    if (!aFolderList.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_FOLDER).toString() );
        ErrorBox(this, WB_OK,aMsg.replaceFirst("$1",aFolderList)).Execute();
    }
}

void SfxTemplateManagerDlg::OnRepositoryDelete()
{
    if(deleteRepository(mpOnlineView->getCurRegionId()))
    {
        // switch to local view
        switchMainView(true);

        createRepositoryMenu();
    }
}

void SfxTemplateManagerDlg::OnTemplateSaveAs()
{
    assert(m_xModel.is());

    if (!maView->isNonRootRegionVisible() && maSelFolders.empty())
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

            if (maView->isNonRootRegionVisible())
            {
                sal_uInt16 nRegionItemId = maView->getRegionId(maView->getCurRegionId()-1);

                if (!maView->isTemplateNameUnique(nRegionItemId,aName))
                {
                    aQMsg = aQMsg.replaceFirst("$1",aName);
                    aQueryDlg.SetMessText(aQMsg.replaceFirst("$2",maView->getCurRegionName()));

                    if (aQueryDlg.Execute() == RET_NO)
                        return;
                }

                if (!maView->saveTemplateAs(nRegionItemId,m_xModel,aName))
                    aFolderList = maView->getCurRegionName();
            }
            else
            {
                std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
                for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
                {
                    TemplateContainerItem *pItem = (TemplateContainerItem*)(*pIter);

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

void SfxTemplateManagerDlg::createRepositoryMenu()
{
    mpRepositoryMenu->Clear();

    mpRepositoryMenu->InsertItem(MNI_REPOSITORY_LOCAL,SfxResId(STR_REPOSITORY_LOCAL).toString());

    const std::vector<TemplateRepository*> &rRepos = getRepositories();

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
            OUString aName = dlg.getEntryText();

            if (!aName.isEmpty())
                nItemId = maView->createRegion(aName);
        }
    }
    else
    {
        nItemId = maView->getRegionId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        if (!maView->moveTemplates(maSelTemplates,nItemId))
        {
            OUString aTemplateList;

            std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
            for (pIter = maSelTemplates.begin(); pIter != maSelTemplates.end(); ++pIter)
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
            OUString aName = dlg.getEntryText();

            if (!aName.isEmpty())
                nItemId = maView->createRegion(aName);
        }
    }
    else
    {
        nItemId = maView->getRegionId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        OUString aTemplateList;

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aIter;
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
            aMsg = aMsg.replaceFirst("$1",mpOnlineView->getCurRegionName());
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
            OUString aName = dlg.getEntryText();

            if (!aName.isEmpty())
                nItemId = maView->createRegion(aName);
        }
    }
    else
    {
        nItemId = maView->getRegionId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        OUString aTemplateList;

        // Move templates to desired folder if for some reason move fails
        // try copying them.
        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aIter;
        std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates; //Copy to avoid invalidating an iterator

        for (aIter = aSelTemplates.begin(); aIter != aSelTemplates.end(); ++aIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*aIter);

            if(!maView->moveTemplate(pItem,pItem->mnRegionId,nItemId))
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

void SfxTemplateManagerDlg::loadRepositories()
{
    uno::Reference < uno::XComponentContext > m_context(comphelper::getProcessComponentContext());

    // Load from user settings
    com::sun::star::uno::Sequence<OUString>  aUrls =
            officecfg::Office::Common::Misc::TemplateRepositoryUrls::get(m_context);

    com::sun::star::uno::Sequence<OUString> aNames =
            officecfg::Office::Common::Misc::TemplateRepositoryNames::get(m_context);

    for (sal_Int32 i = 0; i < aUrls.getLength() && i < aNames.getLength(); ++i)
    {
        TemplateRepository *pItem = new TemplateRepository();

        pItem->mnId = i+1;
        pItem->maTitle = aNames[i];
        pItem->setURL(aUrls[i]);

        maRepositories.push_back(pItem);
    }
}

bool SfxTemplateManagerDlg::insertRepository(const OUString &rName, const OUString &rURL)
{
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
    {
        if (maRepositories[i]->maTitle == rName)
            return false;
    }

    TemplateRepository *pItem = new TemplateRepository();

    pItem->mnId = maRepositories.size()+1;
    pItem->maTitle = rName;
    pItem->setURL(rURL);

    maRepositories.push_back(pItem);

    mbIsSynced = false;
    return true;
}

bool SfxTemplateManagerDlg::deleteRepository(const sal_uInt16 nRepositoryId)
{
    bool bRet = false;

    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
    {
        if (maRepositories[i]->mnId == nRepositoryId)
        {
            delete maRepositories[i];

            maRepositories.erase(maRepositories.begin() + i);
            mbIsSynced = false;
            bRet = true;
            break;
        }
    }

    return bRet;
}

void SfxTemplateManagerDlg::syncRepositories() const
{
    if (!mbIsSynced)
    {
        uno::Reference < uno::XComponentContext > pContext(comphelper::getProcessComponentContext());
        boost::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(pContext));

        size_t nSize = maRepositories.size();
        uno::Sequence<OUString> aUrls(nSize);
        uno::Sequence<OUString> aNames(nSize);

        for(size_t i = 0; i < nSize; ++i)
        {
            aUrls[i] = maRepositories[i]->getURL();
            aNames[i] = maRepositories[i]->maTitle;
        }

        officecfg::Office::Common::Misc::TemplateRepositoryUrls::set(aUrls, batch, pContext);
        officecfg::Office::Common::Misc::TemplateRepositoryNames::set(aNames, batch, pContext);
        batch->commit();
    }
}

static bool lcl_getServiceName ( const OUString &rFileURL, OUString &rName )
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

static std::vector<OUString> lcl_getAllFactoryURLs ()
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
