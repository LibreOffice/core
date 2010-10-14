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
#include "precompiled_unotools.hxx"
#include <unotools/progresshandlerwrap.hxx>

namespace utl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;

ProgressHandlerWrap::ProgressHandlerWrap( ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > xSI )
: m_xStatusIndicator( xSI )
{
}

sal_Bool getStatusFromAny_Impl( const Any& aAny, ::rtl::OUString& aText, sal_Int32& nNum )
{
    sal_Bool bNumIsSet = sal_False;

    Sequence< Any > aSetList;
    if( ( aAny >>= aSetList ) && aSetList.getLength() )
        for( int ind = 0; ind < aSetList.getLength(); ind++ )
        {
            if( !bNumIsSet && ( aSetList[ind] >>= nNum ) )
                bNumIsSet = sal_True;
            else
                !aText.getLength() && ( aSetList[ind] >>= aText );
        }

    return bNumIsSet;
}

void SAL_CALL ProgressHandlerWrap::push( const Any& Status )
    throw( RuntimeException )
{
    if( !m_xStatusIndicator.is() )
        return;

    ::rtl::OUString aText;
    sal_Int32 nRange;

    if( getStatusFromAny_Impl( Status, aText, nRange ) )
        m_xStatusIndicator->start( aText, nRange );
}

void SAL_CALL ProgressHandlerWrap::update( const Any& Status )
    throw( RuntimeException )
{
    if( !m_xStatusIndicator.is() )
        return;

    ::rtl::OUString aText;
    sal_Int32 nValue;

    if( getStatusFromAny_Impl( Status, aText, nValue ) )
    {
        if( aText.getLength() ) m_xStatusIndicator->setText( aText );
        m_xStatusIndicator->setValue( nValue );
    }
}

void SAL_CALL ProgressHandlerWrap::pop()
        throw( RuntimeException )
{
    if( m_xStatusIndicator.is() )
        m_xStatusIndicator->end();
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
