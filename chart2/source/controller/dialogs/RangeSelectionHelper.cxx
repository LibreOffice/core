/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RangeSelectionHelper.cxx,v $
 * $Revision: 1.4 $
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

#include "RangeSelectionHelper.hxx"
#include "RangeSelectionListener.hxx"
#include "macros.hxx"
#include "ControllerLockGuard.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XCellRangesAccess.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

// ----------------------------------------

namespace
{
OUString lcl_getTextFromRanges( const Sequence < Reference< table::XCellRange > >& xRanges )
{
    ::rtl::OUStringBuffer aResultBuf;

    try
    {
        sal_Int32 nCount(xRanges.getLength());
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            Reference< sheet::XCellRangeAddressable > xAddressable( xRanges[i], uno::UNO_QUERY );
            if( xAddressable.is())
            {
                // iterate through columns first
                table::CellRangeAddress aAddr = xAddressable->getRangeAddress();
                bool bInsSpace = false;
                for( sal_Int32 nCol=aAddr.StartColumn; nCol<=aAddr.EndColumn; ++nCol )
                    for( sal_Int32 nRow=aAddr.StartRow; nRow<=aAddr.EndRow; ++nRow )
                    {
                        if( bInsSpace )
                            aResultBuf.append( sal_Unicode( ' ' ));
                        else
                            bInsSpace = true;
                        Reference< text::XText > xText( xRanges[i]->getCellByPosition(
                                                            nCol - aAddr.StartColumn,
                                                            nRow - aAddr.StartRow ), uno::UNO_QUERY );
                        if( xText.is())
                            aResultBuf.append( xText->getString());
                    }
            }
            else if (xRanges[i].is())
            {
                // assert that (0,0) is valid.  Otherwise an exception is thrown
                Reference< text::XText > xText( xRanges[i]->getCellByPosition( 0, 0 ), uno::UNO_QUERY );
                if( xText.is() )
                    aResultBuf.append( xText->getString());
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return aResultBuf.makeStringAndClear();
}
} // anonymous namespace

// ----------------------------------------

namespace chart
{

RangeSelectionHelper::RangeSelectionHelper(
    const Reference< chart2::XChartDocument > & xChartDocument ) :
        m_xChartDocument( xChartDocument )
{}

RangeSelectionHelper::~RangeSelectionHelper()
{}

bool RangeSelectionHelper::hasRangeSelection()
{
    return getRangeSelection().is();
}

Reference< sheet::XRangeSelection > RangeSelectionHelper::getRangeSelection()
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
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );

            m_xRangeSelection.clear();
        }
    }

    return m_xRangeSelection;
}

Reference< sheet::XCellRangesAccess > RangeSelectionHelper::getCellRangesAccess()
{
    if( ! m_xCellRangesAccess.is() &&
        m_xChartDocument.is())
    {
        try
        {
            Reference< sheet::XSpreadsheetDocument > xSpreadDoc;

            // try controller
            Reference< chart2::data::XDataProvider > xDataProvider( m_xChartDocument->getDataProvider());
            if( xDataProvider.is())
            {
                Reference< frame::XController > xCtrl( xDataProvider->getRangeSelection(), uno::UNO_QUERY );

                if( xCtrl.is() )
                    xSpreadDoc.set( xCtrl->getModel(), uno::UNO_QUERY_THROW );
            }

            if( xSpreadDoc.is() )
                m_xCellRangesAccess.set( xSpreadDoc->getSheets(), uno::UNO_QUERY );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xCellRangesAccess;
}

void RangeSelectionHelper::raiseRangeSelectionDocument()
{
    Reference< sheet::XRangeSelection > xRangeSel( getRangeSelection());
    if( xRangeSel.is())
    {
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
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

bool RangeSelectionHelper::chooseRange(
    const OUString & aCurrentRange,
    const OUString & aUIString,
    RangeSelectionListenerParent & rListenerParent )
{
    ControllerLockGuard aGuard( Reference< frame::XModel >(m_xChartDocument, uno::UNO_QUERY ) );

    bool bResult = true;
    raiseRangeSelectionDocument();

    try
    {
        Reference< sheet::XRangeSelection > xRangeSel( getRangeSelection());
        if( xRangeSel.is())
        {
            Sequence< beans::PropertyValue > aArgs( 4 );
            aArgs[0] = beans::PropertyValue(
                C2U("InitialValue"), -1, uno::makeAny( aCurrentRange ),
                beans::PropertyState_DIRECT_VALUE );
            aArgs[1] = beans::PropertyValue(
                C2U("Title"), -1,
                uno::makeAny( aUIString ),
                beans::PropertyState_DIRECT_VALUE );
            aArgs[2] = beans::PropertyValue(
                C2U("CloseOnMouseRelease"), -1, uno::makeAny( true ),
                beans::PropertyState_DIRECT_VALUE );
            aArgs[3] = beans::PropertyValue(
                C2U("MultiSelectionMode"), -1, uno::makeAny( true ),
                beans::PropertyState_DIRECT_VALUE );

            if( m_xRangeSelectionListener.is() )
                stopRangeListening();
            m_xRangeSelectionListener.set( Reference< sheet::XRangeSelectionListener >(
                new RangeSelectionListener( rListenerParent, aCurrentRange, Reference< frame::XModel >(m_xChartDocument, uno::UNO_QUERY ) )));

            xRangeSel->addRangeSelectionListener( m_xRangeSelectionListener );
            xRangeSel->startRangeSelection( aArgs );
        }
    }
    catch( uno::Exception & ex )
    {
        bResult = false;
        ASSERT_EXCEPTION( ex );
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

    m_xRangeSelectionListener = 0;
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

OUString RangeSelectionHelper::getCellRangeContent( const OUString & rRangeStr )
{
    OUString aResult;

    Reference< sheet::XCellRangesAccess > xRangesAccess( getCellRangesAccess());
    if( xRangesAccess.is())
    {
        // @todo: the interface should provide a method to determine if a cell
        // range is correct, rather than waiting for an exception
        try
        {
            aResult = lcl_getTextFromRanges( xRangesAccess->getCellRangesByName( rRangeStr ) );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

} //  namespace chart
