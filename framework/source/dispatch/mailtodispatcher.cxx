/*************************************************************************
 *
 *  $RCSfile: mailtodispatcher.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-07-02 13:26:39 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_MAILTODISPATCHER_HXX_
#include <dispatch/mailtodispatcher.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ARGUMENTANALYZER_HXX_
#include <classes/argumentanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_XSIMPLEMAILCLIENTSUPPLIER_HPP_
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SIMPLEMAILCLIENTFLAGS_HPP_
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vos/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/svapp.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::com::sun::star::awt           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::mozilla       ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::util          ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::vos                           ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define MAILTO                                  "mailto:"

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
MailToDispatcher::MailToDispatcher(   const   Reference< XMultiServiceFactory >&  xFactory    ,
                                        const   Reference< XFrame >&                xOwner      )
        //  Init baseclasses first
        :   ThreadHelpBase          ( &Application::GetSolarMutex()  )
        ,   OWeakObject             (                                )
        // Init member
        ,   m_xOwnerWeak            ( xOwner                         )
        ,   m_xFactory              ( xFactory                       )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex() )
        ,   m_bAlreadyDisposed      ( sal_False                      )
{
    // Safe impossible cases
    // We need valid informations about ouer ownerfor work.
    LOG_ASSERT( impldbg_checkParameter_MailToDispatcher( xFactory, xOwner ), "MailToDispatcher::MailToDispatcher()\nInvalid parameter detected!\n" )
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
MailToDispatcher::~MailToDispatcher()
{
    // Warn programmer if he forgot to dispose this instance.
    // We must release all our references ...
    // and a dtor isn't the best place to do that!
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   MailToDispatcher                   ,
                            OWeakObject                         ,
                            DIRECT_INTERFACE(   XTypeProvider   ),
                            DIRECT_INTERFACE(   XDispatch       ),
                            DIRECT_INTERFACE(   XEventListener  )
                        )

DEFINE_XTYPEPROVIDER_3  (   MailToDispatcher       ,
                            XTypeProvider       ,
                            XDispatch           ,
                            XEventListener
                        )


//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MailToDispatcher::dispatch(  const   URL&                        aURL            ,
                                            const   Sequence< PropertyValue >&  seqProperties   ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter
    LOG_ASSERT( impldbg_checkParameter_dispatch( aURL, seqProperties ), "MailToDispatcher::dispatch()\nInvalid parameter detected.\n" )

    Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
    LOG_ASSERT( !(xFrame.is()==sal_False), "MailToDispatcher::dispatch()\nDispatch failed ... can't get reference to owner!\n" )
    if( xFrame.is() == sal_True )
    {
        // mailto URL
        OUString aURLProtocol( aURL.Protocol );
        if ( aURLProtocol.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MAILTO )) )
        {
            Reference< ::com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                                                                            m_xFactory->createInstance(
                                                                        ::rtl::OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" )), UNO_QUERY );
            if ( xSystemShellExecute.is() )
            {
                try
                {
                    // start mail client
                    ::rtl::OUString aURLString( aURL.Complete );
                    xSystemShellExecute->execute( aURLString, ::rtl::OUString(), ::com::sun::star::system::SystemShellExecuteFlags::DEFAULTS );
                }
                catch ( ::com::sun::star::lang::IllegalArgumentException& )
                {
                }
                catch ( ::com::sun::star::system::SystemShellExecuteException& )
                {
                }
            }
        }
    }
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MailToDispatcher::addStatusListener( const   Reference< XStatusListener >&   xControl,
                                                    const   URL&                            aURL    ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter
    LOG_ASSERT( impldbg_checkParameter_addStatusListener( xControl, aURL ), "OMenuDispatcher::addStatusListener()\nInvalid parameter detected.\n" )
    // Add listener to container.
    m_aListenerContainer.addInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MailToDispatcher::removeStatusListener(  const   Reference< XStatusListener >&   xControl,
                                                        const   URL&                            aURL    ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter
    LOG_ASSERT( impldbg_checkParameter_removeStatusListener( xControl, aURL ), "OMenuDispatcher::removeStatusListener()\nInvalid parameter detected.\n" )
    // Add listener to container.
    m_aListenerContainer.removeInterface( aURL.Complete, xControl );
}


//*****************************************************************************************************************
//   XEventListener
//*****************************************************************************************************************
void SAL_CALL MailToDispatcher::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    LOG_ASSERT( !(m_bAlreadyDisposed==sal_True), "MailToDispatcher::disposing()\nObject already disposed .. don't call it again!\n" )

    if( m_bAlreadyDisposed == sal_False )
    {
        m_bAlreadyDisposed = sal_True;

        // Forget mail client supplier
        m_xSimpleMailClientSupplier = 0;

        // Forget our factory.
        m_xFactory = Reference< XMultiServiceFactory >();
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void MailToDispatcher::impl_sendStatusEvent(   const   Reference< XFrame >&    xEventSource    ,
                                                const   OUString&               sURL            ,
                                                sal_Bool                        bLoadState      )
{
    // Get listener for given URL!
    OInterfaceContainerHelper* pListenerForURL = m_aListenerContainer.getContainer( sURL );
    // Send messages to all listener.
    // Do nothing, if there no listener or "getContainer()" works not correct!
    if( pListenerForURL != NULL )
    {
        // Build event for send to listener.
        FeatureStateEvent aEvent;
        aEvent.FeatureURL.Complete  =   sURL                        ;
        aEvent.FeatureDescriptor    =   FEATUREDESCRIPTOR_LOADSTATE ;
        aEvent.IsEnabled            =   bLoadState                  ;
        aEvent.Requery              =   sal_False                   ;
        aEvent.State              <<=   xEventSource                ;

        // Send message to all listener on this URL.
        OInterfaceIteratorHelper aIterator(*pListenerForURL);
        while( aIterator.hasMoreElements() )
        {
            ((XStatusListener*)aIterator.next())->statusChanged( aEvent );
        }
    }
}


void MailToDispatcher::impl_getSequenceFromStringList( css::uno::Sequence< ::rtl::OUString >& aStringSeq, const ::rtl::OUString& aStringList )
{
    sal_Int32 nParamIndex = 0;
    sal_Int32 nPos = 0;
    do
    {
        OUString aName = aStringList.getToken( 0, ',', nParamIndex );
        aStringSeq.realloc( aStringSeq.getLength() + 1 );
        aName = INetURLObject::decode( aName, '%', INetURLObject::DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8);
        aStringSeq[nPos++] = aName;
    }
    while ( nParamIndex >= 0 );
}


//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
sal_Bool MailToDispatcher::impldbg_checkParameter_MailToDispatcher(   const   Reference< XMultiServiceFactory >&  xFactory    ,
                                                                        const   Reference< XFrame >&                xOwner      )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFactory     ==  NULL        )   ||
            ( &xOwner       ==  NULL        )   ||
            ( xFactory.is() ==  sal_False   )   ||
            ( xOwner.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We don't know anything about right values of aURL and seqArguments!
// Check valid references only.
sal_Bool MailToDispatcher::impldbg_checkParameter_dispatch(    const   URL&                        aURL        ,
                                                                const   Sequence< PropertyValue >&  seqArguments)
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL         ==  NULL    )   ||
            ( &seqArguments ==  NULL    )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We need a valid URL. What is meaning with "register for nothing"?!
// xControl must correct to - nobody can advised otherwise!
sal_Bool MailToDispatcher::impldbg_checkParameter_addStatusListener(   const   Reference< XStatusListener >&   xControl,
                                                                        const   URL&                            aURL    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xControl                 ==  NULL    )   ||
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.getLength() <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// The same goes for these case! We have added valid listener for correct URL only.
// We can't remove invalid listener for nothing!
sal_Bool MailToDispatcher::impldbg_checkParameter_removeStatusListener(    const   Reference< XStatusListener >&   xControl,
                                                                            const   URL&                            aURL    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xControl                 ==  NULL    )   ||
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.getLength() <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
