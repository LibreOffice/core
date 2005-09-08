/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aqua_clipboard.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:08:09 $
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

#ifndef _AQUA_CLIPBOARD_HXX_
#include "aqua_clipboard.hxx"
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace aqua;

AquaClipboard::AquaClipboard() :
    WeakComponentImplHelper3< XClipboardEx, XClipboardNotifier, XServiceInfo >( m_aMutex )
{
}

Reference< XTransferable > SAL_CALL AquaClipboard::getContents() throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    return m_aTransferable;
}

void SAL_CALL AquaClipboard::setContents( const Reference< XTransferable >& xTransferable, const Reference< XClipboardOwner >& xClipboardOwner ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    m_aTransferable = xTransferable;
}

OUString SAL_CALL AquaClipboard::getName() throw( RuntimeException )
{
    return OUString();
}

sal_Int8 SAL_CALL AquaClipboard::getRenderingCapabilities() throw( RuntimeException )
{
    return 0;
}

void SAL_CALL AquaClipboard::addClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
}

void SAL_CALL AquaClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
}

OUString SAL_CALL AquaClipboard::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( AQUA_CLIPBOARD_IMPL_NAME );
}

sal_Bool SAL_CALL AquaClipboard::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
    return sal_False;
}

Sequence< OUString > SAL_CALL AquaClipboard::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( AQUA_CLIPBOARD_SERVICE_NAME );
    return aRet;
}

