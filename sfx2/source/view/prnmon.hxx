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

#include <memory>
#include <sal/config.h>
#include <vcl/vclptr.hxx>
#include <vcl/weld.hxx>


class SfxViewShell;
class SfxTabPage;
class SfxItemSet;


class SfxPrintOptionsDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<SfxItemSet>            pOptions;
    std::unique_ptr<weld::Widget>    m_xHelpBtn;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<SfxTabPage> m_xPage;

public:
                            SfxPrintOptionsDialog(weld::Window *pParent,
                                                  SfxViewShell *pViewShell,
                                                  const SfxItemSet *rOptions);
    virtual                 ~SfxPrintOptionsDialog() override;

    virtual short run() override;

    const SfxItemSet&       GetOptions() const { return *pOptions; }
    void                    DisableHelp();
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
