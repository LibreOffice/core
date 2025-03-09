/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <datamapper.hxx>

#include <sal/config.h>

#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include "datatableview.hxx"
#include <memory>

class ScDocument;
class ScDataTransformationBaseControl;
class ScDBData;

class ScDataProviderDlg : public weld::GenericDialogController
{
private:
    std::shared_ptr<ScDocument> mxDoc;
    std::unique_ptr<weld::Container> mxBox;
    css::uno::Reference<css::awt::XWindow> m_xTableParent;
    VclPtr<ScDataTableView> mxTable;
    std::unique_ptr<weld::Container> mxList;
    std::unique_ptr<weld::ComboBox> mxDBRanges;
    std::unique_ptr<weld::Button> mxOKBtn;
    std::unique_ptr<weld::Button> mxCancelBtn;
    std::unique_ptr<weld::Button> mxAddTransformationBtn;
    std::unique_ptr<weld::ScrolledWindow> mxScroll;
    std::unique_ptr<weld::Container> mxTransformationList;
    std::unique_ptr<weld::ComboBox> mxTransformationBox;
    std::unique_ptr<weld::ComboBox> mxProviderList;
    std::unique_ptr<weld::Entry> mxEditURL;
    std::unique_ptr<weld::Entry> mxEditID;
    std::unique_ptr<weld::Button> mxApplyBtn;
    std::unique_ptr<weld::Button> mxBrowseBtn;

    OUString msApplyTooltip;
    OUString msAddTransformationToolTip;

    std::vector<std::unique_ptr<ScDataTransformationBaseControl>> maControls;

    Idle maIdle;

    sal_uInt32 mnIndex;
    ScDBData* pDBData;

    DECL_LINK(StartMenuHdl, const OString&, void);
    DECL_LINK(ColumnMenuHdl, const weld::ComboBox&, void);
    DECL_LINK(ScrollToEnd, Timer*, void);
    DECL_LINK(ApplyQuitHdl, weld::Button&, void);
    DECL_LINK(CancelQuitHdl, weld::Button&, void);
    DECL_LINK(TransformationListHdl, weld::Button&, void);
    DECL_LINK(ProviderSelectHdl, weld::ComboBox&, void);
    DECL_LINK(TransformationSelectHdl, weld::ComboBox&, void);
    DECL_LINK(IDEditHdl, weld::Entry&, void);
    DECL_LINK(URLEditHdl, weld::Entry&, void);
    DECL_LINK(ApplyBtnHdl, weld::Button&, void);
    DECL_LINK(BrowseBtnHdl, weld::Button&, void);

public:
    ScDataProviderDlg(weld::Window* pWindow, std::shared_ptr<ScDocument> pDoc,
                      const ScDocument* pDocument);
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
    void findReplaceTransformation();
    void deleteRowTransformation();
    void swapRowsTransformation();

    void updateApplyBtn(bool bValidConfig);
    void isValid();

    sc::ExternalDataSource getDataSource(ScDocument* pDoc);

    void clearTablePreview();
    void import(ScDocument& rDoc, bool bInternal = false);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
