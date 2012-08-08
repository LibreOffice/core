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
#ifndef _SVX_PASSWD_HXX
#define _SVX_PASSWD_HXX

#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include <vcl/button.hxx>
#include "svx/svxdllapi.h"

// class SvxPasswordDialog -----------------------------------------------

class SVX_DLLPUBLIC SvxPasswordDialog : public SfxModalDialog
{
private:
    FixedLine       aOldFL;
    FixedText       aOldPasswdFT;
    Edit            aOldPasswdED;
    FixedLine       aNewFL;
    FixedText       aNewPasswdFT;
    Edit            aNewPasswdED;
    FixedText       aRepeatPasswdFT;
    Edit            aRepeatPasswdED;
    OKButton        aOKBtn;
    CancelButton    aEscBtn;
    HelpButton      aHelpBtn;

    String          aOldPasswdErrStr;
    String          aRepeatPasswdErrStr;

    Link            aCheckPasswordHdl;

    sal_Bool            bEmpty;

    DECL_LINK(ButtonHdl, void *);
    DECL_LINK(EditModifyHdl, void *);

public:
                    SvxPasswordDialog( Window* pParent, sal_Bool bAllowEmptyPasswords = sal_False, sal_Bool bDisableOldPassword = sal_False );
                    ~SvxPasswordDialog();

    String          GetOldPassword() const { return aOldPasswdED.GetText(); }
    String          GetNewPassword() const { return aNewPasswdED.GetText(); }

    void            SetCheckPasswordHdl( const Link& rLink ) { aCheckPasswordHdl = rLink; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
