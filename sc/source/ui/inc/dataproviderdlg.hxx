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

#include "address.hxx"
#include "datamapper.hxx"
#include "dataprovider.hxx"

class ScDocShell;
class SvtURLBox;
class ScRange;
class ComboBox;

namespace sc {

class DataProviderDlg : public ModalDialog
{
    ScDocShell *mpDocShell;

    VclPtr<SvtURLBox>      m_pCbUrl;
    VclPtr<PushButton>     m_pBtnBrowse;
    VclPtr<OKButton>       m_pBtnOk;
    VclPtr<ListBox>        m_pCBData;
    VclPtr<ListBox>        m_pCBProvider;
    VclPtr<Edit>           m_pEdID;

    DECL_LINK(UpdateClickHdl, Button*, void);
    DECL_LINK(UpdateComboBoxHdl, ComboBox&, void);
    DECL_LINK(BrowseHdl, Button*, void);
    DECL_LINK(EditHdl, Edit&, void);
    DECL_LINK(SelectHdl, ListBox&, void);

    void UpdateEnable();

    std::shared_ptr<ExternalDataSource> mpDataSource;

public:
    DataProviderDlg(ScDocShell *pDocShell, vcl::Window* pParent);
    virtual ~DataProviderDlg() override;
    virtual void dispose() override;

    void Init();

    void StartImport();
};

}

#endif // INCLUDED_SC_SOURCE_UI_INC_DATAPROVIDERDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
