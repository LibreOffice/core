/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_AxisLabel.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:13:54 $
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
#ifndef _CHART2_TP_AXISLABEL_HXX
#define _CHART2_TP_AXISLABEL_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
// header for CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef SVX_DIALCONTROL_HXX
#include <svx/dialcontrol.hxx>
#endif
#ifndef SVX_WRAPFIELD_HXX
#include <svx/wrapfield.hxx>
#endif
#ifndef SVX_ORIENTHELPER_HXX
#include <svx/orienthelper.hxx>
#endif

//#include "attrib.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class SchAxisLabelTabPage : public SfxTabPage
{
private:
    CheckBox            aCbShowDescription;

    FixedLine           aFlOrder;
    RadioButton         aRbSideBySide;
    RadioButton         aRbUpDown;
    RadioButton         aRbDownUp;
    RadioButton         aRbAuto;

    FixedLine           aFlSeparator;
    FixedLine           aFlTextFlow;
    CheckBox            aCbTextOverlap;
    CheckBox            aCbTextBreak;

    FixedLine           aFlOrient;
    svx::DialControl    aCtrlDial;
    FixedText           aFtRotate;
    svx::WrapField      aNfRotate;
    TriStateBox         aCbStacked;
    svx::OrientationHelper aOrientHlp;

    BOOL                m_bShowStaggeringControls;
////    BOOL                bAllowTextOverlap;

    sal_Int32           m_nInitialDegrees;
    bool                m_bHasInitialDegrees;       /// false = DialControl in tristate
    bool                m_bInitialStacking;
    bool                m_bHasInitialStacking;      /// false = checkbox in tristate

    DECL_LINK ( ToggleShowLabel, void* );

public:
    SchAxisLabelTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SchAxisLabelTabPage();

    void    Construct();

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual BOOL FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );

    void ShowStaggeringControls( BOOL bShowStaggeringControls );
////    void AllowTextOverlap( BOOL bOverLap )      { bAllowTextOverlap = bOverLap; }
};
//.............................................................................
} //namespace chart
//.............................................................................

#endif

