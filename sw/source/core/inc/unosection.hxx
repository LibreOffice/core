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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOSECTION_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOSECTION_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextSection.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/Metadatable.hxx>

#include <unobaseclass.hxx>

class SwSectionFormat;

struct SwTextSectionProperties_Impl;

typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::beans::XPropertyState
,   ::com::sun::star::beans::XMultiPropertySet
,   ::com::sun::star::container::XNamed
,   ::com::sun::star::text::XTextSection
> SwXTextSection_Base;

class SwXTextSection
    : public SwXTextSection_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    SwXTextSection(SwSectionFormat *const pFormat, const bool bIndexHeader = false);

    virtual ~SwXTextSection();

public:

    SwSectionFormat*   GetFormat() const;

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextSection >
        CreateXTextSection(SwSectionFormat *const pFormat = 0,
                const bool bIndexHeader = false);

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject() override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModel() override;

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL
        getPropertyState(const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const ::com::sun::star::uno::Sequence<
                OUString >& rPropertyNames)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any >& rValues)
        throw (::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL getPropertyValues(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNamed
    virtual OUString SAL_CALL getName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName(const OUString& rName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextSection
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextSection > SAL_CALL
        getParentSection()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextSection >  > SAL_CALL
        getChildSections()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
