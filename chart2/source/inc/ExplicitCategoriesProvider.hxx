/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ExplicitCategoriesProvider.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX
#define CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX

#include "ServiceMacros.hxx"
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS ExplicitCategoriesProvider :
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::chart2::data::XTextualDataSequence
        >
{
public:
    ExplicitCategoriesProvider( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel );
    SAL_DLLPRIVATE virtual ~ExplicitCategoriesProvider();

    //XTextualDataSequence
    SAL_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getTextualData()
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
