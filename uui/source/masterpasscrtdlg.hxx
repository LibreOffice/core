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

#ifndef UUI_MASTERPASSCRTDLG_HXX
#define UUI_MASTERPASSCRTDLG_HXX

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>

//============================================================================
class MasterPasswordCreateDialog : public ModalDialog
{
private:
    FixedText       aFTInfoText;
    FixedLine       aFLInfoText;

    FixedText       aFTMasterPasswordCrt;
    Edit            aEDMasterPasswordCrt;
    FixedText       aFTMasterPasswordRepeat;
    Edit            aEDMasterPasswordRepeat;

    FixedText       aFTCautionText;
    FixedLine       aFLCautionText;

    FixedText       aFTMasterPasswordWarning;
    FixedLine       aFL;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;


    DECL_LINK( OKHdl_Impl, OKButton * );
    DECL_LINK( EditHdl_Impl, Edit * );

public:
    MasterPasswordCreateDialog( Window* pParent, ResMgr * pResMgr );

    String          GetMasterPassword() const { return aEDMasterPasswordCrt.GetText(); }

private:
    ResMgr*                                         pResourceMgr;
    sal_uInt16                                      nMinLen;

    void            CalculateTextHeight();
};

#endif // UUI_MASTERPASSCRTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
