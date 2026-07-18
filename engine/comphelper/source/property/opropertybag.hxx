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

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/beans/XPropertyBag.hpp>
#include <com/sun/star/container/XSet.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/propstate.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertybag.hxx>
#include <comphelper/uno3.hxx>

#include <map>
#include <set>
#include <memory>


namespace comphelper
{


    struct UnoTypeLess
    {
        bool operator()( const cpo::uno::Type& _rLHS, const cpo::uno::Type& _rRHS ) const
        {
            return rtl_ustr_compare(
                _rLHS.getTypeLibType()->pTypeName->buffer,
                _rRHS.getTypeLibType()->pTypeName->buffer
            ) < 0;
        }
    };

    typedef std::map< sal_Int32, cpo::uno::Any >     MapInt2Any;
    typedef std::set< cpo::uno::Type, UnoTypeLess >  TypeBag;

    typedef ::cppu::WeakImplHelper  <   css::beans::XPropertyBag
                                    ,   css::util::XModifiable
                                    ,   css::lang::XServiceInfo
                                    ,   css::lang::XInitialization
                                    ,   css::container::XSet
                                    >   OPropertyBag_Base;
    typedef ::comphelper::OPropertyStateHelper  OPropertyBag_PBase;

    class OPropertyBag final : public ::comphelper::OMutexAndBroadcastHelper  // must be before OPropertyBag_PBase
                        ,public OPropertyBag_PBase
                        ,public OPropertyBag_Base
                        ,public ::cppu::IEventNotificationHook
    {
    private:
        /// our IPropertyArrayHelper implementation
        std::unique_ptr< ::cppu::OPropertyArrayHelper >
                        m_pArrayHelper;
        ::comphelper::PropertyBag
                        m_aDynamicProperties;
        /// set of allowed property types
        TypeBag         m_aAllowedTypes;
        /// should we automatically add properties which are tried to set, if they don't exist previously?
        bool            m_bAutoAddProperties;

        /// for notification
        ::comphelper::OInterfaceContainerHelper3<css::util::XModifyListener> m_NotifyListeners;
        /// modify flag
        bool            m_isModified;

    public:
        //noncopyable
        OPropertyBag(const OPropertyBag&) = delete;
        const OPropertyBag& operator=(const OPropertyBag&) = delete;
        OPropertyBag();
        virtual ~OPropertyBag() override;

    private:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        /** === begin UNO interface implementations == **/
        // XInitialization
        virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

        // XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

        // XModifiable:
        virtual bool isModified(  ) override;
        virtual void setModified( bool bModified ) override;

        // XModifyBroadcaster
        virtual void addModifyListener(
            const css::uno::Reference<
                    css::util::XModifyListener > & xListener) override;
        virtual void removeModifyListener(
            const css::uno::Reference<
                    css::util::XModifyListener > & xListener) override;

        // XPropertyContainer
        virtual void addProperty( const OUString& Name, ::sal_Int16 Attributes, const cpo::uno::Any& DefaultValue ) override;
        virtual void removeProperty( const OUString& Name ) override;

        // XPropertyAccess
        virtual cpo::uno::Sequence< css::beans::PropertyValue > getPropertyValues(  ) override;
        virtual void setPropertyValues( const cpo::uno::Sequence< css::beans::PropertyValue >& aProps ) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
        virtual void setPropertyValue(const OUString& p1, const cpo::uno::Any& p2) override
           { OPropertyBag_PBase::setPropertyValue(p1, p2); }
        virtual cpo::uno::Any getPropertyValue(const OUString& p1) override
           { return OPropertyBag_PBase::getPropertyValue(p1); }
        virtual void addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
           { OPropertyBag_PBase::addPropertyChangeListener(p1, p2); }
        virtual void removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
           { OPropertyBag_PBase::removePropertyChangeListener(p1, p2); }
        virtual void addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
           { OPropertyBag_PBase::addVetoableChangeListener(p1, p2); }
        virtual void removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
           { OPropertyBag_PBase::removeVetoableChangeListener(p1, p2); }

        // XSet
        virtual bool has( const cpo::uno::Any& aElement ) override;
        virtual void insert( const cpo::uno::Any& aElement ) override;
        virtual void remove( const cpo::uno::Any& aElement ) override;

        // XEnumerationAccess (base of XSet)
        virtual css::uno::Reference< css::container::XEnumeration > createEnumeration(  ) override;

        // XElementAccess (base of XEnumerationAccess)
        virtual cpo::uno::Type getElementType(  ) override;
        virtual bool hasElements(  ) override;
        // UNO interface implementations

        // XPropertyState
        virtual cpo::uno::Any  getPropertyDefaultByHandle( sal_Int32 _nHandle ) const override;

        // OPropertyStateHelper
        virtual css::beans::PropertyState  getPropertyStateByHandle( sal_Int32 _nHandle ) override;

        // OPropertySetHelper
        virtual void getFastPropertyValue( cpo::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual bool convertFastPropertyValue( cpo::uno::Any & rConvertedValue, cpo::uno::Any & rOldValue, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
        virtual void setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
        virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

        // IEventNotificationHook
        virtual void fireEvents(
            sal_Int32 * pnHandles,
            sal_Int32 nCount,
            bool bVetoable,
            bool bIgnoreRuntimeExceptionsWhileFiring) override;

        void setModifiedImpl( bool bModified,
            bool bIgnoreRuntimeExceptionsWhileFiring);

        /** finds a free property handle
            @precond
                our mutex is locked
        */
        sal_Int32   findFreeHandle() const;

        /** implements the setPropertyValues method
            @param _rProps
                the property values to set

            @throws PropertyVetoException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws css::lang::IllegalArgumentException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws css::lang::WrappedTargetException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws css::uno::RuntimeException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws css::beans::UnknownPropertyException
                if the XMultiPropertySet::setPropertyValues call does so, and <arg>_bTolerateUnknownProperties</arg>
                was set to <FALSE/>

            @throws css::lang::WrappedTargetException
                if the XMultiPropertySet::setPropertyValues call did throw an exception not listed
                above
        */
        void impl_setPropertyValues_throw( const cpo::uno::Sequence< css::beans::PropertyValue >& _rProps );

        using ::cppu::OPropertySetHelper::getPropertyValues;
        using ::cppu::OPropertySetHelper::setPropertyValues;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };


} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
