/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <classes/framecontainer.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <vcl/svapp.hxx>

namespace framework{

/**-***************************************************************************************************************
    @short      initialize an empty container
    @descr      The container will be empty then - special features (e.g. the async quit mechanism) are disabled.

    @threadsafe not necessary - its not a singleton
 *****************************************************************************************************************/
FrameContainer::FrameContainer()
        
        
        : ThreadHelpBase ( &Application::GetSolarMutex()                  )
/*DEPRECATEME
        , m_bAsyncQuit   ( sal_False                                      ) 
        , m_aAsyncCall   ( LINK( this, FrameContainer, implts_asyncQuit ) )
*/
{
}

/**-***************************************************************************************************************
    @short      deinitialize may a filled container
    @descr      Special features (if the currently are running) will be dsiabled and we free all used other resources.

    @threadsafe not necessary - its not a singleton
 *****************************************************************************************************************/
FrameContainer::~FrameContainer()
{
    
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
    
    WriteGuard aWriteLock( m_aLock );

    TFrameIterator aSearchedItem = ::std::find( m_aContainer.begin(), m_aContainer.end(), xFrame );
    if (aSearchedItem!=m_aContainer.end())
    {
        m_aContainer.erase( aSearchedItem );

        
        if (m_xActiveFrame==xFrame)
            m_xActiveFrame = css::uno::Reference< css::frame::XFrame >();

        
        
        aWriteLock.downgrade();
    }

    aWriteLock.unlock();
    
}

/**-***************************************************************************************************************
    @short      check if the given frame currently exist inside the container
    @descr      -

    @param      xFrame
                    reference to the queried frame

    @return     <TRUE/> if frame is oart of this container
                <FALSE/> otherwise

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
    
    WriteGuard aWriteLock( m_aLock );

    
    m_aContainer.clear();
    
    
    
    m_xActiveFrame = css::uno::Reference< css::frame::XFrame >();

    aWriteLock.unlock();
    
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
                So it will be marked as deprecatedf and should be replaced by "getAllElements()".

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
        
        
        /* SAFE { */
        ReadGuard aReadLock( m_aLock );
        xFrame = m_aContainer.at( nIndex );
        aReadLock.unlock();
        /* } SAFE */
    }
    catch( const std::out_of_range& )
    {
        
        
        SAL_INFO( "fwk", "FrameContainer::operator[]: Exception caught: std::out_of_range" );
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
css::uno::Reference< css::frame::XFrame > FrameContainer::searchOnAllChildrens( const OUString& sName ) const
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
css::uno::Reference< css::frame::XFrame > FrameContainer::searchOnDirectChildrens( const OUString& sName ) const
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
