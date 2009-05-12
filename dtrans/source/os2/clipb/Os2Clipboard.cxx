/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Os2Clipboard.cxx,v $
 * $Revision: 1.5 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include "Os2Clipboard.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::datatransfer::clipboard::RenderingCapabilities;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace os2;

const Type CPPUTYPE_SEQINT8  = getCppuType( ( Sequence< sal_Int8 >* )0 );
const Type CPPUTYPE_OUSTRING = getCppuType( (OUString*)0 );

#define DTRANS_OBJ_CLASSNAME "DTRANSOBJWND"

// -----------------------------------------------------------------------

inline void SetWindowPtr( HWND hWnd, Os2Clipboard* pThis )
{
    WinSetWindowULong( hWnd, QWL_USER, (ULONG)pThis );
}

inline Os2Clipboard* GetWindowPtr( HWND hWnd )
{
    return (Os2Clipboard*)WinQueryWindowULong( hWnd, QWL_USER );
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY DtransObjWndProc( HWND hWnd, ULONG nMsg, MPARAM nMP1, MPARAM nMP2 )
{

    switch ( nMsg )
    {
    case WM_DRAWCLIPBOARD:  // clipboard content has changed
        {
            Os2Clipboard* os2Clipboard = GetWindowPtr( hWnd);
            if (os2Clipboard)
            {
                //MutexGuard aGuard(os2Clipboard->m_aMutex);
                debug_printf("WM_DRAWCLIPBOARD os2Clipboard %08x\n", os2Clipboard);
                if (os2Clipboard->m_bInSetClipboardData)
                {
                    debug_printf("WM_DRAWCLIPBOARD our change\n");
                }
                else
                {
                    // notify listener for clipboard change
                    debug_printf("WM_DRAWCLIPBOARD notify change\n");
                    os2Clipboard->notifyAllClipboardListener();
                }
            }
        }
        break;
    }

    return WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );
}

// -----------------------------------------------------------------------

Os2Clipboard::Os2Clipboard() :
    m_aMutex(),
    WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization > (m_aMutex),
    m_bInitialized(sal_False),
    m_bInSetClipboardData(sal_False)
{
    MutexGuard aGuard(m_aMutex);

    debug_printf("Os2Clipboard::Os2Clipboard\n");
    hAB = WinQueryAnchorBlock( HWND_DESKTOP );
    hText = 0;
    hBitmap = 0;

#if 0
    // register object class
    if ( WinRegisterClass( hAB, (PSZ)DTRANS_OBJ_CLASSNAME,
                            (PFNWP)DtransObjWndProc, 0, sizeof(ULONG) ))
    {
        APIRET  rc;
        // create object window to get clip viewer messages
        hObjWnd = WinCreateWindow( HWND_OBJECT, (PCSZ)DTRANS_OBJ_CLASSNAME,
                                        (PCSZ)"", 0, 0, 0, 0, 0,
                                        HWND_OBJECT, HWND_TOP,
                                        222, NULL, NULL);
        // store pointer
        SetWindowPtr( hObjWnd, this);
        // register the viewer window
        rc = WinOpenClipbrd(hAB);
        rc = WinSetClipbrdViewer(hAB, hObjWnd);
        rc = WinCloseClipbrd(hAB);
    }
#endif

}

Os2Clipboard::~Os2Clipboard()
{
    debug_printf("Os2Clipboard::~Os2Clipboard\n");
}

void SAL_CALL Os2Clipboard::initialize( const Sequence< Any >& aArguments )
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

OUString SAL_CALL Os2Clipboard::getImplementationName() throw( RuntimeException )
{
    debug_printf("Os2Clipboard::getImplementationName\n");
    return OUString::createFromAscii( OS2_CLIPBOARD_IMPL_NAME );
}

sal_Bool SAL_CALL Os2Clipboard::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
    debug_printf("Os2Clipboard::supportsService\n");
    Sequence < OUString > SupportedServicesNames = Os2Clipboard_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

Sequence< OUString > SAL_CALL Os2Clipboard::getSupportedServiceNames() throw( RuntimeException )
{
    debug_printf("Os2Clipboard::getSupportedServiceNames\n");
    return Os2Clipboard_getSupportedServiceNames();
}

Reference< XTransferable > SAL_CALL Os2Clipboard::getContents() throw( RuntimeException )
{
    debug_printf("Os2Clipboard::getContents\n");
    MutexGuard aGuard(m_aMutex);

    // os2 can have only one viewer at time, and we don't get a notification
    // when the viewer changes. So we need to check handles of clipboard
    // data and compare with previous handles
    if (UWinOpenClipbrd(hAB)) {
        sal_Bool    fireChanged = sal_False;
        ULONG handle = UWinQueryClipbrdData( hAB, UCLIP_CF_UNICODETEXT);
        if (handle) {
            if (handle != hText) {
                hText = handle;
                fireChanged = sal_True;
            }
        }
        handle = UWinQueryClipbrdData( hAB, UCLIP_CF_BITMAP);
        if (handle) {
            if (handle != hBitmap) {
                hBitmap = handle;
                fireChanged = sal_True;
            }
        }
        UWinCloseClipbrd( hAB);
        if (fireChanged)
        {
            // notify listener for clipboard change
            debug_printf("Os2Clipboard::getContents notify change\n");
            notifyAllClipboardListener();
        }
    }

    if( ! m_aContents.is() )
        m_aContents = new Os2Transferable( static_cast< OWeakObject* >(this) );

    return m_aContents;
}

void SAL_CALL Os2Clipboard::setContents( const Reference< XTransferable >& xTrans, const Reference< XClipboardOwner >& xClipboardOwner ) throw( RuntimeException )
{
    debug_printf("Os2Clipboard::setContents\n");
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

#if OSL_DEBUG_LEVEL>0
    // dump list of available mimetypes
    Sequence< DataFlavor > aFlavors( m_aContents->getTransferDataFlavors() );
    for( int i = 0; i < aFlavors.getLength(); i++ )
        debug_printf("Os2Clipboard::setContents available mimetype: %d %s\n",
            i, CHAR_POINTER(aFlavors.getConstArray()[i].MimeType));
#endif

    // we can only export text or bitmap
    DataFlavor nFlavorText( OUString::createFromAscii( "text/plain;charset=utf-16" ),
                        OUString::createFromAscii( "Unicode-Text" ), CPPUTYPE_OUSTRING);
    DataFlavor nFlavorBitmap( OUString::createFromAscii( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" ),
                        OUString::createFromAscii( "Bitmap" ), CPPUTYPE_DEFAULT);

    // try text transfer data (if any)
    PSZ pSharedText = NULL;
    HBITMAP hbm = NULL;
    try
    {
        Any aAny = m_aContents->getTransferData( nFlavorText );
        if (aAny.hasValue())
        {
            APIRET rc;
            // copy unicode text to clipboard
            OUString aString;
            aAny >>= aString;
            // share text
            rc = DosAllocSharedMem( (PPVOID) &pSharedText, NULL,
                aString.getLength() * 2 + 2,
                PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE | OBJ_ANY);
            if (!rc)
                memcpy( pSharedText, aString.getStr(), aString.getLength() * 2 + 2 );
            else
                pSharedText = NULL;
            debug_printf("Os2Clipboard::setContents SetClipbrdData text done\n");
        }
    } catch ( UnsupportedFlavorException&) {
        debug_printf("Os2Clipboard::setContents UnsupportedFlavorException (no text)\n");
    }

    // try bitmap transfer data (if any)
    try
    {
        Any aAnyB = m_aContents->getTransferData( nFlavorBitmap );
        if (aAnyB.hasValue())
        {
            hbm = OOoBmpToOS2Handle( aAnyB);
            debug_printf("Os2Clipboard::setContents SetClipbrdData bitmap done\n");
        }
    } catch ( UnsupportedFlavorException&) {
        debug_printf("Os2Clipboard::setContents UnsupportedFlavorException (no bitmap)\n");
    }

    // copy to clipboard
    if ( UWinOpenClipbrd( hAB) && (pSharedText || hbm))
    {
        // set the flag, so we will ignore the next WM_DRAWCLIPBOARD
        // since we generate it with following code.
        m_bInSetClipboardData = sal_True;
        UWinEmptyClipbrd( hAB);
        // give pointer to clipboard (it will become owner of pSharedText!)
        if (pSharedText) {
            UWinSetClipbrdData( hAB, (ULONG) pSharedText, UCLIP_CF_UNICODETEXT, CFI_POINTER);
            // update internal handle to avoid detection of this text as new data
            hText = (ULONG)pSharedText;
        }
        // give bitmap to clipboard
        if (hbm) {
            UWinSetClipbrdData( hAB, (ULONG) hbm, UCLIP_CF_BITMAP, CFI_HANDLE);
            // update internal handle to avoid detection of this bitmap as new data
            hBitmap = hbm;
        }
        // reset the flag, so we will not ignore next WM_DRAWCLIPBOARD
        m_bInSetClipboardData = sal_False;
        UWinCloseClipbrd( hAB);
    }

}

OUString SAL_CALL Os2Clipboard::getName() throw( RuntimeException )
{
    debug_printf("Os2Clipboard::getName\n");
    return m_aName;
}

sal_Int8 SAL_CALL Os2Clipboard::getRenderingCapabilities() throw( RuntimeException )
{
    debug_printf("Os2Clipboard::getRenderingCapabilities\n");
    return Delayed;
}

//========================================================================
// XClipboardNotifier
//========================================================================

void SAL_CALL Os2Clipboard::addClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
    debug_printf("Os2Clipboard::addClipboardListener\n");
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bInDispose, "do not add listeners in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.addInterface( getCppuType( (const ::com::sun::star::uno::Reference< XClipboardListener > *) 0), listener );
}

void SAL_CALL Os2Clipboard::removeClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
    debug_printf("Os2Clipboard::removeClipboardListener\n");
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.removeInterface( getCppuType( (const Reference< XClipboardListener > *) 0 ), listener ); \
}

// ------------------------------------------------------------------------

void SAL_CALL Os2Clipboard::notifyAllClipboardListener( )
{
    if ( !rBHelper.bDisposed )
    {
        ClearableMutexGuard aGuard( rBHelper.rMutex );
        if ( !rBHelper.bDisposed )
        {
            aGuard.clear( );

            ClearableMutexGuard aGuard(m_aMutex);
            // copy member references on stack so they can be called
            // without having the mutex
            Reference< XClipboardOwner > xOwner( m_aOwner );
            Reference< XTransferable > xTrans( m_aContents );
            // clear members
            m_aOwner.clear();
            m_aContents.clear();
            // release the mutex
            aGuard.clear();

            // inform previous owner of lost ownership
            if ( xOwner.is() )
                xOwner->lostOwnership(static_cast < XClipboard * > (this), m_aContents);

            OInterfaceContainerHelper* pICHelper = rBHelper.aLC.getContainer(
                getCppuType( ( Reference< XClipboardListener > * ) 0 ) );

            if ( pICHelper )
            {
                try
                {
                    OInterfaceIteratorHelper iter(*pICHelper);
                    m_aContents = 0;
                    m_aContents = new Os2Transferable( static_cast< OWeakObject* >(this) );
                    ClipboardEvent aClipbEvent(static_cast<XClipboard*>(this), m_aContents);

                    while(iter.hasMoreElements())
                    {
                        try
                        {
                            Reference<XClipboardListener> xCBListener(iter.next(), UNO_QUERY);
                            if (xCBListener.is())
                                xCBListener->changedContents(aClipbEvent);
                        }
                        catch(RuntimeException&)
                        {
                            OSL_ENSURE( false, "RuntimeException caught" );
                            debug_printf( "RuntimeException caught" );
                        }
                    }
                }
                catch(const ::com::sun::star::lang::DisposedException&)
                {
                    OSL_ENSURE(false, "Service Manager disposed");
                    debug_printf( "Service Manager disposed");

                    // no further clipboard changed notifications
                    //m_pImpl->unregisterClipboardViewer();
                }

            } // end if
        } // end if
    } // end if
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Os2Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( OS2_CLIPBOARD_SERVICE_NAME );
    return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL Os2Clipboard_createInstance(
    const Reference< XMultiServiceFactory > & xMultiServiceFactory)
{
    return Reference < XInterface >( ( OWeakObject * ) new Os2Clipboard());
}

