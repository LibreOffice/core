/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: generic_clipboard.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_dtrans.hxx"

#include <generic_clipboard.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;

using ::dtrans::GenericClipboard;
using ::rtl::OUString;

GenericClipboard::GenericClipboard() :
    WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization > (m_aMutex),
    m_bInitialized(sal_False)
{
}

// ------------------------------------------------------------------------

GenericClipboard::~GenericClipboard()
{
}

// ------------------------------------------------------------------------

void SAL_CALL GenericClipboard::initialize( const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
    if (!m_bInitialized)
    {
        for (sal_Int32 n = 0, nmax = aArguments.getLength(); n < nmax; n++)
            if (aArguments[n].getValueType() == getCppuType((OUString *) 0))
            {
                aArguments[0] >>= m_aName;
                break;
            }
    }
}

// ------------------------------------------------------------------------

OUString SAL_CALL GenericClipboard::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString::createFromAscii(GENERIC_CLIPBOARD_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL GenericClipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence < OUString > SupportedServicesNames = GenericClipboard_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL GenericClipboard::getSupportedServiceNames(    )
    throw(RuntimeException)
{
    return GenericClipboard_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL GenericClipboard::getContents()
    throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL GenericClipboard::setContents(const Reference< XTransferable >& xTrans,
                                      const Reference< XClipboardOwner >& xClipboardOwner )
    throw(RuntimeException)
{
    // remember old values for callbacks before setting the new ones.
    ClearableMutexGuard aGuard(m_aMutex);

    Reference< XClipboardOwner > oldOwner(m_aOwner);
    m_aOwner = xClipboardOwner;

    Reference< XTransferable > oldContents(m_aContents);
    m_aContents = xTrans;

    aGuard.clear();

    // notify old owner on loss of ownership
    if( oldOwner.is() )
        oldOwner->lostOwnership(static_cast < XClipboard * > (this), oldContents);

    // notify all listeners on content changes
    OInterfaceContainerHelper *pContainer =
        rBHelper.aLC.getContainer(getCppuType( (Reference < XClipboardListener > *) 0));
    if (pContainer)
    {
        ClipboardEvent aEvent(static_cast < XClipboard * > (this), m_aContents);
        OInterfaceIteratorHelper aIterator(*pContainer);

        while (aIterator.hasMoreElements())
        {
            Reference < XClipboardListener > xListener(aIterator.next(), UNO_QUERY);
            if (xListener.is())
                xListener->changedContents(aEvent);
        }
    }
}

// ------------------------------------------------------------------------

OUString SAL_CALL GenericClipboard::getName()
    throw(RuntimeException)
{
    return m_aName;
}

// ------------------------------------------------------------------------

sal_Int8 SAL_CALL GenericClipboard::getRenderingCapabilities()
    throw(RuntimeException)
{
    return RenderingCapabilities::Delayed;
}


// ------------------------------------------------------------------------

void SAL_CALL GenericClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bInDispose, "do not add listeners in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.addInterface( getCppuType( (const ::com::sun::star::uno::Reference< XClipboardListener > *) 0), listener );
}

// ------------------------------------------------------------------------

void SAL_CALL GenericClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.removeInterface( getCppuType( (const Reference< XClipboardListener > *) 0 ), listener ); \
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL GenericClipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.GenericClipboard");
    return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL GenericClipboard_createInstance(
    const Reference< XMultiServiceFactory > & /*xMultiServiceFactory*/)
{
    return Reference < XInterface >( ( OWeakObject * ) new GenericClipboard());
}
