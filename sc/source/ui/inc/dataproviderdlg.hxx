/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_DATAPROVIDERDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DATAPROVIDERDLG_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/listctrl.hxx>
#include <vcl/button.hxx>

#include "datatableview.hxx"

#include <memory>

class ScDocument;
class ScDataProviderBaseControl;
class ScDBData;

class ScDataProviderDlg : public ModalDialog
{
private:

    std::shared_ptr<ScDocument> mpDoc;
    VclPtr<ScDataTableView> mpTable;
    VclPtr<ListControl> mpList;
    VclPtr<MenuBar> mpBar;
    VclPtr<ScDataProviderBaseControl> mpDataProviderCtrl;
    VclPtr<ListBox> mpDBRanges;

    ScDBData* pDBData;

    ScDocument* mpDocument;

    void InitMenu();

    DECL_LINK( StartMenuHdl, Menu*, bool );
    DECL_LINK( ColumnMenuHdl, Menu*, bool );
    DECL_LINK( ImportHdl, Window*, void );

public:

    ScDataProviderDlg(vcl::Window* pWindow, std::shared_ptr<ScDocument> pDoc, ScDocument* pDocument);

    virtual ~ScDataProviderDlg() override;
    virtual void dispose() override;

    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;

    void applyAndQuit();
    void cancelAndQuit();

    void deleteColumn();
    void splitColumn();
    void mergeColumns();

    void import();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
