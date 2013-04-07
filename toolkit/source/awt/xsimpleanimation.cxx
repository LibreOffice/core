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
    void SAL_CALL XSimpleAnimation::setProperty( const OUString& PropertyName, const uno::Any& Value )
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
    uno::Any SAL_CALL XSimpleAnimation::getProperty( const OUString& PropertyName )
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
