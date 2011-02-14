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
#ifndef _SWLINENUM_HXX
#define _SWLINENUM_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <numberingtypelistbox.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class Window;
class SfxItemSet;
class SwView;
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung: SingleTabDialog
 --------------------------------------------------------------------*/

class SwLineNumberingDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;

    DECL_LINK( OKHdl, Button * );

public:
    inline SwWrtShell* GetWrtShell() const { return pSh; }

     SwLineNumberingDlg(SwView *pVw);
    ~SwLineNumberingDlg();
};

/*--------------------------------------------------------------------
   Beschreibung: TabPage
 --------------------------------------------------------------------*/

class SwLineNumberingPage : public SfxTabPage
{
    CheckBox        aNumberingOnCB;
    FixedText       aCharStyleFT;
    ListBox         aCharStyleLB;
    FixedText       aFormatFT;
    SwNumberingTypeListBox   aFormatLB;
    FixedText       aPosFT;
    ListBox         aPosLB;
    FixedText       aOffsetFT;
    MetricField     aOffsetMF;
    FixedText       aNumIntervalFT;
    NumericField    aNumIntervalNF;
    FixedText       aNumRowsFT;
    FixedLine        aDisplayFL;
    FixedText       aDivisorFT;
    Edit            aDivisorED;
    FixedText       aDivIntervalFT;
    NumericField    aDivIntervalNF;
    FixedText       aDivRowsFT;
    FixedLine        aDivisorFL;
    CheckBox        aCountEmptyLinesCB;
    CheckBox        aCountFrameLinesCB;
    CheckBox        aRestartEachPageCB;
    FixedLine        aCountFL;

    SwWrtShell*     pSh;

    SwLineNumberingPage( Window* pParent, const SfxItemSet& rSet );
    ~SwLineNumberingPage();

    DECL_LINK( LineOnOffHdl, CheckBox *pCB = 0 );
    DECL_LINK( ModifyHdl, Edit *pED = 0 );

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif


