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
#include <svtools/svmedit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/menu.hxx>
#include <vcl/lstbox.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
