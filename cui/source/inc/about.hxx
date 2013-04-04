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
#ifndef _ABOUT_HXX
#define _ABOUT_HXX

#include <vcl/accel.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>
#include <sfx2/basedlgs.hxx>        ///< for SfxModalDialog
#include <vector>

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    BitmapEx           aBackgroundBitmap;
    BitmapEx           aLogoBitmap;

    VclMultiLineEdit*    m_pVersion;
    FixedText*           m_pDescriptionText;
    FixedText*           m_pCopyrightText;
    FixedImage*          m_pLogoImage;
    FixedText*           m_pLogoReplacement;
    PushButton*          m_pCreditsButton;
    PushButton*          m_pWebsiteButton;

    OUString aVersionData;
    OUString m_aVersionTextStr;
    OUString m_aVendorTextStr;
    OUString m_aCopyrightTextStr;
    OUString m_aBasedTextStr;
    OUString m_aBasedDerivedTextStr;
    OUString m_aCreditsLinkStr;
    OUString m_sBuildStr;

    void StyleControls();
    void SetLogo();

    OUString GetBuildId();
    OUString GetVersionString();
    OUString GetCopyrightString();

protected:
    virtual sal_Bool Close();
    virtual void Paint( const Rectangle& rRect );
    virtual void Resize();

public:
    AboutDialog(Window* pParent);

    DECL_LINK( HandleClick, PushButton* );
};

#endif // #ifndef _ABOUT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
