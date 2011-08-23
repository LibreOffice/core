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
#ifndef _FORMEDT_HXX
#define _FORMEDT_HXX

#include <bf_svx/stddlg.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include "toxe.hxx"
namespace binfilter {

class SwWrtShell;
class SwForm;

/*--------------------------------------------------------------------
     Beschreibung:	Markierung fuer Verzeichniseintrag einfuegen
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
    void			UpdatePattern();
    void			Apply();

    ListBox 		aEntryLB;
    OKButton		aOKBtn;
    CancelButton	aCancelBT;
    FixedText		aLevelFT;
    Edit			aEntryED;
    PushButton		aEntryBT;
    PushButton		aTabBT;
    PushButton		aPageBT;
    PushButton		aJumpBT;
    FixedLine       aEntryFL;
    FixedText		aLevelFT2;
    ListBox 		aLevelLB;
    FixedText		aTemplateFT;
    ListBox 		aParaLayLB;
    PushButton		aStdBT;
    PushButton		aAssignBT;
    FixedLine       aFormatFL;

    SwWrtShell	   &rSh;
    SwForm		   *pForm;
    USHORT			nAktLevel;
    BOOL			bLastLinkIsEnd;

public:
    SwIdxFormDlg( Window* pParent, SwWrtShell &rShell, const SwForm& rForm );
    ~SwIdxFormDlg();

    static BOOL		IsNoNum(SwWrtShell& rSh, const String& rName);
    const SwForm&	GetTOXForm();
};

inline const SwForm& SwIdxFormDlg::GetTOXForm()
{
    return *pForm;
}

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
