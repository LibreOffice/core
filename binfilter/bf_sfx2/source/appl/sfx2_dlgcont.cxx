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



#include "dlgcont.hxx"


#include <xmlscript/xmldlg_imexp.hxx>


#include <bf_sfx2/sfxuno.hxx>

namespace binfilter {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star;
using namespace cppu;
using namespace rtl;
using namespace osl;


//============================================================================
// Implementation class SfxDialogLibraryContainer

/*N*/ sal_Bool SfxDialogLibraryContainer::init
/*N*/     ( const ::rtl::OUString& aInitialisationParam, SotStorage* pStor )
/*N*/ {
/*N*/     return SfxLibraryContainer_Impl::init( 
/*N*/           aInitialisationParam,
/*N*/           OUString ( RTL_CONSTASCII_USTRINGPARAM("dialog") ),
/*N*/ 		  OUString ( RTL_CONSTASCII_USTRINGPARAM("dialogs") ),
/*N*/ 		  OUString ( RTL_CONSTASCII_USTRINGPARAM("xdl") ),
/*N*/ 		  OUString ( RTL_CONSTASCII_USTRINGPARAM("Dialogs") ),
/*N*/           pStor );
/*N*/ }

// Ctor for service
// Ctor for service
SfxDialogLibraryContainer::SfxDialogLibraryContainer( void )
{
    // all initialisation has to be done
    // by calling XInitialization::initialize
}

/*N*/ SfxDialogLibraryContainer::SfxDialogLibraryContainer( SotStorage* pStor )
/*N*/ {
/*N*/     OUString aInitialisationParam;
/*N*/     init( aInitialisationParam, pStor );
/*N*/ }

// Methods to get library instances of the correct type
/*N*/ SfxLibrary_Impl* SfxDialogLibraryContainer::implCreateLibrary( void )
/*N*/ {
/*N*/ 	SfxLibrary_Impl* pRet = (SfxLibrary_Impl*) new SfxDialogLibrary( mxMSF, mxSFI );
/*N*/ 	return pRet;
/*N*/ }

/*N*/ SfxLibrary_Impl* SfxDialogLibraryContainer::implCreateLibraryLink
/*N*/ 	( const OUString& aLibInfoFileURL, const OUString& StorageURL, sal_Bool ReadOnly )
/*N*/ {
/*N*/ 	SfxLibrary_Impl* pRet = 
/*N*/ 		(SfxLibrary_Impl*) new SfxDialogLibrary
/*N*/             ( mxMSF, mxSFI, aLibInfoFileURL, StorageURL, ReadOnly );
/*N*/ 	return pRet;
/*N*/ }

/*N*/ Any SAL_CALL SfxDialogLibraryContainer::createEmptyLibraryElement( void )
/*N*/ {
/*N*/ 	Reference< XInputStreamProvider > xISP;
/*N*/ 	Any aRetAny;
/*N*/ 	aRetAny <<= xISP;
/*N*/ 	return aRetAny;
/*N*/ }

/*?*/ sal_Bool SAL_CALL SfxDialogLibraryContainer::isLibraryElementValid( Any aElement )
/*?*/ {
        Reference< XInputStreamProvider > xISP;
        aElement >>= xISP;
        sal_Bool bRet = xISP.is();
        return bRet;

/*?*/ }

/*?*/ void SAL_CALL SfxDialogLibraryContainer::writeLibraryElement
/*?*/ ( 
/*?*/ 	Any aElement,
/*?*/ 	const OUString& aElementName, 
/*?*/ 	Reference< XOutputStream > xOutput 
/*?*/ )
/*?*/ 	throw(Exception)
/*?*/ {
        Reference< XInputStreamProvider > xISP;
        aElement >>= xISP;
        if( !xISP.is() )
            return;

        Reference< XInputStream > xInput( xISP->createInputStream() );
        Sequence< sal_Int8 > bytes;
        sal_Int32 nRead = xInput->readBytes( bytes, xInput->available() );
        for (;;)
        {
            if( nRead )
                xOutput->writeBytes( bytes );

            nRead = xInput->readBytes( bytes, 1024 );
            if (! nRead)
                break;
        }
        xInput->closeInput();
/*?*/ }


/*?*/ Any SAL_CALL SfxDialogLibraryContainer::importLibraryElement
/*?*/ 	( const OUString& aFile, SotStorageStreamRef xElementStream )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); Any aAny; return aAny;//STRIP001 
/*?*/ }

/*?*/ void SAL_CALL SfxDialogLibraryContainer::importFromOldStorage( const ::rtl::OUString& aFile )
/*?*/ {
/*?*/     // Nothing to do here, old dialogs cannot be imported
/*?*/ }



//============================================================================
// Methods XInitialization
/*?*/ void SAL_CALL SfxDialogLibraryContainer::initialize( const Sequence< Any >& aArguments ) 
/*?*/     throw (::com::sun::star::uno::Exception, 
/*?*/            ::com::sun::star::uno::RuntimeException)
/*?*/ {
        sal_Int32 nArgCount = aArguments.getLength();
        OSL_ENSURE( nArgCount, "SfxDialogLibraryContainer::initialize() called with no arguments\n" );

        OUString aInitialisationParam;
        if( nArgCount )
        {
            const Any* pArgs = aArguments.getConstArray();
            pArgs[0] >>= aInitialisationParam;
            OSL_ENSURE( aInitialisationParam.getLength(), 
                "SfxDialogLibraryContainer::initialize() called with empty url\n" );
        }

        init( aInitialisationParam );
/*?*/ }


//============================================================================
// Service
/*N*/ SFX_IMPL_SINGLEFACTORY( SfxDialogLibraryContainer )

Sequence< OUString > SfxDialogLibraryContainer::impl_getStaticSupportedServiceNames()
{
    static Sequence< OUString > seqServiceNames( 1 );
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        OUString* pSeq = seqServiceNames.getArray();
        pSeq[0] = OUString::createFromAscii( "com.sun.star.script.DialogLibraryContainer" );
        bNeedsInit = sal_False;
    }
    return seqServiceNames;
}

OUString SfxDialogLibraryContainer::impl_getStaticImplementationName()
{
    static OUString aImplName;
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        aImplName = OUString::createFromAscii( "com.sun.star.comp.sfx2.DialogLibraryContainer" );
        bNeedsInit = sal_False;
    }
    return aImplName;
}

Reference< XInterface > SAL_CALL SfxDialogLibraryContainer::impl_createInstance
    ( const Reference< XMultiServiceFactory >& xServiceManager ) 
        throw( Exception )
{
    Reference< XInterface > xRet = 
        static_cast< XInterface* >( static_cast< OWeakObject* >(new SfxDialogLibraryContainer()) );
    return xRet;
}

//============================================================================
// Implementation class SfxDialogLibrary

// Ctor
/*N*/ SfxDialogLibrary::SfxDialogLibrary( Reference< XMultiServiceFactory > xMSF, 
/*N*/                                     Reference< XSimpleFileAccess > xSFI )
/*N*/ 	: SfxLibrary_Impl( getCppuType( (const Reference< XInputStreamProvider > *)0 ), xMSF, xSFI )
/*N*/ {
/*N*/ }

/*N*/ SfxDialogLibrary::SfxDialogLibrary( Reference< XMultiServiceFactory > xMSF, 
/*N*/                                     Reference< XSimpleFileAccess > xSFI,
/*N*/                                     const OUString& aLibInfoFileURL, 
/*N*/                                     const OUString& aStorageURL, 
/*N*/                                     sal_Bool ReadOnly )
/*N*/ 	: SfxLibrary_Impl( getCppuType( (const Reference< XInputStreamProvider > *)0 ),
/*N*/                        xMSF, xSFI, aLibInfoFileURL, aStorageURL, ReadOnly)
/*N*/ {
/*N*/ }

//============================================================================

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
