/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>

#include <vector>
#include <o3tl/sorted_vector.hxx>
#include "config_map.hxx"

#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/configuration/XDocumentation.hpp>
#include <com/sun/star/container/XHierarchicalNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/interlck.h>
#include <rtl/ref.hxx>
#include <sal/types.h>

#include "modifications.hxx"
#include "type.hxx"

namespace com::sun::star {
    namespace beans {
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
    }
    namespace util { struct ElementChange; }
}

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
    public css::configuration::XDocumentation,
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

    virtual std::vector<OUString> getAbsolutePath() = 0;
    virtual std::vector<OUString> getRelativePath() = 0;

    virtual OUString getRelativePathRepresentation() = 0;
    virtual const rtl::Reference< Node > & getNode() = 0;

    virtual bool isFinalized() = 0;

    virtual void initBroadcaster(
        Modifications::Node const & modifications, Broadcaster * broadcaster);

    using OWeakObject::acquire;
    using OWeakObject::release;

    virtual cpo::uno::Sequence< cpo::uno::Type >
    getTypes() override;

    virtual cpo::uno::Sequence< sal_Int8 >
    getImplementationId() override;

    virtual OUString getImplementationName() override;

    virtual bool supportsService(OUString const & ServiceName) override;

    virtual cpo::uno::Sequence< OUString >
    getSupportedServiceNames() override;

    virtual void dispose() override;

    virtual void addEventListener(
        css::uno::Reference< css::lang::XEventListener >
            const & xListener) override;

    virtual void removeEventListener(
        css::uno::Reference< css::lang::XEventListener >
            const & aListener) override;

    virtual cpo::uno::Type getElementType() override;

    virtual bool hasElements() override;

    virtual cpo::uno::Any getByName(
        OUString const & aName) override;

    virtual cpo::uno::Sequence< OUString >
    getElementNames() override;

    virtual bool hasByName(OUString const & aName) override;

    virtual cpo::uno::Any getByHierarchicalName(
        OUString const & aName) override;

    virtual OUString getDescriptionByHierarchicalName(
        OUString const & aName) override;

    virtual cpo::uno::Type getTypeByHierarchicalName(
        OUString const & aName) override;

    virtual bool getModifiedByHierarchicalName(
        OUString const & aName) override;

    virtual bool hasByHierarchicalName(OUString const & aName) override;

    virtual void replaceByHierarchicalName(
        OUString const & aName, cpo::uno::Any const & aElement) override;

    virtual void addContainerListener(
        css::uno::Reference<
            css::container::XContainerListener > const & xListener) override;

    virtual void removeContainerListener(
        css::uno::Reference<
            css::container::XContainerListener > const & xListener) override;

    virtual OUString getExactName(
        OUString const & aApproximateName) override;

    virtual cpo::uno::Sequence< css::beans::Property >
    getProperties() override;

    virtual css::beans::Property getPropertyByName(
        OUString const & aName) override;

    virtual bool hasPropertyByName(OUString const & Name) override;

    virtual OUString getHierarchicalName() override;

    virtual OUString composeHierarchicalName(
        OUString const & aRelativeName) override;

    virtual OUString getName() override;

    virtual void setName(OUString const & aName) override;

    virtual css::beans::Property getAsProperty() override;

    virtual
    css::uno::Reference< css::beans::XPropertySetInfo >
    getPropertySetInfo() override;

    virtual void setPropertyValue(
        OUString const & aPropertyName,
        cpo::uno::Any const & aValue) override;

    virtual cpo::uno::Any getPropertyValue(
        OUString const & PropertyName) override;

    virtual void addPropertyChangeListener(
        OUString const & aPropertyName,
        css::uno::Reference<
            css::beans::XPropertyChangeListener > const & xListener) override;

    virtual void removePropertyChangeListener(
        OUString const & aPropertyName,
        css::uno::Reference<
            css::beans::XPropertyChangeListener > const & aListener) override;

    virtual void addVetoableChangeListener(
        OUString const & PropertyName,
        css::uno::Reference<
            css::beans::XVetoableChangeListener > const & aListener) override;

    virtual void removeVetoableChangeListener(
        OUString const & PropertyName,
        css::uno::Reference<
            css::beans::XVetoableChangeListener > const & aListener) override;

    virtual void setPropertyValues(
        cpo::uno::Sequence< OUString > const & aPropertyNames,
        cpo::uno::Sequence< cpo::uno::Any > const &
            aValues) override;

    virtual cpo::uno::Sequence< cpo::uno::Any >
    getPropertyValues(
        cpo::uno::Sequence< OUString > const & aPropertyNames) override;

    virtual void addPropertiesChangeListener(
        cpo::uno::Sequence< OUString > const & aPropertyNames,
        css::uno::Reference<
            css::beans::XPropertiesChangeListener > const &
                xListener) override;

    virtual void removePropertiesChangeListener(
        css::uno::Reference<
            css::beans::XPropertiesChangeListener > const &
                xListener) override;

    virtual void firePropertiesChangeEvent(
        cpo::uno::Sequence< OUString > const & aPropertyNames,
        css::uno::Reference<
            css::beans::XPropertiesChangeListener > const &
                xListener) override;

    virtual
    css::uno::Reference<
        css::beans::XHierarchicalPropertySetInfo >
    getHierarchicalPropertySetInfo() override;

    virtual void setHierarchicalPropertyValue(
        OUString const & aHierarchicalPropertyName,
        cpo::uno::Any const & aValue) override;

    virtual cpo::uno::Any getHierarchicalPropertyValue(
        OUString const & aHierarchicalPropertyName) override;

    virtual void setHierarchicalPropertyValues(
        cpo::uno::Sequence< OUString > const &
            aHierarchicalPropertyNames,
        cpo::uno::Sequence< cpo::uno::Any > const &
            Values) override;

    virtual cpo::uno::Sequence< cpo::uno::Any >
    getHierarchicalPropertyValues(
        cpo::uno::Sequence< OUString > const &
            aHierarchicalPropertyNames) override;

    virtual css::beans::Property
    getPropertyByHierarchicalName(OUString const & aHierarchicalName) override;

    virtual bool hasPropertyByHierarchicalName(
        OUString const & aHierarchicalName) override;

    virtual void replaceByName(
        OUString const & aName, cpo::uno::Any const & aElement) override;

    virtual void insertByName(
        OUString const & aName, cpo::uno::Any const & aElement) override;

    virtual void removeByName(OUString const & aName) override;

    virtual css::uno::Reference< css::uno::XInterface >
    createInstance() override;

    virtual css::uno::Reference< css::uno::XInterface >
    createInstanceWithArguments(
        cpo::uno::Sequence< cpo::uno::Any > const &
            aArguments) override;

protected:
    explicit Access(Components & components);

    virtual ~Access() override;

    virtual const OUString & getNameInternal() = 0;
    virtual rtl::Reference< RootAccess > getRootAccess() = 0;
    virtual rtl::Reference< Access > getParentAccess() = 0;

    virtual void addTypes(std::vector< cpo::uno::Type > * types)
        const = 0;

    virtual void addSupportedServiceNames(
        std::vector<OUString> * services) = 0;

    virtual void initDisposeBroadcaster(Broadcaster * broadcaster);
    virtual void clearListeners() noexcept;

    virtual cpo::uno::Any queryInterface(
        cpo::uno::Type const & aType) override;

    Components & getComponents() const { return components_;}

    void checkLocalizedPropertyAccess();

    rtl::Reference< Node > getParentNode();
    rtl::Reference< ChildAccess > getChild(OUString const & name);
    void forAllChildren(const std::function<bool(ChildAccess&)> & f);
    bool isAllChildrenEmpty();

    void checkValue(
        cpo::uno::Any const & value, Type type, bool nillable);

    void insertLocalizedValueChild(
        OUString const & name, cpo::uno::Any const & value,
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
            rtl::Reference< ChildAccess > theChild,
            bool theDirectlyModified);
    };

    typedef config_map< ModifiedChild > ModifiedChildren;

    rtl::Reference< ChildAccess > getModifiedChild(
        ModifiedChildren::iterator const & childIterator);

    rtl::Reference< ChildAccess > getUnmodifiedChild(
        OUString const & name);

    rtl::Reference< ChildAccess > getSubChild(OUString const & path);

    bool setChildProperty(
        OUString const & name, cpo::uno::Any const & value,
        Modifications * localModifications);

    css::beans::Property asProperty();

    bool getByNameFast(const OUString & name, cpo::uno::Any & value);
    rtl::Reference< ChildAccess > createUnmodifiedChild(const OUString &name,
                                                        const rtl::Reference< Node > &node);

    void checkFinalized();

    void checkKnownProperty(OUString const & descriptor);

    rtl::Reference< ChildAccess > getFreeSetMember( cpo::uno::Any const & value);

    rtl::Reference< Access > getNotificationRoot();

    typedef config_map< ChildAccess * > WeakChildMap;

    typedef
        o3tl::sorted_vector<
            css::uno::Reference<
                css::lang::XEventListener > >
        DisposeListeners;

    typedef
        o3tl::sorted_vector<
            css::uno::Reference<
                css::container::XContainerListener > >
        ContainerListeners;

    typedef
        o3tl::sorted_vector<
            css::uno::Reference<
                css::beans::XPropertyChangeListener > >
        PropertyChangeListenersElement;

    typedef config_map< PropertyChangeListenersElement >
        PropertyChangeListeners;

    typedef
        o3tl::sorted_vector<
            css::uno::Reference<
                css::beans::XVetoableChangeListener > >
        VetoableChangeListenersElement;

    typedef config_map< VetoableChangeListenersElement >
        VetoableChangeListeners;

    typedef
        o3tl::sorted_vector<
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
