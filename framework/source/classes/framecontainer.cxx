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


//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <classes/framecontainer.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/frame/FrameSearchFlag.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>

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
    @descr      Special features (if the currently are running) will be dsiabled and we free all used other resources.

    @threadsafe not neccessary - its not a singleton
 *****************************************************************************************************************/
FrameContainer::~FrameContainer()
{
    // Don't forget to free memory!
    m_aContainer.clear();
    m_xActiveFrame.clear();
}

/**-***************************************************************************************************************
    @short      append a new frame to the container
    @descr      We accept the incoming frame only, if it is a valid reference and dosnt exist already.

    @param      xFrame
                    frame, which should be added to this container
                    Must be a valid reference.

    @threadsafe yes
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
                    a valud between 0 and (getCount()-1) to address one container item

    @return     a reference to a frame inside the container, which match with given index

    @threadsafe yes
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
    catch( const std::out_of_range& )
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
    @short      set the given frame as  the new active one inside this container
    @descr      We accept this frame only, if it's already a part of this container.

    @param      xFrame
                    reference to the new active frame
                    Must be a valid reference and already part of this container.

    @threadsafe yes
 *****************************************************************************************************************/
void FrameContainer::setActive( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if ( !xFrame.is() || exist(xFrame) )
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
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::getActive() const
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return m_xActiveFrame;
    /* } SAFE */
}

/**-***************************************************************************************************************
    @short      implements a simple search based on current container items
    @descr      It can be used for findFrame() and implements a deep down search.

    @param      sName
                    target name, which is searched

    @return     reference to the found frame or NULL if not.

    @threadsafe yes
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
