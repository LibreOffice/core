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
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include "formcontrolfont.hxx"

#include <set>


namespace frm
{

    typedef ::cppu::ImplHelper  <   css::awt::XControlModel
                                > ONavigationBarModel_BASE;

    class ONavigationBarModel
                        :public OControlModel
                        ,public FontControlModel
                        ,public OPropertyContainerHelper
                        ,public ONavigationBarModel_BASE
    {
        // <properties>
        css::uno::Any                m_aTabStop;
        css::uno::Any                m_aBackgroundColor;
        OUString                     m_sDefaultControl;
        OUString                     m_sHelpText;
        OUString                     m_sHelpURL;
        sal_Int16                    m_nIconSize;
        sal_Int16                    m_nBorder;
        sal_Int32                    m_nDelay;
        bool                         m_bEnabled;
        bool                         m_bEnableVisible;
        bool                         m_bShowPosition;
        bool                         m_bShowNavigation;
        bool                         m_bShowActions;
        bool                         m_bShowFilterSort;
        sal_Int16                    m_nWritingMode;
        sal_Int16                    m_nContextWritingMode;
        // </properties>

    public:
        DECLARE_DEFAULT_LEAF_XTOR( ONavigationBarModel );

        // XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarModel, OControlModel )
        virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()  throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // XPersistObject
        virtual OUString SAL_CALL getServiceName() throw ( css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) override;

        // XPropertySet
        virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                              sal_Int32 nHandle, const css::uno::Any& rValue )
                                            throw(css::lang::IllegalArgumentException) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) throw ( css::uno::Exception, std::exception) override;

        // XPropertyState
        virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
        ) const override;

        // prevent method hiding
        using OControlModel::disposing;
        using OControlModel::getFastPropertyValue;

    protected:
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        void implInitPropertyContainer();
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_NAVIGATIONBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
