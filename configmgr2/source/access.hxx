/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_ACCESS_HXX
#define INCLUDED_CONFIGMGR_ACCESS_HXX

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XExactName.hpp"
#include "com/sun/star/beans/XHierarchicalPropertySet.hpp"
#include "com/sun/star/beans/XMultiHierarchicalPropertySet.hpp"
#include "com/sun/star/beans/XMultiPropertySet.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XContainer.hpp"
#include "com/sun/star/container/XHierarchicalName.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/container/XNamed.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/util/ChangesSet.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/util/XChangesNotifier.hpp"
#include "osl/mutex.hxx"
#include "sal/types.h"

#if !defined INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 13
#include "comphelper/implbase_var.hxx"
#undef COMPHELPER_IMPLBASE_INTERFACE_NUMBER
#endif

namespace com { namespace sun { namespace star {
    namespace beans {
        class XHierarchicalPropertySetInfo;
        class XPropertiesChangeListener;
        class XPropertyChangeListener;
        class XVetoableChangeListener;
        struct Property;
    }
    namespace container { class XContainerListener; }
    namespace uno {
        class Any;
        class Type;
    }
    namespace util {
        class XChangesListener;
    }
} } }

namespace configmgr {

class Node;
class RootAccess;

class Access:
    private osl::Mutex,
    public comphelper::WeakComponentImplHelper13<
        com::sun::star::container::XHierarchicalNameAccess,
        com::sun::star::container::XContainer,
        com::sun::star::beans::XExactName,
        com::sun::star::beans::XPropertySetInfo,
        com::sun::star::container::XHierarchicalName,
        com::sun::star::container::XNamed,
        com::sun::star::beans::XPropertySet,
        com::sun::star::beans::XMultiPropertySet,
        com::sun::star::beans::XHierarchicalPropertySet,
        com::sun::star::beans::XMultiHierarchicalPropertySet,
        com::sun::star::util::XChangesNotifier,
        com::sun::star::container::XNameContainer,
        com::sun::star::util::XChangesBatch >,
    private boost::noncopyable
{
public:
    Access(RootAccess const * root, Node * node);

protected:
    Access(RootAccess const * root);

    virtual ~Access();

    virtual Node * getNode();

    Node * node_;

private:
    virtual com::sun::star::uno::Any SAL_CALL getByHierarchicalName(
        rtl::OUString const & aName)
        throw (
            com::sun::star::container::NoSuchElementException,
            com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByHierarchicalName(rtl::OUString const & aName)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addContainerListener(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & xListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeContainerListener(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & xListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getExactName(
        rtl::OUString const & aApproximateName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    SAL_CALL getProperties() throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::beans::Property SAL_CALL getPropertyByName(
        rtl::OUString const & aName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasPropertyByName(rtl::OUString const & Name)
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getHierarchicalName()
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL composeHierarchicalName(
        rtl::OUString const & aRelativeName)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::NoSupportException,
            com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getName()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setName(rtl::OUString const & aName)
        throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
    SAL_CALL getPropertySetInfo() throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValue(
        rtl::OUString const & aPropertyName,
        com::sun::star::uno::Any const & aValue)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue(
        rtl::OUString const & PropertyName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPropertyChangeListener(
        rtl::OUString const & aPropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > const & xListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener(
        rtl::OUString const & aPropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > const & aListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addVetoableChangeListener(
        rtl::OUString const & PropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XVetoableChangeListener > const & aListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener(
        rtl::OUString const & PropertyName,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XVetoableChangeListener > const & aListener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValues(
        com::sun::star::uno::Sequence< rtl::OUString > const & aPropertyNames,
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            aValues)
        throw (
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Any > SAL_CALL
    getPropertyValues(
        com::sun::star::uno::Sequence< rtl::OUString > const & aPropertyNames)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPropertiesChangeListener(
        com::sun::star::uno::Sequence< rtl::OUString > const & aPropertyNames,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const &
                xListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePropertiesChangeListener(
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const &
                xListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL firePropertiesChangeEvent(
        com::sun::star::uno::Sequence< rtl::OUString > const & aPropertyNames,
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const &
                xListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Reference<
        com::sun::star::beans::XHierarchicalPropertySetInfo > SAL_CALL
    getHierarchicalPropertySetInfo()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setHierarchicalPropertyValue(
        rtl::OUString const & aHierarchicalPropertyName,
        com::sun::star::uno::Any const & aValue)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Any SAL_CALL getHierarchicalPropertyValue(
        rtl::OUString const & aHierarchicalPropertyName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setHierarchicalPropertyValues(
        com::sun::star::uno::Sequence< rtl::OUString > const &
            aHierarchicalPropertyNames,
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            Values)
        throw (
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Any > SAL_CALL
    getHierarchicalPropertyValues(
        com::sun::star::uno::Sequence< rtl::OUString > const &
            aHierarchicalPropertyNames)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addChangesListener(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeChangesListener(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Type SAL_CALL getElementType()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasElements()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Any SAL_CALL getByName(
        rtl::OUString const & aName)
        throw (
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getElementNames() throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByName(rtl::OUString const & aName)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL replaceByName(
        rtl::OUString const & aName, com::sun::star::uno::Any const & aElement)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL insertByName(
        rtl::OUString const & aName, com::sun::star::uno::Any const & aElement)
        throw (
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::container::ElementExistException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeByName(rtl::OUString const & aName)
        throw (
            com::sun::star::container::NoSuchElementException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL commitChanges()
        throw (
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasPendingChanges()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::util::ChangesSet getPendingChanges()
        throw (com::sun::star::uno::RuntimeException);

    RootAccess const * root_;

#if OSL_DEBUG_LEVEL > 0
    enum {
        IS_GROUP = 0x01, IS_SET = 0x02, IS_GROUP_OR_SET = 0x04, IS_ROOT = 0x08,
        IS_GROUP_MEMBER = 0x10, IS_SET_MEMBER = 0x20, IS_UPDATE = 0x40 };
    bool thisIs(int what);
#endif
};

}

#endif
