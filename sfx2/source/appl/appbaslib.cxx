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
    DBG_ERROR( "SfxBasicManagerHolder::getLibraryContainer: illegal container type!" );
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
        pSeq[0] = OUString::createFromAscii( "com.sun.star.script.ApplicationDialogLibraryContainer" );
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
        aImplName = OUString::createFromAscii( "com.sun.star.comp.sfx2.ApplicationDialogLibraryContainer" );
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
        pSeq[0] = OUString::createFromAscii( "com.sun.star.script.ApplicationScriptLibraryContainer" );
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
        aImplName = OUString::createFromAscii( "com.sun.star.comp.sfx2.ApplicationScriptLibraryContainer" );
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

