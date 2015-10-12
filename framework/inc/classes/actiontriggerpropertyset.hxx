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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_ACTIONTRIGGERPROPERTYSET_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_ACTIONTRIGGERPROPERTYSET_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <framework/fwedllapi.h>

#define SERVICENAME_ACTIONTRIGGER "com.sun.star.ui.ActionTrigger"
#define IMPLEMENTATIONNAME_ACTIONTRIGGER "com.sun.star.comp.ui.ActionTrigger"

namespace framework
{

class ActionTriggerPropertySet :  private cppu::BaseMutex,
                                    public ::com::sun::star::lang::XServiceInfo ,
                                    public ::com::sun::star::lang::XTypeProvider,
                                    public ::cppu::OBroadcastHelper             ,
                                    public ::cppu::OPropertySetHelper           ,   // -> XPropertySet, XFastPropertySet, XMultiPropertySet
                                    public ::cppu::OWeakObject
{
    public:
        FWE_DLLPUBLIC ActionTriggerPropertySet();
        FWE_DLLPUBLIC virtual ~ActionTriggerPropertySet();

        // XInterface
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual FWE_DLLPUBLIC void SAL_CALL acquire() throw () override;
        virtual FWE_DLLPUBLIC void SAL_CALL release() throw () override;

        // XServiceInfo
        virtual FWE_DLLPUBLIC OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual FWE_DLLPUBLIC sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:

        //  OPropertySetHelper

        virtual sal_Bool SAL_CALL convertFastPropertyValue( com::sun::star::uno::Any&       aConvertedValue,
                                                            com::sun::star::uno::Any&       aOldValue,
                                                            sal_Int32                       nHandle,
                                                            const com::sun::star::uno::Any& aValue          )
            throw( com::sun::star::lang::IllegalArgumentException ) override;

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any& aValue )
            throw( com::sun::star::uno::Exception, std::exception ) override;

        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void SAL_CALL getFastPropertyValue( com::sun::star::uno::Any& aValue, sal_Int32 nHandle ) const override;

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

        //  helper

        bool impl_tryToChangeProperty(  const   OUString&              aCurrentValue   ,
                                            const   com::sun::star::uno::Any&   aNewValue       ,
                                            com::sun::star::uno::Any&           aOldValue       ,
                                            com::sun::star::uno::Any&           aConvertedValue ) throw( com::sun::star::lang::IllegalArgumentException );

        bool impl_tryToChangeProperty(  const   com::sun::star::uno::Reference< com::sun::star::awt::XBitmap >  xBitmap,
                                            const   com::sun::star::uno::Any&   aNewValue       ,
                                            com::sun::star::uno::Any&           aOldValue       ,
                                            com::sun::star::uno::Any&           aConvertedValue ) throw( com::sun::star::lang::IllegalArgumentException );

        bool impl_tryToChangeProperty(  const   com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xInterface,
                                            const   com::sun::star::uno::Any&   aNewValue       ,
                                            com::sun::star::uno::Any&           aOldValue       ,
                                            com::sun::star::uno::Any&           aConvertedValue ) throw( com::sun::star::lang::IllegalArgumentException );

        //  members

        OUString                                                         m_aCommandURL;
        OUString                                                         m_aHelpURL;
        OUString                                                         m_aText;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >    m_xBitmap;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xActionTriggerContainer;
};

}

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_ACTIONTRIGGERPROPERTYSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
