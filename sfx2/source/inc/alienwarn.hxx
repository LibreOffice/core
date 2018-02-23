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
#ifndef INCLUDED_SFX2_SOURCE_INC_ALIENWARN_HXX
#define INCLUDED_SFX2_SOURCE_INC_ALIENWARN_HXX

#include <vcl/weld.hxx>

class SfxAlienWarningDialog
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::MessageDialog> m_xDialog;
    std::unique_ptr<weld::Button> m_xKeepCurrentBtn;
    std::unique_ptr<weld::Button> m_xUseDefaultFormatBtn;
    std::unique_ptr<weld::CheckButton> m_xWarningOnBox;

    std::unique_ptr<weld::Container> m_xOrigParent;
    std::unique_ptr<weld::Container> m_xContentArea;

public:
    SfxAlienWarningDialog(weld::Window* pParent, const OUString& _rFormatName,
                          const OUString& _rDefaultExtension, bool rDefaultIsAlien);
    short run() { return m_xDialog->run(); }
    ~SfxAlienWarningDialog();
};

#endif // INCLUDED_SFX2_SOURCE_INC_ALIENWARN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
