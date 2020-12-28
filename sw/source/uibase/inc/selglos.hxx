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

#include <sal/config.h>

#include <string_view>

#include <vcl/weld.hxx>

class SwSelGlossaryDlg final : public weld::GenericDialogController
{
    std::unique_ptr<weld::Frame> m_xFrame;
    std::unique_ptr<weld::TreeView> m_xGlosBox;

    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
public:
    SwSelGlossaryDlg(weld::Window * pParent, std::u16string_view rShortName);
    virtual ~SwSelGlossaryDlg() override;

    void InsertGlos(std::u16string_view rRegion, std::u16string_view rGlosName)
    {
        const OUString aTmp = OUString::Concat(rRegion) + ":" + rGlosName;
        m_xGlosBox->append_text(aTmp);
    }
    sal_Int32 GetSelectedIdx() const
    {
        return m_xGlosBox->get_selected_index();
    }
    void SelectEntryPos(sal_Int32 nIdx)
    {
        m_xGlosBox->select(nIdx);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
