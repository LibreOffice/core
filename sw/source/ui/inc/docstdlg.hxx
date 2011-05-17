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
#ifndef _DOCSTDLG_HXX
#define _DOCSTDLG_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>

#include "docstat.hxx"

/*--------------------------------------------------------------------
    Beschreibung:   DocInfo jetzt als Page
 --------------------------------------------------------------------*/
class SwDocStatPage: public SfxTabPage
{
public:
    SwDocStatPage(Window *pParent, const SfxItemSet &rSet);
    ~SwDocStatPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

protected:
    virtual sal_Bool    FillItemSet(      SfxItemSet &rSet);
    virtual void    Reset      (const SfxItemSet &rSet);

    DECL_LINK( UpdateHdl, PushButton*);

private:
    FixedText       aPageLbl;
    FixedInfo       aPageNo;
    FixedText       aTableLbl;
    FixedInfo       aTableNo;
    FixedText       aGrfLbl;
    FixedInfo       aGrfNo;
    FixedText       aOLELbl;
    FixedInfo       aOLENo;
    FixedText       aParaLbl;
    FixedInfo       aParaNo;
    FixedText       aWordLbl;
    FixedInfo       aWordNo;
    FixedText       aCharLbl;
    FixedInfo       aCharNo;
    FixedText       aCharExclSpacesLbl;
    FixedInfo       aCharExclSpacesNo;
    FixedText       aLineLbl;
    FixedInfo       aLineNo;

    PushButton      aUpdatePB;
    SwDocStat       aDocStat;

    void            Update();

    using Window::SetData;
    void            SetData(const SwDocStat &rStat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
