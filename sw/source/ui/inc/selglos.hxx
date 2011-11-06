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


#ifndef _SELGLOS_HXX
#define _SELGLOS_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <vcl/fixed.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwSelGlossaryDlg : public ModalDialog
{
protected:
    ListBox         aGlosBox;
    FixedLine       aGlosFL;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    DECL_LINK(DoubleClickHdl, ListBox*);
public:
    SwSelGlossaryDlg(Window * pParent, const String &rShortName);
    ~SwSelGlossaryDlg();
    void InsertGlos(const String &rRegion, const String &rGlosName);    // inline
    sal_uInt16 GetSelectedIdx() const;  // inline
    void SelectEntryPos(sal_uInt16 nIdx);   // inline
};

inline void SwSelGlossaryDlg::InsertGlos(const String &rRegion,
                                const String &rGlosName)
{
    String aTmp( rRegion );
    aTmp += ':';
    aTmp += rGlosName;
    aGlosBox.InsertEntry( aTmp );
}
inline sal_uInt16 SwSelGlossaryDlg::GetSelectedIdx() const
{   return aGlosBox.GetSelectEntryPos(); }
inline void SwSelGlossaryDlg::SelectEntryPos(sal_uInt16 nIdx)
{   aGlosBox.SelectEntryPos(nIdx); }

#endif
