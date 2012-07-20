/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _ABOUT_HXX
#define _ABOUT_HXX

// include ---------------------------------------------------------------

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
