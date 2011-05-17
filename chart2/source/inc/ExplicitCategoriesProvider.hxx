/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX
#define CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX

#include "ServiceMacros.hxx"
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "charttoolsdllapi.hxx"

#include <vector>

namespace chart
{

struct OOO_DLLPUBLIC_CHARTTOOLS ComplexCategory
{
    rtl::OUString Text;
    sal_Int32 Count;

    ComplexCategory( const rtl::OUString& rText, sal_Int32 nCount ) : Text( rText ), Count (nCount)
    {}
};

class OOO_DLLPUBLIC_CHARTTOOLS SplitCategoriesProvider
{
public:
    virtual ~SplitCategoriesProvider();

    virtual sal_Int32 getLevelCount() const = 0;
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > getStringsForLevel( sal_Int32 nIndex ) const = 0;
};

struct DatePlusIndex
{
    DatePlusIndex()
        : fValue(1.0)
        , nIndex( -1 )
    {
    }

    DatePlusIndex( const double& _fValue, sal_Int32 _nIndex )
        : fValue(_fValue)
        , nIndex( _nIndex )
    {
    }

    double fValue;
    sal_Int32 nIndex;
};

class OOO_DLLPUBLIC_CHARTTOOLS ExplicitCategoriesProvider
{
public:
    ExplicitCategoriesProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel
                       , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel
                       );
    virtual ~ExplicitCategoriesProvider();

    void init();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > getOriginalCategories();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > getSimpleCategories();
    ::std::vector< ComplexCategory > getCategoriesByLevel( sal_Int32 nLevel );

    static ::rtl::OUString getCategoryByIndex(
          const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xChartModel
        , sal_Int32 nIndex );

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getExplicitSimpleCategories(
            const SplitCategoriesProvider& rSplitCategoriesProvider );

    static void convertCategoryAnysToText( ::com::sun::star::uno::Sequence< rtl::OUString >& rOutTexts
        , const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rInAnys
        , ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xChartModel );

    bool hasComplexCategories() const;
    sal_Int32 getCategoryLevelCount() const;

    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> >& getSplitCategoriesList();

    bool isDateAxis();
    const std::vector< DatePlusIndex >&  getDateCategories();

private: //member
    bool volatile m_bDirty;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::chart2::XCoordinateSystem >   m_xCooSysModel;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XModel >               m_xChartModel;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> m_xOriginalCategories;

    bool m_bIsExplicitCategoriesInited;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aExplicitCategories;
    ::std::vector< ::std::vector< ComplexCategory > >   m_aComplexCats;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> > m_aSplitCategoriesList;

    bool m_bIsDateAxis;
    bool m_bIsAutoDate;
    std::vector< DatePlusIndex >  m_aDateCategories;
};

} //  namespace chart

// CHART2_VIEW_EXPLICITCATEGORIESPROVIDER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
