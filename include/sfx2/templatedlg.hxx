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

    SAL_DLLPRIVATE void setDocumentModel(const css::uno::Reference<css::frame::XModel>& rModel);
    SAL_DLLPRIVATE void setTemplateViewMode(TemplateViewMode eViewMode);
    SAL_DLLPRIVATE TemplateViewMode getTemplateViewMode() const;

protected:
    SAL_DLLPRIVATE void getApplicationSpecificSettings();

    SAL_DLLPRIVATE void readSettings();

    SAL_DLLPRIVATE void writeSettings();

    SAL_DLLPRIVATE void fillFolderComboBox();

    DECL_DLLPRIVATE_LINK(SelectApplicationHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(SelectRegionHdl, weld::ComboBox&, void);

    DECL_DLLPRIVATE_LINK(OkClickHdl, weld::Button&, void);
    SAL_DLLPRIVATE void ImportActionHdl();
    SAL_DLLPRIVATE static void ExtensionsActionHdl();

    DECL_DLLPRIVATE_LINK(TVItemStateHdl, const ThumbnailViewItem*, void);

    DECL_DLLPRIVATE_LINK(MenuSelectHdl, const OUString&, void);
    SAL_DLLPRIVATE void DefaultTemplateMenuSelectHdl(std::u16string_view rIdent);

    DECL_DLLPRIVATE_LINK(OpenRegionHdl, void*, void);
    DECL_DLLPRIVATE_LINK(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_DLLPRIVATE_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_DLLPRIVATE_LINK(EditTemplateHdl, ThumbnailViewItem*, void);
    DECL_DLLPRIVATE_LINK(DeleteTemplateHdl, void*, void);
    DECL_DLLPRIVATE_LINK(DefaultTemplateHdl, ThumbnailViewItem*, void);
    DECL_DLLPRIVATE_LINK(MoveTemplateHdl, void*, void);
    DECL_DLLPRIVATE_LINK(ExportTemplateHdl, void*, void);

    SAL_DLLPRIVATE void SearchUpdate();

    DECL_DLLPRIVATE_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(LoseFocusHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(ImplUpdateDataHdl, Timer*, void);
    DECL_DLLPRIVATE_LINK(KeyInputHdl, const KeyEvent&, bool);

    DECL_DLLPRIVATE_LINK(ListViewHdl, weld::Toggleable&, void);
    DECL_DLLPRIVATE_LINK(ThumbnailViewHdl, weld::Toggleable&, void);
    DECL_DLLPRIVATE_LINK(FocusRectLocalHdl, weld::Widget&, tools::Rectangle);

    SAL_DLLPRIVATE void OnTemplateImportCategory(std::u16string_view sCategory);
    //    static void OnTemplateLink ();
    SAL_DLLPRIVATE void OnTemplateOpen();
    SAL_DLLPRIVATE void OnTemplateExport();

    SAL_DLLPRIVATE void OnTemplateState(const ThumbnailViewItem* pItem);

    SAL_DLLPRIVATE void OnCategoryNew();
    SAL_DLLPRIVATE void OnCategoryRename();
    SAL_DLLPRIVATE void OnCategoryDelete();

    SAL_DLLPRIVATE void updateMenuItems();

    /**
     *
     * Move templates stored in the filesystem to another folder.
     *
     **/

    SAL_DLLPRIVATE void localMoveTo(sal_uInt16 nMenuId);

    /// Return filter according to the currently selected application filter.
    SAL_DLLPRIVATE FILTER_APPLICATION getCurrentApplicationFilter() const;

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
    std::unique_ptr<weld::Entry> mxNewCategoryEdit;
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
    DECL_DLLPRIVATE_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_DLLPRIVATE_LINK(OkClickHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(TimeOut, Timer*, void);

    OUString msTemplatePath;
    Idle maIdle;
};

#endif // INCLUDED_SFX2_INC_TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
