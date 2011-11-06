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



#ifndef SC_LBSELDLG_HXX
#define SC_LBSELDLG_HXX


#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif


#include <vcl/dialog.hxx>

//------------------------------------------------------------------------

class ScSelEntryDlg : public ModalDialog
{
private:
    FixedLine       aFlLbTitle;
    ListBox         aLb;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    DECL_LINK( DblClkHdl, void * );

public:
            ScSelEntryDlg(      Window* pParent,
                                sal_uInt16  nResId,
                          const String& aTitle,
                          const String& aLbTitle,
                                List&   aEntryList );
            ~ScSelEntryDlg();

    String GetSelectEntry() const;
//UNUSED2008-05  sal_uInt16 GetSelectEntryPos() const;
};


#endif // SC_LBSELDLG_HXX


