/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pggrid.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:33:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _PGGRID_HXX
#define _PGGRID_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _COLEX_HXX
#include <colex.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _CTRLBOX_HXX
#include <svtools/ctrlbox.hxx>
#endif
/*--------------------------------------------------------------------
    Description:   TabPage Format/(Styles/)Page/Text grid
 --------------------------------------------------------------------*/

class SwTextGridPage: public SfxTabPage
{
    FixedLine       aGridTypeFL;
    RadioButton     aNoGridRB;
    RadioButton     aLinesGridRB;
    RadioButton     aCharsGridRB;
    CheckBox        aSnapToCharsCB;

    SwPageGridExample   aExampleWN;

    FixedLine       aLayoutFL;

    FixedText       aLinesPerPageFT;
    NumericField    aLinesPerPageNF;

    FixedText       aTextSizeFT;
    MetricField     aTextSizeMF;

    FixedText       aCharsPerLineFT;
    NumericField    aCharsPerLineNF;

    FixedText       aCharWidthFT;
    MetricField     aCharWidthMF;

    FixedText       aRubySizeFT;
    MetricField     aRubySizeMF;

    CheckBox        aRubyBelowCB;

    FixedLine       aDisplayFL;

    CheckBox        aDisplayCB;
    CheckBox        aPrintCB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;

    Window*         aControls[18];

    sal_Int32       m_nRubyUserValue;
    sal_Bool        m_bRubyUserValue;
    Size            m_aPageSize;
    sal_Bool        m_bVertical;
    sal_Bool        m_bSquaredMode;

    SwTextGridPage(Window *pParent, const SfxItemSet &rSet);
    ~SwTextGridPage();

    void UpdatePageSize(const SfxItemSet& rSet);
    void PutGridItem(SfxItemSet& rSet);

    DECL_LINK(GridTypeHdl, RadioButton*);
    DECL_LINK(CharorLineChangedHdl, SpinField*);
    DECL_LINK(TextSizeChangedHdl, SpinField*);
    DECL_LINK(GridModifyHdl, void*);
    DECL_LINK(DisplayGridHdl, CheckBox*);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static USHORT* GetRanges();

    virtual BOOL    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );
};

#endif
