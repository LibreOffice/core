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
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/weak.hxx>

#include <rtl/ref.hxx>
#include <ucbhelper/ucbhelperdllapi.h>
#include <memory>

namespace com::sun::star::ucb {
    struct CommandInfo;
    class XCommandEnvironment;
    class XCommandInfo;
    class XPersistentPropertySet;
}

namespace com::sun::star::beans {
    struct Property;
    class XPropertySetInfo;
}

namespace com::sun::star::uno { class XComponentContext; }

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
  * - all required interfaces for service css::ucb::Content
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
                public css::lang::XTypeProvider,
                public css::lang::XServiceInfo,
                public css::lang::XComponent,
                public css::ucb::XContent,
                public css::ucb::XCommandProcessor,
                public css::beans::XPropertiesChangeNotifier,
                public css::beans::XPropertyContainer,
                public css::beans::XPropertySetInfoChangeNotifier,
                public css::ucb::XCommandInfoChangeNotifier,
                public css::container::XChild
{
    friend class PropertySetInfo;
    friend class CommandProcessorInfo;

    std::unique_ptr<ucbhelper_impl::ContentImplHelper_Impl> m_pImpl;

protected:
    osl::Mutex                       m_aMutex;
    css::uno::Reference< css::uno::XComponentContext >
                                     m_xContext;
    css::uno::Reference< css::ucb::XContentIdentifier >
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
    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) = 0;

    /**
      * Your implementation of this method must return a sequence containing
      * the meta data of the commands supported by the content.
      *
      * @param xEnv is an environment to use for example, for interactions.
      * @return a sequence containing the command meta data.
      */
    UCBHELPER_DLLPRIVATE
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) = 0;

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
    css::uno::Reference< css::beans::XPropertySetInfo >
    getPropertySetInfo( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
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
    css::uno::Reference< css::ucb::XCommandInfo >
    getCommandInfo( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
                    bool bCache = true );

    /**
      * This method can be used to propagate changes of property values.
      *
      * @param evt is a sequence of property change events.
      */
    void notifyPropertiesChange(
        const css::uno::Sequence< css::beans::PropertyChangeEvent >& evt ) const;

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
        const css::beans::PropertySetInfoChangeEvent& evt ) const;

    /**
      * This method can be used to propagate content events.
      *
      * @param evt is a sequence of content events.
      */
    void notifyContentEvent(
            const css::ucb::ContentEvent& evt ) const;

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
    bool exchange( const css::uno::Reference< css::ucb::XContentIdentifier >& rNewId );

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
    css::uno::Reference< css::ucb::XPersistentPropertySet >
    getAdditionalPropertySet( bool bCreate );

    /**
      * This method renames the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  rOldKey is the old key of the propertyset.
      * @param  rNewKey is the new key for the propertyset.
      * @return True, if the operation succeeded - False, otherwise.
      */
    bool renameAdditionalPropertySet( const OUString& rOldKey,
                                          const OUString& rNewKey );

    /**
      * This method copies the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  rSourceKey is the key of the source propertyset.
      * @param  rTargetKey is the key of the target propertyset.
      * @return True, if the operation succeeded - False, otherwise.
      */
    bool copyAdditionalPropertySet( const OUString& rSourceKey,
                                        const OUString& rTargetKey );

    /**
      * This method removes the propertyset containing the Additional Core
      * Properties of the content.
      *
      * Propertysets for children described by rOldKey are removed too.
      *
      * @return True, if the operation succeeded - False, otherwise.
      */
    bool removeAdditionalPropertySet();

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
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const rtl::Reference< ContentProviderImplHelper >& rxProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier );

    /**
      * Destructor.
      *
      * Note that the implementation of this dtor deregisters itself from its
      * content provider. The provider implementation removes the content
      * from a hash map.
      */
    virtual ~ContentImplHelper() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { cppu::OWeakObject::acquire(); }
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL
    getImplementationName() override = 0;
    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override = 0;

    // XComponent
    virtual void SAL_CALL
    dispose() override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;

    // XContent
    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    getIdentifier() override;
    virtual OUString SAL_CALL
    getContentType() override = 0;
    virtual void SAL_CALL
    addContentEventListener(
        const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) override;
    virtual void SAL_CALL
    removeContentEventListener(
        const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) override;

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier() override;
    virtual css::uno::Any SAL_CALL
    execute( const css::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override = 0;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId ) override = 0;

    // XPropertiesChangeNotifier
    virtual void SAL_CALL
    addPropertiesChangeListener(
        const css::uno::Sequence< OUString >& PropertyNames,
         const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) override;
    virtual void SAL_CALL
    removePropertiesChangeListener(
        const css::uno::Sequence< OUString >& PropertyNames,
        const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) override;

    // XCommandInfoChangeNotifier
    virtual void SAL_CALL
    addCommandInfoChangeListener(
        const css::uno::Reference< css::ucb::XCommandInfoChangeListener >& Listener ) override;
    virtual void SAL_CALL
    removeCommandInfoChangeListener(
        const css::uno::Reference< css::ucb::XCommandInfoChangeListener >& Listener ) override;

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
                 const css::uno::Any& DefaultValue ) override;

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
    removeProperty( const OUString& Name ) override;

    // XPropertySetInfoChangeNotifier
    virtual void SAL_CALL
    addPropertySetInfoChangeListener(
        const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;
    virtual void SAL_CALL
    removePropertySetInfoChangeListener(
        const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;

    // XChild

    /**
      * This method returns the content representing the parent of a content,
      * if such a parent exists. The implementation of this method uses your
      * implementation of getParentURL.
      */
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    getParent() override;

    /**
      * The implementation of this method always throws a NoSupportException.
      */
    virtual void SAL_CALL
    setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;


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
