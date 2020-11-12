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

#pragma once

#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <set>

class SfxItemSet;
namespace dbaui
{
// ODatasourceSelector
class ODatasourceSelectDialog final : public weld::GenericDialogController
{
    std::unique_ptr<weld::TreeView> m_xDatasource;
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xCancel;
    std::unique_ptr<weld::Button> m_xManageDatasources;
#ifdef HAVE_ODBC_ADMINISTRATION
    std::unique_ptr<OOdbcManagement> m_xODBCManagement;
#endif

public:
    ODatasourceSelectDialog(weld::Window* pParent, const std::set<OUString>& rDatasources);
    virtual ~ODatasourceSelectDialog() override;
    OUString GetSelected() const { return m_xDatasource->get_selected_text(); }
    void Select(const OUString& _rEntry) { m_xDatasource->select_text(_rEntry); }

    virtual short run() override;

private:
    DECL_LINK(ListDblClickHdl, weld::TreeView&, bool);
#ifdef HAVE_ODBC_ADMINISTRATION
    DECL_LINK(ManageClickHdl, weld::Button&, void);
    DECL_LINK(ManageProcessFinished, void*, void);
#endif
    void fillListBox(const std::set<OUString>& _rDatasources);
};

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
