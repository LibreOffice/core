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


#ifndef _INPDLG_HXX
#define _INPDLG_HXX

#include <svx/stddlg.hxx>

#ifndef _SV_SVMEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif
#include <vcl/fixed.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwInputField;
class SwSetExpField;
class SwUserFieldType;
class SwField;
class SwWrtShell;

/*--------------------------------------------------------------------
     Beschreibung: Einfuegen Felder
 --------------------------------------------------------------------*/

class SwFldInputDlg: public SvxStandardDialog
{
    virtual void    Apply();
    virtual void    StateChanged( StateChangedType );

    SwWrtShell         &rSh;
    SwInputField*       pInpFld;
    SwSetExpField*      pSetFld;
    SwUserFieldType*    pUsrType;

    Edit                aLabelED;

    MultiLineEdit       aEditED;
    FixedLine           aEditFL;

    OKButton            aOKBT;
    CancelButton        aCancelBT;
    PushButton          aNextBT;
    HelpButton          aHelpBT;

    DECL_LINK(NextHdl, PushButton*);
public:
    SwFldInputDlg(  Window *pParent, SwWrtShell &rSh,
                    SwField* pField, sal_Bool bNextButton = sal_False );
    ~SwFldInputDlg();
};


#endif
