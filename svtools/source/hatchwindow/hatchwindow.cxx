/*************************************************************************
 *
 *  $RCSfile: hatchwindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:45:40 $
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

#ifndef _COM_SUN_STAR_EMBED_XHATCHWINDOWCONTROLLER_HPP_
#include <com/sun/star/embed/XHatchWindowController.hpp>
#endif

#include "hatchwindow.hxx"
#include "ipwin.hxx"

#include <toolkit/helper/convert.hxx>

using namespace ::com::sun::star;

VCLXHatchWindow::VCLXHatchWindow()
: VCLXWindow()
{
}

VCLXHatchWindow::~VCLXHatchWindow()
{
}

void VCLXHatchWindow::initializeWindow( const uno::Reference< awt::XWindowPeer >& xParent,
                const awt::Rectangle& aBounds,
                const awt::Size& aSize )
{
    Window* pParent = NULL;
    VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( xParent );

    if ( pParentComponent )
        pParent = pParentComponent->GetWindow();

    OSL_ENSURE( pParent, "No parent window is provided!\n" );
    if ( !pParent )
        throw lang::IllegalArgumentException(); // TODO

    SvResizeWindow* pHatchWindow = new SvResizeWindow( pParent, this );
    pHatchWindow->SetPosSizePixel( aBounds.X, aBounds.Y, aBounds.Width, aBounds.Height );
    pHatchWindow->SetHatchBorderPixel( Size( aSize.Width, aSize.Height ) );

    SetWindow( pHatchWindow );
    pHatchWindow->SetComponentInterface( this );

    //pHatchWindow->Show();
}

void VCLXHatchWindow::QueryObjAreaPixel( Rectangle & aRect )
{
    if ( m_xController.is() )
    {
        awt::Rectangle aUnoRequestRect = AWTRectangle( aRect );

        try {
            awt::Rectangle aUnoResultRect = m_xController->calcAdjustedRectangle( aUnoRequestRect );
            aRect = VCLRectangle( aUnoResultRect );
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Can't adjust rectangle size!\n" );
        }
    }
}

void VCLXHatchWindow::RequestObjAreaPixel( const Rectangle & aRect )
{
    if ( m_xController.is() )
    {
        awt::Rectangle aUnoRequestRect = AWTRectangle( aRect );

        try {
            m_xController->requestPositioning( aUnoRequestRect );
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Can't request resizing!\n" );
        }
    }
}

void VCLXHatchWindow::InplaceDeactivate()
{
    if ( m_xController.is() )
    {
        // TODO: communicate with controller
    }
}


uno::Any SAL_CALL VCLXHatchWindow::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    uno::Any aReturn( ::cppu::queryInterface( rType,
                                           static_cast< embed::XHatchWindow* >( this ) ) );

    if ( aReturn.hasValue() == sal_True )
    {
        return aReturn ;
    }

    return VCLXWindow::queryInterface( rType ) ;
}

void SAL_CALL VCLXHatchWindow::acquire()
    throw()
{
    VCLXWindow::acquire();
}

void SAL_CALL VCLXHatchWindow::release()
    throw()
{
    VCLXWindow::release();
}

uno::Sequence< uno::Type > SAL_CALL VCLXHatchWindow::getTypes()
    throw( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pTypeCollection == NULL )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                    ::getCppuType(( const uno::Reference< embed::XHatchWindow >* )NULL ),
                    VCLXHatchWindow::getTypes() );

            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

uno::Sequence< sal_Int8 > SAL_CALL VCLXHatchWindow::getImplementationId()
    throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pID == NULL )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

void SAL_CALL VCLXHatchWindow::setController( const uno::Reference< embed::XHatchWindowController >& xController )
    throw (uno::RuntimeException)
{
    m_xController = xController;
}

void SAL_CALL VCLXHatchWindow::dispose()
    throw (uno::RuntimeException)
{
    VCLXWindow::dispose();
}

void SAL_CALL VCLXHatchWindow::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    VCLXWindow::addEventListener( xListener );
}

void SAL_CALL VCLXHatchWindow::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    VCLXWindow::removeEventListener( xListener );
}

