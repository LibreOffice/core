/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scanner.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:46:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXT_SCANNER_HXX
#define _EXT_SCANNER_HXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef __RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef __COM_SUN_STAR_AWT_XBITMAP_HPP
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef __COM_SUN_STAR_LANG_EVENTOBJECT_HPP
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SCANNER_SCANNEREXCEPTION_HPP_
#include <com/sun/star/scanner/ScannerException.hpp>
#endif

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

    void                                    DestroyData();

public:

                                            ScannerManager();
    virtual                                 ~ScannerManager();

    // XInterface
    virtual ANY SAL_CALL                    queryInterface( const Type & rType ) throw( RuntimeException );
    virtual void SAL_CALL                   acquire() throw();
    virtual void SAL_CALL                   release() throw();

    // XScannerManager
    virtual SEQ( ScannerContext ) SAL_CALL  getAvailableScanners() throw();
    virtual BOOL SAL_CALL                   configureScanner( ScannerContext& scanner_context ) throw( ScannerException );
    virtual void SAL_CALL                   startScan( const ScannerContext& scanner_context, const REF( com::sun::star::lang::XEventListener )& rxListener ) throw( ScannerException );
    virtual ScanError SAL_CALL              getError( const ScannerContext& scanner_context ) throw( ScannerException );
    virtual REF( AWT::XBitmap ) SAL_CALL    getBitmap( const ScannerContext& scanner_context ) throw( ScannerException );

    // XBitmap
    virtual AWT::Size SAL_CALL              getSize() throw();
    virtual SEQ( sal_Int8 ) SAL_CALL        getDIB() throw();
    virtual SEQ( sal_Int8 ) SAL_CALL        getMaskDIB() throw();

    // Misc
    OUString                                getImplementationName() throw();
    static OUString                         getImplementationName_Static() throw();
    Sequence< OUString >                    getSupportedServiceNames() throw();
    static Sequence< OUString >             getSupportedServiceNames_Static() throw();
    BOOL                                    supportsService( const OUString& ServiceName ) throw();

    void                                    Lock() { maProtector.acquire(); }
    void                                    Unlock() { maProtector.release(); }

    void*                                   GetData() const { return mpData; }
    void                                    SetData( void* pData ) { DestroyData(); mpData = pData; }
};

// -----------------------------------------------------------------------------

REF( XInterface ) SAL_CALL ScannerManager_CreateInstance( const REF( com::sun::star::lang::XMultiServiceFactory )& rxFactory ) throw( Exception );

#endif
