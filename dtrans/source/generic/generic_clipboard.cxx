/*************************************************************************
 *
 *  $RCSfile: generic_clipboard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 14:26:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
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

#include <generic_clipboard.hxx>

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_RENDERINGCAPABILITIES_HPP_
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;

OClipboard::OClipboard() :
    m_aMutex(),
    WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization > (m_aMutex),
    m_bInitialized(sal_False)
{
}

// ------------------------------------------------------------------------

OClipboard::OClipboard(const ::rtl::OUString& rName) :
    m_aMutex(),
    WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization > (m_aMutex),
    m_aName(rName),
    m_bInitialized(sal_True)
{
}

// ------------------------------------------------------------------------

OClipboard::~OClipboard()
{
}

// ------------------------------------------------------------------------

void SAL_CALL OClipboard::initialize( const Sequence< Any >& aArguments )
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

OUString SAL_CALL OClipboard::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString::createFromAscii(GENERIC_CLIPBOARD_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL OClipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence < OUString > SupportedServicesNames = GenericClipboard_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL OClipboard::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return GenericClipboard_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL OClipboard::getContents()
    throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL OClipboard::setContents(const Reference< XTransferable >& xTrans,
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

OUString SAL_CALL OClipboard::getName()
    throw(RuntimeException)
{
    return m_aName;
}

// ------------------------------------------------------------------------

sal_Int8 SAL_CALL OClipboard::getRenderingCapabilities()
    throw(RuntimeException)
{
    return RenderingCapabilities::Delayed;
}


// ------------------------------------------------------------------------

void SAL_CALL OClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bInDispose, "do not add listeners in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.addInterface( getCppuType( (const ::com::sun::star::uno::Reference< XClipboardListener > *) 0), listener );
}

// ------------------------------------------------------------------------

void SAL_CALL OClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
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
    const Reference< XMultiServiceFactory > & xMultiServiceFactory)
{
    return Reference < XInterface >( ( OWeakObject * ) new OClipboard());
}
