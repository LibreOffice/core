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
#include <svtools/svmedit.hxx>
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class ModalDialogGROSSER_TEST_DLG : public ModalDialog
{
protected:
    CheckBox aCheckBox1;
    TriStateBox aTriStateBox1;
    OKButton aOKButton1;
    TimeField aTimeField1;
    MultiLineEdit aMultiLineEdit1;
    GroupBox aGroupBox1;
    RadioButton aRadioButton1;
    RadioButton aRadioButton2;
    MultiListBox aMultiListBox1;
    ComboBox aComboBox1;
    DateBox aDateBox1;
    FixedText aFixedText1;
public:
    ModalDialogGROSSER_TEST_DLG( Window * pParent, const ResId & rResId, sal_Bool bFreeRes = sal_True );
};

class MenuMENU_CLIENT : public MenuBar
{
protected:
public:
    MenuMENU_CLIENT( const ResId & rResId, sal_Bool bFreeRes = sal_True );
};

