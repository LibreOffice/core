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
#include <RangeSelectionHelper.hxx>
#include <DataInterpreter.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <DataSourceHelper.hxx>
#include <Diagram.hxx>
#include <strings.hrc>
#include <ResId.hxx>
#include <ControllerLockGuard.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartTypeTemplate.hxx>
#include <ThreeDHelper.hxx>
#include <ChartModel.hxx>
#include <BaseCoordinateSystem.hxx>
#include <LabeledDataSequence.hxx>

#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <rtl/ustring.hxx>

#include <utility>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
constexpr OUString lcl_aLabelRole( u"label"_ustr );


OUString lcl_ConvertRole( const OUString & rRoleString )
{
    OUString aResult( rRoleString );

    typedef std::map< OUString, OUString > tTranslationMap;
    static const tTranslationMap aTranslationMap =
    {
        { "categories", ::chart::SchResId( STR_DATA_ROLE_CATEGORIES ) },
        { "error-bars-x", ::chart::SchResId( STR_DATA_ROLE_X_ERROR ) },
        { "error-bars-x-positive", ::chart::SchResId( STR_DATA_ROLE_X_ERROR_POSITIVE ) },
        { "error-bars-x-negative", ::chart::SchResId( STR_DATA_ROLE_X_ERROR_NEGATIVE ) },
        { "error-bars-y", ::chart::SchResId( STR_DATA_ROLE_Y_ERROR ) },
        { "error-bars-y-positive", ::chart::SchResId( STR_DATA_ROLE_Y_ERROR_POSITIVE ) },
        { "error-bars-y-negative", ::chart::SchResId( STR_DATA_ROLE_Y_ERROR_NEGATIVE ) },
        { "label",        ::chart::SchResId( STR_DATA_ROLE_LABEL ) },
        { "values-first", ::chart::SchResId( STR_DATA_ROLE_FIRST ) },
        { "values-last",  ::chart::SchResId( STR_DATA_ROLE_LAST ) },
        { "values-max",   ::chart::SchResId( STR_DATA_ROLE_MAX ) },
        { "values-min",   ::chart::SchResId( STR_DATA_ROLE_MIN ) },
        { "values-x",     ::chart::SchResId( STR_DATA_ROLE_X ) },
        { "values-y",     ::chart::SchResId( STR_DATA_ROLE_Y ) },
        { "values-size",  ::chart::SchResId( STR_DATA_ROLE_SIZE ) },
        { "FillColor",    ::chart::SchResId( STR_PROPERTY_ROLE_FILLCOLOR ) },
        { "BorderColor",  ::chart::SchResId( STR_PROPERTY_ROLE_BORDERCOLOR ) },
    };

    tTranslationMap::const_iterator aIt( aTranslationMap.find( rRoleString ));
    if( aIt != aTranslationMap.end())
    {
        aResult = (*aIt).second;
    }
    return aResult;
}

typedef std::map< OUString, sal_Int32 > lcl_tRoleIndexMap;

lcl_tRoleIndexMap lcl_createRoleIndexMap()
{
    lcl_tRoleIndexMap aMap;
    sal_Int32 nIndex = 0;

    aMap[ u"label"_ustr ] =                 ++nIndex;
    aMap[ u"categories"_ustr ] =            ++nIndex;
    aMap[ u"values-x"_ustr ] =              ++nIndex;
    aMap[ u"values-y"_ustr ] =              ++nIndex;
    aMap[ u"error-bars-x"_ustr ] =          ++nIndex;
    aMap[ u"error-bars-x-positive"_ustr ] = ++nIndex;
    aMap[ u"error-bars-x-negative"_ustr ] = ++nIndex;
    aMap[ u"error-bars-y"_ustr ] =          ++nIndex;
    aMap[ u"error-bars-y-positive"_ustr ] = ++nIndex;
    aMap[ u"error-bars-y-negative"_ustr ] = ++nIndex;
    aMap[ u"values-first"_ustr ] =          ++nIndex;
    aMap[ u"values-min"_ustr ] =            ++nIndex;
    aMap[ u"values-max"_ustr ] =            ++nIndex;
    aMap[ u"values-last"_ustr ] =           ++nIndex;
    aMap[ u"values-size"_ustr ] =           ++nIndex;

    return aMap;
}


struct lcl_RolesWithRangeAppend
{
    typedef Reference< data::XLabeledDataSequence > value_type;
    typedef ::chart::DialogModel::tRolesWithRanges tContainerType;

    explicit lcl_RolesWithRangeAppend( tContainerType * rCnt,
                                       OUString aLabelRole )
            : m_rDestCnt( rCnt ),
              m_aRoleForLabelSeq(std::move( aLabelRole ))
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
                    if( xProp->getPropertyValue( u"Role"_ustr ) >>= aRole )
                    {
                        m_rDestCnt->emplace(aRole, xSeq->getSourceRangeRepresentation());
                        // label
                        if( aRole == m_aRoleForLabelSeq )
                        {
                            Reference< data::XDataSequence > xLabelSeq( xVal->getLabel());
                            if( xLabelSeq.is())
                            {
                                m_rDestCnt->emplace(
                                        lcl_aLabelRole, xLabelSeq->getSourceRangeRepresentation());
                            }
                        }
                    }
                }
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
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

}

namespace std
{
    template<> struct iterator_traits<lcl_RolesWithRangeAppend>
    {
        typedef std::output_iterator_tag iterator_category;
        typedef Reference< data::XLabeledDataSequence > value_type;
        typedef value_type& reference;
    };
}

namespace {

void lcl_SetSequenceRole(
    const Reference< data::XDataSequence > & xSeq,
    const OUString & rRole )
{
    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY );
    if( xProp.is())
        xProp->setPropertyValue( u"Role"_ustr , uno::Any( rRole ));
}

Sequence< OUString > lcl_CopyExcludingValuesFirst(
    Sequence< OUString > const & i_aInput )
{
    Sequence< OUString > aOutput( i_aInput.getLength());
    auto pOutput = aOutput.getArray();
    int nSourceIndex, nDestIndex;
    for( nSourceIndex = nDestIndex = 0; nSourceIndex < i_aInput.getLength(); nSourceIndex++ )
    {
        if( i_aInput[nSourceIndex] == "values-first" )
        {
            aOutput.realloc( aOutput.getLength() - 1 );
            pOutput = aOutput.getArray();
        }
        else
        {
            pOutput[nDestIndex] = i_aInput[nSourceIndex];
            nDestIndex++;
        }
    }
    return aOutput;
}

rtl::Reference< ::chart::DataSeries > lcl_CreateNewSeries(
    const rtl::Reference< ::chart::ChartType > & xChartType,
    sal_Int32 nNewSeriesIndex,
    sal_Int32 nTotalNumberOfSeriesInCTGroup,
    const rtl::Reference< ::chart::Diagram > & xDiagram,
    const rtl::Reference< ::chart::ChartTypeTemplate > & xTemplate,
    bool bCreateDataCachedSequences )
{
    // create plain series
    rtl::Reference< ::chart::DataSeries > xResult = new ::chart::DataSeries();
    if( xTemplate.is())
    {
        // @deprecated: correct default color should be found by view
        // without setting it as hard attribute
        Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
        if( xColorScheme.is())
            xResult->setPropertyValue( u"Color"_ustr , uno::Any( xColorScheme->getColorByIndex( nNewSeriesIndex )));
        std::size_t nGroupIndex=0;
        if( xChartType.is())
        {
            std::vector< rtl::Reference< ::chart::ChartType > > aCTs =
                xDiagram->getChartTypes();
            for( ; nGroupIndex < aCTs.size(); ++nGroupIndex)
                if( aCTs[nGroupIndex] == xChartType )
                    break;
            if( nGroupIndex == aCTs.size())
                nGroupIndex = 0;
        }
        xTemplate->applyStyle2( xResult, nGroupIndex, nNewSeriesIndex, nTotalNumberOfSeriesInCTGroup );
    }

    if( bCreateDataCachedSequences )
    {
        // set chart type specific roles
        if( xChartType.is() )
        {
            std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aNewSequences;
            const OUString aRoleOfSeqForSeriesLabel = xChartType->getRoleOfSequenceForSeriesLabel();
            const OUString aLabel(::chart::SchResId(STR_DATA_UNNAMED_SERIES));
            Sequence< OUString > aPossibleRoles( xChartType->getSupportedMandatoryRoles());
            Sequence< OUString > aPossibleOptRoles( xChartType->getSupportedOptionalRoles());

            //special handling for candlestick type
            if( xTemplate.is())
            {
                rtl::Reference< ::chart::DataInterpreter > xInterpreter( xTemplate->getDataInterpreter2());
                if( xInterpreter.is())
                {
                    sal_Int32 nStockVariant;
                    if( xInterpreter->getChartTypeSpecificData(u"stock variant"_ustr) >>= nStockVariant )
                    {
                        if( nStockVariant == 0 || nStockVariant == 2) {
                            //delete "values-first" role
                            aPossibleRoles = lcl_CopyExcludingValuesFirst(aPossibleRoles);
                            aPossibleOptRoles = lcl_CopyExcludingValuesFirst(aPossibleOptRoles);
                        }
                    }
                }
            }

            const Sequence< OUString > aRoles( aPossibleRoles );
            const Sequence< OUString > aOptRoles( aPossibleOptRoles );

            for(OUString const & role : aRoles)
            {
                if( role == lcl_aLabelRole )
                    continue;
                Reference< data::XDataSequence > xSeq( ::chart::DataSourceHelper::createCachedDataSequence() );
                lcl_SetSequenceRole( xSeq, role );
                // assert that aRoleOfSeqForSeriesLabel is part of the mandatory roles
                if( role == aRoleOfSeqForSeriesLabel )
                {
                    Reference< data::XDataSequence > xLabel( ::chart::DataSourceHelper::createCachedDataSequence( aLabel ));
                    lcl_SetSequenceRole( xLabel, lcl_aLabelRole );
                    aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq, xLabel ));
                }
                else
                    aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq ));
            }

            for(OUString const & role : aOptRoles)
            {
                if( role == lcl_aLabelRole )
                    continue;
                Reference< data::XDataSequence > xSeq( ::chart::DataSourceHelper::createCachedDataSequence());
                lcl_SetSequenceRole( xSeq, role );
                aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq ));
            }

            xResult->setData( aNewSequences );
        }
    }

    return xResult;
}

struct lcl_addSeriesNumber
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
    rtl::Reference<::chart::ChartModel> xChartDocument ) :
        m_xChartDocument(std::move( xChartDocument )),
        m_aTimerTriggeredControllerLock( m_xChartDocument )
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
    const rtl::Reference< ChartTypeTemplate > & xTemplate )
{
    m_xTemplate = xTemplate;
}

std::shared_ptr< RangeSelectionHelper > const &
    DialogModel::getRangeSelectionHelper() const
{
    if( ! m_spRangeSelectionHelper)
        m_spRangeSelectionHelper =
            std::make_shared<RangeSelectionHelper>( m_xChartDocument );

    return m_spRangeSelectionHelper;
}

const rtl::Reference<::chart::ChartModel> & DialogModel::getChartModel() const
{
    return m_xChartDocument;
}

Reference< data::XDataProvider > DialogModel::getDataProvider() const
{
    Reference< data::XDataProvider > xResult;
    if( m_xChartDocument.is())
        xResult.set( m_xChartDocument->getDataProvider());
    return xResult;
}

std::vector< rtl::Reference< ChartType > >
    DialogModel::getAllDataSeriesContainers() const
{
    std::vector< rtl::Reference< ChartType > > aResult;

    try
    {
        if( !m_xChartDocument )
            return {};
        rtl::Reference< Diagram > xDiagram = m_xChartDocument->getFirstChartDiagram();
        if( xDiagram.is())
        {
            const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysSeq(
                xDiagram->getBaseCoordinateSystems());
            for( rtl::Reference< BaseCoordinateSystem > const & coords : aCooSysSeq )
            {

                for (const auto & rxChartType : coords->getChartTypes2())
                    aResult.push_back(rxChartType);
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return aResult;
}

std::vector< DialogModel::tSeriesWithChartTypeByName >
    DialogModel::getAllDataSeriesWithLabel() const
{
    std::vector< tSeriesWithChartTypeByName > aResult;
    std::vector< rtl::Reference< ChartType > > aContainers(
        getAllDataSeriesContainers());

    for (const auto & rxChartType : aContainers )
    {
        try
        {
            const std::vector< rtl::Reference< DataSeries > > & aSeq = rxChartType->getDataSeries2();
            OUString aRole = rxChartType->getRoleOfSequenceForSeriesLabel();
            for( rtl::Reference< DataSeries > const & dataSeries : aSeq )
            {
                aResult.push_back(
                    ::chart::DialogModel::tSeriesWithChartTypeByName(
                        dataSeries->getLabelForRole( aRole ),
                        std::make_pair( dataSeries, rxChartType )));
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return aResult;
}

namespace {

void addMissingRoles(DialogModel::tRolesWithRanges& rResult, const uno::Sequence<OUString>& rRoles)
{
    for(OUString const & role : rRoles)
    {
        if(rResult.find(role) == rResult.end())
            rResult.emplace(role, OUString());
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
    const rtl::Reference<ChartType>& xChartType,
    const rtl::Reference<DataSeries>& xSeries,
    const rtl::Reference<DataSeries>& xNewSeries )
{
    auto aSeries = xChartType->getDataSeries2();

    auto aIt = std::find( aSeries.begin(), aSeries.end(), xSeries);

    if( aIt == aSeries.end())
        // if we have no series we insert at the first position.
        aIt = aSeries.begin();
    else
        // vector::insert inserts before, so we have to advance
        ++aIt;

    aSeries.insert(aIt, xNewSeries);
    xChartType->setDataSeries(aSeries);
}

}

DialogModel::tRolesWithRanges DialogModel::getRolesWithRanges(
    const Reference< XDataSeries > & xSeries,
    const OUString & aRoleOfSequenceForLabel,
    const rtl::Reference< ::chart::ChartType > & xChartType )
{
    DialogModel::tRolesWithRanges aResult;
    try
    {
        Reference< data::XDataSource > xSource( xSeries, uno::UNO_QUERY_THROW );
        const Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSource->getDataSequences());
        std::copy( aSeq.begin(), aSeq.end(),
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
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aResult;
}

void DialogModel::moveSeries(
    const rtl::Reference< DataSeries > & xSeries,
    MoveDirection eDirection )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( m_xChartDocument );

    rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram());
    xDiagram->moveSeries( xSeries, eDirection==MoveDirection::Down );
}

rtl::Reference< ::chart::DataSeries > DialogModel::insertSeriesAfter(
    const Reference< XDataSeries > & xUnoSeries,
    const rtl::Reference< ::chart::ChartType > & xChartType,
    bool bCreateDataCachedSequences /* = false */ )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( m_xChartDocument );
    rtl::Reference< ::chart::DataSeries > xNewSeries;
    rtl::Reference<DataSeries> xSeries = dynamic_cast<DataSeries*>(xUnoSeries.get());
    assert(xSeries || !xUnoSeries);

    try
    {
        rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram() );
        ThreeDLookScheme e3DScheme = xDiagram->detectScheme();

        sal_Int32 nSeriesInChartType = 0;
        const sal_Int32 nTotalSeries = countSeries();
        if( xChartType.is())
        {
            nSeriesInChartType = xChartType->getDataSeries().getLength();
        }

        // create new series
        xNewSeries =
            lcl_CreateNewSeries(
                xChartType,
                nTotalSeries, // new series' index
                nSeriesInChartType,
                xDiagram,
                m_xTemplate,
                bCreateDataCachedSequences );

        // add new series to container
        if( xNewSeries.is())
            addNewSeriesToContainer(xChartType, xSeries, xNewSeries);

        xDiagram->setScheme( e3DScheme );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return xNewSeries;
}

void DialogModel::deleteSeries(
    const rtl::Reference< DataSeries > & xSeries,
    const rtl::Reference< ChartType > & xChartType )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( m_xChartDocument );

    DataSeriesHelper::deleteSeries( xSeries, xChartType );
}

uno::Reference< chart2::data::XLabeledDataSequence > DialogModel::getCategories() const
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;
    try
    {
        if( m_xChartDocument.is())
        {
            rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram());
            if (xDiagram.is())
                xResult = xDiagram->getCategories();
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return xResult;
}

void DialogModel::setCategories( const Reference< chart2::data::XLabeledDataSequence > & xCategories )
{
    if( !m_xChartDocument.is())
        return;

    rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram());
    if( !xDiagram.is())
        return;

    // categories
    bool bSupportsCategories = true;

    rtl::Reference< ChartType > xFirstChartType( xDiagram->getChartTypeByIndex( 0 ) );
    if( xFirstChartType.is() )
    {
        sal_Int32 nAxisType = ChartTypeHelper::getAxisType( xFirstChartType, 0 ); // x-axis
        bSupportsCategories = (nAxisType == AxisType::CATEGORY);
    }
    xDiagram->setCategories( xCategories, true, bSupportsCategories );
}

OUString DialogModel::getCategoriesRange() const
{
    OUString aRange;
    try
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLSeq( getCategories());
        if( xLSeq.is())
        {
            Reference< data::XDataSequence > xSeq( xLSeq->getValues());
            if( xSeq.is())
                aRange = xSeq->getSourceRangeRepresentation();
        }
    }
    catch (const lang::DisposedException&)
    {
        TOOLS_WARN_EXCEPTION( "chart2", "unexpected exception caught" );
    }
    return aRange;
}

bool DialogModel::isCategoryDiagram() const
{
    bool bRet = false;
    if( m_xChartDocument.is() && m_xChartDocument->getFirstChartDiagram())
        bRet = m_xChartDocument->getFirstChartDiagram()->isCategory();
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
        {
            (void)DataSourceHelper::detectRangeSegmentation(
                m_xChartDocument,
                rOutRangeString, aSequenceMapping, rOutUseColumns, rOutFirstCellAsLabel, rOutHasCategories );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
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
    ControllerLockGuardUNO aLockedControllers( m_xChartDocument );

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

        rtl::Reference< ::chart::DataInterpreter > xInterpreter(
            m_xTemplate->getDataInterpreter2());
        if( xInterpreter.is())
        {
            rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram() );
            ThreeDLookScheme e3DScheme = xDiagram->detectScheme();

            std::vector< rtl::Reference< DataSeries > > aSeriesToReUse =
                xDiagram->getDataSeries();
            applyInterpretedData(
                xInterpreter->interpretDataSource(
                    xDataSource, rArguments,
                    aSeriesToReUse ),
                aSeriesToReUse);

            xDiagram->setScheme( e3DScheme );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
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
    return lcl_ConvertRole( rRoleString );
}

OUString DialogModel::GetRoleDataLabel()
{
    return ::chart::SchResId(STR_OBJECT_DATALABELS);
}

sal_Int32 DialogModel::GetRoleIndexForSorting( const OUString & rInternalRoleString )
{
    static lcl_tRoleIndexMap aRoleIndexMap = lcl_createRoleIndexMap();

    lcl_tRoleIndexMap::const_iterator aIt( aRoleIndexMap.find( rInternalRoleString ));
    if( aIt != aRoleIndexMap.end())
        return aIt->second;

    return 0;
}

// private methods

void DialogModel::applyInterpretedData(
    const InterpretedData & rNewData,
    const std::vector< rtl::Reference< DataSeries > > & rSeriesToReUse )
{
    if( ! m_xChartDocument.is())
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram());
    if( !xDiagram.is())
        return;

    // styles
    if( m_xTemplate.is() )
    {
        sal_Int32 nGroup = 0;
        sal_Int32 nSeriesCounter = 0;
        sal_Int32 nNewSeriesIndex = static_cast< sal_Int32 >( rSeriesToReUse.size());
        const sal_Int32 nOuterSize=rNewData.Series.size();

        for(; nGroup < nOuterSize; ++nGroup)
        {
            const std::vector< rtl::Reference< DataSeries > > & aSeries( rNewData.Series[ nGroup ] );
            const sal_Int32 nSeriesInGroup = aSeries.size();
            for( sal_Int32 nSeries=0; nSeries<nSeriesInGroup; ++nSeries, ++nSeriesCounter )
            {
                if( std::find( rSeriesToReUse.begin(), rSeriesToReUse.end(), aSeries[nSeries] )
                    == rSeriesToReUse.end())
                {
                    if( aSeries[nSeries].is())
                    {
                        // @deprecated: correct default color should be found by view
                        // without setting it as hard attribute
                        Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
                        if( xColorScheme.is())
                            aSeries[nSeries]->setPropertyValue( u"Color"_ustr ,
                                uno::Any( xColorScheme->getColorByIndex( nSeriesCounter )));
                    }
                    m_xTemplate->applyStyle2( aSeries[nSeries], nGroup, nNewSeriesIndex++, nSeriesInGroup );
                }
            }
        }
    }

    // data series
    std::vector< rtl::Reference< ChartType > > aSeriesCnt = getAllDataSeriesContainers();

    OSL_ASSERT( aSeriesCnt.size() == rNewData.Series.size());

    auto aSrcIt = rNewData.Series.begin();
    auto aDestIt = aSeriesCnt.begin();
    for(; aSrcIt != rNewData.Series.end() && aDestIt != aSeriesCnt.end();
        ++aSrcIt, ++aDestIt )
    {
        try
        {
            OSL_ASSERT( (*aDestIt).is());
            (*aDestIt)->setDataSeries( *aSrcIt );
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    DialogModel::setCategories(rNewData.Categories);
}

sal_Int32 DialogModel::countSeries() const
{
    std::vector< rtl::Reference< ChartType > > aCnt( getAllDataSeriesContainers());
    return std::accumulate( aCnt.begin(), aCnt.end(), 0, lcl_addSeriesNumber());
}

ChartModel& DialogModel::getModel() const
{
    return *m_xChartDocument;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
