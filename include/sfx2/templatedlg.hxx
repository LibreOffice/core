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
#include <com/sun/star/frame/XDesktop2.hpp>

#include <sfx2/templateabstractview.hxx>

class Edit;
class PopupMenu;
class TemplateAbstractView;
class TemplateLocalView;
class TemplateRemoteView;
class TemplateRepository;
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

    virtual ~SfxTemplateManagerDlg();
    virtual void dispose() override;
    virtual short Execute() override;

    void setDocumentModel (const css::uno::Reference<css::frame::XModel> &rModel);

private:

    void getApplicationSpecificSettings();

    void readSettings ();

    void writeSettings ();

    void fillFolderComboBox();

    DECL_LINK_TYPED(TBXDropdownHdl, ToolBox*, void);

    DECL_LINK_TYPED(SelectApplicationHdl, ListBox&, void);
    DECL_LINK_TYPED(SelectRegionHdl, ListBox&, void);

    DECL_LINK_TYPED(OkClickHdl, Button*, void);
    DECL_LINK_TYPED(MoveClickHdl, Button*, void);
    DECL_LINK_TYPED(ExportClickHdl, Button*, void);
    DECL_LINK_TYPED(ImportClickHdl, Button*, void);
    DECL_STATIC_LINK_TYPED(SfxTemplateManagerDlg, LinkClickHdl, Button*, void);

    DECL_LINK_TYPED(TVItemStateHdl, const ThumbnailViewItem*, void);

    DECL_LINK_TYPED(MenuSelectHdl, Menu*, bool);
    DECL_LINK_TYPED(RepositoryMenuSelectHdl, Menu*, bool);
    DECL_LINK_TYPED(DefaultTemplateMenuSelectHdl, Menu*, bool);

    DECL_LINK_TYPED(OpenRegionHdl, void*, void);
    DECL_LINK_TYPED(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(EditTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(DeleteTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(DefaultTemplateHdl, ThumbnailViewItem*, void);

    DECL_LINK_TYPED(SearchUpdateHdl, Edit&, void);
    DECL_LINK_TYPED(GetFocusHdl, Control&, void);

    void OnTemplateImportCategory(const OUString& sCategory);
    static void OnTemplateLink ();
    void OnTemplateOpen ();
    void OnTemplateExport ();

    void OnTemplateState (const ThumbnailViewItem *pItem);

    void OnCategoryNew ();
    void OnCategoryRename();
    void OnCategoryDelete();

    void OnRegionState (const ThumbnailViewItem *pItem);

    void OnRepositoryDelete ();

    void createRepositoryMenu ();

    void createDefaultTemplateMenu ();

    // Exchange view between local/online view.
    void switchMainView (bool bDisplayLocal);

    /**
     *
     * Move templates stored in the filesystem to another folder.
     *
     **/

    void localMoveTo (sal_uInt16 nMenuId);

    void remoteMoveTo (const sal_uInt16 nMenuId);

    /**
     *
     * Move search result templates stored in the filesystem to another folder.
     *
     **/

    void localSearchMoveTo (sal_uInt16 nMenuId);

    // Remote repositories handling methods
    void loadRepositories ();

    const std::vector<TemplateRepository*>& getRepositories () const { return maRepositories; }

    bool insertRepository (const OUString &rName, const OUString &rURL);

    bool deleteRepository (const sal_uInt16 nRepositoryId);

    void syncRepositories () const;

    /// Return filter according to the currently selected application filter.
    FILTER_APPLICATION getCurrentApplicationFilter();

private:

    VclPtr<Edit> mpSearchFilter;
    VclPtr<ListBox> mpCBApp;
    VclPtr<ListBox> mpCBFolder;

    VclPtr<PushButton> mpOKButton;
    VclPtr<PushButton> mpMoveButton;
    VclPtr<PushButton> mpExportButton;
    VclPtr<PushButton> mpImportButton;
    VclPtr<PushButton> mpLinkButton;
    VclPtr<ToolBox> mpActionBar;
    VclPtr<TemplateSearchView> mpSearchView;
    VclPtr<TemplateAbstractView> mpCurView;
    VclPtr<TemplateLocalView> mpLocalView;
    VclPtr<TemplateRemoteView> mpRemoteView;
    PopupMenu *mpActionMenu;
    PopupMenu *mpRepositoryMenu;
    PopupMenu *mpTemplateDefaultMenu;

    std::set<const ThumbnailViewItem*,selection_cmp_fn> maSelTemplates;

    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::frame::XDesktop2 > mxDesktop;

    bool mbIsSynced; ///< Tells whether maRepositories is synchronized with the user config
    std::vector<TemplateRepository*> maRepositories; ///< Stores the remote repositories for templates
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
    DECL_LINK_TYPED(NewCategoryEditHdl, Edit&, void);
    DECL_LINK_TYPED(SelectCategoryHdl, ListBox&, void);

    void SetCategoryLBEntries(std::vector<OUString> names);

    void HideNewCategoryOption();

    inline const OUString& GetSelectedCategory() const {
        return msSelectedCategory;
    };

    inline void SetSelectLabelText(OUString sText) const {
        mpSelectLabel->SetText(sText);
    };

    inline bool IsNewCategoryCreated() const {
        return mbIsNewCategory;
    }

public:

    explicit SfxTemplateCategoryDialog(vcl::Window *parent = nullptr);

    virtual ~SfxTemplateCategoryDialog();
    virtual void dispose() override;
};

#endif // INCLUDED_SFX2_INC_TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
