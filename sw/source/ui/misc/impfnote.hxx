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
#ifndef _IMPFNOTE_HXX
#define _IMPFNOTE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <numberingtypelistbox.hxx>

class SwWrtShell;

class SwEndNoteOptionPage : public SfxTabPage
{
    FixedText       aNumTypeFT;
    SwNumberingTypeListBox   aNumViewBox;
    FixedText       aOffsetLbl;
    NumericField    aOffsetFld;
    FixedText       aNumCountFT;
    ListBox         aNumCountBox;
    FixedText       aPrefixFT;
    Edit            aPrefixED;
    FixedText       aSuffixFT;
    Edit            aSuffixED;
    FixedText       aPosFT;
    RadioButton     aPosPageBox;
    RadioButton     aPosChapterBox;
    FixedLine       aNumFL;

    FixedText       aParaTemplLbl;
    ListBox         aParaTemplBox;
    FixedText       aPageTemplLbl;
    ListBox         aPageTemplBox;
    FixedLine        aTemplFL;

    FixedText       aFtnCharAnchorTemplLbl;
    ListBox         aFtnCharAnchorTemplBox;
    FixedText       aFtnCharTextTemplLbl;
    ListBox         aFtnCharTextTemplBox;
    FixedLine        aCharTemplFL;

    FixedText aContLbl;
    Edit aContEdit;
    FixedText aContFromLbl;
    Edit aContFromEdit;
    FixedLine aContFL;

    String aNumDoc;
    String aNumPage;
    String aNumChapter;
    SwWrtShell *pSh;
    BOOL    bPosDoc;
    BOOL    bEndNote;

    inline void SelectNumbering(int eNum);
    int GetNumbering() const;

    DECL_LINK( PosPageHdl, Button * );
    DECL_LINK( PosChapterHdl, Button * );
    DECL_LINK( NumCountHdl, ListBox * );


public:
    SwEndNoteOptionPage( Window *pParent, BOOL bEndNote,
                         const SfxItemSet &rSet );
    ~SwEndNoteOptionPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    virtual BOOL FillItemSet(SfxItemSet &rSet);
    virtual void Reset( const SfxItemSet& );

    void SetShell( SwWrtShell &rShell );
};

class SwFootNoteOptionPage : public SwEndNoteOptionPage
{
    SwFootNoteOptionPage( Window *pParent, const SfxItemSet &rSet );
    ~SwFootNoteOptionPage();

public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
