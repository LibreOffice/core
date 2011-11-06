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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#include "swtypes.hxx"

#include "selglos.hxx"

#include "selglos.hrc"
#include "dochdl.hrc"
#include <vcl/msgbox.hxx>

// STATIC DATA -----------------------------------------------------------


// CTOR / DTOR -----------------------------------------------------------

SwSelGlossaryDlg::SwSelGlossaryDlg(Window * pParent, const String &rShortName)
    : ModalDialog(pParent, SW_RES(DLG_SEL_GLOS)),
    aGlosBox(this, SW_RES( LB_GLOS)),
    aGlosFL(this, SW_RES( FL_GLOS)),
    aOKBtn(this, SW_RES( BT_OK)),
    aCancelBtn(this, SW_RES( BT_CANCEL)),
    aHelpBtn(this, SW_RES(BT_HELP))
{
    String sText(aGlosFL.GetText());
    sText += rShortName;
    aGlosFL.SetText(sText);
    FreeResource();

    aGlosBox.SetDoubleClickHdl(LINK(this, SwSelGlossaryDlg, DoubleClickHdl));
}

/*-----------------25.02.94 20:50-------------------
 dtor ueberladen
--------------------------------------------------*/
SwSelGlossaryDlg::~SwSelGlossaryDlg() {}
/* -----------------25.10.99 08:33-------------------

 --------------------------------------------------*/
IMPL_LINK(SwSelGlossaryDlg, DoubleClickHdl, ListBox*, /*pBox*/)
{
    EndDialog(RET_OK);
    return 0;
}
