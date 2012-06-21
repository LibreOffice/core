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

#ifndef _PPTIMPORTER_HXX
#define _PPTIMPORTER_HXX

#include "pptcom.hxx"
#include "ppttoxml.hxx"

// ---------------
// - PptImporter -
// ---------------

class PptImporter : public NMSP_CPPU::WeakImplHelper4
<
    NMSP_DOCUMENT::XFilter,
    NMSP_DOCUMENT::XImporter,
    NMSP_LANG::XInitialization,
    NMSP_LANG::XServiceInfo
>
{
    PptToXml                                aFilter;
    REF( NMSP_SAX::XDocumentHandler )       xHdl;
    REF( NMSP_LANG::XMultiServiceFactory )  xFact;
    REF( NMSP_DOCUMENT::XImporter )         xImporter;

public:

                                            PptImporter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                                 ~PptImporter();

    // XInterface
    virtual void SAL_CALL                   acquire() throw();
    virtual void SAL_CALL                   release() throw();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const SEQ( NMSP_BEANS::PropertyValue )& aDescriptor )
        throw ( NMSP_UNO::RuntimeException );
    virtual void SAL_CALL cancel(  )
        throw ( NMSP_UNO::RuntimeException );

    // XImporter
    virtual void SAL_CALL setTargetDocument( const REF(NMSP_LANG::XComponent)& xDoc )
        throw ( NMSP_LANG::IllegalArgumentException, NMSP_UNO::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const SEQ( NMSP_UNO::Any )& aArguments )
        throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException );

    // XServiceInfo
    virtual NMSP_RTL::OUString SAL_CALL getImplementationName()
        throw ( NMSP_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const NMSP_RTL::OUString& ServiceName )
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( NMSP_RTL::OUString ) SAL_CALL getSupportedServiceNames()
        throw ( NMSP_UNO::RuntimeException );

};

NMSP_RTL::OUString PptImporter_getImplementationName()
    throw ( NMSP_UNO::RuntimeException );
sal_Bool SAL_CALL PptImportert_supportsService( const NMSP_RTL::OUString& ServiceName )
    throw( NMSP_UNO::RuntimeException );
SEQ( NMSP_RTL::OUString ) SAL_CALL PptImporter_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
