/*************************************************************************
 *
 *  $RCSfile: framecontainer.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:12 $
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

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCH_FLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::std                       ;
using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::frame     ;

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
//  constructor
//*****************************************************************************************************************
FrameContainer::FrameContainer()
        // initialize RWLock-member at first!
        :   FairRWLockBase()
{
    // Make object ready for working.
    // change working mode from E_INIT to E_WORK
    // We don't must look for current set modi - a ctor
    // couldn't be called more then ones ... I think so :-)
    m_aLock.setWorkingMode( E_WORK );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
FrameContainer::~FrameContainer()
{
    // Disable object for working!
    // All further requests will be refused ...
    // but I think it's alittle bit superflous by using in dtor ...
    // May be - it's neccessary?
    // So we wait for current working reader/writer till they finish her work.
    m_aLock.setWorkingMode( E_CLOSE );

    // Disable possible active quit timer!
    // He can be active for owner=desktop only.
    impl_disableQuitTimer();
    // Don't forget to free memory!
    impl_clear();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::append( const Reference< XFrame >& xFrame )
{
    // Safe impossible cases
    // a)   This method is not defined for ALL incoming parameters!
    // b)   Warn programmer at already existing elements in container.
    // c)   Warn programmer if an already existing frame has no component inside!
    //      These frames are created (e.g. by dispatch()) but not used ...
    LOG_ASSERT2( implcp_append( xFrame ), "FrameContainer::append()", "Invalid parameter detected!"             )
    LOG_ASSERT2( exist(xFrame)==sal_True, "FrameContainer::append()", "New frame already exist in container!"   )
    // LOG_ASSERT2( impldbg_existZombie(), "FrameContainer::append()", "Zombie frame detected!"                 )

    // Append new frame to container.
    // Make it threadsafe by using write lock - Look for refused calls => Do nothing then.
    ERefusalReason  eReason                         ;
    WriteGuard      aWriteLock( m_aLock, eReason )  ;
    if( eReason == E_NOREASON )
    {
        m_aContainer.push_back( xFrame );
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::remove( const Reference< XFrame >& xFrame )
{
    // Safe impossible cases
    // a)   This method is not defined for ALL incoming parameters!
    // b)   Warn programmer at non existing elements in container.
    LOG_ASSERT2( implcp_remove( xFrame )    , "FrameContainer::remove()", "Invalid parameter detected!"             )
    LOG_ASSERT2( exist(xFrame)==sal_False   , "FrameContainer::remove()", "Frame to remove not exist in container!" )

    // Search frame and remove it from container if he exist.
    // Make it threadsafe. I think removing container elements must be "atomar".
    // We shouldn't make a copy of current container items to work on it.
    // Use write lock for whole method.
    ERefusalReason  eReason                         ;
    WriteGuard      aWriteLock( m_aLock, eReason )  ;
    if( eReason == E_NOREASON )
    {
        TFrameIterator aSearchedItem = find( m_aContainer.begin(), m_aContainer.end(), xFrame );
        if( aSearchedItem != m_aContainer.end() )
        {
            m_aContainer.erase( aSearchedItem );
            // If removed frame the current active frame - reset state variable.
            if( m_xActiveFrame == xFrame )
            {
                m_xActiveFrame = Reference< XFrame >();
            }
            // We don't need the write lock any longer ...
            // downgrade to read access.
            aWriteLock.downgrade();
            // If last frame was removed and special quit timer is enabled by the desktop
            // we must terminate the desktop by using this timer!
            if  (
                    ( m_aContainer.size()       <   1           )   &&
                    ( m_rQuitTimer.isValid()    ==  sal_True    )
                )
            {
                m_rQuitTimer->start();
            }
        }
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool FrameContainer::exist( const Reference< XFrame >& xFrame ) const
{
    // Safe impossible cases
    // a)   This method is not defined for ALL incoming parameters!
    LOG_ASSERT2( implcp_exist( xFrame ), "FrameContainer::exist()", "Invalid parameter detected!" )

    // Search for frame.
    // Make it threadsafe by using readlock. Declare default return value for refused calls!
    sal_Bool        bExist      = sal_False         ;
    ERefusalReason  eReason                         ;
    ReadGuard       aReadLock   ( m_aLock, eReason );
    if( eReason == E_NOREASON )
    {
        bExist =    (   find(   m_aContainer.begin()    ,
                                m_aContainer.end()      ,
                                   xFrame                   ) != m_aContainer.end()
                    );
    }

    return bExist;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::clear()
{
    // We need write access to our member.
    ERefusalReason  eReason                         ;
    WriteGuard      aWriteLock  ( m_aLock, eReason );
    if( eReason == E_NOREASON )
    {
        impl_clear();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt32 FrameContainer::getCount() const
{
    // We need read access to our member.
    // Declare default return value for refused calls.
    sal_uInt32      nCount      = 0                 ;
    ERefusalReason  eReason                         ;
    ReadGuard       aReadLock   ( m_aLock, eReason );
    if( eReason == E_NOREASON )
    {
        nCount = (sal_uInt32)m_aContainer.size();
    }
    return nCount;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::operator[]( sal_uInt32 nIndex ) const
{
    // Safe impossible cases
    // a)   This method is not defined for ALL incoming parameters!
    LOG_ASSERT2( implcp_IndexOperator( nIndex, getCount() ), "FrameContainer::operator[]()", "Invalid parameter detected!" )

    // Use read lock to make it threadsafe. Declare default return value for refused calls!
    Reference< XFrame > xFrame                          ;
    ERefusalReason      eReason                         ;
    ReadGuard           aReadLock   ( m_aLock, eReason );
    if( eReason == E_NOREASON )
    {
        try
        {
            // Get element form container WITH automatic test of ranges!
            // If index not valid, a out_of_range exception is thrown.
            xFrame = m_aContainer.at( nIndex );
        }
        catch( std::out_of_range& )
        {
            // The index is not valid for current container-content - we must handle this case!
            // We can return the default value ...
            LOG_EXCEPTION( "FrameContainer::operator[]", "Exception catched ...", DECLARE_ASCII("::std::out_of_range") )
        }
    }
    return xFrame;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< Reference< XFrame > > FrameContainer::getAllElements() const
{
    // Use read lock. Declare default return value if call is refused.
    Sequence< Reference< XFrame > > lElements                       ;
    ERefusalReason                  eReason                         ;
    ReadGuard                       aReadLock   ( m_aLock, eReason );
    if( eReason == E_NOREASON )
    {
        sal_uInt32 nCount = (sal_uInt32)m_aContainer.size();
        lElements.realloc( nCount );

        for( sal_uInt32 nPosition=0; nPosition<nCount; ++nPosition )
        {
            lElements[nPosition] = m_aContainer[nPosition];
        }
    }
    return lElements;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool FrameContainer::hasElements() const
{
    // Use read lock. Declare default return value if call is refused.
    sal_Bool        bHasElements=   sal_False       ;
    ERefusalReason  eReason                         ;
    ReadGuard       aReadLock   ( m_aLock, eReason );
    if( eReason == E_NOREASON )
    {
        bHasElements = ( m_aContainer.size() > 0 );
    }
    return bHasElements;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::setActive( const Reference< XFrame >& xFrame )
{
    // Safe impossible cases
    // a)   This method is not defined for ALL incoming parameters!
    // b)   The new active frame MUST exist in container.
    LOG_ASSERT2( implcp_setActive( xFrame )                         , "FrameContainer::setActive()", "Invalid parameter detected!"                                                      )
    LOG_ASSERT2( xFrame.is()==sal_True && exist(xFrame)==sal_False  , "FrameContainer::setActive()", "The new active frame is not a member of current container!You cant activate it."  )

    ERefusalReason  eReason                         ;
    WriteGuard      aWriteLock( m_aLock, eReason )  ;
    if( eReason == E_NOREASON )
    {
        m_xActiveFrame = xFrame;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::getActive() const
{
    // Use read lock. Declare default return value if call is refused.
    Reference< XFrame > xActive                         ;
    ERefusalReason      eReason                         ;
    ReadGuard           aReadLock( m_aLock, eReason )   ;
    if( eReason == E_NOREASON )
    {
        xActive = m_xActiveFrame;
    }
    return xActive;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::enableQuitTimer( const Reference< XDesktop >& xDesktop )
{
    // If no current timer exist - create a new one.
    ERefusalReason  eReason                         ;
    WriteGuard      aWriteLock( m_aLock, eReason )  ;
    if( eReason == E_NOREASON )
    {
        if( m_rQuitTimer.isEmpty() == sal_True )
        {
            m_rQuitTimer.bind( new AsyncQuit( xDesktop ) );
        }
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::disableQuitTimer()
{
    // Delete current quit timer.
    // If user wish to create it again he must do it with "enableQuitTimer()".
    ERefusalReason  eReason                         ;
    WriteGuard      aWriteLock( m_aLock, eReason )  ;
    if( eReason == E_NOREASON )
    {
        impl_disableQuitTimer();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::searchDeepDown( const OUString& sName ) const
{
    // Check incoming parameter.
    LOG_ASSERT2( implcp_searchDeepDown( sName ), "FrameContainer::searchDeepDown()", "Invalid parameter detected!" )

    // Use read lock to make it threadsafe.
    // Declare default return value for refused calls.
    Reference< XFrame > xSearchedFrame                  ;
    ERefusalReason      eReason                         ;
    ReadGuard           aReadLock( m_aLock, eReason )   ;
    if( eReason == E_NOREASON )
    {
        // Step over all child frames. But if direct child isn't the right one search on his children first - before
        // you go to next direct child of this container!
        for( TConstFrameIterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
        {
            if( (*pIterator)->getName() == sName )
            {
                xSearchedFrame = *pIterator;
                break;
            }
            else
            {
                xSearchedFrame = (*pIterator)->findFrame( sName, FrameSearchFlag::CHILDREN );
                if( xSearchedFrame.is() == sal_True )
                {
                    break;
                }
            }
        }
    }
    return xSearchedFrame;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::searchFlatDown( const OUString& sName ) const
{
    // Check incoming parameter.
    LOG_ASSERT2( implcp_searchFlatDown( sName ), "FrameContainer::searchFlatDown()", "Invalid parameter detected!" )

    // Use read lock to make it threadsafe.
    // Declare default return value for refused calls.
    Reference< XFrame > xSearchedFrame                  ;
    ERefusalReason      eReason                         ;
    ReadGuard           aReadLock( m_aLock, eReason )   ;
    if( eReason == E_NOREASON )
    {
        // Step over all direct child frames first.
        // Even right frame wasn't found, start search at children of direct children.
        for( TConstFrameIterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
        {
            if( (*pIterator)->getName() == sName )
            {
                xSearchedFrame = *pIterator;
                break;
            }
        }

        if( xSearchedFrame.is() == sal_False )
        {
            for( pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
            {
                xSearchedFrame = (*pIterator)->findFrame( sName, FrameSearchFlag::CHILDREN | FrameSearchFlag::SIBLINGS );
                if( xSearchedFrame.is() == sal_True )
                {
                    break;
                }
            }
        }
    }
    return xSearchedFrame;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::searchDirectChildren( const OUString& sName ) const
{
    // Check incoming parameter.
    LOG_ASSERT2( implcp_searchDirectChildren( sName ), "FrameContainer::searchDirectChildren()", "Invalid parameter detected!" )

    // Use read lock to make it threadsafe.
    // Declare default return value for refused calls.
    Reference< XFrame > xSearchedFrame                  ;
    ERefusalReason      eReason                         ;
    ReadGuard           aReadLock( m_aLock, eReason )   ;
    if( eReason == E_NOREASON )
    {
        // Step over all current container items and search for right target.
        for( TConstFrameIterator pIterator=m_aContainer.begin(); pIterator!=m_aContainer.end(); ++pIterator )
        {
            if( (*pIterator)->getName() == sName )
            {
                xSearchedFrame = *pIterator;
                break;
            }
        }
    }
    return xSearchedFrame;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void FrameContainer::impl_clear()
{
    // Clear the container ...
    m_aContainer.erase( m_aContainer.begin(), m_aContainer.end() );
    m_aContainer.clear();
    // ... and don't forget to reset the active frame.
    // Its an reference to a valid container-item.
    // But no container item => no active frame!
    m_xActiveFrame = Reference< XFrame >();
    // If special quit timer is used - we must terminate the desktop.
    // He is the owner of this container and can't work without any visible tasks/frames!
    if( m_rQuitTimer.isValid() == sal_True )
    {
        m_rQuitTimer->start();
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void FrameContainer::impl_disableQuitTimer()
{
    if( m_rQuitTimer.isValid() == sal_True )
    {
        m_rQuitTimer.unbind();
    }
}

}       //  namespace framework
