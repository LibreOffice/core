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
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <sfx2/templateabstractview.hxx>

class Edit;
class PopupMenu;
class SfxTemplateInfoDlg;
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

    void setSaveMode();

    void setDocumentModel (const css::uno::Reference<css::frame::XModel> &rModel);

    DECL_LINK_TYPED(ActivatePageHdl, TabControl*, void);

private:

    void readSettings ();

    void writeSettings ();

    DECL_LINK_TYPED(TBXViewHdl, ToolBox*, void);
    DECL_LINK_TYPED(TBXActionHdl, ToolBox*, void);
    DECL_LINK_TYPED(TBXTemplateHdl, ToolBox*, void);
    DECL_LINK_TYPED(TBXDropdownHdl, ToolBox*, void);

    DECL_LINK_TYPED(TVItemStateHdl, const ThumbnailViewItem*, void);

    DECL_LINK_TYPED(MenuSelectHdl, Menu*, bool);
    DECL_LINK_TYPED(MoveMenuSelectHdl, Menu*, bool);
    DECL_LINK_TYPED(RepositoryMenuSelectHdl, Menu*, bool);
    DECL_LINK_TYPED(DefaultTemplateMenuSelectHdl, Menu*, bool);

    DECL_LINK_TYPED(OpenRegionHdl, void*, void);
    DECL_LINK_TYPED(OpenTemplateHdl, ThumbnailViewItem*, void);

    DECL_LINK_TYPED(SearchUpdateHdl, Edit&, void);

    void OnTemplateImport ();
    void OnTemplateSearch ();
    static void OnTemplateLink ();
    void OnTemplateOpen ();
    void OnTemplateEdit ();
    void OnTemplateProperties ();
    void OnTemplateDelete ();
    void OnTemplateAsDefault ();
    void OnTemplateExport ();

    void OnTemplateState (const ThumbnailViewItem *pItem);

    void OnFolderNew ();
    void OnFolderDelete ();

    void OnRegionState (const ThumbnailViewItem *pItem);

    void OnRepositoryDelete ();
    void OnTemplateSaveAs ();

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

    /// Return filter according to the currently selected tab page.
    FILTER_APPLICATION getCurrentFilter();

private:

    VclPtr<TabControl> mpTabControl;

    VclPtr<Edit> mpSearchEdit;
    VclPtr<ToolBox> mpViewBar;
    VclPtr<ToolBox> mpActionBar;
    VclPtr<ToolBox> mpTemplateBar;
    VclPtr<TemplateSearchView> mpSearchView;
    VclPtr<TemplateAbstractView> mpCurView;
    VclPtr<TemplateLocalView> mpLocalView;
    VclPtr<TemplateRemoteView> mpRemoteView;
    PopupMenu *mpActionMenu;
    PopupMenu *mpRepositoryMenu;
    PopupMenu *mpTemplateDefaultMenu;

    std::set<const ThumbnailViewItem*,selection_cmp_fn> maSelTemplates;
    std::set<const ThumbnailViewItem*,selection_cmp_fn> maSelFolders;

    bool mbIsSaveMode;  ///< Flag that indicates if we are in save mode or not.
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::frame::XDesktop2 > mxDesktop;

    bool mbIsSynced; ///< Tells whether maRepositories is synchronized with the user config
    std::vector<TemplateRepository*> maRepositories; ///< Stores the remote repositories for templates
};

#endif // INCLUDED_SFX2_INC_TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
