/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatedlg.hxx>

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
#include <svtools/langhelp.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/PlaceEditDialog.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

#include "doc.hrc"

const char TM_SETTING_MANAGER[] = "TemplateManager";
const char TM_SETTING_LASTFOLDER[] = "LastFolder";
const char TM_SETTING_FILTER[] = "SelectedFilter";

const char SERVICENAME_CFGREADACCESS[] = "com.sun.star.configuration.ConfigurationAccess";

const char VIEWBAR_REPOSITORY[] = "repository";
const char VIEWBAR_IMPORT[] = "import";
const char VIEWBAR_DELETE[] = "delete";
const char VIEWBAR_SAVE[] = "save";
const char VIEWBAR_NEW_FOLDER[] = "new_folder";
const char TEMPLATEBAR_SAVE[] = "template_save";
const char TEMPLATEBAR_OPEN[] = "open";
const char TEMPLATEBAR_EDIT[] = "edit";
const char TEMPLATEBAR_PROPERTIES[] = "properties";
const char TEMPLATEBAR_DEFAULT[] = "default";
const char TEMPLATEBAR_MOVE[] = "move";
const char TEMPLATEBAR_EXPORT[] = "export";
const char TEMPLATEBAR_DELETE[] = "template_delete";
const char ACTIONBAR_SEARCH[] = "search";
const char ACTIONBAR_ACTION[] = "action_menu";
const char ACTIONBAR_TEMPLATE[] = "template_link";
const char FILTER_DOCS[] = "filter_docs";
const char FILTER_SHEETS[] = "filter_sheets";
const char FILTER_PRESENTATIONS[] = "filter_presentations";
const char FILTER_DRAWINGS[] = "filter_draws";

#define MNI_ACTION_SORT_NAME 1
#define MNI_ACTION_REFRESH   2
#define MNI_ACTION_DEFAULT   3
#define MNI_MOVE_NEW         1
#define MNI_MOVE_FOLDER_BASE 2
#define MNI_REPOSITORY_LOCAL 1
#define MNI_REPOSITORY_NEW   2
#define MNI_REPOSITORY_BASE  3

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

        if (meApp == FILTER_APPLICATION::WRITER)
        {
            bRet = aExt == "ott" || aExt == "stw" || aExt == "oth" || aExt == "dot" || aExt == "dotx";
        }
        else if (meApp == FILTER_APPLICATION::CALC)
        {
            bRet = aExt == "ots" || aExt == "stc" || aExt == "xlt" || aExt == "xltm" || aExt == "xltx";
        }
        else if (meApp == FILTER_APPLICATION::IMPRESS)
        {
            bRet = aExt == "otp" || aExt == "sti" || aExt == "pot" || aExt == "potm" || aExt == "potx";
        }
        else if (meApp == FILTER_APPLICATION::DRAW)
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

SfxTemplateManagerDlg::SfxTemplateManagerDlg(vcl::Window *parent)
    : ModalDialog(parent, "TemplateDialog", "sfx/ui/templatedlg.ui"),
      maSelTemplates(cmpSelectionItems),
      maSelFolders(cmpSelectionItems),
      mbIsSaveMode(false),
      mxDesktop( Desktop::create(comphelper::getProcessComponentContext()) ),
      mbIsSynced(false),
      maRepositories()
{
    get(mpTabControl, "tab_control");
    get(mpSearchEdit, "search_edit");
    get(mpViewBar, "action_view");
    get(mpActionBar, "action_action");
    get(mpTemplateBar, "action_templates");
    get(mpLocalView, "template_view");
    get(mpSearchView, "search_view");
    get(mpRemoteView, "remote_view");

    TabPage *pTabPage = mpTabControl->GetTabPage(mpTabControl->GetPageId("filter_docs"));
    pTabPage->Show();
    mpTabControl->SetTabPage(mpTabControl->GetPageId("filter_sheets"), pTabPage);
    pTabPage->Show();
    mpTabControl->SetTabPage(mpTabControl->GetPageId("filter_presentations"), pTabPage);
    pTabPage->Show();
    mpTabControl->SetTabPage(mpTabControl->GetPageId("filter_draws"), pTabPage);
    pTabPage->Show();

    // Create popup menus
    mpActionMenu = new PopupMenu;
    mpActionMenu->InsertItem(MNI_ACTION_SORT_NAME,
        SfxResId(STR_ACTION_SORT_NAME).toString(),
        Image(SfxResId(IMG_ACTION_SORT)));
    mpActionMenu->InsertItem(MNI_ACTION_REFRESH,
        SfxResId(STR_ACTION_REFRESH).toString(),
        Image(SfxResId(IMG_ACTION_REFRESH)));
    mpActionMenu->InsertItem(MNI_ACTION_DEFAULT,SfxResId(STR_ACTION_DEFAULT).toString());
    mpActionMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,MenuSelectHdl));

    mpRepositoryMenu = new PopupMenu;
    mpRepositoryMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,RepositoryMenuSelectHdl));

    mpTemplateDefaultMenu = new PopupMenu;
    mpTemplateDefaultMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,DefaultTemplateMenuSelectHdl));
    mpActionMenu->SetPopupMenu(MNI_ACTION_DEFAULT,mpTemplateDefaultMenu);

    // Set toolbox styles
    mpViewBar->SetButtonType(ButtonType::SYMBOLTEXT);
    mpTemplateBar->SetButtonType(ButtonType::SYMBOLTEXT);

    // Set toolbox button bits
    mpViewBar->SetItemBits(mpViewBar->GetItemId(VIEWBAR_REPOSITORY), ToolBoxItemBits::DROPDOWNONLY);
    mpActionBar->SetItemBits(mpActionBar->GetItemId(ACTIONBAR_ACTION), ToolBoxItemBits::DROPDOWNONLY);
    mpTemplateBar->SetItemBits(mpTemplateBar->GetItemId(TEMPLATEBAR_MOVE), ToolBoxItemBits::DROPDOWNONLY);

    // Set toolbox handlers
    mpViewBar->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,TBXViewHdl));
    mpViewBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));
    mpActionBar->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,TBXActionHdl));
    mpActionBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));
    mpTemplateBar->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,TBXTemplateHdl));
    mpTemplateBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));
    mpSearchEdit->SetUpdateDataHdl(LINK(this,SfxTemplateManagerDlg,SearchUpdateHdl));
    mpSearchEdit->EnableUpdateData();

    mpLocalView->SetStyle(mpLocalView->GetStyle() | WB_VSCROLL);
    mpLocalView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpLocalView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_PADDING);

    mpLocalView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mpLocalView->setOpenRegionHdl(LINK(this,SfxTemplateManagerDlg,OpenRegionHdl));
    mpLocalView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));

    // Set online view position and dimensions
    mpRemoteView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpRemoteView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mpRemoteView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mpRemoteView->setOpenRegionHdl(LINK(this,SfxTemplateManagerDlg,OpenRegionHdl));
    mpRemoteView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));

    mpSearchView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpSearchView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT_SUB-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mpSearchView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mpSearchView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));

    mpTabControl->SetActivatePageHdl(LINK(this, SfxTemplateManagerDlg, ActivatePageHdl));

    SvtMiscOptions aMiscOptions;
    if ( !aMiscOptions.IsExperimentalMode() )
    {
        sal_uInt16 nPos = mpViewBar->GetItemPos(mpViewBar->GetItemId(VIEWBAR_REPOSITORY));
        mpViewBar->RemoveItem(nPos);
    }

    mpViewBar->Show();
    mpActionBar->Show();

    switchMainView(true);

    loadRepositories();

    createRepositoryMenu();
    createDefaultTemplateMenu();

    mpLocalView->Populate();
    mpCurView->filterItems(ViewFilter_Application(FILTER_APPLICATION::WRITER));

    readSettings();

    mpLocalView->Show();
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg()
{
    disposeOnce();
}

void SfxTemplateManagerDlg::dispose()
{
    writeSettings();

    // Synchronize the config before deleting it
    syncRepositories();
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
        delete maRepositories[i];

    // Ignore view events since we are cleaning the object
    mpLocalView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpLocalView->setOpenRegionHdl(Link<void*,void>());
    mpLocalView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());

    mpRemoteView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpRemoteView->setOpenRegionHdl(Link<void*,void>());
    mpRemoteView->setOpenTemplateHdl(Link<ThumbnailViewItem*,void>());

    mpSearchView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpSearchView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());

    mpTabControl.clear();
    mpSearchEdit.clear();
    mpViewBar.clear();
    mpActionBar.clear();
    mpTemplateBar.clear();
    mpSearchView.clear();
    mpCurView.clear();
    mpLocalView.clear();
    mpRemoteView.clear();
    ModalDialog::dispose();
}

void SfxTemplateManagerDlg::setSaveMode()
{
    mbIsSaveMode = true;

    // FIXME We used to call just mpTabControl->Clear() here; but that worked
    // only with .src dialogs, as the tab pages could have existed even
    // without TabControl containing them.  This is not possible with .ui
    // definitions any more (and rightly so!), so leave just one tab here for
    // now, until we do a bigger Template manager rework.
    while (mpTabControl->GetPageCount() > 1)
        mpTabControl->RemovePage(mpTabControl->GetPageId(1));

    mpCurView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));

    mpViewBar->ShowItem(VIEWBAR_SAVE);
    mpViewBar->HideItem(VIEWBAR_IMPORT);
    mpViewBar->HideItem(VIEWBAR_REPOSITORY);

    mpTemplateBar->ShowItem(TEMPLATEBAR_SAVE);
    mpTemplateBar->ShowItem(TEMPLATEBAR_PROPERTIES);
    mpTemplateBar->ShowItem(TEMPLATEBAR_DEFAULT);
    mpTemplateBar->HideItem(TEMPLATEBAR_OPEN);
    mpTemplateBar->HideItem(TEMPLATEBAR_EDIT);
    mpTemplateBar->HideItem(TEMPLATEBAR_MOVE);
    mpTemplateBar->HideItem(TEMPLATEBAR_EXPORT);
    mpTemplateBar->HideItem(TEMPLATEBAR_DELETE);
}

void SfxTemplateManagerDlg::setDocumentModel(const uno::Reference<frame::XModel> &rModel)
{
    m_xModel = rModel;
}

FILTER_APPLICATION SfxTemplateManagerDlg::getCurrentFilter()
{
    const sal_uInt16 nCurPageId = mpTabControl->GetCurPageId();

    if (nCurPageId == mpTabControl->GetPageId(FILTER_DOCS))
        return FILTER_APPLICATION::WRITER;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_PRESENTATIONS))
        return FILTER_APPLICATION::IMPRESS;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_SHEETS))
        return FILTER_APPLICATION::CALC;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_DRAWINGS))
        return FILTER_APPLICATION::DRAW;

    return FILTER_APPLICATION::NONE;
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, ActivatePageHdl, TabControl*, void)
{
    mpCurView->filterItems(ViewFilter_Application(getCurrentFilter()));
    mpCurView->showRootRegion(); // fdo#60586 show the root region of the applied filter

    if (mpSearchView->IsVisible())
        SearchUpdateHdl(*mpSearchEdit);
}

void SfxTemplateManagerDlg::readSettings ()
{
    OUString aLastFolder;
    sal_uInt16 nPageId = 0;
    SvtViewOptions aViewSettings( E_DIALOG, TM_SETTING_MANAGER );

    if ( aViewSettings.Exists() )
    {
        sal_uInt16 nTmp = 0;
        aViewSettings.GetUserItem(TM_SETTING_LASTFOLDER) >>= aLastFolder;
        aViewSettings.GetUserItem(TM_SETTING_FILTER) >>= nTmp;
        FILTER_APPLICATION nFilter = static_cast<FILTER_APPLICATION>(nTmp);
        switch (nFilter)
        {
            case FILTER_APPLICATION::WRITER:
                nPageId = mpTabControl->GetPageId(FILTER_DOCS);
                break;
            case FILTER_APPLICATION::IMPRESS:
                nPageId = mpTabControl->GetPageId(FILTER_PRESENTATIONS);
                break;
            case FILTER_APPLICATION::CALC:
                nPageId = mpTabControl->GetPageId(FILTER_SHEETS);
                break;
            case FILTER_APPLICATION::DRAW:
                nPageId = mpTabControl->GetPageId(FILTER_DRAWINGS);
                break;
            default: break;
        }
    }

    if (!aLastFolder.getLength())
        mpLocalView->showRootRegion();
    else
        mpLocalView->showRegion(aLastFolder);

    mpTabControl->SelectTabPage(nPageId);
}

void SfxTemplateManagerDlg::writeSettings ()
{
    OUString aLastFolder;

    if (mpCurView == mpLocalView && mpLocalView->getCurRegionId())
        aLastFolder = mpLocalView->getRegionName(mpLocalView->getCurRegionId()-1);

    // last folder
    Sequence< NamedValue > aSettings
    {
        { TM_SETTING_LASTFOLDER, css::uno::makeAny(aLastFolder) },
        { TM_SETTING_FILTER,     css::uno::makeAny(sal_uInt16(getCurrentFilter())) }
    };

    // write
    SvtViewOptions aViewSettings(E_DIALOG, TM_SETTING_MANAGER);
    aViewSettings.SetUserData(aSettings);
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg,TBXViewHdl, ToolBox *, void)
{
    const sal_uInt16 nCurItemId = mpViewBar->GetCurItemId();

    if (nCurItemId == mpViewBar->GetItemId(VIEWBAR_IMPORT))
        OnTemplateImport();
    else if (nCurItemId == mpViewBar->GetItemId(VIEWBAR_DELETE))
    {
        if (mpCurView == mpLocalView)
            OnFolderDelete();
        else
            OnRepositoryDelete();
    }
    else if (nCurItemId == mpViewBar->GetItemId(VIEWBAR_NEW_FOLDER))
        OnFolderNew();
    else if (nCurItemId == mpViewBar->GetItemId(VIEWBAR_SAVE))
        OnTemplateSaveAs();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, TBXActionHdl, ToolBox *, void)
{
    const sal_uInt16 nCurItemId = mpActionBar->GetCurItemId();

    if (nCurItemId == mpActionBar->GetItemId(ACTIONBAR_SEARCH))
        OnTemplateSearch();
    else if (nCurItemId == mpActionBar->GetItemId(ACTIONBAR_TEMPLATE))
        OnTemplateLink();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, TBXTemplateHdl, ToolBox *, void)
{
    const sal_uInt16 nCurItemId = mpTemplateBar->GetCurItemId();

    if (nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_OPEN))
        OnTemplateOpen();
    else if (nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_EDIT))
        OnTemplateEdit();
    else if (nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_PROPERTIES))
        OnTemplateProperties();
    else if (nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_DELETE))
        OnTemplateDelete();
    else if (nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_DEFAULT))
        OnTemplateAsDefault();
    else if (nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_EXPORT))
        OnTemplateExport();
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, TBXDropdownHdl, ToolBox*, pBox, void)
{
    const sal_uInt16 nCurItemId = pBox->GetCurItemId();

    if (pBox == mpActionBar && nCurItemId == mpActionBar->GetItemId(ACTIONBAR_ACTION))
    {
        pBox->SetItemDown( nCurItemId, true );

        mpActionMenu->Execute(pBox, pBox->GetItemRect(nCurItemId), PopupMenuFlags::ExecuteDown);

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
    }
    else if (pBox == mpTemplateBar && nCurItemId == mpTemplateBar->GetItemId(TEMPLATEBAR_MOVE))
    {
        pBox->SetItemDown( nCurItemId, true );

        std::vector<OUString> aNames = mpLocalView->getFolderNames();

        PopupMenu *pMoveMenu = new PopupMenu;
        pMoveMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,MoveMenuSelectHdl));

        if (!aNames.empty())
        {
            for (size_t i = 0, n = aNames.size(); i < n; ++i)
                pMoveMenu->InsertItem(MNI_MOVE_FOLDER_BASE+i,aNames[i]);
        }

        pMoveMenu->InsertSeparator();

        pMoveMenu->InsertItem(MNI_MOVE_NEW, SfxResId(STR_MOVE_NEW));

        pMoveMenu->Execute(pBox, pBox->GetItemRect(nCurItemId), PopupMenuFlags::ExecuteDown);

        delete pMoveMenu;

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
    }
    else if (pBox == mpViewBar && nCurItemId == mpViewBar->GetItemId(VIEWBAR_REPOSITORY))
    {
        pBox->SetItemDown( nCurItemId, true );

        mpRepositoryMenu->Execute(pBox, pBox->GetItemRect(nCurItemId), PopupMenuFlags::ExecuteDown);

        pBox->SetItemDown( nCurItemId, false );
        pBox->EndSelection();
        pBox->Invalidate();
    }
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, TVItemStateHdl, const ThumbnailViewItem*, pItem, void)
{
    const TemplateContainerItem *pCntItem = dynamic_cast<const TemplateContainerItem*>(pItem);

    if (pCntItem)
        OnRegionState(pItem);
    else
        OnTemplateState(pItem);
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, MenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_ACTION_SORT_NAME:
        mpLocalView->sortItems(SortView_Name());
        break;
    case MNI_ACTION_REFRESH:
        mpCurView->reload();
        break;
    default:
        break;
    }

    return false;
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, MoveMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    if (mpSearchView->IsVisible())
    {
        // Check if we are searching the local or remote templates
        if (mpCurView == mpLocalView)
            localSearchMoveTo(nMenuId);
    }
    else
    {
        // Check if we are displaying the local or remote templates
        if (mpCurView == mpLocalView)
            localMoveTo(nMenuId);
        else
            remoteMoveTo(nMenuId);
    }

    return false;
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, RepositoryMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    if (nMenuId == MNI_REPOSITORY_LOCAL)
    {
        switchMainView(true);
    }
    else if (nMenuId == MNI_REPOSITORY_NEW)
    {
        ScopedVclPtrInstance< PlaceEditDialog > dlg(this);

        if (dlg->Execute())
        {
            std::shared_ptr<Place> xPlace = dlg->GetPlace();

            if (insertRepository(xPlace->GetName(), xPlace->GetUrl()))
            {
                // update repository list menu.
                createRepositoryMenu();
            }
            else
            {
                OUString aMsg(SfxResId(STR_MSG_ERROR_REPOSITORY_NAME).toString());
                aMsg = aMsg.replaceFirst("$1", xPlace->GetName());
                ScopedVclPtrInstance<MessageDialog>(this, aMsg)->Execute();
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

        if (mpRemoteView->loadRepository(pRepository,false))
            switchMainView(false);
    }

    return false;
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, DefaultTemplateMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    OUString aServiceName = SfxObjectShell::GetServiceNameFromFactory( mpTemplateDefaultMenu->GetItemCommand(nId));
    SfxObjectFactory::SetStandardTemplate( aServiceName, OUString() );

    createDefaultTemplateMenu();

    return false;
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, OpenRegionHdl, void*, void)
{
    maSelFolders.clear();
    maSelTemplates.clear();

    mpViewBar->ShowItem(VIEWBAR_NEW_FOLDER, mpCurView->isNestedRegionAllowed());

    if (!mbIsSaveMode)
        mpViewBar->ShowItem(VIEWBAR_IMPORT, mpCurView->isImportAllowed());

    mpTemplateBar->Hide();
    mpViewBar->Show();
    mpActionBar->Show();
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    if (!mbIsSaveMode)
    {
        uno::Sequence< PropertyValue > aArgs(4);
        aArgs[0].Name = "AsTemplate";
        aArgs[0].Value <<= sal_True;
        aArgs[1].Name = "MacroExecutionMode";
        aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
        aArgs[2].Name = "UpdateDocMode";
        aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;
        aArgs[3].Name = "InteractionHandler";
        aArgs[3].Value <<= task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), 0 );

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
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, SearchUpdateHdl, Edit&, void)
{
    OUString aKeyword = mpSearchEdit->GetText();

    if (!aKeyword.isEmpty())
    {
        mpSearchView->Clear();

        // if the search view is hidden, hide the folder view and display search one
        if (!mpSearchView->IsVisible())
        {
            mpCurView->deselectItems();
            mpSearchView->Show();
            mpCurView->Hide();
        }

        bool bDisplayFolder = !mpCurView->isNonRootRegionVisible();

        std::vector<TemplateItemProperties> aItems =
                mpLocalView->getFilteredItems(SearchView_Keyword(aKeyword, getCurrentFilter()));

        for (size_t i = 0; i < aItems.size(); ++i)
        {
            TemplateItemProperties *pItem = &aItems[i];

            OUString aFolderName;

            if (bDisplayFolder)
                aFolderName = mpLocalView->getRegionName(pItem->nRegionId);

            mpSearchView->AppendItem(pItem->nId,mpLocalView->getRegionId(pItem->nRegionId),
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
        mpSearchView->deselectItems();
        mpSearchView->Hide();
        mpCurView->Show();
    }
}

void SfxTemplateManagerDlg::OnRegionState (const ThumbnailViewItem *pItem)
{
    if (pItem->isSelected())
    {
        if (maSelFolders.empty() && !mbIsSaveMode)
        {
            mpViewBar->ShowItem(VIEWBAR_IMPORT);
            mpViewBar->ShowItem(VIEWBAR_DELETE);
            mpViewBar->HideItem(VIEWBAR_NEW_FOLDER);
        }

        maSelFolders.insert(pItem);
    }
    else
    {
        maSelFolders.erase(pItem);

        if (maSelFolders.empty() && !mbIsSaveMode)
        {
            mpViewBar->HideItem(VIEWBAR_IMPORT);
            mpViewBar->HideItem(VIEWBAR_DELETE);
            mpViewBar->ShowItem(VIEWBAR_NEW_FOLDER);
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateState (const ThumbnailViewItem *pItem)
{
    bool bInSelection = maSelTemplates.find(pItem) != maSelTemplates.end();

    if (pItem->isSelected())
    {
        if (maSelTemplates.empty())
        {
            mpViewBar->Show(false);
            mpTemplateBar->Show();
        }
        else if (maSelTemplates.size() != 1 || !bInSelection)
        {
            if (!mbIsSaveMode)
            {
                mpTemplateBar->HideItem(TEMPLATEBAR_OPEN);
                mpTemplateBar->HideItem(TEMPLATEBAR_EDIT);
                mpTemplateBar->HideItem(TEMPLATEBAR_PROPERTIES);
                mpTemplateBar->HideItem(TEMPLATEBAR_DEFAULT);
            }
            else
            {
                mpTemplateBar->HideItem(TEMPLATEBAR_SAVE);
                mpTemplateBar->HideItem(TEMPLATEBAR_PROPERTIES);
                mpTemplateBar->HideItem(TEMPLATEBAR_DEFAULT);
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

            if (maSelTemplates.empty())
            {
                mpTemplateBar->Show(false);
                mpViewBar->Show();
            }
            else if (maSelTemplates.size() == 1)
            {
                if (!mbIsSaveMode)
                {
                    mpTemplateBar->ShowItem(TEMPLATEBAR_OPEN);
                    mpTemplateBar->ShowItem(TEMPLATEBAR_EDIT);
                    mpTemplateBar->ShowItem(TEMPLATEBAR_PROPERTIES);
                    mpTemplateBar->ShowItem(TEMPLATEBAR_DEFAULT);
                }
                else
                {
                    mpTemplateBar->ShowItem(TEMPLATEBAR_SAVE);
                    mpTemplateBar->ShowItem(TEMPLATEBAR_PROPERTIES);
                    mpTemplateBar->ShowItem(TEMPLATEBAR_DEFAULT);
                }
            }
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateImport ()
{
    sal_Int16 nDialogType =
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;

    sfx2::FileDialogHelper aFileDlg(nDialogType, SFXWB_MULTISELECTION);

    // add "All" filter
    aFileDlg.AddFilter( SfxResId(STR_SFX_FILTERNAME_ALL).toString(),
                        OUString(FILEDIALOG_FILTER_ALL) );

    // add template filter
    OUString sFilterExt;
    OUString sFilterName( SfxResId( STR_TEMPLATE_FILTER ).toString() );

    // add filters of modules which are installed
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
        sFilterExt += "*.ott;*.stw;*.oth";

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.ots;*.stc";
    }

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.otp;*.sti";
    }

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
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
        css::uno::Sequence<OUString> aFiles = aFileDlg.GetSelectedFiles();

        if (aFiles.hasElements())
        {
            if (!maSelFolders.empty())
            {
                //Import to the selected regions
                std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
                for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
                {
                    OUString aTemplateList;
                    TemplateContainerItem *pFolder = const_cast<TemplateContainerItem*>(static_cast<const TemplateContainerItem*>(*pIter));

                    for (size_t i = 0, n = aFiles.getLength(); i < n; ++i)
                    {
                        if(!mpLocalView->copyFrom(pFolder,aFiles[i]))
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
                        ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst("$2",aTemplateList))->Execute();
                    }
                }
            }
            else
            {
                //Import to current region
                OUString aTemplateList;
                for (size_t i = 0, n = aFiles.getLength(); i < n; ++i)
                {
                    if(!mpLocalView->copyFrom(aFiles[i]))
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
                    aMsg = aMsg.replaceFirst("$1",mpLocalView->getCurRegionName());
                    ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst("$2",aTemplateList))->Execute();
                }
            }

            mpLocalView->Invalidate(InvalidateFlags::NoErase);
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

                if (!mpLocalView->exportTo(pItem->mnAssocId,pItem->mnRegionId,aPath))
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
            sal_uInt16 nRegionItemId = mpLocalView->getCurRegionItemId();

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

                if (!mpLocalView->exportTo(pItem->mnId,nRegionItemId,aPath))
                {
                    if (aTemplateList.isEmpty())
                        aTemplateList = pItem->maTitle;
                    else
                        aTemplateList = aTemplateList + "\n" + pItem->maTitle;
                }
            }

            mpLocalView->deselectItems();
        }

        if (!aTemplateList.isEmpty())
        {
            OUString aText( SfxResId(STR_MSG_ERROR_EXPORT).toString() );
            ScopedVclPtrInstance<MessageDialog>::Create(this, aText.replaceFirst("$1",aTemplateList))->Execute();
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateSearch ()
{
    bool bVisible = mpSearchEdit->IsVisible();

    mpActionBar->SetItemState(mpActionBar->GetItemId(ACTIONBAR_SEARCH),
            bVisible? TRISTATE_FALSE: TRISTATE_TRUE);

    // fdo#74782 We are switching views. No matter to which state,
    // deselect and hide our current SearchView items.
    mpSearchView->deselectItems();
    mpSearchView->Hide();

    // Hide search view
    if (bVisible)
    {
        mpCurView->Show();
    }

    mpSearchEdit->Show(!bVisible);
    mpSearchEdit->SetText(OUString());
    if (!bVisible)
        mpSearchEdit->GrabFocus();
}

void SfxTemplateManagerDlg::OnTemplateLink ()
{
    OUString sNode("TemplateRepositoryURL");
    OUString sNodePath("/org.openoffice.Office.Common/Help/StartCenter");
    try
    {
        Reference<lang::XMultiServiceFactory> xConfig = configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
        Sequence<Any> args(1);
        PropertyValue val(
            "nodepath",
            0,
            Any(sNodePath),
            PropertyState_DIRECT_VALUE);
        args.getArray()[0] <<= val;
        Reference<container::XNameAccess> xNameAccess(xConfig->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,args), UNO_QUERY);
        if( xNameAccess.is() )
        {
            OUString sURL;
            //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
            Any value( xNameAccess->getByName(sNode) );
            sURL = value.get<OUString> ();
            localizeWebserviceURI(sURL);

            Reference< css::system::XSystemShellExecute > xSystemShellExecute(
                css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
            xSystemShellExecute->execute( sURL, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
        }
    }
    catch (const Exception&)
    {
    }
}

void SfxTemplateManagerDlg::OnTemplateOpen ()
{
    ThumbnailViewItem *pItem = const_cast<ThumbnailViewItem*>(*maSelTemplates.begin());

    OpenTemplateHdl(pItem);
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
    std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates(
            maSelTemplates); // Avoids invalid iterators from LoseFocus
    std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
    for (pIter = aSelTemplates.begin(); pIter != aSelTemplates.end(); ++pIter)
    {
        const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(*pIter);

        try
        {
            xStorable.set( mxDesktop->loadComponentFromURL(pItem->getPath(),"_default", 0, aArgs ),
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

    ScopedVclPtrInstance< SfxTemplateInfoDlg > aDlg;
    aDlg->loadDocument(pItem->getPath());
    aDlg->Execute();
}

void SfxTemplateManagerDlg::OnTemplateDelete ()
{
    ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

    if ( aQueryDlg->Execute() != RET_YES )
        return;

    OUString aTemplateList;

    if (mpSearchView->IsVisible())
    {
        std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates; //Avoids invalid iterators

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
        for (pIter = aSelTemplates.begin(); pIter != aSelTemplates.end(); ++pIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*pIter);

            if (!mpLocalView->removeTemplate(pItem->mnAssocId,pItem->mnRegionId))
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
        sal_uInt16 nRegionItemId = mpLocalView->getCurRegionItemId();
        std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates;  //Avoid invalid iterators

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
        for (pIter = aSelTemplates.begin(); pIter != aSelTemplates.end(); ++pIter)
        {
            if (!mpLocalView->removeTemplate((*pIter)->mnId,nRegionItemId))
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
        ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst("$1",aTemplateList))->Execute();
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

void SfxTemplateManagerDlg::OnFolderNew()
{
    ScopedVclPtrInstance< InputDialog > dlg(SfxResId(STR_INPUT_NEW).toString(),this);

    int ret = dlg->Execute();

    if (ret)
    {
        OUString aName = dlg->getEntryText();

        mpCurView->createRegion(aName);
    }
}

void SfxTemplateManagerDlg::OnFolderDelete()
{
    ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, SfxResId(STR_QMSG_SEL_FOLDER_DELETE), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

    if ( aQueryDlg->Execute() != RET_YES )
        return;

    OUString aFolderList;

    std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
    std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelFolders = maSelFolders; //Copy to avoid invalidating an iterator

    for (pIter = aSelFolders.begin(); pIter != aSelFolders.end(); ++pIter)
    {
        if (!mpLocalView->removeRegion((*pIter)->mnId))
        {
            if (aFolderList.isEmpty())
                aFolderList = (*pIter)->maTitle;
            else
                aFolderList = aFolderList + "\n" + (*pIter)->maTitle;

            ++pIter;
            if (pIter == aSelFolders.end())
                break;
        }
    }

    if (!aFolderList.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_FOLDER).toString() );
        ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst("$1",aFolderList))->Execute();
    }
}

void SfxTemplateManagerDlg::OnRepositoryDelete()
{
    if(deleteRepository(mpRemoteView->getCurRegionId()))
    {
        // switch to local view
        switchMainView(true);

        createRepositoryMenu();
    }
}

void SfxTemplateManagerDlg::OnTemplateSaveAs()
{
    assert(m_xModel.is());

    if (!mpLocalView->isNonRootRegionVisible() && maSelFolders.empty())
    {
        ScopedVclPtrInstance<MessageDialog>::Create(this, SfxResId(STR_MSG_ERROR_SELECT_FOLDER))->Execute();
        return;
    }

    ScopedVclPtrInstance< InputDialog > aDlg(SfxResId(STR_INPUT_TEMPLATE_NEW).toString(),this);

    if (aDlg->Execute())
    {
        OUString aName = aDlg->getEntryText();

        if (!aName.isEmpty())
        {
            OUString aFolderList;
            OUString aQMsg(SfxResId(STR_QMSG_TEMPLATE_OVERWRITE).toString());
            ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, OUString(), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

            if (mpLocalView->isNonRootRegionVisible())
            {
                sal_uInt16 nRegionItemId = mpLocalView->getRegionId(mpLocalView->getCurRegionId()-1);

                if (!mpLocalView->isTemplateNameUnique(nRegionItemId,aName))
                {
                    aQMsg = aQMsg.replaceFirst("$1",aName);
                    aQueryDlg->set_primary_text(aQMsg.replaceFirst("$2",mpLocalView->getCurRegionName()));

                    if (aQueryDlg->Execute() == RET_NO)
                        return;
                }

                if (!mpLocalView->saveTemplateAs(nRegionItemId,m_xModel,aName))
                    aFolderList = mpLocalView->getCurRegionName();
            }
            else
            {
                std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator pIter;
                for (pIter = maSelFolders.begin(); pIter != maSelFolders.end(); ++pIter)
                {
                    TemplateContainerItem *pItem = const_cast<TemplateContainerItem*>(static_cast<const TemplateContainerItem*>(*pIter));

                    if (!mpLocalView->isTemplateNameUnique(pItem->mnId,aName))
                    {
                        OUString aDQMsg = aQMsg.replaceFirst("$1",aName);
                        aQueryDlg->set_primary_text(aDQMsg.replaceFirst("$2",pItem->maTitle));

                        if (aQueryDlg->Execute() == RET_NO)
                            continue;
                    }

                    if (!mpLocalView->saveTemplateAs(pItem,m_xModel,aName))
                    {
                        if (aFolderList.isEmpty())
                            aFolderList = (*pIter)->maTitle;
                        else
                            aFolderList = aFolderList + "\n" + (*pIter)->maTitle;
                    }
                }
            }

            if (!aFolderList.isEmpty())
            {
            }

            // After save file, just close the dialog
            Close();
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
            mpTemplateDefaultMenu->InsertItem(nItemId, aTitle, SvFileInformationManager::GetImage(aObj));
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
        mpCurView = mpLocalView.get();

        mpViewBar->HideItem(VIEWBAR_DELETE);

        // Enable deleting and exporting items from the filesystem
        mpTemplateBar->ShowItem(TEMPLATEBAR_EXPORT);
        mpTemplateBar->ShowItem(TEMPLATEBAR_DELETE);

        mpRemoteView->Hide();
        mpLocalView->Show();
    }
    else
    {
        mpCurView = mpRemoteView.get();

        mpViewBar->ShowItem(VIEWBAR_DELETE);

        // Disable deleting and exporting items from remote repositories
        mpTemplateBar->HideItem(TEMPLATEBAR_EXPORT);
        mpTemplateBar->HideItem(TEMPLATEBAR_DELETE);

        mpLocalView->Hide();
        mpRemoteView->Show();
    }
}

void SfxTemplateManagerDlg::localMoveTo(sal_uInt16 nMenuId)
{
    sal_uInt16 nItemId = 0;

    if (nMenuId == MNI_MOVE_NEW)
    {
        ScopedVclPtrInstance< InputDialog > dlg(SfxResId(STR_INPUT_NEW).toString(),this);

        int ret = dlg->Execute();

        if (ret)
        {
            OUString aName = dlg->getEntryText();

            if (!aName.isEmpty())
                nItemId = mpLocalView->createRegion(aName);
        }
    }
    else
    {
        nItemId = mpLocalView->getRegionId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        if (!mpLocalView->moveTemplates(maSelTemplates,nItemId))
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

            OUString aDst = mpLocalView->getRegionItemName(nItemId);
            OUString aMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE).toString());
            aMsg = aMsg.replaceFirst("$1",aDst);
            ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst( "$2",aTemplateList))->Execute();
        }
    }
}

void SfxTemplateManagerDlg::remoteMoveTo(const sal_uInt16 nMenuId)
{
    sal_uInt16 nItemId = 0;

    if (nMenuId == MNI_MOVE_NEW)
    {
        ScopedVclPtrInstance< InputDialog > dlg(SfxResId(STR_INPUT_NEW).toString(),this);

        int ret = dlg->Execute();

        if (ret)
        {
            OUString aName = dlg->getEntryText();

            if (!aName.isEmpty())
                nItemId = mpLocalView->createRegion(aName);
        }
    }
    else
    {
        nItemId = mpLocalView->getRegionId(nMenuId-MNI_MOVE_FOLDER_BASE);
    }

    if (nItemId)
    {
        OUString aTemplateList;

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aIter;
        for (aIter = maSelTemplates.begin(); aIter != maSelTemplates.end(); ++aIter)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(*aIter);

            if(!mpLocalView->copyFrom(nItemId,pItem->maPreview1,pItem->getPath()))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + pItem->maTitle;
            }
        }

        mpLocalView->Invalidate(InvalidateFlags::NoErase);

        if (!aTemplateList.isEmpty())
        {
            OUString aMsg(SfxResId(STR_MSG_ERROR_REMOTE_MOVE).toString());
            aMsg = aMsg.replaceFirst("$1",mpRemoteView->getCurRegionName());
            aMsg = aMsg.replaceFirst("$2",mpLocalView->getRegionItemName(nItemId));
            ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst("$1",aTemplateList))->Execute();
        }
    }
}

void SfxTemplateManagerDlg::localSearchMoveTo(sal_uInt16 nMenuId)
{
    sal_uInt16 nItemId = 0;

    if (nMenuId == MNI_MOVE_NEW)
    {
        ScopedVclPtrInstance< InputDialog > dlg(SfxResId(STR_INPUT_NEW).toString(),this);

        int ret = dlg->Execute();

        if (ret)
        {
            OUString aName = dlg->getEntryText();

            if (!aName.isEmpty())
                nItemId = mpLocalView->createRegion(aName);
        }
    }
    else
    {
        nItemId = mpLocalView->getRegionId(nMenuId-MNI_MOVE_FOLDER_BASE);
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

            if(!mpLocalView->moveTemplate(pItem,pItem->mnRegionId,nItemId))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = (*aIter)->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + (*aIter)->maTitle;
            }
        }

        if (!aTemplateList.isEmpty())
        {
            OUString aDst = mpLocalView->getRegionItemName(nItemId);
            OUString aMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE).toString());
            aMsg = aMsg.replaceFirst("$1",aDst);
            ScopedVclPtrInstance<MessageDialog>::Create(this, aMsg.replaceFirst( "$2",aTemplateList))->Execute();
        }
    }

    // Deselect all items and update search results
    mpSearchView->deselectItems();

    SearchUpdateHdl(*mpSearchEdit);
}

void SfxTemplateManagerDlg::loadRepositories()
{
    uno::Reference < uno::XComponentContext > m_context(comphelper::getProcessComponentContext());

    // Load from user settings
    css::uno::Sequence<OUString>  aUrls =
            officecfg::Office::Common::Misc::TemplateRepositoryUrls::get(m_context);

    css::uno::Sequence<OUString> aNames =
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
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(pContext));

        size_t nSize = maRepositories.size();
        uno::Sequence<OUString> aUrls(nSize);
        uno::Sequence<OUString> aNames(nSize);

        for(size_t i = 0; i < nSize; ++i)
        {
            aUrls[i] = maRepositories[i]->getURL();
            aNames[i] = maRepositories[i]->maTitle;
        }

        officecfg::Office::Common::Misc::TemplateRepositoryUrls::set(aUrls, batch);
        officecfg::Office::Common::Misc::TemplateRepositoryNames::set(aNames, batch);
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

            SotClipboardFormatId nFormat = SotStorage::GetFormatID( xStorage );

            const SfxFilter* pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4ClipBoardId( nFormat );

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
    const css::uno::Sequence<OUString> &aServiceNames = aModOpt.GetAllServiceNames();

    for( sal_Int32 i=0, nCount = aServiceNames.getLength(); i < nCount; ++i )
    {
        if ( ! SfxObjectFactory::GetStandardTemplate( aServiceNames[i] ).isEmpty() )
        {
            SvtModuleOptions::EFactory eFac = SvtModuleOptions::EFactory::WRITER;
            SvtModuleOptions::ClassifyFactoryByName( aServiceNames[i], eFac );
            aList.push_back(aModOpt.GetFactoryEmptyDocumentURL(eFac));
        }
    }

    return aList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
