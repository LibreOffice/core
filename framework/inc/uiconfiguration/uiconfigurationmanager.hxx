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

#ifndef __FRAMEWORK_UICONFIGURATION_UICONFIGMANAGER_HXX_
#define __FRAMEWORK_UICONFIGURATION_UICONFIGMANAGER_HXX_


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
#include <uiconfiguration/imagemanager.hxx>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>


namespace framework
{
    class UIConfigurationManager :   private ThreadHelpBase                     ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                     public ::cppu::WeakImplHelper2<
                                            com::sun::star::lang::XServiceInfo  ,
                                            com::sun::star::ui::XUIConfigurationManager2 >
    {
        public:
            //  XInterface, XTypeProvider, XServiceInfo
            DECLARE_XSERVICEINFO

            UIConfigurationManager( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & rxContext );
            virtual ~UIConfigurationManager();

            // XComponent
            virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

            // XUIConfiguration
            virtual void SAL_CALL addConfigurationListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeConfigurationListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

            // XUIConfigurationManager
            virtual void SAL_CALL reset() throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getUIElementsInfo( sal_Int16 ElementType ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > SAL_CALL createSettings(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasSettings( const OUString& ResourceURL ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getSettings( const OUString& ResourceURL, sal_Bool bWriteable ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL replaceSettings( const OUString& ResourceURL, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& aNewData ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeSettings( const OUString& ResourceURL ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL insertSettings( const OUString& NewResourceURL, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& aNewData ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getImageManager() throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > SAL_CALL getShortCutManager() throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getEventsManager() throw (::com::sun::star::uno::RuntimeException);

            // XUIConfigurationPersistence
            virtual void SAL_CALL reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL storeToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isReadOnly() throw (::com::sun::star::uno::RuntimeException);

            // XUIConfigurationStorage
            virtual void SAL_CALL setStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasStorage() throw (::com::sun::star::uno::RuntimeException);

        private:
            // private data types
            enum NotifyOp
            {
                NotifyOp_Remove,
                NotifyOp_Insert,
                NotifyOp_Replace
            };

            struct UIElementInfo
            {
                UIElementInfo( const OUString& rResourceURL, const OUString& rUIName ) :
                    aResourceURL( rResourceURL), aUIName( rUIName ) {}
                OUString   aResourceURL;
                OUString   aUIName;
            };

            struct UIElementData
            {
                UIElementData() : bModified( false ), bDefault( true ) {};

                OUString aResourceURL;
                OUString aName;
                bool          bModified;        // has been changed since last storing
                bool          bDefault;         // default settings
                com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > xSettings;
            };

            struct UIElementType;
            friend struct UIElementType;
            typedef ::boost::unordered_map< OUString, UIElementData, OUStringHash, ::std::equal_to< OUString > > UIElementDataHashMap;

            struct UIElementType
            {
                UIElementType() : bModified( false ),
                                  bLoaded( false ),
                                  bDefaultLayer( false ),
                                  nElementType( ::com::sun::star::ui::UIElementType::UNKNOWN ) {}


                bool                                                              bModified;
                bool                                                              bLoaded;
                bool                                                              bDefaultLayer;
                sal_Int16                                                         nElementType;
                UIElementDataHashMap                                              aElementsHashMap;
                com::sun::star::uno::Reference< com::sun::star::embed::XStorage > xStorage;
            };

            typedef ::std::vector< UIElementType > UIElementTypesVector;
            typedef ::std::vector< ::com::sun::star::ui::ConfigurationEvent > ConfigEventNotifyContainer;
            typedef ::boost::unordered_map< OUString, UIElementInfo, OUStringHash, ::std::equal_to< OUString > > UIElementInfoHashMap;

            // private methods
            void            impl_Initialize();
            void            implts_notifyContainerListener( const ::com::sun::star::ui::ConfigurationEvent& aEvent, NotifyOp eOp );
            void            impl_fillSequenceWithElementTypeInfo( UIElementInfoHashMap& aUIElementInfoCollection, sal_Int16 nElementType );
            void            impl_preloadUIElementTypeList( sal_Int16 nElementType );
            UIElementData*  impl_findUIElementData( const OUString& aResourceURL, sal_Int16 nElementType, bool bLoad = true );
            void            impl_requestUIElementData( sal_Int16 nElementType, UIElementData& aUIElementData );
            void            impl_storeElementTypeData( com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xStorage, UIElementType& rElementType, bool bResetModifyState = true );
            void            impl_resetElementTypeData( UIElementType& rDocElementType, ConfigEventNotifyContainer& rRemoveNotifyContainer );
            void            impl_reloadElementTypeData( UIElementType& rDocElementType, ConfigEventNotifyContainer& rRemoveNotifyContainer, ConfigEventNotifyContainer& rReplaceNotifyContainer );

            UIElementTypesVector                                                            m_aUIElements;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >               m_xDocConfigStorage;
            bool                                                                            m_bReadOnly;
            bool                                                                            m_bModified;
            bool                                                                            m_bConfigRead;
            bool                                                                            m_bDisposed;
            OUString                                                                   m_aXMLPostfix;
            OUString                                                                   m_aPropUIName;
            OUString                                                                   m_aPropResourceURL;
            OUString                                                                   m_aModuleIdentifier;
            com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >        m_xContext;
            ::cppu::OMultiTypeInterfaceContainerHelper                                      m_aListenerContainer;   /// container for ALL Listener
            com::sun::star::uno::Reference< com::sun::star::lang::XComponent >              m_xImageManager;
            com::sun::star::uno::Reference< com::sun::star::ui::XAcceleratorConfiguration > m_xAccConfig;
   };
}

#endif // __FRAMEWORK_UICONFIGURATION_UICONFIGMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
