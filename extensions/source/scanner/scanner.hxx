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
