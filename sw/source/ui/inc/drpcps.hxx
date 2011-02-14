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
#ifndef _DRPCPS_HXX
#define _DRPCPS_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class SwWrtShell;

// class SwDropCapsDlg ******************************************************

class SwDropCapsDlg : public SfxSingleTabDialog
{

public:

     SwDropCapsDlg(Window *pParent, const SfxItemSet &rSet );
    ~SwDropCapsDlg();

};

class SwDropCapsPict;

// class SwDropCapsPage *****************************************************

class SwDropCapsPage : public SfxTabPage
{
friend class SwDropCapsPict;

    CheckBox        aDropCapsBox;
    CheckBox        aWholeWordCB;
    FixedText       aSwitchText;
    NumericField    aDropCapsField;
    FixedText       aLinesText;
    NumericField    aLinesField;
    FixedText       aDistanceText;
    MetricField     aDistanceField;
    FixedLine       aSettingsFL;

    FixedText       aTextText;
    Edit            aTextEdit;
    FixedText       aTemplateText;
    ListBox         aTemplateBox;
    FixedLine       aContentFL;

    SwDropCapsPict  *pPict;

    sal_Bool            bModified;
    sal_Bool            bFormat;
    sal_Bool            bHtmlMode;

    SwWrtShell &rSh;

     SwDropCapsPage(Window *pParent, const SfxItemSet &rSet);
    ~SwDropCapsPage();

    virtual int     DeactivatePage(SfxItemSet *pSet);
    void    FillSet( SfxItemSet &rSet );

    DECL_LINK( ClickHdl, Button * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( WholeWordHdl, CheckBox * );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool FillItemSet(      SfxItemSet &rSet);
    virtual void Reset      (const SfxItemSet &rSet);

    void    SetFormat(sal_Bool bSet){bFormat = bSet;}
};

#endif

