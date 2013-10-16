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
#ifndef _OFA_OPTFLTR_HXX
#define _OFA_OPTFLTR_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svtools/simptabl.hxx>

class OfaMSFilterTabPage : public SfxTabPage
{
    CheckBox*       aWBasicCodeCB;
    CheckBox*       aWBasicWbctblCB;
    CheckBox*       aWBasicStgCB;
    CheckBox*       aEBasicCodeCB;
    CheckBox*       aEBasicExectblCB;
    CheckBox*       aEBasicStgCB;
    CheckBox*       aPBasicCodeCB;
    CheckBox*       aPBasicStgCB;

    OfaMSFilterTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage();

    DECL_LINK(LoadWordBasicCheckHdl_Impl, void *);
    DECL_LINK(LoadExcelBasicCheckHdl_Impl, void *);
public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class OfaMSFilterTabPage2 : public SfxTabPage
{
    class MSFltrSimpleTable : public SvSimpleTable
    {
        using SvTreeListBox::GetCheckButtonState;
        using SvTreeListBox::SetCheckButtonState;
        using SvSimpleTable::SetTabs;

        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked);
        SvButtonState   GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol ) const;
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );
    protected:
        virtual void    SetTabs();
        virtual void    HBarClick();
        virtual void    KeyInput( const KeyEvent& rKEvt );

    public:
        MSFltrSimpleTable(SvSimpleTableContainer& rParent, WinBits nBits = 0)
            : SvSimpleTable(rParent, nBits)
        {
        }
    };

    SvSimpleTableContainer* m_pCheckLBContainer;
    MSFltrSimpleTable* m_pCheckLB;
    OUString sHeader1, sHeader2;
    OUString sChgToFromMath,
           sChgToFromWriter,
           sChgToFromCalc,
           sChgToFromImpress,
           sChgToFromSmartArt;
    SvLBoxButtonData*   pCheckButtonData;

    OfaMSFilterTabPage2( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage2();

    void                InsertEntry( const OUString& _rTxt, sal_IntPtr _nType );
    void                InsertEntry( const OUString& _rTxt, sal_IntPtr _nType,
                                     sal_Bool loadEnabled, sal_Bool saveEnabled );
    SvTreeListEntry*    GetEntry4Type( sal_IntPtr _nType ) const;

public:

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif //


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
