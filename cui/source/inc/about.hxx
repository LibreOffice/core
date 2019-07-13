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
#ifndef INCLUDED_CUI_SOURCE_INC_ABOUT_HXX
#define INCLUDED_CUI_SOURCE_INC_ABOUT_HXX

#include <vcl/bitmapex.hxx>
#include <vcl/weld.hxx>

class AboutDialog : public weld::DialogController
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::shared_ptr<weld::AboutDialog> m_xDialog;
    std::shared_ptr<weld::Container> m_xContentArea;

    BitmapEx           aLogoBitmap;
    BitmapEx           aBackgroundBitmap;

    OUString m_buildIdLinkString;

    weld::Button* m_pCreditsButton;
    weld::Button* m_pWebsiteButton;
    weld::Button* m_pReleaseNotesButton;
    weld::Button* m_pCloseButton;

    void SetBuildIdLink();
    void SetLogo();

    static OUString GetBuildId();
    static OUString GetVersionString();
    static OUString GetCopyrightString();
    static OUString GetLocaleString();
    static bool IsStringValidGitHash(const OUString& hash);

public:
    AboutDialog(weld::Window* pParent);
    virtual weld::Dialog* getDialog() override { return m_xDialog.get(); }
    virtual ~AboutDialog() override;

    DECL_LINK(HandleClick, weld::Button&, void);
    DECL_LINK(SizeAllocHdl, const Size&, void);
};

#endif // INCLUDED_CUI_SOURCE_INC_ABOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
