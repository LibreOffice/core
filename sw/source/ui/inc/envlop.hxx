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
#ifndef _ENVLOP_HXX
#define _ENVLOP_HXX

#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#include <sfx2/tabdlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif

#include "envimg.hxx"

#define GetFldVal(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SetFldVal(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwEnvPage;
class SwEnvFmtPage;
class SwWrtShell;
class Printer;

// class SwEnvPreview ---------------------------------------------------------

class SwEnvPreview : public Window
{
    void Paint(const Rectangle&);

public:

     SwEnvPreview(SfxTabPage* pParent, const ResId& rResID);
    ~SwEnvPreview();

protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

// class SwEnvDlg -----------------------------------------------------------

class SwEnvDlg : public SfxTabDialog
{
friend class SwEnvPage;
friend class SwEnvFmtPage;
friend class SwEnvPrtPage;
friend class SwEnvPreview;

    String          sInsert;
    String          sChange;
    SwEnvItem       aEnvItem;
    SwWrtShell      *pSh;
    Printer         *pPrinter;
    SfxItemSet      *pAddresseeSet;
    SfxItemSet      *pSenderSet;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual short   Ok();

public:
     SwEnvDlg(Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, sal_Bool bInsert);
    ~SwEnvDlg();
};

// class SwEnvPage ----------------------------------------------------------

class SwEnvPage : public SfxTabPage
{
    FixedText     aAddrText;
    MultiLineEdit aAddrEdit;
    FixedText     aDatabaseFT;
    ListBox       aDatabaseLB;
    FixedText     aTableFT;
    ListBox       aTableLB;
    ImageButton   aInsertBT;
    FixedText     aDBFieldFT;
    ListBox       aDBFieldLB;
    CheckBox      aSenderBox;
    MultiLineEdit aSenderEdit;
    SwEnvPreview  aPreview;

    SwWrtShell*   pSh;
    String        sActDBName;

     SwEnvPage(Window* pParent, const SfxItemSet& rSet);
    ~SwEnvPage();

    DECL_LINK( DatabaseHdl, ListBox * );
    DECL_LINK( FieldHdl, Button * );
    DECL_LINK( SenderHdl, Button * );

    void InitDatabaseBox();

    using Window::GetParent;
    SwEnvDlg* GetParent() {return (SwEnvDlg*) SfxTabPage::GetParent()->GetParent();}

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwEnvItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif


