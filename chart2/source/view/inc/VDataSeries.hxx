/*************************************************************************
 *
 *  $RCSfile: VDataSeries.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-12 18:08:05 $
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
#ifndef _CHART2_VIEW_DATASERIES_HXX
#define _CHART2_VIEW_DATASERIES_HXX

#include "ShapeAppearance.hxx"
#include "PropertyMapper.hxx"
/*
#ifndef _B3D_HMATRIX_HXX
#include <goodies/hmatrix.hxx>
#endif
*/
#include <vector>
//for auto_ptr
#include <memory>

/*
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
*/
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATACAPTIONSTYLE_HPP_
#include <drafts/com/sun/star/chart2/DataCaptionStyle.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <drafts/com/sun/star/chart2/XDataSequence.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <drafts/com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <drafts/com/sun/star/chart2/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif


//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VDataSeries
{
public:
    VDataSeries();
    VDataSeries( ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XDataSeries >
               , const ShapeAppearance& rDefaultAppearence );
    virtual ~VDataSeries();

    sal_Int32   getTotalPointCount() const;
    double      getX( sal_Int32 index ) const;
    double      getY( sal_Int32 index ) const;

    rtl::OUString       getCategoryString( sal_Int32 index ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getPropertiesOfPoint( sal_Int32 index ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getPropertiesOfSeries() const;

    ShapeAppearance     getAppearanceOfPoint( sal_Int32 index ) const;
    SymbolType          getSymbolTypeOfPoint( sal_Int32 index ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > m_xShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > m_xLabelsShape;

    //this is only temporarily here for area chart:
    ::com::sun::star::drawing::PolyPolygonShape3D       m_aPolyPolygonShape3D;
    double m_fMinX;
    double m_fMaxX;
    //

    rtl::OUString       getCID() const;
    rtl::OUString       getPointCID_Stub() const;
    rtl::OUString       getLabelsCID() const;
    rtl::OUString       getLabelCID_Stub() const;

    ::drafts::com::sun::star::chart2::DataCaptionStyle*
                        getDataCaptionStyle( sal_Int32 index ) const;
    bool    getTextLabelMultiPropertyLists( sal_Int32 index, tNameSequence*& pPropNames, tAnySequence*& pPropValues ) const;

private: //methods
    bool    isAttributedDataPoint( sal_Int32 index ) const;

private: //member

    ShapeAppearance     m_aAppearanceOfSeries;

    ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDataSeries >       m_xDataSeries;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDataSequence > >   m_aDataSequences;

    //all points given by the model data (here are not only the visible points meant)
    sal_Int32                                                     m_nPointCount;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSequence >         m_xData_XValues;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSequence >         m_xData_YValues;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSequence >         m_xData_ZValues;

    ::com::sun::star::uno::Sequence< double >                     m_YValues_Double;

    ::com::sun::star::uno::Sequence< sal_Int32 >                  m_aAttributedDataPointIndexList;

    rtl::OUString           m_aIdentifier;//model identifier of this series
    rtl::OUString           m_aCID;
    rtl::OUString           m_aPointCID_Stub;
    rtl::OUString           m_aLabelCID_Stub;

    //some cached values for data labels as they are very expensive
    mutable ::std::auto_ptr< ::drafts::com::sun::star::chart2::DataCaptionStyle >
                                                    m_apCaption_Series;
    mutable ::std::auto_ptr< tNameSequence >        m_apLabelPropNames_Series;
    mutable ::std::auto_ptr< tAnySequence >         m_apLabelPropValues_Series;

    mutable ::std::auto_ptr< ::drafts::com::sun::star::chart2::DataCaptionStyle >
                                                    m_apCaption_AttributedPoint;
    mutable ::std::auto_ptr< tNameSequence >        m_apLabelPropNames_AttributedPoint;
    mutable ::std::auto_ptr< tAnySequence >         m_apLabelPropValues_AttributedPoint;
    mutable sal_Int32   m_nCurrentAttributedPoint;
    //
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
