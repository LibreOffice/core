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

#include "ExplicitCategoriesProvider.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "AxisHelper.hxx"
#include "CommonConverters.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "NumberFormatterWrapper.hxx"
#include <unonames.hxx>

#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::std::vector;

ExplicitCategoriesProvider::ExplicitCategoriesProvider( const Reference< chart2::XCoordinateSystem >& xCooSysModel
                                                       , ChartModel& rModel )
    : m_bDirty(true)
    , m_xCooSysModel( xCooSysModel )
    , mrModel(rModel)
    , m_xOriginalCategories()
    , m_bIsExplicitCategoriesInited(false)
    , m_bIsDateAxis(false)
    , m_bIsAutoDate(false)
{
    try
    {
        if( xCooSysModel.is() )
        {
            uno::Reference< XAxis > xAxis( xCooSysModel->getAxisByDimension(0,0) );
            if( xAxis.is() )
            {
                ScaleData aScale( xAxis->getScaleData() );
                m_xOriginalCategories = aScale.Categories;
                m_bIsAutoDate = (aScale.AutoDateAxis && aScale.AxisType==chart2::AxisType::CATEGORY);
                m_bIsDateAxis = (aScale.AxisType == chart2::AxisType::DATE || m_bIsAutoDate);
            }
        }

        if( m_xOriginalCategories.is() )
        {
            uno::Reference< data::XDataProvider > xDataProvider( mrModel.getDataProvider() );

            OUString aCategoriesRange( DataSourceHelper::getRangeFromValues( m_xOriginalCategories ) );
            if( xDataProvider.is() && !aCategoriesRange.isEmpty() )
            {
                const bool bFirstCellAsLabel = false;
                const bool bHasCategories = false;
                const uno::Sequence< sal_Int32 > aSequenceMapping;

                uno::Reference< data::XDataSource > xColumnCategoriesSource( xDataProvider->createDataSource(
                            DataSourceHelper::createArguments( aCategoriesRange, aSequenceMapping, true /*bUseColumns*/
                                , bFirstCellAsLabel, bHasCategories ) ) );

                uno::Reference< data::XDataSource > xRowCategoriesSource( xDataProvider->createDataSource(
                            DataSourceHelper::createArguments( aCategoriesRange, aSequenceMapping, false /*bUseColumns*/
                                , bFirstCellAsLabel, bHasCategories ) ) );

                if( xColumnCategoriesSource.is() &&  xRowCategoriesSource.is() )
                {
                    Sequence< Reference< data::XLabeledDataSequence> > aColumns = xColumnCategoriesSource->getDataSequences();
                    Sequence< Reference< data::XLabeledDataSequence> > aRows = xRowCategoriesSource->getDataSequences();

                    sal_Int32 nColumnCount = aColumns.getLength();
                    sal_Int32 nRowCount = aRows.getLength();
                    if( nColumnCount>1 && nRowCount>1 )
                    {
                        //we have complex categories
                        //->split them in the direction of the first series
                        //detect whether the first series is a row or a column
                        bool bSeriesUsesColumns = true;
                        ::std::vector< Reference< XDataSeries > > aSeries( ChartModelHelper::getDataSeries( mrModel ) );
                        if( !aSeries.empty() )
                        {
                            uno::Reference< data::XDataSource > xSeriesSource( aSeries.front(), uno::UNO_QUERY );
                            OUString aStringDummy;
                            bool bDummy;
                            uno::Sequence< sal_Int32 > aSeqDummy;
                            DataSourceHelper::readArguments( xDataProvider->detectArguments( xSeriesSource),
                                    aStringDummy, aSeqDummy, bSeriesUsesColumns, bDummy, bDummy );
                        }
                        if( bSeriesUsesColumns )
                            m_aSplitCategoriesList=aColumns;
                        else
                            m_aSplitCategoriesList=aRows;
                    }
                }
            }
            if( !m_aSplitCategoriesList.getLength() )
            {
                m_aSplitCategoriesList.realloc(1);
                m_aSplitCategoriesList[0]=m_xOriginalCategories;
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

ExplicitCategoriesProvider::~ExplicitCategoriesProvider()
{
}

Reference< chart2::data::XDataSequence > ExplicitCategoriesProvider::getOriginalCategories()
{
    if( m_xOriginalCategories.is() )
        return m_xOriginalCategories->getValues();
    return nullptr;
}

bool ExplicitCategoriesProvider::hasComplexCategories() const
{
    return m_aSplitCategoriesList.getLength() > 1;
}

sal_Int32 ExplicitCategoriesProvider::getCategoryLevelCount() const
{
    sal_Int32 nCount = m_aSplitCategoriesList.getLength();
    if(!nCount)
        nCount = 1;
    return nCount;
}

std::vector<sal_Int32> lcl_getLimitingBorders( const std::vector< ComplexCategory >& rComplexCategories )
{
    std::vector<sal_Int32> aLimitingBorders;
    std::vector< ComplexCategory >::const_iterator aIt( rComplexCategories.begin() );
    std::vector< ComplexCategory >::const_iterator aEnd( rComplexCategories.end() );
    sal_Int32 nBorderIndex = 0; /*border below the index*/
    for( ; aIt != aEnd; ++aIt )
    {
        ComplexCategory aComplexCategory(*aIt);
        nBorderIndex += aComplexCategory.Count;
        aLimitingBorders.push_back(nBorderIndex);
    }
    return aLimitingBorders;
}

void ExplicitCategoriesProvider::convertCategoryAnysToText( uno::Sequence< OUString >& rOutTexts, const uno::Sequence< uno::Any >& rInAnys, ChartModel& rModel )
{
    sal_Int32 nCount = rInAnys.getLength();
    if(!nCount)
        return;
    rOutTexts.realloc(nCount);
    Reference< util::XNumberFormats > xNumberFormats( rModel.getNumberFormats() );

    sal_Int32 nAxisNumberFormat = 0;
    Reference< XCoordinateSystem > xCooSysModel( ChartModelHelper::getFirstCoordinateSystem( rModel ) );
    if( xCooSysModel.is() )
    {
        Reference< chart2::XAxis > xAxis( xCooSysModel->getAxisByDimension(0,0) );
        nAxisNumberFormat = AxisHelper::getExplicitNumberFormatKeyForAxis(
            xAxis, xCooSysModel, uno::Reference<chart2::XChartDocument>(static_cast< ::cppu::OWeakObject* >(&rModel), uno::UNO_QUERY), false );
    }

    sal_Int32 nLabelColor;
    bool bColorChanged = false;

    NumberFormatterWrapper aNumberFormatterWrapper( rModel.getNumberFormatsSupplier() );

    for(sal_Int32 nN=0;nN<nCount;nN++)
    {
        OUString aText;
        uno::Any aAny = rInAnys[nN];
        if( aAny.hasValue() )
        {
            double fDouble = 0;
            if( aAny>>=fDouble )
            {
                if( !::rtl::math::isNan(fDouble) )
                    aText = aNumberFormatterWrapper.getFormattedString(
                        nAxisNumberFormat, fDouble, nLabelColor, bColorChanged );
            }
            else
            {
                aAny>>=aText;
            }
        }
        rOutTexts[nN] = aText;
    }
}

SplitCategoriesProvider::~SplitCategoriesProvider()
{
}

class SplitCategoriesProvider_ForLabeledDataSequences : public SplitCategoriesProvider
{
public:

    explicit SplitCategoriesProvider_ForLabeledDataSequences(
        const css::uno::Sequence<
            css::uno::Reference< css::chart2::data::XLabeledDataSequence> >& rSplitCategoriesList
        , ChartModel& rModel )
        : m_rSplitCategoriesList( rSplitCategoriesList )
        , mrModel( rModel )
    {}
    virtual ~SplitCategoriesProvider_ForLabeledDataSequences()
    {}

    virtual sal_Int32 getLevelCount() const override;
    virtual uno::Sequence< OUString > getStringsForLevel( sal_Int32 nIndex ) const override;

private:
    const css::uno::Sequence< css::uno::Reference<
        css::chart2::data::XLabeledDataSequence> >& m_rSplitCategoriesList;

    ChartModel& mrModel;
};

sal_Int32 SplitCategoriesProvider_ForLabeledDataSequences::getLevelCount() const
{
    return m_rSplitCategoriesList.getLength();
}
uno::Sequence< OUString > SplitCategoriesProvider_ForLabeledDataSequences::getStringsForLevel( sal_Int32 nLevel ) const
{
    uno::Sequence< OUString > aRet;
    Reference< data::XLabeledDataSequence > xLabeledDataSequence( m_rSplitCategoriesList[nLevel] );
    if( xLabeledDataSequence.is() )
    {
        uno::Reference< data::XDataSequence > xDataSequence( xLabeledDataSequence->getValues() );
        if( xDataSequence.is() )
            ExplicitCategoriesProvider::convertCategoryAnysToText( aRet, xDataSequence->getData(), mrModel );
    }
    return aRet;
}

std::vector< ComplexCategory > lcl_DataSequenceToComplexCategoryVector(
    const uno::Sequence< OUString >& rStrings
    , const std::vector<sal_Int32>& rLimitingBorders, bool bCreateSingleCategories )
{
    std::vector< ComplexCategory > aResult;

    sal_Int32 nMaxCount = rStrings.getLength();
    OUString aPrevious;
    sal_Int32 nCurrentCount=0;
    for( sal_Int32 nN=0; nN<nMaxCount; nN++ )
    {
        const OUString& aCurrent = rStrings[nN];
        if( bCreateSingleCategories || ::std::find( rLimitingBorders.begin(), rLimitingBorders.end(), nN ) != rLimitingBorders.end() )
        {
            aResult.push_back( ComplexCategory(aPrevious,nCurrentCount) );
            nCurrentCount=1;
            aPrevious = aCurrent;
        }
        else
        {
            // Empty value is interpreted as a continuation of the previous
            // category. Note that having the same value as the previous one
            // does not equate to a continuation of the category.

            if (aCurrent.isEmpty())
                ++nCurrentCount;
            else
            {
                aResult.push_back( ComplexCategory(aPrevious,nCurrentCount) );
                nCurrentCount=1;
                aPrevious = aCurrent;
            }
        }
    }
    if( nCurrentCount )
        aResult.push_back( ComplexCategory(aPrevious,nCurrentCount) );

    return aResult;
}

sal_Int32 lcl_getCategoryCount( std::vector< ComplexCategory >& rComplexCategories )
{
    sal_Int32 nCount = 0;
    std::vector< ComplexCategory >::const_iterator aIt( rComplexCategories.begin() );
    std::vector< ComplexCategory >::const_iterator aEnd( rComplexCategories.end() );
    for( ; aIt != aEnd; ++aIt )
        nCount+=aIt->Count;
    return nCount;
}

Sequence< OUString > lcl_getExplicitSimpleCategories(
    const SplitCategoriesProvider& rSplitCategoriesProvider,
    ::std::vector< ::std::vector< ComplexCategory > >& rComplexCats )
{
    Sequence< OUString > aRet;

    rComplexCats.clear();
    sal_Int32 nLCount = rSplitCategoriesProvider.getLevelCount();
    for( sal_Int32 nL = 0; nL < nLCount; nL++ )
    {
        std::vector<sal_Int32> aLimitingBorders;
        if(nL>0)
            aLimitingBorders = lcl_getLimitingBorders( rComplexCats.back() );
        rComplexCats.push_back( lcl_DataSequenceToComplexCategoryVector(
            rSplitCategoriesProvider.getStringsForLevel(nL), aLimitingBorders, nL==(nLCount-1) ) );
    }

    std::vector< std::vector< ComplexCategory > >::iterator aOuterIt( rComplexCats.begin() );
    std::vector< std::vector< ComplexCategory > >::const_iterator aOuterEnd( rComplexCats.end() );

    //ensure that the category count is the same on each level
    sal_Int32 nMaxCategoryCount = 0;
    {
        for( aOuterIt=rComplexCats.begin(); aOuterIt != aOuterEnd; ++aOuterIt )
        {
            sal_Int32 nCurrentCount = lcl_getCategoryCount( *aOuterIt );
            nMaxCategoryCount = std::max( nCurrentCount, nMaxCategoryCount );
        }
        for( aOuterIt=rComplexCats.begin(); aOuterIt != aOuterEnd; ++aOuterIt )
        {
            if ( !aOuterIt->empty() )
            {
                sal_Int32 nCurrentCount = lcl_getCategoryCount( *aOuterIt );
                if( nCurrentCount< nMaxCategoryCount )
                {
                    ComplexCategory& rComplexCategory = aOuterIt->back();
                    rComplexCategory.Count += (nMaxCategoryCount-nCurrentCount);
                }
            }
        }
    }

    //create a list with an element for every index
    std::vector< std::vector< ComplexCategory > > aComplexCatsPerIndex;
    for( aOuterIt=rComplexCats.begin() ; aOuterIt != aOuterEnd; ++aOuterIt )
    {
        std::vector< ComplexCategory > aSingleLevel;
        std::vector< ComplexCategory >::iterator aIt( aOuterIt->begin() );
        std::vector< ComplexCategory >::const_iterator aEnd( aOuterIt->end() );
        for( ; aIt != aEnd; ++aIt )
        {
            ComplexCategory aComplexCategory( *aIt );
            sal_Int32 nCount = aComplexCategory.Count;
            while( nCount-- )
                aSingleLevel.push_back(aComplexCategory);
        }
        aComplexCatsPerIndex.push_back( aSingleLevel );
    }

    if(nMaxCategoryCount)
    {
        aRet.realloc(nMaxCategoryCount);
        aOuterEnd = aComplexCatsPerIndex.end();
        for(sal_Int32 nN=0; nN<nMaxCategoryCount; nN++)
        {
            OUString aText;
            for( aOuterIt=aComplexCatsPerIndex.begin() ; aOuterIt != aOuterEnd; ++aOuterIt )
            {
                if ( static_cast<size_t>(nN) < aOuterIt->size() )
                {
                    OUString aAddText = (*aOuterIt)[nN].Text;
                    if( !aAddText.isEmpty() )
                    {
                        if(!aText.isEmpty())
                            aText += " " + aAddText;
                    }
                }
            }
            aRet[nN]=aText;
        }
    }
    return aRet;
}

Sequence< OUString > ExplicitCategoriesProvider::getExplicitSimpleCategories(
    const SplitCategoriesProvider& rSplitCategoriesProvider )
{
    vector< vector< ComplexCategory > > aComplexCats;
    return lcl_getExplicitSimpleCategories( rSplitCategoriesProvider, aComplexCats );
}

bool lcl_fillDateCategories( const uno::Reference< data::XDataSequence >& xDataSequence, std::vector< double >& rDateCategories, bool bIsAutoDate, ChartModel& rModel )
{
    bool bOnlyDatesFound = true;
    bool bAnyDataFound = false;

    if( xDataSequence.is() )
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        sal_Int32 nCount = aValues.getLength();
        rDateCategories.reserve(nCount);
        Reference< util::XNumberFormats > xNumberFormats( rModel.getNumberFormats() );

        bool bOwnData = false;
        bool bOwnDataAnddAxisHasAnyFormat = false;
        bool bOwnDataAnddAxisHasDateFormat = false;
        Reference< XCoordinateSystem > xCooSysModel( ChartModelHelper::getFirstCoordinateSystem( rModel ) );
        if( xCooSysModel.is() )
        {
            if( rModel.hasInternalDataProvider() )
            {
                bOwnData = true;
                Reference< beans::XPropertySet > xAxisProps( xCooSysModel->getAxisByDimension(0,0), uno::UNO_QUERY );
                sal_Int32 nAxisNumberFormat = 0;
                if (xAxisProps.is() && (xAxisProps->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nAxisNumberFormat))
                {
                    bOwnDataAnddAxisHasAnyFormat = true;
                    bOwnDataAnddAxisHasDateFormat = DiagramHelper::isDateNumberFormat( nAxisNumberFormat, xNumberFormats );
                }
            }
        }

        for(sal_Int32 nN=0;nN<nCount;nN++)
        {
            bool bIsDate = false;
            if( bIsAutoDate )
            {
                if( bOwnData )
                    bIsDate = !bOwnDataAnddAxisHasAnyFormat || bOwnDataAnddAxisHasDateFormat;
                else
                    bIsDate = DiagramHelper::isDateNumberFormat( xDataSequence->getNumberFormatKeyByIndex( nN ), xNumberFormats );
            }
            else
                bIsDate = true;

            bool bContainsEmptyString = false;
            uno::Any aAny = aValues[nN];
            if( aAny.hasValue() )
            {
                OUString aTest;
                double fTest = 0;
                bool bContainsNan = false;
                if( (aAny>>=aTest) && aTest.isEmpty() ) //empty String
                    bContainsEmptyString = true;
                else if( (aAny>>=fTest) &&  ::rtl::math::isNan(fTest) )
                    bContainsNan = true;

                if( !bContainsEmptyString && !bContainsNan )
                    bAnyDataFound = true;
            }
            double aDate( 1.0 );
            if( bIsDate && (aAny >>= aDate) )
                rDateCategories.push_back( aDate );
            else
            {
                if( aAny.hasValue() && !bContainsEmptyString )//empty string does not count as non date value!
                    bOnlyDatesFound=false;
                ::rtl::math::setNan( &aDate );
                rDateCategories.push_back( aDate );
            }
        }
        ::std::sort( rDateCategories.begin(), rDateCategories.end() );
    }

    return bAnyDataFound && bOnlyDatesFound;
}

void ExplicitCategoriesProvider::init()
{
    if( m_bDirty )
    {
        m_aComplexCats.clear();//not one per index
        m_aDateCategories.clear();

        if( m_xOriginalCategories.is() )
        {
            if( !hasComplexCategories() )
            {
                if(m_bIsDateAxis)
                {
                    if( ChartTypeHelper::isSupportingDateAxis( AxisHelper::getChartTypeByIndex( m_xCooSysModel, 0 ), 2, 0 ) )
                        m_bIsDateAxis = lcl_fillDateCategories( m_xOriginalCategories->getValues(), m_aDateCategories, m_bIsAutoDate, mrModel );
                    else
                        m_bIsDateAxis = false;
                }
            }
            else
            {
                m_bIsDateAxis = false;
            }
        }
        else
            m_bIsDateAxis=false;
        m_bDirty = false;
    }
}

Sequence< OUString > ExplicitCategoriesProvider::getSimpleCategories()
{
    if( !m_bIsExplicitCategoriesInited )
    {
        init();
        m_aExplicitCategories.realloc(0);
        if( m_xOriginalCategories.is() )
        {
            if( !hasComplexCategories() )
            {
                uno::Reference< data::XDataSequence > xDataSequence( m_xOriginalCategories->getValues() );
                if( xDataSequence.is() )
                    ExplicitCategoriesProvider::convertCategoryAnysToText( m_aExplicitCategories, xDataSequence->getData(), mrModel );
            }
            else
            {
                m_aExplicitCategories = lcl_getExplicitSimpleCategories(
                    SplitCategoriesProvider_ForLabeledDataSequences( m_aSplitCategoriesList, mrModel ), m_aComplexCats );
            }
        }
        if(!m_aExplicitCategories.getLength())
            m_aExplicitCategories = DiagramHelper::generateAutomaticCategoriesFromCooSys( m_xCooSysModel );
        m_bIsExplicitCategoriesInited = true;
    }
    return m_aExplicitCategories;
}

const std::vector<ComplexCategory>* ExplicitCategoriesProvider::getCategoriesByLevel( sal_Int32 nLevel )
{
    init();
    sal_Int32 nMaxIndex = m_aComplexCats.size()-1;
    if (nLevel >= 0 && nLevel <= nMaxIndex)
        return &m_aComplexCats[nMaxIndex-nLevel];
    return nullptr;
}

OUString ExplicitCategoriesProvider::getCategoryByIndex(
          const Reference< XCoordinateSystem >& xCooSysModel
        , ChartModel& rModel
        , sal_Int32 nIndex )
{
    if( xCooSysModel.is())
    {
        ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSysModel, rModel );
        Sequence< OUString > aCategories( aExplicitCategoriesProvider.getSimpleCategories());
        if( nIndex < aCategories.getLength())
            return aCategories[ nIndex ];
    }
    return OUString();
}

bool ExplicitCategoriesProvider::isDateAxis()
{
    init();
    return m_bIsDateAxis;
}

const std::vector< double >&  ExplicitCategoriesProvider::getDateCategories()
{
    init();
    return m_aDateCategories;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
