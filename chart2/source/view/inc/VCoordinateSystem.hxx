/*************************************************************************
 *
 *  $RCSfile: VCoordinateSystem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:33 $
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
#ifndef _CHART2_VCOORDINATESYSTEM_HXX
#define _CHART2_VCOORDINATESYSTEM_HXX

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <drafts/com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRID_HPP_
#include <drafts/com/sun/star/chart2/XGrid.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
// #ifndef _DRAFTS_COM_SUN_STAR_CHART2_XBOUNDEDCOORDINATESYSTEMCONTAINER_HPP_
// #include <drafts/com/sun/star/chart2/XBoundedCoordinateSystemContainer.hpp>
// #endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class MinimumAndMaximumSupplier;

class VCoordinateSystem
{
public:
    VCoordinateSystem( const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem >& xCooSys );
    virtual ~VCoordinateSystem();

    void addAxis( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis >& xAxis );
    void setOrigin( double* fCoordinateOrigin );
    void addGrid( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XGrid >& xGrid );

    void doAutoScale( MinimumAndMaximumSupplier* pMinMaxSupplier );

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis > getAxisByDimension( sal_Int32 nDim  ) const;

    const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::chart2::ExplicitScaleData >& getExplicitScales() const {return m_aExplicitScales;}
    const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::chart2::ExplicitIncrementData >& getExplicitIncrements() const {return m_aExplicitIncrements;}
    double getOriginByDimension( sal_Int32 nDim ) const { return m_fCoordinateOrigin[nDim]; }

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem >
        getModel() const;

    void createGridShapes( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>& xShapeFactory
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >& xTarget
            , const ::com::sun::star::drawing::HomogenMatrix& rHM_SceneToScreen
        );

private: //methods
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XGrid > >&
        getGridListByDimension( sal_Int32 nDim );

private: //member

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > m_xCooSysModel;

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis > m_xAxis0;
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis > m_xAxis1;
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis > m_xAxis2;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XGrid > > m_xGridList0;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XGrid > > m_xGridList1;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XGrid > > m_xGridList2;

    double m_fCoordinateOrigin[3];

    ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::chart2::ExplicitScaleData >     m_aExplicitScales;
    ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::chart2::ExplicitIncrementData > m_aExplicitIncrements;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
