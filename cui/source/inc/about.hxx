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

#include <vcl/bitmapex.hxx>
#include <vcl/weld.hxx>

class AboutDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::LinkButton> m_pCreditsButton;
    std::unique_ptr<weld::LinkButton> m_pWebsiteButton;
    std::unique_ptr<weld::LinkButton> m_pReleaseNotesButton;
    std::unique_ptr<weld::Button> m_pCloseButton;
    std::unique_ptr<weld::Button> m_pCopyButton;

    std::unique_ptr<weld::Image> m_pBrandImage;
    std::unique_ptr<weld::Image> m_pAboutImage;
    std::unique_ptr<weld::Label> m_pVersionLabel;
    std::unique_ptr<weld::Label> m_pBuildCaption;
    std::unique_ptr<weld::LinkButton> m_pBuildLabel;
    std::unique_ptr<weld::Label> m_pEnvLabel;
    std::unique_ptr<weld::Label> m_pUILabel;
    std::unique_ptr<weld::Label> m_pLocaleLabel;
    std::unique_ptr<weld::Label> m_pMiscLabel;
    std::unique_ptr<weld::Label> m_pCopyrightLabel;

    static OUString GetVersionString();
    static OUString GetBuildString();
    static OUString GetLocaleString(bool bLocalized = true);
    static OUString GetMiscString();

    static OUString GetCopyrightString();
    static bool IsStringValidGitHash(const OUString& hash);

    DECL_LINK(HandleClick, weld::Button&, void);

public:
    AboutDialog(weld::Window* pParent);
    virtual ~AboutDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
