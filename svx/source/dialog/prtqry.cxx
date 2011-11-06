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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <svx/prtqry.hxx>
#include <svx/dialmgr.hxx>
#include <tools/shl.hxx>

/* -----------------------------01.02.00 13:57--------------------------------

 ---------------------------------------------------------------------------*/
SvxPrtQryBox::SvxPrtQryBox(Window* pParent) :
    MessBox(pParent, 0,
            String(SVX_RES(RID_SVXSTR_QRY_PRINT_TITLE)),
            String(SVX_RES(RID_SVXSTR_QRY_PRINT_MSG)))
{
    SetImage( QueryBox::GetStandardImage() );

    AddButton(String(SVX_RES(RID_SVXSTR_QRY_PRINT_SELECTION)), RET_OK,
            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON);

    AddButton(String(SVX_RES(RID_SVXSTR_QRY_PRINT_ALL)), 2, 0);
    AddButton(BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON);
    SetButtonHelpText( RET_OK, String() );
}
/* -----------------------------01.02.00 13:57--------------------------------

 ---------------------------------------------------------------------------*/
SvxPrtQryBox::~SvxPrtQryBox()
{
}


