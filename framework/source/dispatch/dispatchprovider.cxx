/*************************************************************************
 *
 *  $RCSfile: dispatchprovider.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:36 $
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

#include <stdio.h>

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

#ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
#include <dispatch/closedispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_MENUDISPATCHER_HXX_
#include <dispatch/menudispatcher.hxx>
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

#ifndef __FRAMEWORK_PROTOCOLS_H_
#include <protocols.h>
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

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
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
DEFINE_XINTERFACE_2( DispatchProvider                               ,
                     OWeakObject                                    ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider     ),
                     DIRECT_INTERFACE(css::frame::XDispatchProvider)
                   )

DEFINE_XTYPEPROVIDER_2( DispatchProvider             ,
                        css::lang::XTypeProvider     ,
                        css::frame::XDispatchProvider
                      )

//_________________________________________________________________________________________________________________

/**
    @short      standard ctor/dtor
    @descr      These initialize a new instance of tihs class with needed informations for work.
                We hold a weakreference to our owner frame which start dispatches at us.
                We can't use a normal reference because he hold a reference of us too ...
                nobody can die so ...!

    @seealso    using at owner

    @param      xFactory
                    reference to servicemanager to create new services.
    @param      xFrame
                    reference to our owner frame.

    @modified   17.05.2002 10:07, as96863
*/
DispatchProvider::DispatchProvider( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory  ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame    )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xFrame      ( xFrame                        )
        , m_xFactory    ( xFactory                      )
{
}

css::uno::WeakReference< css::mozilla::XPluginInstance > DispatchProvider::m_xPluginInterceptor=css::uno::WeakReference< css::mozilla::XPluginInstance >();

//_________________________________________________________________________________________________________________

/**
    @short      protected(!) dtor for deinitializing
    @descr      We made it protected to prevent using of us as base class instead as a member.

    @modified   17.05.2002 10:05, as96863
 */
DispatchProvider::~DispatchProvider()
{
}

//_________________________________________________________________________________________________________________

/**
    @interface  XDispatchProvider
    @short      search a dispatcher for given URL
    @descr      If no interceptor is set on owner, we search for right frame and dispatch URL to it.
                If no frame was found, we do nothing.
                But we doesn't do it directly here. We detect the type of our owner frame and calls
                specialized queryDispatch() helper dependen from that. Because a Desktop handle some
                requests in another way then a plugin or normal frame.

    @param      aURL
                    URL to dispatch.
    @param      sTargetFrameName
                    name of searched frame.
    @param      nSearchFlags
                    flags for searching.
    @return     A reference to a dispatch object for this URL (if someone was found!).

    @threadsafe yes
    @modified   17.05.2002 10:59, as96863
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL DispatchProvider::queryDispatch( const css::util::URL&  aURL             ,
                                                                                       const ::rtl::OUString& sTargetFrameName ,
                                                                                             sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDesktop >          xDesktopCheck( xOwner, css::uno::UNO_QUERY );
    css::uno::Reference< css::mozilla::XPluginInstance > xPluginCheck ( xOwner, css::uno::UNO_QUERY );

    if (xDesktopCheck.is())
        xDispatcher = implts_queryDesktopDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);
    else
    if (xPluginCheck.is())
        xDispatcher = implts_queryPluginDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);
    else
        xDispatcher = implts_queryFrameDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @interface  XDispatchProvider
    @short      do the same like queryDispatch() ... but handle multiple dispatches at the same time
    @descr      It's an optimism. User give us a list of queries ... and we return a list of dispatcher.
                If one of given queries couldn't be solved to a real existing dispatcher ...
                we return a list with empty references in it! Order of both lists will be retained!

    @seealso    method queryDispatch()

    @param      lDescriptions
                    a list of all dispatch parameters for multiple requests
    @return     A reference a list of dispatch objects for these URLs - may with some <NULL/> values inside.

    @threadsafe yes
    @modified   17.05.2002 09:55, as96863
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL DispatchProvider::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions ) throw( css::uno::RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    sal_Int32                                                          nCount     = lDescriptions.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );

    // Step over all descriptors and try to get any dispatcher for it.
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( lDescriptions[i].FeatureURL  ,
                                        lDescriptions[i].FrameName   ,
                                        lDescriptions[i].SearchFlags );
    }

    return lDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      set a new plugin frame for intercepting special queryDispatch() requests directly
    @descr      Some requests (e.g. creation of new tasks) must be done by forwarding it to the
                used browser. Normaly we can start a search for any existing plugin instance on our
                desktop container to detect (a) if we run in plugin mode and (b) to use one of such
                plugins to send the request to the browser. It can be very expensive. So
                we optimize this process. If the desktop gets a new child frame which represent a plugin
                he set it as a static member on this class. So all dispatch objects inside the frame tree
                knows them and can can use it. But we hold a weak reference only. So we the plugin mustn't
                be deregistered again. We can try to get the real reference and if it failed we don't use it.

    @threadsafe yes
    @modified   17.05.2002 08:25, as96863
 */
void DispatchProvider::setPluginInterceptor( const css::uno::Reference< css::mozilla::XPluginInstance >& xPlugin )
{
    /* STATIC SAFE { */
    WriteGuard aExclusivLock( LockHelper::getGlobalLock() );
    m_xPluginInterceptor = xPlugin;
    aExclusivLock.unlock();
    /* } STATIC SAFE */
}

//_________________________________________________________________________________________________________________

/**
    @short      helper for queryDispatch()
    @descr      Every member of the frame tree (frame, task, plugin, desktop) must handle such request
                in another way. So we implement different specialized metods for every one.

    @threadsafe yes
    @modified   17.05.2002 08:38, as96863
 */
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryDesktopDispatch( const css::uno::Reference< css::frame::XFrame > xDesktop         ,
                                                                                            const css::util::URL&                           aURL             ,
                                                                                            const ::rtl::OUString&                          sTargetFrameName ,
                                                                                                  sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    // ignore wrong requests which are not supported
    if (
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )   ||    // valid for frame dispatches - not for desktop
        (sTargetFrameName==SPECIALTARGET_HELPAGENT)   ||    // valid for frame dispatches - not for desktop
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    // we have no parent by definition
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         // beamer frames are allowed as child of tasks only -
                                                            // and they exist more then ones. We have no idea which our sub tasks is the right one
       )
    {
        return NULL;
    }

    /*  TODO: In special plugin mode we must use the already found plugin frame to forward the request
            to the browser. Thats the only solution till we get the functionality to create
            an emtpy browser task synchronously.
     */

    /* STATIC SAFE { */
    ReadGuard aGlobalReadLock( LockHelper::getGlobalLock() );
    css::uno::Reference< css::frame::XDispatchProvider > xPlugin( m_xPluginInterceptor.get(), css::uno::UNO_QUERY );
    aGlobalReadLock.unlock();
    /* } STATIC SAFE */

    //-----------------------------------------------------------------------------------------------------
    // I) handle special cases which not right for using findFrame() first
    //-----------------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank"
    //  It's not the right place to create a new task here - because we are queried for a dispatch object
    //  only, which can handle such request. Such dispatcher should create the required task on demand.
    //  Normaly the functionality for "_blank" is provided by findFrame() - but that would create it directly
    //  here. Thats why we must "intercept" here.
    //-----------------------------------------------------------------------------------------------------
    if (sTargetFrameName==SPECIALTARGET_BLANK)
    {
        if (implts_isLoadableContent(aURL))
        {
            if (xPlugin.is())
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xPlugin, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_BLANK,0); // ask him for _blank ... not for _self!
            }
            else
            {
                xDispatcher = implts_getOrCreateDispatchHelper( E_BLANKDISPATCHER, xDesktop );
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) "_default"
    //  This is a combination of search an empty task for recycling - or create a new one.
    //  Normaly we can use a specialized dispatch object for that ... but not for plugin mode.
    //  Such search isn't provided there.
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetFrameName==SPECIALTARGET_DEFAULT)
    {
        if (implts_isLoadableContent(aURL))
        {
            if (xPlugin.is())
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xPlugin, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_BLANK,0); // ask him for _blank ... not for _default!
            }
            else
            {
                xDispatcher = implts_getOrCreateDispatchHelper( E_DEFAULTDISPATCHER, xDesktop );
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.III) "_self", "", "_top"
    //  The desktop can't load any document - but he can handle some special protocols like "uno", "slot" ...
    //  Why is "top" here handled too? Because the desktop is the topest frame. Normaly it's superflous
    //  to use this target - but we can handle it in the same manner then "_self".
    //-----------------------------------------------------------------------------------------------------
    else
    if (
        (sTargetFrameName==SPECIALTARGET_SELF)  ||
        (sTargetFrameName==SPECIALTARGET_TOP )  ||
        (sTargetFrameName.getLength()<1      )
       )
    {
        xDispatcher = implts_searchProtocolHandler(aURL);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.IV) no further special targets exist
    //  Now we have to search for the right target frame by calling findFrame() - but should provide our code
    //  against creation of a new task if no frame could be found.
    //  I said it b efore - it's allowed for dispatch() only.
    //-----------------------------------------------------------------------------------------------------
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        // try to find any existing target and ask him for his dispatcher
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xDesktop->findFrame(sTargetFrameName, nRightFlags);
        if (xFoundFrame.is())
        {
            css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
            xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        else
        // if it couldn't be found - but creation was allowed
        // use special dispatcher for creatio or froward it to the browser
        if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
        {
            if (xPlugin.is())
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xPlugin, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_BLANK,0);
            }
            else
            {
                css::uno::Any aParameter;
                aParameter <<= sTargetFrameName;
                xDispatcher = implts_getOrCreateDispatchHelper( E_CREATEDISPATCHER, xDesktop, aParameter );
            }
        }
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryFrameDispatch( const css::uno::Reference< css::frame::XFrame > xFrame           ,
                                                                                          const css::util::URL&                           aURL             ,
                                                                                          const ::rtl::OUString&                          sTargetFrameName ,
                                                                                                sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    /*  TODO: In special plugin mode we must use the already found plugin frame to forward the request
            to the browser. Thats the only solution till we get the functionality to create
            an emtpy browser task synchronously.
     */

    /* STATIC SAFE { */
    ReadGuard aGlobalReadLock( LockHelper::getGlobalLock() );
    css::uno::Reference< css::frame::XDispatchProvider > xPlugin( m_xPluginInterceptor.get(), css::uno::UNO_QUERY );
    aGlobalReadLock.unlock();
    /* } STATIC SAFE */

    //-----------------------------------------------------------------------------------------------------
    // 0) Some URLs are dispatched in a generic way (e.g. by the menu) using the default target "".
    //    But they are specified to use her own fix target. Detect such URLs here and use the correct target.
    //-----------------------------------------------------------------------------------------------------

    ::rtl::OUString sTargetName = sTargetFrameName;

    if (
        (aURL.Complete.equalsAscii(".uno:CloseDoc")) ||
        (aURL.Complete.equalsAscii(".uno:CloseWin"))
       )
    {
        sTargetName = SPECIALTARGET_TOP;
    }

    //-----------------------------------------------------------------------------------------------------
    // I) handle special cases which not right for using findFrame() first
    //-----------------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank", "_default"
    //  It's not the right place to create a new task here. Only the desktop can do that.
    //  Normaly the functionality for "_blank" is provided by findFrame() - but that would create it directly
    //  here. Thats why we must "intercept" here.
    //  Another eason is a possible plugin mode!
    //-----------------------------------------------------------------------------------------------------
    if (
        (sTargetName==SPECIALTARGET_BLANK  ) ||
        (sTargetName==SPECIALTARGET_DEFAULT)
       )
    {
        if (xPlugin.is())
        {
            css::uno::Reference< css::frame::XDispatchProvider > xProvider( xPlugin, css::uno::UNO_QUERY );
            xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_BLANK,0); // ask him for _blank ... not for _default!
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, sTargetName, 0); // its a special target - ignore search flags
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) "_menubar"
    //  Special mode on frame or task to receive the local menu. Not supported by findFrame()
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetName==SPECIALTARGET_MENUBAR)
    {
        xDispatcher = implts_getOrCreateDispatchHelper( E_MENUDISPATCHER, xFrame );
    }

    //-----------------------------------------------------------------------------------------------------
    // I.III) "_helpagent"
    //  Special mode on frame or task to start the help agent.
    //  It's defined for top level frames only.
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetName==SPECIALTARGET_HELPAGENT)
    {
        if (xFrame->isTop())
            xDispatcher = implts_getOrCreateDispatchHelper( E_HELPAGENTDISPATCHER, xFrame );
        else
        {
            // Don''t use findFrame() here - because it's not possible to find
            // a top lebel frame without knowing his name. And a frame with name
            // "" can't be realy searched! That's why forward query to any parent
            // explicitly.
            css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFrame->getCreator(), css::uno::UNO_QUERY );
            if (xProvider.is())
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_HELPAGENT,0);
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.IV) "_helpagent"
    //  Special sub frame of a top frame only. Search or create it. ... OK it's currently a little bit HACKI.
    //  Only the sfx (means the controller) can create it it.
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetName==SPECIALTARGET_BEAMER)
    {
        css::uno::Reference< css::frame::XDispatchProvider > xBeamer( xFrame->findFrame( SPECIALTARGET_BEAMER, css::frame::FrameSearchFlag::CHILDREN | css::frame::FrameSearchFlag::SELF ), css::uno::UNO_QUERY );
        if (xBeamer.is())
        {
            xDispatcher = xBeamer->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (xController.is())
                // force using of special target - but use original search flags
                // May the caller used the CREATE flag or not!
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_BEAMER, nSearchFlags);
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.V) "_parent"
    //  Our parent frame (if it exist) should handle this URL.
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetName==SPECIALTARGET_PARENT)
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            // SELF => we must adress the parent directly... and not his parent or any other parent!
            xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.VI) "_top"
    //  This request must be forwarded to any parent frame, till we reach a top frame.
    //  If no parent exist, we can handle itself.
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetName==SPECIALTARGET_TOP)
    {
        if (xFrame->isTop())
        {
            // There exist a hard coded interception for special URLs.
            if (
                (aURL.Complete.equalsAscii(".uno:CloseDoc"  )) ||
                (aURL.Complete.equalsAscii(".uno:CloseWin"  )) ||
                (aURL.Complete.equalsAscii(".uno:CloseFrame"))
               )
            {
                xDispatcher = implts_getOrCreateDispatchHelper( E_CLOSEDISPATCHER, xFrame );
            }
            else
            {
                // If we are this top frame itself (means our owner frame)
                // we should call ourself recursiv with a better target "_self".
                // So we can share the same code! (see reaction for "_self" inside this methode too.)
                xDispatcher = this->queryDispatch(aURL,SPECIALTARGET_SELF,0);
            }
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            // Normaly if isTop() returned FALSE ... the parent frame MUST(!) exist ...
            // But it seams to be better to check that here to prevent us against an access violation.
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_TOP, 0);
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.VII) "_self", ""
    //  Our owner frame should handle this URL. But we can't do it for all of them.
    //  So we ask the internal setted controller first. If he disagree we try to find a registered
    //  protocol handler. If this failed too - we check for a loadable content and in case of true
    //  we load it into the frame by returning specilized dispatch object.
    //-----------------------------------------------------------------------------------------------------
    else
    if (
        (sTargetName==SPECIALTARGET_SELF)  ||
        (sTargetName.getLength()<1      )
       )
    {
        // Ask our controller for his agreement for these dispatched URL ...
        // because some URLs are internal and can be handled faster by SFX - which most is the current controller!
        // But in case of e.g. the bibliography not all queries will be handled successfully here.
        css::uno::Reference< css::frame::XDispatchProvider > xController( xFrame->getController(), css::uno::UNO_QUERY );
        if (xController.is())
            xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_SELF, 0);

        // If controller has no fun to dispatch these URL - we must search another right dispatcher.
        // Search for any registered protocol handler first.
        if (!xDispatcher.is())
            xDispatcher = implts_searchProtocolHandler(aURL);

        // Not for controller - not for protocol handler
        // It should be a loadable content - may be a file. Check it ...
        // This check is neccessary to found out, that
        // support for some protocols isn't installed by user. May be
        // "ftp" isn't available. So we suppress creation of our self dispatcher.
        // The result will be clear. He can't handle it - but he would try it.
        if (
            ( ! xDispatcher.is()             )  &&
            ( implts_isLoadableContent(aURL) )
           )
        {
            xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.VI) no further special handlings exist
    //  Now we have to search for the right target frame by calling findFrame() - but should provide our code
    //  against creation of a new task if no frame could be found.
    //  I said it before - it's allowed for dispatch() only.
    //-----------------------------------------------------------------------------------------------------
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        // try to find any existing target and ask him for his dispatcher
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xFrame->findFrame(sTargetName, nRightFlags);
        if (xFoundFrame.is())
        {
            // Attention: Found target is our own owner frame!
            // Don't ask him for his dispatcher. We know it already - it's our self dispatch helper.
            // Otherwhise we can start a never ending recursiv call. Why?
            // Somewere called our owner frame - he called some interceptor objects - and may by this dispatch provider
            // is called. If wa use queryDispatch() on our owner frame again - we start this call stack again ... and again.
            if (xFoundFrame==xFrame)
                xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
            else
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
            }
        }
        else
        // if it couldn't be found - but creation was allowed
        // forward request to the browser (for plugin mode) or to the desktop
        // (for nomal mode).
        // Note: The given target name must be used to set the name on new created task!
        //       Don't forward request by changing it to a special one e.g _blank.
        //       Use the CREATE flag only to prevent call against further searches.
        //       We already know it - the target must be created new.
        if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
        {
            if (xPlugin.is())
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xPlugin, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL, sTargetName, css::frame::FrameSearchFlag::CREATE);
            }
            else
            {
                css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
                if (xParent.is())
                    xDispatcher = xParent->queryDispatch(aURL, sTargetName, css::frame::FrameSearchFlag::CREATE);
            }
        }
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryPluginDispatch( const css::uno::Reference< css::frame::XFrame > xPlugin          ,
                                                                                           const css::util::URL&                           aURL             ,
                                                                                           const ::rtl::OUString&                          sTargetFrameName ,
                                                                                                 sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank", "_default", "_self", ""
    //  Such requests must be forwarded to the browser!
    //  Normaly the _default mode search for any target which can be recycled or try to find out
    //  if requested ressource was already loaded. But currently we can't support that for plugin mode.
    //-----------------------------------------------------------------------------------------------------
    if (
        (sTargetFrameName==SPECIALTARGET_BLANK  ) ||
        (sTargetFrameName==SPECIALTARGET_DEFAULT) ||
        (sTargetFrameName==SPECIALTARGET_SELF   ) ||
        (sTargetFrameName.getLength()<1         )
       )
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xPlugin );
        fprintf(stderr,"plugin query for _self return %d\n", xDispatcher.is());
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) Most other targets or flags can be handled by the base class functionality
    //  of a PlugInFrame - the Frame. But look for case of not found target in combination with
    //  the CREATE flag. Such request must be forwarded to the browser too!
    //-----------------------------------------------------------------------------------------------------
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        xDispatcher = implts_queryFrameDispatch(xPlugin, aURL, sTargetFrameName, nRightFlags);
        if ( !xDispatcher.is() && ( nSearchFlags & css::frame::FrameSearchFlag::CREATE ) )
        {
            if ( implts_isLoadableContent ( aURL ) )
            {
                ::com::sun::star::uno::Any aParam;
                aParam <<= sTargetFrameName;
                xDispatcher = implts_getOrCreateDispatchHelper( E_CREATEDISPATCHER, xPlugin, aParam );
            }
        }
        fprintf(stderr,"plugin query for possible CREATE return %d\n", xDispatcher.is());
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      search for a registered protocol handler and ask him for a dispatch object
    @descr      Wes earch a suitable handler inside our cfg package org.openoffice.Office.ProtocolHandler.
                If we found anyone, we create and initialize it. Initialize means: we set our owner frame on it
                as context information. He can use it or leave it. Of course - we are aware of handler implementations,
                which doesn't support initialization. It's an optional feature.

    @param      aURL
                    the dispatch URL for which may a handler is registered

    @return     A dispatch object if a handler was found and agree with the given URL or <NULL/> otherwhise.

    @threadsafe yes
    @modified   05.09.2002 13:43, as96863
*/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_searchProtocolHandler( const css::util::URL& aURL )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    ProtocolHandler                              aHandler   ;

    // This member is threadsafe by himself and lives if we live - we doesn't need any mutex here.
    if (m_aProtocolHandlerCache.search(aURL,&aHandler))
    {
        /* SAFE { */
        ReadGuard aReadLock( m_aLock );

        // create it
        css::uno::Reference< css::frame::XDispatchProvider > xHandler;
        try
        {
            xHandler = css::uno::Reference< css::frame::XDispatchProvider >(
                            m_xFactory->createInstance(aHandler.m_sUNOName),
                            css::uno::UNO_QUERY);
        }
        catch(css::uno::Exception&) {}

        // look if initialization is neccessary
        css::uno::Reference< css::lang::XInitialization > xInit( xHandler, css::uno::UNO_QUERY );
        if (xInit.is())
        {
            css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
            LOG_ASSERT(xOwner.is(), "DispatchProvider::implts_searchProtocolHandler()\nCouldn't get reference to my owner frame. So I can't set may needed context information for this protocol handler.")
            if (xOwner.is())
            {
                try
                {
                    // but do it only, if all context informations are OK
                    css::uno::Sequence< css::uno::Any > lContext(1);
                    lContext[0] <<= xOwner;
                    xInit->initialize(lContext);
                }
                catch(css::uno::Exception&) {}
            }
        }

        aReadLock.unlock();
        /* } SAFE */

        // ask for his (sub)dispatcher for the given URL
        LOG_ASSERT(xHandler.is(), "DispatchProvider::implts_searchProtocolHandler()\nThere is one non realy useable protocol handler, which couldn't be created.")
        if (xHandler.is())
            xDispatcher = xHandler->queryDispatch(aURL,SPECIALTARGET_SELF,0);
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      get or create new dispatch helper
    @descr      Sometimes we need some helper implementations to support dispatching of special URLs or commands.
                But it's not a good idea to hold these services for the whole life time of this provider instance.
                We should create it on demand ...
                Thats why we implement this method. It return an already existing helper or create a new one otherwise.

    @param      eHelper
                    specify the requested dispatch helper
    @param      xOwner
                    the target of possible dispatch() call on created dispatch helper
    @param      aParameters
                    some of such helpers need special (optional) parameters
    @return     A reference to a dispatch helper.

    @threadsafe yes
    @modified   17.05.2002 10:14, as96863
*/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_getOrCreateDispatchHelper( EDispatchHelper                                  eHelper     ,
                                                                                                 const css::uno::Reference< css::frame::XFrame >& xOwner      ,
                                                                                                 const css::uno::Any&                             aParameters )
{
    css::uno::Reference< css::frame::XDispatch > xDispatchHelper;

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    /* } SAFE */

    switch (eHelper)
    {
        case E_MENUDISPATCHER :
                {
                    // Attention: Such menue dispatcher must be a singleton for this frame - means our owner frame.
                    // Otherwhise he can make some trouble.
                    /* SAFE { */
                    WriteGuard aWriteLock( m_aLock );
                    if ( ! m_xMenuDispatcher.is() )
                    {
                        MenuDispatcher* pDispatcher = new MenuDispatcher( xFactory, xOwner );
                        m_xMenuDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                    xDispatchHelper = m_xMenuDispatcher;
                    aWriteLock.unlock();
                    /* } SAFE */
                }
                break;

        case E_HELPAGENTDISPATCHER :
                {
                    // Attention: It's not a good idea to create this help agent twice for the same frame (window)
                    // May it will be shown twice too - and user activate the first one. Then he get the corresponding
                    // help window ... but there exist another help agent window on bottom side of the frame window.
                    // It's superflous. Create it on demand - but hold it alive till this provider dies.
                    /* SAFE { */
                    WriteGuard aWriteLock( m_aLock );
                    if ( ! m_xHelpAgentDispatcher.is() )
                    {
                        HelpAgentDispatcher* pDispatcher = new HelpAgentDispatcher( xOwner );
                        m_xHelpAgentDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                    xDispatchHelper = m_xHelpAgentDispatcher;
                    aWriteLock.unlock();
                    /* } SAFE */
                }
                break;

        case E_CREATEDISPATCHER :
                {
                    ::rtl::OUString sTargetName;
                    aParameters >>= sTargetName;
                    CreateDispatcher* pDispatcher = new CreateDispatcher( xFactory, xOwner, sTargetName );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_BLANKDISPATCHER :
                {
                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                    if (xDesktop.is())
                    {
                        BlankDispatcher* pDispatcher = new BlankDispatcher( xFactory, xDesktop, sal_False );
                        xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                }
                break;

        case E_DEFAULTDISPATCHER :
                {
                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                    if (xDesktop.is())
                    {
                        BlankDispatcher* pDispatcher = new BlankDispatcher( xFactory, xDesktop, sal_True );
                        xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                }
                break;

        case E_SELFDISPATCHER :
                {
                    SelfDispatcher* pDispatcher = new SelfDispatcher( xFactory, xOwner );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_CLOSEDISPATCHER :
                {
                    CloseDispatcher* pDispatcher = new CloseDispatcher( xFactory, xOwner );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_PLUGINDISPATCHER :
                {
                    LOG_WARNING( "DispatchProvider::implts_getOrCreateDispatchHelper( E_PLUGINDISPATCHER )", "Not implemented yet!" )
                }
                break;
    }

    return xDispatchHelper;
}

//_________________________________________________________________________________________________________________

/**
    @short      check URL for support by our used loader or handler
    @descr      If we must return our own dispatch helper implementations (self, blank, create dispatcher!)
                we should be shure, that URL describe any loadable content. Otherwise slot/uno URLs
                will be detected ... but there exist nothing for ral loading into a target frame!

    @param      aURL
                    URL which should be "detected"
    @return     <TRUE/> if somewhere could handle that - <FALSE/> otherwise.

    @threadsafe yes
    @modified   17.05.2002 09:47, as96863
*/
sal_Bool DispatchProvider::implts_isLoadableContent( const css::util::URL& aURL )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::document::XTypeDetection >     xDetection( m_xFactory->createInstance( SERVICENAME_TYPEDETECTION    ), css::uno::UNO_QUERY );
    css::uno::Reference< css::ucb::XContentProviderManager > xUCB      ( m_xFactory->createInstance( SERVICENAME_UCBCONTENTBROKER ), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* } SAFE */

    sal_Bool bLoadable = sal_False;

    // (a) a UCB provide loadable contents only
    if (xUCB.is())
        bLoadable = xUCB->queryContentProvider( aURL.Complete ).is();
    // no else here!
    // (b) if we can detect a type for given URL - it must be loadable too
    if(
        ( bLoadable       == sal_False )    &&
        ( xDetection.is() == sal_True  )
      )
    {
        ::rtl::OUString sTypeName = xDetection->queryTypeByURL( aURL.Complete );
        bLoadable = (sTypeName.getLength()>0);
    }
    // no else here!
    // (c) such special URL indicates a given input stream - it should be loadable too
    if( !bLoadable && ProtocolCheck::isProtocol(aURL.Complete,ProtocolCheck::E_PRIVATE_STREAM) )
        bLoadable = sal_True;

    return bLoadable;
}

} // namespace framework
