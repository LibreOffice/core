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
#ifndef _SFX_MGETEMPL_HXX
#define _SFX_MGETEMPL_HXX

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svmedit.hxx>

#include <sfx2/tabdlg.hxx>

#include <svtools/svmedit2.hxx>

/* expected:
    SID_TEMPLATE_NAME   :   In: StringItem, Name of Template
    SID_TEMPLATE_FAMILY :   In: Family of Template
*/

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxStyleSheetBase;

#ifdef _SFXDLG_HXX
#define FixedInfo FixedText
#endif

class SfxManageStyleSheetPage : public SfxTabPage
{
    FixedText aNameFt;
    Edit aNameEd;
    ExtMultiLineEdit aNameMLE;

    CheckBox    aAutoCB;

    FixedText aFollowFt;
    ListBox aFollowLb;

    FixedText aBaseFt;
    ListBox aBaseLb;

    FixedText aFilterFt;
    ListBox aFilterLb;

    FixedLine aDescGb;
    FixedInfo aDescFt;

    SfxStyleSheetBase *pStyle;
    SfxStyleFamilies *pFamilies;
    const SfxStyleFamilyItem *pItem;
    String aBuf;
    sal_Bool bModified;

    // initial data for the style
    String aName;
    String aFollow;
    String aParent;
    sal_uInt16 nFlags;

private:
friend class SfxStyleDialog;

    DECL_LINK( GetFocusHdl, Edit * );
    DECL_LINK( LoseFocusHdl, Edit * );

    void    UpdateName_Impl(ListBox *, const String &rNew);
    void    SetDescriptionText_Impl();

    SfxManageStyleSheetPage(Window *pParent, const SfxItemSet &rAttrSet );
    ~SfxManageStyleSheetPage();

    static SfxTabPage*  Create(Window *pParent, const SfxItemSet &rAttrSet );

protected:
    virtual sal_Bool        FillItemSet(SfxItemSet &);
    virtual void        Reset(const SfxItemSet &);

    using TabPage::ActivatePage;
        virtual void        ActivatePage(const SfxItemSet &);
        using TabPage::DeactivatePage;
    virtual int     DeactivatePage(SfxItemSet * = 0);
};

#ifdef FixedInfo
#undef FixedInfo
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
