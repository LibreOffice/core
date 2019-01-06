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

#ifndef INCLUDED_SD_SOURCE_UI_UNOIDL_SDDETECT_HXX
#define INCLUDED_SD_SOURCE_UI_UNOIDL_SDDETECT_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>


namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

class SdFilterDetect : public ::cppu::WeakImplHelper< css::document::XExtendedFilterDetection, css::lang::XServiceInfo >
{
public:
                            SdFilterDetect();
    virtual                 ~SdFilterDetect() override;

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XExtendedFilterDetect

    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& lDescriptor ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
