/*************************************************************************
 *
 *  $RCSfile: framecontainer.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: as $ $Date: 2001-03-05 12:55:44 $
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

#include <vector>
#include <stdexcept>
#include <algorithm>
//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::std                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::frame     ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define LOCK_OFF                            sal_False
#define LOCK_ON                             sal_True

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
        :   m_bLock( LOCK_OFF )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
FrameContainer::~FrameContainer()
{
    // Don't forget to free memory!
    disableQuitTimer();
    clear();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::append( const Reference< XFrame >& xFrame )
{
    // Safe impossible cases
    // This method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_append( xFrame ), "FrameContainer::append()\nInvalid parameter detected!\n" )
    // Warn programmer at already existing elements in container.
    LOG_ASSERT( !(exist(xFrame)==sal_True), "FrameContainer::append()\nNew frame already exist in container!\n" )
    // We search for hidden tasks which have no component!
    LOG_ASSERT( impldbg_lookForZombieFrames( xFrame ), "FrameContainer::append()\nZombie frame detected!\n" )

    // Work only, if container not locked!
    if ( m_bLock == LOCK_OFF )
    {
        // Append new frame to container.
        m_aContainer.push_back( xFrame );
    }
    // Else; Warn programmer.
    LOG_ASSERT( !(m_bLock==LOCK_ON), "FrameContainer::append()\nContainer is locked! You can't append frame.\n" )
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::remove( const Reference< XFrame >& xFrame )
{
    // Safe impossible cases
    // This method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_remove( xFrame ), "FrameContainer::remove()\nInvalid parameter detected!\n" )
    // Warn programmer at non existing elements in container.
    LOG_ASSERT( !(exist(xFrame)==sal_False), "FrameContainer::remove()\nFrame to remove not exist in container!\n" )
    // Safe impossible cases.
//  LOG_ASSERT( impldbg_lookForZombieFrames( xFrame ), "FrameContainer::remove()\nZombie frame detected!\n" )

    // Work only, if container not locked!
    if ( m_bLock == LOCK_OFF )
    {
        // Search frame and remove it from container ...
        vector< Reference< XFrame > >::iterator aSearchedItem = find( m_aContainer.begin(), m_aContainer.end(), xFrame );
        // ... if it exist.
        if ( aSearchedItem != m_aContainer.end() )
        {
            m_aContainer.erase( aSearchedItem );
            // And if removed frame the current active frame - reset state variable.
            if ( m_xActiveFrame == xFrame )
            {
                m_xActiveFrame = Reference< XFrame >();
            }
            // If last frame was removed and special quit timer is enabled by the desktop
            // we must terminate the desktop by using this timer!
            if  (
                    ( getCount()                <   1           )   &&
                    ( m_rQuitTimer.isValid()    ==  sal_True    )
                )
            {
                m_rQuitTimer->start();
            }
        }
    }
    // Else; Warn programmer.
    LOG_ASSERT( !(m_bLock==LOCK_ON), "FrameContainer::remove()\nContainer is locked! You can't remove frame.\n" )
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool FrameContainer::exist( const REFERENCE< XFRAME >& xFrame )
{
    // Safe impossible cases
    // This method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_exist( xFrame ), "FrameContainer::exist()\nInvalid parameter detected!\n" )

    // Set default return value.
    sal_Bool bExist = sal_False;

    // We ignore the lock, because we do not change the content of container!
    // Search frame.
    vector< Reference< XFrame > >::iterator aSearchedItem = find( m_aContainer.begin(), m_aContainer.end(), xFrame );
    // If it exist ...
    if ( aSearchedItem != m_aContainer.end() )
    {
        // Set new return value.
        bExist = sal_True;
    }
    // Return result of this operation.
    return bExist;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::clear()
{
    // Safe impossible cases.
    // We search for hidden tasks which have no component!
    LOG_ASSERT( impldbg_lookForZombieFrames( Reference< XFrame >() ), "FrameContainer::clear()\nZombie frame detected!\n" )

    // This method is only allowed, if no lock is set!
    // Warn programmer, if its not true.
    LOG_ASSERT( !(m_bLock==LOCK_ON), "FrameContainer::clear()\nContainer is locked! You can't clear it.\n" )
    if ( m_bLock == LOCK_OFF )
    {
        // Clear the container ...
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
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::lock()
{
    // Lock the container and block all append(), remove() and clear() calls.
    m_bLock = LOCK_ON;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::unlock()
{
    // Unlock the container and block all getCount() and operator[] calls.
    m_bLock = LOCK_OFF;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt32 FrameContainer::getCount() const
{
    // Return size of current container.
    // We ignore the lock, because you can ask for count of container elements ...
    // but if you will have full index access you must lock it!
    return (sal_uInt32)m_aContainer.size();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::operator[]( sal_uInt32 nIndex ) const
{
    // Safe impossible cases
    // This method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_IndexOperator( nIndex ), "FrameContainer::operator[]()\nInvalid parameter detected!\n" )

    // Set default return value.
    Reference< XFrame > xFrame;

    // This operation is allowed only, if lock is set.
    // Warn programmer, if this not true.
    LOG_ASSERT( !(m_bLock==LOCK_OFF), "FrameContainer::operator[]()\nContainer is not locked! You can't do this.\n" )

    if ( m_bLock == LOCK_ON )
    {
        try
        {
            // Get element form container WITH automatic test of ranges!
            // If index not valid, a out_of_range exception is thrown.
            xFrame = m_aContainer.at( nIndex );
        }
        catch( std::out_of_range& )
        {
            // The index is not valid for current container-content ...
            // ... but we must handle this case!
            // We return a NULL reference.
            xFrame = Reference< XFrame >();
        }
    }

    // Return result of this operation.
    return xFrame;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< Reference< XFrame > > FrameContainer::getAllElements()
{
    // Lock the container for this method.
    // Nobody should append or remove anything between creation of snapshot.
    // But - don't forget to unlock the container.
    m_bLock = LOCK_ON;

    // Get memory for return sequence.
    sal_uInt32 nCount = (sal_uInt32)m_aContainer.size();
    Sequence< Reference< XFrame > > seqReturn( nCount );

    // Copy from container to return list.
    Reference< XFrame >* pArray = seqReturn.getArray();
    for ( sal_uInt32 nPosition=0; nPosition<nCount; ++nPosition )
    {
        pArray[nPosition] = m_aContainer[nPosition];
    }

    // Don't forget this!
    m_bLock = LOCK_OFF;

    // Return result of this operation.
    return seqReturn;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool FrameContainer::hasElements() const
{
    // Has container some elements?
    return ( m_aContainer.size() > 0 );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::setActive( const Reference< XFrame >& xFrame )
{
    // Safe impossible cases
    // This method is not defined for ALL incoming parameters!
    // BUT we accept null refrences for reset active state. => No frame is active then.
    LOG_ASSERT( impldbg_checkParameter_setActive( xFrame ), "FrameContainer::setActive()\nInvalid parameter detected!\n" )
    // The new active frame MUST exist in container.
    // Control this.
    LOG_ASSERT( !(xFrame.is()==sal_True && exist(xFrame)==sal_False), "FrameContainer::setActive()\nThe new active frame is not a member of current container!You cant activate it.\n" )

    // All incoming parameters are controlled.
    // We have a new active frame or a null reference to reset this state.
    // Actualize member.
    m_xActiveFrame = xFrame;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > FrameContainer::getActive() const
{
    // Return member.
    // The correct state of this variable we have controlled in setActive()!
    // But we accept null reference for these variable. => There is no active frame in the moment.
    return m_xActiveFrame;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::enableQuitTimer( const Reference< XDesktop >& xDesktop )
{
    // If no current timer exist - create a new one.
    if( m_rQuitTimer.isEmpty() == sal_True )
    {
        m_rQuitTimer.bind( new AsyncQuit( xDesktop ) );
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void FrameContainer::disableQuitTimer()
{
    // Delete current quit timer.
    // If user wish to create it again he must do it with "enableQuitTimer()".
    if( m_rQuitTimer.isValid() == sal_True )
    {
        m_rQuitTimer.unbind();
    }
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
// We accept valid references for working with container only.
sal_Bool FrameContainer::impldbg_checkParameter_append( const Reference< XFrame >& xFrame ) const
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFrame       ==  NULL        )   ||
            ( xFrame.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We accept valid references for working with container only.
sal_Bool FrameContainer::impldbg_checkParameter_remove( const Reference< XFrame >& xFrame ) const
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFrame       ==  NULL        )   ||
            ( xFrame.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We accept valid references for working with container only.
sal_Bool FrameContainer::impldbg_checkParameter_exist( const Reference< XFrame >& xFrame ) const
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFrame       ==  NULL        )   ||
            ( xFrame.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// The index in to the container must be in range of 0 and count-1!
sal_Bool FrameContainer::impldbg_checkParameter_IndexOperator( sal_uInt32 nIndex ) const
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( nIndex >= (sal_uInt32)m_aContainer.size() )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// setActive accept valid- or null-reference but no null-pointer!
sal_Bool FrameContainer::impldbg_checkParameter_setActive( const Reference< XFrame >& xFrame ) const
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFrame == NULL )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
sal_Bool FrameContainer::impldbg_lookForZombieFrames( const Reference< XFrame >& xFrame ) const
{
    // Step over all container items and search for a frame which contains no component.
    // (He has no component window!)

    // Use return value directly for an assert ...
    sal_Bool bSuppressAssert = sal_True;

    sal_Int32 nCount = m_aContainer.size();
    for( sal_Int32 nPosition=0; nPosition<nCount; ++nPosition )
    {
        Reference< XFrame > xZombieFrame = m_aContainer.at(nPosition);
        if  (
                (   xZombieFrame.is()                       ==  sal_True    )   &&
                (   xZombieFrame                            !=  xFrame      )   &&
                (   xZombieFrame->getComponentWindow().is() ==  sal_False   )
            )
        {
            bSuppressAssert = sal_False;
        }
    }

    return bSuppressAssert;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
