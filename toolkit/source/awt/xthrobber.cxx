/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xthrobber.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_toolkit.hxx"
#include "toolkit/awt/xthrobber.hxx"
#include "toolkit/helper/property.hxx"
#include <toolkit/helper/tkresmgr.hxx>
#include <toolkit/helper/throbberimpl.hxx>

#ifndef _TOOLKIT_AWT_XTHROBBER_HRC_
#include "xthrobber.hrc"
#endif
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star;

    //====================================================================
    //= XThrobber
    //====================================================================
    DBG_NAME( XThrobber )

    //--------------------------------------------------------------------
    XThrobber::XThrobber()
    {
        DBG_CTOR( XThrobber, NULL );

        mpThrobber = new Throbber_Impl( this, 100, sal_True );

        InitImageList();
    }

    //--------------------------------------------------------------------
    XThrobber::~XThrobber()
    {
        DBG_DTOR( XThrobber, NULL );
        delete mpThrobber;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( XThrobber, VCLXWindow, XThrobber_Base )

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( XThrobber, VCLXWindow, XThrobber_Base )

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::start() throw ( uno::RuntimeException )
    {
        mpThrobber->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::stop() throw ( uno::RuntimeException )
    {
        mpThrobber->stop();
    }

    //--------------------------------------------------------------------
    void XThrobber::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
    {
        static bool bInit = false;
        if ( !bInit )
        {
            // Images won't be shown if set too early
            mpThrobber->initImage();
            bInit = true;
        }
        // TODO: XSimpleAnimation::ProcessWindowEvent
        //::vos::OClearableGuard aGuard( GetMutex() );
        //Reference< XSimpleAnimation > xKeepAlive( this );
        //SpinButton* pSpinButton = static_cast< SpinButton* >( GetWindow() );
        //if ( !pSpinButton )
        //    return;

        VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::setProperty( const ::rtl::OUString& PropertyName, const uno::Any& Value )
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        if ( GetWindow() )
        {
            VCLXWindow::setProperty( PropertyName, Value );
        }
    }

    //--------------------------------------------------------------------
    uno::Any SAL_CALL XThrobber::getProperty( const ::rtl::OUString& PropertyName )
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        uno::Any aReturn;

        if ( GetWindow() )
        {
            aReturn = VCLXWindow::getProperty( PropertyName );
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::InitImageList()
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );
        uno::Sequence< uno::Reference< graphic::XGraphic > > aImageList(12);
        sal_uInt16 nIconIdStart = RID_TK_ICON_THROBBER_START;

        if ( mpThrobber->isHCMode() )
            nIconIdStart = RID_TK_HC_ICON_THROBBER_START;

        for ( sal_uInt16 i=0; i<12; i++ )
        {
            Image aImage = TK_RES_IMAGE( nIconIdStart + i );
            aImageList[i] = aImage.GetXGraphic();
        }

        mpThrobber->setImageList( aImageList );
    }

//........................................................................
}   // namespace toolkit
//........................................................................
