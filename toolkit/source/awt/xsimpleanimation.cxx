/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <tools/debug.hxx>
#include <vcl/throbber.hxx>
#include <vcl/svapp.hxx>

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
    }

    //--------------------------------------------------------------------
    XSimpleAnimation::~XSimpleAnimation()
    {
        DBG_DTOR( XSimpleAnimation, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::start() throw ( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::stop() throw ( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->stop();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& rImageList )
        throw ( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->setImageList( rImageList );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::setProperty( const ::rtl::OUString& PropertyName, const uno::Any& Value )
        throw( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL )
        {
            VCLXWindow::setProperty( PropertyName, Value );
            return;
        }

        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            case BASEPROPERTY_STEP_TIME: {
                sal_Int32   nStepTime( 0 );
                if ( Value >>= nStepTime )
                    pThrobber->setStepTime( nStepTime );

                break;
            }
            case BASEPROPERTY_REPEAT: {
                sal_Bool bRepeat( sal_True );
                if ( Value >>= bRepeat )
                    pThrobber->setRepeat( bRepeat );
                break;
            }
            default:
                VCLXWindow::setProperty( PropertyName, Value );
        }
    }

    //--------------------------------------------------------------------
    uno::Any SAL_CALL XSimpleAnimation::getProperty( const ::rtl::OUString& PropertyName )
        throw( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL )
            return VCLXWindow::getProperty( PropertyName );

        uno::Any aReturn;
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
        case BASEPROPERTY_STEP_TIME:
            aReturn <<= pThrobber->getStepTime();
            break;
        case BASEPROPERTY_REPEAT:
            aReturn <<= pThrobber->getRepeat();
            break;
        default:
            aReturn = VCLXWindow::getProperty( PropertyName );
        }
        return aReturn;
    }

//........................................................................
}   // namespace toolkit
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
