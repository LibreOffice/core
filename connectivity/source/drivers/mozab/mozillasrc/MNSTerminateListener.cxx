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
#include "precompiled_connectivity.hxx"
#include <MNSTerminateListener.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <MNSInit.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
::com::sun::star::uno::Reference< ::com::sun::star::frame::XTerminateListener>          MNSTerminateListener::mxTerminateListener = new MNSTerminateListener();

// -----------------------------------------
// - MNSTerminateListener -
// -----------------------------------------

MNSTerminateListener::MNSTerminateListener(  )
{
}

// -----------------------------------------------------------------------------

MNSTerminateListener::~MNSTerminateListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::disposing( const EventObject& /*Source*/ ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::queryTermination( const EventObject& /*aEvent*/ ) throw( TerminationVetoException, RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::notifyTermination( const EventObject& /*aEvent*/ ) throw( RuntimeException )
{
    MNS_Term(sal_True); //Force XPCOM to shutdown
}

void MNSTerminateListener::addTerminateListener()
{
    Reference< XMultiServiceFactory >   xFact( ::comphelper::getProcessServiceFactory() );

    if( xFact.is() )
    {
        Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ), UNO_QUERY );

        if( xDesktop.is() )
            xDesktop->addTerminateListener(mxTerminateListener);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
