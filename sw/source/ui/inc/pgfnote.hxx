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
#ifndef _PGFNOTE_HXX
#define _PGFNOTE_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>

#include <vcl/group.hxx>
#include <svtools/ctrlbox.hxx>

/*--------------------------------------------------------------------
    Description:    footnote settings TabPage
 --------------------------------------------------------------------*/
class SwFootNotePage: public SfxTabPage
{
public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
    virtual void Reset(const SfxItemSet &rSet);

private:
    SwFootNotePage(Window *pParent, const SfxItemSet &rSet);
    ~SwFootNotePage();

    FixedLine       aPosHeader;
    RadioButton     aMaxHeightPageBtn;
    RadioButton     aMaxHeightBtn;
    MetricField     aMaxHeightEdit;
    FixedText       aDistLbl;
    MetricField     aDistEdit;

    FixedLine       aLineHeader;
    FixedText       aLinePosLbl;
    ListBox         aLinePosBox;
    FixedText       aLineTypeLbl;
    LineListBox     aLineTypeBox;
    FixedText       aLineWidthLbl;
    MetricField     aLineWidthEdit;
    FixedText       aLineColorLbl;
    ColorListBox    aLineColorBox;
    FixedText       aLineLengthLbl;
    MetricField     aLineLengthEdit;
    FixedText       aLineDistLbl;
    MetricField     aLineDistEdit;

    DECL_LINK(HeightPage, void *);
    DECL_LINK(HeightMetric, void *);
    DECL_LINK(HeightModify, void *);
    DECL_LINK( LineWidthChanged_Impl, void * );
    DECL_LINK( LineColorSelected_Impl, void * );

    long            lMaxHeight;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
