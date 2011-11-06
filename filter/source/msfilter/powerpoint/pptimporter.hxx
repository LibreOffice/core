/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
