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

#include <vcl/button.hxx>
#include <vcl/accel.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/stdctrl.hxx>
#include "svtools/fixedhyper.hxx"
#include <sfx2/basedlgs.hxx>        ///< for SfxModalDialog
#include <vector>

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    BitmapEx           aBackgroundBitmap;
    BitmapEx           aLogoBitmap;


    MultiLineEdit       aVersionText;
    MultiLineEdit       aDescriptionText;
    MultiLineEdit       aCopyrightText;
    FixedImage          aLogoImage;
    MultiLineEdit       aLogoReplacement;
    PushButton          aCreditsButton;
    PushButton          aWebsiteButton;
    CancelButton        aCancelButton;

    String aVersionData;
    rtl::OUString m_aVersionTextStr;
    String m_aVendorTextStr;
    String m_aCopyrightTextStr;
    String m_aBasedTextStr;
    String m_aBasedDerivedTextStr;
    String m_aCreditsLinkStr;
    rtl::OUString m_sBuildStr;
    String m_aDescriptionTextStr;

    void StyleControls();
    void LayoutControls();
    void LayoutButtons(sal_Int32 aContentWidth, sal_Int32 aDialogBorder,
                       Point& aButtonPos, Size& aButtonSize, sal_Int32& aButtonsWidth );
    void MoveControl(Control& rControl, sal_Int32 X, sal_Int32 Y);
    rtl::OUString GetBuildId();
    rtl::OUString GetVersionString();
    rtl::OUString GetCopyrightString();

protected:
    virtual sal_Bool Close();
    virtual void     Paint( const Rectangle& rRect );

public:
    AboutDialog( Window* pParent, const ResId& rId);

    DECL_LINK( CancelHdl, void * );
    DECL_LINK( HandleClick, PushButton* );
};

#endif // #ifndef _ABOUT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
