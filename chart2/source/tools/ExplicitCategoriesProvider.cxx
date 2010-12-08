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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ExplicitCategoriesProvider.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "AxisHelper.hxx"
#include "CommonConverters.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"

#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::std::vector;


ExplicitCategoriesProvider::ExplicitCategoriesProvider( const Reference< chart2::XCoordinateSystem >& xCooSysModel
                                                       , const uno::Reference< frame::XModel >& xChartModel )
    : m_bDirty(true)
    , m_xCooSysModel( xCooSysModel )
    , m_xChartModel( xChartModel )
    , m_xOriginalCategories()
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
            Reference< chart2::XChartDocument > xChartDoc( xChartModel, uno::UNO_QUERY );
            if( xChartDoc.is() )
            {
                uno::Reference< data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );

                if( xDataProvider.is() )
                {
                    OUString aCatgoriesRange( DataSourceHelper::getRangeFromValues( m_xOriginalCategories ) );
                    const bool bFirstCellAsLabel = false;
                    const bool bHasCategories = false;
                    const uno::Sequence< sal_Int32 > aSequenceMapping;

                    uno::Reference< data::XDataSource > xColumnCategoriesSource( xDataProvider->createDataSource(
                         DataSourceHelper::createArguments( aCatgoriesRange, aSequenceMapping, true /*bUseColumns*/
                            , bFirstCellAsLabel, bHasCategories ) ) );

                    uno::Reference< data::XDataSource > xRowCategoriesSource( xDataProvider->createDataSource(
                         DataSourceHelper::createArguments( aCatgoriesRange, aSequenceMapping, false /*bUseColumns*/
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
                            ::std::vector< Reference< XDataSeries > > aSeries( ChartModelHelper::getDataSeries( xChartModel ) );
                            if( !aSeries.empty() )
                            {
                                uno::Reference< data::XDataSource > xSeriesSource( aSeries.front(), uno::UNO_QUERY );
                                ::rtl::OUString aStringDummy;
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
    return 0;
}

const Sequence< Reference< data::XLabeledDataSequence> >& ExplicitCategoriesProvider::getSplitCategoriesList()
{
    return m_aSplitCategoriesList;
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

uno::Sequence< rtl::OUString > lcl_DataToStringSequence( const uno::Reference< data::XDataSequence >& xDataSequence )
{
    uno::Sequence< rtl::OUString > aStrings;

    OSL_ASSERT( xDataSequence.is());
    if( !xDataSequence.is() )
        return aStrings;


    uno::Reference< data::XTextualDataSequence > xTextualDataSequence( xDataSequence, uno::UNO_QUERY );
    if( xTextualDataSequence.is() )
    {
        aStrings = xTextualDataSequence->getTextualData();
    }
    else
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        aStrings.realloc(aValues.getLength());

        for(sal_Int32 nN=aValues.getLength();nN--;)
            aValues[nN] >>= aStrings[nN];
    }

    return aStrings;
}

SplitCategoriesProvider::~SplitCategoriesProvider()
{
}

class SplitCategoriesProvider_ForLabeledDataSequences : public SplitCategoriesProvider
{
public:

    explicit SplitCategoriesProvider_ForLabeledDataSequences( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> >& rSplitCategoriesList )
        : m_rSplitCategoriesList( rSplitCategoriesList )
    {}
    virtual ~SplitCategoriesProvider_ForLabeledDataSequences()
    {}

    virtual sal_Int32 getLevelCount() const;
    virtual uno::Sequence< rtl::OUString > getStringsForLevel( sal_Int32 nIndex ) const;

private:
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence> >& m_rSplitCategoriesList;
};

sal_Int32 SplitCategoriesProvider_ForLabeledDataSequences::getLevelCount() const
{
    return m_rSplitCategoriesList.getLength();
}
uno::Sequence< rtl::OUString > SplitCategoriesProvider_ForLabeledDataSequences::getStringsForLevel( sal_Int32 nLevel ) const
{
    uno::Sequence< rtl::OUString > aRet;
    Reference< data::XLabeledDataSequence > xLabeledDataSequence( m_rSplitCategoriesList[nLevel] );
    if( xLabeledDataSequence.is() )
        aRet = lcl_DataToStringSequence( xLabeledDataSequence->getValues() );
    return aRet;
}

std::vector< ComplexCategory > lcl_DataSequenceToComplexCategoryVector(
    const uno::Sequence< rtl::OUString >& rStrings
    , const std::vector<sal_Int32>& rLimitingBorders, bool bCreateSingleCategories )
{
    std::vector< ComplexCategory > aResult;

    sal_Int32 nMaxCount = rStrings.getLength();
    OUString aPrevious;
    sal_Int32 nCurrentCount=0;
    for( sal_Int32 nN=0; nN<nMaxCount; nN++ )
    {
        OUString aCurrent = rStrings[nN];
        if( bCreateSingleCategories || ::std::find( rLimitingBorders.begin(), rLimitingBorders.end(), nN ) != rLimitingBorders.end() )
        {
            aResult.push_back( ComplexCategory(aPrevious,nCurrentCount) );
            nCurrentCount=1;
            aPrevious = aCurrent;
        }
        else
        {
            if( aCurrent.getLength() && aPrevious != aCurrent )
            {
                aResult.push_back( ComplexCategory(aPrevious,nCurrentCount) );
                nCurrentCount=1;
                aPrevious = aCurrent;
            }
            else
                nCurrentCount++;
        }
    }
    if( nCurrentCount )
        aResult.push_back( ComplexCategory(aPrevious,nCurrentCount) );

    return aResult;
}

sal_Int32 lcl_getCategoryCount( std::vector< ComplexCategory >& rComplexCategories )
{
    sal_Int32 nCount = 0;
    std::vector< ComplexCategory >::iterator aIt( rComplexCategories.begin() );
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
            sal_Int32 nCurrentCount = lcl_getCategoryCount( *aOuterIt );
            if( nCurrentCount< nMaxCategoryCount )
            {
                ComplexCategory& rComplexCategory = aOuterIt->back();
                rComplexCategory.Count += (nMaxCategoryCount-nCurrentCount);
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
        OUString aSpace(C2U(" "));
        for(sal_Int32 nN=0; nN<nMaxCategoryCount; nN++)
        {
            OUString aText;
            for( aOuterIt=aComplexCatsPerIndex.begin() ; aOuterIt != aOuterEnd; ++aOuterIt )
            {
                OUString aAddText = (*aOuterIt)[nN].Text;
                if( aAddText.getLength() )
                {
                    if(aText.getLength())
                        aText += aSpace;
                    aText += aAddText;
                }
            }
            aRet[nN]=aText;
        }
    }
    return aRet;
}

//static
Sequence< OUString > ExplicitCategoriesProvider::getExplicitSimpleCategories(
    const SplitCategoriesProvider& rSplitCategoriesProvider )
{
    vector< vector< ComplexCategory > > aComplexCats;
    return lcl_getExplicitSimpleCategories( rSplitCategoriesProvider, aComplexCats );
}

struct DatePlusIndexComparator
{
    inline bool operator() ( const DatePlusIndex& aFirst,
                             const DatePlusIndex& aSecond )
    {
        return ( aFirst.fValue < aSecond.fValue );
    }
};

bool lcl_isDateFormat( sal_Int32 nNumberFormat, const Reference< util::XNumberFormats >& xNumberFormats )
{
    bool bIsDate = false;
    if( !xNumberFormats.is() )
        return bIsDate;

    Reference< beans::XPropertySet > xKeyProps = xNumberFormats->getByKey( nNumberFormat );
    if( xKeyProps.is() )
    {
        sal_Int32 nType = util::NumberFormat::DATETIME;
        xKeyProps->getPropertyValue( C2U("Type") ) >>= nType;
        bIsDate = (nType&util::NumberFormat::DATE || nType&util::NumberFormat::DATETIME);
    }
    return bIsDate;
}

bool lcl_fillDateCategories( const uno::Reference< data::XDataSequence >& xDataSequence, std::vector< DatePlusIndex >& rDateCategories, bool bIsAutoDate, Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier )
{
    bool bOnlyDatesFound = true;

    if( xDataSequence.is() )
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        sal_Int32 nCount = aValues.getLength();
        rDateCategories.reserve(nCount);
        Reference< util::XNumberFormats > xNumberFormats;
        if( xNumberFormatsSupplier.is() )
             xNumberFormats = Reference< util::XNumberFormats >( xNumberFormatsSupplier->getNumberFormats() );

        bool bOwnData = false;
        bool bOwnDataAndDateFormat = false;
        sal_Int32 nAxisNumberFormat = 0;
        Reference< chart2::XChartDocument > xChartDoc( xNumberFormatsSupplier, uno::UNO_QUERY );
        Reference< XCoordinateSystem > xCooSysModel( ChartModelHelper::getFirstCoordinateSystem( Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ) ) );
        if( xChartDoc.is() && xCooSysModel.is() )
        {
            if( xChartDoc->hasInternalDataProvider() )
            {
                bOwnData = true;
                Reference< beans::XPropertySet > xAxisProps( xCooSysModel->getAxisByDimension(0,0), uno::UNO_QUERY );
                if( xAxisProps.is() && (xAxisProps->getPropertyValue( C2U("NumberFormat") ) >>= nAxisNumberFormat) )
                    bOwnDataAndDateFormat = lcl_isDateFormat( nAxisNumberFormat, xNumberFormats );
            }
        }

        for(sal_Int32 nN=0;nN<nCount;nN++)
        {
            bool bIsDate = false;
            if( bIsAutoDate )
            {
                if( bOwnData )
                    bIsDate = bOwnDataAndDateFormat;
                else
                    bIsDate = lcl_isDateFormat( xDataSequence->getNumberFormatKeyByIndex( nN ), xNumberFormats );
            }
            else
                bIsDate = true;

            uno::Any aAny = aValues[nN];
            DatePlusIndex aDatePlusIndex( 1.0, nN );
            if( bIsDate && (aAny >>= aDatePlusIndex.fValue) )
                rDateCategories.push_back( aDatePlusIndex );
            else
            {
                if( aAny.hasValue() )
                {
                    OUString aTest;
                    if( !( (aAny>>=aTest) && !aTest.getLength() ) )//empty string does not count as non date value!
                        bOnlyDatesFound=false;
                }
                ::rtl::math::setNan( &aDatePlusIndex.fValue );
                rDateCategories.push_back( aDatePlusIndex );
            }
        }
        ::std::sort( rDateCategories.begin(), rDateCategories.end(), DatePlusIndexComparator() );
    }

    return bOnlyDatesFound;
}

void ExplicitCategoriesProvider::init()
{
    if( m_bDirty )
    {
        m_aExplicitCategories.realloc(0);
        m_aComplexCats.clear();//not one per index
        m_aDateCategories.clear();

        if( m_xOriginalCategories.is() )
        {
            if( !hasComplexCategories() )
            {
                m_aExplicitCategories = DataSequenceToStringSequence(m_xOriginalCategories->getValues());
                if(m_bIsDateAxis)
                {
                    if( ChartTypeHelper::isSupportingDateAxis( AxisHelper::getChartTypeByIndex( m_xCooSysModel, 0 ), 2, 0 ) )
                        m_bIsDateAxis = lcl_fillDateCategories( m_xOriginalCategories->getValues(), m_aDateCategories, m_bIsAutoDate, Reference< util::XNumberFormatsSupplier >( m_xChartModel.get(), uno::UNO_QUERY ) );
                    else
                        m_bIsDateAxis = false;
                }
            }
            else
            {
                m_aExplicitCategories = lcl_getExplicitSimpleCategories(
                    SplitCategoriesProvider_ForLabeledDataSequences( m_aSplitCategoriesList ), m_aComplexCats );
                m_bIsDateAxis = false;
            }
        }
        else
            m_bIsDateAxis=false;
        if(!m_aExplicitCategories.getLength())
            m_aExplicitCategories = DiagramHelper::generateAutomaticCategoriesFromCooSys( m_xCooSysModel );
        m_bDirty = false;
    }
}


Sequence< ::rtl::OUString > ExplicitCategoriesProvider::getSimpleCategories()
{
    init();
    return m_aExplicitCategories;
}

std::vector< ComplexCategory >  ExplicitCategoriesProvider::getCategoriesByLevel( sal_Int32 nLevel )
{
    std::vector< ComplexCategory > aRet;
    init();
    sal_Int32 nMaxIndex = m_aComplexCats.size()-1;
    if( nLevel >= 0 && nLevel <= nMaxIndex  )
        aRet = m_aComplexCats[nMaxIndex-nLevel];
    return aRet;
}

// static
OUString ExplicitCategoriesProvider::getCategoryByIndex(
          const Reference< XCoordinateSystem >& xCooSysModel
        , const uno::Reference< frame::XModel >& xChartModel
        , sal_Int32 nIndex )
{
    if( xCooSysModel.is())
    {
        ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSysModel, xChartModel );
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

const std::vector< DatePlusIndex >&  ExplicitCategoriesProvider::getDateCategories()
{
    init();
    return m_aDateCategories;
}

//.............................................................................
} //namespace chart
//.............................................................................
