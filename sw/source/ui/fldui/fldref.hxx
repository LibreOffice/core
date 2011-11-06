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


#ifndef _SWFLDREF_HXX
#define _SWFLDREF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/group.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
// --> OD 2007-11-15 #i83479#
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>
#include <FldRefTreeListBox.hxx>
class SwTxtNode;
// <--

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldRefPage : public SwFldPage
{
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    FixedText       aSelectionFT;
    ListBox         aSelectionLB;
    // --> OD 2007-11-21 #i83479#
    SwFldRefTreeListBox aSelectionToolTipLB;
    // <--
    FixedText       aFormatFT;
    ListBox         aFormatLB;
    FixedText       aNameFT;
    Edit            aNameED;
    FixedText       aValueFT;
    Edit            aValueED;
    const String    sBookmarkTxt;
    const String    sFootnoteTxt;
    const String    sEndnoteTxt;
    // --> OD 2007-11-09 #i83479#
    const String    sHeadingTxt;
    const String    sNumItemTxt;

    IDocumentOutlineNodes::tSortedOutlineNodeList maOutlineNodes;
    IDocumentListItems::tSortedNodeNumList maNumItems;

    // selected text node in the listbox for headings and numbered items
    // in order to restore selection after update of selection listbox
    const SwTxtNode* mpSavedSelectedTxtNode;
    // fallback, if previously selected text node doesn't exist anymore
    sal_uInt16 mnSavedSelectedPos;
    // <--

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pEd = 0 );

    void                UpdateSubType();
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

    // --> OD 2007-12-05 #i83479#
    void SaveSelectedTxtNode();
    const SwTxtNode* GetSavedSelectedTxtNode() const;
    sal_uInt16 GetSavedSelectedPos() const;
    // <--

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldRefPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldRefPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

