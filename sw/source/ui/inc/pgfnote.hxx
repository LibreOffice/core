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
#ifndef _PGFNOTE_HXX
#define _PGFNOTE_HXX

#include <sfx2/tabdlg.hxx>

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svtools/ctrlbox.hxx>

/*--------------------------------------------------------------------
    Beschreibung:   Fussnoteneinstellungs-TabPage
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

    RadioButton     aMaxHeightPageBtn;
    RadioButton     aMaxHeightBtn;
    MetricField     aMaxHeightEdit;
    FixedText       aDistLbl;
    MetricField     aDistEdit;
    FixedLine       aPosHeader;

    FixedText       aLinePosLbl;
    ListBox         aLinePosBox;
    FixedText       aLineTypeLbl;
    LineListBox     aLineTypeBox;
    FixedText       aLineWidthLbl;
    MetricField     aLineWidthEdit;
    FixedText       aLineDistLbl;
    MetricField     aLineDistEdit;
    FixedLine       aLineHeader;

    DECL_LINK( HeightPage, Button * );
    DECL_LINK( HeightMetric, Button * );
    DECL_LINK( HeightModify, MetricField * );

    long            lMaxHeight;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

};

#endif
