/*************************************************************************
 *
 *  $RCSfile: VAxisProperties.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2004-01-17 13:09:55 $
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
#ifndef _CHART2_VAXIS_PROPERTIES_HXX
#define _CHART2_VAXIS_PROPERTIES_HXX

#include "VLineProperties.hxx"
#include "PlottingPositionHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_NUMBERFORMAT_HPP_
#include <drafts/com/sun/star/chart2/NumberFormat.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <drafts/com/sun/star/chart2/XAxis.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct TickmarkProperties
{
    sal_Int32 RelativePos;//Position in screen values relative to the axis where the tickmark line starts
    sal_Int32 Length;//Length of the tickmark line in screen values

    VLineProperties aLineProperties;
};

//These properties describe how a couple of labels are arranged one to another.
//The couple can contain all labels for all tickmark depth or just the labels for one single depth or
//the labels from an coherent range of tick depths (e.g. the major and first minor tickmarks should be handled together).
//... only allow side by side for different tick depth
enum AxisLabelStaggering
{
      SIDE_BY_SIDE
    , STAGGER_EVEN
    , STAGGER_ODD
    , STAGGER_AUTO
};

struct AxisLabelProperties
{
    AxisLabelProperties();

    drafts::com::sun::star::chart2::NumberFormat        aNumberFormat;

    sal_Bool             bDisplayLabels;

    AxisLabelStaggering  eStaggering;

    sal_Bool             bLineBreakAllowed;
    sal_Bool             bOverlapAllowed;

    sal_Bool             bStackCharacters;
    double               fRotationAngleDegree;

    sal_Int32   nRhythm; //show only each nth label with n==nRhythm
    bool        bRhythmIsFix; //states wether the given rythm is fix or may be changed

    //methods:
    void init( const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::chart2::XAxis >&  xAxisModel );

    sal_Bool            getIsStaggered() const;
};

struct AxisProperties
{
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis > m_xAxisModel;

    bool        m_bIsYAxis;
    bool        m_bIsLeftOrBottomAxis;
    double*     m_pfMainLinePositionAtOtherAxis;
    double*     m_pfExrtaLinePositionAtOtherAxis;

    /*
    sal_Int32   m_nOrthogonalAxisScreenPosition;//gives a position in the direction orthogonal to the axis direction
    //e.g. for an y-axis this value describes a x position on the screen
    sal_Int32   m_nOrthogonalAxisExtraLineScreenPosition;//gives the screen value for an additional line ( e.g. a line at 0 ) at the other axis
    */

    enum RelativeLabelPosition { NONE, LEFTORBOTTOM_OF_DIAGRAM, RIGHTORTOP_OF_DIAGRAM,
                        LEFTORBOTTOM_OF_AXIS, RIGHTORTOP_OF_AXIS };

    RelativeLabelPosition m_eRelativeLabelPosition;
    ::com::sun::star::awt::Size m_aReferenceSize;


    /*
    0: no tickmarks         1: inner tickmarks
    2: outer tickmarks      3: inner and outer tickmarks
    */
    sal_Int32                           m_nMajorTickmarks;
    sal_Int32                           m_nMinorTickmarks;
    ::std::vector<TickmarkProperties>   m_aTickmarkPropertiesList;

    VLineProperties                      m_aLineProperties;

    //methods:
    AxisProperties();
    AxisProperties( const AxisProperties& rAxisProperties );
    ~AxisProperties();
    void init();//init from model data (m_xAxisModel)

private:
    TickmarkProperties  makeTickmarkProperties( sal_Int32 nDepth ) const;
    sal_Int32           calcTickLengthForDepth( sal_Int32 nDepth, sal_Int32 nTickmarkStyle ) const;
    VLineProperties      makeLinePropertiesForDepth( sal_Int32 nDepth ) const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
