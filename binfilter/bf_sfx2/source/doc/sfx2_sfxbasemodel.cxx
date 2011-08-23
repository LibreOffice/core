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

//________________________________________________________________________________________________________
//	my own includes
//________________________________________________________________________________________________________

#ifndef _SFX_SFXBASEMODEL_HXX_
#include <sfxbasemodel.hxx>
#endif
#include <bf_sfx2/app.hxx>

//________________________________________________________________________________________________________
//	include of other projects
//________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_VIEW_XPRINTJOB_HPP_
#include <com/sun/star/view/XPrintJob.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_ILLEGALARGUMENTIOEXCEPTION_HPP_
#include <com/sun/star/frame/IllegalArgumentIOException.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_PAPERFORMAT_HPP_
#include <com/sun/star/view/PaperFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <bf_svtools/itemset.hxx>
#endif

#ifndef _SFXSTRITEM_HXX
#include <bf_svtools/stritem.hxx>
#endif

#ifndef _SFXINTITEM_HXX
#include <bf_svtools/intitem.hxx>
#endif

#ifndef _SFXENUMITEM_HXX
#include <bf_svtools/eitem.hxx>
#endif

#include "bf_basic/sbx.hxx"

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

#ifndef _URLOBJ_HXX_
#include <tools/urlobj.hxx>
#endif

#ifndef _UNOTOOLS_TEMPFILE_HXX_
#include <unotools/tempfile.hxx>
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX_
#include <unotools/localfilehelper.hxx>
#endif

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#include <vos/mutex.hxx>
#include <vcl/salctype.hxx>

//________________________________________________________________________________________________________
//	includes of my own project
//________________________________________________________________________________________________________

#include "appuno.hxx"

#ifndef _SFX_OBJSHIMP_HXX
#include <objshimp.hxx>
#endif


#ifndef _SFXDOCFILE_HXX
#include <docfile.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <request.hxx>
#endif

#ifndef _SFX_OBJUNO_HXX
#include <objuno.hxx>
#endif

#ifndef _SFX_PRINTER_HXX
#include <printer.hxx>
#endif

#ifndef _SFXEVENT_HXX
#include <event.hxx>
#endif

#ifndef _SFX_EVENTSUPPLIER_HXX_
#include <eventsupplier.hxx>
#endif

#ifndef _SFX_EVENTCONF_HXX
#include <evntconf.hxx>
#endif

#ifndef _SFX_INTERNO_HXX
#include <interno.hxx>
#endif

#ifndef _SFX_HRC
#include "sfx.hrc"
#endif

#include "openflag.hxx"
#include "appdata.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"

#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _BASMGR_HXX
#include "bf_basic/basmgr.hxx"
#endif

namespace binfilter {

//________________________________________________________________________________________________________
//	defines
//________________________________________________________________________________________________________

#define	SfxIOException_Impl( nErr )				::com::sun::star::io::IOException()

#define	XFRAME									::com::sun::star::frame::XFrame
#define	XINTERFACE								::com::sun::star::uno::XInterface
#define	OMULTITYPEINTERFACECONTAINERHELPER		::cppu::OMultiTypeInterfaceContainerHelper
#define	UNO_QUERY								::com::sun::star::uno::UNO_QUERY
#define	DISPOSEDEXCEPTION						::com::sun::star::lang::DisposedException
#define	MAPPING									::com::sun::star::uno::Mapping
#define	XSELECTIONSUPPLIER						::com::sun::star::view::XSelectionSupplier
#define	ANY										::com::sun::star::uno::Any
#define	ILLEGALARGUMENTEXCEPTION				::com::sun::star::lang::IllegalArgumentException
#define	ILLEGALARGUMENTIOEXCEPTION				::com::sun::star::frame::IllegalArgumentIOException
#define	DOUBLEINITIALIZATIONEXCEPTION			::com::sun::star::frame::DoubleInitializationException
#define	OINTERFACECONTAINERHELPER               ::cppu::OInterfaceContainerHelper
#define	OINTERFACEITERATORHELPER				::cppu::OInterfaceIteratorHelper
#define	SIZE									::com::sun::star::awt::Size
#define	PAPERFORMAT								::com::sun::star::view::PaperFormat
#define	PAPERORIENTATION						::com::sun::star::view::PaperOrientation
#define OTYPECOLLECTION							::cppu::OTypeCollection
#define OIMPLEMENTATIONID						::cppu::OImplementationId
#define	MUTEXGUARD								::osl::MutexGuard
#define XINDEXCONTAINER							::com::sun::star::container::XIndexContainer
#define UNSUPPORTEDFLAVOREXCEPTION				::com::sun::star::datatransfer::UnsupportedFlavorException
#define XPRINTJOBLISTENER						::com::sun::star::view::XPrintJobListener

//________________________________________________________________________________________________________
//	namespaces
//________________________________________________________________________________________________________


// Don't use using ... here, because there are at least two classes with the same name in use

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//________________________________________________________________________________________________________
//	impl. declarations
//________________________________________________________________________________________________________

struct IMPL_SfxBaseModel_DataContainer
{
    SfxObjectShellRef								m_pObjectShell			;
    //SfxObjectShellLock								m_pObjectShellLock		;
    OUSTRING										m_sURL					;
    sal_uInt16										m_nControllerLockCount	;
    OMULTITYPEINTERFACECONTAINERHELPER				m_aInterfaceContainer	;
    REFERENCE< XINTERFACE >							m_xParent				;
    REFERENCE< XCONTROLLER >						m_xCurrent				;
    REFERENCE< XDOCUMENTINFO >						m_xDocumentInfo			;
    REFERENCE< XSTARBASICACCESS >					m_xStarBasicAccess		;
    REFERENCE< XNAMEREPLACE >						m_xEvents				;
    SEQUENCE< PROPERTYVALUE>						m_seqArguments			;
    SEQUENCE< REFERENCE< XCONTROLLER > >			m_seqControllers		;
    REFERENCE< XINDEXACCESS >						m_contViewData			;
    sal_Bool										m_bLoadDone				;
    sal_Bool										m_bLoadState			;
    sal_Bool										m_bClosed				;
    sal_Bool										m_bClosing				;
    REFERENCE< ::com::sun::star::view::XPrintJob>     m_xPrintJob				;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aPrintOptions;

    IMPL_SfxBaseModel_DataContainer(	MUTEX&			aMutex			,
                                                                        SfxObjectShell*	pObjectShell	)
            :	m_pObjectShell			( pObjectShell	)
//			,	m_pObjectShellLock		( pObjectShell	)
            ,	m_sURL					( String()		)
            ,	m_nControllerLockCount	( 0				)
            ,	m_aInterfaceContainer	( aMutex		)
            ,	m_bLoadDone				( sal_False		)
            ,	m_bLoadState			( sal_False		)
            ,	m_bClosed				( sal_False		)
            ,	m_bClosing				( sal_False		)
    {
    }
} ;

extern void* getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta );
extern void* getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize );
extern SvMemoryStream* getMetaMemStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, sal_uInt32 nFormat );
extern sal_Bool supportsMetaFileHandle_Impl();


//________________________________________________________________________________________________________
//	constructor
//________________________________________________________________________________________________________

/*N*/ SfxBaseModel::SfxBaseModel( SfxObjectShell *pObjectShell )
/*N*/ : IMPL_SfxBaseModel_MutexContainer()
/*N*/ , m_pData( new IMPL_SfxBaseModel_DataContainer( m_aMutex, pObjectShell ) )
/*N*/ {
/*N*/ 	if ( pObjectShell != NULL )
/*N*/ 	{
/*N*/ 		StartListening( *pObjectShell ) ;
/*N*/ 	}
/*N*/ }

//________________________________________________________________________________________________________
//	destructor
//________________________________________________________________________________________________________

/*N*/ SfxBaseModel::~SfxBaseModel()
/*N*/ {
/*N*/ }

//________________________________________________________________________________________________________
//	XInterface
//________________________________________________________________________________________________________

/*N*/ ANY SAL_CALL SfxBaseModel::queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// Attention:
/*N*/ 	//	Don't use mutex or guard in this method!!! Is a method of XInterface.
/*N*/
/*N*/ 	// Ask for my own supported interfaces ...
/*N*/ 	ANY aReturn( ::cppu::queryInterface(	rType											,
/*N*/ 									   		static_cast< XTYPEPROVIDER*			> ( this )	,
/*N*/ 									   		static_cast< XCHILD*				> ( this )	,
/*N*/ 									   		static_cast< XDOCUMENTINFOSUPPLIER*	> ( this )	,
/*N*/ 									   		static_cast< XEVENTLISTENER*		> ( this )	,
/*N*/ 									   		static_cast< XMODEL*				> ( this )	,
/*N*/ 									   		static_cast< XMODIFIABLE*			> ( this )	,
/*N*/                                             static_cast< XCOMPONENT*            > ( this )  ,
/*N*/ 									   		static_cast< XPRINTABLE*			> ( this )	,
/*N*/ 									   		static_cast< XSTARBASICACCESS*		> ( this )	,
/*N*/                                             static_cast< XSTORABLE*             > ( this )  ,
/*N*/                                             static_cast< XLOADABLE*             > ( this )  ,
/*N*/                                             static_cast< XCLOSEABLE*            > ( this )  ) );
/*N*/
/*N*/ 	if ( aReturn.hasValue() == sal_False )
/*N*/ 	{
/*N*/ 		aReturn = ::cppu::queryInterface(	rType											,
/*N*/                                             static_cast< XMODIFYBROADCASTER*    > ( this )  ,
/*N*/                                             static_cast< XTRANSFERABLE*    > ( this )  ,
/*N*/ 									   		static_cast< XPRINTJOBBROADCASTER*	> ( this )	,
/*N*/  									   		static_cast< XCLOSEBROADCASTER*		> ( this )	,
/*N*/                                             static_cast< XVIEWDATASUPPLIER*     > ( this )  ,
/*N*/ 									   		static_cast< XEVENTBROADCASTER*		> ( this )	,
/*N*/ 									   		static_cast< XEVENTSSUPPLIER*		> ( this )	) ;
/*N*/ 	}
/*N*/ 	// If searched interface supported by this class ...
/*N*/ 	if ( aReturn.hasValue() == sal_True )
/*N*/ 	{
/*N*/ 		// ... return this information.
/*N*/ 		return aReturn ;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Else; ... ask baseclass for interfaces!
/*N*/ 		return OWeakObject::queryInterface( rType ) ;
/*N*/ 	}
/*N*/ }

//________________________________________________________________________________________________________
//	XInterface
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::acquire() throw( )
/*N*/ {
/*N*/ 	// Attention:
/*N*/ 	//	Don't use mutex or guard in this method!!! Is a method of XInterface.
/*N*/
/*N*/ 	// Forward to baseclass
/*N*/ 	OWeakObject::acquire() ;
/*N*/ }

//________________________________________________________________________________________________________
//	XInterface
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::release() throw( )
/*N*/ {
/*N*/ 	// Attention:
/*N*/ 	//	Don't use mutex or guard in this method!!! Is a method of XInterface.
/*N*/
/*N*/ 	// Forward to baseclass
/*N*/ 	OWeakObject::release() ;
/*N*/ }

//________________________________________________________________________________________________________
//	XTypeProvider
//________________________________________________________________________________________________________

/*N*/ SEQUENCE< UNOTYPE > SAL_CALL SfxBaseModel::getTypes() throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// Optimize this method !
/*N*/ 	// We initialize a static variable only one time. And we don't must use a mutex at every call!
/*N*/ 	// For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
/*N*/ 	static OTYPECOLLECTION* pTypeCollection = NULL ;
/*N*/
/*N*/ 	if ( pTypeCollection == NULL )
/*N*/ 	{
/*N*/ 		// Ready for multithreading; get global mutex for first call of this method only! see before
/*N*/ 		MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;
/*N*/
/*N*/ 		// Control these pointer again ... it can be, that another instance will be faster then these!
/*N*/ 		if ( pTypeCollection == NULL )
/*N*/ 		{
/*N*/ 			// Create a static typecollection ...
/*N*/             static OTYPECOLLECTION aTypeCollectionFirst( ::getCppuType(( const REFERENCE< XTYPEPROVIDER          >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XCHILD                 >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XDOCUMENTINFOSUPPLIER  >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XEVENTLISTENER         >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XMODEL                 >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XMODIFIABLE            >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XPRINTABLE             >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XSTORABLE              >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XLOADABLE              >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XCLOSEABLE             >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XSTARBASICACCESS       >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XEVENTBROADCASTER      >*)NULL ) );
/*N*/
/*N*/             static OTYPECOLLECTION aTypeCollection     ( ::getCppuType(( const REFERENCE< XVIEWDATASUPPLIER      >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XTRANSFERABLE          >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XPRINTJOBBROADCASTER   >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XEVENTSSUPPLIER        >*)NULL ) ,
/*N*/                                                          ::getCppuType(( const REFERENCE< XCLOSEBROADCASTER      >*)NULL ) ,
/*N*/                                                          aTypeCollectionFirst.getTypes()                                   );
/*N*/
/*N*/ 			// ... and set his address to static pointer!
/*N*/ 			pTypeCollection = &aTypeCollection ;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return pTypeCollection->getTypes() ;
/*N*/ }

//________________________________________________________________________________________________________
//	XTypeProvider
//________________________________________________________________________________________________________

/*N*/ SEQUENCE< sal_Int8 > SAL_CALL SfxBaseModel::getImplementationId() throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// Create one Id for all instances of this class.
/*N*/ 	// Use ethernet address to do this! (sal_True)
/*N*/
/*N*/ 	// Optimize this method
/*N*/ 	// We initialize a static variable only one time. And we don't must use a mutex at every call!
/*N*/ 	// For the first call; pID is NULL - for the second call pID is different from NULL!
/*N*/ 	static OIMPLEMENTATIONID* pID = NULL ;
/*N*/
/*N*/ 	if ( pID == NULL )
/*N*/ 	{
/*N*/ 		// Ready for multithreading; get global mutex for first call of this method only! see before
/*N*/ 		MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;
/*N*/
/*N*/ 		// Control these pointer again ... it can be, that another instance will be faster then these!
/*N*/ 		if ( pID == NULL )
/*N*/ 		{
/*N*/ 			// Create a new static ID ...
/*N*/ 			static OIMPLEMENTATIONID aID( sal_False ) ;
/*N*/ 			// ... and set his address to static pointer!
/*N*/ 			pID = &aID ;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return pID->getImplementationId() ;
/*N*/ }

//________________________________________________________________________________________________________
//	XStarBasicAccess
//________________________________________________________________________________________________________

/*N*/ REFERENCE< XSTARBASICACCESS > implGetStarBasicAccess( SfxObjectShell* pObjectShell )
/*N*/ {
/*N*/ 	REFERENCE< XSTARBASICACCESS > xRet;
/*N*/ 	if( pObjectShell )
/*N*/ 	{
/*N*/ 		BasicManager* pMgr = pObjectShell->GetBasicManager();
/*N*/ 		xRet = getStarBasicAccess( pMgr );
/*N*/ 	}
/*N*/ 	return xRet;
/*N*/ }

/*N*/ REFERENCE< XNAMECONTAINER > SAL_CALL SfxBaseModel::getLibraryContainer() throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
/*N*/ 	if( !rxAccess.is() )
/*N*/ 		rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );
/*N*/
/*N*/ 	REFERENCE< XNAMECONTAINER > xRet;
/*N*/ 	if( rxAccess.is() )
/*N*/ 		xRet = rxAccess->getLibraryContainer();
/*N*/ 	return xRet;
/*N*/ }

/**___________________________________________________________________________________________________
    @seealso	XStarBasicAccess
*/
/*N*/ void SAL_CALL SfxBaseModel::createLibrary( const OUSTRING& LibName, const OUSTRING& Password,
/*N*/ 	const OUSTRING& ExternalSourceURL, const OUSTRING& LinkTargetURL )
/*N*/ 		throw(ELEMENTEXISTEXCEPTION, RUNTIMEEXCEPTION)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
/*N*/ 	if( !rxAccess.is() )
/*N*/ 		rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );
/*N*/
/*N*/ 	if( rxAccess.is() )
/*N*/ 		rxAccess->createLibrary( LibName, Password, ExternalSourceURL, LinkTargetURL );
/*N*/ }

/**___________________________________________________________________________________________________
    @seealso	XStarBasicAccess
*/
/*N*/ void SAL_CALL SfxBaseModel::addModule( const OUSTRING& LibraryName, const OUSTRING& ModuleName,
/*N*/ 	const OUSTRING& Language, const OUSTRING& Source )
/*N*/ 		throw( NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
/*N*/ 	if( !rxAccess.is() )
/*N*/ 		rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );
/*N*/
/*N*/ 	if( rxAccess.is() )
/*N*/ 		rxAccess->addModule( LibraryName, ModuleName, Language, Source );
/*N*/ }

/**___________________________________________________________________________________________________
    @seealso	XStarBasicAccess
*/
/*N*/ void SAL_CALL SfxBaseModel::addDialog( const OUSTRING& LibraryName, const OUSTRING& DialogName,
/*N*/ 	const ::com::sun::star::uno::Sequence< sal_Int8 >& Data )
/*N*/ 		throw(NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
/*N*/ 	if( !rxAccess.is() )
/*N*/ 		rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );
/*N*/
/*N*/ 	if( rxAccess.is() )
/*N*/ 		rxAccess->addDialog( LibraryName, DialogName, Data );
/*N*/ }


//________________________________________________________________________________________________________
//	XChild
//________________________________________________________________________________________________________

/*N*/ REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getParent() throw( RUNTIMEEXCEPTION )
/*N*/ {
/*	#77222#
     AS->MBA: There is one view only at the moment. We don't must search for other parents in other frames ...!?

    if ( !m_pData->m_xParent.is() && m_pData->m_xCurrent.is() )
    {
        // If no parent is set get the parent by view hierarchy
        REFERENCE< XFRAME >  xParentFrame( m_pData->m_xCurrent->getFrame()->getCreator(), UNO_QUERY );
        if ( xParentFrame.is() )
        {
            REFERENCE< XCONTROLLER >  xCtrl( xParentFrame->getController() );
            if ( xCtrl.is() )
                return xCtrl->getModel();
        }
    }
*/
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	return m_pData->m_xParent;
/*N*/ }

//________________________________________________________________________________________________________
//	XChild
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::setParent(const REFERENCE< XINTERFACE >& Parent) throw(NOSUPPORTEXCEPTION, RUNTIMEEXCEPTION)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( Parent.is() && getParent().is() )
/*N*/ 		// only set parent when no parent is available
/*N*/ 		throw NOSUPPORTEXCEPTION();
/*N*/
/*N*/ 	m_pData->m_xParent = Parent;
/*N*/ }

//________________________________________________________________________________________________________
//	XChild
//________________________________________________________________________________________________________

/*N*/ long SfxObjectShellClose_Impl( void* pObj, void* pArg );

/*N*/ void SAL_CALL SfxBaseModel::dispose() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/
/*N*/ 	// object already disposed?
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if  ( !m_pData->m_bClosed )
/*N*/ 	{
/*N*/ 		// gracefully accept wrong dispose calls instead of close call
/*N*/ 		// and try to make it work (may be really disposed later!)
/*N*/ 		try
/*N*/ 		{
/*N*/ 			close( sal_True );
/*N*/ 		}
/*N*/ 		catch ( ::com::sun::star::util::CloseVetoException& )
/*N*/ 		{
/*N*/ 		}
/*N*/
/*N*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	EVENTOBJECT aEvent( (XMODEL *)this );
/*N*/ 	m_pData->m_aInterfaceContainer.disposeAndClear( aEvent );
/*N*/
/*N*/ 	// is an object shell assigned?
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/ 	{
/*N*/ 		// Rekursion vermeiden
/*N*/         SfxObjectShellRef pShell;
/*N*/         //SfxObjectShellLock pShellLock;
/*N*/
/*N*/         {
/*N*/             // am I "ThisComponent" in AppBasic?
/*N*/             StarBASIC* pBas = SFX_APP()->GetBasic_Impl();
/*N*/             if ( pBas && SFX_APP()->Get_Impl()->pThisDocument == m_pData->m_pObjectShell )
/*N*/             {
                    DBG_ERROR( "SfxBaseModel::dispose: dead code!" );
/*N*/             }
/*N*/
/*N*/             pShell = m_pData->m_pObjectShell;
/*N*/             //pShellLock = m_pData->m_pObjectShellLock;
/*N*/             EndListening( *pShell );
/*N*/             m_pData->m_pObjectShell = SfxObjectShellRef();
/*N*/         	//m_pData->m_pObjectShellLock = SfxObjectShellLock();
/*N*/         }
/*N*/
/*N*/ 		// Bei dispose keine Speichern-R"uckfrage
/*N*/       //if ( pShell->IsEnableSetModified() && !pShell->Get_Impl()->bClosing )
/*N*/       //    pShell->SetModified( sal_False );
/*N*/ 		pShell->Get_Impl()->bDisposing = TRUE;
/*N*/         //pShellLock = SfxObjectShellLock();
/*N*/ 		SfxObjectShellClose_Impl( 0, (void*) pShell );
/*N*/ 	}
/*N*/
/*N*/ 	m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
/*N*/ 	m_pData->m_seqControllers = SEQUENCE< REFERENCE< XCONTROLLER > > () ;
/*N*/
/*N*/ 	DELETEZ(m_pData);
/*N*/ }

//________________________________________________________________________________________________________
//	XChild
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
/*N*/     throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
/*N*/ }

//________________________________________________________________________________________________________
//	XChild
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
/*N*/     throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
/*N*/ }

//________________________________________________________________________________________________________
//	XDOCUMENTINFOSupplier
//________________________________________________________________________________________________________

/*N*/ REFERENCE< XDOCUMENTINFO > SAL_CALL SfxBaseModel::getDocumentInfo() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( !m_pData->m_xDocumentInfo.is() && m_pData->m_pObjectShell.Is() )
/*N*/ 		((SfxBaseModel*)this)->m_pData->m_xDocumentInfo = new SfxDocumentInfoObject( m_pData->m_pObjectShell ) ;
/*N*/
/*N*/ 	return m_pData->m_xDocumentInfo;
/*N*/ }

//________________________________________________________________________________________________________
//	XEVENTLISTENER
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::disposing( const EVENTOBJECT& aObject )
/*N*/     throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/     REFERENCE< XMODIFYLISTENER >  xMod( aObject.Source, UNO_QUERY );
/*N*/     REFERENCE< XEVENTLISTENER >  xListener( aObject.Source, UNO_QUERY );
/*N*/     REFERENCE< XDOCEVENTLISTENER >  xDocListener( aObject.Source, UNO_QUERY );
/*N*/
/*N*/     if ( xMod.is() )
/*N*/         m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xMod );
/*N*/     else if ( xListener.is() )
/*N*/         m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), xListener );
/*N*/     else if ( xDocListener.is() )
/*N*/         m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), xListener );
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::attachResource(	const	OUSTRING&					rURL	,
/*N*/ 												const	SEQUENCE< PROPERTYVALUE >&	rArgs	)
/*N*/     throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( rURL.getLength() == 0 && rArgs.getLength() == 1 && rArgs[0].Name.equalsAscii( "SetEmbedded" ) )
/*N*/ 	{
/*N*/ 		// allows to set a windowless document to EMBEDDED state
/*N*/ 		// but _only_ before load() or initNew() methods
/*?*/ 		if ( m_pData->m_pObjectShell.Is() && !m_pData->m_pObjectShell->GetMedium() )
/*?*/ 		{
/*?*/ 			sal_Bool bEmb;
/*?*/ 			if ( ( rArgs[0].Value >>= bEmb ) && bEmb )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 m_pData->m_pObjectShell->SetCreateMode_Impl( SFX_CREATE_MODE_EMBEDDED );
/*?*/ 		}
/*N*/
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/ 	{
/*N*/ 		m_pData->m_sURL = rURL;
/*N*/ 		m_pData->m_seqArguments = rArgs;
/*N*/
/*N*/ 		sal_Int32 nNewLength = rArgs.getLength();
/*N*/ 		for ( sal_Int32 nInd = 0; nInd < rArgs.getLength(); nInd++ )
/*N*/ 			if ( rArgs[nInd].Name.equalsAscii( "WinExtent" ) )
/*N*/ 			{
/*N*/ 				// the document should be resized
/*?*/ 				SfxInPlaceObject* pInPlaceObj = m_pData->m_pObjectShell->GetInPlaceObject();
/*?*/ 				if ( pInPlaceObj )
/*?*/ 				{
/*?*/ 					Sequence< sal_Int32 > aSize;
/*?*/ 					if ( ( rArgs[nInd].Value >>= aSize ) && aSize.getLength() == 4 )
/*?*/ 					{
/*?*/ 						Rectangle aTmpRect( aSize[0], aSize[1], aSize[2], aSize[3] );
/*?*/ 						aTmpRect = OutputDevice::LogicToLogic( aTmpRect, MAP_100TH_MM, pInPlaceObj->GetMapUnit() );
/*?*/
/*?*/ 						pInPlaceObj->SetVisArea( aTmpRect );
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 		if( m_pData->m_pObjectShell->GetMedium() )
/*N*/ 		{
/*N*/ 			SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
/*N*/ 			TransformParameters( SID_OPENDOC, rArgs, aSet );
/*N*/ 			m_pData->m_pObjectShell->GetMedium()->GetItemSet()->Put( aSet );
/*N*/ 			SFX_ITEMSET_ARG( &aSet, pItem, SfxStringItem, SID_FILTER_NAME, sal_False );
/*N*/ 			if ( pItem )
/*N*/ 				m_pData->m_pObjectShell->GetMedium()->SetFilter(
/*N*/ 					m_pData->m_pObjectShell->GetFactory().GetFilterContainer()->GetFilter( pItem->GetValue() ) );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return sal_True ;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ OUSTRING SAL_CALL SfxBaseModel::getURL() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	return m_pData->m_sURL ;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ SEQUENCE< PROPERTYVALUE > SAL_CALL SfxBaseModel::getArgs() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/ 	{
/*N*/ 		SEQUENCE< PROPERTYVALUE > seqArgsNew;
/*N*/ 		SEQUENCE< PROPERTYVALUE > seqArgsOld;
/*N*/ 		SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
/*N*/
/*N*/ 		// we need to know which properties are supported by the transformer
/*N*/ 		// hopefully it is a temporary solution, I guess nonconvertable properties
/*N*/ 		// should not be supported so then there will be only ItemSet from medium
/*N*/
/*N*/ 		TransformItems( SID_OPENDOC, *(m_pData->m_pObjectShell->GetMedium()->GetItemSet()), seqArgsNew );
/*N*/ 		TransformParameters( SID_OPENDOC, m_pData->m_seqArguments, aSet );
/*N*/ 		TransformItems( SID_OPENDOC, aSet, seqArgsOld );
/*N*/
/*N*/ 		sal_Int32 nOrgLength = m_pData->m_seqArguments.getLength();
/*N*/ 		sal_Int32 nOldLength = seqArgsOld.getLength();
/*N*/ 		sal_Int32 nNewLength = seqArgsNew.getLength();
/*N*/
/*N*/ 		// "WinExtent" property should be updated always.
/*N*/ 		// We can store it now to overwrite an old value
/*N*/ 		// since it is not from ItemSet
/*N*/ 		SfxInPlaceObject* pInPlaceObj = m_pData->m_pObjectShell->GetInPlaceObject();
/*N*/ 		if ( pInPlaceObj )
/*N*/ 		{
/*N*/ 			Rectangle aTmpRect = pInPlaceObj->GetVisArea( ASPECT_CONTENT );
/*N*/ 			aTmpRect = OutputDevice::LogicToLogic( aTmpRect, pInPlaceObj->GetMapUnit(), MAP_100TH_MM );
/*N*/
/*N*/ 			Sequence< sal_Int32 > aSize(4);
/*N*/ 			aSize[0] = aTmpRect.Left();
/*N*/ 			aSize[1] = aTmpRect.Top();
/*N*/ 			aSize[2] = aTmpRect.Right();
/*N*/ 			aSize[3] = aTmpRect.Bottom();
/*N*/
/*N*/ 			seqArgsNew.realloc( ++nNewLength );
/*N*/ 			seqArgsNew[ nNewLength - 1 ].Name = ::rtl::OUString::createFromAscii( "WinExtent" );
/*N*/ 			seqArgsNew[ nNewLength - 1 ].Value <<= aSize;
/*N*/ 		}
/*N*/
/*N*/ 		for ( sal_Int32 nOrg = 0; nOrg < nOrgLength; nOrg++ )
/*N*/ 		{
/*N*/  			sal_Int32 nOldInd = 0;
/*N*/ 			while ( nOldInd < nOldLength )
/*N*/ 			{
/*N*/ 				if ( m_pData->m_seqArguments[nOrg].Name.equals( seqArgsOld[nOldInd].Name ) )
/*N*/ 					break;
/*N*/ 				nOldInd++;
/*N*/ 			}
/*N*/
/*N*/ 			if ( nOldInd == nOldLength )
/*N*/ 			{
/*N*/ 				// the entity with this name should be new for seqArgsNew
/*N*/ 				// since it is not supported by transformer
/*N*/
/*N*/ 				seqArgsNew.realloc( ++nNewLength );
/*N*/ 				seqArgsNew[ nNewLength - 1 ].Name = m_pData->m_seqArguments[nOrg].Name;
/*N*/ 				seqArgsNew[ nNewLength - 1 ].Value = m_pData->m_seqArguments[nOrg].Value;
/*N*/ 			}
/*N*/
/*N*/ 		}
/*N*/ 		m_pData->m_seqArguments = seqArgsNew;
/*N*/ 	}
/*N*/
/*N*/ 	return m_pData->m_seqArguments ;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::connectController( const REFERENCE< XCONTROLLER >& xController )
/*N*/     throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
/*N*/ 	SEQUENCE< REFERENCE< XCONTROLLER > > aNewSeq( nOldCount + 1 );
/*N*/ 	for ( sal_uInt32 n = 0; n < nOldCount; n++ )
/*N*/ 		aNewSeq.getArray()[n] = m_pData->m_seqControllers.getConstArray()[n];
/*N*/ 	aNewSeq.getArray()[nOldCount] = xController;
/*N*/ 	m_pData->m_seqControllers = aNewSeq;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::disconnectController( const REFERENCE< XCONTROLLER >& xController ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
/*N*/     if ( !nOldCount )
/*N*/         return;
/*N*/
/*N*/ 	SEQUENCE< REFERENCE< XCONTROLLER > > aNewSeq( nOldCount - 1 );
/*N*/ 	for ( sal_uInt32 nOld = 0, nNew = 0; nOld < nOldCount; ++nOld )
/*N*/     {
/*N*/ 		if ( xController != m_pData->m_seqControllers.getConstArray()[nOld] )
/*N*/ 		{
/*N*/ 			aNewSeq.getArray()[nNew] = m_pData->m_seqControllers.getConstArray()[nOld];
/*N*/ 			++nNew;
/*N*/ 		}
/*N*/     }
/*N*/
/*N*/ 	m_pData->m_seqControllers = aNewSeq;
/*N*/
/*N*/ 	if ( xController == m_pData->m_xCurrent )
/*N*/ 		m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::lockControllers() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/ 	++m_pData->m_nControllerLockCount ;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::unlockControllers() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/ 	--m_pData->m_nControllerLockCount ;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::hasControllersLocked() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/ 	return ( m_pData->m_nControllerLockCount != 0 ) ;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ REFERENCE< XCONTROLLER > SAL_CALL SfxBaseModel::getCurrentController() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/     // get the last active controller of this model
/*N*/ 	if ( m_pData->m_xCurrent.is() )
/*N*/ 		return m_pData->m_xCurrent;
/*N*/
/*N*/ 	// get the first controller of this model
/*N*/ 	return m_pData->m_seqControllers.getLength() ? m_pData->m_seqControllers.getConstArray()[0] : m_pData->m_xCurrent;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::setCurrentController( const REFERENCE< XCONTROLLER >& xCurrentController )
/*N*/         throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	m_pData->m_xCurrent = xCurrentController;
/*N*/ }

//________________________________________________________________________________________________________
//	XMODEL
//________________________________________________________________________________________________________

/*N*/ REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getCurrentSelection() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/     REFERENCE< XINTERFACE >     xReturn;
/*N*/ 	REFERENCE< XCONTROLLER >	xController	=	getCurrentController()		;
/*N*/
/*N*/ 	if ( xController.is() )
/*N*/ 	{
/*N*/ 		REFERENCE< XSELECTIONSUPPLIER >  xDocView( xController, UNO_QUERY );
/*N*/ 		if ( xDocView.is() )
/*N*/ 		{
/*N*/ 			ANY xSel = xDocView->getSelection();
/*N*/ 	// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
/*N*/ 	//		return xSel.getReflection() == XINTERFACE_getReflection()
/*N*/ 	//		return xSel.getValueType() == ::getCppuType((const XINTERFACE*)0)
/*N*/ 	//				? *(REFERENCE< XINTERFACE > *) xSel.get() : REFERENCE< XINTERFACE > ();
/*N*/ 			xSel >>= xReturn ;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return xReturn ;
/*N*/ }

//________________________________________________________________________________________________________
//	XModifiable
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::isModified() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsModified() : sal_False;
/*N*/ }

//________________________________________________________________________________________________________
//	XModifiable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::setModified( sal_Bool bModified )
/*N*/         throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/ 		m_pData->m_pObjectShell->SetModified(bModified);
/*N*/ }

//________________________________________________________________________________________________________
//	XModifiable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::addModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0),xListener );
/*N*/ }

//________________________________________________________________________________________________________
//	XModifiable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::removeModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xListener );
/*N*/ }

//____________________________________________________________________________________________________
//  XCloseable
//____________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::close( sal_Bool bDeliverOwnership ) throw (CLOSEVETOEXCEPTION, RUNTIMEEXCEPTION)
/*N*/ {
/*N*/ 	::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( !m_pData || m_pData->m_bClosed || m_pData->m_bClosing )
/*N*/ 		return;
/*N*/
/*N*/     uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );
/*N*/     lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
/*N*/     ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
/*N*/     if (pContainer!=NULL)
/*N*/ 	{
/*N*/         ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
/*N*/         while (pIterator.hasMoreElements())
/*N*/         {
/*N*/             try
/*N*/             {
/*N*/                 ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
/*N*/             }
/*N*/             catch( uno::RuntimeException& )
/*N*/             {
/*N*/                 pIterator.remove();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/
/*N*/ 	// no own objections against closing!
/*N*/ 	m_pData->m_bClosing = sal_True;
/*N*/     m_pData->m_pObjectShell->Broadcast( SfxSimpleHint(SFX_HINT_DEINITIALIZING) );
/*N*/     pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
/*N*/     if (pContainer!=NULL)
/*N*/ 	{
/*N*/         ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
/*N*/         while (pCloseIterator.hasMoreElements())
/*N*/         {
/*N*/             try
/*N*/             {
/*?*/                 DBG_BF_ASSERT(0, "STRIP");//STRIP001 ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
/*?*/             }
/*?*/             catch( uno::RuntimeException& )
/*?*/             {
/*?*/                 pCloseIterator.remove();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/
/*N*/ 	m_pData->m_bClosed = sal_True;
/*N*/ 	m_pData->m_bClosing = sal_False;
/*N*/
/*N*/     dispose();
/*N*/ }

//____________________________________________________________________________________________________
//  XCloseBroadcaster
//____________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::addCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XCLOSELISTENER >*)0), xListener );
/*N*/ }

//____________________________________________________________________________________________________
//  XCloseBroadcaster
//____________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::removeCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XCLOSELISTENER >*)0), xListener );
/*N*/ }

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________
/*?*/ void SAL_CALL SfxBaseModel::print(const SEQUENCE< PROPERTYVALUE >& rOptions)
/*?*/         throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*?*/ }

//________________________________________________________________________________________________________
//	XStorable
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::hasLocation() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->HasName() : sal_False;
/*N*/ }

//________________________________________________________________________________________________________
//	XStorable
//________________________________________________________________________________________________________

/*N*/ OUSTRING SAL_CALL SfxBaseModel::getLocation() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	return m_pData->m_pObjectShell.Is() ? OUSTRING(m_pData->m_pObjectShell->GetMedium()->GetName()) : m_pData->m_sURL;
/*N*/ }

//________________________________________________________________________________________________________
//	XStorable
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::isReadonly() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsReadOnly() : sal_True;
/*N*/ }

//________________________________________________________________________________________________________
//	XStorable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/     {
/*N*/ 		if ( m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
/*N*/ 		{
/*N*/ 			if ( m_pData->m_pObjectShell->DoSave() )
/*N*/ 				m_pData->m_pObjectShell->DoSaveCompleted();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			m_pData->m_pObjectShell->Save_Impl();
/*N*/     }
/*N*/ }

//________________________________________________________________________________________________________
//	XStorable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::storeAsURL(	const	OUSTRING&					rURL	,
/*N*/ 										const	SEQUENCE< PROPERTYVALUE >&	rArgs	)
/*N*/         throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/ 	{
/*N*/ 		impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_False );
/*N*/
/*N*/ 		SEQUENCE< PROPERTYVALUE > aSequence	;
/*N*/ 		TransformItems( SID_OPENDOC, *m_pData->m_pObjectShell->GetMedium()->GetItemSet(), aSequence );
/*N*/ 		attachResource( rURL, aSequence );
/*N*/ 	}
/*N*/ }

//________________________________________________________________________________________________________
//	XStorable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::storeToURL(	const	OUSTRING&					rURL	,
/*N*/ 										const	SEQUENCE< PROPERTYVALUE >&	rArgs	)
/*N*/         throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/     {
/*N*/ 		impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_True );
/*N*/     }
/*N*/ }

//________________________________________________________________________________________________________
// XLoadable
//________________________________________________________________________________________________________

/*N #dochnoetig# */ void SAL_CALL SfxBaseModel::initNew()
/*N*/ 		throw (::com::sun::star::frame::DoubleInitializationException,
/*N*/ 			   ::com::sun::star::io::IOException,
/*N*/ 			   ::com::sun::star::uno::RuntimeException,
/*N*/ 			   ::com::sun::star::uno::Exception)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	// the object shell should exist always
/*N*/ 	DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );
/*N*/ 	if ( m_pData->m_pObjectShell.Is() )
/*N*/ 	{
/*N*/ 		if( m_pData->m_pObjectShell->GetMedium() )
/*N*/ 			throw DOUBLEINITIALIZATIONEXCEPTION();
/*N*/
/*N*/ 		sal_Bool bRes = m_pData->m_pObjectShell->DoInitNew( NULL );
/*N*/ 		sal_uInt32 nErrCode = m_pData->m_pObjectShell->GetError() ?
/*N*/ 									m_pData->m_pObjectShell->GetError() : ERRCODE_IO_CANTCREATE;
/*N*/ 		m_pData->m_pObjectShell->ResetError();
/*N*/
/*N*/ 		if ( !bRes )
/*N*/ 		{
/*N*/ 			throw SfxIOException_Impl( nErrCode );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//________________________________________________________________________________________________________
// XLoadable
//________________________________________________________________________________________________________

/*N #dochnoetig# */ void SAL_CALL SfxBaseModel::load(	const SEQUENCE< PROPERTYVALUE >& seqArguments )
/*N*/ 		throw (::com::sun::star::frame::DoubleInitializationException,
/*N*/                ::com::sun::star::io::IOException,
/*N*/ 			   ::com::sun::star::uno::RuntimeException,
/*N*/ 			   ::com::sun::star::uno::Exception)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	// the object shell should exist always
/*N*/ 	DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );
    if ( m_pData->m_pObjectShell.Is() )
     {
         if( m_pData->m_pObjectShell->GetMedium() )
             throw DOUBLEINITIALIZATIONEXCEPTION();

         SfxAllItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
         TransformParameters( SID_OPENDOC, seqArguments, *pParams );

         ::rtl::OUString aFilterName;
         SFX_ITEMSET_ARG( pParams, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
         if( pFilterNameItem )
             aFilterName = pFilterNameItem->GetValue();

         if( !aFilterName.getLength() )
             throw ILLEGALARGUMENTIOEXCEPTION();

        const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aFilterName );
        BOOL bReadOnly = FALSE;
        SFX_ITEMSET_ARG( pParams, pReadOnlyItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
        if ( pReadOnlyItem && pReadOnlyItem->GetValue() )
            bReadOnly = TRUE;
        SFX_ITEMSET_ARG( pParams, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
        SfxMedium* pMedium = new SfxMedium( pFileNameItem->GetValue(), bReadOnly ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE, FALSE, pFilter, pParams );

        // allow to use an interactionhandler (if there is one)
        pMedium->UseInteractionHandler( TRUE );

        // load document
        sal_uInt32 nError = ERRCODE_NONE;
        BOOL bOK = m_pData->m_pObjectShell->DoLoad(pMedium);
        m_pData->m_pObjectShell->ResetError();
        nError = pMedium->GetError();
        if ( !nError && !bOK )
            nError = ERRCODE_IO_GENERAL;

        if ( nError )
        {
            if ( m_pData->m_pObjectShell->GetMedium() != pMedium )
            {
                // for whatever reason document now has another medium
                DBG_ERROR("Document has rejected the medium?!");
                delete pMedium;
            }

            throw SfxIOException_Impl( nError ? nError : ERRCODE_IO_CANTREAD );
        }
    }
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________

/*?*/ ANY SAL_CALL SfxBaseModel::getTransferData( const DATAFLAVOR& aFlavor )
/*?*/ 		throw (::com::sun::star::datatransfer::UnsupportedFlavorException,
/*?*/ 			   ::com::sun::star::io::IOException,
/*?*/ 			   ::com::sun::star::uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); ANY aAny; return aAny;//STRIP001
/*?*/ }

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________


/*?*/ SEQUENCE< DATAFLAVOR > SAL_CALL SfxBaseModel::getTransferDataFlavors()
/*?*/ 		throw (::com::sun::star::uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); SEQUENCE< DATAFLAVOR > aDATAFLAVOR(0); return aDATAFLAVOR;//STRIP001
/*?*/ }

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________




//--------------------------------------------------------------------------------------------------------
//	XEventsSupplier
//--------------------------------------------------------------------------------------------------------

/*N*/ REFERENCE< XNAMEREPLACE > SAL_CALL SfxBaseModel::getEvents() throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	if ( ! m_pData->m_xEvents.is() )
/*N*/ 	{
/*N*/ 		m_pData->m_xEvents = new SfxEvents_Impl( m_pData->m_pObjectShell, this );
/*N*/ 	}
/*N*/
/*N*/ 	return m_pData->m_xEvents;
/*N*/ }

//--------------------------------------------------------------------------------------------------------
//	XEventBroadcaster
//--------------------------------------------------------------------------------------------------------

/*N*/ void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XDOCEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), aListener );
/*N*/ }

//--------------------------------------------------------------------------------------------------------
//	XEventBroadcaster
//--------------------------------------------------------------------------------------------------------

/*N*/ void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XDOCEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), aListener );
/*N*/ }

//________________________________________________________________________________________________________
//	SfxListener
//________________________________________________________________________________________________________

/*N*/ void addTitle_Impl( Sequence < ::com::sun::star::beans::PropertyValue >& rSeq, const ::rtl::OUString& rTitle )
/*N*/ {
/*N*/     sal_Int32 nCount = rSeq.getLength();
/*N*/     sal_Int32 nArg; for ( nArg=0; nArg<nCount; nArg++ )
/*N*/     {
/*N*/         ::com::sun::star::beans::PropertyValue& rProp = rSeq[nArg];
/*N*/         if ( rProp.Name.equalsAscii("Title") )
/*N*/         {
/*N*/             rProp.Value <<= rTitle;
/*N*/             break;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     if ( nArg == nCount )
/*N*/     {
/*N*/         rSeq.realloc( nCount+1 );
/*N*/         rSeq[nCount].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Title") );
/*N*/         rSeq[nCount].Value <<= rTitle;
/*N*/     }
/*N*/ }

/*N*/ void SfxBaseModel::Notify(			SfxBroadcaster&	rBC		,
/*N*/ 	 						const	SfxHint&		rHint	)
/*N*/ {
/*N*/ 	if ( !m_pData )
/*N*/ 		return;
/*N*/
/*N*/ 	if ( &rBC == m_pData->m_pObjectShell )
/*N*/ 	{
/*N*/ 		SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
/*N*/ 		if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DOCCHANGED )
/*N*/ 			changing();
/*N*/
/*N*/ 		SfxEventHint* pNamedHint = PTR_CAST( SfxEventHint, &rHint );
/*N*/ 		if ( pNamedHint )
/*N*/ 		{
/*N*/ 			if ( SFX_EVENT_SAVEASDOCDONE == pNamedHint->GetEventId() )
/*N*/             {
/*N*/ 				m_pData->m_sURL = m_pData->m_pObjectShell->GetMedium()->GetName();
/*N*/                 SfxItemSet *pSet = m_pData->m_pObjectShell->GetMedium()->GetItemSet();
/*N*/                 ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
/*N*/                 ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
/*N*/                 TransformItems( SID_SAVEASDOC, *pSet, aArgs );
/*N*/                 addTitle_Impl( aArgs, aTitle );
/*N*/                 attachResource( m_pData->m_pObjectShell->GetMedium()->GetName(), aArgs );
/*N*/             }
/*N*/
/*N*/ 			postEvent_Impl( *pNamedHint );
/*N*/ 		}
/*N*/
/*N*/         if ( pSimpleHint )
/*N*/ 		{
/*N*/ 			if ( pSimpleHint->GetId() == SFX_HINT_TITLECHANGED )
/*N*/         	{
/*N*/             	::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
/*N*/             	addTitle_Impl( m_pData->m_seqArguments, aTitle );
/*N*/         	}
/*
            else if ( pSimpleHint->GetId() == SFX_HINT_DYING
                || pSimpleHint->GetId() == SFX_HINT_DEINITIALIZING )
            {
                SfxObjectShellLock pShellLock = m_pData->m_pObjectShellLock;
                m_pData->m_pObjectShellLock = SfxObjectShellLock();
            }
*/
/*N*/ 		}
/*N*/
/*N*/ 	}
/*N*/ }

//________________________________________________________________________________________________________
//	public impl.
//________________________________________________________________________________________________________

/*N*/ void SfxBaseModel::changing()
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0) );
/*N*/ 	if( pIC )
/*N*/
/*N*/ 	{
/*N*/ 		EVENTOBJECT aEvent( (XMODEL *)this );
/*N*/ 		OINTERFACEITERATORHELPER aIt( *pIC );
/*N*/ 		while( aIt.hasMoreElements() )
/*N*/         {
/*N*/             try
/*N*/             {
/*N*/                 ((XMODIFYLISTENER *)aIt.next())->modified( aEvent );
/*N*/             }
/*N*/             catch( RUNTIMEEXCEPTION& )
/*N*/             {
/*N*/                 aIt.remove();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ }

//________________________________________________________________________________________________________
//	public impl.
//________________________________________________________________________________________________________

/*N*/ SfxObjectShell* SfxBaseModel::GetObjectShell() const
/*N*/ {
/*N*/ 	return m_pData ? (SfxObjectShell*) m_pData->m_pObjectShell : 0;
/*N*/ }

//________________________________________________________________________________________________________
//	public impl.
//________________________________________________________________________________________________________

/*N*/ sal_Bool SfxBaseModel::impl_isDisposed() const
/*N*/ {
/*N*/ 	return ( m_pData == NULL ) ;
/*N*/ }

//________________________________________________________________________________________________________
//	private impl.
//________________________________________________________________________________________________________

/*N*/ void SfxBaseModel::impl_store(			SfxObjectShell*				pObjectShell	,
/*N*/ 								const	OUSTRING&					sURL			,
/*N*/ 								const	SEQUENCE< PROPERTYVALUE >&	seqArguments	,
/*N*/ 										sal_Bool					bSaveTo			)
/*N*/ {
/*N*/ 	if( !sURL.getLength() )
/*N*/ 		throw ILLEGALARGUMENTIOEXCEPTION();
/*N*/
/*N*/ 	//sal_Bool aSaveAsTemplate = sal_False;
/*N*/
/*N*/     SfxAllItemSet *aParams = new SfxAllItemSet( SFX_APP()->GetPool() );
/*N*/ 	aParams->Put( SfxStringItem( SID_FILE_NAME, String(sURL) ) );
/*N*/ 	if ( bSaveTo )
/*N*/ 		aParams->Put( SfxBoolItem( SID_SAVETO, sal_True ) );
/*N*/
/*N*/     TransformParameters( SID_SAVEASDOC, seqArguments, *aParams );
/*N*/ 	sal_Bool aRet = pObjectShell->APISaveAs_Impl( sURL, aParams );
/*N*/ 	DELETEZ( aParams );
/*N*/
/*N*/ 	sal_uInt32 nErrCode = pObjectShell->GetError() ? pObjectShell->GetError() : ERRCODE_IO_CANTWRITE;
/*N*/ 	pObjectShell->ResetError();
/*N*/
/*N*/ 	if ( !aRet )
/*N*/ 	{
/*N*/ 		throw SfxIOException_Impl( nErrCode );
/*N*/ 	}
/*N*/ }
/*N*/
//********************************************************************************************************

/*N*/ void SfxBaseModel::postEvent_Impl( const SfxEventHint& rHint )
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer(
/*N*/ 										::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) );
/*N*/ 	if( pIC )
/*N*/
/*N*/ 	{
/*N*/ 		OUSTRING aName = SfxEventConfiguration::GetEventName_Impl( rHint.GetEventId() );
/*N*/ 		DOCEVENTOBJECT aEvent( (XMODEL *)this, aName );
/*N*/ 		OINTERFACEITERATORHELPER aIt( *pIC );
/*N*/ 		while( aIt.hasMoreElements() )
/*N*/         {
/*N*/             try
/*N*/             {
/*N*/                 ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
/*N*/             }
/*N*/             catch( RUNTIMEEXCEPTION& )
/*N*/             {
/*N*/                 aIt.remove();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ }

/*N*/ REFERENCE < XINDEXACCESS > SAL_CALL SfxBaseModel::getViewData() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 			return REFERENCE < XINDEXACCESS >();
/*N*/
/*N*/ }

/*N*/ void SAL_CALL SfxBaseModel::setViewData( const REFERENCE < XINDEXACCESS >& aData ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/     ::vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		throw DISPOSEDEXCEPTION();
/*N*/
/*N*/ 	m_pData->m_contViewData = aData;
/*N*/ }

/** calls all XEventListeners */
/*N*/ void SfxBaseModel::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) const
/*N*/ {
/*N*/ 	// object already disposed?
/*N*/ 	if ( impl_isDisposed() )
/*N*/ 		return;
/*N*/
/*N*/ 	OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer(
/*N*/ 										::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) );
/*N*/ 	if( pIC )
/*N*/
/*N*/ 	{
/*N*/ 		OINTERFACEITERATORHELPER aIt( *pIC );
/*N*/ 		while( aIt.hasMoreElements() )
/*N*/         {
/*N*/             try
/*N*/             {
/*N*/                 ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
/*N*/             }
/*N*/             catch( RUNTIMEEXCEPTION& )
/*N*/             {
/*N*/                 aIt.remove();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ }

/** returns true if someone added a XEventListener to this XEventBroadcaster */
/*N*/ sal_Bool SfxBaseModel::hasEventListeners() const
/*N*/ {
/*N*/ 	return !impl_isDisposed() && (NULL != m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) ) );
/*N*/ }



}
