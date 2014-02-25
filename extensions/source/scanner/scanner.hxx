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

#ifndef _EXT_SCANNER_HXX
#define _EXT_SCANNER_HXX

#include <tools/stream.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <com/sun/star/scanner/ScannerException.hpp>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::scanner;

class ScannerManager : public OWeakObject, XScannerManager2, css::awt::XBitmap
{
protected:

    osl::Mutex                              maProtector;
    void*                                   mpData;

    void                                    AcquireData();
    void                                    ReleaseData();

public:

                                            ScannerManager();
    virtual                                 ~ScannerManager();

    // XInterface
    virtual Any SAL_CALL                    queryInterface( const Type & rType ) throw( RuntimeException, std::exception );
    virtual void SAL_CALL                   acquire() throw();
    virtual void SAL_CALL                   release() throw();

    // XScannerManager
    virtual Sequence< ScannerContext > SAL_CALL  getAvailableScanners() throw(std::exception);
    virtual sal_Bool SAL_CALL               configureScanner( ScannerContext& scanner_context ) throw( ScannerException, std::exception );
    virtual sal_Bool SAL_CALL               configureScannerAndScan( ScannerContext& scanner_context, const Reference< com::sun::star::lang::XEventListener >& rxListener ) throw( ScannerException, std::exception );
    virtual void SAL_CALL                   startScan( const ScannerContext& scanner_context, const Reference< com::sun::star::lang::XEventListener >& rxListener ) throw( ScannerException, std::exception );
    virtual ScanError SAL_CALL              getError( const ScannerContext& scanner_context ) throw( ScannerException, std::exception );
    virtual Reference< css::awt::XBitmap > SAL_CALL    getBitmap( const ScannerContext& scanner_context ) throw( ScannerException, std::exception );

    // XBitmap
    virtual css::awt::Size SAL_CALL              getSize() throw(std::exception);
    virtual Sequence< sal_Int8 > SAL_CALL        getDIB() throw(std::exception);
    virtual Sequence< sal_Int8 > SAL_CALL        getMaskDIB() throw(std::exception);

    // Misc
    static OUString                         getImplementationName_Static() throw();
    static Sequence< OUString >             getSupportedServiceNames_Static() throw();

    void                                    Lock() { maProtector.acquire(); }
    void                                    Unlock() { maProtector.release(); }

    void*                                   GetData() const { return mpData; }
    void                                    SetData( void* pData ) { ReleaseData(); mpData = pData; }
};

Reference< XInterface > SAL_CALL ScannerManager_CreateInstance( const Reference< com::sun::star::lang::XMultiServiceFactory >& rxFactory ) throw( Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
