/*************************************************************************
 *
 *  $RCSfile: aqua_clipboard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pluby $ $Date: 2001-03-16 17:32:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *       - GNU Lesser General Public License Version 2.1
 *       - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

