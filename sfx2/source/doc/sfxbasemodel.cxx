/*************************************************************************
 *
 *  $RCSfile: sfxbasemodel.cxx,v $
 *
 *  $Revision: 1.51 $
 *
 *  last change: $Author: rt $ $Date: 2003-07-31 10:02:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//________________________________________________________________________________________________________
//  my own includes
//________________________________________________________________________________________________________

#ifndef _SFX_SFXBASEMODEL_HXX_
#include <sfxbasemodel.hxx>
#endif

//________________________________________________________________________________________________________
//  include of other projects
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
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

#include <svtools/sbx.hxx>
#include <basic/sbuno.hxx>

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
#include <svtools/printdlg.hxx>

//________________________________________________________________________________________________________
//  includes of my own project
//________________________________________________________________________________________________________

#ifndef _SFX_SFXUNO_HXX
#include <sfxuno.hxx>
#endif

#ifndef _SFX_OBJSHIMP_HXX
#include <objshimp.hxx>
#endif

#ifndef _SFXVIEWFRM_HXX
#include <viewfrm.hxx>
#endif

#ifndef _SFXVIEWSH_HXX
#include <viewsh.hxx>
#endif

#ifndef _SFXDOCFILE_HXX
#include <docfile.hxx>
#endif

#ifndef _SFXDISPATCH_HXX
#include <dispatch.hxx>
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

#ifndef _SFX_BASMGR_HXX
#include <basmgr.hxx>
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


#include "topfrm.hxx"
#include "appdata.hxx"
#include "loadenv.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"

//________________________________________________________________________________________________________
//  defines
//________________________________________________________________________________________________________

#define SfxIOException_Impl( nErr )             ::com::sun::star::io::IOException()

#define XFRAME                                  ::com::sun::star::frame::XFrame
#define XINTERFACE                              ::com::sun::star::uno::XInterface
#define OMULTITYPEINTERFACECONTAINERHELPER      ::cppu::OMultiTypeInterfaceContainerHelper
#define UNO_QUERY                               ::com::sun::star::uno::UNO_QUERY
#define DISPOSEDEXCEPTION                       ::com::sun::star::lang::DisposedException
#define MAPPING                                 ::com::sun::star::uno::Mapping
#define XSELECTIONSUPPLIER                      ::com::sun::star::view::XSelectionSupplier
#define ANY                                     ::com::sun::star::uno::Any
#define ILLEGALARGUMENTEXCEPTION                ::com::sun::star::lang::IllegalArgumentException
#define ILLEGALARGUMENTIOEXCEPTION              ::com::sun::star::frame::IllegalArgumentIOException
#define DOUBLEINITIALIZATIONEXCEPTION           ::com::sun::star::frame::DoubleInitializationException
#define OINTERFACECONTAINERHELPER               ::cppu::OInterfaceContainerHelper
#define OINTERFACEITERATORHELPER                ::cppu::OInterfaceIteratorHelper
#define SIZE                                    ::com::sun::star::awt::Size
#define PAPERFORMAT                             ::com::sun::star::view::PaperFormat
#define PAPERORIENTATION                        ::com::sun::star::view::PaperOrientation
#define OTYPECOLLECTION                         ::cppu::OTypeCollection
#define OIMPLEMENTATIONID                       ::cppu::OImplementationId
#define MUTEXGUARD                              ::osl::MutexGuard
#define XINDEXCONTAINER                         ::com::sun::star::container::XIndexContainer
#define UNSUPPORTEDFLAVOREXCEPTION              ::com::sun::star::datatransfer::UnsupportedFlavorException
#define XPRINTJOBLISTENER                       ::com::sun::star::view::XPrintJobListener

//________________________________________________________________________________________________________
//  namespaces
//________________________________________________________________________________________________________


// Don't use using ... here, because there are at least two classes with the same name in use

//using namespace ::osl                             ;
//using namespace ::rtl                             ;
//using namespace ::cppu                            ;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
//using namespace ::com::sun::star::container       ;
//using namespace ::com::sun::star::frame           ;
//using namespace ::com::sun::star::document        ;
//using namespace ::com::sun::star::lang            ;
//using namespace ::com::sun::star::util            ;
//using namespace ::com::sun::star::view            ;
//using namespace ::com::sun::star::beans           ;

//________________________________________________________________________________________________________
//  impl. declarations
//________________________________________________________________________________________________________

struct IMPL_SfxBaseModel_DataContainer
{
    SfxObjectShellRef                               m_pObjectShell          ;
    //SfxObjectShellLock                                m_pObjectShellLock      ;
    OUSTRING                                        m_sURL                  ;
    sal_uInt16                                      m_nControllerLockCount  ;
    OMULTITYPEINTERFACECONTAINERHELPER              m_aInterfaceContainer   ;
    REFERENCE< XINTERFACE >                         m_xParent               ;
    REFERENCE< XCONTROLLER >                        m_xCurrent              ;
    REFERENCE< XDOCUMENTINFO >                      m_xDocumentInfo         ;
    REFERENCE< XSTARBASICACCESS >                   m_xStarBasicAccess      ;
    REFERENCE< XNAMEREPLACE >                       m_xEvents               ;
    SEQUENCE< PROPERTYVALUE>                        m_seqArguments          ;
    SEQUENCE< REFERENCE< XCONTROLLER > >            m_seqControllers        ;
    REFERENCE< XINDEXACCESS >                       m_contViewData          ;
       LoadEnvironment_Impl*                            m_pLoader               ;
    sal_Bool                                        m_bLoadDone             ;
    sal_Bool                                        m_bLoadState            ;
    sal_Bool                                        m_bClosed               ;
    sal_Bool                                        m_bClosing              ;
    REFERENCE< com::sun::star::view::XPrintJob>     m_xPrintJob             ;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aPrintOptions;

    IMPL_SfxBaseModel_DataContainer::IMPL_SfxBaseModel_DataContainer(   MUTEX&          aMutex          ,
                                                                        SfxObjectShell* pObjectShell    )
            :   m_pObjectShell          ( pObjectShell  )
//          ,   m_pObjectShellLock      ( pObjectShell  )
            ,   m_sURL                  ( String()      )
            ,   m_nControllerLockCount  ( 0             )
            ,   m_aInterfaceContainer   ( aMutex        )
            ,   m_pLoader               ( NULL          )
            ,   m_bLoadDone             ( sal_False     )
            ,   m_bLoadState            ( sal_False     )
            ,   m_bClosed               ( sal_False     )
            ,   m_bClosing              ( sal_False     )
    {
    }
} ;

SIZE impl_Size_Object2Struct( const Size& aSize )
{
    SIZE aReturnValue;

    aReturnValue.Width  = aSize.Width()  ;
    aReturnValue.Height = aSize.Height() ;

    return aReturnValue ;
}

Size impl_Size_Struct2Object( const SIZE& aSize )
{
    Size aReturnValue;

    aReturnValue.Width()  = aSize.Width  ;
    aReturnValue.Height() = aSize.Height ;

    return aReturnValue ;
}

extern void* getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta );
extern void* getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize );
extern SvMemoryStream* getMetaMemStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, sal_uInt32 nFormat );
extern sal_Bool supportsMetaFileHandle_Impl();

class SfxPrintJob_Impl : public cppu::WeakImplHelper1
<
    com::sun::star::view::XPrintJob
>
{
        IMPL_SfxBaseModel_DataContainer* m_pData;

public:
        SfxPrintJob_Impl( IMPL_SfxBaseModel_DataContainer* pData );
        virtual Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPrintOptions(  ) throw (RuntimeException);
        virtual Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPrinter(  ) throw (RuntimeException);
        virtual Reference< ::com::sun::star::view::XPrintable > SAL_CALL getPrintable(  ) throw (RuntimeException);
        virtual void SAL_CALL cancelJob() throw (RuntimeException);
};

SfxPrintJob_Impl::SfxPrintJob_Impl( IMPL_SfxBaseModel_DataContainer* pData )
    : m_pData( pData )
{
}

Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SfxPrintJob_Impl::getPrintOptions() throw (RuntimeException)
{
    return m_pData->m_aPrintOptions;
}

Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SfxPrintJob_Impl::getPrinter() throw (RuntimeException)
{
    Reference < view::XPrintable > xPrintable( m_pData->m_pObjectShell->GetModel(), UNO_QUERY );
    if ( xPrintable.is() )
        return xPrintable->getPrinter();
    return Sequence< ::com::sun::star::beans::PropertyValue >();
}

Reference< ::com::sun::star::view::XPrintable > SAL_CALL SfxPrintJob_Impl::getPrintable() throw (RuntimeException)
{
    Reference < view::XPrintable > xPrintable( m_pData->m_pObjectShell->GetModel(), UNO_QUERY );
    return xPrintable;
}

void SAL_CALL SfxPrintJob_Impl::cancelJob() throw (RuntimeException)
{
    m_pData->m_pObjectShell->Broadcast( SfxPrintingHint( -2, NULL, NULL ) );
}

//________________________________________________________________________________________________________
//  constructor
//________________________________________________________________________________________________________

SfxBaseModel::SfxBaseModel( SfxObjectShell *pObjectShell )
: IMPL_SfxBaseModel_MutexContainer()
, m_pData( new IMPL_SfxBaseModel_DataContainer( m_aMutex, pObjectShell ) )
{
    if ( pObjectShell != NULL )
    {
        StartListening( *pObjectShell ) ;
    }
}

//________________________________________________________________________________________________________
//  destructor
//________________________________________________________________________________________________________

SfxBaseModel::~SfxBaseModel()
{
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

ANY SAL_CALL SfxBaseModel::queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    ANY aReturn( ::cppu::queryInterface(    rType                                           ,
                                               static_cast< XTYPEPROVIDER*          > ( this )  ,
                                               static_cast< XCHILD*             > ( this )  ,
                                               static_cast< XDOCUMENTINFOSUPPLIER*  > ( this )  ,
                                               static_cast< XEVENTLISTENER*     > ( this )  ,
                                               static_cast< XMODEL*             > ( this )  ,
                                               static_cast< XMODIFIABLE*            > ( this )  ,
                                            static_cast< XCOMPONENT*            > ( this )  ,
                                               static_cast< XPRINTABLE*         > ( this )  ,
                                               static_cast< XSTARBASICACCESS*       > ( this )  ,
                                            static_cast< XSTORABLE*             > ( this )  ,
                                            static_cast< XLOADABLE*             > ( this )  ,
                                            static_cast< XCLOSEABLE*            > ( this )  ) );

    if ( aReturn.hasValue() == sal_False )
    {
        aReturn = ::cppu::queryInterface(   rType                                           ,
                                            static_cast< XMODIFYBROADCASTER*    > ( this )  ,
                                            static_cast< XTRANSFERABLE*    > ( this )  ,
                                               static_cast< XPRINTJOBBROADCASTER*   > ( this )  ,
                                                static_cast< XCLOSEBROADCASTER*     > ( this )  ,
                                            static_cast< XVIEWDATASUPPLIER*     > ( this )  ,
                                               static_cast< XEVENTBROADCASTER*      > ( this )  ,
                                               static_cast< XEVENTSSUPPLIER*        > ( this )  ) ;
    }
    // If searched interface supported by this class ...
    if ( aReturn.hasValue() == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface( rType ) ;
    }
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::acquire() throw( )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire() ;
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::release() throw( )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release() ;
}

//________________________________________________________________________________________________________
//  XTypeProvider
//________________________________________________________________________________________________________

SEQUENCE< UNOTYPE > SAL_CALL SfxBaseModel::getTypes() throw( RUNTIMEEXCEPTION )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTYPECOLLECTION* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTYPECOLLECTION aTypeCollectionFirst( ::getCppuType(( const REFERENCE< XTYPEPROVIDER          >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XCHILD                 >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XDOCUMENTINFOSUPPLIER  >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XEVENTLISTENER         >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XMODEL                 >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XMODIFIABLE            >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XPRINTABLE             >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XSTORABLE              >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XLOADABLE              >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XCLOSEABLE             >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XSTARBASICACCESS       >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XEVENTBROADCASTER      >*)NULL ) );

            static OTYPECOLLECTION aTypeCollection     ( ::getCppuType(( const REFERENCE< XVIEWDATASUPPLIER      >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XTRANSFERABLE          >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XPRINTJOBBROADCASTER   >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XEVENTSSUPPLIER        >*)NULL ) ,
                                                         ::getCppuType(( const REFERENCE< XCLOSEBROADCASTER      >*)NULL ) ,
                                                         aTypeCollectionFirst.getTypes()                                   );

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

//________________________________________________________________________________________________________
//  XTypeProvider
//________________________________________________________________________________________________________

SEQUENCE< sal_Int8 > SAL_CALL SfxBaseModel::getImplementationId() throw( RUNTIMEEXCEPTION )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)

    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static OIMPLEMENTATIONID* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static OIMPLEMENTATIONID aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//________________________________________________________________________________________________________
//  XStarBasicAccess
//________________________________________________________________________________________________________

REFERENCE< XSTARBASICACCESS > implGetStarBasicAccess( SfxObjectShell* pObjectShell )
{
    REFERENCE< XSTARBASICACCESS > xRet;
    if( pObjectShell )
    {
        BasicManager* pMgr = pObjectShell->GetBasicManager();
        xRet = getStarBasicAccess( pMgr );
    }
    return xRet;
}

REFERENCE< XNAMECONTAINER > SAL_CALL SfxBaseModel::getLibraryContainer() throw( RUNTIMEEXCEPTION )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    REFERENCE< XNAMECONTAINER > xRet;
    if( rxAccess.is() )
        xRet = rxAccess->getLibraryContainer();
    return xRet;
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::createLibrary( const OUSTRING& LibName, const OUSTRING& Password,
    const OUSTRING& ExternalSourceURL, const OUSTRING& LinkTargetURL )
        throw(ELEMENTEXISTEXCEPTION, RUNTIMEEXCEPTION)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->createLibrary( LibName, Password, ExternalSourceURL, LinkTargetURL );
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addModule( const OUSTRING& LibraryName, const OUSTRING& ModuleName,
    const OUSTRING& Language, const OUSTRING& Source )
        throw( NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->addModule( LibraryName, ModuleName, Language, Source );
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addDialog( const OUSTRING& LibraryName, const OUSTRING& DialogName,
    const ::com::sun::star::uno::Sequence< sal_Int8 >& Data )
        throw(NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->addDialog( LibraryName, DialogName, Data );
}


//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getParent() throw( RUNTIMEEXCEPTION )
{
/*  #77222#
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_xParent;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setParent(const REFERENCE< XINTERFACE >& Parent) throw(NOSUPPORTEXCEPTION, RUNTIMEEXCEPTION)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( Parent.is() && getParent().is() )
        // only set parent when no parent is available
        throw NOSUPPORTEXCEPTION();

    m_pData->m_xParent = Parent;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

long SfxObjectShellClose_Impl( void* pObj, void* pArg );

void SAL_CALL SfxBaseModel::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if  ( !m_pData->m_bClosed )
    {
        // gracefully accept wrong dispose calls instead of close call
        // and try to make it work (may be really disposed later!)
        try
        {
            close( sal_True );
        }
        catch ( com::sun::star::util::CloseVetoException& )
        {
        }

        return;
    }

    EVENTOBJECT aEvent( (XMODEL *)this );
    m_pData->m_aInterfaceContainer.disposeAndClear( aEvent );

    // is an object shell assigned?
    if ( m_pData->m_pObjectShell.Is() )
    {
        // Rekursion vermeiden
        SfxObjectShellRef pShell;
        //SfxObjectShellLock pShellLock;

        {
            // am I "ThisComponent" in AppBasic?
            StarBASIC* pBas = SFX_APP()->GetBasic_Impl();
            if ( pBas && SFX_APP()->Get_Impl()->pThisDocument == m_pData->m_pObjectShell )
            {
                // remove "ThisComponent" reference from AppBasic
                SFX_APP()->Get_Impl()->pThisDocument = NULL;
                SbxVariable *pCompVar = pBas->Find( DEFINE_CONST_UNICODE("ThisComponent"), SbxCLASS_OBJECT );
                if ( pCompVar )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInterface;
                    ::com::sun::star::uno::Any aComponent;
                    aComponent <<= xInterface;
                    pCompVar->PutObject( GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aComponent ) );
                }
            }

            pShell = m_pData->m_pObjectShell;
            //pShellLock = m_pData->m_pObjectShellLock;
            EndListening( *pShell );
            m_pData->m_pObjectShell = SfxObjectShellRef();
            //m_pData->m_pObjectShellLock = SfxObjectShellLock();
        }

        // Bei dispose keine Speichern-R"uckfrage
        if ( pShell->IsEnableSetModified() && !pShell->Get_Impl()->bClosing )
            pShell->SetModified( sal_False );
        pShell->Get_Impl()->bDisposing = TRUE;
        //pShellLock = SfxObjectShellLock();
        SfxObjectShellClose_Impl( 0, (void*) pShell );
    }

    m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
    m_pData->m_seqControllers = SEQUENCE< REFERENCE< XCONTROLLER > > () ;

    DELETEZ(m_pData);
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  XDOCUMENTINFOSupplier
//________________________________________________________________________________________________________

REFERENCE< XDOCUMENTINFO > SAL_CALL SfxBaseModel::getDocumentInfo() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( !m_pData->m_xDocumentInfo.is() && m_pData->m_pObjectShell.Is() )
        ((SfxBaseModel*)this)->m_pData->m_xDocumentInfo = new SfxDocumentInfoObject( m_pData->m_pObjectShell ) ;

    return m_pData->m_xDocumentInfo;
}

//________________________________________________________________________________________________________
//  XEVENTLISTENER
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::disposing( const EVENTOBJECT& aObject )
    throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    REFERENCE< XMODIFYLISTENER >  xMod( aObject.Source, UNO_QUERY );
    REFERENCE< XEVENTLISTENER >  xListener( aObject.Source, UNO_QUERY );
    REFERENCE< XDOCEVENTLISTENER >  xDocListener( aObject.Source, UNO_QUERY );

    if ( xMod.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xMod );
    else if ( xListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), xListener );
    else if ( xDocListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), xListener );
/*
    sal_uInt32 nCount = m_pData->m_seqControllers.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        if( m_pData->m_seqControllers.getConstArray()[n] == aObject.Source )
        {
            m_pData->m_seqControllers.getArray()[n] = REFERENCE< XCONTROLLER > () ;
            break;
        }
    }

    if ( m_pData->m_xCurrent.is() && m_pData->m_xCurrent == aObject.Source )
        m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
*/
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::attachResource( const   OUSTRING&                   rURL    ,
                                                const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
    throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( rURL.getLength() == 0 && rArgs.getLength() == 1 && rArgs[0].Name.equalsAscii( "SetEmbedded" ) )
    {
        // allows to set a windowless document to EMBEDDED state
        // but _only_ before load() or initNew() methods
        if ( m_pData->m_pObjectShell.Is() && !m_pData->m_pObjectShell->GetMedium() )
        {
            sal_Bool bEmb;
            if ( ( rArgs[0].Value >>= bEmb ) && bEmb )
                m_pData->m_pObjectShell->SetCreateMode_Impl( SFX_CREATE_MODE_EMBEDDED );
        }

        return sal_True;
    }

    if ( m_pData->m_pObjectShell.Is() )
    {
        m_pData->m_sURL = rURL;
        m_pData->m_seqArguments = rArgs;

        sal_Int32 nNewLength = rArgs.getLength();
        for ( sal_Int32 nInd = 0; nInd < rArgs.getLength(); nInd++ )
            if ( rArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // the document should be resized
                SfxInPlaceObject* pInPlaceObj = m_pData->m_pObjectShell->GetInPlaceObject();
                if ( pInPlaceObj )
                {
                    Sequence< sal_Int32 > aSize;
                    if ( ( rArgs[nInd].Value >>= aSize ) && aSize.getLength() == 4 )
                    {
                        Rectangle aTmpRect( aSize[0], aSize[1], aSize[2], aSize[3] );
                        aTmpRect = OutputDevice::LogicToLogic( aTmpRect, MAP_100TH_MM, pInPlaceObj->GetMapUnit() );

                        pInPlaceObj->SetVisArea( aTmpRect );
                    }
                }
            }

        if( m_pData->m_pObjectShell->GetMedium() )
        {
            SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
            TransformParameters( SID_OPENDOC, rArgs, aSet );
            m_pData->m_pObjectShell->GetMedium()->GetItemSet()->Put( aSet );
            SFX_ITEMSET_ARG( &aSet, pItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( pItem )
                m_pData->m_pObjectShell->GetMedium()->SetFilter(
                    m_pData->m_pObjectShell->GetFactory().GetFilterContainer()->GetFilter( pItem->GetValue() ) );
        }
    }

    return sal_True ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

OUSTRING SAL_CALL SfxBaseModel::getURL() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_sURL ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

SEQUENCE< PROPERTYVALUE > SAL_CALL SfxBaseModel::getArgs() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
    {
        SEQUENCE< PROPERTYVALUE > seqArgsNew;
        SEQUENCE< PROPERTYVALUE > seqArgsOld;
        SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );

        // we need to know which properties are supported by the transformer
        // hopefully it is a temporary solution, I guess nonconvertable properties
        // should not be supported so then there will be only ItemSet from medium

        TransformItems( SID_OPENDOC, *(m_pData->m_pObjectShell->GetMedium()->GetItemSet()), seqArgsNew );
        TransformParameters( SID_OPENDOC, m_pData->m_seqArguments, aSet );
        TransformItems( SID_OPENDOC, aSet, seqArgsOld );

        sal_Int32 nOrgLength = m_pData->m_seqArguments.getLength();
        sal_Int32 nOldLength = seqArgsOld.getLength();
        sal_Int32 nNewLength = seqArgsNew.getLength();

        // "WinExtent" property should be updated always.
        // We can store it now to overwrite an old value
        // since it is not from ItemSet
        SfxInPlaceObject* pInPlaceObj = m_pData->m_pObjectShell->GetInPlaceObject();
        if ( pInPlaceObj )
        {
            Rectangle aTmpRect = pInPlaceObj->GetVisArea( ASPECT_CONTENT );
            aTmpRect = OutputDevice::LogicToLogic( aTmpRect, pInPlaceObj->GetMapUnit(), MAP_100TH_MM );

            Sequence< sal_Int32 > aSize(4);
            aSize[0] = aTmpRect.Left();
            aSize[1] = aTmpRect.Top();
            aSize[2] = aTmpRect.Right();
            aSize[3] = aTmpRect.Bottom();

            seqArgsNew.realloc( ++nNewLength );
            seqArgsNew[ nNewLength - 1 ].Name = ::rtl::OUString::createFromAscii( "WinExtent" );
            seqArgsNew[ nNewLength - 1 ].Value <<= aSize;
        }

        for ( sal_Int32 nOrg = 0; nOrg < nOrgLength; nOrg++ )
        {
             sal_Int32 nOldInd = 0;
            while ( nOldInd < nOldLength )
            {
                if ( m_pData->m_seqArguments[nOrg].Name.equals( seqArgsOld[nOldInd].Name ) )
                    break;
                nOldInd++;
            }

            if ( nOldInd == nOldLength )
            {
                // the entity with this name should be new for seqArgsNew
                // since it is not supported by transformer

                seqArgsNew.realloc( ++nNewLength );
                seqArgsNew[ nNewLength - 1 ].Name = m_pData->m_seqArguments[nOrg].Name;
                seqArgsNew[ nNewLength - 1 ].Value = m_pData->m_seqArguments[nOrg].Value;
            }

        }
        m_pData->m_seqArguments = seqArgsNew;
    }

    return m_pData->m_seqArguments ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::connectController( const REFERENCE< XCONTROLLER >& xController )
    throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    SEQUENCE< REFERENCE< XCONTROLLER > > aNewSeq( nOldCount + 1 );
    for ( sal_uInt32 n = 0; n < nOldCount; n++ )
        aNewSeq.getArray()[n] = m_pData->m_seqControllers.getConstArray()[n];
    aNewSeq.getArray()[nOldCount] = xController;
    m_pData->m_seqControllers = aNewSeq;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::disconnectController( const REFERENCE< XCONTROLLER >& xController ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    if ( !nOldCount )
        return;

    SEQUENCE< REFERENCE< XCONTROLLER > > aNewSeq( nOldCount - 1 );
    for ( sal_uInt32 nOld = 0, nNew = 0; nOld < nOldCount; ++nOld )
    {
        if ( xController != m_pData->m_seqControllers.getConstArray()[nOld] )
        {
            aNewSeq.getArray()[nNew] = m_pData->m_seqControllers.getConstArray()[nOld];
            ++nNew;
        }
    }

    m_pData->m_seqControllers = aNewSeq;

    if ( xController == m_pData->m_xCurrent )
        m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::lockControllers() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();
    ++m_pData->m_nControllerLockCount ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::unlockControllers() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();
    --m_pData->m_nControllerLockCount ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::hasControllersLocked() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();
    return ( m_pData->m_nControllerLockCount != 0 ) ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

REFERENCE< XCONTROLLER > SAL_CALL SfxBaseModel::getCurrentController() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // get the last active controller of this model
    if ( m_pData->m_xCurrent.is() )
        return m_pData->m_xCurrent;

    // get the first controller of this model
    return m_pData->m_seqControllers.getLength() ? m_pData->m_seqControllers.getConstArray()[0] : m_pData->m_xCurrent;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setCurrentController( const REFERENCE< XCONTROLLER >& xCurrentController )
        throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_xCurrent = xCurrentController;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getCurrentSelection() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XINTERFACE >     xReturn;
    REFERENCE< XCONTROLLER >    xController =   getCurrentController()      ;

    if ( xController.is() )
    {
        REFERENCE< XSELECTIONSUPPLIER >  xDocView( xController, UNO_QUERY );
        if ( xDocView.is() )
        {
            ANY xSel = xDocView->getSelection();
    // automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
    //      return xSel.getReflection() == XINTERFACE_getReflection()
    //      return xSel.getValueType() == ::getCppuType((const XINTERFACE*)0)
    //              ? *(REFERENCE< XINTERFACE > *) xSel.get() : REFERENCE< XINTERFACE > ();
            xSel >>= xReturn ;
        }
    }

    return xReturn ;
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::isModified() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsModified() : sal_False;
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
        m_pData->m_pObjectShell->SetModified(bModified);
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0),xListener );
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xListener );
}

//____________________________________________________________________________________________________
//  XCloseable
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::close( sal_Bool bDeliverOwnership ) throw (CLOSEVETOEXCEPTION, RUNTIMEEXCEPTION)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_pData || m_pData->m_bClosed || m_pData->m_bClosing )
        return;

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );
    lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
            }
            catch( uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    // no own objections against closing!
    m_pData->m_bClosing = sal_True;
    m_pData->m_pObjectShell->Broadcast( SfxSimpleHint(SFX_HINT_DEINITIALIZING) );
    pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
        while (pCloseIterator.hasMoreElements())
        {
            try
            {
                ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
            }
            catch( uno::RuntimeException& )
            {
                pCloseIterator.remove();
            }
        }
    }

    m_pData->m_bClosed = sal_True;
    m_pData->m_bClosing = sal_False;

    dispose();
}

//____________________________________________________________________________________________________
//  XCloseBroadcaster
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XCLOSELISTENER >*)0), xListener );
}

//____________________________________________________________________________________________________
//  XCloseBroadcaster
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XCLOSELISTENER >*)0), xListener );
}

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________

SEQUENCE< PROPERTYVALUE > SAL_CALL SfxBaseModel::getPrinter() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // Printer beschaffen
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.Is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell, 0, sal_False ) : 0;
    if ( !pViewFrm )
        return SEQUENCE< PROPERTYVALUE >();
    const SfxPrinter *pPrinter = pViewFrm->GetViewShell()->GetPrinter(sal_True);
    if ( !pPrinter )
        return SEQUENCE< PROPERTYVALUE >();

    // Printer Eigenschaften uebertragen
    SEQUENCE< PROPERTYVALUE > aPrinter(8);

    aPrinter.getArray()[7].Name = DEFINE_CONST_UNICODE( "CanSetPaperSize" );
    aPrinter.getArray()[7].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_PAPERSIZE ) );

    aPrinter.getArray()[6].Name = DEFINE_CONST_UNICODE( "CanSetPaperFormat" );
    aPrinter.getArray()[6].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_PAPER ) );

    aPrinter.getArray()[5].Name = DEFINE_CONST_UNICODE( "CanSetPaperOrientation" );
    aPrinter.getArray()[5].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_ORIENTATION ) );

    aPrinter.getArray()[4].Name = DEFINE_CONST_UNICODE( "IsBusy" );
    aPrinter.getArray()[4].Value <<= ( pPrinter->IsPrinting() );

    aPrinter.getArray()[3].Name = DEFINE_CONST_UNICODE( "PaperSize" );
    SIZE aSize = impl_Size_Object2Struct(pPrinter->GetPaperSize() );
    aPrinter.getArray()[3].Value <<= aSize;

    aPrinter.getArray()[2].Name = DEFINE_CONST_UNICODE( "PaperFormat" );
    PAPERFORMAT eFormat = (PAPERFORMAT)pPrinter->GetPaper();
    aPrinter.getArray()[2].Value <<= eFormat;

    aPrinter.getArray()[1].Name = DEFINE_CONST_UNICODE( "PaperOrientation" );
    PAPERORIENTATION eOrient = (PAPERORIENTATION)pPrinter->GetOrientation();
    aPrinter.getArray()[1].Value <<= eOrient;

    aPrinter.getArray()[0].Name = DEFINE_CONST_UNICODE( "Name" );
    String sStringTemp = pPrinter->GetName() ;
    aPrinter.getArray()[0].Value <<= ::rtl::OUString( sStringTemp );

    return aPrinter;
}

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________

void SfxBaseModel::impl_setPrinter(const SEQUENCE< PROPERTYVALUE >& rPrinter,SfxPrinter*& pPrinter,sal_uInt16& nChangeFlags,SfxViewShell*& pViewSh)

{
    // alten Printer beschaffen
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.Is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell, 0, sal_False ) : 0;
    if ( !pViewFrm )
        return;
    pViewSh = pViewFrm->GetViewShell();
    pPrinter = pViewSh->GetPrinter(sal_True);
    if ( !pPrinter )
        return;

    // new Printer-Name available?
    nChangeFlags = 0;
    sal_Int32 lDummy;
    for ( int n = 0; n < rPrinter.getLength(); ++n )
    {
        // get Property-Value from printer description
        const PROPERTYVALUE &rProp = rPrinter.getConstArray()[n];

        // Name-Property?
        if ( rProp.Name.compareToAscii( "Name" ) == 0 )
        {
            OUSTRING sTemp;
            if ( ( rProp.Value >>= sTemp ) == sal_False )
                throw ILLEGALARGUMENTEXCEPTION();

            String aPrinterName( sTemp ) ;
            pPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(), aPrinterName );
            nChangeFlags = SFX_PRINTER_PRINTER;
            break;
        }
    }

    Size aSetPaperSize( 0, 0);
    PAPERFORMAT nPaperFormat = (PAPERFORMAT) PAPER_USER;
    // other properties
    for ( int i = 0; i < rPrinter.getLength(); ++i )
    {
        // get Property-Value from printer description
        const PROPERTYVALUE &rProp = rPrinter.getConstArray()[i];

        // PaperOrientation-Property?
        if ( rProp.Name.compareToAscii( "PaperOrientation" ) == 0 )
        {
            PAPERORIENTATION eOrient;
            if ( ( rProp.Value >>= eOrient ) == sal_False )
            {
                if ( ( rProp.Value >>= lDummy ) == sal_False )
                    throw ILLEGALARGUMENTEXCEPTION();
                eOrient = ( PAPERORIENTATION ) lDummy;
            }

            pPrinter->SetOrientation( (Orientation) eOrient );
            nChangeFlags |= SFX_PRINTER_CHG_ORIENTATION;
        }

        // PaperFormat-Property?
        if ( rProp.Name.compareToAscii( "PaperFormat" ) == 0 )
        {
            if ( ( rProp.Value >>= nPaperFormat ) == sal_False )
            {
                if ( ( rProp.Value >>= lDummy ) == sal_False )
                    throw ILLEGALARGUMENTEXCEPTION();
                nPaperFormat = ( PAPERFORMAT ) lDummy;
            }

            pPrinter->SetPaper( (Paper) nPaperFormat );
            nChangeFlags |= SFX_PRINTER_CHG_SIZE;
        }

        // PaperSize-Property?
        if ( rProp.Name.compareToAscii( "PaperSize" ) == 0 )
        {
            SIZE aTempSize ;
            if ( ( rProp.Value >>= aTempSize ) == sal_False )
            {
                throw ILLEGALARGUMENTEXCEPTION();
            }
            else
            {
                aSetPaperSize = impl_Size_Struct2Object(aTempSize);
            }
        }
    }

    //os 12.11.98: die PaperSize darf nur gesetzt werden, wenn tatsaechlich
    //PAPER_USER gilt, sonst koennte vom Treiber ein falsches Format gewaehlt werden
    if(nPaperFormat == PAPER_USER && aSetPaperSize.Width())
    {
        //JP 23.09.98 - Bug 56929 - MapMode von 100mm in die am
        //          Device gesetzten umrechnen. Zusaetzlich nur dann
        //          setzen, wenn sie wirklich veraendert wurden.
        aSetPaperSize = pPrinter->LogicToPixel( aSetPaperSize, MAP_100TH_MM );
        if( aSetPaperSize != pPrinter->GetPaperSizePixel() )
        {
            pPrinter->SetPaperSizeUser( pPrinter->PixelToLogic( aSetPaperSize ) );
            nChangeFlags |= SFX_PRINTER_CHG_SIZE;
        }
    }

    // #96772#: wait until printing is done
    SfxPrinter* pDocPrinter = pViewSh->GetPrinter();
    while ( pDocPrinter->IsPrinting() )
        Application::Yield();
}

void SAL_CALL SfxBaseModel::setPrinter(const SEQUENCE< PROPERTYVALUE >& rPrinter)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    SfxViewShell* pViewSh = NULL;
    SfxPrinter* pPrinter = NULL;
    sal_uInt16 nChangeFlags = 0;
    impl_setPrinter(rPrinter,pPrinter,nChangeFlags,pViewSh);
    // set new printer
    if ( pViewSh && pPrinter )
        pViewSh->SetPrinter( pPrinter, nChangeFlags );
}

//________________________________________________________________________________________________________
//  ImplPrintWatch thread for asynchronous printing with moving temp. file to ucb location
//________________________________________________________________________________________________________

/* This implements a thread which will be started to wait for asynchronous
   print jobs to temp. localy files. If they finish we move the temp. files
   to her right locations by using the ucb.
 */
class ImplUCBPrintWatcher : public ::osl::Thread
{
    private:
        /// of course we must know the printer which execute the job
        SfxPrinter* m_pPrinter;
        /// this describes the target location for the printed temp file
        String m_sTargetURL;
        /// it holds the temp file alive, till the print job will finish and remove it from disk automaticly if the object die
        ::utl::TempFile* m_pTempFile;

    public:
        /* initialize this watcher but don't start it */
        ImplUCBPrintWatcher( SfxPrinter* pPrinter, ::utl::TempFile* pTempFile, const String& sTargetURL )
                : m_pPrinter  ( pPrinter   )
                , m_sTargetURL( sTargetURL )
                , m_pTempFile ( pTempFile  )
        {}

        /* waits for finishing of the print job and moves the temp file afterwards
           Note: Starting of the job is done outside this thread!
           But we have to free some of the given ressources on heap!
         */
        void SAL_CALL run()
        {
            /* SAFE { */
            {
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                while( m_pPrinter->IsPrinting() )
                    Application::Yield();
                m_pPrinter = NULL; // don't delete it! It's borrowed only :-)
            }
            /* } SAFE */

            // lock for further using of our member isn't neccessary - because
            // we truns alone by defenition. Nobody join for us nor use us ...
            ImplUCBPrintWatcher::moveAndDeleteTemp(&m_pTempFile,m_sTargetURL);

            // finishing of this run() method will call onTerminate() automaticly
            // kill this thread there!
        }

        /* nobody wait for this thread. We must kill ourself ...
         */
        void SAL_CALL onTerminated()
        {
            delete this;
        }

        /* static helper to move the temp. file to the target location by using the ucb
           It's static to be useable from outside too. So it's not realy neccessary to start
           the thread, if finishing of the job was detected outside this thread.
           But it must be called without using a corresponding thread for the given parameter!
         */
        static void moveAndDeleteTemp( ::utl::TempFile** ppTempFile, const String& sTargetURL )
        {
            // move the file
            try
            {
                INetURLObject aSplitter(sTargetURL);
                String        sFileName = aSplitter.getName(
                                            INetURLObject::LAST_SEGMENT,
                                            true,
                                            INetURLObject::DECODE_WITH_CHARSET);
                if (aSplitter.removeSegment() && sFileName.Len()>0)
                {
                    ::ucb::Content aSource(
                            ::rtl::OUString((*ppTempFile)->GetURL()),
                            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >());

                    ::ucb::Content aTarget(
                            ::rtl::OUString(aSplitter.GetMainURL(INetURLObject::NO_DECODE)),
                            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >());

                    aTarget.transferContent(
                            aSource,
                            ::ucb::InsertOperation_COPY,
                            ::rtl::OUString(sFileName),
                            ::com::sun::star::ucb::NameClash::OVERWRITE);
                }
            }
            catch( ::com::sun::star::ucb::ContentCreationException& ) { DBG_ERROR("content create exception"); }
            catch( ::com::sun::star::ucb::CommandAbortedException&  ) { DBG_ERROR("command abort exception"); }
            catch( ::com::sun::star::uno::RuntimeException&         ) { DBG_ERROR("runtime exception"); }
            catch( ::com::sun::star::uno::Exception&                ) { DBG_ERROR("unknown exception"); }

            // kill the temp file!
            delete *ppTempFile;
            *ppTempFile = NULL;
        }
};

//------------------------------------------------

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________
void SAL_CALL SfxBaseModel::print(const SEQUENCE< PROPERTYVALUE >& rOptions)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // get view for sfx printing capabilities
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.Is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell, 0, sal_False ) : 0;
    if ( !pViewFrm )
        return;
    SfxViewShell* pView = pViewFrm->GetViewShell();
    if ( !pView )
        return;

    SfxAllItemSet aArgs( pView->GetPool() );
    sal_Bool bMonitor = sal_False;
    // We need this information at the end of this method, if we start the vcl printer
    // by executing the slot. Because if it is a ucb relevant URL we must wait for
    // finishing the print job and move the temporary local file by using the ucb
    // to the right location. But in case of no file name is given or it is already
    // a local one we can supress this special handling. Because then vcl makes all
    // right for us.
    String sUcbUrl;
    ::utl::TempFile* pUCBPrintTempFile = NULL;

    sal_Bool bWaitUntilEnd = sal_False;
    for ( int n = 0; n < rOptions.getLength(); ++n )
    {
        // get Property-Value from options
        const PROPERTYVALUE &rProp = rOptions.getConstArray()[n];

        // FileName-Property?
        if ( rProp.Name.compareToAscii( "FileName" ) == 0 )
        {
            // unpack th URL and check for a valid and well known protocol
            OUSTRING sTemp;
            if (
                ( rProp.Value.getValueType()!=::getCppuType((const OUSTRING*)0))  ||
                (!(rProp.Value>>=sTemp))
               )
            {
                throw ILLEGALARGUMENTEXCEPTION();
            }

            String        sPath        ;
            String        sURL  (sTemp);
            INetURLObject aCheck(sURL );
            if (aCheck.GetProtocol()==INET_PROT_NOT_VALID)
            {
                // OK - it's not a valid URL. But may it's a simple
                // system path directly. It will be supported for historical
                // reasons. Otherwhise we break to much external code ...
                // We try to convert it to a file URL. If its possible
                // we put the system path to the item set and let vcl work with it.
                // No ucb or thread will be neccessary then. In case it couldnt be
                // converted its not an URL nor a system path. Then we can't accept
                // this parameter and have to throw an exception.
                ::rtl::OUString sSystemPath(sTemp);
                ::rtl::OUString sFileURL          ;
                if (::osl::FileBase::getFileURLFromSystemPath(sSystemPath,sFileURL)!=::osl::FileBase::E_None)
                    throw ILLEGALARGUMENTEXCEPTION();
                aArgs.Put( SfxStringItem(SID_FILE_NAME,sTemp) );
            }
            else
            // It's a valid URL. but now we must know, if it is a local one or not.
            // It's a question of using ucb or not!
            if (::utl::LocalFileHelper::ConvertURLToSystemPath(sURL,sPath))
            {
                // it's a local file, we can use vcl without special handling
                // And we have to use the system notation of the incoming URL.
                // But it into the descriptor and let the slot be executed at
                // the end of this method.
                aArgs.Put( SfxStringItem(SID_FILE_NAME,sPath) );
            }
            else
            {
                // it's an ucb target. So we must use a temp. file for vcl
                // and move it after printing by using the ucb.
                // Create a temp file on the heap (because it must delete the
                // real file on disk automaticly if it die - bt we have to share it with
                // some other sources ... e.g. the ImplUCBPrintWatcher).
                // And we put the name of this temp file to the descriptor instead
                // of the URL. The URL we save for later using seperatly.
                // Execution of the print job will be done later by executing
                // a slot ...
                pUCBPrintTempFile = new ::utl::TempFile();
                pUCBPrintTempFile->EnableKillingFile();
                aArgs.Put( SfxStringItem(SID_FILE_NAME,pUCBPrintTempFile->GetFileName()) );
                sUcbUrl = sURL;
            }
        }

        // CopyCount-Property
        else if ( rProp.Name.compareToAscii( "CopyCount" ) == 0 )
        {
            sal_Int32 nCopies = 0;
            if ( ( rProp.Value >>= nCopies ) == sal_False )
                throw ILLEGALARGUMENTEXCEPTION();
            aArgs.Put( SfxInt16Item( SID_PRINT_COPIES, (USHORT) nCopies ) );
        }

        // Collate-Property
        else if ( rProp.Name.compareToAscii( "Collate" ) == 0 )
        {
            sal_Bool bTemp ;
            if ( rProp.Value >>= bTemp )
                aArgs.Put( SfxBoolItem( SID_PRINT_COLLATE, bTemp ) );
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Sort-Property
        else if ( rProp.Name.compareToAscii( "Sort" ) == 0 )
        {
            sal_Bool bTemp ;
            if( rProp.Value >>= bTemp )
                aArgs.Put( SfxBoolItem( SID_PRINT_SORT, bTemp ) );
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Pages-Property
        else if ( rProp.Name.compareToAscii( "Pages" ) == 0 )
        {
            OUSTRING sTemp;
            if( rProp.Value >>= sTemp )
                aArgs.Put( SfxStringItem( SID_PRINT_PAGES, String( sTemp ) ) );
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // MonitorVisible
        else if ( rProp.Name.compareToAscii( "MonitorVisible" ) == 0 )
        {
            if( !(rProp.Value >>= bMonitor) )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // MonitorVisible
        else if ( rProp.Name.compareToAscii( "Wait" ) == 0 )
        {
            if ( !(rProp.Value >>= bWaitUntilEnd) )
                throw ILLEGALARGUMENTEXCEPTION();
        }
    }

    // Execute the print request every time.
    // It doesn'tmatter if it is a real printer used or we print to a local file
    // nor if we print to a temp file and move it afterwards by using the ucb.
    // That will be handled later. see pUCBPrintFile below!
    aArgs.Put( SfxBoolItem( SID_SILENT, !bMonitor ) );
    if ( bWaitUntilEnd )
        aArgs.Put( SfxBoolItem( SID_ASYNCHRON, sal_False ) );
    SfxRequest aReq( SID_PRINTDOC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_API, pView->GetPool() );
    aReq.SetArgs( aArgs );
    pView->ExecuteSlot( aReq );

    // Ok - may be execution before has finished (or started!) printing.
    // And may it was a printing to a file.
    // Now we have to check if we can move the file (if neccessary) via ucb to his right location.
    // Cases:
    //  a) printing finished                        => move the file directly and forget the watcher thread
    //  b) printing is asynchron and runs currently => start watcher thread and exit this method
    //                                                 This thread make all neccessary things by itself.
    if (pUCBPrintTempFile!=NULL)
    {
        // a)
        SfxPrinter* pPrinter = pView->GetPrinter();
        if ( ! pPrinter->IsPrinting() )
            ImplUCBPrintWatcher::moveAndDeleteTemp(&pUCBPrintTempFile,sUcbUrl);
        // b)
        else
        {
            // Note: we create(d) some ressource on the heap. (thread and tep file)
            // They will be delected by the thread automaticly if he finish his run() method.
            ImplUCBPrintWatcher* pWatcher = new ImplUCBPrintWatcher( pPrinter, pUCBPrintTempFile, sUcbUrl );
            pWatcher->create();
        }
    }
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::hasLocation() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->HasName() : sal_False;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

OUSTRING SAL_CALL SfxBaseModel::getLocation() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell.Is() ? OUSTRING(m_pData->m_pObjectShell->GetMedium()->GetName()) : m_pData->m_sURL;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::isReadonly() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsReadOnly() : sal_True;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
    {
        if ( m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            if ( m_pData->m_pObjectShell->DoSave() )
                m_pData->m_pObjectShell->DoSaveCompleted();
        }
        else
            m_pData->m_pObjectShell->Save_Impl();
    }
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::storeAsURL( const   OUSTRING&                   rURL    ,
                                        const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
    {
        impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_False );

        SEQUENCE< PROPERTYVALUE > aSequence ;
        TransformItems( SID_OPENDOC, *m_pData->m_pObjectShell->GetMedium()->GetItemSet(), aSequence );
        attachResource( rURL, aSequence );
    }
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::storeToURL( const   OUSTRING&                   rURL    ,
                                        const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
    {
        impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_True );
    }
}

//________________________________________________________________________________________________________
// XLoadable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::initNew()
        throw (::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );
    if ( m_pData->m_pObjectShell.Is() )
    {
        if( m_pData->m_pObjectShell->GetMedium() )
            throw DOUBLEINITIALIZATIONEXCEPTION();

        sal_Bool bRes = m_pData->m_pObjectShell->DoInitNew( NULL );
        sal_uInt32 nErrCode = m_pData->m_pObjectShell->GetError() ?
                                    m_pData->m_pObjectShell->GetError() : ERRCODE_IO_CANTCREATE;
        m_pData->m_pObjectShell->ResetError();

        if ( !bRes )
        {
            throw SfxIOException_Impl( nErrCode );
        }
    }
}

//________________________________________________________________________________________________________
// XLoadable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::load(   const SEQUENCE< PROPERTYVALUE >& seqArguments )
        throw (::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );

    if ( m_pData->m_pObjectShell.Is() )
    {
        if( m_pData->m_pObjectShell->GetMedium() )
            throw DOUBLEINITIALIZATIONEXCEPTION();

        SfxAllItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
        TransformParameters( SID_OPENDOC, seqArguments, *pParams );

        rtl::OUString aFilterName;
        SFX_ITEMSET_ARG( pParams, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if( pFilterNameItem )
            aFilterName = pFilterNameItem->GetValue();

        if( !aFilterName.getLength() )
            throw ILLEGALARGUMENTIOEXCEPTION();

        pParams->Put( SfxBoolItem( SID_VIEW, sal_False ) );
        pParams->Put( SfxObjectShellItem( SID_OBJECTSHELL, m_pData->m_pObjectShell ) );

           // create LoadEnvironment and set link for callback when it is finished
           m_pData->m_pLoader = LoadEnvironment_Impl::Create( *pParams, TRUE );
           m_pData->m_pLoader->AddRef();
           m_pData->m_pLoader->SetDoneLink( LINK( this, SfxBaseModel, LoadDone_Impl ) );

        m_pData->m_bLoadDone = sal_False;
        m_pData->m_pLoader->Start();

           // wait for callback
           while( m_pData->m_bLoadDone == sal_False )
               Application::Yield();

        m_pData->m_pLoader->ReleaseRef();
        m_pData->m_pLoader = NULL;
        DELETEZ( pParams );

        sal_uInt32 nErrCode = m_pData->m_pObjectShell->GetError() ?
                                m_pData->m_pObjectShell->GetError() : ERRCODE_IO_CANTREAD;
        m_pData->m_pObjectShell->ResetError();
/*
        // remove lock without closing (it is set in the LoadEnvironment, because the document
        // is loaded in a hidden mode)
        m_pData->m_pObjectShell->RemoveOwnerLock();
*/
        if ( !m_pData->m_bLoadState )
        {
            throw SfxIOException_Impl( nErrCode );
        }
    }
}

IMPL_LINK( SfxBaseModel, LoadDone_Impl, void*, pVoid )
{
    DBG_ASSERT( m_pData->m_pLoader, "No Loader created, but LoadDone ?!" );

    if ( m_pData->m_pLoader->GetError() )
    {
        m_pData->m_bLoadDone  = sal_True ;
        m_pData->m_bLoadState = sal_False;
    }
    else
    {
        m_pData->m_bLoadDone  = sal_True;
        m_pData->m_bLoadState = sal_True;
    }

    return NULL;
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________

ANY SAL_CALL SfxBaseModel::getTransferData( const DATAFLAVOR& aFlavor )
        throw (::com::sun::star::datatransfer::UnsupportedFlavorException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    ANY aAny;

    if ( m_pData->m_pObjectShell.Is() )
    {
        if ( aFlavor.MimeType.equalsAscii( "application/x-openoffice;windows_formatname=\"GDIMetaFile\"" ) )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                GDIMetaFile* pMetaFile = m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    SvMemoryStream aMemStm( 65535, 65535 );

                    pMetaFile->Write( aMemStm );
                    delete pMetaFile;
                    aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ),
                                                    aMemStm.Seek( STREAM_SEEK_TO_END ) );
                }
            }
            else
                throw UNSUPPORTEDFLAVOREXCEPTION();
        }
        else if ( aFlavor.MimeType.equalsAscii( "application/x-openoffice;windows_formatname=\"Image EMF\"" ) )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                GDIMetaFile* pMetaFile = m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    SvMemoryStream* pStream = getMetaMemStrFromGDI_Impl( pMetaFile, CVT_EMF );
                    delete pMetaFile;
                    if ( pStream )
                    {
                        aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pStream->GetData() ),
                                                        pStream->Seek( STREAM_SEEK_TO_END ) );
                        delete pStream;
                    }
                }
            }
            else if ( supportsMetaFileHandle_Impl()
              && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            {
                GDIMetaFile* pMetaFile = m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    aAny <<= reinterpret_cast< const sal_uInt64 >( getEnhMetaFileFromGDI_Impl( pMetaFile ) );
                    delete pMetaFile;
                }
            }
            else
                throw UNSUPPORTEDFLAVOREXCEPTION();
        }
        else if ( aFlavor.MimeType.equalsAscii( "application/x-openoffice;windows_formatname=\"Image WMF\"" ) )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                GDIMetaFile* pMetaFile = m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    SvMemoryStream* pStream = getMetaMemStrFromGDI_Impl( pMetaFile, CVT_WMF );
                    delete pMetaFile;

                    if ( pStream )
                    {
                        aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pStream->GetData() ),
                                                        pStream->Seek( STREAM_SEEK_TO_END ) );
                        delete pStream;
                    }
                }
            }
            else if ( supportsMetaFileHandle_Impl()
              && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            {
                // means HGLOBAL handler to memory storage containing METAFILEPICT structure

                GDIMetaFile* pMetaFile = m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    Size aMetaSize = pMetaFile->GetPrefSize();
                    aAny <<= reinterpret_cast< const sal_uInt64 >( getWinMetaFileFromGDI_Impl( pMetaFile, aMetaSize ) );

                    delete pMetaFile;
                }
            }
            else
                throw UNSUPPORTEDFLAVOREXCEPTION();
        }
        else
            throw UNSUPPORTEDFLAVOREXCEPTION();
    }

    return aAny;
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________


SEQUENCE< DATAFLAVOR > SAL_CALL SfxBaseModel::getTransferDataFlavors()
        throw (::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    sal_Int32 nSuppFlavors = supportsMetaFileHandle_Impl() ? 5 : 3;
    SEQUENCE< DATAFLAVOR > aFlavorSeq( nSuppFlavors );

    aFlavorSeq[0].MimeType =
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice;windows_formatname=\"GDIMetaFile\"" ) );
    aFlavorSeq[0].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GDIMetaFile" ) );
    aFlavorSeq[0].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[1].MimeType =
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice;windows_formatname=\"Image EMF\"" ) );
    aFlavorSeq[1].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enhanced Windows MetaFile" ) );
    aFlavorSeq[1].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[2].MimeType =
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice;windows_formatname=\"Image WMF\"" ) );
    aFlavorSeq[2].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Windows MetaFile" ) );
    aFlavorSeq[2].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    if ( nSuppFlavors == 5 )
    {
        aFlavorSeq[3].MimeType =
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice;windows_formatname=\"Image EMF\"" ) );
        aFlavorSeq[3].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enhanced Windows MetaFile" ) );
        aFlavorSeq[3].DataType = getCppuType( (const sal_uInt64*) 0 );

        aFlavorSeq[4].MimeType =
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice;windows_formatname=\"Image WMF\"" ) );
        aFlavorSeq[4].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Windows MetaFile" ) );
        aFlavorSeq[4].DataType = getCppuType( (const sal_uInt64*) 0 );
    }

    return aFlavorSeq;
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________


sal_Bool SAL_CALL SfxBaseModel::isDataFlavorSupported( const DATAFLAVOR& aFlavor )
        throw (::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( aFlavor.MimeType.equalsAscii( "application/x-openoffice;windows_formatname=\"GDIMetaFile\"" ) )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType.equalsAscii( "application/x-openoffice;windows_formatname=\"Image EMF\"" ) )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
        else if ( supportsMetaFileHandle_Impl()
          && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType.equalsAscii( "application/x-openoffice;windows_formatname=\"Image WMF\"" ) )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
        else if ( supportsMetaFileHandle_Impl()
          && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            return sal_True;
    }

    return sal_False;
}


//--------------------------------------------------------------------------------------------------------
//  XEventsSupplier
//--------------------------------------------------------------------------------------------------------

REFERENCE< XNAMEREPLACE > SAL_CALL SfxBaseModel::getEvents() throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( ! m_pData->m_xEvents.is() )
    {
        m_pData->m_xEvents = new SfxEvents_Impl( m_pData->m_pObjectShell, this );
    }

    return m_pData->m_xEvents;
}

//--------------------------------------------------------------------------------------------------------
//  XEventBroadcaster
//--------------------------------------------------------------------------------------------------------

void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XDOCEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), aListener );
}

//--------------------------------------------------------------------------------------------------------
//  XEventBroadcaster
//--------------------------------------------------------------------------------------------------------

void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XDOCEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  SfxListener
//________________________________________________________________________________________________________

void addTitle_Impl( Sequence < ::com::sun::star::beans::PropertyValue >& rSeq, const ::rtl::OUString& rTitle )
{
    sal_Int32 nCount = rSeq.getLength();
    for ( sal_Int32 nArg=0; nArg<nCount; nArg++ )
    {
        ::com::sun::star::beans::PropertyValue& rProp = rSeq[nArg];
        if ( rProp.Name.equalsAscii("Title") )
        {
            rProp.Value <<= rTitle;
            break;
        }
    }

    if ( nArg == nCount )
    {
        rSeq.realloc( nCount+1 );
        rSeq[nCount].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Title") );
        rSeq[nCount].Value <<= rTitle;
    }
}

void SfxBaseModel::Notify(          SfxBroadcaster& rBC     ,
                             const  SfxHint&        rHint   )
{
    if ( !m_pData )
        return;

    if ( &rBC == m_pData->m_pObjectShell )
    {
        SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
        if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DOCCHANGED )
            changing();

        SfxEventHint* pNamedHint = PTR_CAST( SfxEventHint, &rHint );
        if ( pNamedHint )
        {
            if ( SFX_EVENT_SAVEASDOCDONE == pNamedHint->GetEventId() )
            {
                m_pData->m_sURL = m_pData->m_pObjectShell->GetMedium()->GetName();
                SfxItemSet *pSet = m_pData->m_pObjectShell->GetMedium()->GetItemSet();
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
                ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
                TransformItems( SID_SAVEASDOC, *pSet, aArgs );
                addTitle_Impl( aArgs, aTitle );
                attachResource( m_pData->m_pObjectShell->GetMedium()->GetName(), aArgs );
            }

            postEvent_Impl( *pNamedHint );
        }

        if ( pSimpleHint )
        {
            if ( pSimpleHint->GetId() == SFX_HINT_TITLECHANGED )
            {
                ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
                addTitle_Impl( m_pData->m_seqArguments, aTitle );
            }
/*
            else if ( pSimpleHint->GetId() == SFX_HINT_DYING
                || pSimpleHint->GetId() == SFX_HINT_DEINITIALIZING )
            {
                SfxObjectShellLock pShellLock = m_pData->m_pObjectShellLock;
                m_pData->m_pObjectShellLock = SfxObjectShellLock();
            }
*/
        }

        SfxPrintingHint* pPrintHint = PTR_CAST( SfxPrintingHint, &rHint );
        if ( pPrintHint )
        {
            if ( pPrintHint->GetWhich() == -1 )
            {
                if ( !m_pData->m_xPrintJob.is() )
                    m_pData->m_xPrintJob = new SfxPrintJob_Impl( m_pData );

                PrintDialog* pDlg = pPrintHint->GetPrintDialog();
                Printer* pPrinter = pPrintHint->GetPrinter();
                ::rtl::OUString aPrintFile ( ( pPrinter && pPrinter->IsPrintFileEnabled() ) ? pPrinter->GetPrintFile() : String() );
                ::rtl::OUString aRangeText ( ( pDlg && pDlg->IsRangeChecked(PRINTDIALOG_RANGE) ) ? pDlg->GetRangeText() : String() );
                sal_Bool bSelectionOnly = ( ( pDlg && pDlg->IsRangeChecked(PRINTDIALOG_SELECTION) ) ? sal_True : sal_False );

                sal_Int32 nArgs = 2;
                if ( aPrintFile.getLength() )
                    nArgs++;
                if ( aRangeText.getLength() )
                    nArgs++;
                else if ( bSelectionOnly )
                    nArgs++;

                m_pData->m_aPrintOptions.realloc(nArgs);
                m_pData->m_aPrintOptions[0].Name = DEFINE_CONST_UNICODE("CopyCount");
                m_pData->m_aPrintOptions[0].Value <<= (sal_Int16) (pPrinter ? pPrinter->GetCopyCount() : 1 );
                m_pData->m_aPrintOptions[1].Name = DEFINE_CONST_UNICODE("Collate");
                m_pData->m_aPrintOptions[1].Value <<= (sal_Bool) (pDlg ? pDlg->IsCollateChecked() : sal_False );

                if ( bSelectionOnly )
                {
                    m_pData->m_aPrintOptions[2].Name = DEFINE_CONST_UNICODE("Selection");
                    m_pData->m_aPrintOptions[2].Value <<= bSelectionOnly;
                }
                else if ( aRangeText.getLength() )
                {
                    m_pData->m_aPrintOptions[2].Name = DEFINE_CONST_UNICODE("Pages");
                    m_pData->m_aPrintOptions[2].Value <<= aRangeText;
                }

                if ( aPrintFile.getLength() )
                {
                    m_pData->m_aPrintOptions[nArgs-1].Name = DEFINE_CONST_UNICODE("FileName");
                    m_pData->m_aPrintOptions[nArgs-1].Value <<= aPrintFile;
                }
            }
            else if ( pPrintHint->GetWhich() == -3 )
            {
                    sal_Int32 nOld = m_pData->m_aPrintOptions.getLength();
                    sal_Int32 nAdd = pPrintHint->GetAdditionalOptions().getLength();
                    m_pData->m_aPrintOptions.realloc(  nOld + nAdd );
                    for ( sal_Int32 n=0; n<nAdd; n++ )
                        m_pData->m_aPrintOptions[ nOld+n ] = pPrintHint->GetAdditionalOptions()[n];
            }
            else if ( pPrintHint->GetWhich() != -2 )
            {
                view::PrintJobEvent aEvent;
                aEvent.Source = m_pData->m_xPrintJob;
                aEvent.State = (com::sun::star::view::PrintableState) pPrintHint->GetWhich();
                ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< view::XPrintJobListener >*) NULL ) );
                if ( pContainer!=NULL )
                {
                    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
                    while (pIterator.hasMoreElements())
                        ((view::XPrintJobListener*)pIterator.next())->printJobEvent( aEvent );
                }
            }
        }
    }
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

void SfxBaseModel::changing()
{
    // object already disposed?
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0) );
    if( pIC )

    {
        EVENTOBJECT aEvent( (XMODEL *)this );
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XMODIFYLISTENER *)aIt.next())->modified( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

void SfxBaseModel::impl_change()
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0) );
    if( pIC )

    {
        EVENTOBJECT aEvent( (XMODEL *)this );
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XMODIFYLISTENER *)aIt.next())->modified( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

SfxObjectShell* SfxBaseModel::GetObjectShell() const
{
    return m_pData ? (SfxObjectShell*) m_pData->m_pObjectShell : 0;
}

SfxObjectShell* SfxBaseModel::impl_getObjectShell() const
{
    return m_pData ? (SfxObjectShell*) m_pData->m_pObjectShell : 0;
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

sal_Bool SfxBaseModel::IsDisposed() const
{
    return ( m_pData == NULL ) ;
}

sal_Bool SfxBaseModel::impl_isDisposed() const
{
    return ( m_pData == NULL ) ;
}

//________________________________________________________________________________________________________
//  private impl.
//________________________________________________________________________________________________________

void SfxBaseModel::impl_store(          SfxObjectShell*             pObjectShell    ,
                                const   OUSTRING&                   sURL            ,
                                const   SEQUENCE< PROPERTYVALUE >&  seqArguments    ,
                                        sal_Bool                    bSaveTo         )
{
    if( !sURL.getLength() )
        throw ILLEGALARGUMENTIOEXCEPTION();

    //sal_Bool aSaveAsTemplate = sal_False;

    SfxAllItemSet *aParams = new SfxAllItemSet( SFX_APP()->GetPool() );
    aParams->Put( SfxStringItem( SID_FILE_NAME, String(sURL) ) );
    if ( bSaveTo )
        aParams->Put( SfxBoolItem( SID_SAVETO, sal_True ) );

    TransformParameters( SID_SAVEASDOC, seqArguments, *aParams );
    sal_Bool aRet = pObjectShell->APISaveAs_Impl( sURL, aParams );
    DELETEZ( aParams );

    sal_uInt32 nErrCode = pObjectShell->GetError() ? pObjectShell->GetError() : ERRCODE_IO_CANTWRITE;
    pObjectShell->ResetError();

    if ( !aRet )
    {
        throw SfxIOException_Impl( nErrCode );
    }
}

//********************************************************************************************************

void SfxBaseModel::postEvent_Impl( const SfxEventHint& rHint )
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer(
                                        ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) );
    if( pIC )

    {
        OUSTRING aName = SfxEventConfiguration::GetEventName_Impl( rHint.GetEventId() );
        DOCEVENTOBJECT aEvent( (XMODEL *)this, aName );
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

REFERENCE < XINDEXACCESS > SAL_CALL SfxBaseModel::getViewData() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() && !m_pData->m_contViewData.is() )
    {
        SfxViewFrame *pActFrame = SfxViewFrame::Current();
        if ( !pActFrame || pActFrame->GetObjectShell() != m_pData->m_pObjectShell )
            pActFrame = SfxViewFrame::GetFirst(m_pData->m_pObjectShell, TYPE(SfxTopViewFrame));

        if ( !pActFrame )
            // currently no frame for this document at all
            return REFERENCE < XINDEXACCESS >();

        m_pData->m_contViewData = Reference < XINDEXACCESS >(
                ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.document.IndexedPropertyValues") ),
                UNO_QUERY );

        if ( !m_pData->m_contViewData.is() )
        {
            // error: no container class available!
            return REFERENCE < XINDEXACCESS >();
        }

        REFERENCE < XINDEXCONTAINER > xCont( m_pData->m_contViewData, UNO_QUERY );
        sal_Int32 nCount = 0;
        SEQUENCE < PROPERTYVALUE > aSeq;
        ::com::sun::star::uno::Any aAny;
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(m_pData->m_pObjectShell, TYPE(SfxTopViewFrame) ); pFrame;
                pFrame = SfxViewFrame::GetNext(*pFrame, m_pData->m_pObjectShell, TYPE(SfxTopViewFrame) ) )
        {
            BOOL bIsActive = ( pFrame == pActFrame );
            pFrame->GetViewShell()->WriteUserDataSequence( aSeq );
            aAny <<= aSeq;
            xCont->insertByIndex( bIsActive ? 0 : nCount, aAny );
            nCount++;
        }
    }

    return m_pData->m_contViewData;
}

void SAL_CALL SfxBaseModel::setViewData( const REFERENCE < XINDEXACCESS >& aData ) throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_contViewData = aData;
}

/** calls all XEventListeners */
void SfxBaseModel::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) const
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer(
                                        ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) );
    if( pIC )

    {
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

/** returns true if someone added a XEventListener to this XEventBroadcaster */
sal_Bool SfxBaseModel::hasEventListeners() const
{
    return !impl_isDisposed() && (NULL != m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) ) );
}

void SAL_CALL SfxBaseModel::addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XPRINTJOBLISTENER >*)0), xListener );
}

void SAL_CALL SfxBaseModel::removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XPRINTJOBLISTENER >*)0), xListener );
}

