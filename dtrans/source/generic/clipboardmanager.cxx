/*************************************************************************
 *
 *  $RCSfile: clipboardmanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2001-07-27 10:03:36 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <clipboardmanager.hxx>

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace std;

using ::dtrans::ClipboardManager;
using ::rtl::OUString;

// ------------------------------------------------------------------------

ClipboardManager::ClipboardManager():
    m_aMutex(),
    WeakComponentImplHelper3< XClipboardManager, XEventListener, XServiceInfo > (m_aMutex),
    m_aDefaultName(OUString::createFromAscii("default"))
{
}

// ------------------------------------------------------------------------

ClipboardManager::~ClipboardManager()
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL ClipboardManager::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString::createFromAscii(CLIPBOARDMANAGER_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL ClipboardManager::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence < OUString > SupportedServicesNames = ClipboardManager_getSupportedServiceNames();

    for ( sal_Int32 n = 0, nmax = SupportedServicesNames.getLength(); n < nmax; n++ )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL ClipboardManager::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return ClipboardManager_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

Reference< XClipboard > SAL_CALL ClipboardManager::getClipboard( const OUString& aName )
    throw(NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    // object is disposed already
    if (rBHelper.bDisposed)
        throw DisposedException(OUString::createFromAscii("object is disposed."),
                                static_cast < XClipboardManager * > (this));

    ClipboardMap::iterator iter =
        m_aClipboardMap.find(aName.getLength() ? aName : m_aDefaultName);

    if (iter != m_aClipboardMap.end())
        return iter->second;

    throw NoSuchElementException(aName, static_cast < XClipboardManager * > (this));
}

// ------------------------------------------------------------------------

void SAL_CALL ClipboardManager::addClipboard( const Reference< XClipboard >& xClipboard )
    throw(IllegalArgumentException, ElementExistException, RuntimeException)
{
    OSL_ASSERT(xClipboard.is());

    // check parameter
    if (!xClipboard.is())
        throw IllegalArgumentException(OUString::createFromAscii("empty reference"),
                                       static_cast < XClipboardManager * > (this), 1);

    // the name "default" is reserved for internal use
    OUString aName = xClipboard->getName();
    if (m_aDefaultName.compareTo(aName) == 0)
        throw IllegalArgumentException(OUString::createFromAscii("name reserved"),
                                       static_cast < XClipboardManager * > (this), 1);

    // try to add new clipboard to the list
    ClearableMutexGuard aGuard(m_aMutex);
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        pair< const OUString, Reference< XClipboard > > value (
            aName.getLength() ? aName : m_aDefaultName,
            xClipboard );

        pair< ClipboardMap::iterator, bool > p = m_aClipboardMap.insert(value);
        aGuard.clear();

        // insert failed, element must exist already
        if (!p.second)
            throw ElementExistException(aName, static_cast < XClipboardManager * > (this));

        // request disposing notifications
        Reference< XComponent > xComponent(xClipboard, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(static_cast < XEventListener * > (this));
    }
}

// ------------------------------------------------------------------------

void SAL_CALL ClipboardManager::removeClipboard( const OUString& aName )
     throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (!rBHelper.bDisposed)
        m_aClipboardMap.erase(aName.getLength() ? aName : m_aDefaultName );
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL ClipboardManager::listClipboardNames()
    throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    if (rBHelper.bDisposed)
        throw DisposedException(OUString::createFromAscii("object is disposed."),
                                static_cast < XClipboardManager * > (this));

    if (rBHelper.bInDispose)
        return Sequence< OUString > ();

    Sequence< OUString > aRet(m_aClipboardMap.size());
    ClipboardMap::iterator iter = m_aClipboardMap.begin();
    ClipboardMap::iterator imax = m_aClipboardMap.end();

    for (sal_Int32 n = 0; iter != imax; iter++)
        aRet[n++] = iter->first;

    return aRet;
}

// ------------------------------------------------------------------------

void SAL_CALL ClipboardManager::dispose()
    throw(RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        rBHelper.bInDispose = sal_True;
        aGuard.clear();

        // give everyone a chance to save his clipboard instance
        EventObject aEvt(static_cast < XClipboardManager * > (this));
        rBHelper.aLC.disposeAndClear( aEvt );

        // removeClipboard is still allowed here,  so make a copy of the
        // list (to ensure integrety) and clear the original.
        ClearableMutexGuard aGuard2( rBHelper.rMutex );
        ClipboardMap aCopy(m_aClipboardMap);
        m_aClipboardMap.clear();
        aGuard2.clear();

        // dispose all clipboards still in list
        ClipboardMap::iterator iter = aCopy.begin();
        ClipboardMap::iterator imax = aCopy.end();

        for (; iter != imax; iter++)
        {
            Reference< XComponent > xComponent(iter->second, UNO_QUERY);
            if (xComponent.is())
            {
                try
                {
                    xComponent->removeEventListener(static_cast < XEventListener * > (this));
                    xComponent->dispose();
                }

                catch(Exception e)
                {
                    // exceptions can be safely ignored here.
                }
            }
        }

        rBHelper.bDisposed = sal_True;
        rBHelper.bInDispose = sal_False;
    }
}

// ------------------------------------------------------------------------

void SAL_CALL  ClipboardManager::disposing( const EventObject& event )
    throw(RuntimeException)
{
    Reference < XClipboard > xClipboard(event.Source, UNO_QUERY);

    if (xClipboard.is())
        removeClipboard(xClipboard->getName());
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL ClipboardManager_createInstance(
    const Reference< XMultiServiceFactory > & xMultiServiceFactory)
{
    return Reference < XInterface >( ( OWeakObject * ) new ClipboardManager());
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL ClipboardManager_getSupportedServiceNames()
{
    Sequence < OUString > SupportedServicesNames( 1 );
    SupportedServicesNames[0] =
        OUString::createFromAscii("com.sun.star.datatransfer.clipboard.ClipboardManager");
    return SupportedServicesNames;
}





