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

#ifndef SC_DELCODLG_HXX
#define SC_DELCODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include "global.hxx"

//------------------------------------------------------------------------

class ScDeleteContentsDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    CheckBox        aBtnDelAll;
    CheckBox        aBtnDelStrings;
    CheckBox        aBtnDelNumbers;
    CheckBox        aBtnDelDateTime;
    CheckBox        aBtnDelFormulas;
    CheckBox        aBtnDelNotes;
    CheckBox        aBtnDelAttrs;
    CheckBox        aBtnDelObjects;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    sal_Bool            bObjectsDisabled;

    static sal_Bool     bPreviousAllCheck;
    static sal_uInt16   nPreviousChecks;

    void DisableChecks( sal_Bool bDelAllChecked = sal_True );
    DECL_LINK( DelAllHdl, void * );

public:
            ScDeleteContentsDlg( Window* pParent,
                                 sal_uInt16  nCheckDefaults = 0 );
            ~ScDeleteContentsDlg();
    void    DisableObjects();

    sal_uInt16  GetDelContentsCmdBits() const;
};


#endif // SC_DELCODLG_HXX


