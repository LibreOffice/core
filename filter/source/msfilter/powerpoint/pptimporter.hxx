/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    REF( NMSP_LANG::XMultiServiceFactory )	xFact;										
    REF( NMSP_DOCUMENT::XImporter )         xImporter;
                                            
public:										
                                            
                                            PptImporter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual 								~PptImporter();
                                            
    // XInterface							
    virtual void SAL_CALL					acquire() throw();
    virtual void SAL_CALL					release() throw();
    
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
