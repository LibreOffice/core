/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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


#include "commandimageprovider.hxx"

#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/ImageType.hpp>

#include <tools/diagnose_ex.h>

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::ui::XImageManager;
    using ::com::sun::star::ui::XUIConfigurationManagerSupplier;
    using ::com::sun::star::ui::XUIConfigurationManager;
    using ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier;
    using ::com::sun::star::ui::ModuleUIConfigurationManagerSupplier;
    using ::com::sun::star::frame::ModuleManager;
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::graphic::XGraphic;
    /** === end UNO using === **/
    namespace ImageType = ::com::sun::star::ui::ImageType;

    //====================================================================
    //= DocumentCommandImageProvider
    //====================================================================
    class DocumentCommandImageProvider : public ICommandImageProvider
    {
    public:
        DocumentCommandImageProvider( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
        {
            impl_init_nothrow( _rContext, _rxDocument );
        }
        virtual ~DocumentCommandImageProvider()
        {
        }

        // ICommandImageProvider
        virtual CommandImages getCommandImages( const CommandURLs& _rCommandURLs, const bool _bLarge ) const;

    private:
        void    impl_init_nothrow( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument );

    private:
        Reference< XImageManager >    m_xDocumentImageManager;
        Reference< XImageManager >    m_xModuleImageManager;
    };

    //--------------------------------------------------------------------
    void DocumentCommandImageProvider::impl_init_nothrow( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
    {
        OSL_ENSURE( _rxDocument.is(), "DocumentCommandImageProvider::impl_init_nothrow: no document => no images!" );
        if ( !_rxDocument.is() )
            return;

        // obtain the image manager of the document
        try
        {
            Reference< XUIConfigurationManagerSupplier > xSuppUIConfig( _rxDocument, UNO_QUERY_THROW );
            Reference< XUIConfigurationManager > xUIConfig( xSuppUIConfig->getUIConfigurationManager(), UNO_QUERY );
            m_xDocumentImageManager.set( xUIConfig->getImageManager(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // obtain the image manager or the module
        try
        {
            Reference< XModuleManager > xModuleManager( ModuleManager::create(_rContext.getUNOContext()), UNO_QUERY_THROW );
            ::rtl::OUString sModuleID = xModuleManager->identify( _rxDocument );

            Reference< XModuleUIConfigurationManagerSupplier > xSuppUIConfig(
                ModuleUIConfigurationManagerSupplier::create(_rContext.getUNOContext()) );
            Reference< XUIConfigurationManager > xUIConfig(
                xSuppUIConfig->getUIConfigurationManager( sModuleID ), UNO_SET_THROW );
            m_xModuleImageManager.set( xUIConfig->getImageManager(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    CommandImages DocumentCommandImageProvider::getCommandImages( const CommandURLs& _rCommandURLs, const bool _bLarge ) const
    {
        const size_t nCommandCount = _rCommandURLs.getLength();
        CommandImages aImages( nCommandCount );
        try
        {
            const sal_Int16 nImageType = ImageType::COLOR_NORMAL
                                       +  ( _bLarge ? ImageType::SIZE_LARGE : ImageType::SIZE_DEFAULT );

            Sequence< Reference< XGraphic > > aDocImages( nCommandCount );
            Sequence< Reference< XGraphic > > aModImages( nCommandCount );

            // first try the document image manager
            if ( m_xDocumentImageManager.is() )
                aDocImages = m_xDocumentImageManager->getImages( nImageType, _rCommandURLs );

            // then the module's image manager
            if ( m_xModuleImageManager.is() )
                aModImages = m_xModuleImageManager->getImages( nImageType, _rCommandURLs );

            ENSURE_OR_THROW( (size_t)aDocImages.getLength() == nCommandCount, "illegal array size returned by getImages (document image manager)" );
            ENSURE_OR_THROW( (size_t)aModImages.getLength() == nCommandCount, "illegal array size returned by getImages (module image manager)" );

            for ( size_t i=0; i<nCommandCount; ++i )
            {
                if ( aDocImages[i].is() )
                    aImages[i] = Image( aDocImages[i] );
                else
                    aImages[i] = Image( aModImages[i] );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aImages;
    }

    //--------------------------------------------------------------------
    PCommandImageProvider createDocumentCommandImageProvider(
        const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
    {
        PCommandImageProvider pImageProvider( new DocumentCommandImageProvider( _rContext, _rxDocument ) );
        return pImageProvider;
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
