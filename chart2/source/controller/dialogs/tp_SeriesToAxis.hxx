/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_SeriesToAxis.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:49:08 $
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
#ifndef _CHART2_TP_SERIESTOAXIS_HXX
#define _CHART2_TP_SERIESTOAXIS_HXX

// header for SfxTabPage
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
// header for FixedText
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class SchOptionTabPage : public SfxTabPage
{
private:
    FixedLine   aGrpAxis;
    RadioButton aRbtAxis1;
    RadioButton aRbtAxis2;

    FixedLine   aGrpBar;
    FixedText   aFTGap;
    MetricField aMTGap;
    FixedText   aFTOverlap;
    MetricField aMTOverlap;
    CheckBox    aCBConnect;
    CheckBox    aCBAxisSideBySide;

    DECL_LINK(EnableHdl, RadioButton * );

    sal_Int32   m_nAllSeriesAxisIndex;

public:
    SchOptionTabPage(Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchOptionTabPage();

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rInAttrs);
    virtual BOOL FillItemSet(SfxItemSet& rOutAttrs);
    virtual void Reset(const SfxItemSet& rInAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
