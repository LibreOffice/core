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


#include "navbarcontrol.hxx"
#include "frm_strings.hxx"
#include "frm_module.hxx"
#include "FormComponent.hxx"
#include "componenttools.hxx"
#include "navtoolbar.hxx"
#include "commandimageprovider.hxx"
#include "commanddescriptionprovider.hxx"

#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/form/runtime/FormFeature.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

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
    using namespace ::com::sun::star::graphic;
    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;

#define FORWARD_TO_PEER_1( unoInterface, method, param1 )   \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        xTypedPeer->method( param1 );  \
    }

    //==================================================================
    // ONavigationBarControl
    //==================================================================
    DBG_NAME( ONavigationBarControl )
    //------------------------------------------------------------------
    ONavigationBarControl::ONavigationBarControl( const Reference< XComponentContext >& _rxORB)
        :UnoControl(), m_xContext(_rxORB)
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
        //..............................................................
        static WinBits lcl_getWinBits_nothrow( const Reference< XControlModel >& _rxModel )
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
                DBG_UNHANDLED_EXCEPTION();
            }
            return nBits;
        }
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarControl::createPeer( const Reference< XToolkit >& /*_rToolkit*/, const Reference< XWindowPeer >& _rParentPeer ) throw( RuntimeException )
    {
        SolarMutexGuard aGuard;

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
            ONavigationBarPeer* pPeer = ONavigationBarPeer::Create( m_xContext, pParentWin, getModel() );
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

            OControl::initFormControlPeer( getPeer() );
        }
    }

    //------------------------------------------------------------------
    OUString SAL_CALL ONavigationBarControl::getImplementationName()  throw( RuntimeException )
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    Sequence< OUString > SAL_CALL ONavigationBarControl::getSupportedServiceNames()  throw( RuntimeException )
    {
        return getSupportedServiceNames_Static();
    }

    //------------------------------------------------------------------
    OUString SAL_CALL ONavigationBarControl::getImplementationName_Static()
    {
        return OUString( "com.sun.star.comp.form.ONavigationBarControl" );
    }

    //------------------------------------------------------------------
    Sequence< OUString > SAL_CALL ONavigationBarControl::getSupportedServiceNames_Static()
    {
        Sequence< OUString > aServices( 2 );
        aServices[ 0 ] = OUString( "com.sun.star.awt.UnoControl" );
        aServices[ 1 ] = OUString( "com.sun.star.form.control.NavigationToolBar" );
        return aServices;
    }

    //------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ONavigationBarControl::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new ONavigationBarControl( comphelper::getComponentContext(_rxFactory) ) );
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
    ONavigationBarPeer* ONavigationBarPeer::Create( const Reference< XComponentContext >& _rxORB,
        Window* _pParentWindow, const Reference< XControlModel >& _rxModel )
    {
        DBG_TESTSOLARMUTEX();

        // the peer itself
        ONavigationBarPeer* pPeer = new ONavigationBarPeer( _rxORB );
        pPeer->acquire();   // by definition, the returned object is aquired once

        // the VCL control for the peer
        Reference< XModel > xContextDocument( getXModel( _rxModel ) );
        NavigationToolBar* pNavBar = new NavigationToolBar(
            _pParentWindow,
            lcl_getWinBits_nothrow( _rxModel ),
            createDocumentCommandImageProvider( _rxORB, xContextDocument ),
            createDocumentCommandDescriptionProvider( _rxORB, xContextDocument )
        );

        // some knittings
        pNavBar->setDispatcher( pPeer );
        pNavBar->SetComponentInterface( pPeer );

        // we want a faster repeating rate for the slots in this
        // toolbox
        AllSettings aSettings = pNavBar->GetSettings();
        MouseSettings aMouseSettings = aSettings.GetMouseSettings();
        aMouseSettings.SetButtonRepeat( 10 );
        aSettings.SetMouseSettings( aMouseSettings );
        pNavBar->SetSettings( aSettings, sal_True );

        // outta here
        return pPeer;
    }

    //------------------------------------------------------------------
    ONavigationBarPeer::ONavigationBarPeer( const Reference< XComponentContext >& _rxORB )
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
    void SAL_CALL ONavigationBarPeer::setProperty( const OUString& _rPropertyName, const Any& _rValue ) throw( RuntimeException )
    {
        SolarMutexGuard aGuard;

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
    Any SAL_CALL ONavigationBarPeer::getProperty( const OUString& _rPropertyName ) throw( RuntimeException )
    {
        SolarMutexGuard aGuard;

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
    void ONavigationBarPeer::featureStateChanged( sal_Int16 _nFeatureId, sal_Bool _bEnabled )
    {
        // enable this button on the toolbox
        NavigationToolBar* pNavBar = static_cast< NavigationToolBar* >( GetWindow() );
        if ( pNavBar )
        {
            pNavBar->enableFeature( _nFeatureId, _bEnabled );

            // is it a feature with additional state information?
            if ( _nFeatureId == FormFeature::ToggleApplyFilter )
            {   // additional boolean state
                pNavBar->checkFeature( _nFeatureId, getBooleanState( _nFeatureId ) );
            }
            else if ( _nFeatureId == FormFeature::TotalRecords )
            {
                pNavBar->setFeatureText( _nFeatureId, getStringState( _nFeatureId ) );
            }
            else if ( _nFeatureId == FormFeature::MoveAbsolute )
            {
                pNavBar->setFeatureText( _nFeatureId, OUString::number(getIntegerState(_nFeatureId)) );
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
    bool ONavigationBarPeer::isEnabled( sal_Int16 _nFeatureId ) const
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
    void ONavigationBarPeer::getSupportedFeatures( ::std::vector< sal_Int16 >& _rFeatureIds )
    {
        _rFeatureIds.push_back( FormFeature::MoveAbsolute );
        _rFeatureIds.push_back( FormFeature::TotalRecords );
        _rFeatureIds.push_back( FormFeature::MoveToFirst );
        _rFeatureIds.push_back( FormFeature::MoveToPrevious );
        _rFeatureIds.push_back( FormFeature::MoveToNext );
        _rFeatureIds.push_back( FormFeature::MoveToLast );
        _rFeatureIds.push_back( FormFeature::SaveRecordChanges );
        _rFeatureIds.push_back( FormFeature::UndoRecordChanges );
        _rFeatureIds.push_back( FormFeature::MoveToInsertRow );
        _rFeatureIds.push_back( FormFeature::DeleteRecord );
        _rFeatureIds.push_back( FormFeature::ReloadForm );
        _rFeatureIds.push_back( FormFeature::RefreshCurrentControl );
        _rFeatureIds.push_back( FormFeature::SortAscending );
        _rFeatureIds.push_back( FormFeature::SortDescending );
        _rFeatureIds.push_back( FormFeature::InteractiveSort );
        _rFeatureIds.push_back( FormFeature::AutoFilter );
        _rFeatureIds.push_back( FormFeature::InteractiveFilter );
        _rFeatureIds.push_back( FormFeature::ToggleApplyFilter );
        _rFeatureIds.push_back( FormFeature::RemoveFilterAndSort );
    }

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
