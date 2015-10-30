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


#include "fmcontrolbordermanager.hxx"

#include "fmprop.hrc"

#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <tools/debug.hxx>
#include <osl/diagnose.h>


namespace svxform
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::form::validation;


    //= helper


    static void setUnderline( const Reference< XVclWindowPeer >& _rxPeer, const UnderlineDescriptor& _rUnderline )
    {
        OSL_ENSURE( _rxPeer.is(), "setUnderline: invalid peer!" );

        // the underline type is an aspect of the font
        FontDescriptor aFont;
        OSL_VERIFY( _rxPeer->getProperty( FM_PROP_FONT ) >>= aFont );
        aFont.Underline = _rUnderline.nUnderlineType;
        _rxPeer->setProperty( FM_PROP_FONT, makeAny( aFont ) );
        // the underline color is a separate property
        _rxPeer->setProperty( FM_PROP_TEXTLINECOLOR, makeAny( _rUnderline.nUnderlineColor ) );
    }


    static void getUnderline( const Reference< XVclWindowPeer >& _rxPeer, UnderlineDescriptor& _rUnderline )
    {
        OSL_ENSURE( _rxPeer.is(), "getUnderline: invalid peer!" );

        FontDescriptor aFont;
        OSL_VERIFY( _rxPeer->getProperty( FM_PROP_FONT ) >>= aFont );
        _rUnderline.nUnderlineType = aFont.Underline;

        OSL_VERIFY( _rxPeer->getProperty( FM_PROP_TEXTLINECOLOR ) >>= _rUnderline.nUnderlineColor );
    }


    static void getBorder( const Reference< XVclWindowPeer >& _rxPeer, BorderDescriptor& _rBoder )
    {
        OSL_ENSURE( _rxPeer.is(), "getBorder: invalid peer!" );

        OSL_VERIFY( _rxPeer->getProperty( FM_PROP_BORDER ) >>= _rBoder.nBorderType );
        OSL_VERIFY( _rxPeer->getProperty( FM_PROP_BORDERCOLOR ) >>= _rBoder.nBorderColor );
    }


    static void setBorder( const Reference< XVclWindowPeer >& _rxPeer, const BorderDescriptor& _rBoder )
    {
        OSL_ENSURE( _rxPeer.is(), "setBorder: invalid peer!" );

        _rxPeer->setProperty( FM_PROP_BORDER, makeAny( _rBoder.nBorderType ) );
        _rxPeer->setProperty( FM_PROP_BORDERCOLOR, makeAny( _rBoder.nBorderColor ) );
    }

    ControlBorderManager::ControlBorderManager()
        :m_nFocusColor    ( 0x000000FF )
        ,m_nMouseHoveColor( 0x007098BE )
        ,m_nInvalidColor  ( 0x00FF0000 )
        ,m_bDynamicBorderColors( false )
    {
    }


    ControlBorderManager::~ControlBorderManager()
    {
    }


    bool ControlBorderManager::canColorBorder( const Reference< XVclWindowPeer >& _rxPeer )
    {
        OSL_PRECOND( _rxPeer.is(), "ControlBorderManager::canColorBorder: invalid peer!" );

        PeerBag::const_iterator aPos = m_aColorableControls.find( _rxPeer );
        if ( aPos != m_aColorableControls.end() )
            return true;

        aPos = m_aNonColorableControls.find( _rxPeer );
        if ( aPos != m_aNonColorableControls.end() )
            return false;

        // this peer is not yet known

        // no border coloring for controls which are not for text input
        // #i37434# / 2004-11-19 / frank.schoenheit@sun.com
        Reference< XTextComponent > xText( _rxPeer, UNO_QUERY );
        Reference< XListBox > xListBox( _rxPeer, UNO_QUERY );
        if ( xText.is() || xListBox.is() )
        {
            sal_Int16 nBorderStyle = VisualEffect::NONE;
            OSL_VERIFY( _rxPeer->getProperty( FM_PROP_BORDER ) >>= nBorderStyle );
            if ( nBorderStyle == VisualEffect::FLAT )
                // if you change this to also accept LOOK3D, then this would also work, but look ugly
            {
                m_aColorableControls.insert( _rxPeer );
                return true;
            }
        }

        m_aNonColorableControls.insert( _rxPeer );
        return false;
    }


    ControlStatus ControlBorderManager::getControlStatus( const Reference< XControl >& _rxControl )
    {
        ControlStatus nStatus = CONTROL_STATUS_NONE;

        if ( _rxControl.get() == m_aFocusControl.xControl.get() )
            nStatus |= CONTROL_STATUS_FOCUSED;

        if ( _rxControl.get() == m_aMouseHoverControl.xControl.get() )
            nStatus |= CONTROL_STATUS_MOUSE_HOVER;

        if ( m_aInvalidControls.find( ControlData( _rxControl ) ) != m_aInvalidControls.end() )
            nStatus |= CONTROL_STATUS_INVALID;

        return nStatus;
    }


    sal_Int32 ControlBorderManager::getControlColorByStatus( ControlStatus _nStatus )
    {
        // "invalid" is ranked highest
        if ( _nStatus & CONTROL_STATUS_INVALID )
            return m_nInvalidColor;

        // then, "focused" is more important than ...
        if ( _nStatus & CONTROL_STATUS_FOCUSED )
            return m_nFocusColor;

        // ... "mouse over"
        if ( _nStatus & CONTROL_STATUS_MOUSE_HOVER )
            return m_nMouseHoveColor;

        OSL_FAIL( "ControlBorderManager::getControlColorByStatus: invalid status!" );
        return 0x00000000;
    }


    void ControlBorderManager::updateBorderStyle( const Reference< XControl >& _rxControl, const Reference< XVclWindowPeer >& _rxPeer, const BorderDescriptor& _rFallback )
    {
        OSL_PRECOND( _rxControl.is() && _rxPeer.is(), "ControlBorderManager::updateBorderStyle: invalid parameters!" );

        ControlStatus nStatus = getControlStatus( _rxControl );
        BorderDescriptor aBorder;
        aBorder.nBorderType =   ( nStatus == CONTROL_STATUS_NONE )
                            ?   _rFallback.nBorderType
                            :   VisualEffect::FLAT;
        aBorder.nBorderColor =   ( nStatus == CONTROL_STATUS_NONE )
                             ?   _rFallback.nBorderColor
                             :   getControlColorByStatus( nStatus );
        setBorder( _rxPeer, aBorder );
    }


    void ControlBorderManager::determineOriginalBorderStyle( const Reference< XControl >& _rxControl, BorderDescriptor& _rData ) const
    {
        _rData = ControlData();
        if ( m_aFocusControl.xControl.get() == _rxControl.get() )
        {
            _rData = m_aFocusControl;
        }
        else if ( m_aMouseHoverControl.xControl.get() == _rxControl.get() )
        {
            _rData = m_aMouseHoverControl;
        }
        else
        {
            ControlBag::const_iterator aPos = m_aInvalidControls.find( _rxControl );
            if ( aPos != m_aInvalidControls.end() )
            {
                _rData = *aPos;
            }
            else
            {
                Reference< XVclWindowPeer > xPeer( _rxControl->getPeer(), UNO_QUERY );
                getBorder( xPeer, _rData );
            }
        }
    }


    void ControlBorderManager::controlStatusGained( const Reference< XInterface >& _rxControl, ControlData& _rControlData )
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
                // remember the control and its current border color
                _rControlData.xControl.clear(); // so determineOriginalBorderStyle doesn't get confused

                determineOriginalBorderStyle( xAsControl, _rControlData );

                _rControlData.xControl = xAsControl;

                updateBorderStyle( xAsControl, xPeer, _rControlData );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ControlBorderManager::controlStatusGained: caught an exception!" );
        }
    }


    void ControlBorderManager::controlStatusLost( const Reference< XInterface >& _rxControl, ControlData& _rControlData )
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
                _rControlData = ControlData();
                updateBorderStyle( aPreviousStatus.xControl, xPeer, aPreviousStatus );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ControlBorderManager::controlStatusLost: caught an exception!" );
        }
    }


    void ControlBorderManager::enableDynamicBorderColor( )
    {
        m_bDynamicBorderColors = true;
    }


    void ControlBorderManager::disableDynamicBorderColor( )
    {
        m_bDynamicBorderColors = false;
        restoreAll();
    }


    void ControlBorderManager::setStatusColor( ControlStatus _nStatus, sal_Int32 _nColor )
    {
        switch ( _nStatus )
        {
        case CONTROL_STATUS_FOCUSED:
            m_nFocusColor = _nColor;
            break;
        case CONTROL_STATUS_MOUSE_HOVER:
            m_nMouseHoveColor = _nColor;
            break;
        case CONTROL_STATUS_INVALID:
            m_nInvalidColor = _nColor;
            break;
        default:
            OSL_FAIL( "ControlBorderManager::setStatusColor: invalid status!" );
        }
    }


    void ControlBorderManager::restoreAll()
    {
        if ( m_aFocusControl.xControl.is() )
            controlStatusLost( m_aFocusControl.xControl, m_aFocusControl );
        if ( m_aMouseHoverControl.xControl.is() )
            controlStatusLost( m_aMouseHoverControl.xControl, m_aMouseHoverControl );

        ControlBag aInvalidControls;
        m_aInvalidControls.swap( aInvalidControls );

        for ( ControlBag::const_iterator loop = aInvalidControls.begin();
              loop != aInvalidControls.end();
              ++loop
            )
        {
            Reference< XVclWindowPeer > xPeer( loop->xControl->getPeer(), UNO_QUERY );
            if ( xPeer.is() )
            {
                updateBorderStyle( loop->xControl, xPeer, *loop );
                xPeer->setProperty( FM_PROP_HELPTEXT, makeAny( loop->sOriginalHelpText ) );
                setUnderline( xPeer, *loop );
            }
        }
    }


    void ControlBorderManager::focusGained( const Reference< XInterface >& _rxControl )
    {
        if ( m_bDynamicBorderColors )
            controlStatusGained( _rxControl, m_aFocusControl );
    }


    void ControlBorderManager::focusLost( const Reference< XInterface >& _rxControl )
    {
        if ( m_bDynamicBorderColors )
            controlStatusLost( _rxControl, m_aFocusControl );
    }


    void ControlBorderManager::mouseEntered( const Reference< XInterface >& _rxControl )
    {
        if ( m_bDynamicBorderColors )
            controlStatusGained( _rxControl, m_aMouseHoverControl );
    }


    void ControlBorderManager::mouseExited( const Reference< XInterface >& _rxControl )
    {
        if ( m_bDynamicBorderColors )
            controlStatusLost( _rxControl, m_aMouseHoverControl );
    }


    void ControlBorderManager::validityChanged( const Reference< XControl >& _rxControl, const Reference< XValidatableFormComponent >& _rxValidatable )
    {
        try
        {
            OSL_ENSURE( _rxControl.is(), "ControlBorderManager::validityChanged: invalid control!" );
            OSL_ENSURE( _rxValidatable.is(), "ControlBorderManager::validityChanged: invalid validatable!" );

            Reference< XVclWindowPeer > xPeer( _rxControl.is() ? _rxControl->getPeer() : Reference< XWindowPeer >(), UNO_QUERY );
            if ( !xPeer.is() || !_rxValidatable.is() )
                return;

            ControlData aData( _rxControl );

            if ( _rxValidatable->isValid() )
            {
                ControlBag::iterator aPos = m_aInvalidControls.find( aData );
                if ( aPos != m_aInvalidControls.end() )
                {   // invalid before, valid now
                    ControlData aOriginalLayout( *aPos );
                    m_aInvalidControls.erase( aPos );

                    // restore all the things we used to indicate invalidity
                    if ( m_bDynamicBorderColors )
                        updateBorderStyle( _rxControl, xPeer, aOriginalLayout );
                    xPeer->setProperty( FM_PROP_HELPTEXT, makeAny( aOriginalLayout.sOriginalHelpText ) );
                    setUnderline( xPeer, aOriginalLayout );
                }
                return;
            }

            // we're here in the INVALID case
            if ( m_aInvalidControls.find( _rxControl ) == m_aInvalidControls.end() )
            {   // valid before, invalid now

                // remember the current border
                determineOriginalBorderStyle( _rxControl, aData );
                // and tool tip
                xPeer->getProperty( FM_PROP_HELPTEXT ) >>= aData.sOriginalHelpText;
                // and font
                getUnderline( xPeer, aData );

                m_aInvalidControls.insert( aData );

                // update the border to the new invalidity
                if ( m_bDynamicBorderColors && canColorBorder( xPeer ) )
                    updateBorderStyle( _rxControl, xPeer, aData );
                else
                {
                    // and also the new font
                    setUnderline( xPeer, UnderlineDescriptor( css::awt::FontUnderline::WAVE, m_nInvalidColor ) );
                }
            }

            // update the explanation for invalidity (this is always done, even if the validity did not change)
            Reference< XValidator > xValidator = _rxValidatable->getValidator();
            OSL_ENSURE( xValidator.is(), "ControlBorderManager::validityChanged: invalid, but no validator?" );
            OUString sExplainInvalidity = xValidator.is() ? xValidator->explainInvalid( _rxValidatable->getCurrentValue() ) : OUString();
            xPeer->setProperty( FM_PROP_HELPTEXT, makeAny( sExplainInvalidity ) );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ControlBorderManager::validityChanged: caught an exception!" );
        }
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
