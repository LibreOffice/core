/*************************************************************************
 *
 *  $RCSfile: dispatchprovider.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: as $ $Date: 2001-08-16 09:45:23 $
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

#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#include <dispatch/dispatchprovider.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_BLANKDISPATCHER_HXX_
#include <dispatch/blankdispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_CREATEDISPATCHER_HXX_
#include <dispatch/createdispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_SELFDISPATCHER_HXX_
#include <dispatch/selfdispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_MENUDISPATCHER_HXX_
#include <dispatch/menudispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_MAILTODISPATCHER_HXX_
#include <dispatch/mailtodispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_
#include <dispatch/helpagentdispatcher.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#include <classes/targetfinder.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#include <threadhelp/transactionguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
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

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   DispatchProvider                                    ,
                            OWeakObject                                         ,
                            DIRECT_INTERFACE(css::lang::XTypeProvider          ),
                            DIRECT_INTERFACE(css::frame::XDispatchProvider     ),
                            DIRECT_INTERFACE(css::lang::XEventListener         )
                        )

DEFINE_XTYPEPROVIDER_3  (   DispatchProvider                                   ,
                            css::lang::XTypeProvider                           ,
                            css::frame::XDispatchProvider                      ,
                            css::lang::XEventListener
                        )

/*-************************************************************************************************************//**
    @short      standard ctor/dtor
    @descr      These initialize a new instance of tihs class with needed informations for work.
                We hold a weakreference to our owner frame which start dispatches at us.
                We can't use a normal reference because he hold a reference of us too ...
                nobody can die so ...!

    @seealso    using at owner

    @param      "xFactory", reference to servicemanager to create new services.
    @param      "xFrame"  , reference to our owner frame.
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
DispatchProvider::DispatchProvider( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory  ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame    )
        //  Init baseclasses first
        :   ThreadHelpBase          ( &Application::GetSolarMutex()  )
        ,   TransactionBase         (                                )
        ,   OWeakObject             (                                )
        // Init member
        ,   m_xFrame                ( xFrame                         )
        ,   m_xFactory              ( xFactory                       )
        // Don't initialize dispatch helper ...
        // because we create it on demand!
{
    // Safe impossible cases
    // We need valid informations about ouer owner for working.
    LOG_ASSERT2( implcp_ctor( xFactory, xFrame ), "DispatchProvider::DispatchProvider()", "Invalid parameter detected!\n" )

    // Open object for working!
    m_aTransactionManager.setWorkingMode( E_WORK );
}

//*****************************************************************************************************************
DispatchProvider::~DispatchProvider()
{
    // A dtor isn't a good place to make anything ...
    // Do it in dispose/ing()!
    // We are listener on our owner frame ...
}

/*-************************************************************************************************************//**
    @interface  XDispatchProvider
    @short      search a dispatcher for given URL
    @descr      If no interceptor is set on owner, we search for right frame and dispatch URL to it.
                If no frame was found, we do nothing.

    @seealso    -

    @param      "aURL"              , URL to dispatch.
    @param      "sTargetFrameName"  , name of searched frame.
    @param      "nSearchFlags"      , flags for searching.
    @return     A reference a dispatcher for these URL (if someone was found!).

    @onerror    We return a NULL-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XDispatch > SAL_CALL DispatchProvider::queryDispatch( const css::util::URL&  aURL             ,
                                                                                       const ::rtl::OUString& sTargetFrameName ,
                                                                                             sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter ...
    LOG_ASSERT2( implcp_queryDispatch( aURL, sTargetFrameName, nSearchFlags ), "DispatchProvider::queryDispatch()", "Invalid Parameter detected!" )
    // Register operation as transaction and reject wrong calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Set default return value if query failed.
    css::uno::Reference< css::frame::XDispatch > xReturn;

    /* HACK */
    ::rtl::OUString sNewTarget = sTargetFrameName;
    if( aURL.Complete.compareToAscii( "macro:", 6 ) == 0 )
    {
        sNewTarget = DECLARE_ASCII("_self");
    }
    /* HACK */

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Make snapshot of neccessary member by using a read lock.
    // Resolve weakreference to owner and hold a normal reference till the end of this method.
    // So he couldn't die .-)
    css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );

    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xOwner.is() == sal_True )
    {
        // Classify target of this dispatch call to find right dispatch helper!
        // Try to get neccessary informations from tree environment.
        TargetInfo   aInfo  ( xOwner, sNewTarget, nSearchFlags );
        ETargetClass eResult= TargetFinder::classifyQueryDispatch( aInfo );
        switch( eResult )
        {
            //-----------------------------------------------------------------------------------------------------
            case E_SELF          :  {
                                        // Ask our controller for his agreement for these dispatched URL ...
                                        // because some URLs are internal and can be handled faster by SFX - which most is the current controller!
                                        // Attention: Desktop has no controller - don't ask him!
                                        if( aInfo.eFrameType != E_DESKTOP )
                                        {
                                            css::uno::Reference< css::frame::XDispatchProvider > xController( xOwner->getController(), css::uno::UNO_QUERY );
                                            if( xController.is() == sal_True )
                                            {
                                                xReturn = xController->queryDispatch( aURL, sNewTarget, nSearchFlags );
                                            }
                                        }
                                        // If controller has no fun to dispatch these URL - we must search another right dispatcher.
                                        if( xReturn.is() == sal_False )
                                        {
                                            xReturn = implts_searchProtocolHandler( aURL, aInfo );
                                        }
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            case E_CREATETASK    :  {
                                        // Check ucb support before you create dispatch helper.
                                        // He could do nothing then ... but it doesnt perform, if we try it!
                                        if( implts_isLoadableContent( aURL ) == sal_True )
                                        {
                                            xReturn = implts_getOrCreateDispatchHelper( E_BLANKDISPATCHER );
                                        }
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            case E_PARENT        :
            case E_FORWARD_UP    :  {
                                        css::uno::Reference< css::frame::XDispatchProvider > xParent( xOwner->getCreator(), css::uno::UNO_QUERY );
                                        if( xParent.is() == sal_True )
                                        {
                                            xReturn = xParent->queryDispatch( aURL, sNewTarget, nSearchFlags );
                                        }
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            case E_BEAMER        :  {
                                        css::uno::Reference< css::frame::XDispatchProvider > xBeamer( xOwner->findFrame( SPECIALTARGET_BEAMER, css::frame::FrameSearchFlag::CHILDREN || css::frame::FrameSearchFlag::SELF ), css::uno::UNO_QUERY );
                                        if( xBeamer.is() == sal_True )
                                        {
                                            xReturn = xBeamer->queryDispatch( aURL, SPECIALTARGET_SELF, 0 );
                                        }
                                        else
                                        {
                                            css::uno::Reference< css::frame::XDispatchProvider > xController( xOwner->getController(), css::uno::UNO_QUERY );
                                            if( xController.is() == sal_True )
                                            {
                                                xReturn = xController->queryDispatch( aURL, sNewTarget, nSearchFlags );
                                            }
                                        }
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            case E_MENUBAR       :  {
                                        xReturn = implts_getOrCreateDispatchHelper( E_MENUDISPATCHER );
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            case E_HELPAGENT     :  {
                                        xReturn = implts_getOrCreateDispatchHelper( E_HELPAGENTDISPATCHER );
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            case E_TASKS         :
            case E_DEEP_DOWN     :
            case E_FLAT_DOWN     :
            case E_DEEP_BOTH     :
            case E_FLAT_BOTH     :  {
                                        sal_Int32 nNewFlags  =  nSearchFlags                       ;
                                                  nNewFlags &= ~css::frame::FrameSearchFlag::CREATE;
                                        css::uno::Reference< css::frame::XFrame > xFrame = xOwner->findFrame( sNewTarget, nNewFlags );
                                        if( xFrame.is() == sal_True )
                                        {
                                            xReturn = css::uno::Reference< css::frame::XDispatchProvider >( xFrame, css::uno::UNO_QUERY )->queryDispatch( aURL, SPECIALTARGET_SELF, 0 );
                                        }
                                        else
                                        if(
                                            ( aInfo.bCreationAllowed           == sal_True )    &&
                                            ( implts_isLoadableContent( aURL ) == sal_True )
                                          )
                                        {
                                            css::uno::Any aParameter;
                                            aParameter <<= sNewTarget;
                                            xReturn = implts_getOrCreateDispatchHelper( E_CREATEDISPATCHER, aParameter );
                                        }
                                    }
                                    break;
            //-----------------------------------------------------------------------------------------------------
            #ifdef ENABLE_WARNINGS
            default             :   {
                                        if( eResult != E_UNKNOWN )
                                        {
                                            LOG_WARNING( "DispatchProvider::queryDispatch()", "No valid result found ... could it be an algorithm error?!" )
                                        }
                                    }
                                    break;
            #endif
        }
    }

    // Return result of this operation.
    return xReturn;
}

/*-************************************************************************************************************//**
    @interface  XDispatchProvider
    @short      do the same like queryDispatch() ... but handle multiple dispatches at the same time
    @descr      It's an optimism. User give us a list of queries ... and we return a list of dispatcher.
                If one of given queries couldn't be solved to a real existing dispatcher ...
                we return a list with empty references in it! Order of both lists will be retained!

    @seealso    method queryDispatch()

    @param      "aURL"              , URL to dispatch.
    @param      "sTargetFrameName"  , name of searched frame.
    @param      "nSearchFlags"      , flags for searching.
    @return     A reference a dispatcher for these URL (if someone was found!).

    @onerror    We return a NULL-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL DispatchProvider::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter ...
    LOG_ASSERT2( implcp_queryDispatches( lDescriptions ), "DispatchProvider::queryDispatches()", "Invalid Parameter detected!" )
    // Register operation as transaction and reject wrong calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Set default return value if query failed.
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Make snapshot of neccessary member by using a read lock.
    // Resolve weakreference to owner and hold a normal reference till the end of this method.
    // So he couldn't die .-)
    css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );

    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xOwner.is() == sal_True )
    {
        // Get count of all given discriptors ...
        sal_Int32 nCount = lDescriptions.getLength();
        if( nCount > 0 )
        {
            // ... to get enough memory for expected dispatcher list
            // We can't get more dispatcher as descriptors exist!
            lDispatcher.realloc( nCount );
            // Step over all descriptors and try to get any dispatcher for it.
            for( sal_Int32 nPosition=0; nPosition<nCount; ++nPosition )
            {
                lDispatcher[nPosition] = queryDispatch( lDescriptions[nPosition].FeatureURL  ,
                                                        lDescriptions[nPosition].FrameName   ,
                                                        lDescriptions[nPosition].SearchFlags );
            }
        }
    }

    return lDispatcher;
}

/*-****************************************************************************************************//**
    @interface  XEventListener
    @short      release this object
    @descr      This implementation is used as a helper of a Task, PlugInFrame or Frame.
                These services could get a dispose() call from his owner.
                We should die at the same time - disposed by our owner frame!
                That's why we are a listener for disposing events ...

    @attention  We accept disposing events of our owner frame only!

    @seealso    method Frame::dispose()

    @param      "aEvent", describe the source of this event and should be the owner of this object.
    @return     -

    @onerror    We do nothing.
    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL DispatchProvider::disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // This method is not defined for all incoming parameter.
    LOG_ASSERT2( implcp_disposing( aEvent ), "DispatchProvider::disposing()", "Invalid parameter detected!" )

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    // Try to get access on our owner frame. If he already died ... we will get some trouble.
    // But normaly HE sended this event ... if we should handle it.
    css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
    // Ignore event, if source isn't our owner frame!
    // Nobody should dispose us ... if he isn't our real owner.
    sal_Bool bIgnore = sal_True;
    if(
        ( xOwner.is()   == sal_True )   &&
        ( aEvent.Source == xOwner   )
      )
    {
        bIgnore = sal_False;
    }
    aWriteLock.unlock();

    if( bIgnore == sal_False )
    {
        // Follow lines are safe too - because our transaction helper are threadsafe by herself ...
        // and after "setWorkingMode() we are alone!

        /* Disposing of a service is a special procedure.
        We have registered all interface operations as non breakable ones.
        So we must disable this object for working - further requests will be rejected then.
        After that we can release our internal structures ... and die :_(
        */

        // It's a good idea to hold usself alive ... Because we should prevent us against dieing during tis operation.
        // Other people could release there references to use ... and then our dtor is called automaticly!
        css::uno::Reference< css::uno::XInterface > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

        // Register operation as transaction and reject wrong calls!
        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

        // Now we should disable object for real working.
        // Don't forget to disable current running transaction of THIS method.
        // setWorkingMode() will wait for all current registered transactions ...
        // and will wait for ever then!
        aTransaction.stop();
        m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

        // Now we are allone. Further calls are rejected by E_HARDEXCEPTIONS ... see before.
        // We can work on our member without any lock!

        /* Attention
            If you call some internal impl methods during follow lines ... you should
            think about it again. Because: These internal methods could úse E_SOFTEXCEPTIONS to work
            during this disposing call ... and use some internal member too.
            So you must synchronize it!
        */

        // Release our internal dispatch helper ... BUT DON'T dispose it!
        // They are listener on our owner frame too. So they will get same event from it and die herself.
        // But we should release our references and stop working on it.
        m_xMenuDispatcher       = css::uno::Reference< css::frame::XDispatch >()          ;
        m_xMailToDispatcher     = css::uno::Reference< css::frame::XDispatch >()          ;
        m_xHelpAgentDispatcher  = css::uno::Reference< css::frame::XDispatch >()          ;
        m_xBlankDispatcher      = css::uno::Reference< css::frame::XDispatch >()          ;
        m_xSelfDispatcher       = css::uno::Reference< css::frame::XDispatch >()          ;
        m_xAppDispatchProvider  = css::uno::Reference< css::frame::XDispatchProvider >()  ;
        m_xSyncDispatcher       = css::uno::Reference< css::frame::XDispatch >()          ;

        // Forget all other references too.
        m_xFactory              = css::uno::Reference< css::lang::XMultiServiceFactory >();
        m_xFrame                = css::uno::WeakReference< css::frame::XFrame >()         ;

        // Disable object for working ... Do it for ever .-)
        m_aTransactionManager.setWorkingMode( E_CLOSE );
    }
}

/*-************************************************************************************************************//**
    @short      search right dispatcher for given protocol/URL
    @descr      We know different dispatch helper for different types of URLs. So w must decide, which one is required.
                Thats the reason for this method. We check given URL and return right dispatcher.

    @seealso    method queryDispatch()

    @param      "aURL" , URL with protocol, which must handled
    @param      "aInfo", informations about environment of owner frame (neccessary to check typ of frame!)
    @return     A reference to a dispatch helper.

    @onerror    We return a NULL-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_searchProtocolHandler( const css::util::URL& aURL  ,
                                                                                             const TargetInfo&     aInfo )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register operation as transaction and reject wrong calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // default value, if operation failed.
    css::uno::Reference< css::frame::XDispatch > xHandler;

    //-------------------------------------------------------------------------------------------------------------
    /*HACK ... search better place! */
    if( aURL.Complete == DECLARE_ASCII("private:factory/sync") )
    {
        xHandler = implts_getOrCreateDispatchHelper( E_SYNCDISPATCHER );
    }
    else
    //-------------------------------------------------------------------------------------------------------------
    // May be - it's a "mailto:" URL ...?
    if( aURL.Complete.compareToAscii( "mailto:", 7 ) == 0 )
    {
        xHandler = implts_getOrCreateDispatchHelper( E_MAILTODISPATCHER );
    }
    else
    //-------------------------------------------------------------------------------------------------------------
    // May be - it's an internal URL ... like "uno/slot"?
    // But they could be handled by tasks/pluginframes/desktop only.
    // Normal frames don't know it.
    // Supported targets don't know it real too ... but they could ask the global
    // sfx-AppDispatcher!!!
    if(
        ( aURL.Complete.compareToAscii( ".uno"  , 4 ) == 0 ) ||
        ( aURL.Complete.compareToAscii( "slot:" , 5 ) == 0 )
      )
    {
        if(
            ( aInfo.eFrameType == E_TASK        ) ||
            ( aInfo.eFrameType == E_PLUGINFRAME ) ||
            ( aInfo.eFrameType == E_DESKTOP     )
          )
        {
            css::uno::Reference< css::frame::XDispatchProvider > xAppDispatcher = implts_getOrCreateAppDispatchProvider();
            if( xAppDispatcher.is() == sal_True )
            {
                xHandler = xAppDispatcher->queryDispatch( aURL, aInfo.sTargetName, aInfo.nSearchFlags );
            }
        }
    }
    else
    //-------------------------------------------------------------------------------------------------------------
    // We should be last chance for user :-)
    // But loadable ucb content only!
    // This check is neccessary to found out, that
    // support for some protocols isn't installed by user. May be
    // "ftp" isn't available. So we suppress creation of our self dispatcher.
    // The result will be clear. He can't handle it - but he would try it.
    if( implts_isLoadableContent( aURL ) == sal_True )
    {
        xHandler = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER );
    }

    return xHandler;
}

/*-************************************************************************************************************//**
    @short      get or create new dispatch helper
    @descr      Sometimes we need some helper implementations to support dispatching of special URLs or commands.
                But it's not a good idea to hold these services for the whole life time of this provider instance.
                We should create it on demand ...
                Thats why we implement this methods. They return an already existing helper or create a new one otherwise.

    @seealso    -

    @param      "eHelper", specify type of helper
    @return     A reference to a dispatch helper.

    @onerror    We return a NULL-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XDispatchProvider > DispatchProvider::implts_getOrCreateAppDispatchProvider()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register operation as transaction and reject wrong calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if( m_xAppDispatchProvider.is() == sal_False )
    {
        m_xAppDispatchProvider = css::uno::Reference< css::frame::XDispatchProvider >( m_xFactory->createInstance( SERVICENAME_APPDISPATCHPROVIDER ), css::uno::UNO_QUERY );
    }
    return m_xAppDispatchProvider;
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_getOrCreateDispatchHelper( EDispatchHelper eHelper, const css::uno::Any& aParameters )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register operation as transaction and reject wrong calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    css::uno::Reference< css::frame::XDispatch > xDispatchHelper;

    css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
    if( xOwner.is() == sal_True )
    {
        switch( eHelper )
        {
            //-----------------------------------------------------------------------------------------------------
            case E_MENUDISPATCHER       :   {
                                                if( m_xMenuDispatcher.is() == sal_False )
                                                {
                                                    MenuDispatcher* pDispatcher = new MenuDispatcher( m_xFactory, xOwner );
                                                    m_xMenuDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                                                }
                                                xDispatchHelper = m_xMenuDispatcher;
                                            }
                                            break;
            //-----------------------------------------------------------------------------------------------------
            case E_MAILTODISPATCHER     :   {
                                                if( m_xMailToDispatcher.is() == sal_False )
                                                {
                                                    MailToDispatcher* pDispatcher = new MailToDispatcher( m_xFactory, xOwner );
                                                    m_xMailToDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                                                }
                                                xDispatchHelper = m_xMailToDispatcher;
                                            }
                                            break;
            //-----------------------------------------------------------------------------------------------------
            case E_HELPAGENTDISPATCHER  :   {
                                                if( m_xHelpAgentDispatcher.is() == sal_False )
                                                {
                                                    HelpAgentDispatcher* pDispatcher = new HelpAgentDispatcher( xOwner );
                                                    m_xHelpAgentDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                                                }
                                                xDispatchHelper = m_xHelpAgentDispatcher;
                                            }
                                            break;
            //-----------------------------------------------------------------------------------------------------
            case E_CREATEDISPATCHER     :   {
                                                // Don't hold these dispatch helper! Create it on demand for every given target name.
                                                // He could handle one target frame at one time only!
                                                ::rtl::OUString sTargetName;
                                                aParameters >>= sTargetName;
                                                CreateDispatcher* pDispatcher = new CreateDispatcher( m_xFactory, xOwner, sTargetName );
                                                xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                                            }
                                            break;
            //-----------------------------------------------------------------------------------------------------
            case E_BLANKDISPATCHER      :   {
                                                if( m_xBlankDispatcher.is() == sal_False )
                                                {
                                                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                                                    BlankDispatcher* pDispatcher = new BlankDispatcher( m_xFactory, xDesktop );
                                                    m_xBlankDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                                                }
                                                xDispatchHelper = m_xBlankDispatcher;
                                            }
                                            break;
            //-----------------------------------------------------------------------------------------------------
            case E_SELFDISPATCHER       :   {
                                                if( m_xSelfDispatcher.is() == sal_False )
                                                {
                                                    SelfDispatcher* pDispatcher = new SelfDispatcher( m_xFactory, xOwner );
                                                    m_xSelfDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                                                }
                                                xDispatchHelper = m_xSelfDispatcher;
                                            }
                                            break;
            //-----------------------------------------------------------------------------------------------------
            case E_PLUGINDISPATCHER     :   {
                                                LOG_WARNING( "DispatchProvider::implts_getOrCreateDispatchHelper( E_PLUGINDISPATCHER )", "Not implemented yet!" )
                                            }
                                            break;

            //-----------------------------------------------------------------------------------------------------
            case E_SYNCDISPATCHER       :   {
                                                if( m_xSyncDispatcher.is() == sal_False )
                                                {
                                                    try
                                                    {
                                                        m_xSyncDispatcher = css::uno::Reference< css::frame::XDispatch >( m_xFactory->createInstance( SERVICENAME_SYNCDISPATCHER ), css::uno::UNO_QUERY );
                                                    }
                                                    catch( css::uno::Exception& )
                                                    {
                                                        LOG_ERROR( "DispatchProvider::getOrCreateDispatchHelper( E_SYNCDISPATCHER )", "unexpected situation")
                                                    }
                                                }
                                                xDispatchHelper = m_xSyncDispatcher;
                                            }
                                            break;
        }
    }

    return xDispatchHelper;
}

/*-************************************************************************************************************//**
    @short      check URL for support by our used loader or handler
    @descr      If we must return our own dispatch helper implementations (self, blank, create dispatcher!)
                we should be shure, that URL describe any loadable content. Otherwise slot/uno URLs
                will be detected ... but there exist nothing to detect!

    @seealso    method queryDispatch()

    @param      "aURL", URL which should be "detected"
    @return     True  , if somewhere could handle that
                False , otherwise

    @onerror    We return False.
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool DispatchProvider::implts_isLoadableContent( const css::util::URL& aURL )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register operation as transaction and reject wrong calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::document::XTypeDetection >     xDetection( m_xFactory->createInstance( SERVICENAME_TYPEDETECTION    ), css::uno::UNO_QUERY );
    css::uno::Reference< css::ucb::XContentProviderManager > xUCB      ( m_xFactory->createInstance( SERVICENAME_UCBCONTENTBROKER ), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    sal_Bool bLoadable = sal_False;

    if( xUCB.is() == sal_True )
    {
        bLoadable = xUCB->queryContentProvider( aURL.Complete ).is();
    }
    // no else here!
    if(
        ( bLoadable       == sal_False )    &&
        ( xDetection.is() == sal_True  )
      )
    {
        ::rtl::OUString sTypeName = xDetection->queryTypeByURL( aURL.Complete );
        bLoadable = (sTypeName.getLength()>0);
    }
    // no else here!
    if(
        ( bLoadable                                             == sal_False )  &&
        ( aURL.Complete.compareToAscii( "private:stream/", 15 ) == 0         )
      )
    {
        bLoadable = sal_True;
    }

    return bLoadable;
}

/*-************************************************************************************************************//**
    @short      debug-method to check incoming parameter of some other mehods of this class
    @descr      The following methods are used to check parameters for other methods
                of this class. The return value is used directly for an ASSERT(...).

    @attention  These methods are threadsafe ...
                because they don't work on internal member...
                they are static functions!

    @seealso    ASSERTs in implementation!

    @param      references to checking variables
    @return     sal_True  on invalid parameter<BR>
                sal_False otherway

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
sal_Bool DispatchProvider::implcp_ctor( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory  ,
                                        const css::uno::Reference< css::frame::XFrame >&              xFrame    )
{
    return(
            ( &xFactory     ==  NULL        )   ||
            ( &xFrame       ==  NULL        )   ||
            ( xFactory.is() ==  sal_False   )   ||
            ( xFrame.is()   ==  sal_False   )
          );
}

//*****************************************************************************************************************
// Please don't check for empty strings on URL or target name. They are allowed!
sal_Bool DispatchProvider::implcp_queryDispatch( const css::util::URL&     aURL            ,
                                                 const ::rtl::OUString&    sTargetFrameName,
                                                       sal_Int32           nSearchFlags    )
{
    return(
            ( &aURL             ==  NULL    )   ||
            ( &sTargetFrameName ==  NULL    )   ||
            (
                (    nSearchFlags != css::frame::FrameSearchFlag::AUTO        ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::PARENT    ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::SELF      ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::CHILDREN  ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::CREATE    ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::SIBLINGS  ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::TASKS     ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::ALL       ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::GLOBAL    ) )
            )
          );
}

//*****************************************************************************************************************
// Use implcp_queryDispatch() for every descriptor item to check parameter!
sal_Bool DispatchProvider::implcp_queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions )
{
    sal_Bool bFail = sal_False;
    if( &lDescriptions == NULL )
    {
        bFail = sal_True;
    }
    else
    {
        sal_Int32 nCount = lDescriptions.getLength();
        for( sal_Int32 nPosition=0; nPosition<nCount; ++nPosition )
        {
            if( implcp_queryDispatch( lDescriptions[nPosition].FeatureURL  ,
                                      lDescriptions[nPosition].FrameName   ,
                                      lDescriptions[nPosition].SearchFlags ) == sal_False )
            {
                bFail = sal_True;
                break;
            }
        }
    }
    return bFail;
}

//*****************************************************************************************************************
sal_Bool DispatchProvider::implcp_disposing( const css::lang::EventObject& aEvent )
{
    return(
            ( &aEvent               ==  NULL        )   ||
            ( aEvent.Source.is()    ==  sal_False   )
          );
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
