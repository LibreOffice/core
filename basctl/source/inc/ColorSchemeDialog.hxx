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

#include <vcl/weld.hxx>

namespace basctl
{
class ModulWindowLayout;
class BasicColorConfig;

class ColorSchemeDialog : public weld::GenericDialogController
{
private:
    VclPtr<ModulWindowLayout> m_pModulWinLayout;
    OUString m_sSelectedSchemeId;

    std::unique_ptr<weld::TreeView> m_xSchemeList;
    std::unique_ptr<weld::RadioButton> m_xUseAppCollors;
    std::unique_ptr<weld::RadioButton> m_xUseScheme;
    std::unique_ptr<weld::Button> m_xOk;
    std::shared_ptr<BasicColorConfig> m_pColorConfig;

    void Init();

    DECL_LINK(BtnOkHdl, weld::Button&, void);
    DECL_LINK(OptionHdl, weld::Toggleable&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    ColorSchemeDialog(weld::Window* pParent, VclPtr<ModulWindowLayout> pModulWinLayout);
    virtual ~ColorSchemeDialog() override;

    const OUString& GetColorSchemeId() { return m_sSelectedSchemeId; }
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
