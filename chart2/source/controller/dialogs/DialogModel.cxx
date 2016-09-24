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

#include "DialogModel.hxx"
#include "RangeSelectionHelper.hxx"
#include "PropertyHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "DataSourceHelper.hxx"
#include "DiagramHelper.hxx"
#include "macros.hxx"
#include "Strings.hrc"
#include "ResId.hxx"
#include "ContainerHelper.hxx"
#include "CommonFunctors.hxx"
#include "ControllerLockGuard.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <comphelper/sequence.hxx>

#include <rtl/ustring.hxx>

#include <utility>
#include <algorithm>
#include <iterator>
#include <functional>
#include <numeric>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
const OUString lcl_aLabelRole( "label" );

struct lcl_ChartTypeToSeriesCnt : ::std::unary_function<
        Reference< XChartType >, Reference< XDataSeriesContainer > >
{
    Reference< XDataSeriesContainer > operator() (
        const Reference< XChartType > & xChartType )
    {
        return Reference< XDataSeriesContainer >::query( xChartType );
    }
};

OUString lcl_ConvertRole( const OUString & rRoleString, bool bFromInternalToUI )
{
    OUString aResult( rRoleString );

    typedef ::std::map< OUString, OUString > tTranslationMap;
    static tTranslationMap aTranslationMap;

    if( aTranslationMap.empty() )
    {
        aTranslationMap[ "categories" ] =   ::chart::SchResId( STR_DATA_ROLE_CATEGORIES ).toString();
        aTranslationMap[ "error-bars-x" ] = ::chart::SchResId( STR_DATA_ROLE_X_ERROR ).toString();
        aTranslationMap[ "error-bars-x-positive" ] = ::chart::SchResId( STR_DATA_ROLE_X_ERROR_POSITIVE ).toString();
        aTranslationMap[ "error-bars-x-negative" ] = ::chart::SchResId( STR_DATA_ROLE_X_ERROR_NEGATIVE ).toString();
        aTranslationMap[ "error-bars-y" ] = ::chart::SchResId( STR_DATA_ROLE_Y_ERROR ).toString();
        aTranslationMap[ "error-bars-y-positive" ] = ::chart::SchResId( STR_DATA_ROLE_Y_ERROR_POSITIVE ).toString();
        aTranslationMap[ "error-bars-y-negative" ] = ::chart::SchResId( STR_DATA_ROLE_Y_ERROR_NEGATIVE ).toString();
        aTranslationMap[ "label" ] =        ::chart::SchResId( STR_DATA_ROLE_LABEL ).toString();
        aTranslationMap[ "values-first" ] = ::chart::SchResId( STR_DATA_ROLE_FIRST ).toString();
        aTranslationMap[ "values-last" ] =  ::chart::SchResId( STR_DATA_ROLE_LAST ).toString();
        aTranslationMap[ "values-max" ] =   ::chart::SchResId( STR_DATA_ROLE_MAX ).toString();
        aTranslationMap[ "values-min" ] =   ::chart::SchResId( STR_DATA_ROLE_MIN ).toString();
        aTranslationMap[ "values-x" ] =     ::chart::SchResId( STR_DATA_ROLE_X ).toString();
        aTranslationMap[ "values-y" ] =     ::chart::SchResId( STR_DATA_ROLE_Y ).toString();
        aTranslationMap[ "values-size" ] =  ::chart::SchResId( STR_DATA_ROLE_SIZE ).toString();
        aTranslationMap[ "FillColor" ] =    ::chart::SchResId( STR_PROPERTY_ROLE_FILLCOLOR ).toString();
        aTranslationMap[ "BorderColor" ] =  ::chart::SchResId( STR_PROPERTY_ROLE_BORDERCOLOR ).toString();
    }

    if( bFromInternalToUI )
    {
        tTranslationMap::const_iterator aIt( aTranslationMap.find( rRoleString ));
        if( aIt != aTranslationMap.end())
        {
            aResult = (*aIt).second;
        }
    }
    else
    {
        tTranslationMap::const_iterator aIt(
            ::std::find_if( aTranslationMap.begin(), aTranslationMap.end(),
                [&rRoleString]
                ( const tTranslationMap::value_type& cp )
                { return rRoleString == cp.second; } )
            );

        if( aIt != aTranslationMap.end())
            aResult = (*aIt).first;
    }

    return aResult;
}

typedef ::std::map< OUString, sal_Int32 > lcl_tRoleIndexMap;

void lcl_createRoleIndexMap( lcl_tRoleIndexMap & rOutMap )
{
    rOutMap.clear();
    sal_Int32 nIndex = 0;

    rOutMap[ "label" ] =                 ++nIndex;
    rOutMap[ "categories" ] =            ++nIndex;
    rOutMap[ "values-x" ] =              ++nIndex;
    rOutMap[ "values-y" ] =              ++nIndex;
    rOutMap[ "error-bars-x" ] =          ++nIndex;
    rOutMap[ "error-bars-x-positive" ] = ++nIndex;
    rOutMap[ "error-bars-x-negative" ] = ++nIndex;
    rOutMap[ "error-bars-y" ] =          ++nIndex;
    rOutMap[ "error-bars-y-positive" ] = ++nIndex;
    rOutMap[ "error-bars-y-negative" ] = ++nIndex;
    rOutMap[ "values-first" ] =          ++nIndex;
    rOutMap[ "values-min" ] =            ++nIndex;
    rOutMap[ "values-max" ] =            ++nIndex;
    rOutMap[ "values-last" ] =           ++nIndex;
    rOutMap[ "values-size" ] =           ++nIndex;
}

struct lcl_DataSeriesContainerAppend : public
    ::std::iterator< ::std::output_iterator_tag, Reference< XDataSeriesContainer > >
{
    typedef ::std::vector< ::chart::DialogModel::tSeriesWithChartTypeByName > tContainerType;

    explicit lcl_DataSeriesContainerAppend( tContainerType * rCnt )
            : m_rDestCnt( rCnt )
    {}

    lcl_DataSeriesContainerAppend & operator= ( const value_type & xVal )
    {
        try
        {
            if( xVal.is())
            {
                Sequence< Reference< XDataSeries > > aSeq( xVal->getDataSeries());
                OUString aRole( "values-y" );
                Reference< XChartType > xCT( xVal, uno::UNO_QUERY );
                if( xCT.is())
                    aRole = xCT->getRoleOfSequenceForSeriesLabel();
                for( sal_Int32 nI = 0; nI < aSeq.getLength(); ++ nI )
                {
                    m_rDestCnt->push_back(
                        ::chart::DialogModel::tSeriesWithChartTypeByName(
                            ::chart::DataSeriesHelper::getDataSeriesLabel( aSeq[nI], aRole ),
                            ::std::make_pair( aSeq[nI], xCT )));
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
        return *this;
    }

    // Implement output operator requirements as required by std::copy (and
    // implement prefix increment in terms of postfix increment to avoid unused
    // member function warnings for the latter in the common case where
    // std::copy would not actually need it):
    lcl_DataSeriesContainerAppend & operator* ()     { return *this; }
    lcl_DataSeriesContainerAppend & operator++ ()    { return operator++(0); }
    lcl_DataSeriesContainerAppend & operator++ (int) { return *this; }

private:
    tContainerType * m_rDestCnt;
};

struct lcl_RolesWithRangeAppend : public
    ::std::iterator< ::std::output_iterator_tag, Reference< data::XLabeledDataSequence > >
{
    typedef ::chart::DialogModel::tRolesWithRanges tContainerType;

    explicit lcl_RolesWithRangeAppend( tContainerType * rCnt,
                                       const OUString & aLabelRole )
            : m_rDestCnt( rCnt ),
              m_aRoleForLabelSeq( aLabelRole )
    {}

    lcl_RolesWithRangeAppend & operator= ( const value_type & xVal )
    {
        try
        {
            if( xVal.is())
            {
                // data sequence
                Reference< data::XDataSequence > xSeq( xVal->getValues());
                if( xSeq.is())
                {
                    OUString aRole;
                    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY_THROW );
                    if( xProp->getPropertyValue( "Role" ) >>= aRole )
                    {
                        m_rDestCnt->insert(
                            tContainerType::value_type(
                                aRole, xSeq->getSourceRangeRepresentation()));
                        // label
                        if( aRole.equals( m_aRoleForLabelSeq ))
                        {
                            Reference< data::XDataSequence > xLabelSeq( xVal->getLabel());
                            if( xLabelSeq.is())
                            {
                                m_rDestCnt->insert(
                                    tContainerType::value_type(
                                        lcl_aLabelRole, xLabelSeq->getSourceRangeRepresentation()));
                            }
                        }
                    }
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
        return *this;
    }

    // Implement output operator requirements as required by std::copy (and
    // implement prefix increment in terms of postfix increment to avoid unused
    // member function warnings for the latter in the common case where
    // std::copy would not actually need it):
    lcl_RolesWithRangeAppend & operator* ()     { return *this; }
    lcl_RolesWithRangeAppend & operator++ ()    { return operator++(0); }
    lcl_RolesWithRangeAppend & operator++ (int) { return *this; }

private:
    tContainerType * m_rDestCnt;
    OUString m_aRoleForLabelSeq;
};

void lcl_SetSequenceRole(
    const Reference< data::XDataSequence > & xSeq,
    const OUString & rRole )
{
    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY );
    if( xProp.is())
        xProp->setPropertyValue( "Role" , uno::makeAny( rRole ));
}

Reference< XDataSeries > lcl_CreateNewSeries(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< XChartType > & xChartType,
    sal_Int32 nNewSeriesIndex,
    sal_Int32 nTotalNumberOfSeriesInCTGroup,
    const Reference< XDiagram > & xDiagram,
    const Reference< XChartTypeTemplate > & xTemplate,
    bool bCreateDataCachedSequences )
{
    // create plain series
    Reference< XDataSeries > xResult(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.chart2.DataSeries" ,
            xContext ), uno::UNO_QUERY );
    if( xTemplate.is())
    {
        Reference< beans::XPropertySet > xResultProp( xResult, uno::UNO_QUERY );
        if( xResultProp.is())
        {
            // @deprecated: correct default color should be found by view
            // without setting it as hard attribute
            Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
            if( xColorScheme.is())
                xResultProp->setPropertyValue( "Color" , uno::makeAny( xColorScheme->getColorByIndex( nNewSeriesIndex )));
        }
        sal_Int32 nGroupIndex=0;
        if( xChartType.is())
        {
            Sequence< Reference< XChartType > > aCTs(
                ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ));
            for( ; nGroupIndex<aCTs.getLength(); ++nGroupIndex)
                if( aCTs[nGroupIndex] == xChartType )
                    break;
            if( nGroupIndex == aCTs.getLength())
                nGroupIndex = 0;
        }
        xTemplate->applyStyle( xResult, nGroupIndex, nNewSeriesIndex, nTotalNumberOfSeriesInCTGroup );
    }

    if( bCreateDataCachedSequences )
    {
        // set chart type specific roles
        Reference< data::XDataSink > xSink( xResult, uno::UNO_QUERY );
        if( xChartType.is() && xSink.is())
        {
            ::std::vector< Reference< data::XLabeledDataSequence > > aNewSequences;
            const OUString aRoleOfSeqForSeriesLabel = xChartType->getRoleOfSequenceForSeriesLabel();
            const OUString aLabel(::chart::SchResId(STR_DATA_UNNAMED_SERIES).toString());
            const Sequence< OUString > aRoles( xChartType->getSupportedMandatoryRoles());
            const Sequence< OUString > aOptRoles( xChartType->getSupportedOptionalRoles());
            sal_Int32 nI = 0;

            for(nI=0; nI<aRoles.getLength(); ++nI)
            {
                if( aRoles[nI].equals( lcl_aLabelRole ))
                    continue;
                Reference< data::XDataSequence > xSeq( ::chart::DataSourceHelper::createCachedDataSequence() );
                lcl_SetSequenceRole( xSeq, aRoles[nI] );
                // assert that aRoleOfSeqForSeriesLabel is part of the mandatory roles
                if( aRoles[nI].equals( aRoleOfSeqForSeriesLabel ))
                {
                    Reference< data::XDataSequence > xLabel( ::chart::DataSourceHelper::createCachedDataSequence( aLabel ));
                    lcl_SetSequenceRole( xLabel, lcl_aLabelRole );
                    aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq, xLabel ));
                }
                else
                    aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq ));
            }

            for(nI=0; nI<aOptRoles.getLength(); ++nI)
            {
                if( aOptRoles[nI].equals( lcl_aLabelRole ))
                    continue;
                Reference< data::XDataSequence > xSeq( ::chart::DataSourceHelper::createCachedDataSequence());
                lcl_SetSequenceRole( xSeq, aOptRoles[nI] );
                aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq ));
            }

            xSink->setData( comphelper::containerToSequence( aNewSequences ));
        }
    }

    return xResult;
}

struct lcl_addSeriesNumber : public ::std::binary_function<
        sal_Int32, Reference< XDataSeriesContainer >, sal_Int32 >
{
    sal_Int32 operator() ( sal_Int32 nCurrentNumber, const Reference< XDataSeriesContainer > & xCnt ) const
    {
        if( xCnt.is())
            return nCurrentNumber + (xCnt->getDataSeries().getLength());
        return nCurrentNumber;
    }
};

} // anonymous namespace

namespace chart
{

DialogModelTimeBasedInfo::DialogModelTimeBasedInfo():
    bTimeBased(false),
    nStart(0),
    nEnd(0)
{
}

DialogModel::DialogModel(
    const Reference< XChartDocument > & xChartDocument,
    const Reference< uno::XComponentContext > & xContext ) :
        m_xChartDocument( xChartDocument ),
        m_xContext( xContext ),
        m_aTimerTriggeredControllerLock( uno::Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) )
{
}

DialogModel::~DialogModel()
{
    if(maTimeBasedInfo.bTimeBased)
    {
        getModel().setTimeBasedRange(maTimeBasedInfo.nStart, maTimeBasedInfo.nEnd);
    }
}

void DialogModel::setTemplate(
    const Reference< XChartTypeTemplate > & xTemplate )
{
    m_xTemplate = xTemplate;
}

::std::shared_ptr< RangeSelectionHelper > const &
    DialogModel::getRangeSelectionHelper() const
{
    if( ! m_spRangeSelectionHelper.get())
        m_spRangeSelectionHelper.reset(
            new RangeSelectionHelper( m_xChartDocument ));

    return m_spRangeSelectionHelper;
}

Reference< frame::XModel > DialogModel::getChartModel() const
{
    Reference< frame::XModel > xResult( m_xChartDocument, uno::UNO_QUERY );
    return xResult;
}

Reference< data::XDataProvider > DialogModel::getDataProvider() const
{
    Reference< data::XDataProvider > xResult;
    if( m_xChartDocument.is())
        xResult.set( m_xChartDocument->getDataProvider());
    return xResult;
}

::std::vector< Reference< XDataSeriesContainer > >
    DialogModel::getAllDataSeriesContainers() const
{
    ::std::vector< Reference< XDataSeriesContainer > > aResult;

    try
    {
        Reference< XDiagram > xDiagram;
        if( m_xChartDocument.is())
            xDiagram.set( m_xChartDocument->getFirstDiagram());
        if( xDiagram.is())
        {
            Reference< XCoordinateSystemContainer > xCooSysCnt(
                xDiagram, uno::UNO_QUERY_THROW );
            Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCooSysCnt->getCoordinateSystems());
            for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
            {
                Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
                Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
                ::std::transform(
                    aChartTypeSeq.getConstArray(), aChartTypeSeq.getConstArray() + aChartTypeSeq.getLength(),
                    ::std::back_inserter( aResult ),
                    lcl_ChartTypeToSeriesCnt() );
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return aResult;
}

::std::vector< DialogModel::tSeriesWithChartTypeByName >
    DialogModel::getAllDataSeriesWithLabel() const
{
    ::std::vector< tSeriesWithChartTypeByName > aResult;
    ::std::vector< Reference< XDataSeriesContainer > > aContainers(
        getAllDataSeriesContainers());

    ::std::copy( aContainers.begin(), aContainers.end(),
                 lcl_DataSeriesContainerAppend( &aResult ));
    return aResult;
}

namespace {

void addMissingRoles(DialogModel::tRolesWithRanges& rResult, const uno::Sequence<OUString>& rRoles)
{
    for(sal_Int32 i = 0, n = rRoles.getLength(); i < n; ++i)
    {
        if(rResult.find(rRoles[i]) == rResult.end())
            rResult.insert(DialogModel::tRolesWithRanges::value_type(rRoles[i], OUString()));
    }
}

/**
 * Insert a new data series to chart type at position after specified series
 * position.
 *
 * @param xChartType chart type that contains data series.
 * @param xSeries insertion position.  The new series will be inserted after
 *                this one.
 * @param xNewSeries new data series to insert.
 */
void addNewSeriesToContainer(
    const Reference<XChartType>& xChartType,
    const Reference<XDataSeries>& xSeries,
    const Reference<XDataSeries>& xNewSeries )
{
    Reference<XDataSeriesContainer> xSeriesCnt(xChartType, uno::UNO_QUERY_THROW);
    std::vector<Reference<XDataSeries> > aSeries = ContainerHelper::SequenceToVector(xSeriesCnt->getDataSeries());

    std::vector<Reference<XDataSeries> >::iterator aIt =
        std::find( aSeries.begin(), aSeries.end(), xSeries);

    if( aIt == aSeries.end())
        // if we have no series we insert at the first position.
        aIt = aSeries.begin();
    else
        // vector::insert inserts before, so we have to advance
        ++aIt;

    aSeries.insert(aIt, xNewSeries);
    xSeriesCnt->setDataSeries(comphelper::containerToSequence(aSeries));
}

}

DialogModel::tRolesWithRanges DialogModel::getRolesWithRanges(
    const Reference< XDataSeries > & xSeries,
    const OUString & aRoleOfSequenceForLabel,
    const Reference< chart2::XChartType > & xChartType )
{
    DialogModel::tRolesWithRanges aResult;
    try
    {
        Reference< data::XDataSource > xSource( xSeries, uno::UNO_QUERY_THROW );
        const Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSource->getDataSequences());
        ::std::copy( aSeq.begin(), aSeq.end(),
                     lcl_RolesWithRangeAppend( &aResult, aRoleOfSequenceForLabel ));
        if( xChartType.is())
        {
            // add missing mandatory roles
            Sequence< OUString > aRoles( xChartType->getSupportedMandatoryRoles());
            addMissingRoles(aResult, aRoles);

            // add missing optional roles
            aRoles = xChartType->getSupportedOptionalRoles();
            addMissingRoles(aResult, aRoles);

            // add missing property roles
            aRoles = xChartType->getSupportedPropertyRoles();
            addMissingRoles(aResult, aRoles);
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aResult;
}

void DialogModel::moveSeries(
    const Reference< XDataSeries > & xSeries,
    eMoveDirection eDirection )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

    Reference< XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
    DiagramHelper::moveSeries( xDiagram, xSeries, eDirection==MOVE_UP );
}

Reference< chart2::XDataSeries > DialogModel::insertSeriesAfter(
    const Reference< XDataSeries > & xSeries,
    const Reference< XChartType > & xChartType,
    bool bCreateDataCachedSequences /* = false */ )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );
    Reference< XDataSeries > xNewSeries;

    try
    {
        Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram() );
        ThreeDLookScheme e3DScheme = ThreeDHelper::detectScheme( xDiagram );

        sal_Int32 nSeriesInChartType = 0;
        const sal_Int32 nTotalSeries = countSeries();
        if( xChartType.is())
        {
            Reference< XDataSeriesContainer > xCnt( xChartType, uno::UNO_QUERY_THROW );
            nSeriesInChartType = xCnt->getDataSeries().getLength();
        }

        // create new series
        xNewSeries.set(
            lcl_CreateNewSeries(
                m_xContext,
                xChartType,
                nTotalSeries, // new series' index
                nSeriesInChartType,
                xDiagram,
                m_xTemplate,
                bCreateDataCachedSequences ));

        // add new series to container
        if( xNewSeries.is())
            addNewSeriesToContainer(xChartType, xSeries, xNewSeries);

        ThreeDHelper::setScheme( xDiagram, e3DScheme );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xNewSeries;
}

void DialogModel::deleteSeries(
    const Reference< XDataSeries > & xSeries,
    const Reference< XChartType > & xChartType )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

    DataSeriesHelper::deleteSeries( xSeries, xChartType );
}

Reference< data::XLabeledDataSequence > DialogModel::getCategories() const
{
    Reference< data::XLabeledDataSequence > xResult;
    try
    {
        if( m_xChartDocument.is())
        {
            Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
            xResult.set( DiagramHelper::getCategoriesFromDiagram( xDiagram ));
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xResult;
}

void DialogModel::setCategories( const Reference< chart2::data::XLabeledDataSequence > & xCategories )
{
    if( m_xChartDocument.is())
    {
        Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
        if( xDiagram.is())
        {
            // categories
            bool bSupportsCategories = true;

            Reference< XChartType > xFirstChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
            if( xFirstChartType.is() )
            {
                sal_Int32 nAxisType = ChartTypeHelper::getAxisType( xFirstChartType, 0 ); // x-axis
                bSupportsCategories = (nAxisType == AxisType::CATEGORY);
            }
            DiagramHelper::setCategoriesToDiagram( xCategories, xDiagram, true, bSupportsCategories );
        }
    }
}

OUString DialogModel::getCategoriesRange() const
{
    Reference< data::XLabeledDataSequence > xLSeq( getCategories());
    OUString aRange;
    if( xLSeq.is())
    {
        Reference< data::XDataSequence > xSeq( xLSeq->getValues());
        if( xSeq.is())
            aRange = xSeq->getSourceRangeRepresentation();
    }
    return aRange;
}

bool DialogModel::isCategoryDiagram() const
{
    bool bRet = false;
    if( m_xChartDocument.is())
        bRet = DiagramHelper::isCategoryDiagram( m_xChartDocument->getFirstDiagram() );
    return bRet;
}

void DialogModel::detectArguments(
    OUString & rOutRangeString,
    bool & rOutUseColumns,
    bool & rOutFirstCellAsLabel,
    bool & rOutHasCategories ) const
{
    try
    {
        uno::Sequence< sal_Int32 > aSequenceMapping;//todo YYYX

        // Note: unused data is currently not supported in being passed to detectRangeSegmentation
        if( m_xChartDocument.is())
            DataSourceHelper::detectRangeSegmentation(
                Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY_THROW ),
                rOutRangeString, aSequenceMapping, rOutUseColumns, rOutFirstCellAsLabel, rOutHasCategories );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

bool DialogModel::allArgumentsForRectRangeDetected() const
{
    return DataSourceHelper::allArgumentsForRectRangeDetected( m_xChartDocument );
}

void DialogModel::startControllerLockTimer()
{
    m_aTimerTriggeredControllerLock.startTimer();
}

void DialogModel::setData(
    const Sequence< beans::PropertyValue > & rArguments )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

    Reference< data::XDataProvider > xDataProvider( getDataProvider());
    if( ! xDataProvider.is() ||
        ! m_xTemplate.is() )
    {
        OSL_FAIL( "Model objects missing" );
        return;
    }

    try
    {
        Reference< chart2::data::XDataSource > xDataSource(
            xDataProvider->createDataSource( rArguments ) );

        Reference< chart2::XDataInterpreter > xInterpreter(
            m_xTemplate->getDataInterpreter());
        if( xInterpreter.is())
        {
            Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram() );
            ThreeDLookScheme e3DScheme = ThreeDHelper::detectScheme( xDiagram );

            ::std::vector< Reference< XDataSeries > > aSeriesToReUse(
                DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
            applyInterpretedData(
                xInterpreter->interpretDataSource(
                    xDataSource, rArguments,
                    comphelper::containerToSequence( aSeriesToReUse )),
                aSeriesToReUse);

            ThreeDHelper::setScheme( xDiagram, e3DScheme );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void DialogModel::setTimeBasedRange( bool bTimeBased, sal_Int32 nStart, sal_Int32 nEnd) const
{
    maTimeBasedInfo.nStart = nStart;
    maTimeBasedInfo.nEnd = nEnd;
    maTimeBasedInfo.bTimeBased = bTimeBased;
}

OUString DialogModel::ConvertRoleFromInternalToUI( const OUString & rRoleString )
{
    return lcl_ConvertRole( rRoleString, true );
}

OUString DialogModel::GetRoleDataLabel()
{
    return ::chart::SchResId(STR_OBJECT_DATALABELS).toString();
}

sal_Int32 DialogModel::GetRoleIndexForSorting( const OUString & rInternalRoleString )
{
    static lcl_tRoleIndexMap aRoleIndexMap;

    if( aRoleIndexMap.empty())
        lcl_createRoleIndexMap( aRoleIndexMap );

    lcl_tRoleIndexMap::const_iterator aIt( aRoleIndexMap.find( rInternalRoleString ));
    if( aIt != aRoleIndexMap.end())
        return aIt->second;

    return 0;
}

// private methods

void DialogModel::applyInterpretedData(
    const InterpretedData & rNewData,
    const ::std::vector< Reference< XDataSeries > > & rSeriesToReUse )
{
    if( ! m_xChartDocument.is())
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    Reference< XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
    if( xDiagram.is())
    {
        // styles
        if( m_xTemplate.is() )
        {
            sal_Int32 nGroup = 0;
            sal_Int32 nSeriesCounter = 0;
            sal_Int32 nNewSeriesIndex = static_cast< sal_Int32 >( rSeriesToReUse.size());
            const sal_Int32 nOuterSize=rNewData.Series.getLength();

            for(; nGroup < nOuterSize; ++nGroup)
            {
                Sequence< Reference< XDataSeries > > aSeries( rNewData.Series[ nGroup ] );
                const sal_Int32 nSeriesInGroup = aSeries.getLength();
                for( sal_Int32 nSeries=0; nSeries<nSeriesInGroup; ++nSeries, ++nSeriesCounter )
                {
                    if( ::std::find( rSeriesToReUse.begin(), rSeriesToReUse.end(), aSeries[nSeries] )
                        == rSeriesToReUse.end())
                    {
                        Reference< beans::XPropertySet > xSeriesProp( aSeries[nSeries], uno::UNO_QUERY );
                        if( xSeriesProp.is())
                        {
                            // @deprecated: correct default color should be found by view
                            // without setting it as hard attribute
                            Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
                            if( xColorScheme.is())
                                xSeriesProp->setPropertyValue( "Color" ,
                                    uno::makeAny( xColorScheme->getColorByIndex( nSeriesCounter )));
                        }
                        m_xTemplate->applyStyle( aSeries[nSeries], nGroup, nNewSeriesIndex++, nSeriesInGroup );
                    }
                }
            }
        }

        // data series
        ::std::vector< Reference< XDataSeriesContainer > > aSeriesCnt( getAllDataSeriesContainers());
        ::std::vector< Sequence< Reference< XDataSeries > > > aNewSeries(
            ContainerHelper::SequenceToVector( rNewData.Series ));

        OSL_ASSERT( aSeriesCnt.size() == aNewSeries.size());

        ::std::vector< Sequence< Reference< XDataSeries > > >::const_iterator aSrcIt( aNewSeries.begin());
        ::std::vector< Reference< XDataSeriesContainer > >::iterator aDestIt( aSeriesCnt.begin());
        for(; aSrcIt != aNewSeries.end() && aDestIt != aSeriesCnt.end();
            ++aSrcIt, ++aDestIt )
        {
            try
            {
                OSL_ASSERT( (*aDestIt).is());
                (*aDestIt)->setDataSeries( *aSrcIt );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }

        DialogModel::setCategories(rNewData.Categories);
    }
}

sal_Int32 DialogModel::countSeries() const
{
    ::std::vector< Reference< XDataSeriesContainer > > aCnt( getAllDataSeriesContainers());
    return ::std::accumulate( aCnt.begin(), aCnt.end(), 0, lcl_addSeriesNumber());
}

ChartModel& DialogModel::getModel() const
{
    uno::Reference< frame::XModel > xModel = getChartModel();
    ChartModel* pModel = dynamic_cast<ChartModel*>(xModel.get());
    return *pModel;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
