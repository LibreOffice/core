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


#ifndef _SWFLDVAR_HXX
#define _SWFLDVAR_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/group.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
#include "condedit.hxx"
#include "numfmtlb.hxx"

class SwFldVarPage;

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SelectionListBox : public ListBox
{
    SwFldVarPage*   pDlg;
    sal_Bool            bCallAddSelection;

    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SelectionListBox( SwFldVarPage* pDialog, const ResId& rResId );

    //  Selektieren per Ctrl oder Alt erkennen und mit SelectHdl auswerten
    sal_Bool            IsCallAddSelection() const {return bCallAddSelection;}
    void            ResetCallAddSelection() {bCallAddSelection = sal_False;}
};

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldVarPage : public SwFldPage
{
    friend class SelectionListBox;

    FixedText           aTypeFT;
    ListBox             aTypeLB;
    FixedText           aSelectionFT;
    SelectionListBox    aSelectionLB;
    FixedText           aNameFT;
    Edit                aNameED;
    FixedText           aValueFT;
    ConditionEdit       aValueED;
    FixedText           aFormatFT;
    NumFormatListBox    aNumFormatLB;
    ListBox             aFormatLB;
    FixedText           aChapterHeaderFT;
    FixedText           aChapterLevelFT;
    ListBox             aChapterLevelLB;
    CheckBox            aInvisibleCB;
    FixedText           aSeparatorFT;
    Edit                aSeparatorED;
    ToolBox             aNewDelTBX;

    String              sOldValueFT;
    String              sOldNameFT;

    sal_uLong               nOldFormat;
    sal_Bool                bInit;

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pED = 0 );
    DECL_LINK( TBClickHdl, ToolBox *pTB = 0);
    DECL_LINK( ChapterHdl, ListBox *pLB = 0);
    DECL_LINK( SeparatorHdl, Edit *pED = 0 );

    void                UpdateSubType();
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldVarPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldVarPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

