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

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
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
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/event.hxx>
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
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <comphelper/dispatchcommand.hxx>

#include <sfx2/strings.hrc>
#include <bitmaps.hlst>

constexpr OUString TM_SETTING_MANAGER = u"TemplateManager"_ustr;
constexpr OUString TM_SETTING_LASTFOLDER = u"LastFolder"_ustr;
constexpr OUString TM_SETTING_LASTAPPLICATION = u"LastApplication"_ustr;
constexpr OUString TM_SETTING_VIEWMODE = u"ViewMode"_ustr;

constexpr OUString MNI_ACTION_NEW_FOLDER = u"new"_ustr;
constexpr OUString MNI_ACTION_RENAME_FOLDER = u"rename"_ustr;
constexpr OUString MNI_ACTION_DELETE_FOLDER = u"delete"_ustr;
constexpr OUString MNI_ACTION_DEFAULT   = u"default"_ustr;
constexpr OUString MNI_ACTION_DEFAULT_WRITER   = u"default_writer"_ustr;
constexpr OUString MNI_ACTION_DEFAULT_CALC   = u"default_calc"_ustr;
constexpr OUString MNI_ACTION_DEFAULT_IMPRESS   = u"default_impress"_ustr;
constexpr OUString MNI_ACTION_DEFAULT_DRAW   = u"default_draw"_ustr;
constexpr OUString MNI_ACTION_IMPORT   = u"import_template"_ustr;
constexpr OUString MNI_ACTION_EXTENSIONS = u"extensions"_ustr;
#define MNI_ALL_APPLICATIONS 0
#define MNI_WRITER           1
#define MNI_CALC             2
#define MNI_IMPRESS          3
#define MNI_DRAW             4

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::frame;
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
    : GenericDialogController(pParent, u"sfx/ui/templatedlg.ui"_ustr, u"TemplateDialog"_ustr)
    , maSelTemplates(cmpSelectionItems)
    , mxDesktop(Desktop::create(comphelper::getProcessComponentContext()))
    , m_aUpdateDataTimer( "SfxTemplateManagerDlg UpdateDataTimer" )
    , mxSearchFilter(m_xBuilder->weld_entry(u"search_filter"_ustr))
    , mxCBApp(m_xBuilder->weld_combo_box(u"filter_application"_ustr))
    , mxCBFolder(m_xBuilder->weld_combo_box(u"filter_folder"_ustr))
    , mxOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , mxCBXHideDlg(m_xBuilder->weld_check_button(u"hidedialogcb"_ustr))
    , mxActionBar(m_xBuilder->weld_menu_button(u"action_menu"_ustr))
    , mxLocalView(new TemplateDlgLocalView(m_xBuilder->weld_scrolled_window(u"scrolllocal"_ustr, true),
                                           m_xBuilder->weld_menu(u"contextmenu"_ustr),
                                           m_xBuilder->weld_tree_view(u"tree_list"_ustr)))
    , mxLocalViewWeld(new weld::CustomWeld(*m_xBuilder, u"template_view"_ustr, *mxLocalView))
    , mxListViewButton(m_xBuilder->weld_toggle_button(u"list_view_btn"_ustr))
    , mxThumbnailViewButton(m_xBuilder->weld_toggle_button(u"thumbnail_view_btn"_ustr))
    , mViewMode(TemplateViewMode::eThumbnailView)
{
    // Create popup menus
    mxActionBar->append_item(MNI_ACTION_NEW_FOLDER, SfxResId(STR_CATEGORY_NEW), BMP_ACTION_NEW_CATEGORY);
    mxActionBar->append_item(MNI_ACTION_RENAME_FOLDER, SfxResId(STR_CATEGORY_RENAME), BMP_ACTION_RENAME);
    mxActionBar->append_item(MNI_ACTION_DELETE_FOLDER, SfxResId(STR_CATEGORY_DELETE), BMP_ACTION_DELETE_CATEGORY);
    mxActionBar->append_separator(u"separator"_ustr);
    mxActionBar->append_item(MNI_ACTION_DEFAULT, SfxResId(STR_ACTION_RESET_ALL_DEFAULT_TEMPLATES));
    mxActionBar->append_item(MNI_ACTION_DEFAULT_WRITER, SfxResId(STR_ACTION_RESET_WRITER_TEMPLATE), BMP_ACTION_DEFAULT_WRITER);
    mxActionBar->append_item(MNI_ACTION_DEFAULT_CALC, SfxResId(STR_ACTION_RESET_CALC_TEMPLATE), BMP_ACTION_DEFAULT_CALC);
    mxActionBar->append_item(MNI_ACTION_DEFAULT_IMPRESS, SfxResId(STR_ACTION_RESET_IMPRESS_TEMPLATE), BMP_ACTION_DEFAULT_IMPRESS);
    mxActionBar->append_item(MNI_ACTION_DEFAULT_DRAW, SfxResId(STR_ACTION_RESET_DRAW_TEMPLATE), BMP_ACTION_DEFAULT_DRAW);
    mxActionBar->append_separator(u"separator2"_ustr);
    mxActionBar->append_item(MNI_ACTION_IMPORT, SfxResId(STR_ACTION_IMPORT), BMP_ACTION_IMPORT);
    mxActionBar->append_item(MNI_ACTION_EXTENSIONS, SfxResId(STR_ACTION_EXTENSIONS), BMP_ACTION_EXTENSIONS);

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
    mxLocalView->setMoveTemplateHdl(LINK(this,SfxTemplateManagerDlg, MoveTemplateHdl));
    mxLocalView->setExportTemplateHdl(LINK(this,SfxTemplateManagerDlg, ExportTemplateHdl));

    mxLocalView->ShowTooltips(true);

    // Set width and height of the templates thumbnail viewer to accommodate 3 rows and 4 columns of items
    mxLocalViewWeld->set_size_request(TEMPLATE_ITEM_MAX_WIDTH * 5, TEMPLATE_ITEM_MAX_HEIGHT_SUB * 3);

    mxOKButton->connect_clicked(LINK(this, SfxTemplateManagerDlg, OkClickHdl));
    // FIXME: rather than disabling make dispatchCommand(".uno:AdditionsDialog") work in start center
    if ( !SfxModule::GetActiveModule() )
        mxActionBar->set_item_sensitive(MNI_ACTION_EXTENSIONS, false);
    else
        mxActionBar->set_item_sensitive(MNI_ACTION_EXTENSIONS, true);
    mxListViewButton->connect_toggled(LINK(this, SfxTemplateManagerDlg, ListViewHdl));
    mxThumbnailViewButton->connect_toggled(LINK(this, SfxTemplateManagerDlg, ThumbnailViewHdl));

    mxSearchFilter->connect_changed(LINK(this, SfxTemplateManagerDlg, SearchUpdateHdl));
    mxSearchFilter->connect_focus_in(LINK( this, SfxTemplateManagerDlg, GetFocusHdl ));
    mxSearchFilter->connect_focus_out(LINK( this, SfxTemplateManagerDlg, LoseFocusHdl ));
    mxSearchFilter->connect_key_press(LINK( this, SfxTemplateManagerDlg, KeyInputHdl));

    mxActionBar->show();

    mxLocalView->Populate();
    mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));

    mxCBApp->set_active(0);
    fillFolderComboBox();

    mxActionBar->set_item_visible(MNI_ACTION_EXTENSIONS, true);
    mxActionBar->set_item_visible(MNI_ACTION_IMPORT, true);
    mxActionBar->set_item_visible(MNI_ACTION_NEW_FOLDER, true);

    mxOKButton->set_label(SfxResId(STR_OPEN));

    mxCBApp->connect_changed(LINK(this, SfxTemplateManagerDlg, SelectApplicationHdl));
    mxCBFolder->connect_changed(LINK(this, SfxTemplateManagerDlg, SelectRegionHdl));

    mxLocalView->Show();

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SfxTemplateManagerDlg, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    mxLocalView->connect_focus_rect(LINK(this, SfxTemplateManagerDlg, FocusRectLocalHdl));
    bMakeSelItemVisible = false;
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg()
{
    writeSettings();

    // Ignore view events since we are cleaning the object
    mxLocalView->setItemStateHdl(Link<const ThumbnailViewItem*,void>());
    mxLocalView->setOpenRegionHdl(Link<void*,void>());
    mxLocalView->setOpenTemplateHdl(Link<ThumbnailViewItem*, void>());
}

short SfxTemplateManagerDlg::run()
{
    //use application specific settings if there's no previous setting
    getApplicationSpecificSettings();
    readSettings();
    updateMenuItems();

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
            mxSearchFilter->set_text(u""_ustr);
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
    if(eViewMode == TemplateViewMode::eThumbnailView && mViewMode != TemplateViewMode::eThumbnailView)
    {
        mxThumbnailViewButton->set_state(TRISTATE_TRUE);
        mxListViewButton->set_state(TRISTATE_FALSE);
        mxLocalView->ThumbnailView::GrabFocus();
        mViewMode = eViewMode;
        mxLocalView->setTemplateViewMode(eViewMode);
        mxLocalView->Show();
    }
    if(eViewMode == TemplateViewMode::eListView && mViewMode != TemplateViewMode::eListView)
    {
        mxListViewButton->set_state(TRISTATE_TRUE);
        mxThumbnailViewButton->set_state(TRISTATE_FALSE);
        mxLocalView->ListView::grab_focus();
        mViewMode = eViewMode;
        mxLocalView->setTemplateViewMode(eViewMode);
        mxLocalView->Show();
    }
}

TemplateViewMode SfxTemplateManagerDlg::getTemplateViewMode() const
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
    mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, false);
}

void SfxTemplateManagerDlg::getApplicationSpecificSettings()
{
    if ( ! m_xModel.is() )
    {
        mxCBApp->set_active(0);
        mxCBFolder->set_active(0);
        mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, false);
        mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, false);
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
    mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, false);
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
        mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, false);
        mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, false);
        mxLocalView->showAllTemplates();
    }
    else
    {
        mxCBFolder->set_active_text(aLastFolder);
        mxLocalView->showRegion(aLastFolder);
        bool bIsBuiltInRegion = mxLocalView->IsBuiltInRegion(aLastFolder);
        mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, !bIsBuiltInRegion);
        mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, !bIsBuiltInRegion);
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
        { TM_SETTING_LASTFOLDER, css::uno::Any(aLastFolder) },
        { TM_SETTING_LASTAPPLICATION,     css::uno::Any(sal_uInt16(mxCBApp->get_active())) },
        { TM_SETTING_VIEWMODE, css::uno::Any(static_cast<sal_Int16>(getTemplateViewMode()))}
    };

    // write
    SvtViewOptions aViewSettings(EViewType::Dialog, TM_SETTING_MANAGER);
    aViewSettings.SetUserData(aSettings);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SelectApplicationHdl, weld::ComboBox&, void)
{
    mxLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
    SelectRegionHdl(*mxCBFolder);
    updateMenuItems();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, SelectRegionHdl, weld::ComboBox&, void)
{
    const OUString sSelectedRegion = mxCBFolder->get_active_text();

    if(mxCBFolder->get_active() == 0)
    {
        mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, false);
        mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, false);
    }
    else
    {
        bool bIsBuiltInRegion = mxLocalView->IsBuiltInRegion(sSelectedRegion);
        mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, !bIsBuiltInRegion);
        mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, !bIsBuiltInRegion);
    }
    SearchUpdate();
}

IMPL_LINK(SfxTemplateManagerDlg, TVItemStateHdl, const ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

    if (pViewItem)
        OnTemplateState(pItem);
}

IMPL_LINK(SfxTemplateManagerDlg, MenuSelectHdl, const OUString&, rIdent, void)
{
    if (rIdent == MNI_ACTION_NEW_FOLDER)
        OnCategoryNew();
    else if (rIdent == MNI_ACTION_RENAME_FOLDER)
        OnCategoryRename();
    else if (rIdent == MNI_ACTION_DELETE_FOLDER)
        OnCategoryDelete();
    else if (rIdent == MNI_ACTION_DEFAULT)
    {
        DefaultTemplateMenuSelectHdl(MNI_ACTION_DEFAULT_WRITER);
        DefaultTemplateMenuSelectHdl(MNI_ACTION_DEFAULT_CALC);
        DefaultTemplateMenuSelectHdl(MNI_ACTION_DEFAULT_IMPRESS);
        DefaultTemplateMenuSelectHdl(MNI_ACTION_DEFAULT_DRAW);
    }
    else if(rIdent == MNI_ACTION_DEFAULT_WRITER || rIdent == MNI_ACTION_DEFAULT_CALC ||
            rIdent == MNI_ACTION_DEFAULT_IMPRESS || rIdent == MNI_ACTION_DEFAULT_DRAW )
        DefaultTemplateMenuSelectHdl(rIdent);
    else if(rIdent == MNI_ACTION_IMPORT)
        ImportActionHdl();
    else if(rIdent == MNI_ACTION_EXTENSIONS)
        ExtensionsActionHdl();
}

void SfxTemplateManagerDlg::DefaultTemplateMenuSelectHdl(std::u16string_view rIdent)
{
    SvtModuleOptions aModOpt;
    OUString aFactoryURL;
    if (rIdent == MNI_ACTION_DEFAULT_WRITER)
        aFactoryURL = aModOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::WRITER);
    else if (rIdent == MNI_ACTION_DEFAULT_CALC)
        aFactoryURL = aModOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::CALC);
    else if (rIdent == MNI_ACTION_DEFAULT_IMPRESS)
        aFactoryURL = aModOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::IMPRESS);
    else if (rIdent == MNI_ACTION_DEFAULT_DRAW)
        aFactoryURL = aModOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::DRAW);
    else
        return;

    OUString aServiceName = SfxObjectShell::GetServiceNameFromFactory(aFactoryURL);
    OUString sPrevDefault = SfxObjectFactory::GetStandardTemplate( aServiceName );
    if(!sPrevDefault.isEmpty())
    {
        mxLocalView->RemoveDefaultTemplateIcon(sPrevDefault);
    }

    SfxObjectFactory::SetStandardTemplate( aServiceName, OUString() );
    mxLocalView->refreshDefaultColumn();
    updateMenuItems();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, OkClickHdl, weld::Button&, void)
{
   OnTemplateOpen();
   m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, MoveTemplateHdl, void*, void)
{
    // modal dialog to select templates category
    SfxTemplateCategoryDialog aDlg(m_xDialog.get());
    aDlg.SetCategoryLBEntries(mxLocalView->getFolderNames());

    size_t nItemId = 0;

    if (aDlg.run() != RET_OK)
        return;

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

    if(nItemId)
    {
        localMoveTo(nItemId);
    }

    mxLocalView->reload();
    SearchUpdate();
}
IMPL_LINK_NOARG(SfxTemplateManagerDlg, ExportTemplateHdl, void*, void)
{
    OnTemplateExport();
}

void SfxTemplateManagerDlg::ImportActionHdl()
{
    if(mxCBFolder->get_active() == 0)
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
    }
    else
    {
        const auto sCategory = mxCBFolder->get_active_text();
        OnTemplateImportCategory(sCategory);
    }
    mxLocalView->reload();
    SearchUpdate();
}

void SfxTemplateManagerDlg::ExtensionsActionHdl()
{
    uno::Sequence<beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"AdditionsTag"_ustr, u"Templates"_ustr) };
    comphelper::dispatchCommand(u".uno:AdditionsDialog"_ustr, aArgs);
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
    bool bIsDefault = false;
    bool bIsInternal = false;
    std::vector<const TemplateViewItem*> aSelTemplates;
    for(const auto& aSelTmpl : maSelTemplates)
    {
        const TemplateViewItem *aItem = dynamic_cast<const TemplateViewItem*>(aSelTmpl);
        aSelTemplates.push_back(aItem);
    }

    for(const auto& aSelTmpl : aSelTemplates)
    {
        if(aSelTmpl->IsDefaultTemplate())
            bIsDefault = true;
        if(TemplateLocalView::IsInternalTemplate(aSelTmpl->getPath()))
        {
            bIsInternal = true;
            if(bIsDefault)
                break;
        }
    }

    if (!pViewItem)
        return;

    bool bIsSingleSel = maSelTemplates.size() == 1;
    OUString aDefaultImg;
    INetURLObject aUrl(pViewItem->getPath());
    if (ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION::WRITER, aUrl.getExtension()))
        aDefaultImg = BMP_ACTION_DEFAULT_WRITER;
    else if (ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION::CALC, aUrl.getExtension()))
        aDefaultImg = BMP_ACTION_DEFAULT_CALC;
    else if (ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION::IMPRESS, aUrl.getExtension()))
        aDefaultImg = BMP_ACTION_DEFAULT_IMPRESS;
    else if (ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION::DRAW, aUrl.getExtension()))
        aDefaultImg = BMP_ACTION_DEFAULT_DRAW;
    mxLocalView->createContextMenu(bIsDefault, bIsInternal, bIsSingleSel, aDefaultImg);
}

IMPL_LINK(SfxTemplateManagerDlg, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs{
        comphelper::makePropertyValue(u"AsTemplate"_ustr, true),
        comphelper::makePropertyValue(u"MacroExecutionMode"_ustr, MacroExecMode::USE_CONFIG),
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr, UpdateDocMode::ACCORDING_TO_CONFIG),
        comphelper::makePropertyValue(u"InteractionHandler"_ustr, task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr )),
        comphelper::makePropertyValue(u"ReadOnly"_ustr, true)
    };

    TemplateViewItem *pTemplateItem = static_cast<TemplateViewItem*>(pItem);

    try
    {
        mxDesktop->loadComponentFromURL(pTemplateItem->getPath(),u"_default"_ustr, 0, aArgs );
    }
    catch( const uno::Exception& )
    {
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SfxTemplateManagerDlg, EditTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs{
        comphelper::makePropertyValue(u"AsTemplate"_ustr, false),
        comphelper::makePropertyValue(u"MacroExecutionMode"_ustr, MacroExecMode::USE_CONFIG),
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr, UpdateDocMode::ACCORDING_TO_CONFIG)
    };

    uno::Reference< XStorable > xStorable;
    TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);

    try
    {
        xStorable.set( mxDesktop->loadComponentFromURL(pViewItem->getPath(),u"_default"_ustr, 0, aArgs ),
                       uno::UNO_QUERY );
    }
    catch( const uno::Exception& )
    {
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, DeleteTemplateHdl, void*, void)
{
    std::set<const ThumbnailViewItem*,selection_cmp_fn> aSelTemplates = maSelTemplates;
    OUString aDeletedTemplate;

    for (auto const& pItem : aSelTemplates)
    {
        const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(pItem);
        sal_uInt16 nRegionItemId = mxLocalView->getRegionId(pViewItem->mnRegionId);

        if (!mxLocalView->removeTemplate(pViewItem->mnDocId + 1, nRegionItemId))//mnId w.r.t. region is mnDocId + 1;
        {
            aDeletedTemplate += pItem->maTitle+"\n";
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

    updateMenuItems();
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

IMPL_LINK_NOARG ( SfxTemplateManagerDlg, ListViewHdl, weld::Toggleable&, void )
{
    setTemplateViewMode(TemplateViewMode::eListView);
}

IMPL_LINK_NOARG ( SfxTemplateManagerDlg, ThumbnailViewHdl, weld::Toggleable&, void )
{
    setTemplateViewMode(TemplateViewMode::eThumbnailView);
    bMakeSelItemVisible = true;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, FocusRectLocalHdl, weld::Widget&, tools::Rectangle)
{
    if(bMakeSelItemVisible && !maSelTemplates.empty())
        mxLocalView->MakeItemVisible((*maSelTemplates.begin())->mnId);
    bMakeSelItemVisible = false;
    return tools::Rectangle();
}

void SfxTemplateManagerDlg::SearchUpdate()
{
    const OUString sSelectedRegion = mxCBFolder->get_active_text();
    mxLocalView->setCurRegionId(mxLocalView->getRegionId(sSelectedRegion));
    OUString aKeyword = mxSearchFilter->get_text();
    mxLocalView->Clear();
    std::function<bool(const TemplateItemProperties &)> aFunc =
        [&](const TemplateItemProperties &rItem)->bool
        {
            return aKeyword.isEmpty() || SearchView_Keyword(aKeyword, getCurrentApplicationFilter())(rItem);
        };

    std::vector<TemplateItemProperties> aItems = mxLocalView->getFilteredItems(aFunc);
    mxLocalView->insertItems(aItems, mxCBFolder->get_active()!=0, true);
    mxLocalView->Invalidate();
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg, GetFocusHdl, weld::Widget&, void)
{
    mxLocalView->deselectItems();
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

}

void SfxTemplateManagerDlg::OnTemplateImportCategory(std::u16string_view sCategory)
{
    sfx2::FileDialogHelper aFileDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                    FileDialogFlags::MultiSelection, m_xDialog.get());
    aFileDlg.SetContext(sfx2::FileDialogHelper::TemplateImport);

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
    uno::Reference<XFolderPicker2> xFolderPicker = sfx2::createFolderPicker(xContext, m_xDialog.get());

    xFolderPicker->setDisplayDirectory(SvtPathOptions().GetWorkPath());

    sal_Int16 nResult = xFolderPicker->execute();
    sal_Int16 nCount = maSelTemplates.size();

    if( nResult != ExecutableDialogResults::OK )
        return;

    OUString aTemplateList;
    INetURLObject aPathObj(xFolderPicker->getDirectory());
    aPathObj.setFinalSlash();

    // export templates from the current view

    sal_uInt16 i = 1;
    auto aSelTemplates = maSelTemplates;
    for (auto const& selTemplate : aSelTemplates)
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
    dlg.set_title(SfxResId(STR_WINDOW_TITLE_RENAME_NEW_CATEGORY));
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
    dlg.set_title(SfxResId(STR_WINDOW_TITLE_RENAME_CATEGORY));
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
        SearchUpdate();
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
    const auto sCategory = mxCBFolder->get_active_text();
    std::unique_ptr<weld::MessageDialog> popupDlg(Application::CreateMessageDialog(m_xDialog.get(),
                                                VclMessageType::Question, VclButtonsType::YesNo,
                                                SfxResId(STR_QMSG_SEL_FOLDER_DELETE).replaceFirst("$1",sCategory)));
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

    mxLocalView->reload();
    mxLocalView->showAllTemplates();
    mxCBApp->set_active(0);
    mxCBFolder->set_active(0);
    SearchUpdate();
    mxActionBar->set_item_sensitive(MNI_ACTION_RENAME_FOLDER, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DELETE_FOLDER, false);
    updateMenuItems();
}

void SfxTemplateManagerDlg::updateMenuItems ()
{

    mxActionBar->set_item_visible(MNI_ACTION_DEFAULT, false);
    mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_WRITER, false);
    mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_CALC, false);
    mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_IMPRESS, false);
    mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_DRAW, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_WRITER, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_CALC, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_IMPRESS, false);
    mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_DRAW, false);

    SvtModuleOptions aModOpt;
    if( mxCBApp->get_active() == MNI_WRITER)
    {
        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_WRITER, true);
        if(!aModOpt.GetFactoryStandardTemplate( SvtModuleOptions::EFactory::WRITER).isEmpty())
            mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_WRITER, true);
    }
    else if( mxCBApp->get_active() == MNI_CALC )
    {
        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_CALC, true);
        if(!aModOpt.GetFactoryStandardTemplate( SvtModuleOptions::EFactory::CALC).isEmpty())
            mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_CALC, true);
    }
    else if(mxCBApp->get_active() == MNI_IMPRESS)
    {
        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_IMPRESS, true);
        if(!aModOpt.GetFactoryStandardTemplate( SvtModuleOptions::EFactory::IMPRESS).isEmpty())
            mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_IMPRESS, true);
    }
    else if(mxCBApp->get_active() == MNI_DRAW)
    {
        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT_DRAW, true);
        if(!aModOpt.GetFactoryStandardTemplate( SvtModuleOptions::EFactory::DRAW).isEmpty())
            mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT_DRAW, true);
    }
    else if(mxCBApp->get_active() == MNI_ALL_APPLICATIONS)
    {
        mxActionBar->set_item_visible(MNI_ACTION_DEFAULT, true);
        if(!lcl_getAllFactoryURLs().empty())
            mxActionBar->set_item_sensitive(MNI_ACTION_DEFAULT, true);
    }
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
    : GenericDialogController(pParent, u"sfx/ui/templatecategorydlg.ui"_ustr, u"TemplatesCategoryDialog"_ustr)
    , mbIsNewCategory(false)
    , mxLBCategory(m_xBuilder->weld_tree_view(u"categorylb"_ustr))
    , mxNewCategoryEdit(m_xBuilder->weld_entry(u"category_entry"_ustr))
    , mxOKButton(m_xBuilder->weld_button(u"ok"_ustr))
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

// SfxTemplateSelectionDialog -----------------------------------------------------------------

SfxTemplateSelectionDlg::SfxTemplateSelectionDlg(weld::Window* pParent)
    : SfxTemplateManagerDlg(pParent)
    , maIdle("sfx2 SfxTemplateManagerDlg maIdle")
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
    mxActionBar->show();
    mxCBXHideDlg->show();
    mxCBXHideDlg->set_active(true);

    mxLocalView->setOpenTemplateHdl(LINK(this,SfxTemplateSelectionDlg, OpenTemplateHdl));
    mxOKButton->connect_clicked(LINK(this, SfxTemplateSelectionDlg, OkClickHdl));
    updateMenuItems();
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
