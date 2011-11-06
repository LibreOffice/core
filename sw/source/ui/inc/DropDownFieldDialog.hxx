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


#ifndef _SW_DROPDOWNFIELDDIALOG_HXX
#define _SW_DROPDOWNFIELDDIALOG_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwDropDownField;
class SwField;
class SwWrtShell;

/*--------------------------------------------------------------------
     Dialog to edit drop down field selection
 --------------------------------------------------------------------*/
namespace sw
{
class DropDownFieldDialog : public SvxStandardDialog
{
    FixedLine           aItemsFL;
    ListBox             aListItemsLB;

    OKButton            aOKPB;
    CancelButton        aCancelPB;
    PushButton          aNextPB;
    HelpButton          aHelpPB;

    PushButton          aEditPB;

    SwWrtShell          &rSh;
    SwDropDownField*    pDropField;

    DECL_LINK(ButtonHdl, PushButton*);
    virtual void    Apply();
public:
    DropDownFieldDialog(   Window *pParent, SwWrtShell &rSh,
                                SwField* pField, sal_Bool bNextButton = sal_False );
    ~DropDownFieldDialog();
};
} //namespace sw


#endif
