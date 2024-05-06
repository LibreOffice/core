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

#include <RangeSelectionHelper.hxx>
#include <RangeSelectionListener.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <ChartModel.hxx>
#include <utility>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

RangeSelectionHelper::RangeSelectionHelper(
    rtl::Reference<::chart::ChartModel> xChartDocument ) :
        m_xChartDocument(std::move( xChartDocument ))
{}

RangeSelectionHelper::~RangeSelectionHelper()
{}

bool RangeSelectionHelper::hasRangeSelection()
{
    return getRangeSelection().is();
}

Reference< sheet::XRangeSelection > const & RangeSelectionHelper::getRangeSelection()
{
    if( !m_xRangeSelection.is() &&
        m_xChartDocument.is() )
    {
        try
        {
            Reference< chart2::data::XDataProvider > xDataProvider( m_xChartDocument->getDataProvider());
            if( xDataProvider.is())
                m_xRangeSelection.set( xDataProvider->getRangeSelection());
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");

            m_xRangeSelection.clear();
        }
    }

    return m_xRangeSelection;
}

void RangeSelectionHelper::raiseRangeSelectionDocument()
{
    Reference< sheet::XRangeSelection > xRangeSel( getRangeSelection());
    if( !xRangeSel.is())
        return;

    try
    {
        // bring document to front
        Reference< frame::XController > xCtrl( xRangeSel, uno::UNO_QUERY );
        if( xCtrl.is())
        {
            Reference< frame::XFrame > xFrame( xCtrl->getFrame());
            if( xFrame.is())
            {
                Reference< awt::XTopWindow > xWin( xFrame->getContainerWindow(),
                                                   uno::UNO_QUERY_THROW );
                xWin->toFront();
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

bool RangeSelectionHelper::chooseRange(
    const OUString & aCurrentRange,
    const OUString & aUIString,
    RangeSelectionListenerParent & rListenerParent )
{
    ControllerLockGuardUNO aGuard( m_xChartDocument );

    bool bResult = true;
    raiseRangeSelectionDocument();

    try
    {
        Reference< sheet::XRangeSelection > xRangeSel( getRangeSelection());
        if( xRangeSel.is())
        {
            Sequence< beans::PropertyValue > aArgs{
                beans::PropertyValue(
                    u"InitialValue"_ustr, -1, uno::Any( aCurrentRange ),
                    beans::PropertyState_DIRECT_VALUE ),
                beans::PropertyValue(
                    u"Title"_ustr, -1,
                    uno::Any( aUIString ),
                    beans::PropertyState_DIRECT_VALUE ),
                beans::PropertyValue(
                    u"CloseOnMouseRelease"_ustr, -1, uno::Any( true ),
                    beans::PropertyState_DIRECT_VALUE ),
                beans::PropertyValue(
                    u"MultiSelectionMode"_ustr, -1, uno::Any( true ),
                    beans::PropertyState_DIRECT_VALUE )
            };
            if( m_xRangeSelectionListener.is() )
                stopRangeListening();
            m_xRangeSelectionListener.set( Reference< sheet::XRangeSelectionListener >(
                new RangeSelectionListener( rListenerParent, aCurrentRange, m_xChartDocument )));

            xRangeSel->addRangeSelectionListener( m_xRangeSelectionListener );
            xRangeSel->startRangeSelection( aArgs );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
        bResult = false;
    }

    return bResult;
}

void RangeSelectionHelper::stopRangeListening( bool bRemoveListener /* = true */ )
{
    if( bRemoveListener &&
        m_xRangeSelectionListener.is() &&
        m_xRangeSelection.is() )
    {
        m_xRangeSelection->removeRangeSelectionListener( m_xRangeSelectionListener );
    }

    m_xRangeSelectionListener = nullptr;
}

bool RangeSelectionHelper::verifyCellRange( const OUString & rRangeStr )
{
    Reference< chart2::data::XDataProvider > xDataProvider( m_xChartDocument->getDataProvider());
    if( ! xDataProvider.is())
        return false;

    return xDataProvider->createDataSequenceByRangeRepresentationPossible( rRangeStr );
}

bool RangeSelectionHelper::verifyArguments( const Sequence< beans::PropertyValue > & rArguments )
{
    Reference< chart2::data::XDataProvider > xDataProvider( m_xChartDocument->getDataProvider());
    if( ! xDataProvider.is())
        return false;

    return xDataProvider->createDataSourcePossible( rArguments );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
