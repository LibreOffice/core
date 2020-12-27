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
#include <sfx2/module.hxx>
#include <templatesearchview.hxx>
#include <templatesearchviewitem.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatedlglocalview.hxx>
#include <templatecontaineritem.hxx>
#include <templateviewitem.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <sot/storage.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/event.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <comphelper/dispatchcommand.hxx>

#include <sfx2/strings.hrc>

constexpr OUStringLiteral TM_SETTING_MANAGER = u"TemplateManager";
constexpr OUStringLiteral TM_SETTING_LASTFOLDER = u"LastFolder";
constexpr OUStringLiteral TM_SETTING_LASTAPPLICATION = u"LastApplication";
constexpr OUStringLiteral TM_SETTING_VIEWMODE = u"ViewMode";

#define MNI_ACTION_NEW_FOLDER "new"
#define MNI_ACTION_RENAME_FOLDER "rename"
#define MNI_ACTION_DELETE_FOLDER "delete"
#define MNI_ACTION_REFRESH   "refresh"
#define MNI_ACTION_DEFAULT   "default"
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

namespace {

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

    OUString maKeyword;
    FILTER_APPLICATION meApp;
};

}

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

SfxTemplateManagerDlg::SfxTemplateManagerDlg(weld::Window *pParent)
    : GenericDialogController(pParent, "sfx/ui/templatedlg.ui", "TemplateDialog")
    , maSelTemplates(cmpSelectionItems)
    , mxDesktop(Desktop::create(comphelper::getProcessComponentContext()))
    , m_aUpdateDataTimer("UpdateDataTimer")
    , mxSearchFilter(m_xBuilder->weld_entry("search_filter"))
    , mxCBApp(m_xBuilder->weld_combo_box("filter_application"))
    , mxCBFolder(m_xBuilder->weld_combo_box("filter_folder"))
    , mxOKButton(m_xBuilder->weld_button("ok"))
    , mxMoveButton(m_xBuilder->weld_button("move_btn"))
    , mxExportButton(m_xBuilder->weld_button("export_btn"))
    , mxImportButton(m_xBuilder->weld_button("import_btn"))
    , mxMoreTemplatesButton(m_xBuilder->weld_button("btnMoreTemplates"))
    , mxCBXHideDlg(m_xBuilder->weld_check_button("hidedialogcb"))
    , mxActionBar(m_xBuilder->weld_menu_button("action_menu"))
    , mxSearchView(new TemplateSearchView(m_xBuilder->weld_scrolled_window("scrollsearch", true),
                                          m_xBuilder->weld_menu("contextmenu1"),
                                          m_xBuilder->weld_tree_view("treesearch_list")))
    , mxLocalView(new TemplateDlgLocalView(m_xBuilder->weld_scrolled_window("scrolllocal", true),
                                           m_xBuilder->weld_menu("contextmenu2"),
                                           m_xBuilder->weld_tree_view("tree_list")))
    , mxTemplateDefaultMenu(m_xBuilder->weld_menu("submenu"))
    , mxSearchViewWeld(new weld::CustomWeld(*m_xBuilder, "search_view", *mxSearchView))
    , mxLocalViewWeld(new weld::CustomWeld(*m_xBuilder, "template_view", *mxLocalView))
    , mxListViewButton(m_xBuilder->weld_toggle_button("list_view_btn"))
    , mxThumbnailViewButton(m_xBuilder->weld_toggle_button("thumbnail_view_btn"))
    , mViewMode(TemplateViewMode::eThumbnailView)
{
    // Create popup menus
    mxActionBar->insert_item(0, MNI_ACTION_NEW_FOLDER, SfxResId(STR_CATEGORY_NEW), nullptr, nullptr, TRISTATE_INDET);
    mxActionBar->insert_item(1, MNI_ACTION_RENAME_FOLDER, SfxResId(STR_CATEGORY_RENAME), nullptr, nullptr, TRISTATE_INDET);
    mxActionBar->insert_item(2, MNI_ACTION_DELETE_FOLDER, SfxResId(STR_CATEGORY_DELETE), nullptr, nullptr, TRISTATE_INDET);
    mxActionBar->insert_separator(3, "separator");
    mxActionBar->insert_item(4, MNI_ACTION_REFRESH, SfxResId(STR_ACTION_REFRESH), nullptr, nullptr, TRISTATE_INDET);
    mxActionBar->connect_selected(LINK(this,SfxTemplateManagerDlg,MenuSelectHdl));

    mxLocalView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);
    mxLocalView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_MAX_HEIGHT-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                              TEMPLATE_ITEM_PADDING);

    mxLocalView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mxLocalView->setCreateContextMenuHdl(LINK(this,SfxTemplateManagerDlg, CreateContextMenuHdl));
    mxLocalView->setOpenRegionHdl(LINK(this,SfxTemplateManagerDlg, OpenRegionHdl));
    mxLocalView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg, OpenTemplateHdl));
    mxLocalView->setEditTemplateHdl(LINK(this,SfxTemplateManagerDlg, EditTemplateHdl));
    mxLocalView->setDeleteTemplateHdl(LINK(this,SfxTemplateManagerDlg, DeleteTemplateHdl));
    mxLocalView->setDefaultTemplateHdl(LINK(this,SfxTemplateManagerDlg, DefaultTemplateHdl));

    mxSearchView->setItemMaxTextLength(TEMPLATE_ITEM_MAX_TEXT_LENGTH);

    mxSearchView->setItemDimensions(TEMPLATE_ITEM_MAX_WIDTH,TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_MAX_HEIGHT_SUB-TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT,
                                    TEMPLATE_ITEM_PADDING);

    mxSearchView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVItemStateHdl));
    mxSearchView->setCreateContextMenuHdl(LINK(this,SfxTemplateManagerDlg, CreateContextMenuHdl));
    mxSearchView->setOpenTemplateHdl(LINK(this,SfxTemplateManagerDlg,OpenTemplateHdl));
    mxSearchView->setEditTemplateHdl(LINK(this,SfxTemplateManagerDlg, EditTemplateHdl));
    mxSearchView->setDeleteTemplateHdl(LINK(this,SfxTemplateManagerDlg, DeleteTemplateHdl));
    mxSearchView->setDefaultTemplateHdl(LINK(this,SfxTemplateManagerDlg, DefaultTemplateHdl));

    mxLocalView->ShowTooltips(true);
    mxSearchView->ShowTooltips(true);

    mxOKButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, OkClickHdl));
    mxMoveButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, MoveClickHdl));
    mxExportButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, ExportClickHdl));
    mxImportButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, ImportClickHdl));
    // FIXME: rather than disabling make dispatchCommand(".uno:AdditionsDialog") work in start center
    if ( !SfxModule::GetActiveModule() )
        mxMoreTemplatesButton->set_sensitive(false);
    else
        mxMoreTemplatesButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, LinkClickHdl));
    mxListViewButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, ListViewHdl));
    mxThumbnailViewButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, ThumbnailViewHdl));

    mxSearchFilter->connect_changed(LINK(this, SfxTemplateManagerDlg, SearchUpdateHdl));
    mxSearchFilter->connect_focus_in(LINK( this, SfxTemplateManagerDlg, GetFocusHdl ));
    mxSearchFilter->connect_focus_out(LINK( this, SfxTemplateManagerDlg, LoseFocusHdl ));
    mxSearchFilter->connect_key_press(LINK( this, SfxTemplateManagerDlg, KeyInputHdl));

    mxActionBar->show();

    createDefaultTemplateMenu();

    mxLocalView->Populate();
    mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));

    mxCBApp->set_active(0);
    fillFolderComboBox();

    mxExportButton->set_sensitive(false);
    mxMoveButton->set_sensitive(false);
    mxOKButton->set_label(SfxResId(STR_OPEN));

    mxCBApp->connect_changed(LINK(this, SfxTemplateManagerDlg, SelectApplicationHdl));
    mxCBFolder->connect_changed(LINK(this, SfxTemplateManagerDlg, SelectRegionHdl));

    mxLocalView->Show();

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SfxTemplateManagerDlg, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetDebugName( "SfxTemplateManagerDlg UpdateDataTimer" );
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    mxLocalView->connect_focus_rect(LINK(this, SfxTemplateManagerDlg, FocusRectLocalHdl));
    mxSearchView->connect_focus_rect(LINK(this, SfxTemplateManagerDlg, FocusRectSearchHdl));
    bMakeSelItemVisible = false;
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg()
{
    writeSettings();

    // Ignore view events since we are cleaning the object
    mxLocalView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mxLocalView->setOpenRegionHdl(Link<void*,void>());
    mxLocalView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());
    mxSearchView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mxSearchView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());
}

short SfxTemplateManagerDlg::run()
{
    //use application specific settings if there's no previous setting
    getApplicationSpecificSettings();
    readSettings();

    return weld::GenericDialogController::run();
}

IMPL_LINK(SfxTemplateManagerDlg, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    if (mxSearchFilter != nullptr && !mxSearchFilter->get_text().isEmpty())
    {
        vcl::KeyCode    aKeyCode = rKeyEvent.GetKeyCode();
        sal_uInt16      nKeyCode = aKeyCode.GetCode();

        if ( nKeyCode == KEY_ESCAPE )
        {
            mxSearchFilter->set_text("");
            SearchUpdateHdl(*mxSearchFilter);
            return true;
        }
    }
    return false;
}

void SfxTemplateManagerDlg::setDocumentModel(const uno::Reference<frame::XModel> &rModel)
{
    m_xModel = rModel;
}

void SfxTemplateManagerDlg::setTemplateViewMode(TemplateViewMode eViewMode)
{
    mViewMode = eViewMode;
    mxLocalView->setTemplateViewMode(eViewMode);
    mxSearchView->setTemplateViewMode(eViewMode);
    if ( mViewMode == TemplateViewMode::eListView)
    {
        mxThumbnailViewButton->set_active(false);
        mxListViewButton->set_active(true);
    }
    else
    {
        mxThumbnailViewButton->set_active(true);
        mxListViewButton->set_active(false);
    }

    if (! mxSearchFilter->get_text().isEmpty())
    {
        mxSearchView->Show();
        mxLocalView->Hide();
    }
    else
    {
        mxSearchView->Hide();
        mxLocalView->Show();
    }
}

TemplateViewMode SfxTemplateManagerDlg::getTemplateViewMode()
{
    return mViewMode;
}


FILTER_APPLICATION SfxTemplateManagerDlg::getCurrentApplicationFilter() const
{
    const sal_Int16 nCurAppId = mxCBApp->get_active();

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
    std::vector<OUString> aFolderNames = mxLocalView->getFolderNames();

    for (size_t i = 0, n = aFolderNames.size(); i < n; ++i)
        mxCBFolder->append_text(aFolderNames[i]);
    mxCBFolder->set_active(0);
    mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
}

void SfxTemplateManagerDlg::getApplicationSpecificSettings()
{
    if ( ! m_xModel.is() )
    {
        mxCBApp->set_active(0);
        mxCBFolder->set_active(0);
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
        mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mxLocalView->showAllTemplates();
        return;
    }

    SvtModuleOptions::EFactory eFactory = SvtModuleOptions::ClassifyFactoryByModel(m_xModel);

    switch(eFactory)
    {
        case SvtModuleOptions::EFactory::WRITER:
        case SvtModuleOptions::EFactory::WRITERWEB:
        case SvtModuleOptions::EFactory::WRITERGLOBAL:
                            mxCBApp->set_active(MNI_WRITER);
                            break;
        case SvtModuleOptions::EFactory::CALC:
                            mxCBApp->set_active(MNI_CALC);
                            break;
        case SvtModuleOptions::EFactory::IMPRESS:
                            mxCBApp->set_active(MNI_IMPRESS);
                            break;
        case SvtModuleOptions::EFactory::DRAW:
                            mxCBApp->set_active(MNI_DRAW);
                            break;
        default:
                mxCBApp->set_active(0);
                break;
    }

    mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
    mxCBFolder->set_active(0);
    mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
    mxLocalView->showAllTemplates();
}

void SfxTemplateManagerDlg::readSettings ()
{
    OUString aLastFolder;
    SvtViewOptions aViewSettings( EViewType::Dialog, TM_SETTING_MANAGER );
    sal_Int16 nViewMode = -1;

    if ( aViewSettings.Exists() )
    {
        sal_uInt16 nTmp = 0;
        aViewSettings.GetUserItem(TM_SETTING_LASTFOLDER) >>= aLastFolder;
        aViewSettings.GetUserItem(TM_SETTING_LASTAPPLICATION) >>= nTmp;
        aViewSettings.GetUserItem(TM_SETTING_VIEWMODE) >>= nViewMode;

        //open last remembered application only when application model is not set
        if(!m_xModel.is())
        {
            switch (nTmp)
            {
                case MNI_WRITER:
                    mxCBApp->set_active(MNI_WRITER);
                    break;
                case MNI_CALC:
                    mxCBApp->set_active(MNI_CALC);
                    break;
                case MNI_IMPRESS:
                    mxCBApp->set_active(MNI_IMPRESS);
                    break;
                case MNI_DRAW:
                    mxCBApp->set_active(MNI_DRAW);
                    break;
                default:
                    mxCBApp->set_active(0);
                    break;
            }
        }
    }

    mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));

    if (aLastFolder.isEmpty())
    {
        //show all categories
        mxCBFolder->set_active(0);
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
        mxLocalView->showAllTemplates();
    }
    else
    {
        mxCBFolder->set_active_text(aLastFolder);
        mxLocalView->showRegion(aLastFolder);
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, true);
    }

    if(nViewMode == static_cast<sal_Int16>(TemplateViewMode::eListView) ||
        nViewMode == static_cast<sal_Int16>(TemplateViewMode::eThumbnailView))
    {
        TemplateViewMode eViewMode = static_cast<TemplateViewMode>(nViewMode);
        setTemplateViewMode(eViewMode);
    }
    else
    {
        //Default ViewMode
        setTemplateViewMode(TemplateViewMode::eThumbnailView);
    }
}

void SfxTemplateManagerDlg::writeSettings ()
{
    OUString aLastFolder;

    if (mxLocalView->getCurRegionId())
        aLastFolder = mxLocalView->getRegionName(mxLocalView->getCurRegionId()-1);

    // last folder
    Sequence< NamedValue > aSettings
    {
        { TM_SETTING_LASTFOLDER, css::uno::makeAny(aLastFolder) },
        { TM_SETTING_LASTAPPLICATION,     css::uno::makeAny(sal_uInt16(mxCBApp->get_active())) },
        { TM_SETTING_VIEWMODE, css::uno::makeAny(static_cast<sal_Int16>(getTemplateViewMode()))}
    };

    // write
    SvtViewOptions aViewSettings(EViewType::Dialog, TM_SETTING_MANAGER);
    aViewSettings.SetUserData(aSettings);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SelectApplicationHdl, weld::ComboBox&, void)
{
    if (mxLocalView->IsVisible())
    {
        mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mxLocalView->showAllTemplates();
        mxCBFolder->set_active(0);
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
    }
    if (mxSearchView->IsVisible())
        SearchUpdate();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SelectRegionHdl, weld::ComboBox&, void)
{
    const OUString sSelectedRegion = mxCBFolder->get_active_text();

    if(mxCBFolder->get_active() == 0)
    {
        mxLocalView->showAllTemplates();
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
    }
    else
    {
        mxLocalView->showRegion(sSelectedRegion);
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, true);
    }
    if (mxSearchView->IsVisible())
        SearchUpdate();
}

IMPL_LINK(SfxTemplateManagerDlg, TVItemStateHdl, const ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

    if (pViewItem)
        OnTemplateState(pItem);
}

IMPL_LINK(SfxTemplateManagerDlg, MenuSelectHdl, const OString&, rIdent, void)
{
    if (rIdent == MNI_ACTION_NEW_FOLDER)
        OnCategoryNew();
    else if (rIdent == MNI_ACTION_RENAME_FOLDER)
        OnCategoryRename();
    else if (rIdent == MNI_ACTION_DELETE_FOLDER)
        OnCategoryDelete();
    else if (rIdent == MNI_ACTION_REFRESH)
    {
        mxLocalView->reload();
        if(mxSearchView->IsVisible())
            SearchUpdateHdl(*mxSearchFilter);
    }
    else if (rIdent != MNI_ACTION_DEFAULT)
        DefaultTemplateMenuSelectHdl(rIdent);
}

void SfxTemplateManagerDlg::DefaultTemplateMenuSelectHdl(std::string_view rIdent)
{
    OUString aServiceName = SfxObjectShell::GetServiceNameFromFactory(OUString::fromUtf8(rIdent));

    OUString sPrevDefault = SfxObjectFactory::GetStandardTemplate( aServiceName );
    if(!sPrevDefault.isEmpty())
    {
        if(mxSearchView->IsVisible())
            mxSearchView->RemoveDefaultTemplateIcon(sPrevDefault);
        else
            mxLocalView->RemoveDefaultTemplateIcon(sPrevDefault);
    }

    SfxObjectFactory::SetStandardTemplate( aServiceName, OUString() );
    if (mxSearchView->IsVisible())
        mxSearchView->refreshDefaultColumn();
    else
        mxLocalView->refreshDefaultColumn();
    createDefaultTemplateMenu();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, OkClickHdl, weld::Button&, void)
{
   OnTemplateOpen();
   m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, MoveClickHdl, weld::Button&, void)
{
    // modal dialog to select templates category
    SfxTemplateCategoryDialog aDlg(m_xDialog.get());
    aDlg.SetCategoryLBEntries(mxLocalView->getFolderNames());

    size_t nItemId = 0;

    if (aDlg.run() == RET_OK)
    {
        const OUString& sCategory = aDlg.GetSelectedCategory();
        bool bIsNewCategory = aDlg.IsNewCategoryCreated();
        if(bIsNewCategory)
        {
            if (!sCategory.isEmpty())
            {
                nItemId = mxLocalView->createRegion(sCategory);
                if(nItemId)
                    mxCBFolder->append_text(sCategory);
            }
        }
        else
            nItemId = mxLocalView->getRegionId(sCategory);
    }

    if(nItemId)
    {
        if (mxSearchView->IsVisible())
            localSearchMoveTo(nItemId);
        else
            localMoveTo(nItemId);
    }

    mxLocalView->reload();
    if (mxSearchView->IsVisible())
        SearchUpdate();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, ExportClickHdl, weld::Button&, void)
{
    OnTemplateExport();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, ImportClickHdl, weld::Button&, void)
{
    //Modal Dialog to select Category
    SfxTemplateCategoryDialog aDlg(m_xDialog.get());
    aDlg.SetCategoryLBEntries(mxLocalView->getFolderNames());

    if (aDlg.run() == RET_OK)
    {
        const OUString& sCategory = aDlg.GetSelectedCategory();
        bool bIsNewCategory = aDlg.IsNewCategoryCreated();
        if(bIsNewCategory)
        {
            if(mxLocalView->createRegion(sCategory))
            {
                mxCBFolder->append_text(sCategory);
                OnTemplateImportCategory(sCategory);
            }
            else
            {
                OUString aMsg( SfxResId(STR_CREATE_ERROR) );
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok,
                                                          aMsg.replaceFirst("$1", sCategory)));
                xBox->run();
                return;
            }
        }
        else
            OnTemplateImportCategory(sCategory);
    }

    mxLocalView->reload();
    mxLocalView->showAllTemplates();
    if (mxSearchView->IsVisible())
        SearchUpdate();
    mxCBApp->set_active(0);
    mxCBFolder->set_active(0);
    mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
}

IMPL_STATIC_LINK_NOARG(SfxTemplateManagerDlg, LinkClickHdl, weld::Button&, void)
{
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "AdditionsTag";
    aArgs[0].Value <<= OUString("Templates");
    comphelper::dispatchCommand(".uno:AdditionsDialog", aArgs);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, OpenRegionHdl, void*, void)
{
    maSelTemplates.clear();
    mxOKButton->set_sensitive(false);
    mxActionBar->show();
}

IMPL_LINK(SfxTemplateManagerDlg, CreateContextMenuHdl, ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if (pViewItem)
    {
        if (mxSearchView->IsVisible())
            mxSearchView->createContextMenu(pViewItem->IsDefaultTemplate(), TemplateLocalView::IsInternalTemplate(pViewItem->getPath()));
        else
            mxLocalView->createContextMenu(pViewItem->IsDefaultTemplate(), TemplateLocalView::IsInternalTemplate(pViewItem->getPath()));
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

    m_xDialog->response(RET_OK);
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

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SfxTemplateManagerDlg, DeleteTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    OUString aDeletedTemplate;
    if(mxSearchView->IsVisible())
    {
        TemplateSearchViewItem *pSrchItem = static_cast<TemplateSearchViewItem*>(pItem);

        if (!mxLocalView->removeTemplate(pSrchItem->mnAssocId, pSrchItem->mnRegionId))
        {
            aDeletedTemplate = pSrchItem->maTitle;
        }
    }
    else
    {
        TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);
        sal_uInt16 nRegionItemId = mxLocalView->getRegionId(pViewItem->mnRegionId);

        if (!mxLocalView->removeTemplate(pViewItem->mnDocId + 1, nRegionItemId))//mnId w.r.t. region is mnDocId + 1;
        {
            aDeletedTemplate = pItem->maTitle;
        }
    }

    if (!aDeletedTemplate.isEmpty())
    {
        OUString aMsg( SfxResId(STR_MSG_ERROR_DELETE_TEMPLATE) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
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
            {
                if(mxSearchView->IsVisible())
                    mxSearchView->RemoveDefaultTemplateIcon(sPrevDefault);
                else
                    mxLocalView->RemoveDefaultTemplateIcon(sPrevDefault);
            }
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

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SearchUpdateHdl, weld::Entry&, void)
{
    m_aUpdateDataTimer.Start();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, ImplUpdateDataHdl, Timer*, void)
{
    SearchUpdate();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, LoseFocusHdl, weld::Widget&, void)
{
    if (m_aUpdateDataTimer.IsActive())
    {
        m_aUpdateDataTimer.Stop();
        m_aUpdateDataTimer.Invoke();
    }
}

IMPL_LINK_NOARG ( SfxTemplateManagerDlg, ListViewHdl, weld::Button&, void )
{
    setTemplateViewMode(TemplateViewMode::eListView);

    if (mxSearchFilter->get_text().isEmpty())
        mxLocalView->ListView::grab_focus();
    else
        mxSearchView->ListView::grab_focus();
}

IMPL_LINK_NOARG ( SfxTemplateManagerDlg, ThumbnailViewHdl, weld::Button&, void )
{
    setTemplateViewMode(TemplateViewMode::eThumbnailView);
    bMakeSelItemVisible = true;

    if (mxSearchFilter->get_text().isEmpty())
        mxLocalView->ThumbnailView::GrabFocus();
    else
        mxSearchView->ThumbnailView::GrabFocus();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, FocusRectLocalHdl, weld::Widget&, tools::Rectangle)
{
    if(bMakeSelItemVisible && !maSelTemplates.empty())
        mxLocalView->MakeItemVisible((*maSelTemplates.begin())->mnId);
    bMakeSelItemVisible = false;
    return tools::Rectangle();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, FocusRectSearchHdl, weld::Widget&, tools::Rectangle)
{
    if(bMakeSelItemVisible && !maSelTemplates.empty())
        mxSearchView->MakeItemVisible((*maSelTemplates.begin())->mnId);
    bMakeSelItemVisible = false;
    return tools::Rectangle();
}

void SfxTemplateManagerDlg::SearchUpdate()
{
    OUString aKeyword = mxSearchFilter->get_text();

    if (!aKeyword.isEmpty())
    {
        mxSearchView->Clear();

        // if the search view is hidden, hide the folder view and display search one
        if (!mxSearchView->IsVisible())
        {
            mxLocalView->deselectItems();
            mxSearchView->Show();
            mxLocalView->Hide();
        }

        std::vector<TemplateItemProperties> aItems =
                mxLocalView->getFilteredItems(SearchView_Keyword(aKeyword, getCurrentApplicationFilter()));

        for (const TemplateItemProperties& rItem : aItems)
        {
            OUString aFolderName = mxLocalView->getRegionName(rItem.nRegionId);

            mxSearchView->AppendItem(rItem.nId,mxLocalView->getRegionId(rItem.nRegionId),
                                     rItem.nDocId,
                                     rItem.aName,
                                     aFolderName,
                                     rItem.aPath,
                                     rItem.aThumbnail);
        }
        mxSearchView->sort();
        mxSearchView->Invalidate();
    }
    else
    {
        mxSearchView->deselectItems();
        mxSearchView->Hide();
        mxLocalView->Show();
        mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mxLocalView->reload();
        if(mxSearchView->IsVisible())
            SearchUpdate();
        OUString sLastFolder = mxCBFolder->get_active_text();
        mxLocalView->showRegion(sLastFolder);
        mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, true);
    }
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, GetFocusHdl, weld::Widget&, void)
{
    mxLocalView->deselectItems();
    mxSearchView->deselectItems();

    maSelTemplates.clear();
}

void SfxTemplateManagerDlg::OnTemplateState (const ThumbnailViewItem *pItem)
{
    bool bInSelection = maSelTemplates.find(pItem) != maSelTemplates.end();

    if (pItem->isSelected())
    {
        if (maSelTemplates.empty())
        {
            mxOKButton->set_sensitive(true);
        }
        else if (maSelTemplates.size() != 1 || !bInSelection)
        {
            mxOKButton->set_sensitive(false);
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
                mxOKButton->set_sensitive(false);
            }
            else if (maSelTemplates.size() == 1)
            {
                mxOKButton->set_sensitive(true);
            }
        }
    }

    if(maSelTemplates.empty())
    {
        mxMoveButton->set_sensitive(false);
        mxExportButton->set_sensitive(false);
    }
    else
    {
        mxMoveButton->set_sensitive(true);
        mxExportButton->set_sensitive(true);
    }
}

void SfxTemplateManagerDlg::OnTemplateImportCategory(std::u16string_view sCategory)
{
    sfx2::FileDialogHelper aFileDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                    FileDialogFlags::MultiSelection, m_xDialog.get());

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

    sFilterName += " (" + sFilterExt + ")";

    aFileDlg.AddFilter( sFilterName, sFilterExt );
    aFileDlg.SetCurrentFilter( sFilterName );

    ErrCode nCode = aFileDlg.Execute();

    if ( nCode != ERRCODE_NONE )
        return;

    const css::uno::Sequence<OUString> aFiles = aFileDlg.GetSelectedFiles();

    if (!aFiles.hasElements())
        return;

    //Import to the selected regions
    TemplateContainerItem* pContItem = mxLocalView->getRegion(sCategory);
    if(!pContItem)
        return;

    OUString aTemplateList;

    for (const auto& rFile : aFiles)
    {
        if(!mxLocalView->copyFrom(pContItem, rFile))
        {
            if (aTemplateList.isEmpty())
                aTemplateList = rFile;
            else
                aTemplateList += "\n" + rFile;
        }
    }

    if (!aTemplateList.isEmpty())
    {
        OUString aMsg(SfxResId(STR_MSG_ERROR_IMPORT));
        aMsg = aMsg.replaceFirst("$1",pContItem->maTitle);
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
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

    if (mxSearchView->IsVisible())
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

            if (!mxLocalView->exportTo(pItem->mnAssocId,pItem->mnRegionId,aPath))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList += "\n" + pItem->maTitle;
            }
            ++i;
        }

        mxSearchView->deselectItems();
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

            if (!mxLocalView->exportTo(pItem->mnDocId + 1,   //mnId w.r.t. region = mDocId + 1
                mxLocalView->getRegionId(pItem->mnRegionId), //pItem->mnRegionId does not store actual region Id
                aPath))
            {
                if (aTemplateList.isEmpty())
                    aTemplateList = pItem->maTitle;
                else
                    aTemplateList += "\n" + pItem->maTitle;
            }
            ++i;
        }

        mxLocalView->deselectItems();
    }

    if (!aTemplateList.isEmpty())
    {
        OUString aText( SfxResId(STR_MSG_ERROR_EXPORT) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aText.replaceFirst("$1",aTemplateList)));
        xBox->run();
    }
    else
    {
        OUString sText( SfxResId(STR_MSG_EXPORT_SUCCESS) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                  sText.replaceFirst("$1", OUString::number(nCount))));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnTemplateOpen ()
{
    ThumbnailViewItem *pItem = const_cast<ThumbnailViewItem*>(*maSelTemplates.begin());

    OpenTemplateHdl(pItem);
}

void SfxTemplateManagerDlg::OnCategoryNew()
{
    InputDialog dlg(m_xDialog.get(), SfxResId(STR_INPUT_NEW));

    int ret = dlg.run();

    if (!ret)
        return;

    OUString aName = dlg.GetEntryText();

    if(mxLocalView->createRegion(aName))
        mxCBFolder->append_text(aName);
    else
    {
        OUString aMsg( SfxResId(STR_CREATE_ERROR) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMsg.replaceFirst("$1", aName)));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnCategoryRename()
{
    OUString sCategory = mxCBFolder->get_active_text();
    InputDialog dlg(m_xDialog.get(), SfxResId(STR_INPUT_NEW));

    dlg.SetEntryText(sCategory);
    int ret = dlg.run();

    if (!ret)
        return;

    OUString aName = dlg.GetEntryText();

    if(mxLocalView->renameRegion(sCategory, aName))
    {
        sal_Int32 nPos = mxCBFolder->find_text(sCategory);
        mxCBFolder->remove(nPos);
        mxCBFolder->insert_text(nPos, aName);
        mxCBFolder->set_active(nPos);

        mxLocalView->reload();
        mxLocalView->showRegion(aName);
    }
    else
    {
        OUString aMsg( SfxResId(STR_CREATE_ERROR) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMsg.replaceFirst("$1", aName)));
        xBox->run();
    }
}

void SfxTemplateManagerDlg::OnCategoryDelete()
{
    SfxTemplateCategoryDialog aDlg(m_xDialog.get());
    aDlg.SetCategoryLBEntries(mxLocalView->getFolderNames());
    aDlg.HideNewCategoryOption();
    aDlg.set_title(MnemonicGenerator::EraseAllMnemonicChars(SfxResId(STR_CATEGORY_DELETE)));
    aDlg.SetSelectLabelText(SfxResId(STR_CATEGORY_SELECT));

    if (aDlg.run() == RET_OK)
    {
        const OUString& sCategory = aDlg.GetSelectedCategory();
        std::unique_ptr<weld::MessageDialog> popupDlg(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Question, VclButtonsType::YesNo,
                                                      SfxResId(STR_QMSG_SEL_FOLDER_DELETE)));
        if (popupDlg->run() != RET_YES)
            return;

        sal_Int16 nItemId = mxLocalView->getRegionId(sCategory);

        if (!mxLocalView->removeRegion(nItemId))
        {
            OUString sMsg( SfxResId(STR_MSG_ERROR_DELETE_FOLDER) );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      sMsg.replaceFirst("$1",sCategory)));
            xBox->run();
        }
        else
        {
            mxCBFolder->remove_text(sCategory);
        }
    }

    mxLocalView->reload();
    mxLocalView->showAllTemplates();
    mxCBApp->set_active(0);
    mxCBFolder->set_active(0);
    mxActionBar->set_item_visible(MNI_ACTION_RENAME_FOLDER, false);
}

void SfxTemplateManagerDlg::createDefaultTemplateMenu ()
{
    std::vector<OUString> aList = lcl_getAllFactoryURLs();

    if (!aList.empty())
    {
        mxTemplateDefaultMenu->clear();

        for (auto const& elem : aList)
        {
            INetURLObject aObj(elem);
            OUString aTitle = SvFileInformationManager::GetDescription(aObj);
            mxTemplateDefaultMenu->append(elem, aTitle, SvFileInformationManager::GetImageId(aObj));
        }

        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT, true);
    }
    else
        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT, false);
}

void SfxTemplateManagerDlg::localMoveTo(sal_uInt16 nItemId)
{
    if (nItemId)
    {
        // Move templates to desired folder if for some reason move fails
        // try copying them.
        mxLocalView->moveTemplates(maSelTemplates,nItemId);
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

            if(!mxLocalView->moveTemplate(pItem,pItem->mnRegionId,nItemId))
            {
                OUString sDst = mxLocalView->getRegionItemName(nItemId);
                OUString sMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE));
                sMsg = sMsg.replaceFirst("$1",sDst);
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok,
                                                          sMsg.replaceFirst( "$2",pItem->maTitle)));
                xBox->run();
            }
        }
    }

    // Deselect all items and update search results
    mxSearchView->deselectItems();

    SearchUpdateHdl(*mxSearchFilter);
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

    for( const auto& rServiceName : aServiceNames )
    {
        if ( ! SfxObjectFactory::GetStandardTemplate( rServiceName ).isEmpty() )
        {
            SvtModuleOptions::EFactory eFac = SvtModuleOptions::EFactory::WRITER;
            SvtModuleOptions::ClassifyFactoryByName( rServiceName, eFac );
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

SfxTemplateSelectionDlg::SfxTemplateSelectionDlg(weld::Window* pParent)
    : SfxTemplateManagerDlg(pParent)
    , msTemplatePath(OUString())
{
    mxCBApp->set_active(MNI_IMPRESS);
    mxCBFolder->set_active(0);
    m_xDialog->set_title(SfxResId(STR_TEMPLATE_SELECTION));

    if (mxLocalView->IsVisible())
    {
        mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mxLocalView->showAllTemplates();
    }

    mxCBApp->set_sensitive(false);
    mxActionBar->hide();
    mxMoveButton->hide();
    mxExportButton->hide();
    mxCBXHideDlg->show();
    mxCBXHideDlg->set_active(true);

    mxLocalView->setOpenTemplateHdl(LINK(this,SfxTemplateSelectionDlg, OpenTemplateHdl));
    mxSearchView->setOpenTemplateHdl(LINK(this,SfxTemplateSelectionDlg, OpenTemplateHdl));

    mxSearchView->SetMultiSelectionEnabled(false);

    mxOKButton->connect_clicked(LINK(this, SfxTemplateSelectionDlg, OkClickHdl));
}

SfxTemplateSelectionDlg::~SfxTemplateSelectionDlg()
{
    maIdle.Stop();
}

short SfxTemplateSelectionDlg::run()
{
    // tdf#124597 at startup this dialog is launched before its parent window
    // has taken its final size. The parent size request is processed during
    // the dialogs event loop so configure this dialog to center to
    // the parents pending geometry request
    m_xDialog->set_centered_on_parent(true);

    // tdf#125079 toggle off the size tracking at some future idle point
    maIdle.SetPriority(TaskPriority::LOWEST);
    maIdle.SetInvokeHandler(LINK(this,SfxTemplateSelectionDlg,TimeOut));
    maIdle.Start();
    setTemplateViewMode(TemplateViewMode::eThumbnailView);

    return weld::GenericDialogController::run();
}

IMPL_LINK_NOARG(SfxTemplateSelectionDlg, TimeOut, Timer*, void)
{
    m_xDialog->set_centered_on_parent(false);
}

IMPL_LINK(SfxTemplateSelectionDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);
    msTemplatePath = pViewItem->getPath();

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SfxTemplateSelectionDlg, OkClickHdl, weld::Button&, void)
{
   TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(const_cast<ThumbnailViewItem*>(*maSelTemplates.begin()));
   msTemplatePath = pViewItem->getPath();

   m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
