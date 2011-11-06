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



#ifndef SC_SHTABDLG_HXX
#define SC_SHTABDLG_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svtools/tooltiplbox.hxx>
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

//------------------------------------------------------------------------

class ScShowTabDlg : public ModalDialog
{
private:
    FixedText                       aFtLbTitle;
    ::svtools::ToolTipMultiListBox  aLb;
    OKButton                        aBtnOk;
    CancelButton                    aBtnCancel;
    HelpButton                      aBtnHelp;

    DECL_LINK( DblClkHdl, void * );

public:
            ScShowTabDlg( Window* pParent );
            ~ScShowTabDlg();

    /** Sets dialog title, fixed text for listbox and help IDs. */
    void    SetDescription(
                const String& rTitle, const String& rFixedText,
                const ::rtl::OString& nDlgHelpId, const rtl::OString& nLbHelpId );

    /** Inserts a string into the ListBox. */
    void    Insert( const String& rString, sal_Bool bSelected );

    sal_uInt16  GetSelectEntryCount() const;
    String  GetSelectEntry(sal_uInt16 nPos) const;
    sal_uInt16  GetSelectEntryPos(sal_uInt16 nPos) const;
};


#endif


