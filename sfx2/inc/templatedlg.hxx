/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
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

class Edit;
class PopupMenu;
class TemplateAbstractView;
class TemplateLocalView;
class TemplateOnlineView;
class TemplateSearchView;
class ThumbnailView;
class ThumbnailViewItem;
class ToolBox;

namespace com {
    namespace sun { namespace star { namespace frame {
        class XComponentLoader;
    }   }   }
}

class SfxTemplateManagerDlg : public ModelessDialog
{
public:

    SfxTemplateManagerDlg (Window *parent = NULL);

    ~SfxTemplateManagerDlg ();

    DECL_LINK(ViewAllHdl, void*);
    DECL_LINK(ViewDocsHdl, void*);
    DECL_LINK(ViewPresentsHdl, void*);
    DECL_LINK(ViewSheetsHdl, void*);
    DECL_LINK(ViewDrawsHdl, void*);

private:

    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    DECL_LINK(CloseOverlayHdl, void*);

    DECL_LINK(OnClickSelectionMode, ImageButton*);

    DECL_LINK(TBXViewHdl, void*);
    DECL_LINK(TBXActionHdl, void*);
    DECL_LINK(TBXTemplateHdl, void*);
    DECL_LINK(TBXDropdownHdl, ToolBox*);

    DECL_LINK(TVFolderStateHdl, const ThumbnailViewItem*);
    DECL_LINK(TVTemplateStateHdl, const ThumbnailViewItem*);

    DECL_LINK(MenuSelectHdl, Menu*);
    DECL_LINK(MoveMenuSelectHdl, Menu*);
    DECL_LINK(RepositoryMenuSelectHdl, Menu*);
    DECL_LINK(DefaultTemplateMenuSelectHdl, Menu*);

    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*);

    DECL_LINK(SearchUpdateHdl, void*);

    void OnTemplateImport ();
    void OnTemplateSearch ();
    void OnTemplateEdit ();
    void OnTemplateProperties ();
    void OnTemplateDelete ();
    void OnTemplateAsDefault ();
    void OnFolderDelete ();
    void OnRepositoryDelete ();

    void centerTopButtons ();

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

private:

    PushButton aButtonAll;
    PushButton aButtonDocs;
    PushButton aButtonPresents;
    PushButton aButtonSheets;
    PushButton aButtonDraws;
    ImageButton maButtonSelMode;

    Edit *mpSearchEdit;
    ToolBox *mpViewBar;
    ToolBox *mpActionBar;
    ToolBox *mpTemplateBar;
    TemplateSearchView *mpSearchView;
    TemplateAbstractView *mpCurView;
    TemplateLocalView *maView;
    TemplateOnlineView *mpOnlineView;
    PopupMenu *mpActionMenu;
    PopupMenu *mpRepositoryMenu;
    PopupMenu *mpTemplateDefaultMenu;

    int mnSelectionCount;
    std::set<const ThumbnailViewItem*> maSelTemplates;
    std::set<const ThumbnailViewItem*> maSelFolders;

    com::sun::star::uno::Reference< com::sun::star::frame::XComponentLoader > mxDesktop;
};

#endif // TEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
