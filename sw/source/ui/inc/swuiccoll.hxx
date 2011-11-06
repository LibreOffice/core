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


#ifndef _SWUI_CCOLL_HXX
#define _SWUI_CCOLL_HXX

#include "ccoll.hxx"

class SwWrtShell;
class SwFmt;

class SwCondCollPage : public SfxTabPage
{
    FixedLine           aConditionFL;
    CheckBox            aConditionCB;

    FixedText           aContextFT;
    FixedText           aUsedFT;
    SvTabListBox        aTbLinks;

    FixedText           aStyleFT;
    ListBox             aStyleLB;
    ListBox             aFilterLB;

    PushButton          aRemovePB;
    PushButton          aAssignPB;

    String              sNoTmpl;
    ResStringArray      aStrArr;

    SwWrtShell          &rSh;
    const CommandStruct*pCmds;
    SwFmt*              pFmt;

    CollName*           pNms;

    sal_Bool                bNewTemplate;


    SwCondCollPage(Window *pParent, const SfxItemSet &rSet);
    virtual ~SwCondCollPage();

    virtual int     DeactivatePage(SfxItemSet *pSet);

    DECL_LINK( OnOffHdl, CheckBox* );
    DECL_LINK( AssignRemoveHdl, PushButton*);
    DECL_LINK( SelectHdl, ListBox* );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool FillItemSet(      SfxItemSet &rSet);
    virtual void Reset      (const SfxItemSet &rSet);

    void SetCollection( SwFmt* pFormat, sal_Bool bNew );
};



#endif

