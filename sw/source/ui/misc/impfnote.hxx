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
    SwNumberingTypeListBox* m_pNumViewBox;
    FixedText* m_pOffsetLbl;
    NumericField* m_pOffsetFld;
    ListBox* m_pNumCountBox;
    Edit* m_pPrefixED;
    Edit* m_pSuffixED;
    FixedText* m_pPosFT;
    RadioButton* m_pPosPageBox;
    RadioButton* m_pPosChapterBox;

    VclContainer *m_pStylesContainer;

    ListBox* m_pParaTemplBox;
    FixedText* m_pPageTemplLbl;
    ListBox* m_pPageTemplBox;

    ListBox* m_pFtnCharAnchorTemplBox;
    ListBox* m_pFtnCharTextTemplBox;

    Edit* m_pContEdit;
    Edit* m_pContFromEdit;

    String aNumDoc;
    String aNumPage;
    String aNumChapter;
    SwWrtShell *pSh;
    sal_Bool    bPosDoc;
    sal_Bool    bEndNote;

    inline void SelectNumbering(int eNum);
    int GetNumbering() const;

    DECL_LINK(PosPageHdl, void *);
    DECL_LINK(PosChapterHdl, void *);
    DECL_LINK(NumCountHdl, void *);


public:
    SwEndNoteOptionPage( Window *pParent, sal_Bool bEndNote,
                         const SfxItemSet &rSet );
    ~SwEndNoteOptionPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
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
