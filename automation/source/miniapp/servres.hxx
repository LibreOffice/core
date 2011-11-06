/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

