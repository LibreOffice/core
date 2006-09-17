/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progresshandlerwrap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:29:40 $
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
#include "precompiled_unotools.hxx"

#ifndef _UTL_PROGRESSHANDLERWRAP_HXX_
#include <unotools/progresshandlerwrap.hxx>
#endif

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

