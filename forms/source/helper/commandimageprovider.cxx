/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "commandimageprovider.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/ImageType.hpp>
/** === end UNO includes === **/

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
        virtual CommandImages getCommandImages( const CommandURLs& _rCommandURLs, const bool _bLarge, const bool _bHiContrast ) const;

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
            Reference< XModuleManager > xModuleManager( _rContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
            ::rtl::OUString sModuleID = xModuleManager->identify( _rxDocument );

            Reference< XModuleUIConfigurationManagerSupplier > xSuppUIConfig(
                _rContext.createComponent( "com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ), UNO_QUERY_THROW );
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
    CommandImages DocumentCommandImageProvider::getCommandImages( const CommandURLs& _rCommandURLs, const bool _bLarge, const bool _bHiContrast ) const
    {
        const size_t nCommandCount = _rCommandURLs.getLength();
        CommandImages aImages( nCommandCount );
        try
        {
            const sal_Int16 nImageType =
                    ( _bLarge       ? ImageType::SIZE_LARGE         : ImageType::SIZE_DEFAULT )
                +   ( _bHiContrast  ? ImageType::COLOR_HIGHCONTRAST : ImageType::COLOR_NORMAL );

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
