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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_NAVIGATIONBAR_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_NAVIGATIONBAR_HXX

#include "FormComponent.hxx"
#include <com/sun/star/io/XPersistObject.hpp>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainerhelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include "formcontrolfont.hxx"

#include <set>


namespace frm
{



    // ONavigationBarModel

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XControlModel
                                > ONavigationBarModel_BASE;

    class ONavigationBarModel
                        :public OControlModel
                        ,public FontControlModel
                        ,public OPropertyContainerHelper
                        ,public ONavigationBarModel_BASE
    {
        // <properties>
        ::com::sun::star::uno::Any          m_aTabStop;
        ::com::sun::star::uno::Any          m_aBackgroundColor;
        OUString                     m_sDefaultControl;
        OUString                     m_sHelpText;
        OUString                     m_sHelpURL;
        sal_Int16                           m_nIconSize;
        sal_Int16                           m_nBorder;
        sal_Int32                           m_nDelay;
        sal_Bool                            m_bEnabled;
        sal_Bool                            m_bEnableVisible;
        sal_Bool                            m_bShowPosition;
        sal_Bool                            m_bShowNavigation;
        sal_Bool                            m_bShowActions;
        sal_Bool                            m_bShowFilterSort;
        sal_Int16                           m_nWritingMode;
        sal_Int16                           m_nContextWritingMode;
        // </properties>

    public:
        DECLARE_DEFAULT_LEAF_XTOR( ONavigationBarModel );

        // XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarModel, OControlModel )
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // OComponentHelper
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // XPersistObject
        virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertySet
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                              sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

        // XPropertyState
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const SAL_OVERRIDE;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const SAL_OVERRIDE;

        // prevent method hiding
        using OControlModel::disposing;
        using OControlModel::getFastPropertyValue;

    protected:
        DECLARE_XCLONEABLE();

    private:
        void implInitPropertyContainer();
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_NAVIGATIONBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
