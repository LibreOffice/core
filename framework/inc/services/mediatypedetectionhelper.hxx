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

#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_MEDIATYPEDETECTIONHELPER_HXX
#define INCLUDED_FRAMEWORK_INC_SERVICES_MEDIATYPEDETECTIONHELPER_HXX

#include <macros/xserviceinfo.hxx>
#include <general.h>

#include <com/sun/star/util/XStringMapping.hpp>

#include <cppuhelper/implbase.hxx>

//  namespaces

namespace framework{

/*-************************************************************************************************************
    @implements     XInterface
                    XTypeProvider
                    XServiceInfo
                    XStringMapping
    @base           OWeakObject

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class MediaTypeDetectionHelper  :   public ::cppu::WeakImplHelper< css::util::XStringMapping, css::lang::XServiceInfo>
{

    //  public methods

    public:

        //  constructor / destructor

        /*-****************************************************************************************************
            @short      standard ctor
            @descr      These initialize a new instance of this class with all needed information for work.
            @param      "xFactory", reference to factory which has created our owner(!). We can use these to create new uno-services.
        *//*-*****************************************************************************************************/

         MediaTypeDetectionHelper( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

        /*-****************************************************************************************************
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
        *//*-*****************************************************************************************************/

        virtual ~MediaTypeDetectionHelper() override;

        //  XInterface, XTypeProvider, XServiceInfo

        DECLARE_XSERVICEINFO_NOFACTORY
        /* Helper for registry */
        /// @throws css::uno::Exception
        static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
        static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

        //  XStringMapping

        /*-****************************************************************************************************
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL mapStrings(css::uno::Sequence< OUString >& seqParameter) override;

};

}       // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_MEDIATYPEDETECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
