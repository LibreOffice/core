/*************************************************************************
 *
 *  $RCSfile: pggrid.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2002-04-10 15:04:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    SwPageGridExample   aExampleWN;

    FixedLine       aLayoutFL;

    FixedText       aLinesPerPageFT;
    NumericField    aLinesPerPageNF;
    FixedText       aLinesPerPageHintFT;

    FixedText       aTextSizeFT;
    MetricField     aTextSizeMF;

    FixedText       aCharsPerLineFT;
    NumericField    aCharsPerLineNF;
    FixedText       aCharsPerLineHintFT;

    FixedText       aRubySizeFT;
    MetricField     aRubySizeMF;

    CheckBox        aRubyBelowCB;

    FixedLine       aDisplayFL;

    CheckBox        aDisplayCB;
    CheckBox        aPrintCB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;

    sal_Int32       m_nRubyUserValue;
    sal_Bool        m_bRubyUserValue;
    sal_Int32       m_nPageWidth;
    sal_Bool        m_bVertical;

    SwTextGridPage(Window *pParent, const SfxItemSet &rSet);
    ~SwTextGridPage();

    void UpdatePageWidth(const SfxItemSet& rSet);
    void PutGridItem(SfxItemSet& rSet);

    DECL_LINK(CharSizeChangedHdl, SpinField*);
    DECL_LINK(GridModifyHdl, void*);
public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static USHORT* GetRanges();

    virtual BOOL    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );
};

#endif
