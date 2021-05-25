/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_INC_TEMPLATEDLG_HXX
#define INCLUDED_SFX2_INC_TEMPLATEDLG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <set>
#include <string_view>

#include <vcl/idle.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

#include <sfx2/templatedlglocalview.hxx>

class ThumbnailViewItem;

namespace com
{
namespace sun::star::frame
{
class XDesktop2;
class XModel;
}
}

class SFX2_DLLPUBLIC SfxTemplateManagerDlg : public weld::GenericDialogController
{
    typedef bool (*selection_cmp_fn)(const ThumbnailViewItem*, const ThumbnailViewItem*);

public:
    SfxTemplateManagerDlg(weld::Window* parent);

    virtual ~SfxTemplateManagerDlg() override;
    virtual short run() override;

    void setDocumentModel(const css::uno::Reference<css::frame::XModel>& rModel);
    void setTemplateViewMode(TemplateViewMode eViewMode);
    TemplateViewMode getTemplateViewMode() const;

protected:
    void getApplicationSpecificSettings();

    void readSettings();

    void writeSettings();

    void fillFolderComboBox();

    DECL_LINK(SelectApplicationHdl, weld::ComboBox&, void);
    DECL_LINK(SelectRegionHdl, weld::ComboBox&, void);

    DECL_LINK(OkClickHdl, weld::Button&, void);
    void ImportActionHdl();
    static void ExtensionsActionHdl();

    DECL_LINK(TVItemStateHdl, const ThumbnailViewItem*, void);

    DECL_LINK(MenuSelectHdl, const OString&, void);
    void DefaultTemplateMenuSelectHdl(std::string_view rIdent);

    DECL_LINK(OpenRegionHdl, void*, void);
    DECL_LINK(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(EditTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(DeleteTemplateHdl, void*, void);
    DECL_LINK(DefaultTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(MoveTemplateHdl, void*, void);
    DECL_LINK(ExportTemplateHdl, void*, void);

    void SearchUpdate();

    DECL_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_LINK(LoseFocusHdl, weld::Widget&, void);
    DECL_LINK(ImplUpdateDataHdl, Timer*, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    DECL_LINK(ListViewHdl, weld::Toggleable&, void);
    DECL_LINK(ThumbnailViewHdl, weld::Toggleable&, void);
    DECL_LINK(FocusRectLocalHdl, weld::Widget&, tools::Rectangle);

    void OnTemplateImportCategory(std::u16string_view sCategory);
    //    static void OnTemplateLink ();
    void OnTemplateOpen();
    void OnTemplateExport();

    void OnTemplateState(const ThumbnailViewItem* pItem);

    void OnCategoryNew();
    void OnCategoryRename();
    void OnCategoryDelete();

    void updateMenuItems();

    /**
     *
     * Move templates stored in the filesystem to another folder.
     *
     **/

    void localMoveTo(sal_uInt16 nMenuId);

    /// Return filter according to the currently selected application filter.
    FILTER_APPLICATION getCurrentApplicationFilter() const;

protected:
    std::set<const ThumbnailViewItem*, selection_cmp_fn> maSelTemplates;
    css::uno::Reference<css::frame::XModel> m_xModel;
    css::uno::Reference<css::frame::XDesktop2> mxDesktop;

    Timer m_aUpdateDataTimer;

    std::unique_ptr<weld::Entry> mxSearchFilter;
    std::unique_ptr<weld::ComboBox> mxCBApp;
    std::unique_ptr<weld::ComboBox> mxCBFolder;

    std::unique_ptr<weld::Button> mxOKButton;
    std::unique_ptr<weld::CheckButton> mxCBXHideDlg;
    std::unique_ptr<weld::MenuButton> mxActionBar;
    std::unique_ptr<TemplateDlgLocalView> mxLocalView;
    std::unique_ptr<weld::Menu> mxTemplateDefaultMenu;
    std::unique_ptr<weld::CustomWeld> mxLocalViewWeld;
    std::unique_ptr<weld::Toggleable> mxListViewButton;
    std::unique_ptr<weld::Toggleable> mxThumbnailViewButton;
    TemplateViewMode mViewMode;
    bool bMakeSelItemVisible;
};

//  class SfxTemplateCategoryDialog -------------------------------------------------------------------

class SfxTemplateCategoryDialog final : public weld::GenericDialogController
{
private:
    OUString msSelectedCategory;
    bool mbIsNewCategory;

    std::unique_ptr<weld::TreeView> mxLBCategory;
    std::unique_ptr<weld::Label> mxSelectLabel;
    std::unique_ptr<weld::Entry> mxNewCategoryEdit;
    std::unique_ptr<weld::Label> mxCreateLabel;
    std::unique_ptr<weld::Button> mxOKButton;

public:
    DECL_LINK(NewCategoryEditHdl, weld::Entry&, void);
    DECL_LINK(SelectCategoryHdl, weld::TreeView&, void);

    void SetCategoryLBEntries(std::vector<OUString> names);

    const OUString& GetSelectedCategory() const { return msSelectedCategory; };

    bool IsNewCategoryCreated() const { return mbIsNewCategory; }

public:
    explicit SfxTemplateCategoryDialog(weld::Window* pParent);

    virtual ~SfxTemplateCategoryDialog() override;
};

//  class SfxTemplateSelectionDialog -------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxTemplateSelectionDlg final : public SfxTemplateManagerDlg
{
public:
    SfxTemplateSelectionDlg(weld::Window* parent);

    virtual ~SfxTemplateSelectionDlg() override;
    virtual short run() override;

    OUString const& getTemplatePath() const { return msTemplatePath; };
    bool IsStartWithTemplate() const { return mxCBXHideDlg->get_active(); };

private:
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(OkClickHdl, weld::Button&, void);
    DECL_LINK(TimeOut, Timer*, void);

    OUString msTemplatePath;
    Idle maIdle;
};

#endif // INCLUDED_SFX2_INC_TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
