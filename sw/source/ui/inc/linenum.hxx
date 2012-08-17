/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <numberingtypelistbox.hxx>

class Window;
class SwView;
class SwWrtShell;

class SwLineNumberingDlg : public SfxModalDialog
{
private:
    SwWrtShell* pSh;

    CheckBox        aNumberingOnCB;
    FixedLine       aDisplayFL;
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
    FixedLine       aDivisorFL;
    FixedText       aDivisorFT;
    Edit            aDivisorED;
    FixedText       aDivIntervalFT;
    NumericField    aDivIntervalNF;
    FixedText       aDivRowsFT;
    FixedLine       aCountFL;
    CheckBox        aCountEmptyLinesCB;
    CheckBox        aCountFrameLinesCB;
    CheckBox        aRestartEachPageCB;

    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;

    DECL_LINK(OKHdl, void *);
    DECL_LINK(LineOnOffHdl, void * = 0);
    DECL_LINK(ModifyHdl, void * = 0);

public:
    SwWrtShell* GetWrtShell() const { return pSh; }

     SwLineNumberingDlg(SwView *pVw);
    ~SwLineNumberingDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
