/*************************************************************************
 *
 *  $RCSfile: sfxbasemodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-19 17:04:52 $
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

#ifndef _COM_SUN_STAR_VIEW_PAPERFORMAT_HPP_
#include <com/sun/star/view/PaperFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

//ASDBG #ifndef _USR_SMARTCONV_HXX_
//ASDBG #include <usr/smartconv.hxx>
//ASDBG #endif

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

#include <vos/mutex.hxx>

#include "sfxsids.hrc"

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
#define OINTERFACECONTAINERHELPER               ::cppu::OInterfaceContainerHelper
#define OINTERFACEITERATORHELPER                ::cppu::OInterfaceIteratorHelper
#define SIZE                                    ::com::sun::star::awt::Size
#define PAPERFORMAT                             ::com::sun::star::view::PaperFormat
#define PAPERORIENTATION                        ::com::sun::star::view::PaperOrientation
#define OTYPECOLLECTION                         ::cppu::OTypeCollection
#define OIMPLEMENTATIONID                       ::cppu::OImplementationId
#define MUTEXGUARD                              ::osl::MutexGuard

//________________________________________________________________________________________________________
//  namespaces
//________________________________________________________________________________________________________

//using namespace ::osl                             ;
//using namespace ::rtl                             ;
//using namespace ::cppu                            ;
//using namespace ::com::sun::star::uno             ;
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
    SfxObjectShell*                                 m_pObjectShell          ;
    OUSTRING                                        m_sURL                  ;
    sal_uInt16                                      m_nControllerLockCount  ;
    OMULTITYPEINTERFACECONTAINERHELPER              m_aInterfaceContainer   ;
    REFERENCE< XINTERFACE >                         m_xParent               ;
    REFERENCE< XCONTROLLER >                        m_xCurrent              ;
    REFERENCE< XDOCUMENTINFO >                      m_xDocumentInfo         ;
    SEQUENCE< PROPERTYVALUE>                        m_seqArguments          ;
    SEQUENCE< REFERENCE< XCONTROLLER > >            m_seqControllers        ;

    IMPL_SfxBaseModel_DataContainer::IMPL_SfxBaseModel_DataContainer(   MUTEX&          aMutex          ,
                                                                        SfxObjectShell* pObjectShell    )
            :   m_pObjectShell          ( pObjectShell  )
            ,   m_sURL                  ( String()      )
            ,   m_nControllerLockCount  ( 0             )
            ,   m_aInterfaceContainer   ( aMutex        )
    {
    }

} ;

/* ASDBG
//________________________________________________________________________________________________________
//  methods for bridging smart<->uno3
//  Please don't use! Will be deleted in ... days :-)
//________________________________________________________________________________________________________

REFERENCE< XMODEL > impl_Smart2Uno3_XModel( XModel* pSmart )
{
    MAPPING aConverter( "smart" , CPPU_CURRENT_LANGUAGE_BINDING_NAME ) ;

    XMODEL*             pUno3   =   SAL_STATIC_CAST(XMODEL*,(aConverter.mapInterface( pSmart, ::getCppuType((const REFERENCE< XMODEL >*)0) )))  ;
    REFERENCE< XMODEL > xUno3   =   pUno3                                                                                                       ;

    xUno3->release() ;

    return xUno3 ;
}

REFERENCE< XCONTROLLER > impl_Smart2Uno3_XController( XController* pSmart )
{
    MAPPING aConverter( "smart" , CPPU_CURRENT_LANGUAGE_BINDING_NAME ) ;

    XCONTROLLER*                pUno3   =   SAL_STATIC_CAST(XCONTROLLER*,(aConverter.mapInterface( pSmart, ::getCppuType((const REFERENCE< XCONTROLLER >*)0) )))    ;
    REFERENCE< XCONTROLLER >    xUno3   =   pUno3                                                                                                                   ;

    xUno3->release() ;

    return xUno3 ;
}

REFERENCE< XDOCUMENTINFO > impl_Smart2Uno3_XDocumentInfo( XDocumentInfo* pSmart )
{
    MAPPING aConverter( "smart" , CPPU_CURRENT_LANGUAGE_BINDING_NAME ) ;

    XDOCUMENTINFO*              pUno3   =   SAL_STATIC_CAST(XDOCUMENTINFO*,(aConverter.mapInterface( pSmart, ::getCppuType((const REFERENCE< XDOCUMENTINFO >*)0) )))    ;
    REFERENCE< XDOCUMENTINFO >  xUno3   =   pUno3                                                                                                                       ;

    xUno3->release() ;

    return xUno3 ;
}

SEQUENCE< PROPERTYVALUE > impl_Smart2Uno3_seqPropertyValue( const Sequence< PropertyValue >& seqSmart )
{
    sal_Int32               nCount          =   seqSmart.getLen()           ;
    const PropertyValue*    pSmartProperty  =   seqSmart.getConstArray()    ;
    sal_Int32               nPosition       =   0                           ;

    SEQUENCE< PROPERTYVALUE > seqUno3 ( nCount ) ;
    PROPERTYVALUE* pUno3Property = seqUno3.getArray() ;

    for ( nPosition=0; nPosition<nCount; nPosition++ )
    {
        pUno3Property[nPosition].Name   =   pSmartProperty[nPosition].Name      ;
        pUno3Property[nPosition].Handle =   pSmartProperty[nPosition].Handle    ;
        switch ( pSmartProperty[nPosition].State )
        {
            case PropertyState_DIRECT_VALUE     :   pUno3Property[nPosition].State  =   ::com::sun::star::beans::PropertyState_DIRECT_VALUE     ;
                                                    break;
            case PropertyState_DEFAULT_VALUE    :   pUno3Property[nPosition].State  =   ::com::sun::star::beans::PropertyState_DEFAULT_VALUE    ;
                                                    break;
            case PropertyState_AMBIGUOUS_VALUE  :   pUno3Property[nPosition].State  =   ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE  ;
                                                    break;
            default: DBG_ASSERT ( sal_False, "SfxBaseModel::impl_Smart2Uno3_sePropertyValue()\nPropertyState unknown!\n" ) ;
        }
        ::usr::convertUsr2UnoAny( pUno3Property[nPosition].Value, pSmartProperty[nPosition].Value ) ;
    }

    return seqUno3 ;
}
*/
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

//________________________________________________________________________________________________________
//  constructor
//________________________________________________________________________________________________________

SfxBaseModel::SfxBaseModel( SfxObjectShell *pObjectShell )  :   IMPL_SfxBaseModel_MutexContainer    (                                                               )
                                                            ,   m_pData                             ( new IMPL_SfxBaseModel_DataContainer( m_aMutex, pObjectShell ) )
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
    delete m_pData ;
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
                                            static_cast< XMODIFYBROADCASTER*    > ( this )  ,
                                            static_cast< XCOMPONENT*            > ( this )  ,
                                               static_cast< XPRINTABLE*         > ( this )  ,
                                               static_cast< XSTORABLE*              > ( this )  ) ) ;

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

void SAL_CALL SfxBaseModel::acquire() throw( RUNTIMEEXCEPTION )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire() ;
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::release() throw( RUNTIMEEXCEPTION )
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
            static OTYPECOLLECTION aTypeCollection( ::getCppuType(( const REFERENCE< XTYPEPROVIDER          >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XCHILD                   >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XDOCUMENTINFOSUPPLIER    >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XEVENTLISTENER           >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XMODEL                   >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XMODIFIABLE          >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XPRINTABLE               >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XSTORABLE                >*)NULL ) ) ;

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
    return m_pData->m_xParent;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setParent(const REFERENCE< XINTERFACE >& Parent) throw(NOSUPPORTEXCEPTION, RUNTIMEEXCEPTION)
{
    if ( Parent.is() && getParent().is() )
        // only set parent when no parent is available
        throw NOSUPPORTEXCEPTION();

    m_pData->m_xParent = Parent;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

long SfxObjectShellClose_Impl( void* pObj, void* pArg );

void SAL_CALL SfxBaseModel::dispose()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    EVENTOBJECT aEvent( (XMODEL *)this );
    m_pData->m_aInterfaceContainer.disposeAndClear( aEvent );

    // is an object shell assigned?
    if ( m_pData->m_pObjectShell )
    {
        // Rekursion vermeiden
        SfxObjectShell *pShell;
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            pShell = m_pData->m_pObjectShell;
            EndListening( *pShell );
            m_pData->m_pObjectShell = NULL;
        }

        // Bei dispose keine Speichern-R"uckfrage
        if ( pShell->IsEnableSetModified() && !pShell->Get_Impl()->bClosing )
            pShell->SetModified( sal_False );
        SfxObjectShellClose_Impl( 0, (void*) pShell );
    }

    ::osl::MutexGuard aGuard( m_aMutex );
    m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
//ASDBG for ( sal_uInt32 n = m_pData->m_seqControllers.getLength(); n; --n )
//ASDBG     SEQUENCERemoveElementAt( m_pData->m_seqControllers, n-1 );
    m_pData->m_seqControllers = SEQUENCE< REFERENCE< XCONTROLLER > > () ;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  XDOCUMENTINFOSupplier
//________________________________________________________________________________________________________

REFERENCE< XDOCUMENTINFO > SAL_CALL SfxBaseModel::getDocumentInfo()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pData->m_xDocumentInfo.is() && m_pData->m_pObjectShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        ((SfxBaseModel*)this)->m_pData->m_xDocumentInfo = new SfxDocumentInfoObject( m_pData->m_pObjectShell ) ;
    }

    return m_pData->m_xDocumentInfo;
}

//________________________________________________________________________________________________________
//  XEVENTLISTENER
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::disposing( const EVENTOBJECT& aObject )
{
    REFERENCE< XMODIFYLISTENER >  xMod( aObject.Source, UNO_QUERY );
    REFERENCE< XEVENTLISTENER >  xListener( aObject.Source, UNO_QUERY );
    if ( xMod.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xMod );
    else if ( xListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), xListener );

    ::osl::MutexGuard aGuard( m_aMutex );
    sal_uInt32 nCount = m_pData->m_seqControllers.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        if( m_pData->m_seqControllers.getConstArray()[n] == aObject.Source )
        {
            m_pData->m_seqControllers.getArray()[n] = REFERENCE< XCONTROLLER > () ;
            break;
        }
//ASDBG     if( m_pData->m_seqControllers.getConstArray()[n] == aObject.Source )
//ASDBG     {
//ASDBG         SequenceRemoveElementAt( m_pData->m_seqControllers, n );
//ASDBG         break;
//ASDBG     }
    }

    if ( m_pData->m_xCurrent.is() && m_pData->m_xCurrent == aObject.Source )
        m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::attachResource( const   OUSTRING&                   rURL    ,
                                                const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pData->m_sURL         =   rURL    ;
    m_pData->m_seqArguments =   rArgs   ;
    return sal_True ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

OUSTRING SAL_CALL SfxBaseModel::getURL()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pData->m_sURL ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

SEQUENCE< PROPERTYVALUE > SAL_CALL SfxBaseModel::getArgs()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pData->m_seqArguments ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::connectController( const REFERENCE< XCONTROLLER >& xController )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    SEQUENCE<REFERENCE< XCONTROLLER > > aNewSeq( nOldCount + 1 );
    for ( sal_uInt32 n = 0; n < nOldCount; n++ )
        aNewSeq.getArray()[n] = m_pData->m_seqControllers.getConstArray()[n];
    aNewSeq.getArray()[nOldCount] = xController;
    m_pData->m_seqControllers = aNewSeq;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::disconnectController( const REFERENCE< XCONTROLLER >& xController )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    SEQUENCE<REFERENCE< XCONTROLLER > > aNewSeq( nOldCount - 1 );
    for ( sal_uInt32 nOld = 0, nNew = 0; nOld < nOldCount; ++nOld )
        if ( xController != m_pData->m_seqControllers.getConstArray()[nOld] )
        {
            aNewSeq.getArray()[nNew] = m_pData->m_seqControllers.getConstArray()[nOld];
            ++nNew;
        }
    m_pData->m_seqControllers = aNewSeq;

    if ( xController == m_pData->m_xCurrent )
        m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::lockControllers()
{
    ++m_pData->m_nControllerLockCount ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::unlockControllers()
{
    --m_pData->m_nControllerLockCount ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::hasControllersLocked()
{
    return ( m_pData->m_nControllerLockCount != 0 ) ;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

REFERENCE< XCONTROLLER > SAL_CALL SfxBaseModel::getCurrentController()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // get the last active controller of this model
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pData->m_xCurrent.is() )
        return m_pData->m_xCurrent;

    // get the first controller of this model
    return m_pData->m_seqControllers.getLength() ? m_pData->m_seqControllers.getConstArray()[0] : m_pData->m_xCurrent;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setCurrentController( const REFERENCE< XCONTROLLER >& xCurrentController )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    ::osl::MutexGuard aGuard( m_aMutex );
    m_pData->m_xCurrent = xCurrentController;
}

//________________________________________________________________________________________________________
//  XMODEL
//________________________________________________________________________________________________________

REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getCurrentSelection()
{
    // object already disposed?
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

sal_Bool SAL_CALL SfxBaseModel::isModified()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pData->m_pObjectShell ? m_pData->m_pObjectShell->IsModified() : sal_False;
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setModified( sal_Bool bModified )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        m_pData->m_pObjectShell->SetModified(bModified);
    }
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw(RUNTIMEEXCEPTION )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0),xListener );
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw(RUNTIMEEXCEPTION )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xListener );
}

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________

SEQUENCE< PROPERTYVALUE > SAL_CALL SfxBaseModel::getPrinter()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // Printer beschaffen
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell ? SfxViewFrame::GetFirst( m_pData->m_pObjectShell, 0, sal_False ) : 0;
    if ( !pViewFrm )
        return SEQUENCE<PROPERTYVALUE>();
    const SfxPrinter *pPrinter = pViewFrm->GetViewShell()->GetPrinter(sal_True);
    if ( !pPrinter )
        return SEQUENCE<PROPERTYVALUE>();

    // Printer Eigenschaften uebertragen
    SEQUENCE<PROPERTYVALUE> aPrinter(8);

    aPrinter.getArray()[7].Name = DEFINE_CONST_UNICODE( "CanSetPaperSize" );
//ASDBG aPrinter.getArray()[7].Value.setBOOL( pPrinter->HasSupport( SUPPORT_SET_PAPERSIZE ) );
    aPrinter.getArray()[7].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_PAPERSIZE ) );

    aPrinter.getArray()[6].Name = DEFINE_CONST_UNICODE( "CanSetPaperFormat" );
//ASDBG aPrinter.getArray()[6].Value.setBOOL( pPrinter->HasSupport( SUPPORT_SET_PAPER ) );
    aPrinter.getArray()[6].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_PAPER ) );

    aPrinter.getArray()[5].Name = DEFINE_CONST_UNICODE( "CanSetPaperOrientation" );
//ASDBG aPrinter.getArray()[5].Value.setBOOL( pPrinter->HasSupport( SUPPORT_SET_ORIENTATION ) );
    aPrinter.getArray()[5].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_ORIENTATION ) );

    aPrinter.getArray()[4].Name = DEFINE_CONST_UNICODE( "IsBusy" );
//ASDBG aPrinter.getArray()[4].Value.setBOOL( pPrinter->IsJobActive() );
    aPrinter.getArray()[4].Value <<= ( pPrinter->IsJobActive() );

    aPrinter.getArray()[3].Name = DEFINE_CONST_UNICODE( "PaperSize" );
    SIZE aSize = impl_Size_Object2Struct(pPrinter->GetPaperSize() );
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//  aPrinter.getArray()[3].Value.set( &aSize, Size_getReflection() );
    aPrinter.getArray()[3].Value <<= aSize;

    aPrinter.getArray()[2].Name = DEFINE_CONST_UNICODE( "PaperFormat" );
    PAPERFORMAT eFormat = (PAPERFORMAT)pPrinter->GetPaper();
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//  aPrinter.getArray()[2].Value.set( &eFormat, PaperFormat_getReflection() );
    aPrinter.getArray()[2].Value <<= eFormat;

    aPrinter.getArray()[1].Name = DEFINE_CONST_UNICODE( "PaperOrientation" );
    PAPERORIENTATION eOrient = (PAPERORIENTATION)pPrinter->GetOrientation();
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//  aPrinter.getArray()[1].Value.set( &eOrient, PaperOrientation_getReflection() );
    aPrinter.getArray()[1].Value <<= eOrient;

    aPrinter.getArray()[0].Name = DEFINE_CONST_UNICODE( "Name" );
//  aPrinter.getArray()[0].Value <<= ( S2U(pPrinter->GetName()) );
    String sStringTemp = pPrinter->GetName() ;
    aPrinter.getArray()[0].Value <<= ::rtl::OUString( sStringTemp );

    return aPrinter;
}

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setPrinter(const SEQUENCE< PROPERTYVALUE >& rPrinter)
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // alten Printer beschaffen
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell ? SfxViewFrame::GetFirst( m_pData->m_pObjectShell, 0, sal_False ) : 0;
    if ( !pViewFrm )
        return;
    SfxViewShell *pViewSh = pViewFrm->GetViewShell();
    SfxPrinter *pPrinter = pViewSh->GetPrinter(sal_True);
    if ( !pPrinter )
        return;

    // new Printer-Name available?
    sal_uInt16 nChangeFlags = 0;
    for ( int n = 0; n < rPrinter.getLength(); ++n )
    {
        // get Property-Value from printer description
        const PROPERTYVALUE &rProp = rPrinter.getConstArray()[n];

        // Name-Property?
        if ( rProp.Name.compareToAscii( "Name" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() != OOUSTRING_getReflection() )
            if ( rProp.Value.getValueType() != ::getCppuType((const OUSTRING*)0) )

                throw ILLEGALARGUMENTEXCEPTION();

//          String aPrinterName( U2S( rProp.Value.getString() ) );
            OUSTRING sTemp;
            rProp.Value >>= sTemp ;
            String aPrinterName( sTemp ) ;

            pPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(),
                                       aPrinterName );
            nChangeFlags = SFX_PRINTER_PRINTER;
            break;
        }
    }

    Size aSetPaperSize( 0, 0);
    sal_Int32 nPaperFormat = PAPER_USER;
    // other properties
    for ( int i = 0; i < rPrinter.getLength(); ++i )
    {
        // get Property-Value from printer description
        const PROPERTYVALUE &rProp = rPrinter.getConstArray()[i];
//      IMPL_SmartAny aValue( rProp.Value );

        // PaperOrientation-Property?
        if ( rProp.Name.compareToAscii( "PaperOrientation" ) == 0 )
        {
            sal_Int32 nOrient;
//          if ( !aValue.queryEnum( nOrient ) )
            if ( ( rProp.Value >>= nOrient ) == sal_False )
                throw ILLEGALARGUMENTEXCEPTION();

            pPrinter->SetOrientation( (Orientation) nOrient );
            nChangeFlags |= SFX_PRINTER_CHG_ORIENTATION;
        }

        // PaperFormat-Property?
        if ( rProp.Name.compareToAscii( "PaperFormat" ) == 0 )
        {
//          if ( !aValue.queryEnum( nPaperFormat ) )
            if ( ( rProp.Value >>= nPaperFormat ) == sal_False )
                throw ILLEGALARGUMENTEXCEPTION();

            pPrinter->SetPaper( (Paper) nPaperFormat );
            nChangeFlags |= SFX_PRINTER_CHG_SIZE;
        }

        // PaperSize-Property?
        if ( rProp.Name.compareToAscii( "PaperSize" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() != Size_getReflection() )
//          if ( rProp.Value.getValueType() != ::getCppuType((const Size*)0) )
//              throw( ILLEGALARGUMENTEXCEPTION() );
//
//          aSetPaperSize = ( *(const Size*) rProp.Value.get() );
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
    if(nPaperFormat == PAPER_USER &&
        aSetPaperSize.Width())
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

    // set new printer
    pViewSh->SetPrinter( pPrinter, nChangeFlags );
}

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::print(const SEQUENCE< PROPERTYVALUE >& rOptions)
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // get view for sfx printing capabilities
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell ? SfxViewFrame::GetFirst( m_pData->m_pObjectShell, 0, sal_False ) : 0;
    if ( !pViewFrm )
        return;
    SfxViewShell* pView = pViewFrm->GetViewShell();
    if ( !pView )
        return;

    SfxAllItemSet aArgs( pView->GetPool() );
    sal_Bool bMonitor = sal_False;

    sal_Bool bWaitUntilEnd = sal_False;
    for ( int n = 0; n < rOptions.getLength(); ++n )
    {
        // get Property-Value from options
        const PROPERTYVALUE &rProp = rOptions.getConstArray()[n];
//      IMPL_SmartAny aValue( rProp.Value );

        // FileName-Property?
        if ( rProp.Name.compareToAscii( "FileName" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() == OOUSTRING_getReflection() )
            if ( rProp.Value.getValueType() == ::getCppuType((const OUSTRING*)0) )
            {
//              aArgs.Put( SfxStringItem( SID_FILE_NAME, U2S( rProp.Value.getString() ) ) );
                OUSTRING sTemp;
                rProp.Value >>= sTemp;
                aArgs.Put( SfxStringItem( SID_FILE_NAME, String( sTemp ) ) );
            }
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          else if ( rProp.Value.getReflection() != Void_getReflection() )
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // CopyCount-Property
        else if ( rProp.Name.compareToAscii( "CopyCount" ) == 0 )
        {
            sal_Int32 nCopies = 0;
//          if ( !aValue.queryINT32( nCopies ) )
            if ( ( rProp.Value >>= nCopies ) == sal_False )
                throw ILLEGALARGUMENTEXCEPTION();
            aArgs.Put( SfxInt16Item( SID_PRINT_COPIES, nCopies ) );
        }

        // Collate-Property
        else if ( rProp.Name.compareToAscii( "Collate" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() == BOOL_getReflection() )
            if ( rProp.Value.getValueType() == ::getCppuBooleanType() )
            {
                sal_Bool bTemp ;
                rProp.Value >>= bTemp ;
                aArgs.Put( SfxBoolItem( SID_PRINT_COLLATE, bTemp ) );
            }
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Sort-Property
        else if ( rProp.Name.compareToAscii( "Sort" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() == BOOL_getReflection() )
            if ( rProp.Value.getValueType() == ::getCppuBooleanType() )
            {
                sal_Bool bTemp ;
                rProp.Value >>= bTemp ;
                aArgs.Put( SfxBoolItem( SID_PRINT_SORT, bTemp ) );
            }
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Pages-Property
        else if ( rProp.Name.compareToAscii( "Pages" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() == OOUSTRING_getReflection() )
            if ( rProp.Value.getValueType() == ::getCppuType((const OUSTRING*)0) )
            {
                OUSTRING sTemp;
                rProp.Value >>= sTemp;
                aArgs.Put( SfxStringItem( SID_PRINT_PAGES, String( sTemp ) ) );
            }
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // MonitorVisible
        else if ( rProp.Name.compareToAscii( "MonitorVisible" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() == BOOL_getReflection() )
            if ( rProp.Value.getValueType() == ::getCppuBooleanType() )
                rProp.Value >>= bMonitor ;
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // MonitorVisible
        else if ( rProp.Name.compareToAscii( "Wait" ) == 0 )
        {
// automatisch auskommentiert - Wird von UNO III nicht weiter unterstützt!
//          if ( rProp.Value.getReflection() == BOOL_getReflection() )
            if ( rProp.Value.getValueType() == ::getCppuBooleanType() )
                rProp.Value >>= bWaitUntilEnd ;
            else
                throw ILLEGALARGUMENTEXCEPTION();
        }
    }

    aArgs.Put( SfxBoolItem( SID_SILENT, !bMonitor ) );
    if ( bWaitUntilEnd )
        aArgs.Put( SfxBoolItem( SID_ASYNCHRON, sal_False ) );
    SfxRequest aReq( SID_PRINTDOC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_API, pView->GetPool() );
    aReq.SetArgs( aArgs );
    pView->ExecuteSlot( aReq );
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::hasLocation()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell ? m_pData->m_pObjectShell->HasName() : sal_False;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

OUSTRING SAL_CALL SfxBaseModel::getLocation()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell ? OUSTRING(m_pData->m_pObjectShell->GetMedium()->GetName()) : m_pData->m_sURL;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::isReadonly()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell ? m_pData->m_pObjectShell->IsReadOnly() : sal_True;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::store()
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        m_pData->m_pObjectShell->Save_Impl();
    }
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::storeAsURL( const   OUSTRING&                   rURL    ,
                                        const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
{
    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_True );
    }
}

//________________________________________________________________________________________________________
//  SfxListener
//________________________________________________________________________________________________________

void SfxBaseModel::Notify(          SfxBroadcaster& rBC     ,
                             const  SfxHint&        rHint   )
{
    if ( &rBC == m_pData->m_pObjectShell )
    {
        SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
        if ( pHint && pHint->GetId() == SFX_HINT_DOCCHANGED )
            changing();
    }
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

void SfxBaseModel::changing()
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
            ((XMODIFYLISTENER *)aIt.next())->modified( aEvent );
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
            ((XMODIFYLISTENER *)aIt.next())->modified( aEvent );
    }
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

SfxObjectShell* SfxBaseModel::GetObjectShell() const
{
    return m_pData ? m_pData->m_pObjectShell : 0;
}

SfxObjectShell* SfxBaseModel::impl_getObjectShell() const
{
    return m_pData ? m_pData->m_pObjectShell : 0;
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
                                const   SEQUENCE<PROPERTYVALUE>&    seqArguments    ,
                                        sal_Bool                    bSaveTo         )
{
    SfxRequest aReq( SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, pObjectShell->GetPool() );
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, String(sURL) ) );
    if ( bSaveTo )
        aReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );

    // Parameter auswerten
    for ( int n = 0; n < seqArguments.getLength(); ++n )
    {
        // get Property-Value from args
        const PROPERTYVALUE &rProp = seqArguments.getConstArray()[n];

        // FilterName-Property?
        if ( rProp.Name.compareToAscii( "FilterName" ) == 0 )
        {
            OUSTRING sTemp;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // FilterFlags-Property?
        else if ( rProp.Name.compareToAscii( "FilterOptions" ) == 0 )
        {
            OUSTRING sTemp;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_FILE_FILTEROPTIONS, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Version-Property?
        else if ( rProp.Name.compareToAscii( "Version" ) == 0 )
        {
            OUSTRING sTemp ;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_VERSION, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Author-Property?
        else if ( rProp.Name.compareToAscii( "Author" ) == 0 )
        {
            OUSTRING sTemp ;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_DOCINFO_AUTHOR, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Password-Property?
        else if ( rProp.Name.compareToAscii( "Password" ) == 0 )
        {
            OUSTRING sTemp ;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_PASSWORD, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Overwrite-Property?
        else if ( rProp.Name.compareToAscii( "Overwrite" ) == 0 )
        {
            sal_Bool bTemp ;
            if ( ( rProp.Value >>= bTemp ) == sal_True )
                aReq.AppendItem( SfxBoolItem( SID_OVERWRITE, bTemp ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // TemplateRegion-Property?
        else if ( rProp.Name.compareToAscii( "TemplateRegion" ) == 0 )
        {
            aReq.SetSlot( SID_DOCTEMPLATE );
            OUSTRING sTemp ;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_TEMPLATE_REGIONNAME, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        // Template-Property?
        else if ( rProp.Name.compareToAscii( "TemplateName" ) == 0 )
        {
            aReq.SetSlot( SID_DOCTEMPLATE );
            OUSTRING sTemp ;
            if ( ( rProp.Value >>= sTemp ) == sal_True )
                aReq.AppendItem( SfxStringItem( SID_TEMPLATE_NAME, String( sTemp ) ) );
            else if ( rProp.Value.getValueType() != ::getCppuVoidType() )
                throw ILLEGALARGUMENTEXCEPTION();
        }

        else
        {
            throw ::com::sun::star::beans::UnknownPropertyException(
                    rProp.Name, *this );
        }
    }

    const SfxBoolItem *pRet = (const SfxBoolItem*) pObjectShell->ExecuteSlot( aReq );
    if ( !pRet || !pRet->GetValue() )
        throw SfxIOException_Impl( ERRCODE_IO_CANTWRITE );
}
