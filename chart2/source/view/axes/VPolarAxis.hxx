/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarAxis.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:40:23 $
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
#ifndef _CHART2_VPOLARAXIS_HXX
#define _CHART2_VPOLARAXIS_HXX

#include "TickmarkHelper.hxx"
#include "VMeterBase.hxx"
#include "VAxisProperties.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class NumberFormatterWrapper;
class PolarPlottingPositionHelper;

class VPolarAxis : public VMeterBase
{
public:
    VPolarAxis( const AxisProperties& rAxisProperties
           , NumberFormatterWrapper* pNumberFormatterWrapper
           , sal_Int32 nDimensionCount );
    virtual ~VPolarAxis();

    virtual void SAL_CALL createShapes();

    void setIncrements( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::chart2::ExplicitIncrementData >& rIncrements );

private: //methods
    void create2DAngleAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos );
    void create2DRadiusAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos );

private: //member
    AxisProperties               m_aAxisProperties;
    NumberFormatterWrapper*      m_pNumberFormatterWrapper;
    PolarPlottingPositionHelper* m_pPosHelper;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::chart2::ExplicitIncrementData >   m_aIncrements;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
