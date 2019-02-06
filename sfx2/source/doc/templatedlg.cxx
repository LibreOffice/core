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
#include <templatesearchview.hxx>
#include <templatesearchviewitem.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
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
#include <sfx2/templateviewitem.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <sot/storage.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/langhelp.hxx>
#include <svtools/miscopt.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/edit.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

#include <sfx2/strings.hrc>
#include <bitmaps.hlst>

const char TM_SETTING_MANAGER[] = "TemplateManager";
const char TM_SETTING_LASTFOLDER[] = "LastFolder";
const char TM_SETTING_LASTAPPLICATION[] = "LastApplication";

const char ACTIONBAR_ACTION[] = "action_menu";

#define MNI_ACTION_NEW_FOLDER 1
#define MNI_ACTION_RENAME_FOLDER 2
#define MNI_ACTION_DELETE_FOLDER 3
#define MNI_ACTION_REFRESH   4
#define MNI_ACTION_DEFAULT   5
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
        : maKeyword(rKeyword.toAsciiLowerCase()), meApp(App)
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

    bool MatchSubstring( OUString const & sItemName )
    {
        if(maKeyword.isEmpty())
            return false;
        return sItemName.toAsciiLowerCase().indexOf(maKeyword) >= 0;
    }

private:

    OUString const maKeyword;
    FILTER_APPLICATION const meApp;
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
      mxDesktop( Desktop::create(comphelper::getProcessComponentContext()) )
{
    get(mpSearchFilter, "search_filter");
    get(mpCBApp, "filter_application");
    get(mpCBFolder, "filter_folder");
    get(mpActionBar, "action_action");
    get(mpLocalView, "template_view");
    get(mpSearchView, "search_view");
    get(mpOKButton, "ok");
    get(mpMoveButton, "move_btn");
    get(mpExportButton, "export_btn");
    get(mpImportButton, "import_btn");
    get(mpLinkButton, "online_link");
    get(mpCBXHideDlg, "hidedialogcb");

    // Create popup menus
    mpActionMenu = VclPtr<PopupMenu>::Create();
    mpActionMenu->InsertItem(MNI_ACTION_NEW_FOLDER,
        SfxResId(STR_CATEGORY_NEW),
        Image(StockImage::Yes, BMP_ACTION_REFRESH));
    mpActionMenu->InsertItem(MNI_ACTION_RENAME_FOLDER,
        SfxResId(STR_CATEGORY_RENAME));
    mpActionMenu->InsertItem(MNI_ACTION_DELETE_FOLDER,
        SfxResId(STR_CATEGORY_DELETE));
    mpActionMenu->InsertSeparator();
    mpActionMenu->InsertItem(MNI_ACTION_REFRESH,
        SfxResId(STR_ACTION_REFRESH),
        Image(StockImage::Yes, BMP_ACTION_REFRESH));
    mpActionMenu->InsertItem(MNI_ACTION_DEFAULT,SfxResId(STR_ACTION_DEFAULT));
    mpActionMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,MenuSelectHdl));

    mpTemplateDefaultMenu = VclPtr<PopupMenu>::Create();
    mpTemplateDefaultMenu->SetSelectHdl(LINK(this,SfxTemplateManagerDlg,DefaultTemplateMenuSelectHdl));
    mpActionMenu->SetPopupMenu(MNI_ACTION_DEFAULT,mpTemplateDefaultMenu);

    // Set toolbox button bits
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

    mpActionBar->Show();

    createDefaultTemplateMenu();

    mpLocalView->Populate();
    mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));

    mpCBApp->SelectEntryPos(0);
    fillFolderComboBox();

    mpExportButton->Disable();
    mpMoveButton->Disable();
    mpOKButton->SetText(SfxResId(STR_OPEN));

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

    // Ignore view events since we are cleaning the object
    mpLocalView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpLocalView->setOpenRegionHdl(Link<void*,void>());
    mpLocalView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());

    mpSearchView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mpSearchView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());

    mpOKButton.clear();
    mpMoveButton.clear();
    mpExportButton.clear();
    mpImportButton.clear();
    mpLinkButton.clear();
    mpCBXHideDlg.clear();
    mpSearchFilter.clear();
    mpCBApp.clear();
    mpCBFolder.clear();
    mpActionBar.clear();
    mpSearchView.clear();
    mpLocalView.clear();
    mpActionMenu.disposeAndClear();
    mpTemplateDefaultMenu.clear();

    ModalDialog::dispose();
}

short SfxTemplateManagerDlg::Execute()
{
    //use application specific settings if there's no previous setting
    getApplicationSpecificSettings();
    readSettings();

    return ModalDialog::Execute();
}

bool SfxTemplateManagerDlg::EventNotify( NotifyEvent& rNEvt )
{
    if (mpSearchFilter != nullptr &&
        mpSearchFilter->HasControlFocus() &&
        !mpSearchFilter->GetText().isEmpty() &&
        rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        const KeyEvent* pKEvt    = rNEvt.GetKeyEvent();
        vcl::KeyCode    aKeyCode = pKEvt->GetKeyCode();
        sal_uInt16      nKeyCode = aKeyCode.GetCode();

        if ( nKeyCode == KEY_ESCAPE )
        {
            mpSearchFilter->SetText("");
            mpSearchFilter->UpdateData();
            return true;
        }
    }
    return ModalDialog::EventNotify(rNEvt);
}

void SfxTemplateManagerDlg::setDocumentModel(const uno::Reference<frame::XModel> &rModel)
{
    m_xModel = rModel;
}

FILTER_APPLICATION SfxTemplateManagerDlg::getCurrentApplicationFilter()
{
    const sal_Int16 nCurAppId = mpCBApp->GetSelectedEntryPos();

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

    for (size_t i = 0, n = aFolderNames.size(); i < n; ++i)
        mpCBFolder->InsertEntry(aFolderNames[i], i+1);
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
        mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
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

    mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
    mpCBFolder->SelectEntryPos(0);
    mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
    mpLocalView->showAllTemplates();
}

void SfxTemplateManagerDlg::readSettings ()
{
    OUString aLastFolder;
    SvtViewOptions aViewSettings( EViewType::Dialog, TM_SETTING_MANAGER );

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

    mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));

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

    if (mpLocalView->getCurRegionId())
        aLastFolder = mpLocalView->getRegionName(mpLocalView->getCurRegionId()-1);

    // last folder
    Sequence< NamedValue > aSettings
    {
        { TM_SETTING_LASTFOLDER, css::uno::makeAny(aLastFolder) },
        { TM_SETTING_LASTAPPLICATION,     css::uno::makeAny(sal_uInt16(mpCBApp->GetSelectedEntryPos())) }
    };

    // write
    SvtViewOptions aViewSettings(EViewType::Dialog, TM_SETTING_MANAGER);
    aViewSettings.SetUserData(aSettings);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SelectApplicationHdl, ListBox&, void)
{
    if(mpLocalView->IsVisible())
    {
        mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpLocalView->showAllTemplates();
        mpCBFolder->SelectEntryPos(0);
        mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
    }

    if(mpSearchView->IsVisible())
        SearchUpdateHdl(*mpSearchFilter);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SelectRegionHdl, ListBox&, void)
{
    const OUString sSelectedRegion = mpCBFolder->GetSelectedEntry();

    if(mpCBFolder->GetSelectedEntryPos() == 0)
    {
        mpLocalView->showAllTemplates();
        mpActionMenu->HideItem(MNI_ACTION_RENAME_FOLDER);
    }
    else
    {
        mpLocalView->showRegion(sSelectedRegion);
        mpActionMenu->ShowItem(MNI_ACTION_RENAME_FOLDER);
    }

    if(mpSearchView->IsVisible())
        SearchUpdateHdl(*mpSearchFilter);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, TBXDropdownHdl, ToolBox*, void)
{
    const sal_uInt16 nCurItemId = mpActionBar->GetCurItemId();
    mpActionBar->SetItemDown( nCurItemId, true );

    if (nCurItemId == mpActionBar->GetItemId(ACTIONBAR_ACTION))
        mpActionMenu->Execute(mpActionBar, mpActionBar->GetItemRect(nCurItemId), PopupMenuFlags::ExecuteDown);

    mpActionBar->SetItemDown( nCurItemId, false );
    mpActionBar->EndSelection();
    mpActionBar->Invalidate();
}

IMPL_LINK(SfxTemplateManagerDlg, TVItemStateHdl, const ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

    if (pViewItem)
        OnTemplateState(pItem);
}

IMPL_LINK(SfxTemplateManagerDlg, MenuSelectHdl, Menu*, pMenu, bool)
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
        mpLocalView->reload();
        break;
    default:
        break;
    }

    return false;
}

IMPL_LINK(SfxTemplateManagerDlg, DefaultTemplateMenuSelectHdl, Menu*, pMenu, bool)
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

IMPL_LINK_NOARG(SfxTemplateManagerDlg, OkClickHdl, Button*, void)
{
   OnTemplateOpen();
   EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, MoveClickHdl, Button*, void)
{
    // modal dialog to select templates category
    SfxTemplateCategoryDialog aDlg(GetFrameWeld());
    aDlg.SetCategoryLBEntries(mpLocalView->getFolderNames());

    size_t nItemId = 0;

    if (aDlg.run() == RET_OK)
    {
        const OUString& sCategory = aDlg.GetSelectedCategory();
        bool bIsNewCategory = aDlg.IsNewCategoryCreated();
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
            nItemId = mpLocalView->getRegionId(sCategory);
    }

    if(nItemId)
    {
        if (mpSearchView->IsVisible())
            localSearchMoveTo(nItemId);
        else
            localMoveTo(nItemId);
    }

    mpLocalView->reload();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, ExportClickHdl, Button*, void)
{
    OnTemplateExport();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, ImportClickHdl, Button*, void)
{
    //Modal Dialog to select Category
    SfxTemplateCategoryDialog aDlg(GetFrameWeld());
    aDlg.SetCategoryLBEntries(mpLocalView->getFolderNames());

    if (aDlg.run() == RET_OK)
    {
        const OUString& sCategory = aDlg.GetSelectedCategory();
        bool bIsNewCategory = aDlg.IsNewCategoryCreated();
        if(bIsNewCategory)
        {
            if(mpLocalView->createRegion(sCategory))
            {
                mpCBFolder->InsertEntry(sCategory);
                OnTemplateImportCategory(sCategory);
            }
            else
            {
                OUString aMsg( SfxResId(STR_CREATE_ERROR) );
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                          aMsg.replaceFirst("$1", sCategory)));
                xBox->run();
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

IMPL_STATIC_LINK_NOARG(SfxTemplateManagerDlg, LinkClickHdl, Button*, void)
{
    OnTemplateLink();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, OpenRegionHdl, void*, void)
{
    maSelTemplates.clear();
    mpOKButton->Disable();
    mpActionBar->Show();
}

IMPL_LINK(SfxTemplateManagerDlg, CreateContextMenuHdl, ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if (pViewItem)
    {
        if(mpSearchView->IsVisible())
            mpSearchView->createContextMenu(pViewItem->IsDefaultTemplate());
        else
            mpLocalView->createContextMenu(pViewItem->IsDefaultTemplate());
    }
}


IMPL_LINK(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs(5);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= true;
    aArgs[1].Name = "MacroExecutionMode";
    aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
    aArgs[2].Name = "UpdateDocMode";
    aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;
    aArgs[3].Name = "InteractionHandler";
    aArgs[3].Value <<= task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr );
    aArgs[4].Name = "ReadOnly";
    aArgs[4].Value <<= true;

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

IMPL_LINK(SfxTemplateManagerDlg, EditTemplateHdl, ThumbnailViewItem*, pItem, void)
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

IMPL_LINK(SfxTemplateManagerDlg, DeleteTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    OUString aDeletedTemplate;

    if(mpSearchView->IsVisible())
    {
        TemplateSearchViewItem *pSrchItem = static_cast<TemplateSearchViewItem*>(pItem);

        if (!mpLocalView->removeTemplate(pSrchItem->mnAssocId, pSrchItem->mnRegionId))
        {
            aDeletedTemplate = pSrchItem->maTitle;
        }
    }
    else
    {
        TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);
        sal_uInt16 nRegionItemId = mpLocalView->getRegionId(pViewItem->mnRegionId);

        if (!mpLocalView->removeTemplate(pViewItem->mnDocId + 1, nRegionItemId))//mnId w.r.t. region is mnDocId + 1;
        {
            aDeletedTemplate = pItem->maTitle;
        }
    }

    if (!aDeletedTemplate.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_TEMPLATE) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMsg.replaceFirst("$1",aDeletedTemplate)));
        xBox->run();
    }
}

IMPL_LINK(SfxTemplateManagerDlg, DefaultTemplateHdl, ThumbnailViewItem*, pItem, void)
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

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SearchUpdateHdl, Edit&, void)
{
    OUString aKeyword = mpSearchFilter->GetText();

    if (!aKeyword.isEmpty())
    {
        mpSearchView->Clear();

        // if the search view is hidden, hide the folder view and display search one
        if (!mpSearchView->IsVisible())
        {
            mpLocalView->deselectItems();
            mpSearchView->Show();
            mpLocalView->Hide();
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
        mpLocalView->Show();
        mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpLocalView->reload();
        OUString sLastFolder = mpCBFolder->GetSelectedEntry();
        mpLocalView->showRegion(sLastFolder);
        mpActionMenu->ShowItem(MNI_ACTION_RENAME_FOLDER);
    }
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, GetFocusHdl, Control&, void)
{
    mpLocalView->deselectItems();
    mpSearchView->deselectItems();

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
    sfx2::FileDialogHelper aFileDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                    FileDialogFlags::MultiSelection, GetFrameWeld());

    // add "All" filter
    aFileDlg.AddFilter( SfxResId(STR_SFX_FILTERNAME_ALL),
                        FILEDIALOG_FILTER_ALL );

    // add template filter
    OUString sFilterExt;
    OUString sFilterName( SfxResId( STR_TEMPLATE_FILTER ) );

    // add filters of modules which are installed
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
        sFilterExt += "*.ott;*.stw;*.oth;*.dotx;*.dot";

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.ots;*.stc;*.xltx;*.xlt";
    }

    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        if ( !sFilterExt.isEmpty() )
            sFilterExt += ";";

        sFilterExt += "*.otp;*.sti;*.pot;*.potx";
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

    if ( nCode != ERRCODE_NONE )
        return;

    css::uno::Sequence<OUString> aFiles = aFileDlg.GetSelectedFiles();

    if (!aFiles.hasElements())
        return;

    //Import to the selected regions
    TemplateContainerItem* pContItem = mpLocalView->getRegion(sCategory);
    if(!pContItem)
        return;

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
        OUString aMsg(SfxResId(STR_MSG_ERROR_IMPORT));
        aMsg = aMsg.replaceFirst("$1",pContItem->maTitle);
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMsg.replaceFirst("$2",aTemplateList)));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnTemplateExport()
{
    uno::Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<XFolderPicker2> xFolderPicker = FolderPicker::create(xContext);

    xFolderPicker->setDisplayDirectory(SvtPathOptions().GetWorkPath());

    sal_Int16 nResult = xFolderPicker->execute();
    sal_Int16 nCount = maSelTemplates.size();

    if( nResult != ExecutableDialogResults::OK )
        return;

    OUString aTemplateList;
    INetURLObject aPathObj(xFolderPicker->getDirectory());
    aPathObj.setFinalSlash();

    if (mpSearchView->IsVisible())
    {
        sal_uInt16 i = 1;

        for (auto const& selTemplate : maSelTemplates)
        {
            const TemplateSearchViewItem *pItem = static_cast<const TemplateSearchViewItem*>(selTemplate);

            INetURLObject aItemPath(pItem->getPath());

            if ( 1 == i )
                aPathObj.Append(aItemPath.getName());
            else
                aPathObj.setName(aItemPath.getName());

            OUString aPath = aPathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            if (!mpLocalView->exportTo(pItem->mnAssocId,pItem->mnRegionId,aPath))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + pItem->maTitle;
            }
            ++i;
        }

        mpSearchView->deselectItems();
    }
    else
    {
        // export templates from the current view

        sal_uInt16 i = 1;

        for (auto const& selTemplate : maSelTemplates)
        {
            const TemplateViewItem *pItem = static_cast<const TemplateViewItem*>(selTemplate);

            INetURLObject aItemPath(pItem->getPath());

            if ( 1 == i )
                aPathObj.Append(aItemPath.getName());
            else
                aPathObj.setName(aItemPath.getName());

            OUString aPath = aPathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            if (!mpLocalView->exportTo(pItem->mnDocId + 1,   //mnId w.r.t. region = mDocId + 1
                mpLocalView->getRegionId(pItem->mnRegionId), //pItem->mnRegionId does not store actual region Id
                aPath))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList = aTemplateList + "\n" + pItem->maTitle;
            }
            ++i;
        }

        mpLocalView->deselectItems();
    }

    if (!aTemplateList.isEmpty())
    {
        OUString aText( SfxResId(STR_MSG_ERROR_EXPORT) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                  aText.replaceFirst("$1",aTemplateList)));
        xBox->run();
    }
    else
    {
        OUString sText( SfxResId(STR_MSG_EXPORT_SUCCESS) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Info, VclButtonsType::Ok,
                                                  sText.replaceFirst("$1", OUString::number(nCount))));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnTemplateLink ()
{
    OUString sNodePath("/org.openoffice.Office.Common/Help/StartCenter");
    try
    {
        Reference<lang::XMultiServiceFactory> xConfig = configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
        uno::Sequence<uno::Any> args(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(sNodePath)}
        }));
        Reference<container::XNameAccess> xNameAccess(xConfig->createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", args), UNO_QUERY);
        if( xNameAccess.is() )
        {
            OUString sURL;
            //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
            Any value( xNameAccess->getByName("TemplateRepositoryURL") );
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
    InputDialog dlg(GetFrameWeld(), SfxResId(STR_INPUT_NEW));

    int ret = dlg.run();

    if (!ret)
        return;

    OUString aName = dlg.GetEntryText();

    if(mpLocalView->createRegion(aName))
        mpCBFolder->InsertEntry(aName);
    else
    {
        OUString aMsg( SfxResId(STR_CREATE_ERROR) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMsg.replaceFirst("$1", aName)));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnCategoryRename()
{
    OUString sCategory = mpCBFolder->GetSelectedEntry();
    InputDialog dlg(GetFrameWeld(), SfxResId(STR_INPUT_NEW));

    dlg.SetEntryText(sCategory);
    int ret = dlg.run();

    if (!ret)
        return;

    OUString aName = dlg.GetEntryText();

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
        OUString aMsg( SfxResId(STR_CREATE_ERROR) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMsg.replaceFirst("$1", aName)));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnCategoryDelete()
{
    SfxTemplateCategoryDialog aDlg(GetFrameWeld());
    aDlg.SetCategoryLBEntries(mpLocalView->getFolderNames());
    aDlg.HideNewCategoryOption();
    aDlg.set_title(SfxResId(STR_CATEGORY_DELETE));
    aDlg.SetSelectLabelText(SfxResId(STR_CATEGORY_SELECT));

    if (aDlg.run() == RET_OK)
    {
        const OUString& sCategory = aDlg.GetSelectedCategory();
        std::unique_ptr<weld::MessageDialog> popupDlg(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo,
                                                      SfxResId(STR_QMSG_SEL_FOLDER_DELETE)));
        if (popupDlg->run() != RET_YES)
            return;

        sal_Int16 nItemId = mpLocalView->getRegionId(sCategory);

        if (!mpLocalView->removeRegion(nItemId))
        {
            OUString sMsg( SfxResId(STR_MSG_ERROR_DELETE_FOLDER) );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                      sMsg.replaceFirst("$1",sCategory)));
            xBox->run();
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

void SfxTemplateManagerDlg::createDefaultTemplateMenu ()
{
    std::vector<OUString> aList = lcl_getAllFactoryURLs();

    if (!aList.empty())
    {
        mpTemplateDefaultMenu->Clear();

        sal_uInt16 nItemId = MNI_ACTION_DEFAULT + 1;
        for (auto const& elem : aList)
        {
            INetURLObject aObj(elem);
            OUString aTitle = SvFileInformationManager::GetDescription(aObj);
            mpTemplateDefaultMenu->InsertItem(nItemId, aTitle, SvFileInformationManager::GetImage(aObj));
            mpTemplateDefaultMenu->SetItemCommand(nItemId++, elem);
        }

        mpActionMenu->ShowItem(MNI_ACTION_DEFAULT);
    }
    else
        mpActionMenu->HideItem(MNI_ACTION_DEFAULT);
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

void SfxTemplateManagerDlg::localSearchMoveTo(sal_uInt16 nItemId)
{
    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates; //Copy to avoid invalidating an iterator

        for (auto const& selTemplate : aSelTemplates)
        {
            const TemplateSearchViewItem *pItem =
                    static_cast<const TemplateSearchViewItem*>(selTemplate);

            if(!mpLocalView->moveTemplate(pItem,pItem->mnRegionId,nItemId))
            {
                OUString sDst = mpLocalView->getRegionItemName(nItemId);
                OUString sMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE));
                sMsg = sMsg.replaceFirst("$1",sDst);
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok,
                                                          sMsg.replaceFirst( "$2",pItem->maTitle)));
                xBox->run();
            }
        }
    }

    // Deselect all items and update search results
    mpSearchView->deselectItems();

    SearchUpdateHdl(*mpSearchFilter);
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

SfxTemplateCategoryDialog::SfxTemplateCategoryDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "sfx/ui/templatecategorydlg.ui", "TemplatesCategoryDialog")
    , msSelectedCategory(OUString())
    , mbIsNewCategory(false)
    , mxLBCategory(m_xBuilder->weld_tree_view("categorylb"))
    , mxSelectLabel(m_xBuilder->weld_label("select_label"))
    , mxNewCategoryEdit(m_xBuilder->weld_entry("category_entry"))
    , mxCreateLabel(m_xBuilder->weld_label("create_label"))
    , mxOKButton(m_xBuilder->weld_button("ok"))
{
    mxLBCategory->append_text(SfxResId(STR_CATEGORY_NONE));
    mxNewCategoryEdit->connect_changed(LINK(this, SfxTemplateCategoryDialog, NewCategoryEditHdl));
    mxLBCategory->set_size_request(mxLBCategory->get_approximate_digit_width() * 32,
                                   mxLBCategory->get_height_rows(8));
    mxLBCategory->connect_changed(LINK(this, SfxTemplateCategoryDialog, SelectCategoryHdl));
    mxOKButton->set_sensitive(false);
}

SfxTemplateCategoryDialog::~SfxTemplateCategoryDialog()
{
}

IMPL_LINK_NOARG(SfxTemplateCategoryDialog, NewCategoryEditHdl, weld::Entry&, void)
{
    OUString sParam = comphelper::string::strip(mxNewCategoryEdit->get_text(), ' ');
    mxLBCategory->set_sensitive(sParam.isEmpty());
    if(!sParam.isEmpty())
    {
        msSelectedCategory = sParam;
        mbIsNewCategory = true;
        mxOKButton->set_sensitive(true);
    }
    else
    {
        SelectCategoryHdl(*mxLBCategory);
        mbIsNewCategory = false;
    }
}

IMPL_LINK_NOARG(SfxTemplateCategoryDialog, SelectCategoryHdl, weld::TreeView&, void)
{
    if (mxLBCategory->get_selected_index() == 0)
    {
        msSelectedCategory = OUString();
        mxOKButton->set_sensitive(false);
        mxNewCategoryEdit->set_sensitive(true);
    }
    else
    {
        msSelectedCategory = mxLBCategory->get_selected_text();
        mxNewCategoryEdit->set_sensitive(false);
        mxOKButton->set_sensitive(true);
    }

    mbIsNewCategory = false;
}

void SfxTemplateCategoryDialog::SetCategoryLBEntries(std::vector<OUString> aFolderNames)
{
    for (size_t i = 0, n = aFolderNames.size(); i < n; ++i)
        mxLBCategory->append_text(aFolderNames[i]);
    mxLBCategory->select(0);
}

void SfxTemplateCategoryDialog::HideNewCategoryOption()
{
    mxCreateLabel->hide();
    mxNewCategoryEdit->hide();
}

// SfxTemplateSelectionDialog -----------------------------------------------------------------

SfxTemplateSelectionDlg::SfxTemplateSelectionDlg(vcl::Window* pParent):
    SfxTemplateManagerDlg(pParent),
    msTemplatePath(OUString())
{
    mpCBApp->SelectEntryPos(MNI_IMPRESS);
    mpCBFolder->SelectEntryPos(0);
    SetText(SfxResId(STR_TEMPLATE_SELECTION));

    if(mpLocalView->IsVisible())
    {
        mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpLocalView->showAllTemplates();
    }

    mpCBApp->Disable();
    mpActionBar->Hide();
    mpLinkButton->Hide();
    mpMoveButton->Hide();
    mpExportButton->Hide();
    mpCBXHideDlg->Show();
    mpCBXHideDlg->Check();

    mpLocalView->setOpenTemplateHdl(LINK(this,SfxTemplateSelectionDlg, OpenTemplateHdl));
    mpSearchView->setOpenTemplateHdl(LINK(this,SfxTemplateSelectionDlg, OpenTemplateHdl));

    mpLocalView->SetMultiSelectionEnabled(false);
    mpSearchView->SetMultiSelectionEnabled(false);

    mpOKButton->SetClickHdl(LINK(this, SfxTemplateSelectionDlg, OkClickHdl));
}

SfxTemplateSelectionDlg::~SfxTemplateSelectionDlg()
{
    disposeOnce();
}

void SfxTemplateSelectionDlg::dispose()
{
    SfxTemplateManagerDlg::dispose();
}

short SfxTemplateSelectionDlg::Execute()
{
    return ModalDialog::Execute();
}

IMPL_LINK(SfxTemplateSelectionDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);
    msTemplatePath = pViewItem->getPath();

    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SfxTemplateSelectionDlg, OkClickHdl, Button*, void)
{
   TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(const_cast<ThumbnailViewItem*>(*maSelTemplates.begin()));
   msTemplatePath = pViewItem->getPath();

   EndDialog(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
