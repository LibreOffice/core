/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_MGETEMPL_HXX
#define _SFX_MGETEMPL_HXX

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#include <sfx2/tabdlg.hxx>

#include <svtools/svmedit2.hxx>
#include <svtools/svmedit.hxx>

/* erwartet:
    SID_TEMPLATE_NAME   :   In: StringItem, Name der Vorlage
    SID_TEMPLATE_FAMILY :   In: Familie der Vorlage
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

    FixedInfo aDescFt;
    MultiLineEdit aDescED;
    FixedLine aDescGb;

    SfxStyleSheetBase *pStyle;
    SfxStyleFamilies *pFamilies;
    const SfxStyleFamilyItem *pItem;
    String aBuf;
    sal_Bool bModified;

        // initiale Daten des Styles
    String aName;
    String aFollow;
    String aParent;
    sal_uInt16 nFlags;

private:
friend class SfxStyleDialog;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( GetFocusHdl, Edit * );
    DECL_LINK( LoseFocusHdl, Edit * );

    void    UpdateName_Impl(ListBox *, const String &rNew);
    void    SetDescriptionText_Impl();
//#endif

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

