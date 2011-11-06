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


#ifndef _SWCHARDLG_HXX
#define _SWCHARDLG_HXX
#include <sfx2/tabdlg.hxx>

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class SwView;
class SvxMacroItem;

/*--------------------------------------------------------------------
   Beschreibung:    Der Tabdialog Traeger der TabPages
 --------------------------------------------------------------------*/

class SwCharDlg: public SfxTabDialog
{
    SwView&   rView;
    sal_Bool      bIsDrwTxtMode;

public:
    SwCharDlg(Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
              const String* pFmtStr = 0, sal_Bool bIsDrwTxtDlg = sal_False);

    ~SwCharDlg();

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
};

/*-----------------14.08.96 11.03-------------------
    Beschreibung: Tabpage fuer URL-Attribut
--------------------------------------------------*/

class SwCharURLPage : public SfxTabPage
{
    FixedLine           aURLFL;

    FixedText           aURLFT;
    Edit                aURLED;
    FixedText           aTextFT;
    Edit                aTextED;
    FixedText           aNameFT;
    Edit                aNameED;
    FixedText           aTargetFrmFT;
    ComboBox            aTargetFrmLB;
    PushButton          aURLPB;
    PushButton          aEventPB;
    FixedLine           aStyleFL;
    FixedText           aVisitedFT;
    ListBox             aVisitedLB;
    FixedText           aNotVisitedFT;
    ListBox             aNotVisitedLB;

    SvxMacroItem*       pINetItem;
    sal_Bool                bModified;

    DECL_LINK( InsertFileHdl, PushButton * );
    DECL_LINK( EventHdl, PushButton * );

public:
                        SwCharURLPage( Window* pParent,
                                           const SfxItemSet& rSet );

                        ~SwCharURLPage();
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

