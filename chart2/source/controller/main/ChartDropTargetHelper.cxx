/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

} 

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
                if( aStrings.size() >= 3 && aStrings[0] == "soffice" )
                {
                    OUString aRangeString( aStrings[2] );
                    Reference< container::XChild > xChild( m_xChartDocument, uno::UNO_QUERY );
                    if( xChild.is())
                    {
                        Reference< frame::XModel > xParentModel( xChild->getParent(), uno::UNO_QUERY );
                        if( xParentModel.is() &&
                            m_xChartDocument.is())
                        {
                            bool bDataComesFromParent = true;
                            
                            
                            
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
                                        if ( aArguments[i].Name == "CellRangeRepresentation" )
                                        {
                                            pCellRange = (aArguments.getArray() + i);
                                            aArguments[i].Value >>= aOldRange;
                                            break;
                                        }
                                    }
                                    if( pCellRange )
                                    {
                                        
                                        if( rEvt.mnAction == DND_ACTION_COPY )
                                        {
                                            
                                            
                                            pCellRange->Value <<= (aOldRange + ";" + aRangeString );
                                        }
                                        
                                        else
                                        {
                                            pCellRange->Value <<= aRangeString;
                                        }

                                        xDataSource.set( xDataProvider->createDataSource( aArguments ));
                                        xDiagram->setDiagramData( xDataSource, aArguments );

                                        
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
