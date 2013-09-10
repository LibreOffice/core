/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEDLG_HXX
#define TEMPLATEDLG_HXX

#include <set>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <com/sun/star/frame/XDesktop2.hpp>

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

class SfxTemplateManagerDlg : public ModelessDialog
{
    typedef bool (*selection_cmp_fn)(const ThumbnailViewItem*,const ThumbnailViewItem*);

public:

    SfxTemplateManagerDlg (Window *parent = DIALOG_NO_PARENT);

    ~SfxTemplateManagerDlg ();

    void setSaveMode (bool bMode);

    void setDocumentModel (const com::sun::star::uno::Reference<com::sun::star::frame::XModel> &rModel);

    DECL_LINK(ActivatePageHdl, void*);

private:

    void readSettings ();

    void writeSettings ();

    virtual void Resize ();

    DECL_LINK(TBXViewHdl, void*);
    DECL_LINK(TBXActionHdl, void*);
    DECL_LINK(TBXTemplateHdl, void*);
    DECL_LINK(TBXDropdownHdl, ToolBox*);

    DECL_LINK(TVItemStateHdl, const ThumbnailViewItem*);

    DECL_LINK(MenuSelectHdl, Menu*);
    DECL_LINK(MoveMenuSelectHdl, Menu*);
    DECL_LINK(RepositoryMenuSelectHdl, Menu*);
    DECL_LINK(DefaultTemplateMenuSelectHdl, Menu*);

    DECL_LINK(OpenRegionHdl, void*);
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*);

    DECL_LINK(SearchUpdateHdl, void*);

    void OnTemplateImport ();
    void OnTemplateSearch ();
    void OnTemplateLink ();
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

private:

    TabControl maTabControl;
    TabPage maTabPage;

    Edit *mpSearchEdit;
    ToolBox *mpViewBar;
    ToolBox *mpActionBar;
    ToolBox *mpTemplateBar;
    TemplateSearchView *mpSearchView;
    TemplateAbstractView *mpCurView;
    TemplateLocalView *mpLocalView;
    TemplateRemoteView *mpRemoteView;
    PopupMenu *mpActionMenu;
    PopupMenu *mpRepositoryMenu;
    PopupMenu *mpTemplateDefaultMenu;

    std::set<const ThumbnailViewItem*,selection_cmp_fn> maSelTemplates;
    std::set<const ThumbnailViewItem*,selection_cmp_fn> maSelFolders;

    bool mbIsSaveMode;  ///< Flag that indicates if we are in save mode or not.
    com::sun::star::uno::Reference< com::sun::star::frame::XModel > m_xModel;
    com::sun::star::uno::Reference< com::sun::star::frame::XDesktop2 > mxDesktop;

    bool mbIsSynced; ///< Tells whether maRepositories is synchronized with the user config
    std::vector<TemplateRepository*> maRepositories; ///< Stores the remote repositories for templates
};

#endif // TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
