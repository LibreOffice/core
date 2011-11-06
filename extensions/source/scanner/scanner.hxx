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



#ifndef _EXT_SCANNER_HXX
#define _EXT_SCANNER_HXX

#include <tools/stream.hxx>
#include <vos/mutex.hxx>
#ifndef __RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#ifndef __COM_SUN_STAR_AWT_XBITMAP_HPP
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#ifndef __COM_SUN_STAR_LANG_EVENTOBJECT_HPP
#include <com/sun/star/lang/EventObject.hpp>
#endif
#include <com/sun/star/scanner/XScannerManager.hpp>
#include <com/sun/star/scanner/ScannerException.hpp>

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::scanner;

// -----------
// - Defines -
// -----------

#define REF( _def_Obj )                     Reference< _def_Obj >
#define SEQ( _def_Obj )                     Sequence< _def_Obj >
#define ANY                                 Any
#define AWT                                 com::sun::star::awt

// ------------------
// - ScannerManager -
// ------------------

class ScannerManager : public OWeakObject, XScannerManager, AWT::XBitmap
{
protected:

    vos::OMutex                             maProtector;
    void*                                   mpData;

    void                                    AcquireData();
    void                                    ReleaseData();

public:

                                            ScannerManager();
    virtual                                 ~ScannerManager();

    // XInterface
    virtual ANY SAL_CALL                    queryInterface( const Type & rType ) throw( RuntimeException );
    virtual void SAL_CALL                   acquire() throw();
    virtual void SAL_CALL                   release() throw();

    // XScannerManager
    virtual SEQ( ScannerContext ) SAL_CALL  getAvailableScanners() throw();
    virtual sal_Bool SAL_CALL                   configureScanner( ScannerContext& scanner_context ) throw( ScannerException );
    virtual void SAL_CALL                   startScan( const ScannerContext& scanner_context, const REF( com::sun::star::lang::XEventListener )& rxListener ) throw( ScannerException );
    virtual ScanError SAL_CALL              getError( const ScannerContext& scanner_context ) throw( ScannerException );
    virtual REF( AWT::XBitmap ) SAL_CALL    getBitmap( const ScannerContext& scanner_context ) throw( ScannerException );

    // XBitmap
    virtual AWT::Size SAL_CALL              getSize() throw();
    virtual SEQ( sal_Int8 ) SAL_CALL        getDIB() throw();
    virtual SEQ( sal_Int8 ) SAL_CALL        getMaskDIB() throw();

    // Misc
    static OUString                         getImplementationName_Static() throw();
    static Sequence< OUString >             getSupportedServiceNames_Static() throw();

    void                                    Lock() { maProtector.acquire(); }
    void                                    Unlock() { maProtector.release(); }

    void*                                   GetData() const { return mpData; }
    void                                    SetData( void* pData ) { ReleaseData(); mpData = pData; }
};

// -----------------------------------------------------------------------------

REF( XInterface ) SAL_CALL ScannerManager_CreateInstance( const REF( com::sun::star::lang::XMultiServiceFactory )& rxFactory ) throw( Exception );

#endif
