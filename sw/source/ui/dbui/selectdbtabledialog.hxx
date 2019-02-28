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

namespace com{namespace sun{namespace star{
    namespace sdbc{
        class XConnection;
    }
}}}

class SwSelectDBTableDialog : public SfxDialogController
{
    css::uno::Reference<css::sdbc::XConnection> m_xConnection;

    std::unique_ptr<weld::TreeView> m_xTable;
    std::unique_ptr<weld::Button> m_xPreviewPB;

    DECL_LINK(PreviewHdl, weld::Button&, void);
public:
    SwSelectDBTableDialog(weld::Window* pParent,
        const css::uno::Reference<css::sdbc::XConnection>& xConnection);
    virtual ~SwSelectDBTableDialog() override;

    OUString    GetSelectedTable(bool& bIsTable);
    void        SetSelectedTable(const OUString& rTable, bool bIsTable);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
