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

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <sfx2/templatelocalview.hxx>

class Edit;
class PopupMenu;
class TemplateLocalView;
class TemplateSearchView;
class ThumbnailView;
class ThumbnailViewItem;
class ToolBox;

namespace com {
    namespace sun { namespace star { namespace frame {
        class XComponentLoader;
        class XModel;
    }   }   }
}

class SFX2_DLLPUBLIC SfxTemplateManagerDlg : public ModalDialog
{
    typedef bool (*selection_cmp_fn)(const ThumbnailViewItem*,const ThumbnailViewItem*);

public:

    SfxTemplateManagerDlg(vcl::Window *parent = nullptr);

    virtual ~SfxTemplateManagerDlg() override;
    virtual void dispose() override;
    virtual short Execute() override;
    virtual bool EventNotify( NotifyEvent& rNEvt ) override;

    void setDocumentModel (const css::uno::Reference<css::frame::XModel> &rModel);

protected:

    void getApplicationSpecificSettings();

    void readSettings ();

    void writeSettings ();

    void fillFolderComboBox();

    DECL_LINK(TBXDropdownHdl, ToolBox*, void);

    DECL_LINK(SelectApplicationHdl, ListBox&, void);
    DECL_LINK(SelectRegionHdl, ListBox&, void);

    DECL_LINK(OkClickHdl, Button*, void);
    DECL_LINK(MoveClickHdl, Button*, void);
    DECL_LINK(ExportClickHdl, Button*, void);
    DECL_LINK(ImportClickHdl, Button*, void);
    DECL_STATIC_LINK(SfxTemplateManagerDlg, LinkClickHdl, Button*, void);

    DECL_LINK(TVItemStateHdl, const ThumbnailViewItem*, void);

    DECL_LINK(MenuSelectHdl, Menu*, bool);
    DECL_LINK(DefaultTemplateMenuSelectHdl, Menu*, bool);

    DECL_LINK(OpenRegionHdl, void*, void);
    DECL_LINK(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(EditTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(DeleteTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(DefaultTemplateHdl, ThumbnailViewItem*, void);

    DECL_LINK(SearchUpdateHdl, Edit&, void);
    DECL_LINK(GetFocusHdl, Control&, void);

    void OnTemplateImportCategory(const OUString& sCategory);
    static void OnTemplateLink ();
    void OnTemplateOpen ();
    void OnTemplateExport ();

    void OnTemplateState (const ThumbnailViewItem *pItem);

    void OnCategoryNew ();
    void OnCategoryRename();
    void OnCategoryDelete();

    void createDefaultTemplateMenu ();

    /**
     *
     * Move templates stored in the filesystem to another folder.
     *
     **/

    void localMoveTo (sal_uInt16 nMenuId);

    /**
     *
     * Move search result templates stored in the filesystem to another folder.
     *
     **/

    void localSearchMoveTo (sal_uInt16 nMenuId);

    /// Return filter according to the currently selected application filter.
    FILTER_APPLICATION getCurrentApplicationFilter();

protected:

    VclPtr<Edit> mpSearchFilter;
    VclPtr<ListBox> mpCBApp;
    VclPtr<ListBox> mpCBFolder;

    VclPtr<PushButton> mpOKButton;
    VclPtr<PushButton> mpMoveButton;
    VclPtr<PushButton> mpExportButton;
    VclPtr<PushButton> mpImportButton;
    VclPtr<PushButton> mpLinkButton;
    VclPtr<CheckBox> mpCBXHideDlg;
    VclPtr<ToolBox> mpActionBar;
    VclPtr<TemplateSearchView> mpSearchView;
    VclPtr<TemplateLocalView> mpLocalView;
    VclPtr<PopupMenu> mpActionMenu;
    VclPtr<PopupMenu> mpTemplateDefaultMenu;

    std::set<const ThumbnailViewItem*,selection_cmp_fn> maSelTemplates;

    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::frame::XDesktop2 > mxDesktop;
};

//  class SfxTemplateCategoryDialog -------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxTemplateCategoryDialog : public ModalDialog
{
private:
    VclPtr<ListBox>         mpLBCategory;
    VclPtr<FixedText>       mpSelectLabel;
    VclPtr<Edit>            mpNewCategoryEdit;
    VclPtr<FixedText>       mpCreateLabel;
    VclPtr<PushButton>      mpOKButton;

    OUString   msSelectedCategory;
    bool       mbIsNewCategory;

public:
    DECL_LINK(NewCategoryEditHdl, Edit&, void);
    DECL_LINK(SelectCategoryHdl, ListBox&, void);

    void SetCategoryLBEntries(std::vector<OUString> names);

    void HideNewCategoryOption();

    const OUString& GetSelectedCategory() const {
        return msSelectedCategory;
    };

    void SetSelectLabelText(OUString const & sText) const {
        mpSelectLabel->SetText(sText);
    };

    bool IsNewCategoryCreated() const {
        return mbIsNewCategory;
    }

public:

    explicit SfxTemplateCategoryDialog();

    virtual ~SfxTemplateCategoryDialog() override;
    virtual void dispose() override;
};


//  class SfxTemplateSelectionDialog -------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxTemplateSelectionDlg : public SfxTemplateManagerDlg
{
public:
    SfxTemplateSelectionDlg(vcl::Window *parent);

    virtual ~SfxTemplateSelectionDlg() override;
    virtual void dispose() override;
    virtual short Execute() override;

    OUString const & getTemplatePath() const { return msTemplatePath; };
    bool IsStartWithTemplate() const { return mpCBXHideDlg->IsChecked(); };

private:
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(OkClickHdl, Button*, void);

    OUString   msTemplatePath;
};


#endif // INCLUDED_SFX2_INC_TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
