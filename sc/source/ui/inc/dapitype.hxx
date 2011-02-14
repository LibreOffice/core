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

#ifndef SC_DAPITYPE_HXX
#define SC_DAPITYPE_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>

//------------------------------------------------------------------------

class ScDataPilotSourceTypeDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    RadioButton     aBtnSelection;
    RadioButton     aBtnDatabase;
    RadioButton     aBtnExternal;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotSourceTypeDlg( Window* pParent, sal_Bool bEnableExternal );
            ~ScDataPilotSourceTypeDlg();

    sal_Bool    IsDatabase() const;
    sal_Bool    IsExternal() const;
};


class ScDataPilotServiceDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    FixedText       aFtService;
    ListBox         aLbService;
    FixedText       aFtSource;
    Edit            aEdSource;
    FixedText       aFtName;
    Edit            aEdName;
    FixedText       aFtUser;
    Edit            aEdUser;
    FixedText       aFtPasswd;
    Edit            aEdPasswd;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotServiceDlg( Window* pParent,
                    const com::sun::star::uno::Sequence<rtl::OUString>& rServices );
            ~ScDataPilotServiceDlg();

    String  GetServiceName() const;
    String  GetParSource() const;
    String  GetParName() const;
    String  GetParUser() const;
    String  GetParPass() const;
};


#endif


