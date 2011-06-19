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
#ifndef _SVX_RECOVER_HXX
#define _SVX_RECOVER_HXX

#include <layout/layout.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <layout/core/box.hxx>
#include <layout/layout-pre.hxx>

class SvxRecoverDialog : public SfxModalDialog
{
private:
    FixedImage aHeaderImage;
    FixedText aHeaderText;
    FixedLine aHeaderLine;
    FixedText aRecoverText;
    FixedText aTextAdvanced;

    CheckBox aCheckBoxDoc;
    FixedImage aImageDoc;
    FixedText aTextDoc;
    CheckBox aCheckBoxSheet;
    FixedImage aImageSheet;
    FixedText aTextSheet;
    CheckBox aCheckBoxDraw;
    FixedImage aImageDraw;
    FixedText aTextDraw;
    CheckBox aCheckBoxPresent;
    FixedImage aImagePresent;
    FixedText aTextPresent;
    AdvancedButton aButtonAdvanced;

    FixedText aProgressText;
    ProgressBar aProgressBar;
    CheckBox aCheckBoxLogFile;
    OKButton aOKBtn;
    CancelButton aCancelBtn;
    HelpButton aHelpBtn;

public:
#if TEST_LAYOUT
    SvxRecoverDialog( Window* pParent );
#else /* !TEST_LAYOUT */
    SvxRecoverDialog( Window* pParent, const SfxItemSet& rCoreSet );
#endif /* !TEST_LAYOUT */
    ~SvxRecoverDialog();
};

#include <layout/layout-post.hxx>

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
