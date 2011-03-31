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
#ifndef _DOCSTDLG_HXX
#define _DOCSTDLG_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>

#include "docstat.hxx"

/*--------------------------------------------------------------------
    Description:   DocInfo now as page
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

    DECL_LINK(UpdateHdl, void *);

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
