/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xsimpleanimation.cxx,v $
 * $Revision: 1.4 $
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
#include "toolkit/awt/xsimpleanimation.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/throbberimpl.hxx"
#include <tools/debug.hxx>

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
