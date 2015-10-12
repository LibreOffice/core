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

#include <vcl/accel.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>
#include <sfx2/basedlgs.hxx>
#include <vector>

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    BitmapEx           aBackgroundBitmap;
    BitmapEx           aLogoBitmap;

    VclPtr<VclMultiLineEdit>    m_pVersion;
    VclPtr<FixedText>           m_pDescriptionText;
    VclPtr<FixedText>           m_pCopyrightText;
    VclPtr<FixedImage>          m_pLogoImage;
    VclPtr<FixedText>           m_pLogoReplacement;
    VclPtr<PushButton>          m_pCreditsButton;
    VclPtr<PushButton>          m_pWebsiteButton;

    OUString m_aVersionTextStr;
    OUString m_aVendorTextStr;
    OUString m_aCopyrightTextStr;
    OUString m_aBasedTextStr;
    OUString m_aBasedDerivedTextStr;
    OUString m_aCreditsLinkStr;
    OUString m_sBuildStr;
    OUString m_aLocaleStr;

    void StyleControls();
    void SetLogo();

    static OUString GetBuildId();
    OUString GetVersionString();
    OUString GetCopyrightString();
    static OUString GetLocaleString();

protected:
    virtual bool Close() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void Resize() override;

public:
    AboutDialog(vcl::Window* pParent);
    virtual ~AboutDialog();
    virtual void dispose() override;

    DECL_LINK_TYPED( HandleClick, Button*, void );
};

#endif // INCLUDED_CUI_SOURCE_INC_ABOUT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
