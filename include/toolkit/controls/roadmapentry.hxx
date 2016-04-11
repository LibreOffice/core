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
#ifndef INCLUDED_TOOLKIT_CONTROLS_ROADMAPENTRY_HXX
#define INCLUDED_TOOLKIT_CONTROLS_ROADMAPENTRY_HXX

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>


#define RM_PROPERTY_ID_LABEL        1
#define RM_PROPERTY_ID_ID           2
#define RM_PROPERTY_ID_ENABLED      4
#define RM_PROPERTY_ID_INTERACTIVE  5

typedef ::cppu::WeakImplHelper  <   css::lang::XServiceInfo
                                >   ORoadmapEntry_Base;

class ORoadmapEntry :public ORoadmapEntry_Base
            ,public ::comphelper::OMutexAndBroadcastHelper
            ,public ::comphelper::OPropertyContainer
            ,public ::comphelper::OPropertyArrayUsageHelper< ORoadmapEntry >
{

public:
       ORoadmapEntry();

protected:
    DECLARE_XINTERFACE()        // merge XInterface implementations
    DECLARE_XTYPEPROVIDER()     // merge XTypeProvider implementations

    /// @see css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
        SAL_CALL getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;


    // other stuff

    // (e.g. DECLARE_SERVICE_INFO();)

protected:
    // <properties>
    OUString     m_sLabel;
    sal_Int32           m_nID;
    bool            m_bEnabled;
    bool            m_bInteractive;

    // </properties>
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
