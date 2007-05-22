/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarRadiusAxis.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:13:58 $
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
#ifndef _CHART2_VPOLARRADIUSAXIS_HXX
#define _CHART2_VPOLARRADIUSAXIS_HXX

#include "VPolarAxis.hxx"

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VCartesianAxis;

class VPolarRadiusAxis : public VPolarAxis
{
public:
    VPolarRadiusAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionCount );
    virtual ~VPolarRadiusAxis();

    virtual void SAL_CALL initPlotter(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xLogicTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xFinalTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& xFactory
        , const rtl::OUString& rCID
                ) throw (::com::sun::star::uno::RuntimeException );

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );

    virtual void SAL_CALL setScales(
          const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& rScales
            , sal_Bool bSwapXAndYAxis )
                throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setExplicitScaleAndIncrement(
            const ::com::sun::star::chart2::ExplicitScaleData& rScale
          , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL initAxisLabelProperties(
                    const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels );

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();

    virtual void SAL_CALL createMaximumLabels();
    virtual void SAL_CALL createLabels();
    virtual void SAL_CALL updatePositions();

    virtual void SAL_CALL createShapes();

protected: //methods
    virtual bool prepareShapeCreation();

private: //member
    std::auto_ptr<VCartesianAxis>  m_apAxisWithLabels;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
