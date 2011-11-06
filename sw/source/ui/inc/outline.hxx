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


#ifndef _OUTLINE_HXX
#define _OUTLINE_HXX

#include <sfx2/tabdlg.hxx>


#include <vcl/menu.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svtools/stdctrl.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#include "swtypes.hxx"      //fuer MAXLEVEL
#include <numprevw.hxx>
#include <numberingtypelistbox.hxx>

class SwWrtShell;
class SwNumRule;
class SwChapterNumRules;

/* -----------------07.07.98 13:38-------------------
 *
 * --------------------------------------------------*/
class SwOutlineTabDialog : public SfxTabDialog
{
    static     sal_uInt16    nNumLevel;

    String              aNullStr;
    String              aCollNames[MAXLEVEL];
    PopupMenu           aFormMenu;

    SwWrtShell&         rWrtSh;
    SwNumRule*          pNumRule;
    SwChapterNumRules*  pChapterNumRules;

    sal_Bool                bModified : 1;

    protected:
    DECL_LINK( CancelHdl, Button * );
    DECL_LINK( FormHdl, Button * );
    DECL_LINK( MenuSelectHdl, Menu * );

        virtual void    PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage);
        virtual short   Ok();

    public:
        SwOutlineTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
        ~SwOutlineTabDialog();

    SwNumRule*          GetNumRule() {return pNumRule;}
    sal_uInt16              GetLevel(const String &rFmtName) const;
    String*             GetCollNames() {return aCollNames;}

    static sal_uInt16       GetActNumLevel() {return nNumLevel;}
    static void         SetActNumLevel(sal_uInt16 nSet) {nNumLevel = nSet;}
};
/* -----------------07.07.98 13:47-------------------
 *
 * --------------------------------------------------*/
class SwOutlineSettingsTabPage : public SfxTabPage
{
    FixedLine       aLevelFL;
    ListBox         aLevelLB;

    FixedLine        aNumberFL;
    FixedText       aCollLbl;
    ListBox         aCollBox;
    FixedText       aNumberLbl;
    SwNumberingTypeListBox  aNumberBox;
    FixedText       aCharFmtFT;
    ListBox         aCharFmtLB;
    FixedText       aAllLevelFT;
    NumericField    aAllLevelNF;
    FixedText       aDelim;
    FixedText       aPrefixFT;
    Edit            aPrefixED;
    FixedText       aSuffixFT;
    Edit            aSuffixED;
    FixedText       aStartLbl;
    NumericField    aStartEdit;
    NumberingPreview aPreviewWIN;

    String              aNoFmtName;
    String              aSaveCollNames[MAXLEVEL];
    SwWrtShell*         pSh;
    SwNumRule*          pNumRule;
    String*             pCollNames;
    sal_uInt16              nActLevel;

    DECL_LINK( LevelHdl, ListBox * );
    DECL_LINK( ToggleComplete, NumericField * );
    DECL_LINK( CollSelect, ListBox * );
    DECL_LINK( CollSelectGetFocus, ListBox * );
    DECL_LINK( NumberSelect, SwNumberingTypeListBox * );
    DECL_LINK( DelimModify, Edit * );
    DECL_LINK( StartModified, NumericField * );
    DECL_LINK( CharFmtHdl, ListBox * );

    void    Update();

    void    SetModified(){aPreviewWIN.Invalidate();}
    void    CheckForStartValue_Impl(sal_uInt16 nNumberingType);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwOutlineSettingsTabPage(Window* pParent, const SfxItemSet& rSet);
    ~SwOutlineSettingsTabPage();

    void SetWrtShell(SwWrtShell* pShell);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};
#endif
