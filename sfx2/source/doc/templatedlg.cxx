/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatedlg.hxx>

#include <sfx2/inputdlg.hxx>
#include "templatesearchview.hxx"
#include "templatesearchviewitem.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/storagehelper.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxresid.hxx>
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
const char TM_SETTING_LASTAPPLICATION[] = "LastApplication";

const char SERVICENAME_CFGREADACCESS[] = "com.sun.star.configuration.ConfigurationAccess";

const char ACTIONBAR_REPOSITORY[] = "repository";
const char ACTIONBAR_ACTION[] = "action_menu";

#define MNI_ACTION_NEW_FOLDER 1
#define MNI_ACTION_RENAME_FOLDER 2
#define MNI_ACTION_DELETE_FOLDER 3
#define MNI_ACTION_REFRESH   4
#define MNI_ACTION_DEFAULT   5
#define MNI_REPOSITORY_LOCAL 1
#define MNI_REPOSITORY_NEW   2
#define MNI_REPOSITORY_BASE  3
#define MNI_WRITER           1
#define MNI_CALC             2
#define MNI_IMPRESS          3
#define MNI_DRAW             4

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

        return bRet && MatchSubstring(rItem.aName);
    }

    bool MatchSubstring( OUString sItemName )
    {
        if(maKeyword.isEmpty())
            return false;
        sItemName = sItemName.toAsciiLowerCase();
        maKeyword = maKeyword.toAsciiLowerCase();
        if(sItemName.indexOf(maKeyword) >= 0)
            return true;
        return false;
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
      mxDesktop( Desktop::create(comphelper::getProcessComponentContext()) ),
      mbIsSynced(false),
      maRepositories()
{
    get(mpSearchFilter, "search_filter");
    get(mpCBApp, "filter_application");
    get(mpCBFolder, "filter_folder");
    get(mpActionBar, "action_action");
    get(mpLocalView, "template_view");
    get(mpSearchView, "search_view");
    get(mpRemoteView, "remote_view");
    get(mpOKButton, "ok");
    get(mpMoveButton, "move_btn");
    get(mpExportButton, "export_btn");
    get(mpImportButton, "import_btn");
    get(mpLinkButton, "online_link");

    // Create popup menus
    mpActionMenu = new PopupMenu;
    mpActionMenu->InsertItem(MNI_ACTION_NEW_FOLDER,
        SfxResId(STR_CATEGORY_NEW).toString(),
        Image(SfxResId(IMG_ACTION_REFRESH)));
    mpActionMenu->InsertItem(MNI_ACTION_RENAME_FOLDER,
        SfxResId(STR_CATEGORY_RENAME).toString());
    mpActionMenu->InsertItem(MNI_ACTION_DELETE_FOLDER,
        SfxResId(STR_CATEGORY_DELETE).toString());
    mpActionMenu->InsertSeparator();
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
    mpActionBar->SetButtonType(ButtonType::SYMBOLTEXT);

    // Set toolbox button bits
    mpActionBar->SetItemBits(mpActionBar->GetItemId(ACTIONBAR_REPOSITORY), ToolBoxItemBits::DROPDOWNONLY);
    mpActionBar->SetItemBits(mpActionBar->GetItemId(ACTIONBAR_ACTION), ToolBoxItemBits::DROPDOWNONLY);

    // Set toolbox handlers
    mpActionBar->SetDropdownClickHdl(LINK(this,SfxTemplateManagerDlg,TBXDropdownHdl));

    mpLocalView->SetStyle(mpLocalView->GetStyle() | WB_VSCROLL);
    mpLocalView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mpLocalView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_PADDING);

    mpLocalView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mpLocalView->setCreateContextMenuHdl(LINK(this,SfxTemplateManagerDlg, CreateContextMenuHdl));
    mpLocalView->setOpenRegionHdl(LINK(this,SfxTemplateManagerDlg, OpenRegionHdl));
    mpLocalView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg, OpenTemplateHdl));
    mpLocalView->setEditTemplateHdl(LINK(this,SfxTemplateManagerDlg, EditTemplateHdl));
    mpLocalView->setDeleteTemplateHdl(LINK(this,SfxTemplateManagerDlg, DeleteTemplateHdl));
    mpLocalView->setDefaultTemplateHdl(LINK(this,SfxTemplateManagerDlg, DefaultTemplateHdl));

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
    mpSearchView->setCreateContextMenuHdl(LINK(this,SfxTemplateManagerDlg, CreateContextMenuHdl));
    mpSearchView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));
    mpSearchView->setEditTemplateHdl(LINK(this,SfxTemplateManagerDlg, EditTemplateHdl));
    mpSearchView->setDeleteTemplateHdl(LINK(this,SfxTemplateManagerDlg, DeleteTemplateHdl));
    mpSearchView->setDefaultTemplateHdl(LINK(this,SfxTemplateManagerDlg, DefaultTemplateHdl));

    mpLocalView->ShowTooltips(true);
    mpSearchView->ShowTooltips(true);

    mpOKButton->SetClickHdl(LINK(this, SfxTemplateManagerDlg, OkClickHdl));
    mpMoveButton->SetClickHdl(LINK(this, SfxTemplateManagerDlg, MoveClickHdl));
    mpExportButton->SetClickHdl(LINK(this, SfxTemplateManagerDlg, ExportClickHdl));
    mpImportButton->SetClickHdl(LINK(this, SfxTemplateManagerDlg, ImportClickHdl));
    mpLinkButton->SetClickHdl(LINK(this, SfxTemplateManagerDlg, LinkClickHdl));

    mpSearchFilter->SetUpdateDataHdl(LINK(this, SfxTemplateManagerDlg, SearchUpdateHdl));
    mpSearchFilter->EnableUpdateData();
    mpSearchFilter->SetGetFocusHdl(LINK( this, SfxTemplateManagerDlg, GetFocusHdl ));

    SvtMiscOptions aMiscOptions;
    if ( !aMiscOptions.IsExperimentalMode() )
    {
        sal_uInt16 nPos = mpActionBar->GetItemPos(mpActionBar->GetItemId(ACTIONBAR_REPOSITORY));
        mpActionBar->RemoveItem(nPos);
    }

    mpActionBar->Show();

    switchMainView(true);

    loadRepositories();

    createRepositoryMenu();
    createDefaultTemplateMenu();

    mpLocalView->Populate();
    mpCurView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));

    mpCBApp->SelectEntryPos(0);
    fillFolderComboBox();

    mpExportButton->Disable();
    mpMoveButton->Disable();
    mpOKButton->SetText(SfxResId(STR_OPEN).toString());

    mpCBApp->SetSelectHdl(LINK(this, SfxTemplateManagerDlg, SelectApplicationHdl));
    mpCBFolder->SetSelectHdl(LINK(this, SfxTemplateManagerDlg, SelectRegionHdl));

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
    for (TemplateRepository* p : maRepositories)
        delete p;

    // Ignore view events since we are cleaning the object
    mpLocalView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpLocalView->setOpenRegionHdl(Link<void*,void>());
    mpLocalView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());

    mpRemoteView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpRemoteView->setOpenRegionHdl(Link<void*,void>());
    mpRemoteView->setOpenTemplateHdl(Link<ThumbnailViewItem*,void>());

    mpSearchView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpSearchView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());

    mpOKButton.clear();
    mpMoveButton.clear();
    mpExportButton.clear();
    mpImportButton.clear();
    mpLinkButton.clear();
    mpSearchFilter.clear();
    mpCBApp.clear();
    mpCBFolder.clear();
    mpActionBar.clear();
    mpSearchView.clear();
    mpCurView.clear();
    mpLocalView.clear();
    mpRemoteView.clear();
    ModalDialog::dispose();
}

short SfxTemplateManagerDlg::Execute()
{
    //use application specific settings if there's no previous setting
    getApplicationSpecificSettings();
    readSettings();

    return ModalDialog::Execute();
}

void SfxTemplateManagerDlg::setDocumentModel(const uno::Reference<frame::XModel> &rModel)
{
    m_xModel = rModel;
}

FILTER_APPLICATION SfxTemplateManagerDlg::getCurrentApplicationFilter()
{
    const sal_Int16 nCurAppId = mpCBApp->GetSelectEntryPos();

    if (nCurAppId == MNI_WRITER)
        return FILTER_APPLICATION::WRITER;
    else if (nCurAppId == MNI_IMPRESS)
        return FILTER_APPLICATION::IMPRESS;
    else if (nCurAppId == MNI_CALC)
        return FILTER_APPLICATION::CALC;
    else if (nCurAppId == MNI_DRAW)
        return FILTER_APPLICATION::DRAW;

    return FILTER_APPLICATION::NONE;
}

void SfxTemplateManagerDlg::fillFolderComboBox()
{
    std::vector<OUString> aFolderNames = mpLocalView->getFolderNames();

    if (!aFolderNames.empty())
    {
        for (size_t i = 0, n = aFolderNames.size(); i < n; ++i)
            mpCBFolder->InsertEntry(aFolderNames[i], i+1);
    }
    mpCBFolder->SelectEntryPos(0);
    mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
}

void SfxTemplateManagerDlg::getApplicationSpecificSettings()
{
    if ( ! m_xModel.is() )
    {
        mpCBApp->SelectEntryPos(0);
        mpCBFolder->SelectEntryPos(0);
        mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
        mpCurView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpLocalView->showAllTemplates();
        return;
    }

    SvtModuleOptions::EFactory eFactory = SvtModuleOptions::ClassifyFactoryByModel(m_xModel);

    switch(eFactory)
    {
        case SvtModuleOptions::EFactory::WRITER:
        case SvtModuleOptions::EFactory::WRITERWEB:
        case SvtModuleOptions::EFactory::WRITERGLOBAL:
                            mpCBApp->SelectEntryPos(MNI_WRITER);
                            break;
        case SvtModuleOptions::EFactory::CALC:
                            mpCBApp->SelectEntryPos(MNI_CALC);
                            break;
        case SvtModuleOptions::EFactory::IMPRESS:
                            mpCBApp->SelectEntryPos(MNI_IMPRESS);
                            break;
        case SvtModuleOptions::EFactory::DRAW:
                            mpCBApp->SelectEntryPos(MNI_DRAW);
                            break;
        default:
                mpCBApp->SelectEntryPos(0);
                break;
    }

    mpCurView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
    mpCBFolder->SelectEntryPos(0);
    mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
    mpLocalView->showAllTemplates();
}

void SfxTemplateManagerDlg::readSettings ()
{
    OUString aLastFolder;
    SvtViewOptions aViewSettings( E_DIALOG, TM_SETTING_MANAGER );

    if ( aViewSettings.Exists() )
    {
        sal_uInt16 nTmp = 0;
        aViewSettings.GetUserItem(TM_SETTING_LASTFOLDER) >>= aLastFolder;
        aViewSettings.GetUserItem(TM_SETTING_LASTAPPLICATION) >>= nTmp;

        //open last remembered application only when application model is not set
        if(!m_xModel.is())
        {
            switch (nTmp)
            {
                case MNI_WRITER:
                    mpCBApp->SelectEntryPos(MNI_WRITER);
                    break;
                case MNI_CALC:
                    mpCBApp->SelectEntryPos(MNI_CALC);
                    break;
                case MNI_IMPRESS:
                    mpCBApp->SelectEntryPos(MNI_IMPRESS);
                    break;
                case MNI_DRAW:
                    mpCBApp->SelectEntryPos(MNI_DRAW);
                    break;
                default:
                    mpCBApp->SelectEntryPos(0);
                    break;
            }
        }
    }

    mpCurView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));

    if (aLastFolder.isEmpty())
    {
        //show all categories
        mpCBFolder->SelectEntryPos(0);
        mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
        mpLocalView->showAllTemplates();
    }
    else
    {
        mpCBFolder->SelectEntry(aLastFolder);
        mpLocalView->showRegion(aLastFolder);
        mpActionMenu->ShowItem(MNI_ACTION_RENAME_FOLDER);
    }
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
        { TM_SETTING_LASTAPPLICATION,     css::uno::makeAny(sal_uInt16(mpCBApp->GetSelectEntryPos())) }
    };

    // write
    SvtViewOptions aViewSettings(E_DIALOG, TM_SETTING_MANAGER);
    aViewSettings.SetUserData(aSettings);
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, SelectApplicationHdl, ListBox&, void)
{
    if(mpCurView == mpLocalView && mpCurView->IsVisible())
    {
        mpCurView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpCurView->showAllTemplates();
        mpCBFolder->SelectEntryPos(0);
        mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
    }

    if(mpSearchView->IsVisible())
        SearchUpdateHdl(*mpSearchFilter);
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, SelectRegionHdl, ListBox&, void)
{
    if(mpCurView == mpLocalView)
    {
        const OUString sSelectedRegion = mpCBFolder->GetSelectEntry();
        if(mpCBFolder->GetSelectEntryPos() == 0)
        {
            mpLocalView->showAllTemplates();
            mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
        }
        else
        {
            mpLocalView->showRegion(sSelectedRegion);
            mpActionMenu->ShowItem(MNI_ACTION_RENAME_FOLDER);
        }
    }

    if(mpSearchView->IsVisible())
        SearchUpdateHdl(*mpSearchFilter);
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, TBXDropdownHdl, ToolBox*, void)
{
    const sal_uInt16 nCurItemId = mpActionBar->GetCurItemId();
    mpActionBar->SetItemDown( nCurItemId, true );

    if (nCurItemId == mpActionBar->GetItemId(ACTIONBAR_ACTION))
        mpActionMenu->Execute(mpActionBar, mpActionBar->GetItemRect(nCurItemId), PopupMenuFlags::ExecuteDown);
    else if (nCurItemId == mpActionBar->GetItemId(ACTIONBAR_REPOSITORY))
        mpRepositoryMenu->Execute(mpActionBar, mpActionBar->GetItemRect(nCurItemId), PopupMenuFlags::ExecuteDown);

    mpActionBar->SetItemDown( nCurItemId, false );
    mpActionBar->EndSelection();
    mpActionBar->Invalidate();
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, TVItemStateHdl, const ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

    if (pViewItem)
        OnTemplateState(pItem);
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, MenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_ACTION_NEW_FOLDER:
        OnCategoryNew();
        break;
    case MNI_ACTION_RENAME_FOLDER:
        OnCategoryRename();
        break;
    case MNI_ACTION_DELETE_FOLDER:
        OnCategoryDelete();
        break;
    case MNI_ACTION_REFRESH:
        mpCurView->reload();
        break;
    default:
        break;
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

        TemplateRepository *pRepository = nullptr;

        for (TemplateRepository* p : maRepositories)
        {
            if (p->mnId == nRepoId)
            {
                pRepository = p;
                break;
            }
        }

        if (mpRemoteView->loadRepository(pRepository))
            switchMainView(false);
    }

    return false;
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, DefaultTemplateMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    OUString aServiceName = SfxObjectShell::GetServiceNameFromFactory( mpTemplateDefaultMenu->GetItemCommand(nId));

    OUString sPrevDefault = SfxObjectFactory::GetStandardTemplate( aServiceName );
    if(!sPrevDefault.isEmpty())
        mpLocalView->RemoveDefaultTemplateIcon(sPrevDefault);

    SfxObjectFactory::SetStandardTemplate( aServiceName, OUString() );

    createDefaultTemplateMenu();

    return false;
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, OkClickHdl, Button*, void)
{
   OnTemplateOpen();
   EndDialog(RET_OK);
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, MoveClickHdl, Button*, void)
{
    // modal dialog to select templates category
    ScopedVclPtrInstance<SfxTemplateCategoryDialog> aDlg;
    aDlg->SetCategoryLBEntries(mpLocalView->getFolderNames());

    size_t nItemId = 0;

    if(aDlg->Execute() == RET_OK)
    {
        OUString sCategory = aDlg->GetSelectedCategory();
        bool bIsNewCategory = aDlg->IsNewCategoryCreated();
        if(bIsNewCategory)
        {
            if (!sCategory.isEmpty())
            {
                nItemId = mpLocalView->createRegion(sCategory);
                if(nItemId)
                    mpCBFolder->InsertEntry(sCategory);
            }
        }
        else
        {
            nItemId = mpLocalView->getRegionId(sCategory);
        }
    }

    if(nItemId)
    {
        if (mpSearchView->IsVisible())
        {
            // Check if we are searching the local or remote templates
            if (mpCurView == mpLocalView)
                localSearchMoveTo(nItemId);
        }
        else
        {
            // Check if we are displaying the local or remote templates
            if (mpCurView == mpLocalView)
                localMoveTo(nItemId);
            else
                remoteMoveTo(nItemId);
        }
    }

    mpLocalView->reload();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, ExportClickHdl, Button*, void)
{
    OnTemplateExport();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, ImportClickHdl, Button*, void)
{
    //Modal Dialog to select Category
    ScopedVclPtrInstance<SfxTemplateCategoryDialog> aDlg;
    aDlg->SetCategoryLBEntries(mpLocalView->getFolderNames());

    if(aDlg->Execute() == RET_OK)
    {
        OUString sCategory = aDlg->GetSelectedCategory();
        bool bIsNewCategory = aDlg->IsNewCategoryCreated();
        if(bIsNewCategory)
        {
            if(mpCurView->createRegion(sCategory))
            {
                mpCBFolder->InsertEntry(sCategory);
                OnTemplateImportCategory(sCategory);
            }
            else
            {
                OUString aMsg( SfxResId(STR_CREATE_ERROR).toString() );
                ScopedVclPtrInstance<MessageDialog>(this, aMsg.replaceFirst("$1", sCategory))->Execute();
                return;
            }
        }
        else
            OnTemplateImportCategory(sCategory);
    }

    mpLocalView->reload();
    mpLocalView->showAllTemplates();
    mpCBApp->SelectEntryPos(0);
    mpCBFolder->SelectEntryPos(0);
    mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
}

IMPL_STATIC_LINK_NOARG_TYPED(SfxTemplateManagerDlg, LinkClickHdl, Button*, void)
{
    OnTemplateLink();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, OpenRegionHdl, void*, void)
{
    maSelTemplates.clear();
    mpOKButton->Disable();
    mpActionBar->Show();
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, CreateContextMenuHdl, ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if (pViewItem)
    {
        if(mpCurView == mpLocalView)
        {
            if(mpSearchView->IsVisible())
                mpSearchView->createContextMenu(pViewItem->IsDefaultTemplate());
            else
                mpLocalView->createContextMenu(pViewItem->IsDefaultTemplate());
        }
    }
}


IMPL_LINK_TYPED(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs(4);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= true;
    aArgs[1].Name = "MacroExecutionMode";
    aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
    aArgs[2].Name = "UpdateDocMode";
    aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;
    aArgs[3].Name = "InteractionHandler";
    aArgs[3].Value <<= task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr );

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

IMPL_LINK_TYPED(SfxTemplateManagerDlg, EditTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs(3);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= false;
    aArgs[1].Name = "MacroExecutionMode";
    aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
    aArgs[2].Name = "UpdateDocMode";
    aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;

    uno::Reference< XStorable > xStorable;
    TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);

    try
    {
        xStorable.set( mxDesktop->loadComponentFromURL(pViewItem->getPath(),"_default", 0, aArgs ),
                       uno::UNO_QUERY );
    }
    catch( const uno::Exception& )
    {
    }

    Close();
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, DeleteTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    OUString aDeletedTemplate;

    if(mpSearchView->IsVisible())
    {
        TemplateSearchViewItem *pSrchItem = static_cast<TemplateSearchViewItem*>(pItem);

        if (!mpLocalView->removeTemplate((pSrchItem)->mnAssocId, pSrchItem->mnRegionId))
        {
            aDeletedTemplate = (pSrchItem)->maTitle;
        }
    }
    else
    {
        TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);
        sal_uInt16 nRegionItemId = mpLocalView->getRegionId(pViewItem->mnRegionId);

        if (!mpLocalView->removeTemplate((pViewItem)->mnDocId + 1, nRegionItemId))//mnId w.r.t. region is mnDocId + 1;
        {
            aDeletedTemplate = (pItem)->maTitle;
        }
    }

    if (!aDeletedTemplate.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_TEMPLATE).toString() );
        ScopedVclPtrInstance<MessageDialog>(this, aMsg.replaceFirst("$1",aDeletedTemplate))->Execute();
    }
}

IMPL_LINK_TYPED(SfxTemplateManagerDlg, DefaultTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);
    OUString aServiceName;

    if(!pViewItem->IsDefaultTemplate())
    {
        if (lcl_getServiceName(pViewItem->getPath(),aServiceName))
        {
            OUString sPrevDefault = SfxObjectFactory::GetStandardTemplate( aServiceName );
            if(!sPrevDefault.isEmpty())
                mpLocalView->RemoveDefaultTemplateIcon(sPrevDefault);

            SfxObjectFactory::SetStandardTemplate(aServiceName,pViewItem->getPath());
            pViewItem->showDefaultIcon(true);
        }
    }
    else
    {
        if(lcl_getServiceName(pViewItem->getPath(),aServiceName))
        {
            SfxObjectFactory::SetStandardTemplate( aServiceName, OUString() );
            pViewItem->showDefaultIcon(false);
        }
    }

    createDefaultTemplateMenu();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, SearchUpdateHdl, Edit&, void)
{
    OUString aKeyword = mpSearchFilter->GetText();

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

        std::vector<TemplateItemProperties> aItems =
                mpLocalView->getFilteredItems(SearchView_Keyword(aKeyword, getCurrentApplicationFilter()));

        for (TemplateItemProperties& rItem : aItems)
        {
            OUString aFolderName;

            aFolderName = mpLocalView->getRegionName(rItem.nRegionId);

            mpSearchView->AppendItem(rItem.nId,mpLocalView->getRegionId(rItem.nRegionId),
                                     rItem.nDocId,
                                     rItem.aName,
                                     aFolderName,
                                     rItem.aPath,
                                     rItem.aThumbnail);
        }

        mpSearchView->Invalidate();
    }
    else
    {
        mpSearchView->deselectItems();
        mpSearchView->Hide();
        mpCurView->Show();
        mpCurView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        if(mpCurView == mpLocalView)
        {
            mpLocalView->reload();
            OUString sLastFolder = mpCBFolder->GetSelectEntry();
            mpLocalView->showRegion(sLastFolder);
            mpActionMenu->ShowItem(MNI_ACTION_RENAME_FOLDER);
        }
    }
}

IMPL_LINK_NOARG_TYPED(SfxTemplateManagerDlg, GetFocusHdl, Control&, void)
{
    mpCurView->deselectItems();
    maSelTemplates.clear();
}

void SfxTemplateManagerDlg::OnTemplateState (const ThumbnailViewItem *pItem)
{
    bool bInSelection = maSelTemplates.find(pItem) != maSelTemplates.end();

    if (pItem->isSelected())
    {
        if (maSelTemplates.empty())
        {
            mpOKButton->Enable();
        }
        else if (maSelTemplates.size() != 1 || !bInSelection)
        {
            mpOKButton->Disable();
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
                mpOKButton->Disable();
            }
            else if (maSelTemplates.size() == 1)
            {
                mpOKButton->Enable();
            }
        }
    }

    if(maSelTemplates.empty())
    {
        mpMoveButton->Disable();
        mpExportButton->Disable();
    }
    else
    {
        mpMoveButton->Enable();
        mpExportButton->Enable();
    }
}

void SfxTemplateManagerDlg::OnTemplateImportCategory(const OUString& sCategory)
{
    sal_Int16 nDialogType =
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;

    sfx2::FileDialogHelper aFileDlg(nDialogType, FileDialogFlags::MultiSelection);

    // add "All" filter
    aFileDlg.AddFilter( SfxResId(STR_SFX_FILTERNAME_ALL).toString(),
                        FILEDIALOG_FILTER_ALL );

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
            //Import to the selected regions
            TemplateContainerItem* pContItem = mpLocalView->getRegion(sCategory);
            if(pContItem)
            {
                OUString aTemplateList;

                for (size_t i = 0, n = aFiles.getLength(); i < n; ++i)
                {
                    if(!mpLocalView->copyFrom(pContItem,aFiles[i]))
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
                    aMsg = aMsg.replaceFirst("$1",pContItem->maTitle);
                    ScopedVclPtrInstance<MessageDialog>(this, aMsg.replaceFirst("$2",aTemplateList))->Execute();
                }
            }
        }
    }
}

void SfxTemplateManagerDlg::OnTemplateExport()
{
    uno::Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<XFolderPicker2> xFolderPicker = FolderPicker::create(xContext);

    xFolderPicker->setDisplayDirectory(SvtPathOptions().GetWorkPath());

    sal_Int16 nResult = xFolderPicker->execute();
    sal_Int16 nCount = maSelTemplates.size();

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

                if (!mpLocalView->exportTo(pItem->mnDocId + 1,   //mnId w.r.t. region = mDocId + 1
                    mpLocalView->getRegionId(pItem->mnRegionId), //pItem->mnRegionId does not store actual region Id
                    aPath))
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
            ScopedVclPtrInstance<MessageDialog>(this, aText.replaceFirst("$1",aTemplateList))->Execute();
        }
        else
        {
            OUString sText( SfxResId(STR_MSG_EXPORT_SUCCESS).toString() );
            ScopedVclPtrInstance<MessageDialog>(this, sText.replaceFirst("$1", OUString::number(nCount)), VclMessageType::VCL_MESSAGE_INFO)->Execute();
        }
    }
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

void SfxTemplateManagerDlg::OnCategoryNew()
{
    ScopedVclPtrInstance< InputDialog > dlg(SfxResId(STR_INPUT_NEW).toString(),this);

    int ret = dlg->Execute();

    if (ret)
    {
        OUString aName = dlg->GetEntryText();

        if(mpCurView->createRegion(aName))
            mpCBFolder->InsertEntry(aName);
        else
        {
            OUString aMsg( SfxResId(STR_CREATE_ERROR).toString() );
            ScopedVclPtrInstance<MessageDialog>(this, aMsg.replaceFirst("$1", aName))->Execute();
        }
    }
}

void SfxTemplateManagerDlg::OnCategoryRename()
{
    OUString sCategory = mpCBFolder->GetSelectEntry();
    ScopedVclPtrInstance< InputDialog > dlg(SfxResId(STR_INPUT_NEW).toString(),this);

    dlg->SetEntryText(sCategory);
    int ret = dlg->Execute();

    if (ret)
    {
        OUString aName = dlg->GetEntryText();

        if(mpLocalView->renameRegion(sCategory, aName))
        {
            sal_Int32 nPos = mpCBFolder->GetEntryPos(sCategory);
            mpCBFolder->RemoveEntry(nPos);
            mpCBFolder->InsertEntry(aName, nPos);
            mpCBFolder->SelectEntryPos(nPos);

            mpLocalView->reload();
            mpLocalView->showRegion(aName);
        }
        else
        {
            OUString aMsg( SfxResId(STR_CREATE_ERROR).toString() );
            ScopedVclPtrInstance<MessageDialog>(this, aMsg.replaceFirst("$1", aName))->Execute();
        }
    }
}

void SfxTemplateManagerDlg::OnCategoryDelete()
{
    ScopedVclPtrInstance< SfxTemplateCategoryDialog > aDlg;
    aDlg->SetCategoryLBEntries(mpLocalView->getFolderNames());
    aDlg->HideNewCategoryOption();
    aDlg->SetText(SfxResId(STR_CATEGORY_DELETE).toString());
    aDlg->SetSelectLabelText(SfxResId(STR_CATEGORY_SELECT).toString());

    if(aDlg->Execute() == RET_OK)
    {
        OUString sCategory = aDlg->GetSelectedCategory();
        aDlg->Close();
        ScopedVclPtrInstance< MessageDialog > popupDlg(this, SfxResId(STR_QMSG_SEL_FOLDER_DELETE),
            VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

        if ( popupDlg->Execute() != RET_YES )
            return;

        sal_Int16 nItemId = mpLocalView->getRegionId(sCategory);

        if (!mpLocalView->removeRegion(nItemId))
        {
            OUString sMsg( SfxResId(STR_MSG_ERROR_DELETE_FOLDER).toString() );
            ScopedVclPtrInstance<MessageDialog>(this, sMsg.replaceFirst("$1",sCategory))->Execute();
        }
        else
        {
            mpCBFolder->RemoveEntry(sCategory);
        }
    }

    mpLocalView->reload();
    mpLocalView->showAllTemplates();
    mpCBApp->SelectEntryPos(0);
    mpCBFolder->SelectEntryPos(0);
    mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
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

void SfxTemplateManagerDlg::createRepositoryMenu()
{
    mpRepositoryMenu->Clear();

    mpRepositoryMenu->InsertItem(MNI_REPOSITORY_LOCAL,SfxResId(STR_REPOSITORY_LOCAL).toString());

    const std::vector<TemplateRepository*> &rRepos = getRepositories();

    for (const TemplateRepository* pRepo : rRepos)
        mpRepositoryMenu->InsertItem(MNI_REPOSITORY_BASE+pRepo->mnId, pRepo->maTitle);

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

        mpRemoteView->Hide();
        mpLocalView->Show();
    }
    else
    {
        mpCurView = mpRemoteView.get();
        mpLocalView->Hide();
        mpRemoteView->Show();
    }
}

void SfxTemplateManagerDlg::localMoveTo(sal_uInt16 nItemId)
{
    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        mpLocalView->moveTemplates(maSelTemplates,nItemId);
    }
}

void SfxTemplateManagerDlg::remoteMoveTo(const sal_uInt16 nItemId)
{
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

void SfxTemplateManagerDlg::localSearchMoveTo(sal_uInt16 nItemId)
{
    if (nItemId)
    {
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
                OUString sDst = mpLocalView->getRegionItemName(nItemId);
                OUString sMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE).toString());
                sMsg = sMsg.replaceFirst("$1",sDst);
                ScopedVclPtrInstance<MessageDialog>(this, sMsg.replaceFirst( "$2",pItem->maTitle))->Execute();
            }
        }
    }

    // Deselect all items and update search results
    mpSearchView->deselectItems();

    SearchUpdateHdl(*mpSearchFilter);
}

void SfxTemplateManagerDlg::loadRepositories()
{
    uno::Reference< uno::XComponentContext > xContext(comphelper::getProcessComponentContext());

    // Load from user settings
    css::uno::Sequence<OUString>  aUrls =
            officecfg::Office::Common::Misc::TemplateRepositoryUrls::get(xContext);

    css::uno::Sequence<OUString> aNames =
            officecfg::Office::Common::Misc::TemplateRepositoryNames::get(xContext);

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
    for (TemplateRepository* pRepo : maRepositories)
    {
        if (pRepo->maTitle == rName)
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

            std::shared_ptr<const SfxFilter> pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4ClipBoardId( nFormat );

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


//   Class SfxTemplateCategoryDialog --------------------------------------------------

SfxTemplateCategoryDialog::SfxTemplateCategoryDialog( vcl::Window* pParent):
        ModalDialog(pParent, "TemplatesCategoryDialog", "sfx/ui/templatecategorydlg.ui"),
        msSelectedCategory(OUString()),
        mbIsNewCategory(false)
{
    get(mpLBCategory, "categorylb");
    get(mpNewCategoryEdit, "category_entry");
    get(mpOKButton, "ok");
    get(mpSelectLabel, "select_label");
    get(mpCreateLabel, "create_label");

    mpNewCategoryEdit->SetModifyHdl(LINK(this, SfxTemplateCategoryDialog, NewCategoryEditHdl));
    mpLBCategory->SetSelectHdl(LINK(this, SfxTemplateCategoryDialog, SelectCategoryHdl));

    mpOKButton->Disable();
}

SfxTemplateCategoryDialog::~SfxTemplateCategoryDialog()
{
    disposeOnce();
}

void SfxTemplateCategoryDialog::dispose()
{
    mpLBCategory.clear();
    mpNewCategoryEdit.clear();
    mpOKButton.clear();

    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SfxTemplateCategoryDialog, NewCategoryEditHdl, Edit&, void)
{
    OUString sParam = comphelper::string::strip(mpNewCategoryEdit->GetText(), ' ');
    mpLBCategory->Enable(sParam.isEmpty());
    if(!sParam.isEmpty())
    {
        msSelectedCategory = sParam;
        mbIsNewCategory = true;
        mpOKButton->Enable();
    }
    else
    {
        SelectCategoryHdl(*mpLBCategory);
        mbIsNewCategory = false;
    }
}

IMPL_LINK_NOARG_TYPED(SfxTemplateCategoryDialog, SelectCategoryHdl, ListBox&, void)
{
    if(mpLBCategory->GetSelectEntryPos() == 0)
    {
        msSelectedCategory = OUString();
        mpOKButton->Disable();
        mpNewCategoryEdit->Enable();
    }
    else
    {
        msSelectedCategory = mpLBCategory->GetSelectEntry();
        mpNewCategoryEdit->Disable();
        mpOKButton->Enable();
    }

    mbIsNewCategory = false;
}

void SfxTemplateCategoryDialog::SetCategoryLBEntries(std::vector<OUString> aFolderNames)
{
    if (!aFolderNames.empty())
    {
        for (size_t i = 0, n = aFolderNames.size(); i < n; ++i)
            mpLBCategory->InsertEntry(aFolderNames[i], i+1);
    }
    mpLBCategory->SelectEntryPos(0);
}

void SfxTemplateCategoryDialog::HideNewCategoryOption()
{
    mpCreateLabel->Hide();
    mpNewCategoryEdit->Hide();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
