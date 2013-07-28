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

#include "CommandDispatchContainer.hxx"
#include "UndoCommandDispatch.hxx"
#include "StatusBarCommandDispatch.hxx"
#include "DisposeHelper.hxx"
#include "macros.hxx"
#include "ChartController.hxx"
#include "DrawCommandDispatch.hxx"
#include "ShapeController.hxx"

#include <comphelper/InlineContainer.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

CommandDispatchContainer::CommandDispatchContainer(
    const Reference< uno::XComponentContext > & xContext, ChartController* pController )
        :m_xContext( xContext )
        ,m_pChartController( pController )
        ,m_pDrawCommandDispatch( NULL )
        ,m_pShapeController( NULL )
{
    m_aContainerDocumentCommands =
        ::comphelper::MakeSet< OUString >
        ("AddDirect")    ("NewDoc")             ("Open")
        ("Save")         ("SaveAs")             ("SendMail")
        ("EditDoc")      ("ExportDirectToPDF")  ("PrintDefault")
        ;
}

void CommandDispatchContainer::setModel(
    const Reference< frame::XModel > & xModel )
{
    // remove all existing dispatcher that base on the old model
    m_aCachedDispatches.clear();
    DisposeHelper::DisposeAllElements( m_aToBeDisposedDispatches );
    m_aToBeDisposedDispatches.clear();
    m_xModel = xModel;
}

void CommandDispatchContainer::setChartDispatch(
    const Reference< frame::XDispatch > xChartDispatch,
    const ::std::set< OUString > & rChartCommands )
{
    OSL_ENSURE(xChartDispatch.is(),"Invalid fall back dispatcher!");
    m_xChartDispatcher.set( xChartDispatch );
    m_aChartCommands = rChartCommands;
    m_aToBeDisposedDispatches.push_back( m_xChartDispatcher );
}

Reference< frame::XDispatch > CommandDispatchContainer::getDispatchForURL(
    const util::URL & rURL )
{
    Reference< frame::XDispatch > xResult;
    tDispatchMap::const_iterator aIt( m_aCachedDispatches.find( rURL.Complete ));
    if( aIt != m_aCachedDispatches.end())
    {
        xResult.set( (*aIt).second );
    }
    else
    {
        uno::Reference< frame::XModel > xModel( m_xModel );

        if( xModel.is() && ( rURL.Path == "Undo" || rURL.Path == "Redo" ) )
        {
            CommandDispatch * pDispatch = new UndoCommandDispatch( m_xContext, xModel );
            xResult.set( pDispatch );
            pDispatch->initialize();
            m_aCachedDispatches[ ".uno:Undo" ].set( xResult );
            m_aCachedDispatches[ ".uno:Redo" ].set( xResult );
            m_aToBeDisposedDispatches.push_back( xResult );
        }
        else if( xModel.is() && ( rURL.Path == "Context" || rURL.Path == "ModifiedStatus" ) )
        {
            Reference< view::XSelectionSupplier > xSelSupp( xModel->getCurrentController(), uno::UNO_QUERY );
            CommandDispatch * pDispatch = new StatusBarCommandDispatch( m_xContext, xModel, xSelSupp );
            xResult.set( pDispatch );
            pDispatch->initialize();
            m_aCachedDispatches[ ".uno:Context" ].set( xResult );
            m_aCachedDispatches[ ".uno:ModifiedStatus" ].set( xResult );
            m_aToBeDisposedDispatches.push_back( xResult );
        }
        else if( xModel.is() &&
                 (m_aContainerDocumentCommands.find( rURL.Path ) != m_aContainerDocumentCommands.end()) )
        {
            xResult.set( getContainerDispatchForURL( xModel->getCurrentController(), rURL ));
            // ToDo: can those dispatches be cached?
            m_aCachedDispatches[ rURL.Complete ].set( xResult );
        }
        else if( m_xChartDispatcher.is() &&
                 (m_aChartCommands.find( rURL.Path ) != m_aChartCommands.end()) )
        {
            xResult.set( m_xChartDispatcher );
            m_aCachedDispatches[ rURL.Complete ].set( xResult );
        }
        // #i12587# support for shapes in chart
        // Note, that the chart dispatcher must be queried first, because
        // the chart dispatcher is the default dispatcher for all context
        // sensitive commands.
        else if ( m_pDrawCommandDispatch && m_pDrawCommandDispatch->isFeatureSupported( rURL.Complete ) )
        {
            xResult.set( m_pDrawCommandDispatch );
            m_aCachedDispatches[ rURL.Complete ].set( xResult );
        }
        else if ( m_pShapeController && m_pShapeController->isFeatureSupported( rURL.Complete ) )
        {
            xResult.set( m_pShapeController );
            m_aCachedDispatches[ rURL.Complete ].set( xResult );
        }
    }

    return xResult;
}

Sequence< Reference< frame::XDispatch > > CommandDispatchContainer::getDispatchesForURLs(
    const Sequence< frame::DispatchDescriptor > & aDescriptors )
{
    sal_Int32 nCount = aDescriptors.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet( nCount );

    for( sal_Int32 nPos = 0; nPos < nCount; ++nPos )
    {
        if ( aDescriptors[ nPos ].FrameName == "_self" )
            aRet[ nPos ] = getDispatchForURL( aDescriptors[ nPos ].FeatureURL );
    }
    return aRet;
}

void CommandDispatchContainer::DisposeAndClear()
{
    m_aCachedDispatches.clear();
    DisposeHelper::DisposeAllElements( m_aToBeDisposedDispatches );
    m_aToBeDisposedDispatches.clear();
    m_xChartDispatcher.clear();
    m_aChartCommands.clear();
    m_pChartController = NULL;
    m_pDrawCommandDispatch = NULL;
    m_pShapeController = NULL;
}

Reference< frame::XDispatch > CommandDispatchContainer::getContainerDispatchForURL(
    const Reference< frame::XController > & xChartController,
    const util::URL & rURL )
{
    Reference< frame::XDispatch > xResult;
    if( xChartController.is())
    {
        Reference< frame::XFrame > xFrame( xChartController->getFrame());
        if( xFrame.is())
        {
            Reference< frame::XDispatchProvider > xDispProv( xFrame->getCreator(), uno::UNO_QUERY );
            if( xDispProv.is())
                xResult.set( xDispProv->queryDispatch( rURL, "_self", 0 ));
        }
    }
    return xResult;
}

void CommandDispatchContainer::setDrawCommandDispatch( DrawCommandDispatch* pDispatch )
{
    m_pDrawCommandDispatch = pDispatch;
    m_aToBeDisposedDispatches.push_back( Reference< frame::XDispatch >( pDispatch ) );
}

void CommandDispatchContainer::setShapeController( ShapeController* pController )
{
    m_pShapeController = pController;
    m_aToBeDisposedDispatches.push_back( Reference< frame::XDispatch >( pController ) );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
