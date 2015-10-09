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

#ifndef INCLUDED_COMPHELPER_SOURCE_PROPERTY_OPROPERTYBAG_HXX
#define INCLUDED_COMPHELPER_SOURCE_PROPERTY_OPROPERTYBAG_HXX

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/beans/XPropertyBag.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XSet.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/propstate.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertybag.hxx>
#include <comphelper/uno3.hxx>

#include <map>
#include <set>
#include <memory>


namespace comphelper
{


    struct UnoTypeLess : public ::std::unary_function< css::uno::Type, bool >
    {
        inline bool operator()( const css::uno::Type& _rLHS, const css::uno::Type& _rRHS ) const
        {
            return rtl_ustr_compare(
                _rLHS.getTypeLibType()->pTypeName->buffer,
                _rRHS.getTypeLibType()->pTypeName->buffer
            ) < 0;
        }
    };

    typedef ::std::map< sal_Int32, css::uno::Any >     MapInt2Any;
    typedef ::std::set< css::uno::Type, UnoTypeLess >  TypeBag;

    typedef ::cppu::WeakAggImplHelper   <   css::beans::XPropertyBag
                                        ,   css::util::XModifiable
                                        ,   css::lang::XServiceInfo
                                        ,   css::lang::XInitialization
                                        ,   css::container::XSet
                                        >   OPropertyBag_Base;
    typedef ::comphelper::OPropertyStateHelper  OPropertyBag_PBase;

    class OPropertyBag  :public ::comphelper::OMutexAndBroadcastHelper  // must be before OPropertyBag_PBase
                        ,public OPropertyBag_PBase
                        ,public OPropertyBag_Base
                        ,public ::cppu::IEventNotificationHook
    {
    private:
        /// our IPropertyArrayHelper implementation
        ::std::unique_ptr< ::cppu::OPropertyArrayHelper >
                        m_pArrayHelper;
        ::comphelper::PropertyBag
                        m_aDynamicProperties;
        /// set of allowed property types
        TypeBag         m_aAllowedTypes;
        /// should we automatically add properties which are tried to set, if they don't exist previously?
        bool            m_bAutoAddProperties;

        /// for notification
        ::comphelper::OInterfaceContainerHelper2 m_NotifyListeners;
        /// modify flag
        bool            m_isModified;

    public:
        //noncopyable
        OPropertyBag(const OPropertyBag&) = delete;
        const OPropertyBag& operator=(const OPropertyBag&) = delete;
        OPropertyBag();
        virtual ~OPropertyBag() override;

    protected:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        /** === begin UNO interface implementations == **/
        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XModifiable:
        virtual sal_Bool SAL_CALL isModified(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setModified( sal_Bool bModified )
            throw (css::beans::PropertyVetoException,
                    css::uno::RuntimeException, std::exception) override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener(
            const css::uno::Reference<
                    css::util::XModifyListener > & xListener)
            throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeModifyListener(
            const css::uno::Reference<
                    css::util::XModifyListener > & xListener)
            throw (css::uno::RuntimeException, std::exception) override;

        // XPropertyContainer
        virtual void SAL_CALL addProperty( const OUString& Name, ::sal_Int16 Attributes, const css::uno::Any& DefaultValue ) throw (css::beans::PropertyExistException, css::beans::IllegalTypeException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeProperty( const OUString& Name ) throw (css::beans::UnknownPropertyException, css::beans::NotRemoveableException, css::uno::RuntimeException, std::exception) override;

        // XPropertyAccess
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue(const rtl::OUString& p1, const css::uno::Any& p2) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
           { OPropertyBag_PBase::setPropertyValue(p1, p2); }
        virtual css::uno::Any SAL_CALL getPropertyValue(const rtl::OUString& p1) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
           { return OPropertyBag_PBase::getPropertyValue(p1); }
        virtual void SAL_CALL addPropertyChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
           { OPropertyBag_PBase::addPropertyChangeListener(p1, p2); }
        virtual void SAL_CALL removePropertyChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
           { OPropertyBag_PBase::removePropertyChangeListener(p1, p2); }
        virtual void SAL_CALL addVetoableChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
           { OPropertyBag_PBase::addVetoableChangeListener(p1, p2); }
        virtual void SAL_CALL removeVetoableChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
           { OPropertyBag_PBase::removeVetoableChangeListener(p1, p2); }

        // XSet
        virtual sal_Bool SAL_CALL has( const css::uno::Any& aElement ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL insert( const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL remove( const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

        // XEnumerationAccess (base of XSet)
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XElementAccess (base of XEnumerationAccess)
        virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;
        // UNO interface implementations

        // XPropertyState
        virtual css::uno::Any  getPropertyDefaultByHandle( sal_Int32 _nHandle ) const override;

        // OPropertyStateHelper
        virtual css::beans::PropertyState  getPropertyStateByHandle( sal_Int32 _nHandle ) override;

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::lang::IllegalArgumentException, css::beans::UnknownPropertyException) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // IEventNotificationHook
        virtual void fireEvents(
            sal_Int32 * pnHandles,
            sal_Int32 nCount,
            sal_Bool bVetoable,
            bool bIgnoreRuntimeExceptionsWhileFiring) override;

        void SAL_CALL setModifiedImpl( bool bModified,
            bool bIgnoreRuntimeExceptionsWhileFiring);

    private:
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
        void impl_setPropertyValues_throw( const css::uno::Sequence< css::beans::PropertyValue >& _rProps );

    protected:
        using ::cppu::OPropertySetHelper::getPropertyValues;
        using ::cppu::OPropertySetHelper::setPropertyValues;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_SOURCE_PROPERTY_OPROPERTYBAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
