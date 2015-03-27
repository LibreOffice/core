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


    #define DECLARE_SERVICE_REGISTRATION( classname ) \
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE; \
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE; \
        \
        static  OUString SAL_CALL getImplementationName_Static(); \
        static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(); \

    #define IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
        OUString SAL_CALL classname::getImplementationName(  ) throw ( RuntimeException, std::exception ) \
        { return getImplementationName_Static(); } \
        \
        Sequence< OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw (RuntimeException, std::exception) \
        { \
            return ::comphelper::combineSequences( \
                getAggregateServiceNames(), \
                getSupportedServiceNames_Static() \
            ); \
        } \
        \
        OUString SAL_CALL classname::getImplementationName_Static() \
        { return OUString( "com.sun.star.comp.forms."#classname ); } \
        \

    #define IMPLEMENT_SERVICE_REGISTRATION_2( classname, baseclass, service1, service2 ) \
        IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
        Sequence< OUString > SAL_CALL classname::getSupportedServiceNames_Static() \
        { \
            Sequence< OUString > aOwnNames( 2 ); \
            aOwnNames[ 0 ] = service1; \
            aOwnNames[ 1 ] = service2; \
            \
            return ::comphelper::concatSequences( \
                baseclass::getSupportedServiceNames_Static(), \
                aOwnNames \
            ); \
        } \

    #define IMPLEMENT_SERVICE_REGISTRATION_8( classname, baseclass, service1, service2, service3, service4 , service5, service6, service7, service8 ) \
        IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
           Sequence< OUString > SAL_CALL classname::getSupportedServiceNames_Static() \
           { \
                   Sequence< OUString > aOwnNames( 8 ); \
                   aOwnNames[ 0 ] = service1; \
                   aOwnNames[ 1 ] = service2; \
                   aOwnNames[ 2 ] = service3; \
                   aOwnNames[ 3 ] = service4; \
                   aOwnNames[ 4 ] = service5; \
                   aOwnNames[ 5 ] = service6; \
                   aOwnNames[ 6 ] = service7; \
                   aOwnNames[ 7 ] = service8; \
            \
            return ::comphelper::concatSequences( \
                baseclass::getSupportedServiceNames_Static(), \
                aOwnNames \
            ); \
           } \


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
