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

#ifndef INCLUDED_UCBHELPER_CONTENTHELPER_HXX
#define INCLUDED_UCBHELPER_CONTENTHELPER_HXX

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#include <com/sun/star/ucb/XCommandInfoChangeNotifier.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <cppuhelper/weak.hxx>

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace com { namespace sun { namespace star { namespace ucb {
    struct CommandInfo;
    class XCommandEnvironment;
    class XCommandInfo;
    class XPersistentPropertySet;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    class XPropertySetInfo;
} } } }

namespace ucbhelper_impl { struct ContentImplHelper_Impl; }

namespace ucbhelper
{



class ContentProviderImplHelper;

/**
  * This is an abstract base class for implementations of the service
  * com.sun.star.ucb.Content. Implementations derived from this class are
  * objects provided by implementations derived from
  * class ucb::ContentProviderImplHelper.
  *
  * Features of the base class implementation:
  * - standard interfaces ( XInterface, XTypeProvider, XServiceInfo )
  * - all required interfaces for service com::sun::star::ucb::Content
  * - all required listener containers
  *   ( XComponent, XPropertiesChangeNotifier, XPropertySetInfoChangeNotifier,
  *     XCommandInfoChangeNotifier )
  * - XPropertyContainer implementation ( persistence is implemented using
  *   service com.sun.star.ucb.Store )
  * - complete XPropertySetInfo implementation ( including Additional Core
  *   Properties supplied via XPropertyContainer interface )
  *   -> protected method: getPropertySetInfo
  * - complete XCommandInfo implementation
  *    -> protected method: getCommandInfo
  */
class UCBHELPER_DLLPUBLIC ContentImplHelper :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XComponent,
                public com::sun::star::ucb::XContent,
                public com::sun::star::ucb::XCommandProcessor,
                public com::sun::star::beans::XPropertiesChangeNotifier,
                public com::sun::star::beans::XPropertyContainer,
                public com::sun::star::beans::XPropertySetInfoChangeNotifier,
                public com::sun::star::ucb::XCommandInfoChangeNotifier,
                public com::sun::star::container::XChild
{
    friend class PropertySetInfo;
    friend class CommandProcessorInfo;

    ucbhelper_impl::ContentImplHelper_Impl* m_pImpl;

protected:
    osl::Mutex                       m_aMutex;
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
                                     m_xContext;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >
                                     m_xIdentifier;
    rtl::Reference< ContentProviderImplHelper >
                                     m_xProvider;
    sal_uInt32                       m_nCommandId;

private:
    /**
      * Your implementation of this method must return a sequence containing
      * the meta data of the properties supported by the content.
      * Note: If you wish to provide your own implementation of the interface
      * XPropertyContainer ( completely override addContent and removeContent
      * implementation of this base class in this case ), you can supply the
      * meta data for your Additional Core Properties here to get a fully
      * featured getPropertySetInfo method ( see below ).
      *
      * @param xEnv is an environment to use for example, for interactions.
      * @return a sequence containing the property meta data.
      */
    UCBHELPER_DLLPRIVATE
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv ) = 0;

    /**
      * Your implementation of this method must return a sequence containing
      * the meta data of the commands supported by the content.
      *
      * @param xEnv is an environment to use for example, for interactions.
      * @return a sequence containing the command meta data.
      */
    UCBHELPER_DLLPRIVATE
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv ) = 0;

    /**
      * The implementation of this method shall return the URL of the parent
      * of your content.
      *
      * @return the URL of the parent content or an empty string.
      *         Note that not all contents must have one parent. There may
      *         be contents with no parent. In that case an empty string must
      *         be returned. If your content has more than one parent you may
      *         return the URL of one "preferred" parent or an empty string.
      */
    UCBHELPER_DLLPRIVATE virtual OUString getParentURL() = 0;

protected:
    /**
      * This method returns complete meta data for the properties ( including
      * Additional Core Properties supplied via XPropertyContainer interface )
      * supported by the content. To implement the required command
      * "getPropertySetInfo" simply return the return value of this method.
      *
      * @param xEnv is an environment to use for example, for interactions.
      * @param bCache indicates, whether the implementation should use
      *        cached data, if exist.
      * @return an XPropertySetInfo implementation object containing meta data
      *         for the properties supported by this content.
      */
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
    getPropertySetInfo( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment > & xEnv,
                        bool bCache = true );

    /**
      * This method returns complete meta data for the commands supported by
      * the content. To implement the required command "getCommandInfo" simply
      * return the return value of this method.
      *
      * @param xEnv is an environment to use for example, for interactions.
      * @param bCache indicates, whether the implementation should use
      *        cached data, if exist.
      * @return an XCommandInfo implementation object containing meta data
      *         for the commands supported by this content.
      */
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo >
    getCommandInfo( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment > & xEnv,
                    bool bCache = true );

    /**
      * This method can be used to propagate changes of property values.
      *
      * @param evt is a sequence of property change events.
      */
    void notifyPropertiesChange(
        const com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyChangeEvent >& evt ) const;

    /**
      * This method can be used to propagate changes of the propertyset
      * info of your content (i.e. this happens if a new property is added
      * to your content via its XPropertyContainer interface). This base class
      * automatically generates events when the propertyset info changes. If
      * you provide your own implementations of addproperty and removeProperty,
      * then you must call "notifyPropertySetInfoChange" by yourself.
      *
      * @param evt is a sequence of property change events.
      */
    void notifyPropertySetInfoChange(
        const com::sun::star::beans::PropertySetInfoChangeEvent& evt ) const;

    /**
      * This method can be used to propagate content events.
      *
      * @param evt is a sequence of content events.
      */
    void notifyContentEvent(
            const com::sun::star::ucb::ContentEvent& evt ) const;

    /**
      * Use this method to announce the insertion of this content at
      * the end of your implementation of the command "insert". The
      * implementation of is method propagates a ContentEvent( INSERTED ).
      */
    void inserted();

    /**
      * Use this method to announce the destruction of this content at
      * the end of your implementation of the command "delete". The
      * implementation of is method propagates a ContentEvent( DELETED )
      * and a ContentEvent( REMOVED ) at the parent of the deleted content,
      * if a parent exists.
      */
    void deleted();

    /**
      * Use this method to change the identity of a content. The implementation
      * of this method will replace the content identifier of the content and
      * propagate the appropriate ContentEvent( EXCHANGED ).
      *
      * @param  rNewId is the new content identifier for the content.
      * @return a success indicator.
      */
    bool exchange( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& rNewId );

    /**
      * Use this method to get access to the Additional Core Properties of
      * the content ( added using content's XPropertyContainer interface ).
      * If you supply your own XPropertyContainer implementation, this method
      * will always return an empty propertyset.
      *
      * @param  bCreate indicates whether a new propertyset shall be created
      *         if it does not exist.
      * @return the implementation of the service
      *         com.sun.star.ucb.PersistentPropertySet.
      */
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XPersistentPropertySet >
    getAdditionalPropertySet( bool bCreate );

    /**
      * This method renames the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  rOldKey is the old key of the propertyset.
      * @param  rNewKey is the new key for the propertyset.
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rOldKey shall be renamed too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    bool renameAdditionalPropertySet( const OUString& rOldKey,
                                          const OUString& rNewKey,
                                          bool bRecursive );

    /**
      * This method copies the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  rSourceKey is the key of the source propertyset.
      * @param  rTargetKey is the key of the target propertyset.
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rSourceKey shall be copied too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    bool copyAdditionalPropertySet( const OUString& rSourceKey,
                                        const OUString& rTargetKey,
                                        bool bRecursive );

    /**
      * This method removes the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rOldKey shall be removed too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    bool removeAdditionalPropertySet( bool bRecursive );

public:
    /**
      * Constructor.
      *
      * Note that the implementation of this ctor registers itself at its
      * content provider. The provider implementation inserts the content
      * in a hash map. So it easily can be found and reused when the provider
      * is asked for a content.
      *
      * @param rxContext is a Service Manager.
      * @param rxProvider is the provider for the content.
      * @param Identifier is the content identifier for the content.
      */
    ContentImplHelper(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            const rtl::Reference< ContentProviderImplHelper >& rxProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    /**
      * Destructor.
      *
      * Note that the implementation of this dtor deregisters itself from its
      * content provider. The provider implementation removes the content
      * from a hash map.
      */
    virtual ~ContentImplHelper();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw() SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw() SAL_OVERRIDE;

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;
    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XContent
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    getIdentifier()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;
    virtual void SAL_CALL
    addContentEventListener(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    removeContentEventListener(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;

    // XPropertiesChangeNotifier
    virtual void SAL_CALL
    addPropertiesChangeListener(
        const com::sun::star::uno::Sequence< OUString >& PropertyNames,
         const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    removePropertiesChangeListener(
        const com::sun::star::uno::Sequence< OUString >& PropertyNames,
        const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XCommandInfoChangeNotifier
    virtual void SAL_CALL
    addCommandInfoChangeListener(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    removeCommandInfoChangeListener(
        const com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XPropertyContainer

    /**
      * This method adds a property to the content according to the interface
      * specification. The properties will be stored using the service
      * com.sun.star.ucb.Store.
      *
      * Note: You may provide your own implementation of this method, for
      * instance, if your data source supports adding/removing of properties.
      * Don't forget to return the meta data for these properties in your
      * implementation of getPropertyInfoTable.
      */
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**
      * This method removes a property from the content according to the
      * interface specification. The properties will be stored using the
      * service com.sun.star.ucb.Store.
      *
      * Note: You may provide your own implementation of this method, for
      * instance, if your data source supports adding/removing of properties.
      * Don't forget to return the meta data for these properties in your
      * implementation of getPropertyInfoTable.
      */
    virtual void SAL_CALL
    removeProperty( const OUString& Name )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::NotRemoveableException,
               com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XPropertySetInfoChangeNotifier
    virtual void SAL_CALL
    addPropertySetInfoChangeListener(
        const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    removePropertySetInfoChangeListener(
        const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XChild

    /**
      * This method returns the content representing the parent of a content,
      * if such a parent exists. The implementation of this method uses your
      * implementation of getParentURL.
      */
    virtual com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface > SAL_CALL
    getParent()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**
      * The implementation of this method always throws a NoSupportException.
      */
    virtual void SAL_CALL
    setParent( const com::sun::star::uno::Reference<
                        com::sun::star::uno::XInterface >& Parent )
        throw( com::sun::star::lang::NoSupportException,
               com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


    // Non-interface methods.


    /**
      * This method returns the provider of the content.
      *
      * @return the provider of the content.
      */
    const rtl::Reference< ContentProviderImplHelper >& getProvider() const
    { return m_xProvider; }
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_CONTENTHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
