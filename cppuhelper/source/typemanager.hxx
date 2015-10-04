/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_SOURCE_TYPEMANAGER_HXX
#define INCLUDED_CPPUHELPER_SOURCE_TYPEMANAGER_HXX

#include <sal/config.h>

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/reflection/InvalidTypeNameException.hpp>
#include <com/sun/star/reflection/NoSuchTypeNameException.hpp>
#include <com/sun/star/reflection/TypeDescriptionSearchDepth.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace uno { class Any; }
    namespace reflection { class XTypeDescription; }
} } }
namespace rtl { class OUString; }
namespace unoidl {
    class ConstantGroupEntity;
    class Entity;
    class EnumTypeEntity;
    class Manager;
}

namespace cppuhelper {

typedef cppu::WeakComponentImplHelper<
    css::lang::XServiceInfo, css::container::XHierarchicalNameAccess,
    css::container::XSet, css::reflection::XTypeDescriptionEnumerationAccess >
TypeManager_Base;

class TypeManager: private cppu::BaseMutex, public TypeManager_Base {
public:
    TypeManager();

    using TypeManager_Base::acquire;
    using TypeManager_Base::release;

    void init(rtl::OUString const & rdbUris);

    css::uno::Any find(rtl::OUString const & name);

    css::uno::Reference< css::reflection::XTypeDescription > resolve(
        rtl::OUString const & name);

private:
    virtual ~TypeManager() throw ();

    virtual void SAL_CALL disposing() override;

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        rtl::OUString const & aName)
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasByHierarchicalName(rtl::OUString const & aName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
    createEnumeration() throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL has(css::uno::Any const & aElement)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL insert(css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL remove(css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
    SAL_CALL createTypeDescriptionEnumeration(
        rtl::OUString const & moduleName,
        css::uno::Sequence< css::uno::TypeClass > const & types,
        css::reflection::TypeDescriptionSearchDepth depth)
        throw (
            css::reflection::NoSuchTypeNameException,
            css::reflection::InvalidTypeNameException,
            css::uno::RuntimeException, std::exception) override;

    void readRdbs(rtl::OUString const & uris);

    void readRdbDirectory(rtl::OUString const & uri, bool optional);

    void readRdbFile(rtl::OUString const & uri, bool optional);

    css::uno::Any getSequenceType(rtl::OUString const & name);

    css::uno::Any getInstantiatedStruct(
        rtl::OUString const & name, sal_Int32 separator);

    css::uno::Any getInterfaceMember(
        rtl::OUString const & name, sal_Int32 separator);

    css::uno::Any getNamed(
        rtl::OUString const & name,
        rtl::Reference< unoidl::Entity > const & entity);

    static css::uno::Any getEnumMember(
        rtl::Reference< unoidl::EnumTypeEntity > const & entity,
        rtl::OUString const & member);

    static css::uno::Any getConstant(
        rtl::OUString const & constantGroupName,
        rtl::Reference< unoidl::ConstantGroupEntity > const & entity,
        rtl::OUString const & member);

    rtl::Reference< unoidl::Entity > findEntity(rtl::OUString const & name);

    rtl::Reference< unoidl::Manager > manager_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
