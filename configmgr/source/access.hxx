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

#ifndef INCLUDED_CONFIGMGR_SOURCE_ACCESS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_ACCESS_HXX

#include <sal/config.h>

#include <map>
#include <memory>
#include <set>
#include <vector>
#include "config_map.hxx"

#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XHierarchicalNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/interlck.h>
#include <rtl/ref.hxx>
#include <sal/types.h>

#include "modifications.hxx"
#include "path.hxx"
#include "type.hxx"

namespace com { namespace sun { namespace star {
    namespace beans {
        class XHierarchicalPropertySetInfo;
        class XPropertiesChangeListener;
        class XPropertyChangeListener;
        class XVetoableChangeListener;
        struct Property;
    }
    namespace container { class XContainerListener; }
    namespace lang { class XEventListener; }
    namespace uno {
        class Any;
        class Type;
        class XInterface;
    }
    namespace util { struct ElementChange; }
} } }

namespace configmgr {

class Broadcaster;
class ChildAccess;
class Components;
class Node;
class RootAccess;

class Access:
    public cppu::OWeakObject, public css::lang::XTypeProvider,
    public css::lang::XServiceInfo,
    public css::lang::XComponent,
    public css::container::XHierarchicalNameReplace,
    public css::container::XContainer,
    public css::beans::XExactName,
    public css::beans::XPropertySetInfo,
    public css::container::XHierarchicalName,
    public css::container::XNamed,
    public css::beans::XProperty,
    public css::beans::XPropertySet,
    public css::beans::XMultiPropertySet,
    public css::beans::XHierarchicalPropertySet,
    public css::beans::XMultiHierarchicalPropertySet,
    public css::beans::XHierarchicalPropertySetInfo,
    public css::container::XNameContainer,
    public css::lang::XSingleServiceFactory
{
public:
    oslInterlockedCount acquireCounting();

    void releaseNondeleting();

    bool isValue();

    void markChildAsModified(rtl::Reference< ChildAccess > const & child);
    void releaseChild(OUString const & name);

    virtual Path getAbsolutePath() = 0;
    virtual Path getRelativePath() = 0;

    virtual OUString getRelativePathRepresentation() = 0;
    virtual rtl::Reference< Node > getNode() = 0;

    virtual bool isFinalized() = 0;

    virtual void initBroadcaster(
        Modifications::Node const & modifications, Broadcaster * broadcaster);

    using OWeakObject::acquire;
    using OWeakObject::release;

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addEventListener(
        css::uno::Reference< css::lang::XEventListener >
            const & xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeEventListener(
        css::uno::Reference< css::lang::XEventListener >
            const & aListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Any SAL_CALL getByName(
        OUString const & aName)
        throw (
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getElementNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasByName(OUString const & aName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        OUString const & aName)
        throw (
            css::container::NoSuchElementException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasByHierarchicalName(OUString const & aName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL replaceByHierarchicalName(
        OUString const & aName, css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addContainerListener(
        css::uno::Reference<
            css::container::XContainerListener > const & xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeContainerListener(
        css::uno::Reference<
            css::container::XContainerListener > const & xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getExactName(
        OUString const & aApproximateName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< css::beans::Property >
    SAL_CALL getProperties() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::beans::Property SAL_CALL getPropertyByName(
        OUString const & aName)
        throw (
            css::beans::UnknownPropertyException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasPropertyByName(OUString const & Name)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getHierarchicalName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL composeHierarchicalName(
        OUString const & aRelativeName)
        throw (
            css::lang::IllegalArgumentException,
            css::lang::NoSupportException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setName(OUString const & aName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::beans::Property SAL_CALL getAsProperty()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual
    css::uno::Reference< css::beans::XPropertySetInfo >
    SAL_CALL getPropertySetInfo() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyValue(
        OUString const & aPropertyName,
        css::uno::Any const & aValue)
        throw (
            css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Any SAL_CALL getPropertyValue(
        OUString const & PropertyName)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const & aPropertyName,
        css::uno::Reference<
            css::beans::XPropertyChangeListener > const & xListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const & aPropertyName,
        css::uno::Reference<
            css::beans::XPropertyChangeListener > const & aListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const & PropertyName,
        css::uno::Reference<
            css::beans::XVetoableChangeListener > const & aListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const & PropertyName,
        css::uno::Reference<
            css::beans::XVetoableChangeListener > const & aListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyValues(
        css::uno::Sequence< OUString > const & aPropertyNames,
        css::uno::Sequence< css::uno::Any > const &
            aValues)
        throw (
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
    getPropertyValues(
        css::uno::Sequence< OUString > const & aPropertyNames)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertiesChangeListener(
        css::uno::Sequence< OUString > const & aPropertyNames,
        css::uno::Reference<
            css::beans::XPropertiesChangeListener > const &
                xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePropertiesChangeListener(
        css::uno::Reference<
            css::beans::XPropertiesChangeListener > const &
                xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL firePropertiesChangeEvent(
        css::uno::Sequence< OUString > const & aPropertyNames,
        css::uno::Reference<
            css::beans::XPropertiesChangeListener > const &
                xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual
    css::uno::Reference<
        css::beans::XHierarchicalPropertySetInfo > SAL_CALL
    getHierarchicalPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setHierarchicalPropertyValue(
        OUString const & aHierarchicalPropertyName,
        css::uno::Any const & aValue)
        throw (
            css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Any SAL_CALL getHierarchicalPropertyValue(
        OUString const & aHierarchicalPropertyName)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setHierarchicalPropertyValues(
        css::uno::Sequence< OUString > const &
            aHierarchicalPropertyNames,
        css::uno::Sequence< css::uno::Any > const &
            Values)
        throw (
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
    getHierarchicalPropertyValues(
        css::uno::Sequence< OUString > const &
            aHierarchicalPropertyNames)
        throw (
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::beans::Property SAL_CALL
    getPropertyByHierarchicalName(OUString const & aHierarchicalName)
        throw (
            css::beans::UnknownPropertyException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasPropertyByHierarchicalName(
        OUString const & aHierarchicalName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL replaceByName(
        OUString const & aName, css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL insertByName(
        OUString const & aName, css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::ElementExistException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeByName(OUString const & aName)
        throw (
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL createInstance()
        throw (
            css::uno::Exception,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL createInstanceWithArguments(
        css::uno::Sequence< css::uno::Any > const &
            aArguments)
        throw (
            css::uno::Exception,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    explicit Access(Components & components);

    virtual ~Access();

    virtual OUString getNameInternal() = 0;
    virtual rtl::Reference< RootAccess > getRootAccess() = 0;
    virtual rtl::Reference< Access > getParentAccess() = 0;

    virtual void addTypes(std::vector< css::uno::Type > * types)
        const = 0;

    virtual void addSupportedServiceNames(
        std::vector< OUString > * services) = 0;

    virtual void initDisposeBroadcaster(Broadcaster * broadcaster);
    virtual void clearListeners() throw ();

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & aType)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    Components & getComponents() const { return components_;}

    void checkLocalizedPropertyAccess();

    rtl::Reference< Node > getParentNode();
    rtl::Reference< ChildAccess > getChild(OUString const & name);
    std::vector< rtl::Reference< ChildAccess > > getAllChildren();

    void checkValue(
        css::uno::Any const & value, Type type, bool nillable);

    void insertLocalizedValueChild(
        OUString const & name, css::uno::Any const & value,
        Modifications * localModifications);

    void reportChildChanges(
        std::vector< css::util::ElementChange > * changes);

    void commitChildChanges(bool valid, Modifications * globalModifications);

    void initBroadcasterAndChanges(
        Modifications::Node const & modifications, Broadcaster * broadcaster,
        std::vector< css::util::ElementChange > * changes);

    bool isDisposed() const { return disposed_;}

private:
    Access(const Access&) = delete;
    Access& operator=(const Access&) = delete;

    struct ModifiedChild {
        rtl::Reference< ChildAccess > child;
        bool directlyModified;

        ModifiedChild();

        ModifiedChild(
            rtl::Reference< ChildAccess > const & theChild,
            bool theDirectlyModified);
    };

    typedef config_map< ModifiedChild > ModifiedChildren;

    rtl::Reference< ChildAccess > getModifiedChild(
        ModifiedChildren::iterator const & childIterator);

    rtl::Reference< ChildAccess > getUnmodifiedChild(
        OUString const & name);

    rtl::Reference< ChildAccess > getSubChild(OUString const & path);

    bool setChildProperty(
        OUString const & name, css::uno::Any const & value,
        Modifications * localModifications);

    css::beans::Property asProperty();

    bool getByNameFast(const OUString & name, css::uno::Any & value);
    rtl::Reference< ChildAccess > createUnmodifiedChild(const OUString &name,
                                                        const rtl::Reference< Node > &node);

    void checkFinalized();

    void checkKnownProperty(OUString const & descriptor);

    rtl::Reference< ChildAccess > getFreeSetMember( css::uno::Any const & value);

    rtl::Reference< Access > getNotificationRoot();

    typedef config_map< ChildAccess * > WeakChildMap;

    typedef
        std::multiset<
            css::uno::Reference<
                css::lang::XEventListener > >
        DisposeListeners;

    typedef
        std::multiset<
            css::uno::Reference<
                css::container::XContainerListener > >
        ContainerListeners;

    typedef
        std::multiset<
            css::uno::Reference<
                css::beans::XPropertyChangeListener > >
        PropertyChangeListenersElement;

    typedef config_map< PropertyChangeListenersElement >
        PropertyChangeListeners;

    typedef
        std::multiset<
            css::uno::Reference<
                css::beans::XVetoableChangeListener > >
        VetoableChangeListenersElement;

    typedef config_map< VetoableChangeListenersElement >
        VetoableChangeListeners;

    typedef
        std::multiset<
            css::uno::Reference<
                css::beans::XPropertiesChangeListener > >
        PropertiesChangeListeners;

    Components & components_;
    ModifiedChildren modifiedChildren_;
    WeakChildMap cachedChildren_;
    DisposeListeners disposeListeners_;
    ContainerListeners containerListeners_;
    PropertyChangeListeners propertyChangeListeners_;
    VetoableChangeListeners vetoableChangeListeners_;
    PropertiesChangeListeners propertiesChangeListeners_;
    bool disposed_;

    std::shared_ptr<osl::Mutex> lock_;

#if !defined NDEBUG
protected:
    enum {
        IS_ANY = 0, IS_GROUP = 0x01, IS_SET = 0x02, IS_EXTENSIBLE = 0x04,
        IS_GROUP_MEMBER = 0x08, IS_SET_MEMBER = 0x10, IS_UPDATE = 0x20 };
    bool thisIs(int what);
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
