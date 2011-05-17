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
#ifndef _SFXNFLTDLG_HXX
#define _SFXNFLTDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
class SfxFilterMatcher;

class SfxMedium;
class SfxFilterDialog : public ModalDialog
{
private:
    FixedLine               aMimeTypeFL;
    FixedText               aMIMEType;
    FixedText               aExtension;
    FixedLine               aFilterFL;
    ListBox                 aListbox;
    OKButton                aOkBtn;
    CancelButton            aCancelBtn;
    HelpButton              aHelpBtn;

    const SfxFilterMatcher& rMatcher;

    DECL_LINK( OkHdl, Control * );

public:
    SfxFilterDialog( Window * pParent,
                     SfxMedium* pMed,
                     const SfxFilterMatcher &rMatcher,
                     const String *pDefFilter, sal_Bool bShowConvert = sal_True );

    String GetSelectEntry() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
