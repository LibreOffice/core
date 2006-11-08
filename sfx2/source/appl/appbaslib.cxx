/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appbaslib.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-08 11:56:47 $
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

#ifndef APPBASLIB_HXX
#include "appbaslib.hxx"
#endif

#include "sfxuno.hxx"
#include "sfxtypes.hxx"
#include "app.hxx"

#include <basic/basmgr.hxx>
#include <basic/namecont.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::embed;
using ::rtl::OUString;
using ::osl::MutexGuard;
using ::osl::Mutex;

using ::basic::SfxLibraryContainer;

//============================================================================
SfxBasicManagerHolder::SfxBasicManagerHolder()
    :mpBasicManager( NULL )
    ,mpBasicLibContainer( NULL )
    ,mpDialogLibContainer( NULL )
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
        const LibraryContainerInfo& rContainerInfo( mpBasicManager->GetLibraryContainerInfo() );
        mpBasicLibContainer = dynamic_cast< SfxLibraryContainer* >( rContainerInfo.mxScriptCont.get() );
        mpDialogLibContainer = dynamic_cast< SfxLibraryContainer* >( rContainerInfo.mxDialogCont.get() );
        DBG_ASSERT( mpBasicLibContainer && mpDialogLibContainer,
            "SfxBasicManagerHolder::reset: invalid library container implementations!" );

        if ( mpBasicLibContainer )
            mpBasicLibContainer->acquire();
        if ( mpDialogLibContainer )
            mpDialogLibContainer->acquire();
    }
}

bool SfxBasicManagerHolder::isAnyContainerModified() const
{
    if ( mpBasicLibContainer && mpBasicLibContainer->isContainerModified() )
        return true;
    if ( mpDialogLibContainer && mpDialogLibContainer->isContainerModified() )
        return true;
    return false;
}

void SfxBasicManagerHolder::storeLibraries( ContainerType _eType, bool _bComplete )
{
    SfxLibraryContainer* pContainer = impl_getContainer( _eType );
    if ( pContainer )
        pContainer->storeLibraries( _bComplete );
}

void SfxBasicManagerHolder::setStorage( const Reference< XStorage >& _rxStorage )
{
    SfxLibraryContainer* pContainer = impl_getContainer( SCRIPTS );
    if ( pContainer )
        pContainer->setStorage( _rxStorage );

    pContainer = impl_getContainer( DIALOGS );
    if ( pContainer )
        pContainer->setStorage( _rxStorage );
}

void SfxBasicManagerHolder::storeLibrariesToStorage( const Reference< XStorage >& _rxStorage )
{
    SfxLibraryContainer* pContainer = impl_getContainer( SCRIPTS );
    if ( pContainer )
        pContainer->storeLibrariesToStorage( _rxStorage );

    pContainer = impl_getContainer( DIALOGS );
    if ( pContainer )
        pContainer->storeLibrariesToStorage( _rxStorage );
}

Reference< XLibraryContainer > SfxBasicManagerHolder::getLibraryContainer( ContainerType _eType )
{
    return impl_getContainer( _eType );
}

void SfxBasicManagerHolder::impl_releaseContainers()
{
    if ( mpBasicLibContainer )
        mpBasicLibContainer->release();
    mpBasicLibContainer = NULL;
    if ( mpDialogLibContainer )
        mpDialogLibContainer->release();
    mpDialogLibContainer = NULL;
}

SfxLibraryContainer* SfxBasicManagerHolder::impl_getContainer( ContainerType _eType )
{
    switch ( _eType )
    {
    case SCRIPTS:   return mpBasicLibContainer;
    case DIALOGS:   return mpDialogLibContainer;
    }
    DBG_ERROR( "SfxBasicManagerHolder::impl_getContainer: illegal container type!" );
    return NULL;
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

