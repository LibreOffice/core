/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

#include <toolkit/helper/vclunohelper.hxx>
//#include <toolkit/unohlp.hxx>

#include <rtl/ref.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include "uielement/popupmenucontroller.hxx"
#include "services.h"

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

// --------------------------------------------------------------------


namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   PopupMenuController                         ,
                                            OWeakObject                      ,
                                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ) )            ,
                                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.framework.PopupMenuController" ) )
                                        )

DEFINE_INIT_SERVICE                     (   PopupMenuController, {} )

class PopupMenuControllerImpl
{
};

//========================================================================
// class PopupMenuController
//========================================================================

PopupMenuController::PopupMenuController( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::ToolboxController( rServiceManager, Reference< frame::XFrame >(), OUString() )
{
}

// --------------------------------------------------------------------

PopupMenuController::~PopupMenuController()
{
}

// --------------------------------------------------------------------
// XInterface
// --------------------------------------------------------------------

Any SAL_CALL PopupMenuController::queryInterface( const Type& aType )  throw (RuntimeException)
{
    Any a( ToolboxController::queryInterface( aType ) );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::acquire() throw ()
{
    ToolboxController::acquire();
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::release() throw ()
{
    ToolboxController::release();
}

// --------------------------------------------------------------------
// XComponent
// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::dispose() throw (RuntimeException)
{
    if( mxPopupMenuController.is() )
    {
        Reference< XComponent > xComponent( mxPopupMenuController, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();
        mxPopupMenuController.clear();
    }

    mxPopupMenu.clear();

    svt::ToolboxController::dispose();
}

// --------------------------------------------------------------------
// XStatusListener
// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::statusChanged( const frame::FeatureStateEvent& rEvent ) throw ( RuntimeException )
{
    svt::ToolboxController::statusChanged(rEvent);
    enable( rEvent.IsEnabled );
}

// --------------------------------------------------------------------
// XToolbarController
// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::execute( sal_Int16 KeyModifier ) throw (RuntimeException)
{
    svt::ToolboxController::execute( KeyModifier );
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::click() throw (RuntimeException)
{
    svt::ToolboxController::click();
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::doubleClick() throw (RuntimeException)
{
    svt::ToolboxController::doubleClick();
}

// --------------------------------------------------------------------

bool PopupMenuController::CreatePopupMenuController() throw (Exception)
{
    Reference< XMultiComponentFactory > xPopupMenuControllerRegistration( getServiceManager()->createInstance( SERVICENAME_POPUPMENUCONTROLLERFACTORY ), UNO_QUERY_THROW );

    Sequence< Any > aSeq( 2 );
    PropertyValue aPropValue;

    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
    aPropValue.Value <<= getModuleName();
    aSeq[0] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropValue.Value <<= m_xFrame;
    aSeq[1] <<= aPropValue;

    Reference< XPropertySet >      xProps( getServiceManager(), UNO_QUERY_THROW );
    Reference< XComponentContext > xComponentContext( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), UNO_QUERY_THROW );

    Reference< XPopupMenuController > xPopupMenuController( xPopupMenuControllerRegistration->createInstanceWithArgumentsAndContext( getCommandURL(), aSeq, xComponentContext ), UNO_QUERY );
    if ( xPopupMenuController.is() )
    {
        mxPopupMenuController = xPopupMenuController;
        return true;
    }
    return false;
}

Reference< awt::XWindow > SAL_CALL PopupMenuController::createPopupWindow() throw (RuntimeException)
{
    ::vos::OGuard aSolarLock(Application::GetSolarMutex());

    Reference< awt::XWindow > xRet;

    try
    {
        ToolBox* pToolBox = dynamic_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
        if( !pToolBox )
            return xRet;

        // get selected button
        sal_uInt16 nItemId = pToolBox->GetDownItemId();
        if( !nItemId )
            return xRet;

        ::Rectangle aRect( pToolBox->GetItemRect( nItemId ) );

        if( !mxPopupMenuController.is() && !CreatePopupMenuController() )
            return xRet;

        if( !mxPopupMenu.is() )
        {
            mxPopupMenu = Reference< awt::XPopupMenu >( getServiceManager()->createInstance( DECLARE_ASCII( "com.sun.star.awt.PopupMenu" ) ), UNO_QUERY_THROW );
            mxPopupMenuController->setPopupMenu( mxPopupMenu );
        }
        else
        {
            mxPopupMenuController->updatePopupMenu();

        }
        pToolBox->SetItemDown( nItemId, sal_True );
        Reference< awt::XWindowPeer > xPeer( getParent(), UNO_QUERY_THROW );
        mxPopupMenu->execute( xPeer, awt::Point( aRect.getX(), aRect.getY() ), 0 );
        pToolBox->SetItemDown( nItemId, sal_False );
    }
    catch( Exception& )
    {
    }

    return xRet;
}

// --------------------------------------------------------------------

Reference< awt::XWindow > SAL_CALL PopupMenuController::createItemWindow( const Reference< awt::XWindow >& /*Parent*/ )  throw (RuntimeException)
{
    return Reference< awt::XWindow >();
}

// --------------------------------------------------------------------

}

