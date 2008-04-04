/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsimpleanimation.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:54:54 $
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

#ifndef   TOOLKIT_AWT_XSIMPLEANIMATION_HXX
#include "toolkit/awt/xsimpleanimation.hxx"
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include "toolkit/helper/property.hxx"
#endif
#ifndef  _TOOLKIT_HELPER_THROBBERIMPL_HXX_
#include "toolkit/helper/throbberimpl.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star;

    //====================================================================
    //= XSimpleAnimation
    //====================================================================
    DBG_NAME( XSimpleAnimation )

    //--------------------------------------------------------------------
    XSimpleAnimation::XSimpleAnimation()
    {
        DBG_CTOR( XSimpleAnimation, NULL );
        mbRepeat = sal_True;
        mnStepTime = 100;
        mpThrobber = new Throbber_Impl( this, mnStepTime, mbRepeat );
    }

    //--------------------------------------------------------------------
    XSimpleAnimation::~XSimpleAnimation()
    {
        DBG_DTOR( XSimpleAnimation, NULL );
        delete mpThrobber;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( XSimpleAnimation, VCLXWindow, XSimpleAnimation_Base )

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( XSimpleAnimation, VCLXWindow, XSimpleAnimation_Base )

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::start() throw ( uno::RuntimeException )
    {
        mpThrobber->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::stop() throw ( uno::RuntimeException )
    {
        mpThrobber->stop();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& rImageList )
        throw ( uno::RuntimeException )
    {
        mpThrobber->setImageList( rImageList );
    }

    //--------------------------------------------------------------------
    void XSimpleAnimation::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
    {
        // TODO: XSimpleAnimation::ProcessWindowEvent
        //::vos::OClearableGuard aGuard( GetMutex() );
        //Reference< XSimpleAnimation > xKeepAlive( this );
        //SpinButton* pSpinButton = static_cast< SpinButton* >( GetWindow() );
        //if ( !pSpinButton )
        //    return;

        VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::setProperty( const ::rtl::OUString& PropertyName, const uno::Any& Value )
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        if ( GetWindow() )
        {
            sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
            switch ( nPropertyId )
            {
                case BASEPROPERTY_STEP_TIME: {
                    sal_Int32   nStepTime( 0 );
                    if ( Value >>= nStepTime )
                    {
                        mnStepTime = nStepTime;
                        mpThrobber->setStepTime( mnStepTime );
                    }

                    break;
                }
                case BASEPROPERTY_REPEAT: {
                    sal_Bool bRepeat( sal_True );
                    if ( Value >>= bRepeat )
                    {
                        mbRepeat = bRepeat;
                        mpThrobber->setRepeat( mbRepeat );
                    }
                    break;
                }
                default:
                    VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }

    //--------------------------------------------------------------------
    uno::Any SAL_CALL XSimpleAnimation::getProperty( const ::rtl::OUString& PropertyName )
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        uno::Any aReturn;

        if ( GetWindow() )
        {
            sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
            switch ( nPropertyId )
            {
            case BASEPROPERTY_STEP_TIME:
                aReturn <<= mnStepTime;
                break;
            case BASEPROPERTY_REPEAT:
                aReturn <<= mbRepeat;
                break;
            default:
                aReturn = VCLXWindow::getProperty( PropertyName );
            }
        }
        return aReturn;
    }

//........................................................................
}   // namespace toolkit
//........................................................................
