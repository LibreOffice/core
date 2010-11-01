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
#include "precompiled_sfx2.hxx"

#include "helpdispatch.hxx"
#include <sfx2/sfxuno.hxx>
#include "newhelp.hxx"
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

// class HelpInterceptor_Impl --------------------------------------------

HelpDispatch_Impl::HelpDispatch_Impl( HelpInterceptor_Impl& _rInterceptor,
                                      const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::frame::XDispatch >& _xDisp ) :

    m_rInterceptor  ( _rInterceptor ),
    m_xRealDispatch ( _xDisp )

{
}

// -----------------------------------------------------------------------

HelpDispatch_Impl::~HelpDispatch_Impl()
{
}

// -----------------------------------------------------------------------
// XDispatch

void SAL_CALL HelpDispatch_Impl::dispatch(

    const URL& aURL, const Sequence< PropertyValue >& aArgs ) throw( RuntimeException )

{
    DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );

    // search for a keyword (dispatch from the basic ide)
    sal_Bool bHasKeyword = sal_False;
    String sKeyword;
    const PropertyValue* pBegin = aArgs.getConstArray();
    const PropertyValue* pEnd   = pBegin + aArgs.getLength();
    for ( ; pBegin != pEnd; ++pBegin )
    {
        if ( 0 == ( *pBegin ).Name.compareToAscii( "HelpKeyword" ) )
        {
            rtl::OUString sHelpKeyword;
            if ( ( ( *pBegin ).Value >>= sHelpKeyword ) && sHelpKeyword.getLength() > 0 )
            {
                sKeyword = String( sHelpKeyword );
                bHasKeyword = ( sKeyword.Len() > 0 );
                break;
            }
        }
    }

    // if a keyword was found, then open it
    SfxHelpWindow_Impl* pHelpWin = m_rInterceptor.GetHelpWindow();
    DBG_ASSERT( pHelpWin, "invalid HelpWindow" );
    if ( bHasKeyword )
    {
        pHelpWin->OpenKeyword( sKeyword );
        return;
    }

    pHelpWin->loadHelpContent(aURL.Complete);
}

// -----------------------------------------------------------------------

void SAL_CALL HelpDispatch_Impl::addStatusListener(

    const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
    DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );
    m_xRealDispatch->addStatusListener( xControl, aURL );
}

// -----------------------------------------------------------------------

void SAL_CALL HelpDispatch_Impl::removeStatusListener(

    const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
    DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );
    m_xRealDispatch->removeStatusListener( xControl, aURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
