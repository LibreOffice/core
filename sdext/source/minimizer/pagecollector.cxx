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


#include "pagecollector.hxx"
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

void PageCollector::CollectCustomShowPages( const css::uno::Reference< css::frame::XModel >& rxModel, std::u16string_view rCustomShowName, std::vector< Reference< XDrawPage > >& rUsedPageList )
{
    try
    {
        Reference< XCustomPresentationSupplier > aXCPSup( rxModel, UNO_QUERY_THROW );
        Reference< XNameContainer > aXCont( aXCPSup->getCustomPresentations() );
        if ( aXCont.is() )
        {
            // creating a list of every page that is used within our customshow
            const Sequence< OUString> aNameSeq( aXCont->getElementNames() );
            for ( OUString const & i :aNameSeq )
            {
                if ( i == rCustomShowName )
                {
                    Reference< container::XIndexContainer > aXIC( aXCont->getByName( i ), UNO_QUERY_THROW );
                    sal_Int32 j, nSlideCount = aXIC->getCount();
                    for ( j = 0; j < nSlideCount; j++ )
                    {
                        Reference< XDrawPage > xDrawPage( aXIC->getByIndex( j ), UNO_QUERY_THROW );
                        auto aIter = std::find(rUsedPageList.begin(), rUsedPageList.end(), xDrawPage);
                        if ( aIter == rUsedPageList.end() )
                            rUsedPageList.push_back( xDrawPage );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {

    }
}

void PageCollector::CollectNonCustomShowPages( const css::uno::Reference< css::frame::XModel >& rxModel, std::u16string_view rCustomShowName, std::vector< Reference< XDrawPage > >& rNonUsedPageList )
{
    try
    {
        std::vector< Reference< XDrawPage > > vUsedPageList;
        PageCollector::CollectCustomShowPages( rxModel, rCustomShowName, vUsedPageList );
        if ( !vUsedPageList.empty() )
        {
            Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
            Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_SET_THROW );
            for ( sal_Int32 j = 0; j < xDrawPages->getCount(); j++ )
            {
                Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( j ), UNO_QUERY_THROW );
                auto aIter = std::find(vUsedPageList.begin(), vUsedPageList.end(), xDrawPage);
                if ( aIter == vUsedPageList.end() )
                    rNonUsedPageList.push_back( xDrawPage );
            }
        }
    }
    catch( Exception& )
    {
    }
}


void PageCollector::CollectMasterPages( const Reference< XModel >& rxModel, std::vector< PageCollector::MasterPageEntity >& rMasterPageList )
{
    try
    {
        // generating list of all master pages
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_SET_THROW );
        for ( sal_Int32 i = 0; i < xMasterPages->getCount(); i++ )
        {
            Reference< XDrawPage > xMasterPage( xMasterPages->getByIndex( i ), UNO_QUERY_THROW );
            auto aIter = std::find_if(rMasterPageList.begin(), rMasterPageList.end(),
                [&xMasterPage](const MasterPageEntity& rEntity) { return rEntity.xMasterPage == xMasterPage; });
            if ( aIter == rMasterPageList.end() )
            {
                MasterPageEntity aMasterPageEntity;
                aMasterPageEntity.xMasterPage = xMasterPage;
                aMasterPageEntity.bUsed = false;
                rMasterPageList.push_back( aMasterPageEntity );
            }
        }

        // mark masterpages which are referenced by drawpages
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_SET_THROW );
        for ( sal_Int32 j = 0; j < xDrawPages->getCount(); j++ )
        {
            Reference< XMasterPageTarget > xMasterPageTarget( xDrawPages->getByIndex( j ), UNO_QUERY_THROW );
            Reference< XDrawPage > xMasterPage( xMasterPageTarget->getMasterPage(), UNO_SET_THROW );
            auto aIter = std::find_if(rMasterPageList.begin(), rMasterPageList.end(),
                [&xMasterPage](const MasterPageEntity& rEntity) { return rEntity.xMasterPage == xMasterPage; });
            if ( aIter == rMasterPageList.end() )
                throw uno::RuntimeException();
            aIter->bUsed = true;
        }
    }
    catch( Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
