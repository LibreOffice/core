/*************************************************************************
 *
 *  $RCSfile: fmcontrolbordermanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:50:06 $
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

#ifndef SVX_SOURCE_FORM_FMCONTROLBORDERMANAGER_HXX
#include "fmcontrolbordermanager.hxx"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;

    //====================================================================
    //= ControlBorderManager
    //====================================================================
    //--------------------------------------------------------------------
    ControlBorderManager::ControlBorderManager()
        :m_bEnabled( false )
        ,m_nFocusColor    ( 0x000000FF )
        ,m_nMouseHoveColor( 0x007098BE )
        ,m_nCombinedColor ( 0x000000FF )
    {
    }

    //--------------------------------------------------------------------
    ControlBorderManager::~ControlBorderManager()
    {
    }

    //--------------------------------------------------------------------
    bool ControlBorderManager::canColorBorder( const Reference< XVclWindowPeer >& _rxPeer )
    {
        OSL_PRECOND( _rxPeer.is(), "ControlBorderManager::canColorBorder: invalid peer!" );
        sal_Int16 nBorderStyle = VisualEffect::NONE;
        OSL_VERIFY( _rxPeer->getProperty( FM_PROP_BORDER ) >>= nBorderStyle );
        return ( nBorderStyle == VisualEffect::FLAT );
    }

    //--------------------------------------------------------------------
    ControlStatus ControlBorderManager::getControlStatus( const Reference< XControl >& _rxControl ) SAL_THROW(())
    {
        ControlStatus nStatus = CONTROL_STATUS_NONE;
        if ( _rxControl.get() == m_aFocusControl.xControl.get() )
            nStatus |= CONTROL_STATUS_FOCUSED;
        if ( _rxControl.get() == m_aMouseHoverControl.xControl.get() )
            nStatus |= CONTROL_STATUS_MOUSE_HOVER;
        return nStatus;
    }

    //--------------------------------------------------------------------
    sal_Int32 ControlBorderManager::getControlColorByStatus( ControlStatus _nStatus )
    {
        switch ( _nStatus )
        {
        case CONTROL_STATUS_FOCUSED:
            return m_nFocusColor;
        case CONTROL_STATUS_BOTH:
            return m_nCombinedColor;
        case CONTROL_STATUS_MOUSE_HOVER:
            return m_nMouseHoveColor;
        }
        OSL_ENSURE( sal_False, "ControlBorderManager::getControlColorByStatus: invalid status!" );
        return 0x00000000;
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::updateControlColor( const Reference< XControl >& _rxControl, const Reference< XVclWindowPeer >& _rxPeer, sal_Int32 _nFallbackColor ) SAL_THROW(())
    {
        OSL_PRECOND( _rxControl.is() && _rxPeer.is(), "ControlBorderManager::updateControlColor: invalid parameters!" );

        ControlStatus nStatus = getControlStatus( _rxControl );
        sal_Int32 nRestoreColor =   ( nStatus == CONTROL_STATUS_NONE )
                                ?   _nFallbackColor
                                :   getControlColorByStatus( nStatus );
        _rxPeer->setProperty( FM_PROP_BORDERCOLOR, makeAny( nRestoreColor ) );
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::controlStatusGained( const Reference< XInterface >& _rxControl, ControlData& _rControlData, const ControlData& _rOppositeStatusData ) SAL_THROW(())
    {
        if ( _rxControl == _rControlData.xControl )
            // nothing to do - though suspicious
            return;

        Reference< XControl > xAsControl( _rxControl, UNO_QUERY );
        DBG_ASSERT( xAsControl.is(), "ControlBorderManager::controlStatusGained: invalid control!" );
        if ( !xAsControl.is() )
            return;

        try
        {
            Reference< XVclWindowPeer > xPeer( xAsControl->getPeer(), UNO_QUERY );
            if ( xPeer.is() && canColorBorder( xPeer ) )
            {
                // remember the control and it's current border color
                _rControlData.xControl = xAsControl;
                if ( _rControlData.xControl == _rOppositeStatusData.xControl )
                {
                    _rControlData.nOriginalColor = _rOppositeStatusData.nOriginalColor;
                }
                else
                {
                    OSL_VERIFY( xPeer->getProperty( FM_PROP_BORDERCOLOR ) >>= _rControlData.nOriginalColor );
                }

                updateControlColor( xAsControl, xPeer, _rControlData.nOriginalColor );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ControlBorderManager::controlStatusGained: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::controlStatusLost( const Reference< XInterface >& _rxControl, ControlData& _rControlData ) SAL_THROW(())
    {
        if ( _rxControl != _rControlData.xControl )
            // nothing to do
            return;

        OSL_PRECOND( _rControlData.xControl.is(), "ControlBorderManager::controlStatusLost: invalid control data - this will crash!" );
        try
        {
            Reference< XVclWindowPeer > xPeer( _rControlData.xControl->getPeer(), UNO_QUERY );
            if ( xPeer.is() && canColorBorder( xPeer ) )
            {
                ControlData aPreviousStatus( _rControlData );
                _rControlData.clear();
                updateControlColor( aPreviousStatus.xControl, xPeer, aPreviousStatus.nOriginalColor );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ControlBorderManager::controlStatusLost: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::enableDynamicBorderColor( )
    {
        m_bEnabled = true;
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::disableDynamicBorderColor( )
    {
        m_bEnabled = false;
        restoreAll();
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::setStatusColor( ControlStatus _nStatus, sal_Int32 _nColor )
    {
        switch ( _nStatus )
        {
        case CONTROL_STATUS_FOCUSED:
            m_nFocusColor = _nColor;
            break;
        case CONTROL_STATUS_BOTH:
            m_nCombinedColor = _nColor;
            break;
        case CONTROL_STATUS_MOUSE_HOVER:
            m_nMouseHoveColor = _nColor;
            break;
        default:
            OSL_ENSURE( sal_False, "ControlBorderManager::setStatusColor: invalid status!" );
        }
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::restoreAll()
    {
        if ( m_aFocusControl.xControl.is() )
            controlStatusLost( m_aFocusControl.xControl, m_aFocusControl );
        if ( m_aMouseHoverControl.xControl.is() )
            controlStatusLost( m_aMouseHoverControl.xControl, m_aMouseHoverControl );
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::focusGained( const Reference< XInterface >& _rxControl ) SAL_THROW(())
    {
        if ( m_bEnabled )
            controlStatusGained( _rxControl, m_aFocusControl, m_aMouseHoverControl );
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::focusLost( const Reference< XInterface >& _rxControl ) SAL_THROW(())
    {
        if ( m_bEnabled )
            controlStatusLost( _rxControl, m_aFocusControl );
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::mouseEntered( const Reference< XInterface >& _rxControl ) SAL_THROW(())
    {
        if ( m_bEnabled )
            controlStatusGained( _rxControl, m_aMouseHoverControl, m_aFocusControl );
    }

    //--------------------------------------------------------------------
    void ControlBorderManager::mouseExited( const Reference< XInterface >& _rxControl ) SAL_THROW(())
    {
        if ( m_bEnabled )
            controlStatusLost( _rxControl, m_aMouseHoverControl );
    }


//........................................................................
} // namespace svxform
//........................................................................

