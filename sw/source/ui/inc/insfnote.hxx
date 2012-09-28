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
#ifndef _INSFNOTE_HXX
#define _INSFNOTE_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

class SwWrtShell;

class SwInsFootNoteDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;

    // everything for the character(s)
    String          aFontName;
    CharSet         eCharSet;
    sal_Bool        bExtCharAvailable;
    sal_Bool        bEdit;
    FixedLine       aNumberFL;
    RadioButton     aNumberAutoBtn;
    RadioButton     aNumberCharBtn;
    Edit            aNumberCharEdit;
    PushButton      aNumberExtChar;

    // everything for the selection footnote/endnote
    FixedLine        aTypeFL;
    RadioButton     aFtnBtn;
    RadioButton     aEndNoteBtn;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    ImageButton     aPrevBT;
    ImageButton     aNextBT;

    DECL_LINK(NumberCharHdl, void *);
    DECL_LINK( NumberEditHdl, void * );
    DECL_LINK(NumberAutoBtnHdl, void *);
    DECL_LINK(NumberExtCharHdl, void *);
    DECL_LINK( NextPrevHdl, Button * );

    virtual void    Apply();

    void            Init();

public:
    SwInsFootNoteDlg(Window * pParent, SwWrtShell &rSh, sal_Bool bEd = sal_False);
    ~SwInsFootNoteDlg();

    CharSet         GetCharSet() { return eCharSet; }
    sal_Bool            IsExtCharAvailable() { return bExtCharAvailable; }
    String          GetFontName() { return aFontName; }
    sal_Bool            IsEndNote() { return aEndNoteBtn.IsChecked(); }
    String          GetStr()
                    {
                        if ( aNumberCharBtn.IsChecked() )
                            return aNumberCharEdit.GetText();
                        else
                            return String();
                    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
