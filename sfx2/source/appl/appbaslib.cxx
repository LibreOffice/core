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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "appbaslib.hxx"

#include <sfx2/sfxuno.hxx>
#include "sfxtypes.hxx"
#include <sfx2/app.hxx>

#include <basic/basmgr.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::embed;
using ::rtl::OUString;
using ::osl::MutexGuard;
using ::osl::Mutex;

//============================================================================
SfxBasicManagerHolder::SfxBasicManagerHolder()
    :mpBasicManager( NULL )
{
}

void SfxBasicManagerHolder::reset( BasicManager* _pBasicManager )
{
    impl_releaseContainers();

    // Note: we do not delete the old BasicManager. BasicManager instances are
    // nowadays obtained from the BasicManagerRepository, and the ownership is with
    // the repository.
    // @see basic::BasicManagerRepository::getApplicationBasicManager
    // @see basic::BasicManagerRepository::getDocumentBasicManager
    mpBasicManager = _pBasicManager;

    if ( mpBasicManager )
    {
        try
        {
            mxBasicContainer.set( mpBasicManager->GetScriptLibraryContainer(), UNO_QUERY_THROW );
            mxDialogContainer.set( mpBasicManager->GetDialogLibraryContainer(), UNO_QUERY_THROW  );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

bool SfxBasicManagerHolder::isAnyContainerModified() const
{
    OSL_PRECOND( isValid(), "SfxBasicManagerHolder::isAnyContainerModified: not initialized!" );

    if ( mxBasicContainer.is() && mxBasicContainer->isModified() )
        return true;
    if ( mxDialogContainer.is() && mxDialogContainer->isModified() )
        return true;

    return false;
}

void SfxBasicManagerHolder::storeAllLibraries()
{
    OSL_PRECOND( isValid(), "SfxBasicManagerHolder::storeAllLibraries: not initialized!" );
    try
    {
        if ( mxBasicContainer.is() )
            mxBasicContainer->storeLibraries();
        if ( mxDialogContainer.is() )
            mxDialogContainer->storeLibraries();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SfxBasicManagerHolder::setStorage( const Reference< XStorage >& _rxStorage )
{
    try
    {
        if ( mxBasicContainer.is() )
            mxBasicContainer->setRootStorage( _rxStorage );
        if ( mxDialogContainer.is() )
            mxDialogContainer->setRootStorage( _rxStorage );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SfxBasicManagerHolder::storeLibrariesToStorage( const Reference< XStorage >& _rxStorage )
{
    OSL_PRECOND( isValid(), "SfxBasicManagerHolder::storeLibrariesToStorage: not initialized!" );

    try
    {
        if ( mxBasicContainer.is() )
            mxBasicContainer->storeLibrariesToStorage( _rxStorage );
        if ( mxDialogContainer.is() )
            mxDialogContainer->storeLibrariesToStorage( _rxStorage );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

Reference< XLibraryContainer > SfxBasicManagerHolder::getLibraryContainer( ContainerType _eType )
{
    OSL_PRECOND( isValid(), "SfxBasicManagerHolder::getLibraryContainer: not initialized!" );

    switch ( _eType )
    {
    case SCRIPTS:   return mxBasicContainer.get();
    case DIALOGS:   return mxDialogContainer.get();
    }
    OSL_FAIL( "SfxBasicManagerHolder::getLibraryContainer: illegal container type!" );
    return NULL;
}

void SfxBasicManagerHolder::impl_releaseContainers()
{
    mxBasicContainer.clear();
    mxDialogContainer.clear();
}

sal_Bool
SfxBasicManagerHolder::LegacyPsswdBinaryLimitExceeded( Sequence< rtl::OUString >& sModules )
{
    if ( mpBasicManager )
        return mpBasicManager->LegacyPsswdBinaryLimitExceeded( sModules );
    return sal_True;
}

//============================================================================
// Service for application library container
SFX_IMPL_ONEINSTANCEFACTORY( SfxApplicationDialogLibraryContainer )

Sequence< OUString > SfxApplicationDialogLibraryContainer::impl_getStaticSupportedServiceNames()
{
    static Sequence< OUString > seqServiceNames( 1 );
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        OUString* pSeq = seqServiceNames.getArray();
        pSeq[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.ApplicationDialogLibraryContainer"));
        bNeedsInit = sal_False;
    }
    return seqServiceNames;
}

OUString SfxApplicationDialogLibraryContainer::impl_getStaticImplementationName()
{
    static OUString aImplName;
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        aImplName = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sfx2.ApplicationDialogLibraryContainer"));
        bNeedsInit = sal_False;
    }
    return aImplName;
}

Reference< XInterface > SAL_CALL SfxApplicationDialogLibraryContainer::impl_createInstance
    ( const Reference< XMultiServiceFactory >& )
        throw( Exception )
{
    SFX_APP()->GetBasicManager();
    Reference< XInterface > xRet =
        Reference< XInterface >( SFX_APP()->GetDialogContainer(), UNO_QUERY );
    return xRet;
}

//============================================================================
// Service for application library container
SFX_IMPL_ONEINSTANCEFACTORY( SfxApplicationScriptLibraryContainer )

Sequence< OUString > SfxApplicationScriptLibraryContainer::impl_getStaticSupportedServiceNames()
{
    static Sequence< OUString > seqServiceNames( 1 );
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        OUString* pSeq = seqServiceNames.getArray();
        pSeq[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.ApplicationScriptLibraryContainer"));
        bNeedsInit = sal_False;
    }
    return seqServiceNames;
}

OUString SfxApplicationScriptLibraryContainer::impl_getStaticImplementationName()
{
    static OUString aImplName;
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        aImplName = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sfx2.ApplicationScriptLibraryContainer"));
        bNeedsInit = sal_False;
    }
    return aImplName;
}

Reference< XInterface > SAL_CALL SfxApplicationScriptLibraryContainer::impl_createInstance
    ( const Reference< XMultiServiceFactory >& )
        throw( Exception )
{
    SFX_APP()->GetBasicManager();
    Reference< XInterface > xRet =
        Reference< XInterface >( SFX_APP()->GetBasicContainer(), UNO_QUERY );
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
