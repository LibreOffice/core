/*************************************************************************
 *
 *  $RCSfile: framecontainer.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:30 $
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

#ifndef __FRAMEWORK_FRAMECONTAINER_HXX_
#include <classes/framecontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_COMMANDS_HXX_
#include <commands.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCH_FLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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

/**-***************************************************************************************************************
    @short      initialize an empty container
    @descr      The container will be empty then - special features (e.g. the async quit mechanism) are disabled.

    @threadsafe not neccessary - its not a singleton
    @modified   01.07.2002 14:42,as96863
 *****************************************************************************************************************/
FrameContainer::FrameContainer()
        // initialize base classes first.
        // Order is neccessary for right initilization of his and OUR member ... m_aLock
        : ThreadHelpBase ( &Application::GetSolarMutex()                  )
/*DEPRECATEME
        , m_bAsyncQuit   ( sal_False                                      ) // default must be "disabled"!
        , m_aAsyncCall   ( LINK( this, FrameContainer, implts_asyncQuit ) )
*/
{
}

/**-***************************************************************************************************************
    @short      deinitialize may a filled container
    @descr      Special features (if the currently are running) will be dsiabled and we free all used other ressources.

    @threadsafe not neccessary - its not a singleton
    @modified   01.07.2002 14:43,as96863
 *****************************************************************************************************************/
FrameContainer::~FrameContainer()
{
    // Don't forget to free memory!
    clear();
}

/**-***************************************************************************************************************
    @short      append a new frame to the container
    @descr      We accept the incoming frame only, if it is a valid reference and dosnt exist already.

    @param      xFrame
                    frame, which should be added to this container
                    Must be a valid reference.

    @threadsafe yes
    @modified   01.07.2002 14:44,as96863
 *****************************************************************************************************************/
void FrameContainer::append( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if (xFrame.is() && ! exist(xFrame))
    {
        /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
        m_aContainer.push_back( xFrame );
        aWriteLock.unlock();
        /* } SAFE */
    }
}

/**-***************************************************************************************************************
    @short      remove a frame from the container
    @descr      In case we remove the last frame and our internal special feature (the async quit mechanism)
                was enabled by the desktop instance, we start it.

    @param      xFrame
                    frame, which should be deleted from this container
                    Must be a valid reference.

    @threadsafe yes
    @modified   01.07.2002 14:52,as96863
 *****************************************************************************************************************/
void FrameContainer::remove( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    /* SAFE { */
    // write lock neccessary for follwing erase()!
    WriteGuard aWriteLock( m_aLock );

    TFrameIterator aSearchedItem = ::std::find( m_aContainer.begin(), m_aContainer.end(), xFrame );
    if (aSearchedItem!=m_aContainer.end())
    {
        m_aContainer.erase( aSearchedItem );

        // If removed frame was the current active frame - reset state variable.
        if (m_xActiveFrame==xFrame)
            m_xActiveFrame = css::uno::Reference< css::frame::XFrame >();

        // We don't need the write lock any longer ...
        // downgrade to read access.
        aWriteLock.downgrade();
/*DEPRECATEME
        // If last frame was removed and special quit mode is enabled by the desktop
        // we must terminate the application by using this asynchronous callback!
        if (m_aContainer.size()<1 && m_bAsyncQuit)
            m_aAsyncCall.Post(0);
*/
    }

    aWriteLock.unlock();
    // } SAFE
}

/**-***************************************************************************************************************
    @short      check if the given frame currently exist inside the container
    @descr      -

    @param      xFrame
                    reference to the queried frame

    @return     <TRUE/> if frame is oart of this container
                <FALSE/> otherwhise

    @threadsafe yes
    @modified   01.07.2002 14:55,as96863
 *****************************************************************************************************************/
sal_Bool FrameContainer::exist( const css::uno::Reference< css::frame::XFrame >& xFrame ) const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return( ::std::find( m_aContainer.begin(), m_aContainer.end(), xFrame ) != m_aContainer.end() );
    /* } SAFE */
}

/**-***************************************************************************************************************
    @short      delete all existing items of the container
    @descr      -

    @threadsafe yes
    @modified   01.07.2002 15:00,as96863
 *****************************************************************************************************************/
void FrameContainer::clear()
{
    // SAFE {
    WriteGuard aWriteLock( m_aLock );

    // Clear the container ...
    m_aContainer.clear();
    // ... and don't forget to reset the active frame.
    // Its an reference to a valid container-item.
    // But no container item => no active frame!
    m_xActiveFrame = css::uno::Reference< css::frame::XFrame >();
/*DEPRECATEME
    // If special quit mode is used - we must terminate the desktop.
    // He is the owner of this container and can't work without any visible tasks/frames!
    if (m_bAsyncQuit)
        m_aAsyncCall.Post(0);
*/

    aWriteLock.unlock();
    // } SAFE
}

/**-***************************************************************************************************************
    @short      returns count of all current existing frames
    @descr      -

    @deprecated This value can't be guaranteed for multithreading environments.
                So it will be marked as deprecated and should be replaced by "getAllElements()".

    @return     the count of existing container items

    @threadsafe yes
    @modified   01.07.2002 15:00,as96863
 *****************************************************************************************************************/
sal_uInt32 FrameContainer::getCount() const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return( (sal_uInt32)m_aContainer.size() );
    /* } SAFE */
}

/**-***************************************************************************************************************
    @short      returns one item of this container
    @descr      -

    @deprecated This value can't be guaranteed for multithreading environments.
                So it will be marked as deprecated and should be replaced by "getAllElements()".

    @param      nIndex
                    a valud between 0 and (getCount()-1) to adress one container item

    @return     a reference to a frame inside the container, which match with given index

    @threadsafe yes
    @modified   01.07.2002 15:03,as96863
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::operator[]( sal_uInt32 nIndex ) const
{

    css::uno::Reference< css::frame::XFrame > xFrame;
    try
    {
        // Get element form container WITH automatic test of ranges!
        // If index not valid, a out_of_range exception is thrown.
        /* SAFE { */
        ReadGuard aReadLock( m_aLock );
        xFrame = m_aContainer.at( nIndex );
        aReadLock.unlock();
        /* } SAFE */
    }
    catch( std::out_of_range& )
    {
        // The index is not valid for current container-content - we must handle this case!
        // We can return the default value ...
        LOG_EXCEPTION( "FrameContainer::operator[]", "Exception catched ...", DECLARE_ASCII("::std::out_of_range") )
    }
    return xFrame;
}

/**-***************************************************************************************************************
    @short      returns a snapshot of all currently existing frames inside this container
    @descr      Should be used to replace the deprecated functions getCount()/operator[]!

    @return     a list of all frame refrences inside this container

    @threadsafe yes
    @modified   01.07.2002 15:09,as96863
 *****************************************************************************************************************/
css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > FrameContainer::getAllElements() const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );

    sal_Int32                                                       nPosition = 0;
    css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > lElements ( (sal_uInt32)m_aContainer.size() );
    for (TConstFrameIterator pItem=m_aContainer.begin(); pItem!=m_aContainer.end(); ++pItem)
        lElements[nPosition++] = *pItem;

    aReadLock.unlock();
    /* } SAFE */

    return lElements;
}

/**-***************************************************************************************************************
    @short      return state information, if container is empty or not
    @descr      -

    @return     <TRUE/> if container is filled or <FALSE/> if he is empty.

    @threadsafe yes
    @modified   01.07.2002 15:09,as96863
 *****************************************************************************************************************/
sal_Bool FrameContainer::hasElements() const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return( m_aContainer.size()>0 );
    /* } SAFE */
}

/**-***************************************************************************************************************
    @short      set the given frame as  the new active one inside this container
    @descr      We accept this frame only, if it's already a part of this container.

    @param      xFrame
                    reference to the new active frame
                    Must be a valid reference and already part of this container.

    @threadsafe yes
    @modified   01.07.2002 15:11,as96863
 *****************************************************************************************************************/
void FrameContainer::setActive( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if (xFrame.is() && exist(xFrame))
    {
        /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
        m_xActiveFrame = xFrame;
        aWriteLock.unlock();
        /* } SAFE */
    }
}

/**-***************************************************************************************************************
    @short      return sthe current active frame of this container
    @descr      Value can be null in case the frame was removed from the container and nobody
                from outside decide which of all others should be the new one ...

    @return     a reference to the current active frame
                Value can be NULL!

    @threadsafe yes
    @modified   01.07.2002 15:11,as96863
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::getActive() const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return m_xActiveFrame;
    /* } SAFE */
}

/**-***************************************************************************************************************
    @short      enables the async quit mode, which terminates the office if last task will be closed
    @descr      If the last visible task will gone, nobody show any UI then. But without any UI the user
                has no chance to quit the application realy. So we must shutdown by ourself.
                We do that by an async quit call.

                But note: It's not neccessary to use this mechanism, if using the office doesn't require it.
                e.g. the command line parameters "-invisible", -headless" starts the office in a server mode.
                In this case the outside user controls the lifetime of it and must terminate it manually.

    @param      xDesktop
                    only the child frame container of the desktop instance can use this special quit mode.
                    Because only top level frames are used for checking.

    @threadsafe yes
    @modified   30.01.2003 11:26,as96863
 *****************************************************************************************************************/
void FrameContainer::enableQuitTimer( const css::uno::Reference< css::frame::XDesktop >& xDesktop )
{
/*DEPRECATEME
    // SAFE {
    WriteGuard aWriteLock( m_aLock );
    m_xDesktop   = xDesktop;
    m_bAsyncQuit = xDesktop.is();
    aWriteLock.unlock();
    // } SAFE
*/
}

/**-***************************************************************************************************************
    @short      disable the async quit mechanism again
    @descr      Set the right state internaly and forget the desktop reference.
                It's not neccessary to save this information any longer.
                If user wish to establish this mode again he must do it with "enableQuitTimer()".

    @threadsafe yes
    @modified   30.01.2003 11:23,as96863
 *****************************************************************************************************************/
void FrameContainer::disableQuitTimer()
{
/*DEPRECATEME
    // SAFE {
    WriteGuard aWriteLock( m_aLock );
    m_bAsyncQuit = sal_False;
    m_xDesktop   = css::uno::WeakReference< css::frame::XDesktop >();
    aWriteLock.unlock();
    // } SAFE
*/
}

/**-***************************************************************************************************************
    @short      asyncronous callback for our special quit feature
    @descr      In case the last frame was removed from this container and our owner is the desktop itself,
                we have to terminate the whole application.
                But we have to check, between starting this mechanism and now no new frame was opened.

    @threadsafe yes
    @modified   30.01.2003 11:24,as96863
 *****************************************************************************************************************/
/*DEPRECATEME
IMPL_LINK( FrameContainer, implts_asyncQuit, void*, pVoid )
{
    // SAFE {
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XDesktop > xDesktop ( m_xDesktop.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();
    // } SAFE

    css::uno::Reference< css::frame::XFramesSupplier > xSupplier( xDesktop, css::uno::UNO_QUERY );
    if (xSupplier.is())
    {
        css::uno::Reference< css::container::XElementAccess > xFrames( xSupplier->getFrames(), css::uno::UNO_QUERY );
        if (!xFrames.is() || !xFrames->hasElements() )
        {
            LOG_WARNING("FrameContainer::implts_asyncQuit()", "force terminate ...\nPlease check if it's realy neccessary!")
            xDesktop->terminate();
        }
    }

    return 0;
}
*/
/**-***************************************************************************************************************
    @short      implements a simple search based on current container items
    @descr      It can be used for findFrame() and implements a deep down search.

    @param      sName
                    target name, which is searched

    @return     reference to the found frame or NULL if not.

    @threadsafe yes
    @modified   01.07.2002 15:22,as96863
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::searchOnAllChildrens( const ::rtl::OUString& sName ) const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );

    // Step over all child frames. But if direct child isn't the right one search on his children first - before
    // you go to next direct child of this container!
    css::uno::Reference< css::frame::XFrame > xSearchedFrame;
    for( TConstFrameIterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
    {
        if ((*pIterator)->getName()==sName)
        {
            xSearchedFrame = *pIterator;
            break;
        }
        else
        {
            xSearchedFrame = (*pIterator)->findFrame( sName, css::frame::FrameSearchFlag::CHILDREN );
            if (xSearchedFrame.is())
                break;
        }
    }
    aReadLock.unlock();
    /* } SAFE */
    return xSearchedFrame;
}

/**-***************************************************************************************************************
    @short      implements a simple search based on current container items
    @descr      It can be used for findFrame() and search on members of this container only!

    @param      sName
                    target name, which is searched

    @return     reference to the found frame or NULL if not.

    @threadsafe yes
    @modified   01.07.2002 15:22,as96863
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::searchOnDirectChildrens( const ::rtl::OUString& sName ) const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );

    css::uno::Reference< css::frame::XFrame > xSearchedFrame;
    for( TConstFrameIterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
    {
        if ((*pIterator)->getName()==sName)
        {
            xSearchedFrame = *pIterator;
            break;
        }
    }
    aReadLock.unlock();
    /* } SAFE */
    return xSearchedFrame;
}

} //  namespace framework
