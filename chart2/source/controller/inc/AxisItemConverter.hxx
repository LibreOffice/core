/*************************************************************************
 *
 *  $RCSfile: AxisItemConverter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:30:08 $
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
#ifndef CHART_AXISITEMCONVERTER_HXX
#define CHART_AXISITEMCONVERTER_HXX

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <drafts/com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif

#include "ItemConverter.hxx"
#include "chartview/NumberFormatterWrapper.hxx"

#include <vector>

class SdrModel;

namespace chart
{
namespace wrapper
{

class AxisItemConverter : public ::comphelper::ItemConverter
{
public:
    AxisItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        NumberFormatterWrapper * pNumFormatter,
        ::drafts::com::sun::star::chart2::ExplicitScaleData * pScale = NULL,
        ::drafts::com::sun::star::chart2::ExplicitIncrementData * pIncrement = NULL,
        double * pExplicitOrigin = NULL );
    virtual ~AxisItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

protected:
    virtual const USHORT * GetWhichPairs() const;
    virtual bool GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const;

    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet ) const
        throw( ::com::sun::star::uno::Exception );

private:
    ::std::vector< ItemConverter * >               m_aConverters;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XAxis >  m_xAxis;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem >
                                                   m_xCoordinateSystem;
    sal_Int32                                      m_nDimension;
    NumberFormatterWrapper *                       m_pNumberFormatterWrapper;

    ::drafts::com::sun::star::chart2::ExplicitScaleData *       m_pExplicitScale;
    ::drafts::com::sun::star::chart2::ExplicitIncrementData *   m_pExplicitIncrement;
    double *                                                m_pExplicitOrigin;
};

} //  namespace wrapper
} //  namespace chart

// CHART_AXISITEMCONVERTER_HXX
#endif
