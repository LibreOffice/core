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


#include "ShapeToolbarController.hxx"

#include <osl/mutex.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxids.hrc>
#include <svx/tbxcustomshapes.hxx>


using namespace com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

//.............................................................................
namespace chart
{
//.............................................................................

::rtl::OUString ShapeToolbarController::getImplementationName() throw (uno::RuntimeException)
{
    return getImplementationName_Static();
}

::rtl::OUString ShapeToolbarController::getImplementationName_Static() throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.ShapeToolbarController" ) );
}

Sequence< ::rtl::OUString > ShapeToolbarController::getSupportedServiceNames_Static() throw (uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ShapeToolbarController" ) );
    return aSupported;
}

::sal_Bool ShapeToolbarController::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
{
    return ::comphelper::existsValue( ServiceName, getSupportedServiceNames_Static() );
}

Sequence< ::rtl::OUString> ShapeToolbarController::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

Reference< uno::XInterface > ShapeToolbarController::create( const Reference< uno::XComponentContext >& xContext )
{
    return *( new ShapeToolbarController( Reference< lang::XMultiServiceFactory >( xContext->getServiceManager(), uno::UNO_QUERY ) ) );
}

ShapeToolbarController::ShapeToolbarController( const Reference< lang::XMultiServiceFactory >& rxFact )
    :m_pToolbarController( NULL )
    ,m_nToolBoxId( 1 )
    ,m_nSlotId( 0 )
{
    osl_atomic_increment( &m_refCount );
    m_xServiceManager = rxFact;
    osl_atomic_decrement( &m_refCount );
}

ShapeToolbarController::~ShapeToolbarController()
{
}

// ::com::sun::star::uno::XInterface
uno::Any ShapeToolbarController::queryInterface( const uno::Type& rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = ToolboxController::queryInterface( rType );
    if ( !aReturn.hasValue() )
    {
        aReturn = ShapeToolbarController_Base::queryInterface( rType );
    }
    return aReturn;
}

void ShapeToolbarController::acquire() throw ()
{
    ToolboxController::acquire();
}

void ShapeToolbarController::release() throw ()
{
    ToolboxController::release();
}

// ::com::sun::star::lang::XInitialization
void ShapeToolbarController::initialize( const Sequence< uno::Any >& rArguments ) throw (uno::Exception, uno::RuntimeException)
{
    ToolboxController::initialize( rArguments );
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    ToolBox* pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
    if ( pToolBox )
    {
        const sal_uInt16 nCount = pToolBox->GetItemCount();
        for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
        {
            const sal_uInt16 nItemId = pToolBox->GetItemId( nPos );
            if ( pToolBox->GetItemCommand( nItemId ) == String( m_aCommandURL ) )
            {
                m_nToolBoxId = nItemId;
                break;
            }
        }
        if ( m_aCommandURL == ".uno:BasicShapes" )
        {
            m_aStates.insert( TCommandState::value_type( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:BasicShapes" ) ), sal_True ) );
            m_nSlotId = SID_DRAWTBX_CS_BASIC;
            m_pToolbarController = TToolbarHelper::createFromQuery( new SvxTbxCtlCustomShapes( m_nSlotId, m_nToolBoxId, *pToolBox ) );
        }
        else if ( m_aCommandURL == ".uno:SymbolShapes" )
        {
            m_aStates.insert( TCommandState::value_type( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SymbolShapes" ) ), sal_True ) );
            m_nSlotId = SID_DRAWTBX_CS_SYMBOL;
            m_pToolbarController = TToolbarHelper::createFromQuery( new SvxTbxCtlCustomShapes( m_nSlotId, m_nToolBoxId, *pToolBox ) );
        }
        else if ( m_aCommandURL == ".uno:ArrowShapes" )
        {
            m_aStates.insert( TCommandState::value_type( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ArrowShapes" ) ), sal_True ) );
            m_nSlotId = SID_DRAWTBX_CS_ARROW;
            m_pToolbarController = TToolbarHelper::createFromQuery( new SvxTbxCtlCustomShapes( m_nSlotId, m_nToolBoxId, *pToolBox) );
        }
        else if ( m_aCommandURL == ".uno:FlowChartShapes" )
        {
            m_aStates.insert( TCommandState::value_type( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FlowChartShapes" ) ), sal_True ) );
            m_nSlotId = SID_DRAWTBX_CS_FLOWCHART;
            m_pToolbarController = TToolbarHelper::createFromQuery( new SvxTbxCtlCustomShapes( m_nSlotId, m_nToolBoxId, *pToolBox ) );
        }
        else if ( m_aCommandURL == ".uno:CalloutShapes" )
        {
            m_aStates.insert( TCommandState::value_type( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CalloutShapes" ) ), sal_True ) );
            m_nSlotId = SID_DRAWTBX_CS_CALLOUT;
            m_pToolbarController = TToolbarHelper::createFromQuery( new SvxTbxCtlCustomShapes( m_nSlotId, m_nToolBoxId, *pToolBox ) );
        }
        else if ( m_aCommandURL == ".uno:StarShapes" )
        {
            m_aStates.insert( TCommandState::value_type( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StarShapes" ) ), sal_True ) );
            m_nSlotId = SID_DRAWTBX_CS_STAR;
            m_pToolbarController = TToolbarHelper::createFromQuery( new SvxTbxCtlCustomShapes( m_nSlotId, m_nToolBoxId, *pToolBox ) );
        }

        for ( TCommandState::iterator aIter( m_aStates.begin() ); aIter != m_aStates.end(); ++aIter )
        {
            addStatusListener( aIter->first );
        }

        if ( m_pToolbarController.is() )
        {
            m_pToolbarController->initialize( rArguments );
        }

        // check if paste special is allowed, when not don't add DROPDOWN
        pToolBox->SetItemBits( m_nToolBoxId, pToolBox->GetItemBits( m_nToolBoxId ) | TIB_DROPDOWN );
    }
}

// ::com::sun::star::frame::XStatusListener
void ShapeToolbarController::statusChanged( const frame::FeatureStateEvent& Event ) throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
    if ( aFind != m_aStates.end() )
    {
        aFind->second = Event.IsEnabled;
        if ( m_pToolbarController.is() )
        {
            sal_Bool bCheckmark = sal_False;
            ToolBox& rTb = m_pToolbarController->GetToolBox();

            for ( sal_uInt16 i = 0; i < rTb.GetItemCount(); ++i )
            {
                sal_uInt16 nId = rTb.GetItemId( i );
                if ( nId == 0 )
                {
                    continue;
                }
                ::rtl::OUString aCmd = rTb.GetItemCommand( nId );
                if ( aCmd == Event.FeatureURL.Complete )
                {
                    rTb.EnableItem( nId, Event.IsEnabled );
                    if ( Event.State >>= bCheckmark )
                    {
                        rTb.CheckItem( nId, bCheckmark );
                    }
                    else
                    {
                        ::rtl::OUString aItemText;
                        if ( Event.State >>= aItemText )
                        {
                            rTb.SetItemText( nId, aItemText );
                        }
                    }
                }
            }
        }
    }
}

// ::com::sun::star::frame::XToolbarController
Reference< awt::XWindow > ShapeToolbarController::createPopupWindow() throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< awt::XWindow > xRet;
    if ( m_pToolbarController.is() )
    {
        xRet = m_pToolbarController.getRef()->createPopupWindow();
    }

    return xRet;
}

// ::com::sun::star::frame::XSubToolbarController
::sal_Bool ShapeToolbarController::opensSubToolbar() throw (uno::RuntimeException)
{
    return ( m_nSlotId == SID_DRAWTBX_CS_BASIC ||
             m_nSlotId == SID_DRAWTBX_CS_SYMBOL ||
             m_nSlotId == SID_DRAWTBX_CS_ARROW ||
             m_nSlotId == SID_DRAWTBX_CS_FLOWCHART ||
             m_nSlotId == SID_DRAWTBX_CS_CALLOUT ||
             m_nSlotId == SID_DRAWTBX_CS_STAR );
}

::rtl::OUString ShapeToolbarController::getSubToolbarName() throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< frame::XSubToolbarController > xSub( m_pToolbarController.getRef(), uno::UNO_QUERY );
    if ( xSub.is() )
    {
        return xSub->getSubToolbarName();
    }
    return ::rtl::OUString();
}

void ShapeToolbarController::functionSelected( const ::rtl::OUString& rCommand ) throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< frame::XSubToolbarController > xSub( m_pToolbarController.getRef(), uno::UNO_QUERY );
    if ( xSub.is() )
    {
        m_aCommandURL = rCommand;
        xSub->functionSelected( rCommand );
    }
}

void ShapeToolbarController::updateImage() throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< frame::XSubToolbarController > xSub( m_pToolbarController.getRef(), uno::UNO_QUERY );
    if ( xSub.is() )
    {
        xSub->updateImage();
    }
}

//.............................................................................
} //  namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
