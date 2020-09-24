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

#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include "datatableview.hxx"
#include <memory>

class ScDocument;
class ScDataProviderBaseControl;
class ScDataTransformationBaseControl;
class ScDBData;

class ScDataProviderDlg : public weld::GenericDialogController
{
private:
    std::shared_ptr<ScDocument> mxDoc;
    std::unique_ptr<weld::Menu> mxStartMenu;
    std::unique_ptr<weld::Menu> mxColumnMenu;
    std::unique_ptr<weld::Container> mxBox;
    css::uno::Reference<css::awt::XWindow> m_xTableParent;
    VclPtr<ScDataTableView> mxTable;
    std::unique_ptr<weld::ScrolledWindow> mxScroll;
    std::unique_ptr<weld::Container> mxList;
    std::unique_ptr<ScDataProviderBaseControl> mxDataProviderCtrl;
    std::unique_ptr<weld::ComboBox> mxDBRanges;

    std::vector<std::unique_ptr<ScDataTransformationBaseControl>> maControls;

    Idle maIdle;

    sal_uInt32 mnIndex;
    ScDBData* pDBData;

    void InitMenu();

    DECL_LINK( StartMenuHdl, const OString&, void );
    DECL_LINK( ColumnMenuHdl, const OString&, void );
    DECL_LINK( ImportHdl, ScDataProviderBaseControl*, void );
    DECL_LINK( ScrollToEnd, Timer*, void );

public:
    ScDataProviderDlg(weld::Window* pWindow, std::shared_ptr<ScDocument> pDoc, const ScDocument* pDocument);
    virtual ~ScDataProviderDlg() override;

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

    void import(ScDocument& rDoc, bool bInternal = false);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
