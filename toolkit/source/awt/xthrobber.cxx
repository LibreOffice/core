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

#include "toolkit/awt/xthrobber.hxx"
#include "toolkit/helper/property.hxx"
#include <toolkit/helper/tkresmgr.hxx>

#include "xthrobber.hrc"
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/throbber.hxx>

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
    }

    //--------------------------------------------------------------------
    XThrobber::~XThrobber()
    {
        DBG_DTOR( XThrobber, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::start() throw ( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::stop() throw ( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->stop();
    }

    //--------------------------------------------------------------------
    void XThrobber::SetWindow( Window* pWindow )
    {
        XThrobber_Base::SetWindow( pWindow );
        InitImageList();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::InitImageList()
        throw( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL)
            return;

        uno::Sequence< uno::Reference< graphic::XGraphic > > aImageList(12);
        sal_uInt16 nIconIdStart = RID_TK_ICON_THROBBER_START;

        for ( sal_uInt16 i=0; i<12; i++ )
        {
            Image aImage = TK_RES_IMAGE( nIconIdStart + i );
            aImageList[i] = aImage.GetXGraphic();
        }

        pThrobber->setImageList( aImageList );
    }

//........................................................................
}   // namespace toolkit
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
