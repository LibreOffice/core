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

#include <sal/config.h>

#include <set>

#include "documentenumeration.hxx"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <tools/diagnose_ex.h>

#include <comphelper/stl_types.hxx>

namespace basctl { namespace docs {

    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::frame::Desktop;
    using ::com::sun::star::frame::XDesktop2;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XFrames;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XModel2;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::frame::XFrame;

    namespace FrameSearchFlag = ::com::sun::star::frame::FrameSearchFlag;

    // DocumentEnumeration_Data
    struct DocumentEnumeration_Data
    {
        Reference< css::uno::XComponentContext > aContext;
        const IDocumentDescriptorFilter*    pFilter;

        DocumentEnumeration_Data( Reference< css::uno::XComponentContext > const & _rContext, const IDocumentDescriptorFilter* _pFilter )
            :aContext( _rContext )
            ,pFilter( _pFilter )
        {
        }
    };

    // DocumentEnumeration
    DocumentEnumeration::DocumentEnumeration( Reference< css::uno::XComponentContext > const & _rContext, const IDocumentDescriptorFilter* _pFilter )
        :m_pData( new DocumentEnumeration_Data( _rContext, _pFilter ) )
    {
    }

    DocumentEnumeration::~DocumentEnumeration()
    {
    }

    namespace
    {
        void lcl_getDocumentControllers_nothrow( DocumentDescriptor& _io_rDocDesc )
        {
            OSL_PRECOND( _io_rDocDesc.xModel.is(), "lcl_getDocumentControllers_nothrow: illegal model!" );

            _io_rDocDesc.aControllers.clear();
            try
            {
                Reference< XModel2 > xModel2( _io_rDocDesc.xModel, UNO_QUERY );
                if ( xModel2.is() )
                {
                    Reference< XEnumeration > xEnum( xModel2->getControllers(), UNO_SET_THROW );
                    while ( xEnum->hasMoreElements() )
                    {
                        Reference< XController > xController( xEnum->nextElement(), UNO_QUERY_THROW );
                        _io_rDocDesc.aControllers.push_back( xController );
                    }
                }
                else if ( _io_rDocDesc.xModel.is() )
                    _io_rDocDesc.aControllers.push_back( _io_rDocDesc.xModel->getCurrentController() );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        void lcl_getDocuments_nothrow( const Sequence< Reference< XFrame > >& _rFrames, Documents& _out_rDocuments,
            const IDocumentDescriptorFilter* _pFilter )
        {
            // ensure we don't encounter some models multiple times
            std::set< Reference< XModel >, ::comphelper::OInterfaceCompare< XModel > > aEncounteredModels;

            for ( auto const & rFrame : _rFrames )
            {
                try
                {
                    OSL_ENSURE( rFrame.is(), "lcl_getDocuments_nothrow: illegal frame!" );
                    if ( !rFrame.is() )
                        continue;
                    Reference< XController > xController( rFrame->getController() );
                    if ( !xController.is() )
                        continue;

                    Reference< XModel > xModel( xController->getModel() );
                    if ( !xModel.is() )
                        // though it's legal for a controller to not have a model, we're not interested in
                        // those
                        continue;

                    if ( aEncounteredModels.find( xModel ) != aEncounteredModels.end() )
                        // there might be multiple frames for the same model
                        // handle it only once
                        continue;
                    aEncounteredModels.insert( xModel );

                    // create a DocumentDescriptor
                    DocumentDescriptor aDescriptor;
                    aDescriptor.xModel = xModel;
                    lcl_getDocumentControllers_nothrow( aDescriptor );

                    // consult filter, if there is one
                    if ( _pFilter && !_pFilter->includeDocument( aDescriptor ) )
                        continue;

                    _out_rDocuments.push_back( aDescriptor );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
    }

    void DocumentEnumeration::getDocuments( Documents& _out_rDocuments ) const
    {
        _out_rDocuments.clear();

        try
        {
            const Reference< XDesktop2 > xDesktop = Desktop::create( m_pData->aContext );
            const Reference< XFrames > xFrames( xDesktop->getFrames(), UNO_SET_THROW );
            const Sequence< Reference< XFrame > > aFrames( xFrames->queryFrames( FrameSearchFlag::ALL ) );

            lcl_getDocuments_nothrow( aFrames, _out_rDocuments, m_pData->pFilter );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

} } // namespace basctl::docs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
