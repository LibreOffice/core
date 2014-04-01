/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_SELECTDBTABLEDIALOG_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_SELECTDBTABLEDIALOG_HXX

#include <sfx2/basedlgs.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
namespace com{namespace sun{namespace star{
    namespace sdbc{
        class XConnection;
    }
}}}

class SwSelectDBTableDialog : public SfxModalDialog
{
    FixedText       m_aSelectFI;
    HeaderBar       m_aTableHB;
    SvTabListBox    m_aTableLB;
    PushButton      m_aPreviewPB;

    FixedLine       m_aSeparatorFL;

    OKButton        m_aOK;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;

    OUString        m_sName;
    OUString        m_sType;
    OUString        m_sTable;
    OUString        m_sQuery;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> m_xConnection;

    DECL_LINK(PreviewHdl, PushButton*);
public:

    SwSelectDBTableDialog(Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection
            );
    virtual ~SwSelectDBTableDialog();

    OUString    GetSelectedTable(bool& bIsTable);
    void        SetSelectedTable(const OUString& rTable, bool bIsTable);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
