/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xthrobber.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:55:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#ifndef   TOOLKIT_AWT_XTHROBBER_HXX
#include "toolkit/awt/xthrobber.hxx"
#endif
#ifndef  _TOOLKIT_HELPER_PROPERTY_HXX_
#include "toolkit/helper/property.hxx"
#endif
#ifndef  _TOOLKIT_HELPER_TKRESMGR_HXX_
#include <toolkit/helper/tkresmgr.hxx>
#endif
#ifndef  _TOOLKIT_HELPER_THROBBERIMPL_HXX_
#include <toolkit/helper/throbberimpl.hxx>
#endif

#ifndef _TOOLKIT_AWT_XTHROBBER_HRC_
#include "xthrobber.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef   _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef   _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

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
