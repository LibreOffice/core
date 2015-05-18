/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CHART2_SOURCE_INC_EXPLICITCATEGORIESPROVIDER_HXX
#define INCLUDED_CHART2_SOURCE_INC_EXPLICITCATEGORIESPROVIDER_HXX

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include "charttoolsdllapi.hxx"
#include "ChartModel.hxx"

#include <vector>

namespace chart
{

struct OOO_DLLPUBLIC_CHARTTOOLS ComplexCategory
{
    OUString Text;
    sal_Int32 Count;

    ComplexCategory( const OUString& rText, sal_Int32 nCount ) : Text( rText ), Count (nCount)
    {}
};

class OOO_DLLPUBLIC_CHARTTOOLS SplitCategoriesProvider
{
public:
    virtual ~SplitCategoriesProvider();

    virtual sal_Int32 getLevelCount() const = 0;
    virtual ::com::sun::star::uno::Sequence< OUString > getStringsForLevel( sal_Int32 nIndex ) const = 0;
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
                       , ChartModel& rChartModel
                       );
    virtual ~ExplicitCategoriesProvider();

    void init();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > getOriginalCategories();

    ::com::sun::star::uno::Sequence< OUString > getSimpleCategories();
    const std::vector<ComplexCategory>* getCategoriesByLevel( sal_Int32 nLevel );

    static OUString getCategoryByIndex(
          const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel
        , ChartModel& rModel
        , sal_Int32 nIndex );

    static ::com::sun::star::uno::Sequence< OUString > getExplicitSimpleCategories(
            const SplitCategoriesProvider& rSplitCategoriesProvider );

    static void convertCategoryAnysToText( ::com::sun::star::uno::Sequence< OUString >& rOutTexts
        , const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rInAnys
        , ChartModel& rModel );

    bool hasComplexCategories() const;
    sal_Int32 getCategoryLevelCount() const;

    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> >& getSplitCategoriesList() { return m_aSplitCategoriesList;}

    bool isDateAxis();
    const std::vector< DatePlusIndex >&  getDateCategories();

private: //member
    bool volatile m_bDirty;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::chart2::XCoordinateSystem >   m_xCooSysModel;
    ChartModel& mrModel;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> m_xOriginalCategories;

    bool m_bIsExplicitCategoriesInited;
    ::com::sun::star::uno::Sequence< OUString >  m_aExplicitCategories;
    ::std::vector< ::std::vector< ComplexCategory > >   m_aComplexCats;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> > m_aSplitCategoriesList;

    bool m_bIsDateAxis;
    bool m_bIsAutoDate;
    std::vector< DatePlusIndex >  m_aDateCategories;
};

}

// INCLUDED_CHART2_SOURCE_INC_EXPLICITCATEGORIESPROVIDER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
