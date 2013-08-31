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

#ifndef __FRAMEWORK_UICONFIGURATION_IMAGEMANAGERIMPL_HXX_
#define __FRAMEWORK_UICONFIGURATION_IMAGEMANAGERIMPL_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <uiconfiguration/imagetype.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ustring.hxx>

#include <vcl/image.hxx>
#include <rtl/ref.hxx>

namespace framework
{
    class CmdImageList
    {
        public:
            CmdImageList( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                          const OUString& aModuleIdentifier );
            virtual ~CmdImageList();

            virtual Image                           getImageFromCommandURL( sal_Int16 nImageType, const OUString& rCommandURL );
            virtual bool                            hasImage( sal_Int16 nImageType, const OUString& rCommandURL );
            virtual ::std::vector< OUString >& getImageNames();
            virtual ::std::vector< OUString >& getImageCommandNames();

        protected:
            void                            impl_fillCommandToImageNameMap();
            ImageList*                      impl_getImageList( sal_Int16 nImageType );
            std::vector< OUString >& impl_getImageNameVector();
            std::vector< OUString >& impl_getImageCommandNameVector();

        private:
            sal_Bool                                                                         m_bVectorInit;
            OUString                                                                    m_aModuleIdentifier;
            ImageList*                                                                       m_pImageList[ImageType_COUNT];
            CommandToImageNameMap                                                            m_aCommandToImageNameMap;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >     m_xContext;
            ::std::vector< OUString >                                                   m_aImageNameVector;
            ::std::vector< OUString >                                                   m_aImageCommandNameVector;
            sal_Int16                                                                        m_nSymbolsStyle;
    };

    class GlobalImageList : public CmdImageList, public rtl::IReference
    {
        public:
            GlobalImageList( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
            virtual ~GlobalImageList();

            virtual Image                           getImageFromCommandURL( sal_Int16 nImageType, const OUString& rCommandURL );
            virtual bool                            hasImage( sal_Int16 nImageType, const OUString& rCommandURL );
            virtual ::std::vector< OUString >& getImageNames();
            virtual ::std::vector< OUString >& getImageCommandNames();

            // IReference
            virtual oslInterlockedCount SAL_CALL acquire();
            virtual oslInterlockedCount SAL_CALL release();

        private:
            oslInterlockedCount                                                              m_nRefCount;
    };

    class ImageManagerImpl : public ThreadHelpBase  // Struct for right initalization of mutex member! Must be first of baseclasses.
    {
        public:
            ImageManagerImpl(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext
                ,::cppu::OWeakObject *pOwner
                ,bool _bUseGlobal);
            ~ImageManagerImpl();

            void dispose();
            void initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );
            void addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
            void removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

            // XImageManager
            void reset() throw (::com::sun::star::uno::RuntimeException);
            ::com::sun::star::uno::Sequence< OUString > getAllImageNames( ::sal_Int16 nImageType ) throw (::com::sun::star::uno::RuntimeException);
            ::sal_Bool hasImage( ::sal_Int16 nImageType, const OUString& aCommandURL ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > getImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aCommandURLSequence ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
            void replaceImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aCommandURLSequence, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& aGraphicsSequence ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
            void removeImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aResourceURLSequence ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
            void insertImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aCommandURLSequence, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& aGraphicSequence ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);

            // XUIConfiguration
            void addConfigurationListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
            void removeConfigurationListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

            // XUIConfigurationPersistence
            void reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            void store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            void storeToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            sal_Bool isModified() throw (::com::sun::star::uno::RuntimeException);
            sal_Bool isReadOnly() throw (::com::sun::star::uno::RuntimeException);

            void clear();

            typedef boost::unordered_map< OUString,
                                   sal_Bool,
                                   OUStringHash,
                                   ::std::equal_to< OUString > > ImageNameMap;

            enum Layer
            {
                LAYER_DEFAULT,
                LAYER_USERDEFINED,
                LAYER_COUNT
            };

            enum NotifyOp
            {
                NotifyOp_Remove,
                NotifyOp_Insert,
                NotifyOp_Replace
            };

            typedef ::std::vector< ::com::sun::star::ui::ConfigurationEvent > ConfigEventNotifyContainer;

            // private methods
            void                                      implts_initialize();
            void                                      implts_notifyContainerListener( const ::com::sun::star::ui::ConfigurationEvent& aEvent, NotifyOp eOp );
            ImageList*                                implts_getUserImageList( ImageType nImageType );
            sal_Bool                                  implts_loadUserImages( ImageType nImageType,
                                                                             const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xUserImageStorage,
                                                                             const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xUserBitmapsStorage );
            sal_Bool                                  implts_storeUserImages( ImageType nImageType,
                                                                              const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xUserImageStorage,
                                                                              const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xUserBitmapsStorage );
            const rtl::Reference< GlobalImageList >&  implts_getGlobalImageList();
            CmdImageList*                             implts_getDefaultImageList();


            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >               m_xUserConfigStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >               m_xUserImageStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >               m_xUserBitmapsStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XTransactedObject >      m_xUserRootCommit;
            com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >        m_xContext;
            ::cppu::OWeakObject*                                                            m_pOwner;
            rtl::Reference< GlobalImageList >                                               m_pGlobalImageList;
            CmdImageList*                                                                   m_pDefaultImageList;
            OUString                                                                   m_aXMLPostfix;
            OUString                                                                   m_aModuleIdentifier;
            OUString                                                                   m_aResourceString;
            ::cppu::OMultiTypeInterfaceContainerHelper                                      m_aListenerContainer;   /// container for ALL Listener
            ImageList*                                                                      m_pUserImageList[ImageType_COUNT];
            bool                                                                            m_bUserImageListModified[ImageType_COUNT];
            bool                                                                            m_bUseGlobal;
            bool                                                                            m_bReadOnly;
            bool                                                                            m_bInitialized;
            bool                                                                            m_bModified;
            bool                                                                            m_bConfigRead;
            bool                                                                            m_bDisposed;
   };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
