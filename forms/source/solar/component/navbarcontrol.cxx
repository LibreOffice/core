/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: navbarcontrol.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "navbarcontrol.hxx"
#include "frm_strings.hxx"
#include "frm_module.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <vcl/svapp.hxx>
#include "navtoolbar.hxx"
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ONavigationBarControl()
{
    static ::frm::OMultiInstanceAutoRegistration< ::frm::ONavigationBarControl > aAutoRegistration;
}

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

#define FORWARD_TO_PEER_1( unoInterface, method, param1 )   \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        xTypedPeer->method( param1 );  \
    }

#define FORWARD_TO_PEER_1_RET( returnType, unoInterface, method, param1 )   \
    returnType aReturn; \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        aReturn = xTypedPeer->method( param1 );  \
    }   \
    return aReturn;

#define FORWARD_TO_PEER_3( unoInterface, method, param1, param2, param3 )   \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        xTypedPeer->method( param1, param2, param3 );  \
    }

#define FORWARD_TO_PEER_3_RET( returnType, unoInterface, method, param1, param2, param3 )   \
    returnType aReturn; \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        aReturn = xTypedPeer->method( param1, param2, param3 );  \
    }   \
    return aReturn;

    //==================================================================
    // ONavigationBarControl
    //==================================================================
    DBG_NAME( ONavigationBarControl )
    //------------------------------------------------------------------
    ONavigationBarControl::ONavigationBarControl( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
    {
        DBG_CTOR( ONavigationBarControl, NULL );
    }

    //------------------------------------------------------------------
    ONavigationBarControl::~ONavigationBarControl()
    {
        DBG_DTOR( ONavigationBarControl, NULL );
    }

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ONavigationBarControl, UnoControl, ONavigationBarControl_Base )

    //------------------------------------------------------------------
    Any SAL_CALL ONavigationBarControl::queryAggregation( const Type& _rType ) throw ( RuntimeException )
    {
        Any aReturn = UnoControl::queryAggregation( _rType );

        if ( !aReturn.hasValue() )
            aReturn = ONavigationBarControl_Base::queryInterface( _rType );

        return aReturn;
    }

    //------------------------------------------------------------------
    namespace
    {
        static WinBits getWinBits( const Reference< XControlModel >& _rxModel )
        {
            WinBits nBits = 0;
            try
            {
                Reference< XPropertySet > xProps( _rxModel, UNO_QUERY );
                if ( xProps.is() )
                {
                    sal_Int16 nBorder = 0;
                    xProps->getPropertyValue( PROPERTY_BORDER ) >>= nBorder;
                    if ( nBorder )
                        nBits |= WB_BORDER;

                    sal_Bool bTabStop = sal_False;
                    if ( xProps->getPropertyValue( PROPERTY_TABSTOP ) >>= bTabStop )
                        nBits |= ( bTabStop ? WB_TABSTOP : WB_NOTABSTOP );
                }
            }
            catch( const Exception& )
            {
                DBG_ERROR( "::getWinBits: caught an exception!" );
            }
            return nBits;
        }
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarControl::createPeer( const Reference< XToolkit >& /*_rToolkit*/, const Reference< XWindowPeer >& _rParentPeer ) throw( RuntimeException )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if (!getPeer().is())
        {
            mbCreatingPeer = sal_True;

            // determine the VLC window for the parent
            Window* pParentWin = NULL;
            if ( _rParentPeer.is() )
            {
                VCLXWindow* pParentXWin = VCLXWindow::GetImplementation( _rParentPeer );
                if ( pParentXWin )
                    pParentWin = pParentXWin->GetWindow();
                DBG_ASSERT( pParentWin, "ONavigationBarControl::createPeer: could not obtain the VCL-level parent window!" );
            }

            // create the peer
            ONavigationBarPeer* pPeer = ONavigationBarPeer::Create( m_xORB, pParentWin, getWinBits( getModel() ) );
            DBG_ASSERT( pPeer, "ONavigationBarControl::createPeer: invalid peer returned!" );
            if ( pPeer )
                // by definition, the returned component is aquired once
                pPeer->release();

            // announce the peer to the base class
            setPeer( pPeer );

            // initialize ourself (and thus the peer) with the model properties
            updateFromModel();

            Reference< XView >  xPeerView( getPeer(), UNO_QUERY );
            if ( xPeerView.is() )
            {
                xPeerView->setZoom( maComponentInfos.nZoomX, maComponentInfos.nZoomY );
                xPeerView->setGraphics( mxGraphics );
            }

            // a lot of initial settings from our component infos
            setPosSize( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight, PosSize::POSSIZE );

            pPeer->setVisible   ( maComponentInfos.bVisible && !mbDesignMode );
            pPeer->setEnable    ( maComponentInfos.bEnable                   );
            pPeer->setDesignMode( mbDesignMode                               );

            peerCreated();

            mbCreatingPeer = sal_False;
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ONavigationBarControl::getImplementationName()  throw( RuntimeException )
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ONavigationBarControl::getSupportedServiceNames()  throw( RuntimeException )
    {
        return getSupportedServiceNames_Static();
    }

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ONavigationBarControl::getImplementationName_Static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.form.ONavigationBarControl" ) );
    }

    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ONavigationBarControl::getSupportedServiceNames_Static()
    {
        Sequence< ::rtl::OUString > aServices( 2 );
        aServices[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControl" ) );
        aServices[ 1 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.control.NavigationToolBar" ) );
        return aServices;
    }

    //------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ONavigationBarControl::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new ONavigationBarControl( _rxFactory ) );
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarControl::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        FORWARD_TO_PEER_1( XDispatchProviderInterception, registerDispatchProviderInterceptor, _rxInterceptor );
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarControl::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        FORWARD_TO_PEER_1( XDispatchProviderInterception, releaseDispatchProviderInterceptor, _rxInterceptor );
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarControl::setDesignMode( sal_Bool _bOn ) throw( RuntimeException )
    {
        UnoControl::setDesignMode( _bOn );
        FORWARD_TO_PEER_1( XVclWindowPeer, setDesignMode, _bOn );
    }

    //==================================================================
    // ONavigationBarPeer
    //==================================================================
    DBG_NAME( ONavigationBarPeer )
    //------------------------------------------------------------------
    ONavigationBarPeer* ONavigationBarPeer::Create( const Reference< XMultiServiceFactory >& _rxORB,
        Window* _pParentWindow, WinBits _nStyle )
    {
        DBG_TESTSOLARMUTEX();

        // the peer itself
        ONavigationBarPeer* pPeer = new ONavigationBarPeer( _rxORB );
        pPeer->acquire();   // by definition, the returned object is aquired once

        // the VCL control for the peer
        NavigationToolBar* pNavBar = new NavigationToolBar( _pParentWindow, _nStyle );

        // some knittings
        pNavBar->setDispatcher( pPeer );
        pNavBar->SetComponentInterface( pPeer );

        // we want a faster repeating rate for the slots in this
        // toolbox
        AllSettings aSettings = pNavBar->GetSettings();
        MouseSettings aMouseSettings = aSettings.GetMouseSettings();
        aMouseSettings.SetButtonRepeat( 10 );
        aSettings.SetMouseSettings( aMouseSettings );
        pNavBar->SetSettings( aSettings, TRUE );

        // outta here
        return pPeer;
    }

    //------------------------------------------------------------------
    ONavigationBarPeer::ONavigationBarPeer( const Reference< XMultiServiceFactory >& _rxORB )
        :OFormNavigationHelper( _rxORB )
    {
        DBG_CTOR( ONavigationBarPeer, NULL );
    }

    //------------------------------------------------------------------
    ONavigationBarPeer::~ONavigationBarPeer()
    {
        DBG_DTOR( ONavigationBarPeer, NULL );
    }

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( ONavigationBarPeer, VCLXWindow, OFormNavigationHelper )

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ONavigationBarPeer, VCLXWindow, OFormNavigationHelper )

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarPeer::dispose(  ) throw( RuntimeException )
    {
        VCLXWindow::dispose();
        OFormNavigationHelper::dispose();
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarPeer::setProperty( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw( RuntimeException )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        NavigationToolBar* pNavBar = static_cast< NavigationToolBar* >( GetWindow() );
        if ( !pNavBar )
        {
            VCLXWindow::setProperty( _rPropertyName, _rValue );
            return;
        }

        bool bVoid = !_rValue.hasValue();

        sal_Bool  bBoolValue = sal_False;
        sal_Int32 nColor = COL_TRANSPARENT;

        // TODO: more generic mechanisms for this (the grid control implementation,
        // when used herein, will do the same stuff for lot of these)

        if ( _rPropertyName.equals( PROPERTY_BACKGROUNDCOLOR ) )
        {
            Wallpaper aTest = pNavBar->GetBackground();
            if ( bVoid )
            {
                pNavBar->SetBackground( pNavBar->GetSettings().GetStyleSettings().GetFaceColor() );
                pNavBar->SetControlBackground();
            }
            else
            {
                OSL_VERIFY( _rValue >>= nColor );
                Color aColor( nColor );
                pNavBar->SetBackground( aColor );
                pNavBar->SetControlBackground( aColor );
            }
        }
        else if ( _rPropertyName.equals( PROPERTY_TEXTLINECOLOR ) )
        {
            if ( bVoid )
            {
                pNavBar->SetTextLineColor();
            }
            else
            {
                OSL_VERIFY( _rValue >>= nColor );
                Color aColor( nColor );
                pNavBar->SetTextLineColor( nColor );
            }
        }
        else if ( _rPropertyName.equals( PROPERTY_ICONSIZE ) )
        {
            sal_Int16 nInt16Value = 0;
            OSL_VERIFY( _rValue >>= nInt16Value );
            pNavBar->SetImageSize( nInt16Value ? NavigationToolBar::eLarge : NavigationToolBar::eSmall );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_POSITION ) )
        {
            OSL_VERIFY( _rValue >>= bBoolValue );
            pNavBar->ShowFunctionGroup( NavigationToolBar::ePosition, bBoolValue );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_NAVIGATION ) )
        {
            OSL_VERIFY( _rValue >>= bBoolValue );
            pNavBar->ShowFunctionGroup( NavigationToolBar::eNavigation, bBoolValue );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_RECORDACTIONS ) )
        {
            OSL_VERIFY( _rValue >>= bBoolValue );
            pNavBar->ShowFunctionGroup( NavigationToolBar::eRecordActions, bBoolValue );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_FILTERSORT ) )
        {
            OSL_VERIFY( _rValue >>= bBoolValue );
            pNavBar->ShowFunctionGroup( NavigationToolBar::eFilterSort, bBoolValue );
        }
        else
        {
            VCLXWindow::setProperty( _rPropertyName, _rValue );
        }
    }

    //------------------------------------------------------------------
    Any SAL_CALL ONavigationBarPeer::getProperty( const ::rtl::OUString& _rPropertyName ) throw( RuntimeException )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        Any aReturn;
        NavigationToolBar* pNavBar = static_cast< NavigationToolBar* >( GetWindow() );

        if ( _rPropertyName.equals( PROPERTY_BACKGROUNDCOLOR ) )
        {
            aReturn <<= (sal_Int32)pNavBar->GetControlBackground().GetColor();
        }
        else if ( _rPropertyName.equals( PROPERTY_TEXTLINECOLOR ) )
        {
            aReturn <<= (sal_Int32)pNavBar->GetTextLineColor().GetColor();
        }
        else if ( _rPropertyName.equals( PROPERTY_ICONSIZE ) )
        {
            sal_Int16 nIconType = ( NavigationToolBar::eLarge == pNavBar->GetImageSize() )
                                ? 1 : 0;
            aReturn <<= nIconType;
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_POSITION ) )
        {
            aReturn <<= (sal_Bool)( pNavBar->IsFunctionGroupVisible( NavigationToolBar::ePosition ) );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_NAVIGATION ) )
        {
            aReturn <<= (sal_Bool)( pNavBar->IsFunctionGroupVisible( NavigationToolBar::eNavigation ) );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_RECORDACTIONS ) )
        {
            aReturn <<= (sal_Bool)( pNavBar->IsFunctionGroupVisible( NavigationToolBar::eRecordActions ) );
        }
        else if ( _rPropertyName.equals( PROPERTY_SHOW_FILTERSORT ) )
        {
            aReturn <<= (sal_Bool)( pNavBar->IsFunctionGroupVisible( NavigationToolBar::eFilterSort ) );
        }
        else
            aReturn = VCLXWindow::getProperty( _rPropertyName );

        return aReturn;
    }

    //------------------------------------------------------------------
    void ONavigationBarPeer::interceptorsChanged( )
    {
        if ( isDesignMode() )
            // not interested in if we're in design mode
            return;

        OFormNavigationHelper::interceptorsChanged();
    }

    //------------------------------------------------------------------
    void ONavigationBarPeer::featureStateChanged( sal_Int32 _nFeatureId, sal_Bool _bEnabled )
    {
        // enable this button on the toolbox
        NavigationToolBar* pNavBar = static_cast< NavigationToolBar* >( GetWindow() );
        if ( pNavBar )
        {
            pNavBar->enableFeature( _nFeatureId, _bEnabled );

            // is it a feature with additional state information?
            if ( _nFeatureId == SID_FM_FORM_FILTERED )
            {   // additional boolean state
                pNavBar->checkFeature( _nFeatureId, getBooleanState( _nFeatureId ) );
            }
            else if ( _nFeatureId == SID_FM_RECORD_TOTAL )
            {
                pNavBar->setFeatureText( _nFeatureId, getStringState( _nFeatureId ) );
            }
            else if ( _nFeatureId == SID_FM_RECORD_ABSOLUTE )
            {
                pNavBar->setFeatureText( _nFeatureId, String::CreateFromInt32( getIntegerState( _nFeatureId ) ) );
            }
        }

        // base class
        OFormNavigationHelper::featureStateChanged( _nFeatureId, _bEnabled );
    }

    //------------------------------------------------------------------
    void ONavigationBarPeer::allFeatureStatesChanged( )
    {
        // force the control to update it's states
        NavigationToolBar* pNavBar = static_cast< NavigationToolBar* >( GetWindow() );
        if ( pNavBar )
            pNavBar->setDispatcher( this );

        // base class
        OFormNavigationHelper::allFeatureStatesChanged( );
    }

    //------------------------------------------------------------------
    bool ONavigationBarPeer::isEnabled( sal_Int32 _nFeatureId ) const
    {
        if ( const_cast< ONavigationBarPeer* >( this )->isDesignMode() )
           return false;

        return OFormNavigationHelper::isEnabled( _nFeatureId );
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarPeer::setDesignMode( sal_Bool _bOn ) throw( RuntimeException )
    {
        VCLXWindow::setDesignMode( _bOn  );

        if ( _bOn )
            disconnectDispatchers();
        else
            connectDispatchers();
            // this will connect if not already connected and just update else
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarPeer::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        VCLXWindow::disposing( _rSource );
        OFormNavigationHelper::disposing( _rSource );
    }

    //------------------------------------------------------------------
    void ONavigationBarPeer::getSupportedFeatures( ::std::vector< sal_Int32 >& _rFeatureIds )
    {
        _rFeatureIds.push_back( SID_FM_RECORD_ABSOLUTE );
        _rFeatureIds.push_back( SID_FM_RECORD_TOTAL );
        _rFeatureIds.push_back( SID_FM_RECORD_FIRST );
        _rFeatureIds.push_back( SID_FM_RECORD_PREV );
        _rFeatureIds.push_back( SID_FM_RECORD_NEXT );
        _rFeatureIds.push_back( SID_FM_RECORD_LAST );
        _rFeatureIds.push_back( SID_FM_RECORD_SAVE );
        _rFeatureIds.push_back( SID_FM_RECORD_UNDO );
        _rFeatureIds.push_back( SID_FM_RECORD_NEW );
        _rFeatureIds.push_back( SID_FM_RECORD_DELETE );
        _rFeatureIds.push_back( SID_FM_REFRESH );
        _rFeatureIds.push_back( SID_FM_SORTUP );
        _rFeatureIds.push_back( SID_FM_SORTDOWN );
        _rFeatureIds.push_back( SID_FM_ORDERCRIT );
        _rFeatureIds.push_back( SID_FM_AUTOFILTER );
        _rFeatureIds.push_back( SID_FM_FILTERCRIT );
        _rFeatureIds.push_back( SID_FM_FORM_FILTERED );
        _rFeatureIds.push_back( SID_FM_REMOVE_FILTER_SORT );
    }

//.........................................................................
}   // namespace frm
//.........................................................................
