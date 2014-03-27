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
#ifndef INCLUDED_SW_SOURCE_UI_INC_FORMEDT_HXX
#define INCLUDED_SW_SOURCE_UI_INC_FORMEDT_HXX

#include <svx/stddlg.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include "toxe.hxx"

class SwWrtShell;
class SwForm;

/*--------------------------------------------------------------------
     Description:   insert marker for index entry
 --------------------------------------------------------------------*/
class SwIdxFormDlg : public SvxStandardDialog
{
    DECL_LINK( EntryHdl, Button * );
    DECL_LINK( PageHdl, Button * );
    DECL_LINK( TabHdl, Button * );
    DECL_LINK( JumpHdl, Button * );
    DECL_LINK( StdHdl, Button * );
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( EnableSelectHdl, ListBox * );
    DECL_LINK( DoubleClickHdl, Button * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( AssignHdl, Button * );
    void            UpdatePattern();
    void            Apply() SAL_OVERRIDE;

    ListBox         aEntryLB;
    OKButton        aOKBtn;
    CancelButton    aCancelBT;
    FixedText       aLevelFT;
    Edit            aEntryED;
    PushButton      aEntryBT;
    PushButton      aTabBT;
    PushButton      aPageBT;
    PushButton      aJumpBT;
    FixedLine       aEntryFL;
    FixedText       aLevelFT2;
    ListBox         aLevelLB;
    FixedText       aTemplateFT;
    ListBox         aParaLayLB;
    PushButton      aStdBT;
    PushButton      aAssignBT;
    FixedLine       aFormatFL;

    SwWrtShell     &rSh;
    SwForm         *pForm;
    sal_uInt16          nAktLevel;
    sal_Bool            bLastLinkIsEnd;

public:
    SwIdxFormDlg( Window* pParent, SwWrtShell &rShell, const SwForm& rForm );
    ~SwIdxFormDlg();

    const SwForm&   GetTOXForm();
};

inline const SwForm& SwIdxFormDlg::GetTOXForm()
{
    return *pForm;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
