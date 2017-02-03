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

#ifndef INCLUDED_FRAMEWORK_SOURCE_UICONFIGURATION_IMAGEMANAGERIMPL_HXX
#define INCLUDED_FRAMEWORK_SOURCE_UICONFIGURATION_IMAGEMANAGERIMPL_HXX

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

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <list>
#include <unordered_map>
#include <vector>

#include "CommandImageResolver.hxx"

namespace framework
{
    class CmdImageList
    {
        public:
            CmdImageList(const css::uno::Reference< css::uno::XComponentContext >& rxContext, const OUString& aModuleIdentifier);
            virtual ~CmdImageList();

            virtual Image getImageFromCommandURL(vcl::ImageType nImageType, const OUString& rCommandURL);
            virtual bool hasImage(vcl::ImageType nImageType, const OUString& rCommandURL);
            virtual std::vector<OUString>& getImageCommandNames();

        protected:
            void initialize();

        private:
            bool m_bInitialized;
            vcl::CommandImageResolver m_aResolver;

            OUString m_aModuleIdentifier;
            css::uno::Reference<css::uno::XComponentContext> m_xContext;
    };

    class GlobalImageList : public CmdImageList, public salhelper::SimpleReferenceObject
    {
        public:
            explicit GlobalImageList(const css::uno::Reference< css::uno::XComponentContext >& rxContext);
            virtual ~GlobalImageList() override;

            virtual Image                           getImageFromCommandURL( vcl::ImageType nImageType, const OUString& rCommandURL ) override;
            virtual bool                            hasImage( vcl::ImageType nImageType, const OUString& rCommandURL ) override;
            virtual ::std::vector< OUString >&      getImageCommandNames() override;
    };

    class ImageManagerImpl
    {
        public:
            ImageManagerImpl(const css::uno::Reference< css::uno::XComponentContext >& rxContext
                ,::cppu::OWeakObject *pOwner
                ,bool _bUseGlobal);
            ~ImageManagerImpl();

            void dispose();
            void initialize( const css::uno::Sequence< css::uno::Any >& aArguments );
            /// @throws css::uno::RuntimeException
            void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener );
            /// @throws css::uno::RuntimeException
            void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener );

            // XImageManager
            /// @throws css::uno::RuntimeException
            /// @throws css::lang::IllegalAccessException
            void reset();
            /// @throws css::uno::RuntimeException
            css::uno::Sequence< OUString > getAllImageNames( ::sal_Int16 nImageType );
            /// @throws css::lang::IllegalArgumentException
            /// @throws css::uno::RuntimeException
            bool hasImage( ::sal_Int16 nImageType, const OUString& aCommandURL );
            /// @throws css::lang::IllegalArgumentException
            /// @throws css::uno::RuntimeException
            css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > > getImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aCommandURLSequence );
            /// @throws css::lang::IllegalArgumentException
            /// @throws css::lang::IllegalAccessException
            /// @throws css::uno::RuntimeException
            void replaceImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aCommandURLSequence, const css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >& aGraphicsSequence );
            /// @throws css::lang::IllegalArgumentException
            /// @throws css::lang::IllegalAccessException
            /// @throws css::uno::RuntimeException
            void removeImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aResourceURLSequence );
            /// @throws css::container::ElementExistException
            /// @throws css::lang::IllegalArgumentException
            /// @throws css::lang::IllegalAccessException
            /// @throws css::uno::RuntimeException
            void insertImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aCommandURLSequence, const css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >& aGraphicSequence );

            // XUIConfiguration
            /// @throws css::uno::RuntimeException
            void addConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener );
            /// @throws css::uno::RuntimeException
            void removeConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener );

            // XUIConfigurationPersistence
            /// @throws css::uno::Exception
            /// @throws css::uno::RuntimeException
            void reload();
            /// @throws css::uno::Exception
            /// @throws css::uno::RuntimeException
            void store();
            /// @throws css::uno::Exception
            /// @throws css::uno::RuntimeException
            void storeToStorage( const css::uno::Reference< css::embed::XStorage >& Storage );
            /// @throws css::uno::RuntimeException
            bool isModified();
            /// @throws css::uno::RuntimeException
            bool isReadOnly();

            void clear();

            typedef std::unordered_map< OUString,
                                        sal_Bool,
                                        OUStringHash > ImageNameMap;

            enum NotifyOp
            {
                NotifyOp_Remove,
                NotifyOp_Insert,
                NotifyOp_Replace
            };

            typedef ::std::vector< css::ui::ConfigurationEvent > ConfigEventNotifyContainer;

            void                                      implts_initialize();
            void                                      implts_notifyContainerListener( const css::ui::ConfigurationEvent& aEvent, NotifyOp eOp );
            ImageList*                                implts_getUserImageList( vcl::ImageType nImageType );
            bool                                      implts_loadUserImages( vcl::ImageType nImageType,
                                                                             const css::uno::Reference< css::embed::XStorage >& xUserImageStorage,
                                                                             const css::uno::Reference< css::embed::XStorage >& xUserBitmapsStorage );
            bool                                      implts_storeUserImages( vcl::ImageType nImageType,
                                                                              const css::uno::Reference< css::embed::XStorage >& xUserImageStorage,
                                                                              const css::uno::Reference< css::embed::XStorage >& xUserBitmapsStorage );
            const rtl::Reference< GlobalImageList >&  implts_getGlobalImageList();
            CmdImageList*                             implts_getDefaultImageList();

            css::uno::Reference< css::embed::XStorage >               m_xUserConfigStorage;
            css::uno::Reference< css::embed::XStorage >               m_xUserImageStorage;
            css::uno::Reference< css::embed::XStorage >               m_xUserBitmapsStorage;
            css::uno::Reference< css::embed::XTransactedObject >      m_xUserRootCommit;
            css::uno::Reference< css::uno::XComponentContext >        m_xContext;
            ::cppu::OWeakObject*                                                            m_pOwner;
            rtl::Reference< GlobalImageList >                                               m_pGlobalImageList;
            CmdImageList*                                                                   m_pDefaultImageList;
            OUString                                                                   m_aModuleIdentifier;
            OUString                                                                   m_aResourceString;
            osl::Mutex m_mutex;
            ::cppu::OMultiTypeInterfaceContainerHelper                                      m_aListenerContainer;   /// container for ALL Listener
            o3tl::enumarray<vcl::ImageType,ImageList*>                                      m_pUserImageList;
            o3tl::enumarray<vcl::ImageType,bool>                                            m_bUserImageListModified;
            bool                                                                            m_bUseGlobal;
            bool                                                                            m_bReadOnly;
            bool                                                                            m_bInitialized;
            bool                                                                            m_bModified;
            bool                                                                            m_bDisposed;
   };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
