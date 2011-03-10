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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ChartDropTargetHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSourceHelper.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <sot/formats.hxx>
#include <vector>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{

::std::vector< OUString > lcl_getStringsFromByteSequence(
    const Sequence< sal_Int8 > & aByteSequence )
{
    ::std::vector< OUString > aResult;
    const sal_Int32 nLength = aByteSequence.getLength();
    const sal_Char * pBytes( reinterpret_cast< const sal_Char* >( aByteSequence.getConstArray()));
    sal_Int32 nStartPos = 0;
    for( sal_Int32 nPos=0; nPos<nLength; ++nPos )
    {
        if( pBytes[nPos] == '\0' )
        {
            aResult.push_back( OUString( pBytes + nStartPos, (nPos - nStartPos), RTL_TEXTENCODING_ASCII_US ));
            nStartPos = nPos + 1;
        }
    }
    return aResult;
}

} // anonymous namespace

namespace chart
{

ChartDropTargetHelper::ChartDropTargetHelper(
    const Reference< datatransfer::dnd::XDropTarget >& rxDropTarget,
    const Reference< chart2::XChartDocument > & xChartDocument ) :
        DropTargetHelper( rxDropTarget ),
        m_xChartDocument( xChartDocument )
{}

ChartDropTargetHelper::~ChartDropTargetHelper()
{}

bool ChartDropTargetHelper::satisfiesPrerequisites() const
{
    return  ( m_xChartDocument.is() &&
              ! m_xChartDocument->hasInternalDataProvider());
}

sal_Int8 ChartDropTargetHelper::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nResult = DND_ACTION_NONE;

    if( ( rEvt.mnAction == DND_ACTION_COPY ||
          rEvt.mnAction == DND_ACTION_MOVE ) &&
        satisfiesPrerequisites() &&
        IsDropFormatSupported( SOT_FORMATSTR_ID_LINK ) )
    {
        // @todo: check if the data is suitable. Is this possible without XTransferable?
        nResult = rEvt.mnAction;
    }

    return nResult;
}

sal_Int8 ChartDropTargetHelper::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nResult = DND_ACTION_NONE;

    if( ( rEvt.mnAction == DND_ACTION_COPY ||
          rEvt.mnAction == DND_ACTION_MOVE ) &&
        rEvt.maDropEvent.Transferable.is() &&
        satisfiesPrerequisites())
    {
        TransferableDataHelper aDataHelper( rEvt.maDropEvent.Transferable );
        if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK ))
        {
            Sequence< sal_Int8 > aBytes;
            if( aDataHelper.GetSequence( SOT_FORMATSTR_ID_LINK, aBytes ))
            {
                ::std::vector< OUString > aStrings( lcl_getStringsFromByteSequence( aBytes ));
                if( aStrings.size() >= 3 &&
                    aStrings[0].equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "soffice" )))
                {
                    OUString aDocName( aStrings[1] );
                    OUString aRangeString( aStrings[2] );
                    Reference< container::XChild > xChild( m_xChartDocument, uno::UNO_QUERY );
                    if( xChild.is())
                    {
                        Reference< frame::XModel > xParentModel( xChild->getParent(), uno::UNO_QUERY );
                        if( xParentModel.is() &&
                            m_xChartDocument.is())
                        {
                            bool bDataComesFromParent = true;
                            // @todo: get the title somehow and compare it to
                            // aDocName if successful (the document is the
                            // parent)
                            if( bDataComesFromParent )
                            {
                                Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram() );
                                Reference< chart2::data::XDataProvider > xDataProvider( m_xChartDocument->getDataProvider());
                                if( xDataProvider.is() && xDiagram.is() &&
                                    DataSourceHelper::allArgumentsForRectRangeDetected( m_xChartDocument ))
                                {
                                    Reference< chart2::data::XDataSource > xDataSource(
                                        DataSourceHelper::pressUsedDataIntoRectangularFormat( m_xChartDocument ));
                                    Sequence< beans::PropertyValue > aArguments(
                                        xDataProvider->detectArguments( xDataSource ));

                                    OUString aOldRange;
                                    beans::PropertyValue * pCellRange = 0;
                                    for( sal_Int32 i=0; i<aArguments.getLength(); ++i )
                                    {
                                        if( aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CellRangeRepresentation")))
                                        {
                                            pCellRange = (aArguments.getArray() + i);
                                            aArguments[i].Value >>= aOldRange;
                                            break;
                                        }
                                    }
                                    if( pCellRange )
                                    {
                                        // copy means add ranges, move means replace
                                        if( rEvt.mnAction == DND_ACTION_COPY )
                                        {
                                            // @todo: using implcit knowledge that ranges can be
                                            // merged with ";". This should be done more general
                                            pCellRange->Value <<= (aOldRange + OUString( sal_Unicode(';')) + aRangeString );
                                        }
                                        // move means replace range
                                        else
                                        {
                                            pCellRange->Value <<= aRangeString;
                                        }

                                        xDataSource.set( xDataProvider->createDataSource( aArguments ));
                                        xDiagram->setDiagramData( xDataSource, aArguments );

                                        // always return copy state to avoid deletion of the dragged range
                                        nResult = DND_ACTION_COPY;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return nResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
