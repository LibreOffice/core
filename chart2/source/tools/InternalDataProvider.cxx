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

#include <rtl/math.hxx>

#include <valarray>

#include "InternalDataProvider.hxx"
#include "LabeledDataSequence.hxx"
#include "DataSource.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "XMLRangeHelper.hxx"
#include "ContainerHelper.hxx"
#include "CommonConverters.hxx"
#include "CommonFunctors.hxx"
#include "UncachedDataSequence.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "ExplicitCategoriesProvider.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <rtl/ustrbuf.hxx>
#include <unotools/charclass.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

namespace
{

static const char lcl_aCategoriesRangeName[] = "categories";
static const char lcl_aCategoriesLevelRangeNamePrefix[] = "categoriesL "; //L <-> level
static const char lcl_aCategoriesPointRangeNamePrefix[] = "categoriesP "; //P <-> point
static const char lcl_aCategoriesRoleName[] = "categories";
static const char lcl_aLabelRangePrefix[] = "label ";
static const char lcl_aCompleteRange[] = "all";

typedef ::std::multimap< OUString, uno::WeakReference< chart2::data::XDataSequence > >
    lcl_tSequenceMap;

std::vector< OUString > lcl_AnyToStringSequence( const std::vector< uno::Any >& aAnySeq )
{
    std::vector< OUString > aResult;
    aResult.resize( aAnySeq.size() );
    int i = 0;
    for (const uno::Any& aAny : aAnySeq)
       aResult[i++] = CommonFunctors::AnyToString()(aAny);
    return aResult;
}

std::vector< uno::Any > lcl_StringToAnyVector( const css::uno::Sequence< OUString >& aStringSeq )
{
    std::vector< uno::Any > aResult;
    aResult.resize( aStringSeq.getLength() );
    int i = 0;
    for (const OUString& aStr : aStringSeq)
       aResult[i++] = CommonFunctors::makeAny<OUString>()(aStr);
    return aResult;
}

struct lcl_setModified : public ::std::unary_function< lcl_tSequenceMap, void >
{
    void operator() ( const lcl_tSequenceMap::value_type & rMapEntry )
    {
        // convert weak reference to reference
        Reference< chart2::data::XDataSequence > xSeq( rMapEntry.second );
        if( xSeq.is())
        {
            Reference< util::XModifiable > xMod( xSeq, uno::UNO_QUERY );
            if( xMod.is())
                xMod->setModified( true );
        }
    }
};

struct lcl_internalizeSeries : public ::std::unary_function< Reference< chart2::XDataSeries >, void >
{
    lcl_internalizeSeries( InternalData & rInternalData,
                           InternalDataProvider & rProvider,
                           bool bConnectToModel, bool bDataInColumns ) :
            m_rInternalData( rInternalData ),
            m_rProvider( rProvider ),
            m_bConnectToModel( bConnectToModel ),
            m_bDataInColumns( bDataInColumns )
    {}
    void operator() ( const Reference< chart2::XDataSeries > & xSeries )
    {
        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        Reference< chart2::data::XDataSink >   xSink(   xSeries, uno::UNO_QUERY );
        if( xSource.is() && xSink.is() )
        {
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aOldSeriesData = xSource->getDataSequences();
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aNewSeriesData( aOldSeriesData.getLength() );
            for( sal_Int32 i=0; i<aOldSeriesData.getLength(); ++i )
            {
                sal_Int32 nNewIndex( m_bDataInColumns ? m_rInternalData.appendColumn() : m_rInternalData.appendRow() );
                OUString aIdentifier( OUString::number( nNewIndex ));
                //@todo: deal also with genericXDataSequence
                Reference< chart2::data::XNumericalDataSequence > xValues( aOldSeriesData[i]->getValues(), uno::UNO_QUERY );
                Reference< chart2::data::XTextualDataSequence > xLabel( aOldSeriesData[i]->getLabel(), uno::UNO_QUERY );
                Reference< chart2::data::XDataSequence > xNewValues;

                if( xValues.is() )
                {
                    ::std::vector< double > aValues( ContainerHelper::SequenceToVector( xValues->getNumericalData()));
                    if( m_bDataInColumns )
                        m_rInternalData.setColumnValues( nNewIndex, aValues );
                    else
                        m_rInternalData.setRowValues( nNewIndex, aValues );
                    if( m_bConnectToModel )
                    {
                        xNewValues.set( m_rProvider.createDataSequenceByRangeRepresentation( aIdentifier ));
                        comphelper::copyProperties(
                            Reference< beans::XPropertySet >( xValues, uno::UNO_QUERY ),
                            Reference< beans::XPropertySet >( xNewValues, uno::UNO_QUERY ));
                    }
                }

                if( xLabel.is() )
                {
                    if( m_bDataInColumns )
                        m_rInternalData.setComplexColumnLabel( nNewIndex, lcl_StringToAnyVector( xLabel->getTextualData() ) );
                    else
                        m_rInternalData.setComplexRowLabel( nNewIndex, lcl_StringToAnyVector( xLabel->getTextualData() ) );
                    if( m_bConnectToModel )
                    {
                        Reference< chart2::data::XDataSequence > xNewLabel(
                            m_rProvider.createDataSequenceByRangeRepresentation( lcl_aLabelRangePrefix + aIdentifier ));
                        comphelper::copyProperties(
                            Reference< beans::XPropertySet >( xLabel, uno::UNO_QUERY ),
                            Reference< beans::XPropertySet >( xNewLabel, uno::UNO_QUERY ));
                        aNewSeriesData[i].set( new LabeledDataSequence( xNewValues, xNewLabel ) );
                    }
                }
                else
                {
                    if( m_bConnectToModel )
                        aNewSeriesData[i].set( new LabeledDataSequence( xNewValues ) );
                }
            }
            if( m_bConnectToModel )
                xSink->setData( aNewSeriesData );
        }
     }

private:
    InternalData &          m_rInternalData;
    InternalDataProvider &  m_rProvider;
    bool                    m_bConnectToModel;
    bool                    m_bDataInColumns;
};

struct lcl_copyFromLevel : public ::std::unary_function< vector< uno::Any >, uno::Any >
{
public:

    explicit lcl_copyFromLevel( sal_Int32 nLevel ) : m_nLevel( nLevel )
    {}

    uno::Any operator() ( const vector< uno::Any >& rVector )
    {
        uno::Any aRet;
        if( m_nLevel <  static_cast< sal_Int32 >(rVector.size()) )
            aRet = rVector[m_nLevel];
        return aRet;
    }

private:
    sal_Int32 m_nLevel;
};

struct lcl_getStringFromLevelVector : public ::std::unary_function< vector< uno::Any >, OUString >
{
public:

    explicit lcl_getStringFromLevelVector( sal_Int32 nLevel ) : m_nLevel( nLevel )
    {}

    OUString operator() ( const vector< uno::Any >& rVector )
    {
        OUString aString;
        if( m_nLevel < static_cast< sal_Int32 >(rVector.size()) )
            aString = CommonFunctors::AnyToString()(rVector[m_nLevel]);
        return aString;
    }

private:
    sal_Int32 m_nLevel;
};

struct lcl_setAnyAtLevel : public ::std::binary_function< vector< uno::Any >, uno::Any, vector< uno::Any > >
{
public:

    explicit lcl_setAnyAtLevel( sal_Int32 nLevel ) : m_nLevel( nLevel )
    {}

    vector< uno::Any > operator() ( const vector< uno::Any >& rVector, const uno::Any& rNewValue )
    {
        vector< uno::Any > aRet( rVector );
        if( m_nLevel >= static_cast< sal_Int32 >(aRet.size()) )
            aRet.resize( m_nLevel+1 );
        aRet[ m_nLevel ]=rNewValue;
        return aRet;
    }

private:
    sal_Int32 m_nLevel;
};

struct lcl_setAnyAtLevelFromStringSequence : public ::std::binary_function< vector< uno::Any >, OUString, vector< uno::Any > >
{
public:

    explicit lcl_setAnyAtLevelFromStringSequence( sal_Int32 nLevel ) : m_nLevel( nLevel )
    {}

    vector< uno::Any > operator() ( const vector< uno::Any >& rVector, const OUString& rNewValue )
    {
        vector< uno::Any > aRet( rVector );
        if( m_nLevel >= static_cast< sal_Int32 >(aRet.size()) )
            aRet.resize( m_nLevel+1 );
        aRet[ m_nLevel ] <<= rNewValue;
        return aRet;
    }

private:
    sal_Int32 m_nLevel;
};

struct lcl_insertAnyAtLevel : public ::std::unary_function< vector< uno::Any >, void >
{
public:

    explicit lcl_insertAnyAtLevel( sal_Int32 nLevel ) : m_nLevel( nLevel )
    {}

    void operator() ( vector< uno::Any >& rVector )
    {
        if( m_nLevel >= static_cast< sal_Int32 >(rVector.size()) )
        {
            rVector.resize( m_nLevel + 1 );
        }
        else
        {
            rVector.insert( rVector.begin() + m_nLevel, uno::Any() );
        }
    }

private:
    sal_Int32 m_nLevel;
};

struct lcl_removeAnyAtLevel : public ::std::unary_function< vector< uno::Any >, void >
{
public:

    explicit lcl_removeAnyAtLevel( sal_Int32 nLevel ) : m_nLevel( nLevel )
    {}

    void operator() ( vector< uno::Any >& rVector )
    {
        if( m_nLevel < static_cast<sal_Int32>(rVector.size()) )
        {
            rVector.erase(rVector.begin() + m_nLevel);
        }
    }

private:
    sal_Int32 m_nLevel;
};

} // anonymous namespace

InternalDataProvider::InternalDataProvider( const Reference< uno::XComponentContext > & /*_xContext*/)
    : m_bDataInColumns( true )
{}

InternalDataProvider::InternalDataProvider(
    const Reference< chart2::XChartDocument > & xChartDoc,
    bool bConnectToModel,
    bool bDefaultDataInColumns)
:   m_bDataInColumns( bDefaultDataInColumns )
{
    try
    {
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartDoc ) );
        if( xDiagram.is())
        {
            Reference< frame::XModel > xChartModel( xChartDoc, uno::UNO_QUERY );

            //data in columns?
            {
                OUString aRangeString;
                bool bFirstCellAsLabel = true;
                bool bHasCategories = true;
                uno::Sequence< sal_Int32 > aSequenceMapping;
                const bool bSomethingDetected(
                    DataSourceHelper::detectRangeSegmentation(
                        xChartModel, aRangeString, aSequenceMapping, m_bDataInColumns, bFirstCellAsLabel, bHasCategories ));

                // #i120559# if no data was available, restore default
                if(!bSomethingDetected && m_bDataInColumns != bDefaultDataInColumns)
                {
                    m_bDataInColumns = bDefaultDataInColumns;
                }
            }

            // categories
            {
                vector< vector< uno::Any > > aNewCategories;//inner count is level
                {
                    ChartModel& rModel = dynamic_cast<ChartModel&>(*xChartModel.get());
                    ExplicitCategoriesProvider aExplicitCategoriesProvider(ChartModelHelper::getFirstCoordinateSystem(xChartModel), rModel);

                    const Sequence< Reference< chart2::data::XLabeledDataSequence> >& rSplitCategoriesList( aExplicitCategoriesProvider.getSplitCategoriesList() );
                    sal_Int32 nLevelCount = rSplitCategoriesList.getLength();
                    for( sal_Int32 nL = 0; nL<nLevelCount; nL++ )
                    {
                        Reference< chart2::data::XLabeledDataSequence > xLDS( rSplitCategoriesList[nL] );
                        if( !xLDS.is() )
                            continue;
                        Sequence< uno::Any > aDataSeq;
                        Reference< chart2::data::XDataSequence > xSeq( xLDS->getValues() );
                        if( xSeq.is() )
                            aDataSeq = xSeq->getData();
                        sal_Int32 nLength = aDataSeq.getLength();
                        sal_Int32 nCatLength = static_cast< sal_Int32 >(aNewCategories.size());
                        if( nCatLength < nLength )
                            aNewCategories.resize( nLength );
                        else if( nLength < nCatLength )
                            aDataSeq.realloc( nCatLength );
                        transform( aNewCategories.begin(), aNewCategories.end(), aDataSeq.getConstArray(),
                            aNewCategories.begin(), lcl_setAnyAtLevel(nL) );
                    }
                    if( !nLevelCount )
                    {
                        Sequence< OUString > aSimplecategories = aExplicitCategoriesProvider.getSimpleCategories();
                        sal_Int32 nLength = aSimplecategories.getLength();
                        aNewCategories.reserve( nLength );
                        for( sal_Int32 nN=0; nN<nLength; nN++)
                        {
                            vector< uno::Any > aVector(1);
                            aVector[0] <<= aSimplecategories[nN];
                            aNewCategories.push_back( aVector );
                        }
                    }
                }

                if( m_bDataInColumns )
                    m_aInternalData.setComplexRowLabels( aNewCategories );
                else
                    m_aInternalData.setComplexColumnLabels( aNewCategories );
                if( bConnectToModel )
                    DiagramHelper::setCategoriesToDiagram( new LabeledDataSequence(
                        createDataSequenceByRangeRepresentation( lcl_aCategoriesRangeName )), xDiagram );
            }

            // data series
            ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( ChartModelHelper::getDataSeries( xChartDoc ));
            lcl_internalizeSeries ftor( m_aInternalData, *this, bConnectToModel, m_bDataInColumns );
            for( const auto& rxScreen : aSeriesVector )
                ftor( rxScreen );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// copy-CTOR
InternalDataProvider::InternalDataProvider( const InternalDataProvider & rOther ) :
        impl::InternalDataProvider_Base(),
        m_aSequenceMap( rOther.m_aSequenceMap ),
        m_aInternalData( rOther.m_aInternalData ),
        m_bDataInColumns( rOther.m_bDataInColumns )
{}

InternalDataProvider::~InternalDataProvider()
{}

void InternalDataProvider::addDataSequenceToMap(
    const OUString & rRangeRepresentation,
    const Reference< chart2::data::XDataSequence > & xSequence )
{
    m_aSequenceMap.insert(
        tSequenceMap::value_type(
            rRangeRepresentation,
            uno::WeakReference< chart2::data::XDataSequence >( xSequence )));
}

void InternalDataProvider::deleteMapReferences( const OUString & rRangeRepresentation )
{
    // set sequence to deleted by setting its range to an empty string
    tSequenceMapRange aRange( m_aSequenceMap.equal_range( rRangeRepresentation ));
    for( tSequenceMap::iterator aIt( aRange.first ); aIt != aRange.second; ++aIt )
    {
        Reference< chart2::data::XDataSequence > xSeq( aIt->second );
        if( xSeq.is())
        {
            Reference< container::XNamed > xNamed( xSeq, uno::UNO_QUERY );
            if( xNamed.is())
                xNamed->setName( OUString());
        }
    }
    // remove from map
    m_aSequenceMap.erase( aRange.first, aRange.second );
}

void InternalDataProvider::adaptMapReferences(
    const OUString & rOldRangeRepresentation,
    const OUString & rNewRangeRepresentation )
{
    tSequenceMapRange aRange( m_aSequenceMap.equal_range( rOldRangeRepresentation ));
    tSequenceMap aNewElements;
    for( tSequenceMap::iterator aIt( aRange.first ); aIt != aRange.second; ++aIt )
    {
        Reference< chart2::data::XDataSequence > xSeq( aIt->second );
        if( xSeq.is())
        {
            Reference< container::XNamed > xNamed( xSeq, uno::UNO_QUERY );
            if( xNamed.is())
                xNamed->setName( rNewRangeRepresentation );
        }
        aNewElements.insert( tSequenceMap::value_type( rNewRangeRepresentation, aIt->second ));
    }
    // erase map values for old index
    m_aSequenceMap.erase( aRange.first, aRange.second );
    // add new entries for values with new index
    ::std::copy( aNewElements.begin(), aNewElements.end(),
                 ::std::inserter( m_aSequenceMap,
                                  m_aSequenceMap.upper_bound( rNewRangeRepresentation )));
}

void InternalDataProvider::increaseMapReferences(
    sal_Int32 nBegin, sal_Int32 nEnd )
{
    for( sal_Int32 nIndex = nEnd - 1; nIndex >= nBegin; --nIndex )
    {
        adaptMapReferences( OUString::number( nIndex ),
                            OUString::number( nIndex + 1 ));
        adaptMapReferences( lcl_aLabelRangePrefix + OUString::number( nIndex ),
                            lcl_aLabelRangePrefix + OUString::number( nIndex + 1 ));
    }
}

void InternalDataProvider::decreaseMapReferences(
    sal_Int32 nBegin, sal_Int32 nEnd )
{
    for( sal_Int32 nIndex = nBegin; nIndex < nEnd; ++nIndex )
    {
        adaptMapReferences( OUString::number( nIndex ),
                            OUString::number( nIndex - 1 ));
        adaptMapReferences( lcl_aLabelRangePrefix + OUString::number( nIndex ),
                            lcl_aLabelRangePrefix + OUString::number( nIndex - 1 ));
    }
}

Reference< chart2::data::XDataSequence > InternalDataProvider::createDataSequenceAndAddToMap(
    const OUString & rRangeRepresentation )
{
    Reference<chart2::data::XDataSequence> xSeq = createDataSequenceFromArray(rRangeRepresentation, OUString());
    if (xSeq.is())
        return xSeq;

    xSeq.set(new UncachedDataSequence(this, rRangeRepresentation));
    addDataSequenceToMap(rRangeRepresentation, xSeq);
    return xSeq;
}

uno::Reference<chart2::data::XDataSequence>
InternalDataProvider::createDataSequenceFromArray( const OUString& rArrayStr, const OUString& rRole )
{
    if (rArrayStr.indexOf('{') != 0 || rArrayStr[rArrayStr.getLength()-1] != '}')
    {
        // Not an array string.
        return uno::Reference<chart2::data::XDataSequence>();
    }

    bool bAllNumeric = true;
    uno::Reference<chart2::data::XDataSequence> xSeq;

    const sal_Unicode* p = rArrayStr.getStr();
    const sal_Unicode* pEnd = p + rArrayStr.getLength();
    const sal_Unicode* pElem = nullptr;
    OUString aElem;

    std::vector<OUString> aRawElems;
    ++p; // Skip the first '{'.
    --pEnd; // Skip the last '}'.
    bool bInQuote = false;
    for (; p != pEnd; ++p)
    {
        if (*p == '"')
        {
            bInQuote = !bInQuote;
            if (bInQuote)
            {
                // Opening quote.
                bAllNumeric = false;
                pElem = nullptr;
            }
            else
            {
                // Closing quote.
                if (pElem)
                    aElem = OUString(pElem, p-pElem);
                aRawElems.push_back(aElem);
                pElem = nullptr;
                aElem.clear();

                ++p; // Skip '"'.
                if (p == pEnd)
                    break;
            }
        }
        else if (*p == ';' && !bInQuote)
        {
            // element separator.
            if (pElem)
                aElem = OUString(pElem, p-pElem);
            aRawElems.push_back(aElem);
            pElem = nullptr;
            aElem.clear();
        }
        else if (!pElem)
            pElem = p;
    }

    if (pElem)
    {
        aElem = OUString(pElem, p-pElem);
        aRawElems.push_back(aElem);
    }

    if (rRole == "values-y" || rRole == "values-first" || rRole == "values-last" ||
        rRole == "values-min" || rRole == "values-max")
    {
        // Column values.  Append a new data column and populate it.

        std::vector<double> aValues;
        aValues.reserve(aRawElems.size());
        for (OUString & aRawElem : aRawElems)
        {
            if (aRawElem.isEmpty())
                aValues.push_back(NAN);
            else
                aValues.push_back(aRawElem.toDouble());
        }
        sal_Int32 n = m_aInternalData.appendColumn();

        m_aInternalData.setColumnValues(n, aValues);

        OUString aRangeRep = OUString::number(n);
        xSeq.set(new UncachedDataSequence(this, aRangeRep));
        addDataSequenceToMap(aRangeRep, xSeq);
    }
    else if (rRole == "values-x")
    {
        std::vector<double> aValues;
        aValues.reserve(aRawElems.size());
        if (bAllNumeric)
        {
            for (OUString & aRawElem : aRawElems)
                aValues.push_back(aRawElem.toDouble());
        }
        else
        {
            for (size_t i = 0; i < aRawElems.size(); ++i)
                aValues.push_back(i+1);
        }

        sal_Int32 n = m_aInternalData.appendColumn();
        m_aInternalData.setColumnValues(n, aValues);

        OUString aRangeRep = OUString::number(n);
        xSeq.set(new UncachedDataSequence(this, aRangeRep));
        addDataSequenceToMap(aRangeRep, xSeq);
    }
    else if (rRole == "categories")
    {
        // Category labels.

        for (size_t i = 0; i < aRawElems.size(); ++i)
        {
            std::vector<uno::Any> aLabels(1, uno::Any(aRawElems[i]));
            m_aInternalData.setComplexRowLabel(i, aLabels);
        }

        xSeq.set(new UncachedDataSequence(this, lcl_aCategoriesRangeName));
        addDataSequenceToMap(lcl_aCategoriesRangeName, xSeq);
    }
    else if (rRole == "label")
    {
        // Data series label.  There should be only one element.  This always
        // goes to the last data column.
        sal_Int32 nColSize = m_aInternalData.getColumnCount();
        if (!aRawElems.empty() && nColSize)
        {
            std::vector<uno::Any> aLabels(1, uno::Any(aRawElems[0]));
            m_aInternalData.setComplexColumnLabel(nColSize-1, aLabels);

            OUString aRangeRep = lcl_aLabelRangePrefix + OUString::number(nColSize-1);
            xSeq.set(new UncachedDataSequence(this, aRangeRep));
            addDataSequenceToMap(aRangeRep, xSeq);
        }
    }

    return xSeq;
}

Reference< chart2::data::XDataSequence > InternalDataProvider::createDataSequenceAndAddToMap(
    const OUString & rRangeRepresentation,
    const OUString & rRole )
{
    Reference< chart2::data::XDataSequence > xSeq(
        new UncachedDataSequence( this, rRangeRepresentation, rRole ));
    addDataSequenceToMap( rRangeRepresentation, xSeq );
    return xSeq;
}

// ____ XDataProvider ____
sal_Bool SAL_CALL InternalDataProvider::createDataSourcePossible( const Sequence< beans::PropertyValue >& /* aArguments */ )
{
    return true;
}

namespace
{

sal_Int32 lcl_getInnerLevelCount( const vector< vector< uno::Any > >& rLabels )
{
    sal_Int32 nCount = 1;//minimum is 1!
    vector< vector< uno::Any > >::const_iterator aLevelIt( rLabels.begin() );
    vector< vector< uno::Any > >::const_iterator aLevelEnd( rLabels.end() );
    for( ;aLevelIt!=aLevelEnd; ++aLevelIt )
    {
        const vector< uno::Any >& rCurrentLevelLabels = *aLevelIt;
        nCount = std::max<sal_Int32>( rCurrentLevelLabels.size(), nCount );
    }
    return nCount;
}

}//end anonymous namespace

Reference< chart2::data::XDataSource > SAL_CALL InternalDataProvider::createDataSource(
    const Sequence< beans::PropertyValue >& aArguments )
{
    OUString aRangeRepresentation;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    DataSourceHelper::readArguments( aArguments, aRangeRepresentation, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    if( aRangeRepresentation == lcl_aCategoriesRangeName )
    {
        //return split complex categories if we have any:
        ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aComplexCategories;
        vector< vector< uno::Any > > aCategories( m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels());
        if( bUseColumns==m_bDataInColumns )
        {
            sal_Int32 nLevelCount = lcl_getInnerLevelCount( aCategories );
            for( sal_Int32 nL=0; nL<nLevelCount; nL++ )
                aComplexCategories.push_back( new LabeledDataSequence(
                    new UncachedDataSequence( this
                        , lcl_aCategoriesLevelRangeNamePrefix + OUString::number( nL )
                        , lcl_aCategoriesRoleName ) ) );
        }
        else
        {
            sal_Int32 nPointCount = m_bDataInColumns ? m_aInternalData.getRowCount() : m_aInternalData.getColumnCount();
            for( sal_Int32 nP=0; nP<nPointCount; nP++ )
                aComplexCategories.push_back( new LabeledDataSequence(
                    new UncachedDataSequence( this
                        , lcl_aCategoriesPointRangeNamePrefix + OUString::number( nP )
                        , lcl_aCategoriesRoleName ) ) );
        }
        //don't add the created sequences to the map as they are used temporarily only ...
        return new DataSource( comphelper::containerToSequence(aComplexCategories) );
    }

    OSL_ASSERT( aRangeRepresentation == lcl_aCompleteRange );

    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultLSeqVec;

    // categories
    if( bHasCategories )
        aResultLSeqVec.push_back(
            new LabeledDataSequence( createDataSequenceAndAddToMap( lcl_aCategoriesRangeName, lcl_aCategoriesRoleName ) ) );

    // data with labels
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aDataVec;
    const sal_Int32 nCount = (bUseColumns ? m_aInternalData.getColumnCount() : m_aInternalData.getRowCount());
    for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
    {
        aDataVec.push_back(
            new LabeledDataSequence(
                createDataSequenceAndAddToMap( OUString::number( nIdx )),
                createDataSequenceAndAddToMap( lcl_aLabelRangePrefix + OUString::number( nIdx ))));
    }

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings
    m_bDataInColumns = bUseColumns;

    //reorder labeled sequences according to aSequenceMapping; ignore categories
    for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
    {
        std::vector< LabeledDataSequence* >::size_type nOldIndex = aSequenceMapping[nNewIndex];
        if( nOldIndex < aDataVec.size() )
        {
            if( aDataVec[nOldIndex].is() )
            {
                aResultLSeqVec.push_back( aDataVec[nOldIndex] );
                aDataVec[nOldIndex] = nullptr;
            }
        }
    }

    //add left over data sequences to result
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::iterator aIt(aDataVec.begin());
    const ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::const_iterator aEndIt(aDataVec.end());
    for( ;aIt!=aEndIt; ++aIt)
    {
        if( aIt->is() )
            aResultLSeqVec.push_back( *aIt );
    }

    return new DataSource( comphelper::containerToSequence(aResultLSeqVec) );
}

Sequence< beans::PropertyValue > SAL_CALL InternalDataProvider::detectArguments(
    const Reference< chart2::data::XDataSource >& /* xDataSource */ )
{
    Sequence< beans::PropertyValue > aArguments( 4 );
    aArguments[0] = beans::PropertyValue(
        "CellRangeRepresentation", -1, uno::Any( OUString(lcl_aCompleteRange) ),
        beans::PropertyState_DIRECT_VALUE );
    aArguments[1] = beans::PropertyValue(
        "DataRowSource", -1, uno::Any(
            m_bDataInColumns
            ? css::chart::ChartDataRowSource_COLUMNS
            : css::chart::ChartDataRowSource_ROWS ),
        beans::PropertyState_DIRECT_VALUE );
    // internal data always contains labels and categories
    aArguments[2] = beans::PropertyValue(
        "FirstCellAsLabel", -1, uno::Any( true ), beans::PropertyState_DIRECT_VALUE );
    aArguments[3] = beans::PropertyValue(
        "HasCategories", -1, uno::Any( true ), beans::PropertyState_DIRECT_VALUE );

    // #i85913# Sequence Mapping is not needed for internal data, as it is
    // applied to the data when the data source is created.

    return aArguments;
}

sal_Bool SAL_CALL InternalDataProvider::createDataSequenceByRangeRepresentationPossible( const OUString& /* aRangeRepresentation */ )
{
    return true;
}

Reference< chart2::data::XDataSequence > SAL_CALL InternalDataProvider::createDataSequenceByRangeRepresentation(
    const OUString& aRangeRepresentation )
{
    if( aRangeRepresentation.match( lcl_aCategoriesRangeName ))
    {
        OSL_ASSERT( aRangeRepresentation == lcl_aCategoriesRangeName );//it is not expected nor implemented that only parts of the categories are really requested

        // categories
        return createDataSequenceAndAddToMap( lcl_aCategoriesRangeName, lcl_aCategoriesRoleName );
    }
    else if( aRangeRepresentation.match( lcl_aLabelRangePrefix ))
    {
        // label
        sal_Int32 nIndex = aRangeRepresentation.copy( strlen(lcl_aLabelRangePrefix)).toInt32();
        return createDataSequenceAndAddToMap( lcl_aLabelRangePrefix + OUString::number( nIndex ));
    }
    else if ( aRangeRepresentation == "last" )
    {
        sal_Int32 nIndex = (m_bDataInColumns
                            ? m_aInternalData.getColumnCount()
                            : m_aInternalData.getRowCount()) - 1;
        return createDataSequenceAndAddToMap( OUString::number( nIndex ));
    }
    else if( !aRangeRepresentation.isEmpty())
    {
        // data
        return createDataSequenceAndAddToMap( aRangeRepresentation );
    }

    return Reference< chart2::data::XDataSequence >();
}

Reference<chart2::data::XDataSequence> SAL_CALL
InternalDataProvider::createDataSequenceByValueArray(
    const OUString& aRole, const OUString& aRangeRepresentation )
{
    return createDataSequenceFromArray(aRangeRepresentation, aRole);
}

Reference< sheet::XRangeSelection > SAL_CALL InternalDataProvider::getRangeSelection()
{
    // there is no range selection component
    return Reference< sheet::XRangeSelection >();
}

// ____ XInternalDataProvider ____
sal_Bool SAL_CALL InternalDataProvider::hasDataByRangeRepresentation( const OUString& aRange )
{
    bool bResult = false;

    if( aRange.match( lcl_aCategoriesRangeName ))
    {
        OSL_ASSERT( aRange == lcl_aCategoriesRangeName );//it is not expected nor implemented that only parts of the categories are really requested
        bResult = true;
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRange.copy( strlen(lcl_aLabelRangePrefix)).toInt32();
        bResult = (nIndex < (m_bDataInColumns ? m_aInternalData.getColumnCount(): m_aInternalData.getRowCount()));
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        bResult = (nIndex < (m_bDataInColumns ? m_aInternalData.getColumnCount(): m_aInternalData.getRowCount()));
    }

    return bResult;
}

Sequence< uno::Any > SAL_CALL InternalDataProvider::getDataByRangeRepresentation( const OUString& aRange )
{
    Sequence< uno::Any > aResult;

    if( aRange.match( lcl_aLabelRangePrefix ) )
    {
        sal_Int32 nIndex = aRange.copy( strlen(lcl_aLabelRangePrefix)).toInt32();
        vector< uno::Any > aComplexLabel = m_bDataInColumns
            ? m_aInternalData.getComplexColumnLabel( nIndex )
            : m_aInternalData.getComplexRowLabel( nIndex );
        if( !aComplexLabel.empty() )
            aResult = comphelper::containerToSequence(aComplexLabel);
    }
    else if( aRange.match( lcl_aCategoriesPointRangeNamePrefix ) )
    {
        sal_Int32 nPointIndex = aRange.copy( strlen(lcl_aCategoriesPointRangeNamePrefix) ).toInt32();
        vector< uno::Any > aComplexCategory = m_bDataInColumns
            ? m_aInternalData.getComplexRowLabel( nPointIndex )
            : m_aInternalData.getComplexColumnLabel( nPointIndex );
        if( !aComplexCategory.empty() )
            aResult = comphelper::containerToSequence(aComplexCategory);
    }
    else if( aRange.match( lcl_aCategoriesLevelRangeNamePrefix ) )
    {
        sal_Int32 nLevel = aRange.copy( strlen(lcl_aCategoriesLevelRangeNamePrefix) ).toInt32();
        vector< vector< uno::Any > > aCategories( m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels());
        if( nLevel < lcl_getInnerLevelCount( aCategories ) )
        {
            aResult.realloc( aCategories.size() );
            transform( aCategories.begin(), aCategories.end(),
                       aResult.getArray(), lcl_copyFromLevel(nLevel) );
        }
    }
    else if( aRange == lcl_aCategoriesRangeName )
    {
        vector< vector< uno::Any > > aCategories( m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels());
        sal_Int32 nLevelCount = lcl_getInnerLevelCount( aCategories );
        if( nLevelCount == 1 )
        {
            sal_Int32 nL=0;
            aResult = this->getDataByRangeRepresentation( lcl_aCategoriesLevelRangeNamePrefix + OUString::number( nL ) );
        }
        else
        {
            Sequence< OUString > aLabels = m_bDataInColumns ? this->getRowDescriptions() : this->getColumnDescriptions();
            aResult.realloc( aLabels.getLength() );
            transform( aLabels.getConstArray(), aLabels.getConstArray() + aLabels.getLength(),
                       aResult.getArray(), CommonFunctors::makeAny< OUString >() );
        }
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        if( nIndex >= 0 )
        {
            Sequence< double > aData;
            if( m_bDataInColumns )
                aData = m_aInternalData.getColumnValues(nIndex);
            else
                aData = m_aInternalData.getRowValues(nIndex);
            if( aData.getLength() )
            {
                aResult.realloc( aData.getLength());
                transform( aData.getConstArray(), aData.getConstArray() + aData.getLength(),
                           aResult.getArray(), CommonFunctors::makeAny< double >());
            }
        }
    }

    return aResult;
}

void SAL_CALL InternalDataProvider::setDataByRangeRepresentation(
    const OUString& aRange, const Sequence< uno::Any >& aNewData )
{
    vector< uno::Any > aNewVector( ContainerHelper::SequenceToVector(aNewData) );
    if( aRange.match( lcl_aLabelRangePrefix ) )
    {
        sal_uInt32 nIndex = aRange.copy( strlen(lcl_aLabelRangePrefix)).toInt32();
        if( m_bDataInColumns )
            m_aInternalData.setComplexColumnLabel( nIndex, aNewVector );
        else
            m_aInternalData.setComplexRowLabel( nIndex, aNewVector );
    }
    else if( aRange.match( lcl_aCategoriesPointRangeNamePrefix ) )
    {
        sal_Int32 nPointIndex = aRange.copy( strlen(lcl_aCategoriesLevelRangeNamePrefix)).toInt32();
        if( m_bDataInColumns )
            m_aInternalData.setComplexRowLabel( nPointIndex, aNewVector );
        else
            m_aInternalData.setComplexColumnLabel( nPointIndex, aNewVector );
    }
    else if( aRange.match( lcl_aCategoriesLevelRangeNamePrefix ) )
    {
        sal_Int32 nLevel = aRange.copy( strlen(lcl_aCategoriesLevelRangeNamePrefix)).toInt32();
        vector< vector< uno::Any > > aComplexCategories = m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels();

        //ensure equal length
        if( aNewVector.size() > aComplexCategories.size() )
            aComplexCategories.resize( aNewVector.size() );
        else if( aNewVector.size() < aComplexCategories.size() )
            aNewVector.resize( aComplexCategories.size() );

        transform( aComplexCategories.begin(), aComplexCategories.end(), aNewVector.begin(),
                   aComplexCategories.begin(), lcl_setAnyAtLevel(nLevel) );

        if( m_bDataInColumns )
            m_aInternalData.setComplexRowLabels( aComplexCategories );
        else
            m_aInternalData.setComplexColumnLabels( aComplexCategories );
    }
    else if( aRange == lcl_aCategoriesRangeName )
    {
        vector< vector< uno::Any > > aComplexCategories;
        aComplexCategories.resize( aNewVector.size() );
        transform( aComplexCategories.begin(), aComplexCategories.end(), aNewVector.begin(),
                            aComplexCategories.begin(), lcl_setAnyAtLevel(0) );
        if( m_bDataInColumns )
            m_aInternalData.setComplexRowLabels( aComplexCategories );
        else
            m_aInternalData.setComplexColumnLabels( aComplexCategories );
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        if( nIndex>=0 )
        {
            vector< double > aNewDataVec;
            transform( aNewData.getConstArray(), aNewData.getConstArray() + aNewData.getLength(),
                       back_inserter( aNewDataVec ), CommonFunctors::AnyToDouble());
            if( m_bDataInColumns )
                m_aInternalData.setColumnValues( nIndex, aNewDataVec );
            else
                m_aInternalData.setRowValues( nIndex, aNewDataVec );
        }
    }
}

void SAL_CALL InternalDataProvider::insertSequence( ::sal_Int32 nAfterIndex )
{
    if( m_bDataInColumns )
    {
        increaseMapReferences( nAfterIndex + 1, m_aInternalData.getColumnCount());
        m_aInternalData.insertColumn( nAfterIndex );
    }
    else
    {
        increaseMapReferences( nAfterIndex + 1, m_aInternalData.getRowCount());
        m_aInternalData.insertRow( nAfterIndex );
    }
}

void SAL_CALL InternalDataProvider::deleteSequence( ::sal_Int32 nAtIndex )
{
    deleteMapReferences( OUString::number( nAtIndex ));
    deleteMapReferences( lcl_aLabelRangePrefix + OUString::number( nAtIndex ));
    if( m_bDataInColumns )
    {
        decreaseMapReferences( nAtIndex + 1, m_aInternalData.getColumnCount());
        m_aInternalData.deleteColumn( nAtIndex );
    }
    else
    {
        decreaseMapReferences( nAtIndex + 1, m_aInternalData.getRowCount());
        m_aInternalData.deleteRow( nAtIndex );
    }
}

void SAL_CALL InternalDataProvider::appendSequence()
{
    if( m_bDataInColumns )
        m_aInternalData.appendColumn();
    else
        m_aInternalData.appendRow();
}

void SAL_CALL InternalDataProvider::insertComplexCategoryLevel( sal_Int32 nLevel )
{
    OSL_ENSURE( nLevel> 0, "you can only insert category levels > 0" );//the first categories level cannot be deleted, check the calling code for error
    if( nLevel>0 )
    {
        vector< vector< uno::Any > > aComplexCategories = m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels();
        ::std::for_each( aComplexCategories.begin(), aComplexCategories.end(), lcl_insertAnyAtLevel(nLevel) );
        if( m_bDataInColumns )
            m_aInternalData.setComplexRowLabels( aComplexCategories );
        else
            m_aInternalData.setComplexColumnLabels( aComplexCategories );

        tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
        ::std::for_each( aRange.first, aRange.second, lcl_setModified());
    }
}
void SAL_CALL InternalDataProvider::deleteComplexCategoryLevel( sal_Int32 nLevel )
{
    OSL_ENSURE( nLevel>0, "you can only delete category levels > 0" );//the first categories level cannot be deleted, check the calling code for error
    if( nLevel>0 )
    {
        vector< vector< uno::Any > > aComplexCategories = m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels();
        ::std::for_each( aComplexCategories.begin(), aComplexCategories.end(), lcl_removeAnyAtLevel(nLevel) );
        if( m_bDataInColumns )
            m_aInternalData.setComplexRowLabels( aComplexCategories );
        else
            m_aInternalData.setComplexColumnLabels( aComplexCategories );

        tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
        ::std::for_each( aRange.first, aRange.second, lcl_setModified());
    }
}

void SAL_CALL InternalDataProvider::insertDataPointForAllSequences( ::sal_Int32 nAfterIndex )
{
    sal_Int32 nMaxRep = 0;
    if( m_bDataInColumns )
    {
        m_aInternalData.insertRow( nAfterIndex );
        nMaxRep = m_aInternalData.getColumnCount();
    }
    else
    {
        m_aInternalData.insertColumn( nAfterIndex );
        nMaxRep = m_aInternalData.getRowCount();
    }

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( "0"));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::number( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_setModified());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_setModified());
}

void SAL_CALL InternalDataProvider::deleteDataPointForAllSequences( ::sal_Int32 nAtIndex )
{
    sal_Int32 nMaxRep = 0;
    if( m_bDataInColumns )
    {
        m_aInternalData.deleteRow( nAtIndex );
        nMaxRep = m_aInternalData.getColumnCount();
    }
    else
    {
        m_aInternalData.deleteColumn( nAtIndex );
        nMaxRep = m_aInternalData.getRowCount();
    }

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( "0"));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::number( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_setModified());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_setModified());
}

void SAL_CALL InternalDataProvider::swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex )
{
    if( m_bDataInColumns )
        m_aInternalData.swapRowWithNext( nAtIndex );
    else
        m_aInternalData.swapColumnWithNext( nAtIndex );
    sal_Int32 nMaxRep = (m_bDataInColumns
                         ? m_aInternalData.getColumnCount()
                         : m_aInternalData.getRowCount());

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( "0"));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::number( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_setModified());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_setModified());
}

void SAL_CALL InternalDataProvider::registerDataSequenceForChanges( const Reference< chart2::data::XDataSequence >& xSeq )
{
    if( xSeq.is())
        addDataSequenceToMap( xSeq->getSourceRangeRepresentation(), xSeq );
}

// ____ XRangeXMLConversion ____
OUString SAL_CALL InternalDataProvider::convertRangeToXML( const OUString& aRangeRepresentation )
{
    XMLRangeHelper::CellRange aRange;
    aRange.aTableName = "local-table";

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings
    if( aRangeRepresentation.match( lcl_aCategoriesRangeName ))
    {
        OSL_ASSERT( aRangeRepresentation == lcl_aCategoriesRangeName );//it is not expected nor implemented that only parts of the categories are really requested
        aRange.aUpperLeft.bIsEmpty = false;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = 0;
            aRange.aUpperLeft.nRow = 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nRow = m_aInternalData.getRowCount();
        }
        else
        {
            aRange.aUpperLeft.nColumn = 1;
            aRange.aUpperLeft.nRow = 0;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nColumn = m_aInternalData.getColumnCount();
        }
    }
    else if( aRangeRepresentation.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRangeRepresentation.copy( strlen(lcl_aLabelRangePrefix)).toInt32();
        aRange.aUpperLeft.bIsEmpty = false;
        aRange.aLowerRight.bIsEmpty = true;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = nIndex + 1;
            aRange.aUpperLeft.nRow = 0;
        }
        else
        {
            aRange.aUpperLeft.nColumn = 0;
            aRange.aUpperLeft.nRow = nIndex + 1;
        }
    }
    else if( aRangeRepresentation == lcl_aCompleteRange )
    {
        aRange.aUpperLeft.bIsEmpty = false;
        aRange.aLowerRight.bIsEmpty = false;
        aRange.aUpperLeft.nColumn = 0;
        aRange.aUpperLeft.nRow = 0;
        aRange.aLowerRight.nColumn = m_aInternalData.getColumnCount();
        aRange.aLowerRight.nRow = m_aInternalData.getRowCount();
    }
    else
    {
        sal_Int32 nIndex = aRangeRepresentation.toInt32();
        aRange.aUpperLeft.bIsEmpty = false;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = nIndex + 1;
            aRange.aUpperLeft.nRow = 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nRow = m_aInternalData.getRowCount();
        }
        else
        {
            aRange.aUpperLeft.nColumn = 1;
            aRange.aUpperLeft.nRow = nIndex + 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nColumn = m_aInternalData.getColumnCount();
        }
    }

    return XMLRangeHelper::getXMLStringFromCellRange( aRange );
}

OUString SAL_CALL InternalDataProvider::convertRangeFromXML( const OUString& aXMLRange )
{
    XMLRangeHelper::CellRange aRange( XMLRangeHelper::getCellRangeFromXMLString( aXMLRange ));
    if( aRange.aUpperLeft.bIsEmpty )
    {
        OSL_ENSURE( aRange.aLowerRight.bIsEmpty, "Weird Range" );
        return OUString();
    }

    // "all"
    if( !aRange.aLowerRight.bIsEmpty &&
        ( aRange.aUpperLeft.nColumn != aRange.aLowerRight.nColumn ) &&
        ( aRange.aUpperLeft.nRow != aRange.aLowerRight.nRow ) )
        return OUString(lcl_aCompleteRange);

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings

    // data in columns
    if( m_bDataInColumns )
    {
        if( aRange.aUpperLeft.nColumn == 0 )
            return OUString(lcl_aCategoriesRangeName);
        if( aRange.aUpperLeft.nRow == 0 )
            return lcl_aLabelRangePrefix + OUString::number( aRange.aUpperLeft.nColumn - 1 );

        return OUString::number( aRange.aUpperLeft.nColumn - 1 );
    }

    // data in rows
    if( aRange.aUpperLeft.nRow == 0 )
        return OUString(lcl_aCategoriesRangeName);
    if( aRange.aUpperLeft.nColumn == 0 )
        return lcl_aLabelRangePrefix + OUString::number( aRange.aUpperLeft.nRow - 1 );

    return OUString::number( aRange.aUpperLeft.nRow - 1 );
}

namespace
{

template< class Type >
Sequence< Sequence< Type > > lcl_convertVectorVectorToSequenceSequence( const vector< vector< Type > >& rIn )
{
    Sequence< Sequence< Type > > aRet;
    sal_Int32 nOuterCount = rIn.size();
    if( nOuterCount )
    {
        aRet.realloc(nOuterCount);
        for( sal_Int32 nN=0; nN<nOuterCount; nN++)
            aRet[nN]= comphelper::containerToSequence( rIn[nN] );
    }
    return aRet;
}

template< class Type >
vector< vector< Type > > lcl_convertSequenceSequenceToVectorVector( const Sequence< Sequence< Type > >& rIn )
{
    vector< vector< Type > > aRet;
    sal_Int32 nOuterCount = rIn.getLength();
    if( nOuterCount )
    {
        aRet.resize(nOuterCount);
        for( sal_Int32 nN=0; nN<nOuterCount; nN++)
            aRet[nN]= ContainerHelper::SequenceToVector( rIn[nN] );
    }
    return aRet;
}

std::vector< Sequence< OUString > > lcl_convertComplexAnyVectorToStringSequence( const vector< vector< uno::Any > >& rIn )
{
    std::vector< Sequence< OUString > > aRet;
    sal_Int32 nOuterCount = rIn.size();
    if( nOuterCount )
    {
        aRet.resize(nOuterCount);
        for( sal_Int32 nN=0; nN<nOuterCount; nN++)
            aRet[nN] = comphelper::containerToSequence(lcl_AnyToStringSequence( rIn[nN] ));
    }
    return aRet;
}

vector< vector< uno::Any > > lcl_convertComplexStringSequenceToAnyVector( const Sequence< Sequence< OUString > >& rIn )
{
    vector< vector< uno::Any > > aRet;
    sal_Int32 nOuterCount = rIn.getLength();
    for( sal_Int32 nN=0; nN<nOuterCount; nN++)
        aRet.push_back( lcl_StringToAnyVector( rIn[nN] ) );
    return aRet;
}

class SplitCategoriesProvider_ForComplexDescriptions : public SplitCategoriesProvider
{
public:

    explicit SplitCategoriesProvider_ForComplexDescriptions( const ::std::vector< ::std::vector< uno::Any > >& rComplexDescriptions )
        : m_rComplexDescriptions( rComplexDescriptions )
    {}

    virtual sal_Int32 getLevelCount() const override;
    virtual uno::Sequence< OUString > getStringsForLevel( sal_Int32 nIndex ) const override;

private:
    const ::std::vector< ::std::vector< uno::Any > >& m_rComplexDescriptions;
};

sal_Int32 SplitCategoriesProvider_ForComplexDescriptions::getLevelCount() const
{
    return lcl_getInnerLevelCount( m_rComplexDescriptions );
}
uno::Sequence< OUString > SplitCategoriesProvider_ForComplexDescriptions::getStringsForLevel( sal_Int32 nLevel ) const
{
    uno::Sequence< OUString > aResult;
    if( nLevel < lcl_getInnerLevelCount( m_rComplexDescriptions ) )
    {
        aResult.realloc( m_rComplexDescriptions.size() );
        transform( m_rComplexDescriptions.begin(), m_rComplexDescriptions.end(),
                   aResult.getArray(), lcl_getStringFromLevelVector(nLevel) );
    }
    return aResult;
}

}//anonymous namespace

// ____ XDateCategories ____
Sequence< double > SAL_CALL InternalDataProvider::getDateCategories()
{
    double fNan = InternalDataProvider::getNotANumber();
    double fValue = fNan;
    vector< vector< uno::Any > > aCategories( m_bDataInColumns ? m_aInternalData.getComplexRowLabels() : m_aInternalData.getComplexColumnLabels());
    sal_Int32 nCount = aCategories.size();
    Sequence< double > aDoubles( nCount );
    vector< vector< uno::Any > >::iterator aIt( aCategories.begin() );
    vector< vector< uno::Any > >::const_iterator aEnd( aCategories.end() );
    for(sal_Int32 nN=0; nN<nCount && aIt!=aEnd; ++nN, ++aIt )
    {
        if( !( !aIt->empty() && ((*aIt)[0]>>=fValue) ) )
            fValue = fNan;
        aDoubles[nN]=fValue;
    }
    return aDoubles;
}

void SAL_CALL InternalDataProvider::setDateCategories( const Sequence< double >& rDates )
{
    sal_Int32 nCount = rDates.getLength();
    vector< vector< uno::Any > > aNewCategories;
    aNewCategories.reserve(nCount);
    vector< uno::Any > aSingleLabel(1);

    for(sal_Int32 nN=0; nN<nCount; ++nN )
    {
        aSingleLabel[0] <<= rDates[nN];
        aNewCategories.push_back(aSingleLabel);
    }

    if( m_bDataInColumns )
        m_aInternalData.setComplexRowLabels( aNewCategories );
    else
        m_aInternalData.setComplexColumnLabels( aNewCategories );
}

// ____ XAnyDescriptionAccess ____
Sequence< Sequence< uno::Any > > SAL_CALL InternalDataProvider::getAnyRowDescriptions()
{
    return lcl_convertVectorVectorToSequenceSequence( m_aInternalData.getComplexRowLabels() );
}
void SAL_CALL InternalDataProvider::setAnyRowDescriptions( const Sequence< Sequence< uno::Any > >& aRowDescriptions )
{
    m_aInternalData.setComplexRowLabels( lcl_convertSequenceSequenceToVectorVector( aRowDescriptions ) );
}
Sequence< Sequence< uno::Any > > SAL_CALL InternalDataProvider::getAnyColumnDescriptions()
{
    return lcl_convertVectorVectorToSequenceSequence( m_aInternalData.getComplexColumnLabels() );
}
void SAL_CALL InternalDataProvider::setAnyColumnDescriptions( const Sequence< Sequence< uno::Any > >& aColumnDescriptions )
{
    m_aInternalData.setComplexColumnLabels( lcl_convertSequenceSequenceToVectorVector( aColumnDescriptions ) );
}

// ____ XComplexDescriptionAccess ____
Sequence< Sequence< OUString > > SAL_CALL InternalDataProvider::getComplexRowDescriptions()
{
    return comphelper::containerToSequence(lcl_convertComplexAnyVectorToStringSequence( m_aInternalData.getComplexRowLabels() ));
}
void SAL_CALL InternalDataProvider::setComplexRowDescriptions( const Sequence< Sequence< OUString > >& aRowDescriptions )
{
    m_aInternalData.setComplexRowLabels( lcl_convertComplexStringSequenceToAnyVector(aRowDescriptions) );
}
Sequence< Sequence< OUString > > SAL_CALL InternalDataProvider::getComplexColumnDescriptions()
{
    return comphelper::containerToSequence(lcl_convertComplexAnyVectorToStringSequence( m_aInternalData.getComplexColumnLabels() ));
}
void SAL_CALL InternalDataProvider::setComplexColumnDescriptions( const Sequence< Sequence< OUString > >& aColumnDescriptions )
{
    m_aInternalData.setComplexColumnLabels( lcl_convertComplexStringSequenceToAnyVector(aColumnDescriptions) );
}

// ____ XChartDataArray ____
Sequence< Sequence< double > > SAL_CALL InternalDataProvider::getData()
{
    return m_aInternalData.getData();
}

void SAL_CALL InternalDataProvider::setData( const Sequence< Sequence< double > >& rDataInRows )
{
    return m_aInternalData.setData( rDataInRows );
}

void SAL_CALL InternalDataProvider::setRowDescriptions( const Sequence< OUString >& aRowDescriptions )
{
    vector< vector< uno::Any > > aComplexDescriptions( aRowDescriptions.getLength() );
    transform( aComplexDescriptions.begin(), aComplexDescriptions.end(), aRowDescriptions.getConstArray(),
               aComplexDescriptions.begin(), lcl_setAnyAtLevelFromStringSequence(0) );
    m_aInternalData.setComplexRowLabels( aComplexDescriptions );
}

void SAL_CALL InternalDataProvider::setColumnDescriptions( const Sequence< OUString >& aColumnDescriptions )
{
    vector< vector< uno::Any > > aComplexDescriptions( aColumnDescriptions.getLength() );
    transform( aComplexDescriptions.begin(), aComplexDescriptions.end(), aColumnDescriptions.getConstArray(),
               aComplexDescriptions.begin(), lcl_setAnyAtLevelFromStringSequence(0) );
    m_aInternalData.setComplexColumnLabels( aComplexDescriptions );
}

Sequence< OUString > SAL_CALL InternalDataProvider::getRowDescriptions()
{
    vector< vector< uno::Any > > aComplexLabels( m_aInternalData.getComplexRowLabels() );
    SplitCategoriesProvider_ForComplexDescriptions aProvider( aComplexLabels );
    return ExplicitCategoriesProvider::getExplicitSimpleCategories( aProvider );
}

Sequence< OUString > SAL_CALL InternalDataProvider::getColumnDescriptions()
{
    vector< vector< uno::Any > > aComplexLabels( m_aInternalData.getComplexColumnLabels() );
    SplitCategoriesProvider_ForComplexDescriptions aProvider( aComplexLabels );
    return ExplicitCategoriesProvider::getExplicitSimpleCategories( aProvider );
}

// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL InternalDataProvider::addChartDataChangeEventListener(
    const Reference< css::chart::XChartDataChangeEventListener >& )
{
}

void SAL_CALL InternalDataProvider::removeChartDataChangeEventListener(
    const Reference< css::chart::XChartDataChangeEventListener >& )
{
}

double SAL_CALL InternalDataProvider::getNotANumber()
{
    double fNan;
    ::rtl::math::setNan( & fNan );
    return fNan;
}

sal_Bool SAL_CALL InternalDataProvider::isNotANumber( double nNumber )
{
    return ::rtl::math::isNan( nNumber )
        || ::rtl::math::isInf( nNumber );
}
// lang::XInitialization:
void SAL_CALL InternalDataProvider::initialize(const uno::Sequence< uno::Any > & _aArguments)
{
    comphelper::SequenceAsHashMap aArgs(_aArguments);
    if ( aArgs.getUnpackedValueOrDefault( "CreateDefaultData", false ) )
            m_aInternalData.createDefaultData();
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL InternalDataProvider::createClone()
{
    return Reference< util::XCloneable >( new InternalDataProvider( *this ));
}

OUString SAL_CALL InternalDataProvider::getImplementationName()
{
    // note: in xmloff this name is used to indicate usage of own data
    return OUString("com.sun.star.comp.chart.InternalDataProvider");
}

sal_Bool SAL_CALL InternalDataProvider::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL InternalDataProvider::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.data.DataProvider" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart_InternalDataProvider_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::InternalDataProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
