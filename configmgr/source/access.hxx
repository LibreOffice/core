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
#include <set>
#include <vector>
#include "config_map.hxx"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
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
    public cppu::OWeakObject, public com::sun::star::lang::XTypeProvider,
    public com::sun::star::lang::XServiceInfo,
    public com::sun::star::lang::XComponent,
    public com::sun::star::container::XHierarchicalNameReplace,
    public com::sun::star::container::XContainer,
    public com::sun::star::beans::XExactName,
    public com::sun::star::beans::XPropertySetInfo,
    public com::sun::star::container::XHierarchicalName,
    public com::sun::star::container::XNamed,
    public com::sun::star::beans::XProperty,
    public com::sun::star::beans::XPropertySet,
    public com::sun::star::beans::XMultiPropertySet,
    public com::sun::star::beans::XHierarchicalPropertySet,
    public com::sun::star::beans::XMultiHierarchicalPropertySet,
    public com::sun::star::beans::XHierarchicalPropertySetInfo,
    public com::sun::star::container::XNameContainer,
    public com::sun::star::lang::XSingleServiceFactory,
    private boost::noncopyable
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

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL dispose()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addEventListener(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeEventListener(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Type SAL_CALL getElementType()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasElements()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Any SAL_CALL getByName(
        OUString const & aName)
        throw (
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getElementNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasByName(OUString const & aName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Any SAL_CALL getByHierarchicalName(
        OUString const & aName)
        throw (
            com::sun::star::container::NoSuchElementException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasByHierarchicalName(OUString const & aName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL replaceByHierarchicalName(
        OUString const & aName, com::sun::star::uno::Any const & aElement)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addContainerListener(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeContainerListener(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getExactName(
        OUString const & aApproximateName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    SAL_CALL getProperties() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::beans::Property SAL_CALL getPropertyByName(
        OUString const & aName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasPropertyByName(OUString const & Name)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getHierarchicalName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL composeHierarchicalName(
        OUString const & aRelativeName)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::NoSupportException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setName(OUString const & aName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::beans::Property SAL_CALL getAsProperty()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
    SAL_CALL getPropertySetInfo() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyValue(
        OUString const & aPropertyName,
        com::sun::star::uno::Any const & aValue)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue(
        OUString const & PropertyName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const & aPropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > const & xListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const & aPropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > const & aListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const & PropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XVetoableChangeListener > const & aListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const & PropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XVetoableChangeListener > const & aListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyValues(
        com::sun::star::uno::Sequence< OUString > const & aPropertyNames,
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            aValues)
        throw (
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Any > SAL_CALL
    getPropertyValues(
        com::sun::star::uno::Sequence< OUString > const & aPropertyNames)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertiesChangeListener(
        com::sun::star::uno::Sequence< OUString > const & aPropertyNames,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const &
                xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePropertiesChangeListener(
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const &
                xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL firePropertiesChangeEvent(
        com::sun::star::uno::Sequence< OUString > const & aPropertyNames,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const &
                xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual
    com::sun::star::uno::Reference<
        com::sun::star::beans::XHierarchicalPropertySetInfo > SAL_CALL
    getHierarchicalPropertySetInfo()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setHierarchicalPropertyValue(
        OUString const & aHierarchicalPropertyName,
        com::sun::star::uno::Any const & aValue)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Any SAL_CALL getHierarchicalPropertyValue(
        OUString const & aHierarchicalPropertyName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setHierarchicalPropertyValues(
        com::sun::star::uno::Sequence< OUString > const &
            aHierarchicalPropertyNames,
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            Values)
        throw (
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Any > SAL_CALL
    getHierarchicalPropertyValues(
        com::sun::star::uno::Sequence< OUString > const &
            aHierarchicalPropertyNames)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByHierarchicalName(OUString const & aHierarchicalName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasPropertyByHierarchicalName(
        OUString const & aHierarchicalName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL replaceByName(
        OUString const & aName, com::sun::star::uno::Any const & aElement)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL insertByName(
        OUString const & aName, com::sun::star::uno::Any const & aElement)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::container::ElementExistException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeByName(OUString const & aName)
        throw (
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL createInstance()
        throw (
            com::sun::star::uno::Exception,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL createInstanceWithArguments(
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            aArguments)
        throw (
            com::sun::star::uno::Exception,
            com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    Access(Components & components);

    virtual ~Access();

    virtual OUString getNameInternal() = 0;
    virtual rtl::Reference< RootAccess > getRootAccess() = 0;
    virtual rtl::Reference< Access > getParentAccess() = 0;

    virtual void addTypes(std::vector< com::sun::star::uno::Type > * types)
        const = 0;

    virtual void addSupportedServiceNames(
        std::vector< OUString > * services) = 0;

    virtual void initDisposeBroadcaster(Broadcaster * broadcaster);
    virtual void clearListeners() throw ();

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & aType)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    Components & getComponents() const { return components_;}

    void checkLocalizedPropertyAccess();

    rtl::Reference< Node > getParentNode();
    rtl::Reference< ChildAccess > getChild(OUString const & name);
    std::vector< rtl::Reference< ChildAccess > > getAllChildren();

    void checkValue(
        com::sun::star::uno::Any const & value, Type type, bool nillable);

    void insertLocalizedValueChild(
        OUString const & name, com::sun::star::uno::Any const & value,
        Modifications * localModifications);

    void reportChildChanges(
        std::vector< com::sun::star::util::ElementChange > * changes);

    void commitChildChanges(bool valid, Modifications * globalModifications);

    void initBroadcasterAndChanges(
        Modifications::Node const & modifications, Broadcaster * broadcaster,
        std::vector< com::sun::star::util::ElementChange > * changes);

    bool isDisposed() const { return disposed_;}

private:
    struct ModifiedChild {
        rtl::Reference< ChildAccess > child;
        bool directlyModified;

        ModifiedChild();

        ModifiedChild(
            rtl::Reference< ChildAccess > const & theChild,
            bool theDirectlyModified);
    };

    typedef std::map< OUString, ModifiedChild, LengthContentsCompare > ModifiedChildren;

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

    rtl::Reference< ChildAccess > getFreeSetMember(
        com::sun::star::uno::Any const & value);

    rtl::Reference< Access > getNotificationRoot();

    typedef std::map< OUString, ChildAccess *, LengthContentsCompare > WeakChildMap;

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::lang::XEventListener > >
        DisposeListeners;

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::container::XContainerListener > >
        ContainerListeners;

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertyChangeListener > >
        PropertyChangeListenersElement;

    typedef std::map< OUString, PropertyChangeListenersElement, LengthContentsCompare >
        PropertyChangeListeners;

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::beans::XVetoableChangeListener > >
        VetoableChangeListenersElement;

    typedef std::map< OUString, VetoableChangeListenersElement, LengthContentsCompare >
        VetoableChangeListeners;

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertiesChangeListener > >
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

    boost::shared_ptr<osl::Mutex> lock_;

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
