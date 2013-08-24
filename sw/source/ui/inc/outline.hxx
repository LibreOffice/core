/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _OUTLINE_HXX
#define _OUTLINE_HXX

#include <sfx2/tabdlg.hxx>


#include <vcl/menu.hxx>

#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/field.hxx>

#include "swtypes.hxx"      //for MAXLEVEL
#include <numprevw.hxx>
#include <numberingtypelistbox.hxx>
#include "rtl/ustring.hxx"

class SwWrtShell;
class SwNumRule;
class SwChapterNumRules;

class SwOutlineTabDialog : public SfxTabDialog
{
    static     sal_uInt16    nNumLevel;

    sal_uInt16 m_nNumPosId;
    sal_uInt16 m_nOutlineId;

    OUString            aCollNames[MAXLEVEL];

    SwWrtShell&         rWrtSh;
    SwNumRule*          pNumRule;
    SwChapterNumRules*  pChapterNumRules;

    sal_Bool                bModified : 1;

    protected:
    DECL_LINK(CancelHdl, void *);
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
    OUString*           GetCollNames() {return aCollNames;}

    static sal_uInt16       GetActNumLevel() {return nNumLevel;}
    static void         SetActNumLevel(sal_uInt16 nSet) {nNumLevel = nSet;}
};

class SwOutlineSettingsTabPage : public SfxTabPage
{
    ListBox*        m_pLevelLB;

    ListBox*        m_pCollBox;
    SwNumberingTypeListBox* m_pNumberBox;
    ListBox*        m_pCharFmtLB;
    FixedText*      m_pAllLevelFT;
    NumericField*   m_pAllLevelNF;
    Edit*           m_pPrefixED;
    Edit*           m_pSuffixED;
    NumericField*   m_pStartEdit;
    NumberingPreview* m_pPreviewWIN;

    String              aNoFmtName;
    String              aSaveCollNames[MAXLEVEL];
    SwWrtShell*         pSh;
    SwNumRule*          pNumRule;
    OUString*           pCollNames;
    sal_uInt16              nActLevel;

    DECL_LINK( LevelHdl, ListBox * );
    DECL_LINK( ToggleComplete, NumericField * );
    DECL_LINK( CollSelect, ListBox * );
    DECL_LINK(CollSelectGetFocus, void *);
    DECL_LINK( NumberSelect, SwNumberingTypeListBox * );
    DECL_LINK(DelimModify, void *);
    DECL_LINK( StartModified, NumericField * );
    DECL_LINK(CharFmtHdl, void *);

    void    Update();

    void    SetModified(){m_pPreviewWIN->Invalidate();}
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
