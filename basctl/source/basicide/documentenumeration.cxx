/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documentenumeration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:23:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include "documentenumeration.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

#include <comphelper/stl_types.hxx>

//........................................................................
namespace basctl { namespace docs {
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::frame::XDesktop;
    using ::com::sun::star::container::XEnumerationAccess;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XFramesSupplier;
    using ::com::sun::star::frame::XFrames;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XModel2;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::frame::XFrame;
    /** === end UNO using === **/
    namespace FrameSearchFlag = ::com::sun::star::frame::FrameSearchFlag;

    //====================================================================
    //= DocumentEnumeration_Data
    //====================================================================
    struct DocumentEnumeration_Data
    {
        ::comphelper::ComponentContext      aContext;
        const IDocumentDescriptorFilter*    pFilter;

        DocumentEnumeration_Data( const ::comphelper::ComponentContext& _rContext, const IDocumentDescriptorFilter* _pFilter )
            :aContext( _rContext )
            ,pFilter( _pFilter )
        {
        }
    };

    //====================================================================
    //= DocumentEnumeration
    //====================================================================
    //--------------------------------------------------------------------
    DocumentEnumeration::DocumentEnumeration( const ::comphelper::ComponentContext& _rContext, const IDocumentDescriptorFilter* _pFilter )
        :m_pData( new DocumentEnumeration_Data( _rContext, _pFilter ) )
    {
    }

    //--------------------------------------------------------------------
    DocumentEnumeration::~DocumentEnumeration()
    {
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
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

        //................................................................
        void lcl_getDocuments_nothrow( const Sequence< Reference< XFrame > >& _rFrames, Documents& _out_rDocuments,
            const IDocumentDescriptorFilter* _pFilter )
        {
            // ensure we don't encounter some models multiple times
            ::std::set< Reference< XModel >, ::comphelper::OInterfaceCompare< XModel > > aEncounteredModels;

            for (   const Reference< XFrame >* pFrame = _rFrames.getConstArray();
                    pFrame != _rFrames.getConstArray() + _rFrames.getLength();
                    ++pFrame
                )
            {
                try
                {
                    OSL_ENSURE( pFrame->is(), "lcl_getDocuments_nothrow: illegal frame!" );
                    if ( !pFrame->is() )
                        continue;
                    Reference< XController > xController( (*pFrame)->getController() );
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

    //--------------------------------------------------------------------
    void DocumentEnumeration::getDocuments( Documents& _out_rDocuments ) const
    {
        _out_rDocuments.clear();

        try
        {
            const Reference< XDesktop > xDesktop( m_pData->aContext.createComponent( "com.sun.star.frame.Desktop" ), UNO_QUERY_THROW );
            const Reference< XFramesSupplier > xSuppFrames( xDesktop, UNO_QUERY_THROW );
            const Reference< XFrames > xFrames( xSuppFrames->getFrames(), UNO_SET_THROW );
            const Sequence< Reference< XFrame > > aFrames( xFrames->queryFrames( FrameSearchFlag::ALL ) );

            lcl_getDocuments_nothrow( aFrames, _out_rDocuments, m_pData->pFilter );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} } // namespace basctl::docs
//........................................................................
