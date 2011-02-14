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
#ifndef _CHART2_TP_AXISLABEL_HXX
#define _CHART2_TP_AXISLABEL_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/wrapfield.hxx>
#include <svx/orienthelper.hxx>
#include "TextDirectionListBox.hxx"

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

    FixedText               m_aFtTextDirection;
    TextDirectionListBox    m_aLbTextDirection;

    sal_Bool                m_bShowStaggeringControls;

    sal_Int32           m_nInitialDegrees;
    bool                m_bHasInitialDegrees;       /// false = DialControl in tristate
    bool                m_bInitialStacking;
    bool                m_bHasInitialStacking;      /// false = checkbox in tristate
    bool                m_bComplexCategories;

    DECL_LINK ( ToggleShowLabel, void* );

public:
    SchAxisLabelTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SchAxisLabelTabPage();

    void    Construct();

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );

    void ShowStaggeringControls( sal_Bool bShowStaggeringControls );
    void SetComplexCategories( bool bComplexCategories );
};
//.............................................................................
} //namespace chart
//.............................................................................

#endif

