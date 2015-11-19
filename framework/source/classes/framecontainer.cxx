/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <classes/framecontainer.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>

namespace framework{

/**-***************************************************************************************************************
    @short      initialize an empty container
    @descr      The container will be empty then - special features (e.g. the async quit mechanism) are disabled.

    @threadsafe not necessary - its not a singleton
 *****************************************************************************************************************/
FrameContainer::FrameContainer()
/*DEPRECATEME
        , m_bAsyncQuit   ( sal_False                                      ) // default must be "disabled"!
        , m_aAsyncCall   ( LINK( this, FrameContainer, implts_asyncQuit ) )
*/
{
}

/**-***************************************************************************************************************
    @short      deinitialize may a filled container
    @descr      Special features (if the currently are running) will be disabled and we free all used other resources.

    @threadsafe not necessary - its not a singleton
 *****************************************************************************************************************/
FrameContainer::~FrameContainer()
{
    // Don't forget to free memory!
    m_aContainer.clear();
    m_xActiveFrame.clear();
}

/**-***************************************************************************************************************
    @short      append a new frame to the container
    @descr      We accept the incoming frame only, if it is a valid reference and doesn't exist already.

    @param      xFrame
                    frame, which should be added to this container
                    Must be a valid reference.

    @threadsafe yes
 *****************************************************************************************************************/
void FrameContainer::append( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if (xFrame.is() && ! exist(xFrame))
    {
        SolarMutexGuard g;
        m_aContainer.push_back( xFrame );
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
    SolarMutexGuard g;

    TFrameContainer::iterator aSearchedItem = ::std::find( m_aContainer.begin(), m_aContainer.end(), xFrame );
    if (aSearchedItem!=m_aContainer.end())
    {
        m_aContainer.erase( aSearchedItem );

        // If removed frame was the current active frame - reset state variable.
        if (m_xActiveFrame==xFrame)
            m_xActiveFrame.clear();
    }
}

/**-***************************************************************************************************************
    @short      check if the given frame currently exist inside the container
    @param      xFrame
                    reference to the queried frame

    @return     <TRUE/> if frame is part of this container
                <FALSE/> otherwise

    @threadsafe yes
 *****************************************************************************************************************/
bool FrameContainer::exist( const css::uno::Reference< css::frame::XFrame >& xFrame ) const
{
    SolarMutexGuard g;
    return( ::std::find( m_aContainer.begin(), m_aContainer.end(), xFrame ) != m_aContainer.end() );
}

/**-***************************************************************************************************************
    @short      delete all existing items of the container
    @threadsafe yes
 *****************************************************************************************************************/
void FrameContainer::clear()
{
    SolarMutexGuard g;
    // Clear the container ...
    m_aContainer.clear();
    // ... and don't forget to reset the active frame.
    // Its an reference to a valid container-item.
    // But no container item => no active frame!
    m_xActiveFrame.clear();
}

/**-***************************************************************************************************************
    @short      returns count of all current existing frames
    @deprecated This value can't be guaranteed for multithreading environments.
                So it will be marked as deprecated and should be replaced by "getAllElements()".

    @return     the count of existing container items

    @threadsafe yes
 *****************************************************************************************************************/
sal_uInt32 FrameContainer::getCount() const
{
    SolarMutexGuard g;
    return( (sal_uInt32)m_aContainer.size() );
}

/**-***************************************************************************************************************
    @short      returns one item of this container
    @deprecated This value can't be guaranteed for multithreading environments.
                So it will be marked as deprecatedf and should be replaced by "getAllElements()".

    @param      nIndex
                    a value between 0 and (getCount()-1) to address one container item

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
        SolarMutexGuard g;
        xFrame = m_aContainer.at( nIndex );
    }
    catch( const std::out_of_range& )
    {
        // The index is not valid for current container-content - we must handle this case!
        // We can return the default value ...
        SAL_INFO( "fwk", "FrameContainer::operator[]: Exception caught: std::out_of_range" );
    }
    return xFrame;
}

/**-***************************************************************************************************************
    @short      returns a snapshot of all currently existing frames inside this container
    @descr      Should be used to replace the deprecated functions getCount()/operator[]!

    @return     a list of all frame references inside this container

    @threadsafe yes
 *****************************************************************************************************************/
css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > FrameContainer::getAllElements() const
{
    SolarMutexGuard g;
    return comphelper::containerToSequence(m_aContainer);
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
        SolarMutexGuard g;
        m_xActiveFrame = xFrame;
    }
}

/**-***************************************************************************************************************
    @short      return the current active frame of this container
    @descr      Value can be null in case the frame was removed from the container and nobody
                from outside decide which of all others should be the new one...

    @return     a reference to the current active frame
                Value can be NULL!

    @threadsafe yes
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::getActive() const
{
    SolarMutexGuard g;
    return m_xActiveFrame;
}

/**-***************************************************************************************************************
    @short      implements a simple search based on current container items
    @descr      It can be used for findFrame() and implements a deep down search.

    @param      sName
                    target name, which is searched

    @return     reference to the found frame or NULL if not.

    @threadsafe yes
 *****************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > FrameContainer::searchOnAllChildrens( const OUString& sName ) const
{
    SolarMutexGuard g;
    // Step over all child frames. But if direct child isn't the right one search on his children first - before
    // you go to next direct child of this container!
    css::uno::Reference< css::frame::XFrame > xSearchedFrame;
    for( TFrameContainer::const_iterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
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
css::uno::Reference< css::frame::XFrame > FrameContainer::searchOnDirectChildrens( const OUString& sName ) const
{
    SolarMutexGuard g;
    css::uno::Reference< css::frame::XFrame > xSearchedFrame;
    for( TFrameContainer::const_iterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
    {
        if ((*pIterator)->getName()==sName)
        {
            xSearchedFrame = *pIterator;
            break;
        }
    }
    return xSearchedFrame;
}

} //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
