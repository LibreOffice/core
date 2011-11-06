/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    FixedText       aLineDistLbl;
    MetricField     aLineDistEdit;

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
