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

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/listctrl.hxx>
#include <vcl/menu.hxx>

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
    sal_uInt32 mpIndex;
    ScDBData* pDBData;

    void InitMenu();

    DECL_LINK( StartMenuHdl, Menu*, bool );
    DECL_LINK( ColumnMenuHdl, Menu*, bool );
    DECL_LINK( ImportHdl, Window*, void );

public:

    ScDataProviderDlg(vcl::Window* pWindow, std::shared_ptr<ScDocument> pDoc, const ScDocument* pDocument);

    virtual ~ScDataProviderDlg() override;
    virtual void dispose() override;

    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;

    void applyAndQuit();
    void cancelAndQuit();

    void deleteColumn();
    void splitColumn();
    void mergeColumns();
    void textTransformation();
    void sortTransformation();
    void aggregateFunction();
    void numberTransformation();
    void deletefromList(sal_uInt32 nIndex);
    void replaceNullTransformation();
    void dateTimeTransformation();

    void import(ScDocument* pDoc, bool bInternal = false);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
