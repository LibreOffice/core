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


#ifndef _SWFLDFUNC_HXX
#define _SWFLDFUNC_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/group.hxx>
#include <vcl/edit.hxx>

#include "condedit.hxx"
#include "fldpage.hxx"
#include <actctrl.hxx>
/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldFuncPage : public SwFldPage
{
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    FixedText       aSelectionFT;
    ListBox         aSelectionLB;
    FixedText       aFormatFT;
    ListBox         aFormatLB;
    FixedText       aNameFT;
    ConditionEdit   aNameED;
    FixedText       aValueFT;
    Edit            aValueED;
    FixedText       aCond1FT;
    ConditionEdit   aCond1ED;
    FixedText       aCond2FT;
    ConditionEdit   aCond2ED;
    PushButton      aMacroBT;

    //controls of "Input list"
    FixedText       aListItemFT;
    ReturnActionEdit aListItemED;
    PushButton      aListAddPB;
    FixedText       aListItemsFT;
    ListBox         aListItemsLB;
    PushButton      aListRemovePB;
    PushButton      aListUpPB;
    PushButton      aListDownPB;
    FixedText       aListNameFT;
    Edit            aListNameED;

    String          sOldValueFT;
    String          sOldNameFT;

    sal_uLong           nOldFormat;
    bool            bDropDownLBChanged;

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SelectHdl, ListBox* pLB = 0 );
    DECL_LINK( InsertMacroHdl, ListBox* pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pEd = 0 );
    DECL_LINK( ListModifyHdl, Control*);
    DECL_LINK( ListEnableHdl, void*);

    // Macro ausw„hlen
    DECL_LINK( MacroHdl, Button * );

    void                UpdateSubType();
    String              TurnMacroString(const String &rMacro);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldFuncPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldFuncPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

