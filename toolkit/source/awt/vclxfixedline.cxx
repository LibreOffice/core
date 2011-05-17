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

#include "vclxfixedline.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

DBG_NAME( VCLXFixedLine )

VCLXFixedLine::VCLXFixedLine()
  : VCLXWindow()
{
    DBG_CTOR( VCLXFixedLine, NULL );
}

VCLXFixedLine::~VCLXFixedLine()
{
    DBG_DTOR( VCLXFixedLine, NULL );
}

IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXFixedLine, VCLXWindow );

void SAL_CALL VCLXFixedLine::dispose() throw(RuntimeException)
{
    {
        SolarMutexGuard aGuard;

        EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
    }

    VCLXWindow::dispose();
}

::com::sun::star::awt::Size SAL_CALL VCLXFixedLine::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    return awt::Size( 8, 8 );
}

void VCLXFixedLine::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    SolarMutexGuard aGuard;

    VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
}

void SAL_CALL VCLXFixedLine::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                VCLXWindow::setProperty( PropertyName, Value );
//        }
    }
}

Any SAL_CALL VCLXFixedLine::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    Any aReturn;
    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                aReturn = VCLXWindow::getProperty( PropertyName );
  //      }
    }
    return aReturn;
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
