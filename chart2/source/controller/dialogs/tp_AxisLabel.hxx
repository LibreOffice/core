/*************************************************************************
 *
 *  $RCSfile: tp_AxisLabel.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dr $ $Date: 2004-08-04 14:33:25 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
// header for SvxWinOrientation
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
#if 0 // DR: TODO new control types
    svx::OrientationHelper  aWOAngle;
#endif
    FixedLine           aFlTextFlow;
    CheckBox            aCbTextOverlap;
    CheckBox            aCbTextBreak;

    FixedLine           aFlOrder;
    RadioButton         aRbSideBySide;
    RadioButton         aRbUpDown;
    RadioButton         aRbDownUp;
    RadioButton         aRbAuto;

    FixedLine           aFlSeparator;

    BOOL                m_bShowStaggeringControls;
////    BOOL                bAllowTextOverlap;

    short               m_nInitialDegrees;
    BOOL                m_bInitialStacking;

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

