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

#include <CommandDispatchContainer.hxx>
#include <ControllerCommandDispatch.hxx>
#include "UndoCommandDispatch.hxx"
#include "StatusBarCommandDispatch.hxx"
#include <DisposeHelper.hxx>
#include "DrawCommandDispatch.hxx"
#include "ShapeController.hxx"
#include <ChartModel.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>

#include <o3tl/sorted_vector.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

CommandDispatchContainer::CommandDispatchContainer(
    const Reference< uno::XComponentContext > & xContext )
        :m_xContext( xContext )
        ,m_pDrawCommandDispatch( nullptr )
        ,m_pShapeController( nullptr )
{
}

CommandDispatchContainer::~CommandDispatchContainer() = default;

void CommandDispatchContainer::setModel(
    const rtl::Reference<::chart::ChartModel> & xModel )
{
    // remove all existing dispatcher that base on the old model
    m_aCachedDispatches.clear();
    DisposeHelper::DisposeAllElements( m_aToBeDisposedDispatches );
    m_aToBeDisposedDispatches.clear();
    m_xModel = xModel.get();
}

void CommandDispatchContainer::setChartDispatch(
    const rtl::Reference< ControllerCommandDispatch >& rChartDispatch,
    const o3tl::sorted_vector< std::u16string_view > & rChartCommands )
{
    OSL_ENSURE(rChartDispatch.is(),"Invalid fall back dispatcher!");
    m_xChartDispatcher = rChartDispatch;
    m_aChartCommands = rChartCommands;
    m_aToBeDisposedDispatches.push_back( m_xChartDispatcher );
}

Reference< frame::XDispatch > CommandDispatchContainer::getDispatchForURL(
    const util::URL & rURL )
{
    static const o3tl::sorted_vector< std::u16string_view >  s_aContainerDocumentCommands {
        u"AddDirect",    u"NewDoc",             u"Open",
        u"Save",         u"SaveAs",             u"SendMail",
        u"EditDoc",      u"ExportDirectToPDF",  u"PrintDefault"};

    if (auto aIt = m_aCachedDispatches.find(rURL.Complete); aIt != m_aCachedDispatches.end())
        return aIt->second;

    auto cacheIt = [this, url = rURL.Complete](const Reference<frame::XDispatch>& val)
    {
        m_aCachedDispatches[url].set(val);
        return val;
    };

    if (rtl::Reference<::chart::ChartModel> xModel{ m_xModel })
    {
        if (rURL.Path == "Undo" || rURL.Path == "Redo" ||
            rURL.Path == "GetUndoStrings" || rURL.Path == "GetRedoStrings")
        {
            rtl::Reference<CommandDispatch> pDispatch = new UndoCommandDispatch( m_xContext, xModel );
            pDispatch->initialize();
            m_aCachedDispatches[u".uno:Undo"_ustr].set(pDispatch);
            m_aCachedDispatches[u".uno:Redo"_ustr].set(pDispatch);
            m_aCachedDispatches[u".uno:GetUndoStrings"_ustr].set(pDispatch);
            m_aCachedDispatches[u".uno:GetRedoStrings"_ustr].set(pDispatch);
            m_aToBeDisposedDispatches.push_back(pDispatch);
            return pDispatch;
        }
        if (rURL.Path == "Context" || rURL.Path == "ModifiedStatus")
        {
            Reference< view::XSelectionSupplier > xSelSupp( xModel->getCurrentController(), uno::UNO_QUERY );
            rtl::Reference<CommandDispatch> pDispatch = new StatusBarCommandDispatch( m_xContext, xModel, xSelSupp );
            pDispatch->initialize();
            m_aCachedDispatches[u".uno:Context"_ustr].set(pDispatch);
            m_aCachedDispatches[u".uno:ModifiedStatus"_ustr].set(pDispatch);
            m_aToBeDisposedDispatches.push_back(pDispatch);
            return pDispatch;
        }
        if (s_aContainerDocumentCommands.count(rURL.Path) > 0)
        {
            // ToDo: can those dispatches be cached?
            return cacheIt(getContainerDispatchForURL(xModel->getCurrentController(), rURL));
        }
    }

    if (m_xChartDispatcher.is() && m_aChartCommands.count(rURL.Path) > 0)
        return cacheIt(m_xChartDispatcher);

    // #i12587# support for shapes in chart
    // Note, that the chart dispatcher must be queried first, because
    // the chart dispatcher is the default dispatcher for all context
    // sensitive commands.
    if (m_pDrawCommandDispatch && m_pDrawCommandDispatch->isFeatureSupported(rURL.Complete))
        return cacheIt(m_pDrawCommandDispatch);

    if (m_pShapeController && m_pShapeController->isFeatureSupported(rURL.Complete))
        return cacheIt(m_pShapeController);

    return {};
}

Sequence< Reference< frame::XDispatch > > CommandDispatchContainer::getDispatchesForURLs(
    const Sequence< frame::DispatchDescriptor > & aDescriptors )
{
    sal_Int32 nCount = aDescriptors.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet( nCount );
    auto aRetRange = asNonConstRange(aRet);

    for( sal_Int32 nPos = 0; nPos < nCount; ++nPos )
    {
        if (aDescriptors[nPos].FrameName.isEmpty() || aDescriptors[nPos].FrameName == "_self")
            aRetRange[ nPos ] = getDispatchForURL( aDescriptors[ nPos ].FeatureURL );
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
    m_pDrawCommandDispatch = nullptr;
    m_pShapeController = nullptr;
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
                xResult.set( xDispProv->queryDispatch( rURL, u"_self"_ustr, 0 ));
        }
    }
    return xResult;
}

void CommandDispatchContainer::setDrawCommandDispatch( DrawCommandDispatch* pDispatch )
{
    m_pDrawCommandDispatch = pDispatch;
    m_aToBeDisposedDispatches.emplace_back( pDispatch );
}

void CommandDispatchContainer::setShapeController( ShapeController* pController )
{
    m_pShapeController = pController;
    m_aToBeDisposedDispatches.emplace_back( pController );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
