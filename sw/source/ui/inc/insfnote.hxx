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
