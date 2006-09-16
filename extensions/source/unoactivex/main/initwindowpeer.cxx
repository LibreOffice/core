/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: initwindowpeer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:29:55 $
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
#include "precompiled_extensions.hxx"

#include "initwindowpeer.hxx"

using namespace ::com::sun::star;


uno::Any SAL_CALL InitWindowPeer::getWindowHandle( const uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType )
    throw ( uno::RuntimeException )
{
    uno::Any aRes;
    sal_Int32 nHwnd = (sal_Int32)m_hwnd;
    aRes <<= nHwnd;
    return aRes;
}


uno::Reference< awt::XToolkit > SAL_CALL InitWindowPeer::getToolkit()
    throw ( uno::RuntimeException )
{
    return uno::Reference< awt::XToolkit >();
}

void SAL_CALL InitWindowPeer::setPointer( const uno::Reference< awt::XPointer >& Pointer )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::setBackground( sal_Int32 Color )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidate( sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidateRect( const awt::Rectangle& Rect, sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::dispose()
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw ( uno::RuntimeException )
{
}

