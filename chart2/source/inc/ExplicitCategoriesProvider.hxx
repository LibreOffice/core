/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExplicitCategoriesProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:16:22 $
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
#ifndef CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX
#define CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX

#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XTEXTUALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

namespace chart
{

class ExplicitCategoriesProvider : public ::cppu::WeakImplHelper1<
        ::com::sun::star::chart2::data::XTextualDataSequence
        >
{
public:
    ExplicitCategoriesProvider( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel );
    virtual ~ExplicitCategoriesProvider();

    //XTextualDataSequence
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getTextualData()
        throw (::com::sun::star::uno::RuntimeException);

    static ::rtl::OUString getCategoryByIndex(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel,
        sal_Int32 nIndex );

private: //member
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aExplicitCategories;
    bool volatile m_bDirty;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::chart2::XCoordinateSystem > m_xCooSysModel;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> m_xCategories;
};

} //  namespace chart

// CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX
#endif
