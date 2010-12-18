/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langbox.hxx,v $
 * $Revision: 1.4.242.1 $
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

#ifndef SC_UI_IMPORTOPTIONS_HXX
#define SC_UI_IMPORTOPTIONS_HXX

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "i18npool/lang.h"
#include "svx/langbox.hxx"

class ScTextImportOptionsDlg : public ModalDialog
{
public:
    ScTextImportOptionsDlg(Window* pParent);
    virtual ~ScTextImportOptionsDlg();

    virtual short Execute();

    LanguageType getLanguageType() const;
    bool isDateConversionSet() const;

private:
    void init();

private:
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    FixedLine       maFlChooseLang;

    RadioButton     maRbAutomatic;
    RadioButton     maRbCustom;

    SvxLanguageBox  maLbCustomLang;

    FixedLine       maFlOption;

    CheckBox        maBtnConvertDate;

    DECL_LINK( OKHdl, OKButton* );

    DECL_LINK( RadioHdl, RadioButton* );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
