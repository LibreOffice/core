/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <cstdlib>

#include <unx/saldisp.hxx>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#if defined(LINUX) || defined(NETBSD) || defined (FREEBSD) || defined(OPENBSD)
#include <sys/poll.h>
#else
#include <poll.h>
#endif
#include <sal/alloca.h>
#include <sal/macros.h>

#include "X11_selection.hxx"
#include "X11_clipboard.hxx"
#include "X11_transferable.hxx"
#include "X11_dndcontext.hxx"
#include "bmp.hxx"

#include <vcl/svapp.hxx>

// pointer bitmaps
#include "copydata_curs.h"
#include "copydata_mask.h"
#include "movedata_curs.h"
#include "movedata_mask.h"
#include "linkdata_curs.h"
#include "linkdata_mask.h"
#include "nodrop_curs.h"
#include "nodrop_mask.h"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <rtl/tencinfo.h>
#include <osl/process.h>

#include <comphelper/processfactory.hxx>
#include <comphelper/solarmutex.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <algorithm>

static constexpr auto DRAG_EVENT_MASK = ButtonPressMask |
                              ButtonReleaseMask |
                              PointerMotionMask |
                              EnterWindowMask |
                              LeaveWindowMask;

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace cppu;

using namespace x11;

// stubs to satisfy solaris compiler's rather rigid linking warning
extern "C"
{
    static void call_SelectionManager_run( void * pMgr )
    {
        SelectionManager::run( pMgr );
    }

    static void call_SelectionManager_runDragExecute( void * pMgr )
    {
        osl_setThreadName("SelectionManager::runDragExecute()");
        SelectionManager::runDragExecute( pMgr );
    }
}

static const long nXdndProtocolRevision = 5;

// mapping between mime types (or what the office thinks of mime types)
// and X convention types
struct NativeTypeEntry
{
    Atom            nAtom;
    const char*     pType;              // Mime encoding on our side
    const char*     pNativeType;        // string corresponding to nAtom for the case of nAtom being uninitialized
    int const       nFormat;            // the corresponding format
};

// the convention for Xdnd is mime types as specified by the corresponding
// RFC's with the addition that text/plain without charset tag contains iso8859-1
// sadly some applications (e.g. gtk) do not honor the mimetype only rule,
// so for compatibility add UTF8_STRING
static NativeTypeEntry aXdndConversionTab[] =
{
    { 0, "text/plain;charset=iso8859-1", "text/plain", 8 },
    { 0, "text/plain;charset=utf-8", "UTF8_STRING", 8 }
};

// for clipboard and primary selections there is only a convention for text
// that the encoding name of the text is taken as type in all capitalized letters
static NativeTypeEntry aNativeConversionTab[] =
{
    { 0, "text/plain;charset=utf-16", "ISO10646-1", 16 },
    { 0, "text/plain;charset=utf-8", "UTF8_STRING", 8 },
    { 0, "text/plain;charset=utf-8", "UTF-8", 8 },
    { 0, "text/plain;charset=utf-8", "text/plain;charset=UTF-8", 8 },
    // ISO encodings
    { 0, "text/plain;charset=iso8859-2", "ISO8859-2", 8 },
    { 0, "text/plain;charset=iso8859-3", "ISO8859-3", 8 },
    { 0, "text/plain;charset=iso8859-4", "ISO8859-4", 8 },
    { 0, "text/plain;charset=iso8859-5", "ISO8859-5", 8 },
    { 0, "text/plain;charset=iso8859-6", "ISO8859-6", 8 },
    { 0, "text/plain;charset=iso8859-7", "ISO8859-7", 8 },
    { 0, "text/plain;charset=iso8859-8", "ISO8859-8", 8 },
    { 0, "text/plain;charset=iso8859-9", "ISO8859-9", 8 },
    { 0, "text/plain;charset=iso8859-10", "ISO8859-10", 8 },
    { 0, "text/plain;charset=iso8859-13", "ISO8859-13", 8 },
    { 0, "text/plain;charset=iso8859-14", "ISO8859-14", 8 },
    { 0, "text/plain;charset=iso8859-15", "ISO8859-15", 8 },
    // asian encodings
    { 0, "text/plain;charset=jisx0201.1976-0", "JISX0201.1976-0", 8 },
    { 0, "text/plain;charset=jisx0208.1983-0", "JISX0208.1983-0", 8 },
    { 0, "text/plain;charset=jisx0208.1990-0", "JISX0208.1990-0", 8 },
    { 0, "text/plain;charset=jisx0212.1990-0", "JISX0212.1990-0", 8 },
    { 0, "text/plain;charset=gb2312.1980-0", "GB2312.1980-0", 8 },
    { 0, "text/plain;charset=ksc5601.1992-0", "KSC5601.1992-0", 8 },
    // eastern european encodings
    { 0, "text/plain;charset=koi8-r", "KOI8-R", 8 },
    { 0, "text/plain;charset=koi8-u", "KOI8-U", 8 },
    // String (== iso8859-1)
    { XA_STRING, "text/plain;charset=iso8859-1", "STRING", 8 },
    // special for compound text
    { 0, "text/plain;charset=compound_text", "COMPOUND_TEXT", 8 },

    // PIXMAP
    { XA_PIXMAP, "image/bmp", "PIXMAP", 32 }
};

rtl_TextEncoding x11::getTextPlainEncoding( const OUString& rMimeType )
{
    rtl_TextEncoding aEncoding = RTL_TEXTENCODING_DONTKNOW;
    OUString aMimeType( rMimeType.toAsciiLowerCase() );
    sal_Int32 nIndex = 0;
    if( aMimeType.getToken( 0, ';', nIndex ) == "text/plain" )
    {
        if( aMimeType.getLength() == 10 ) // only "text/plain"
            aEncoding = RTL_TEXTENCODING_ISO_8859_1;
        else
        {
            while( nIndex != -1 )
            {
                OUString aToken = aMimeType.getToken( 0, ';', nIndex );
                sal_Int32 nPos = 0;
                if( aToken.getToken( 0, '=', nPos ) == "charset" )
                {
                    OString aEncToken = OUStringToOString( aToken.getToken( 0, '=', nPos ), RTL_TEXTENCODING_ISO_8859_1 );
                    aEncoding = rtl_getTextEncodingFromUnixCharset( aEncToken.getStr() );
                    if( aEncoding == RTL_TEXTENCODING_DONTKNOW )
                    {
                        if( aEncToken.equalsIgnoreAsciiCase( "utf-8" ) )
                            aEncoding = RTL_TEXTENCODING_UTF8;
                    }
                    if( aEncoding != RTL_TEXTENCODING_DONTKNOW )
                        break;
                }
            }
        }
    }
#if OSL_DEBUG_LEVEL > 1
    if( aEncoding == RTL_TEXTENCODING_DONTKNOW )
        fprintf( stderr, "getTextPlainEncoding( %s ) failed\n", OUStringToOString( rMimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    return aEncoding;
}

std::unordered_map< OUString, SelectionManager* >& SelectionManager::getInstances()
{
    static std::unordered_map< OUString, SelectionManager* > aInstances;
    return aInstances;
}

SelectionManager::SelectionManager() :
        m_nIncrementalThreshold( 15*1024 ),
        m_pDisplay( nullptr ),
        m_aThread( nullptr ),
        m_aDragExecuteThread( nullptr ),
        m_aWindow( None ),
        m_nSelectionTimeout( 0 ),
        m_nSelectionTimestamp( CurrentTime ),
        m_bDropEnterSent( true ),
        m_aCurrentDropWindow( None ),
        m_nDropTime( None ),
        m_nLastDropAction( 0 ),
        m_nLastX( 0 ),
        m_nLastY( 0 ),
        m_bDropWaitingForCompletion( false ),
        m_aDropWindow( None ),
        m_aDropProxy( None ),
        m_aDragSourceWindow( None ),
        m_nLastDragX( 0 ),
        m_nLastDragY( 0 ),
        m_nNoPosX( 0 ),
        m_nNoPosY( 0 ),
        m_nNoPosWidth( 0 ),
        m_nNoPosHeight( 0 ),
        m_nDragButton( 0 ),
        m_nUserDragAction( 0 ),
        m_nTargetAcceptAction( 0 ),
        m_nSourceActions( 0 ),
        m_bLastDropAccepted( false ),
        m_bDropSuccess( false ),
        m_bDropSent( false ),
        m_nDropTimeout( 0 ),
        m_bWaitingForPrimaryConversion( false ),
        m_aMoveCursor( None ),
        m_aCopyCursor( None ),
        m_aLinkCursor( None ),
        m_aNoneCursor( None ),
        m_aCurrentCursor( None ),
        m_nCurrentProtocolVersion( nXdndProtocolRevision ),
        m_nTARGETSAtom( None ),
        m_nTIMESTAMPAtom( None ),
        m_nTEXTAtom( None ),
        m_nINCRAtom( None ),
        m_nCOMPOUNDAtom( None ),
        m_nMULTIPLEAtom( None ),
        m_nImageBmpAtom( None ),
        m_nXdndAware( None ),
        m_nXdndEnter( None ),
        m_nXdndLeave( None ),
        m_nXdndPosition( None ),
        m_nXdndStatus( None ),
        m_nXdndDrop( None ),
        m_nXdndFinished( None ),
        m_nXdndSelection( None ),
        m_nXdndTypeList( None ),
        m_nXdndProxy( None ),
        m_nXdndActionCopy( None ),
        m_nXdndActionMove( None ),
        m_nXdndActionLink( None ),
        m_nXdndActionAsk( None ),
        m_bShutDown( false )
{
    memset(&m_aDropEnterEvent, 0, sizeof(m_aDropEnterEvent));
    m_aDragRunning.reset();
}

Cursor SelectionManager::createCursor( const unsigned char* pPointerData, const unsigned char* pMaskData, int width, int height, int hotX, int hotY )
{
    Pixmap aPointer;
    Pixmap aMask;
    XColor aBlack, aWhite;

    aBlack.pixel = BlackPixel( m_pDisplay, 0 );
    aBlack.red = aBlack.green = aBlack.blue = 0;
    aBlack.flags = DoRed | DoGreen | DoBlue;

    aWhite.pixel = WhitePixel( m_pDisplay, 0 );
    aWhite.red = aWhite.green = aWhite.blue = 0xffff;
    aWhite.flags = DoRed | DoGreen | DoBlue;

    aPointer =
        XCreateBitmapFromData( m_pDisplay,
                               m_aWindow,
                               reinterpret_cast<const char*>(pPointerData),
                               width,
                               height );
    aMask
        = XCreateBitmapFromData( m_pDisplay,
                                 m_aWindow,
                                 reinterpret_cast<const char*>(pMaskData),
                                 width,
                                 height );
    Cursor aCursor =
        XCreatePixmapCursor( m_pDisplay, aPointer, aMask,
                             &aBlack, &aWhite,
                             hotX,
                             hotY );
    XFreePixmap( m_pDisplay, aPointer );
    XFreePixmap( m_pDisplay, aMask );

    return aCursor;
}

void SelectionManager::initialize( const Sequence< Any >& arguments )
{
    osl::MutexGuard aGuard(m_aMutex);

    if( ! m_xDisplayConnection.is() )
    {
        /*
         *  first argument must be a css::awt::XDisplayConnection
         *  from this we will get the XEvents of the vcl event loop by
         *  registering us as XEventHandler on it.
         *
         *  implementor's note:
         *  FIXME:
         *  finally the clipboard and XDND service is back in the module it belongs
         *  now cleanup and sharing of resources with the normal vcl event loop
         *  needs to be added. The display used whould be that of the normal event loop
         *  and synchronization should be done via the SolarMutex.
         */
        if( arguments.getLength() > 0 )
            arguments.getConstArray()[0] >>= m_xDisplayConnection;
        if( ! m_xDisplayConnection.is() )
        {
        }
        else
            m_xDisplayConnection->addEventHandler( Any(), this, ~0 );
    }

    if( ! m_pDisplay )
    {
        OUString aUDisplay;
        if( m_xDisplayConnection.is() )
        {
            Any aIdentifier;
            aIdentifier = m_xDisplayConnection->getIdentifier();
            aIdentifier >>= aUDisplay;
        }

        OString aDisplayName( OUStringToOString( aUDisplay, RTL_TEXTENCODING_ISO_8859_1 ) );

        m_pDisplay = XOpenDisplay( aDisplayName.isEmpty() ? nullptr : aDisplayName.getStr());

        if( m_pDisplay )
        {
#ifdef SYNCHRONIZE
            XSynchronize( m_pDisplay, True );
#endif
            // special targets
            m_nTARGETSAtom      = getAtom( "TARGETS" );
            m_nTIMESTAMPAtom    = getAtom( "TIMESTAMP" );
            m_nTEXTAtom         = getAtom( "TEXT" );
            m_nINCRAtom         = getAtom( "INCR" );
            m_nCOMPOUNDAtom     = getAtom( "COMPOUND_TEXT" );
            m_nMULTIPLEAtom     = getAtom( "MULTIPLE" );
            m_nImageBmpAtom     = getAtom( "image/bmp" );

            // Atoms for Xdnd protocol
            m_nXdndAware        = getAtom( "XdndAware" );
            m_nXdndEnter        = getAtom( "XdndEnter" );
            m_nXdndLeave        = getAtom( "XdndLeave" );
            m_nXdndPosition     = getAtom( "XdndPosition" );
            m_nXdndStatus       = getAtom( "XdndStatus" );
            m_nXdndDrop         = getAtom( "XdndDrop" );
            m_nXdndFinished     = getAtom( "XdndFinished" );
            m_nXdndSelection    = getAtom( "XdndSelection" );
            m_nXdndTypeList     = getAtom( "XdndTypeList" );
            m_nXdndProxy        = getAtom( "XdndProxy" );
            m_nXdndActionCopy   = getAtom( "XdndActionCopy" );
            m_nXdndActionMove   = getAtom( "XdndActionMove" );
            m_nXdndActionLink   = getAtom( "XdndActionLink" );
            m_nXdndActionAsk    = getAtom( "XdndActionAsk" );

            // initialize map with member none
            m_aAtomToString[ 0 ]= "None";
            m_aAtomToString[ XA_PRIMARY ] = "PRIMARY";

            // create a (invisible) message window
            m_aWindow = XCreateSimpleWindow( m_pDisplay, DefaultRootWindow( m_pDisplay ),
                                             10, 10, 10, 10, 0, 0, 1 );

            // initialize threshold for incremental transfers
            // ICCCM says it should be smaller that the max request size
            // which in turn is guaranteed to be at least 16k bytes
            m_nIncrementalThreshold = XMaxRequestSize( m_pDisplay ) - 1024;

            if( m_aWindow )
            {
                // initialize default cursors
                m_aMoveCursor = createCursor( movedata_curs_bits,
                                              movedata_mask_bits,
                                              movedata_curs_width,
                                              movedata_curs_height,
                                              movedata_curs_x_hot,
                                              movedata_curs_y_hot );
                m_aCopyCursor = createCursor( copydata_curs_bits,
                                              copydata_mask_bits,
                                              copydata_curs_width,
                                              copydata_curs_height,
                                              copydata_curs_x_hot,
                                              copydata_curs_y_hot );
                m_aLinkCursor = createCursor( linkdata_curs_bits,
                                              linkdata_mask_bits,
                                              linkdata_curs_width,
                                              linkdata_curs_height,
                                              linkdata_curs_x_hot,
                                              linkdata_curs_y_hot );
                m_aNoneCursor = createCursor( nodrop_curs_bits,
                                              nodrop_mask_bits,
                                              nodrop_curs_width,
                                              nodrop_curs_height,
                                              nodrop_curs_x_hot,
                                              nodrop_curs_y_hot );

                // just interested in SelectionClear/Notify/Request and PropertyChange
                XSelectInput( m_pDisplay, m_aWindow, PropertyChangeMask );
                // create the transferable for Drag operations
                m_xDropTransferable = new X11Transferable( *this, m_nXdndSelection );
                registerHandler( m_nXdndSelection, *this );

                m_aThread = osl_createSuspendedThread( call_SelectionManager_run, this );
                if( m_aThread )
                    osl_resumeThread( m_aThread );
#if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "SelectionManager::initialize: creation of dispatch thread failed !\n" );
#endif

                if (pipe(m_EndThreadPipe) != 0) {
                    #if OSL_DEBUG_LEVEL > 1
                    fprintf(stderr, "Failed to create endThreadPipe\n");
                    #endif
                    m_EndThreadPipe[0] = m_EndThreadPipe[1] = 0;
                }
            }
        }
    }
}

SelectionManager::~SelectionManager()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SelectionManager::~SelectionManager (%s)\n", m_pDisplay ? DisplayString(m_pDisplay) : "no display" );
#endif
    {
        osl::MutexGuard aGuard( *osl::Mutex::getGlobalMutex() );

        auto it = std::find_if(getInstances().begin(), getInstances().end(),
            [&](const std::pair< OUString, SelectionManager* >& rInstance) { return rInstance.second == this; });
        if( it != getInstances().end() )
            getInstances().erase( it );
    }

    if( m_aThread )
    {
        osl_terminateThread( m_aThread );
        osl_joinWithThread( m_aThread );
        osl_destroyThread( m_aThread );
    }

    if( m_aDragExecuteThread )
    {
        osl_terminateThread( m_aDragExecuteThread );
        osl_joinWithThread( m_aDragExecuteThread );
        m_aDragExecuteThread = nullptr;
        // thread handle is freed in dragDoDispatch()
    }

    osl::MutexGuard aGuard(m_aMutex);

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "shutting down SelectionManager\n" );
#endif

    if( m_pDisplay )
    {
        deregisterHandler( m_nXdndSelection );
        // destroy message window
        if( m_aWindow )
            XDestroyWindow( m_pDisplay, m_aWindow );
        // release cursors
        if (m_aMoveCursor != None)
            XFreeCursor(m_pDisplay, m_aMoveCursor);
        if (m_aCopyCursor != None)
            XFreeCursor(m_pDisplay, m_aCopyCursor);
        if (m_aLinkCursor != None)
            XFreeCursor(m_pDisplay, m_aLinkCursor);
        if (m_aNoneCursor != None)
            XFreeCursor(m_pDisplay, m_aNoneCursor);

        // paranoia setting, the drag thread should have
        // done that already
        XUngrabPointer( m_pDisplay, CurrentTime );
        XUngrabKeyboard( m_pDisplay, CurrentTime );

        XCloseDisplay( m_pDisplay );
    }
}

SelectionAdaptor* SelectionManager::getAdaptor( Atom selection )
{
    std::unordered_map< Atom, Selection* >::iterator it =
          m_aSelections.find( selection );
    return it != m_aSelections.end() ? it->second->m_pAdaptor : nullptr;
}

OUString SelectionManager::convertFromCompound( const char* pText, int nLen )
{
    osl::MutexGuard aGuard( m_aMutex );
    OUStringBuffer aRet;
    if( nLen < 0 )
        nLen = strlen( pText );

    char** pTextList = nullptr;
    int nTexts = 0;

    XTextProperty aProp;
    aProp.value     = reinterpret_cast<unsigned char *>(const_cast<char *>(pText));
    aProp.encoding  = m_nCOMPOUNDAtom;
    aProp.format    = 8;
    aProp.nitems    = nLen;
    XmbTextPropertyToTextList( m_pDisplay,
                               &aProp,
                               &pTextList,
                               &nTexts );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    for( int i = 0; i < nTexts; i++ )
        aRet.append(OStringToOUString( pTextList[i], aEncoding ));

    if( pTextList )
        XFreeStringList( pTextList );

    return aRet.makeStringAndClear();
}

OString SelectionManager::convertToCompound( const OUString& rText )
{
    osl::MutexGuard aGuard( m_aMutex );
    XTextProperty aProp;
    aProp.value = nullptr;
    aProp.encoding = XA_STRING;
    aProp.format = 8;
    aProp.nitems = 0;

    OString aRet( rText.getStr(), rText.getLength(), osl_getThreadTextEncoding() );
    char* pT = const_cast<char*>(aRet.getStr());

    XmbTextListToTextProperty( m_pDisplay,
                               &pT,
                               1,
                               XCompoundTextStyle,
                               &aProp );
    if( aProp.value )
    {
        aRet = reinterpret_cast<char*>(aProp.value);
        XFree( aProp.value );
#ifdef __sun
        /*
         *  for currently unknown reasons XmbTextListToTextProperty on Solaris returns
         *  no data in ISO8859-n encodings (at least for n = 1, 15)
         *  in these encodings the directly converted text does the
         *  trick, also.
         */
        if( aRet.isEmpty() && !rText.isEmpty() )
            aRet = OUStringToOString( rText, osl_getThreadTextEncoding() );
#endif
    }
    else
        aRet.clear();

    return aRet;
}

bool SelectionManager::convertData(
                                   const css::uno::Reference< XTransferable >& xTransferable,
                                   Atom nType,
                                   Atom nSelection,
                                   int& rFormat,
                                   Sequence< sal_Int8 >& rData )
{
    bool bSuccess = false;

    if( ! xTransferable.is() )
        return bSuccess;

    try
    {

        DataFlavor aFlavor;
        aFlavor.MimeType = convertTypeFromNative( nType, nSelection, rFormat );

        sal_Int32 nIndex = 0;
        if( aFlavor.MimeType.getToken( 0, ';', nIndex ) == "text/plain" )
        {
            if( aFlavor.MimeType.getToken( 0, ';', nIndex ) == "charset=utf-16" )
                aFlavor.DataType = cppu::UnoType<OUString>::get();
            else
                aFlavor.DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();
        }
        else
            aFlavor.DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

        if( xTransferable->isDataFlavorSupported( aFlavor ) )
        {
            Any aValue( xTransferable->getTransferData( aFlavor ) );
            if( aValue.getValueTypeClass() == TypeClass_STRING )
            {
                OUString aString;
                aValue >>= aString;
                rData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aString.getStr()), aString.getLength() * sizeof( sal_Unicode ) );
                bSuccess = true;
            }
            else if( aValue.getValueType() == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            {
                aValue >>= rData;
                bSuccess = true;
            }
        }
        else if( aFlavor.MimeType.startsWith("text/plain") )
        {
            rtl_TextEncoding aEncoding = RTL_TEXTENCODING_DONTKNOW;
            bool bCompoundText = false;
            if( nType == m_nCOMPOUNDAtom )
                bCompoundText = true;
            else
                aEncoding = getTextPlainEncoding( aFlavor.MimeType );
            if( aEncoding != RTL_TEXTENCODING_DONTKNOW || bCompoundText )
            {
                aFlavor.MimeType = "text/plain;charset=utf-16";
                aFlavor.DataType = cppu::UnoType<OUString>::get();
                if( xTransferable->isDataFlavorSupported( aFlavor ) )
                {
                    Any aValue( xTransferable->getTransferData( aFlavor ) );
                    OUString aString;
                    aValue >>= aString;
                    OString aByteString( bCompoundText ? convertToCompound( aString ) : OUStringToOString( aString, aEncoding ) );
                    rData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aByteString.getStr()), aByteString.getLength() * sizeof( sal_Char ) );
                    bSuccess = true;
                }
            }
        }
    }
    // various exceptions possible ... which all lead to a failed conversion
    // so simplify here to a catch all
    catch(...)
    {
    }

    return bSuccess;
}

SelectionManager& SelectionManager::get( const OUString& rDisplayName )
{
    osl::MutexGuard aGuard( *osl::Mutex::getGlobalMutex() );

    OUString aDisplayName( rDisplayName );
    if( aDisplayName.isEmpty() )
        aDisplayName = OStringToOUString( getenv( "DISPLAY" ), RTL_TEXTENCODING_ISO_8859_1 );
    SelectionManager* pInstance = nullptr;

    std::unordered_map< OUString, SelectionManager* >::iterator it = getInstances().find( aDisplayName );
    if( it != getInstances().end() )
        pInstance = it->second;
    else pInstance = getInstances()[ aDisplayName ] = new SelectionManager();

    return *pInstance;
}

OUString SelectionManager::getString( Atom aAtom )
{
    osl::MutexGuard aGuard(m_aMutex);

    if( m_aAtomToString.find( aAtom ) == m_aAtomToString.end() )
    {
        char* pAtom = m_pDisplay ? XGetAtomName( m_pDisplay, aAtom ) : nullptr;
        if( ! pAtom )
            return OUString();
        OUString aString( OStringToOUString( pAtom, RTL_TEXTENCODING_ISO_8859_1 ) );
        XFree( pAtom );
        m_aStringToAtom[ aString ] = aAtom;
        m_aAtomToString[ aAtom ] = aString;
    }
    return m_aAtomToString[ aAtom ];
}

Atom SelectionManager::getAtom( const OUString& rString )
{
    osl::MutexGuard aGuard(m_aMutex);

    if( m_aStringToAtom.find( rString ) == m_aStringToAtom.end() )
    {
        static Atom nNoDisplayAtoms = 1;
        Atom aAtom = m_pDisplay ? XInternAtom( m_pDisplay, OUStringToOString( rString, RTL_TEXTENCODING_ISO_8859_1 ).getStr(), False ) : nNoDisplayAtoms++;
        m_aStringToAtom[ rString ] = aAtom;
        m_aAtomToString[ aAtom ] = rString;
    }
    return m_aStringToAtom[ rString ];
}

bool SelectionManager::requestOwnership( Atom selection )
{
    bool bSuccess = false;
    if( m_pDisplay && m_aWindow )
    {
        osl::MutexGuard aGuard(m_aMutex);

        SelectionAdaptor* pAdaptor = getAdaptor( selection );
        if( pAdaptor )
        {
            XSetSelectionOwner( m_pDisplay, selection, m_aWindow, CurrentTime );
            if( XGetSelectionOwner( m_pDisplay, selection ) == m_aWindow )
                bSuccess = true;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "%s ownership for selection %s\n",
                     bSuccess ? "acquired" : "failed to acquire",
                     OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
            Selection* pSel = m_aSelections[ selection ];
            pSel->m_bOwner = bSuccess;
            delete pSel->m_pPixmap;
            pSel->m_pPixmap = nullptr;
            pSel->m_nOrigTimestamp = m_nSelectionTimestamp;
        }
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "no adaptor for selection %s\n",
                     OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );

        if( pAdaptor->getTransferable().is() )
        {
            Sequence< DataFlavor > aTypes = pAdaptor->getTransferable()->getTransferDataFlavors();
            for( int i = 0; i < aTypes.getLength(); i++ )
            {
                fprintf( stderr, "   %s\n", OUStringToOString( aTypes.getConstArray()[i].MimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
            }
        }
#endif
    }
    return bSuccess;
}

void SelectionManager::convertTypeToNative( const OUString& rType, Atom selection, int& rFormat, ::std::list< Atom >& rConversions, bool bPushFront )
{
    NativeTypeEntry* pTab = selection == m_nXdndSelection ? aXdndConversionTab : aNativeConversionTab;
    int nTabEntries = selection == m_nXdndSelection ? SAL_N_ELEMENTS(aXdndConversionTab) : SAL_N_ELEMENTS(aNativeConversionTab);

    OString aType( OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ) );
    SAL_INFO( "vcl.unx.dtrans", "convertTypeToNative " << aType );
    rFormat = 0;
    for( int i = 0; i < nTabEntries; i++ )
    {
        if( aType.equalsIgnoreAsciiCase( pTab[i].pType ) )
        {
            if( ! pTab[i].nAtom )
                pTab[i].nAtom = getAtom( OStringToOUString( pTab[i].pNativeType, RTL_TEXTENCODING_ISO_8859_1 ) );
            rFormat = pTab[i].nFormat;
            if( bPushFront )
                rConversions.push_front( pTab[i].nAtom );
            else
                rConversions.push_back( pTab[i].nAtom );
            if( pTab[i].nFormat == XA_PIXMAP )
            {
                if( bPushFront )
                {
                    rConversions.push_front( XA_VISUALID );
                    rConversions.push_front( XA_COLORMAP );
                }
                else
                {
                    rConversions.push_back( XA_VISUALID );
                    rConversions.push_back( XA_COLORMAP );
                }
            }
        }
    }
    if( ! rFormat )
        rFormat = 8; // byte buffer
    if( bPushFront )
        rConversions.push_front( getAtom( rType ) );
    else
        rConversions.push_back( getAtom( rType ) );
};

void SelectionManager::getNativeTypeList( const Sequence< DataFlavor >& rTypes, std::list< Atom >& rOutTypeList, Atom targetselection )
{
    rOutTypeList.clear();

    int nFormat;
    int nFlavors = rTypes.getLength();
    const DataFlavor* pFlavors = rTypes.getConstArray();
    bool bHaveText = false;
    for( int i = 0; i < nFlavors; i++ )
    {
        if( pFlavors[i].MimeType.startsWith("text/plain"))
            bHaveText = true;
        else
            convertTypeToNative( pFlavors[i].MimeType, targetselection, nFormat, rOutTypeList );
    }
    if( bHaveText )
    {
        if( targetselection != m_nXdndSelection )
        {
            // only mimetypes should go into Xdnd type list
            rOutTypeList.push_front( XA_STRING );
            rOutTypeList.push_front( m_nCOMPOUNDAtom );
        }
        convertTypeToNative( "text/plain;charset=utf-8", targetselection, nFormat, rOutTypeList, true );
    }
    if( targetselection != m_nXdndSelection )
        rOutTypeList.push_back( m_nMULTIPLEAtom );
}

OUString SelectionManager::convertTypeFromNative( Atom nType, Atom selection, int& rFormat )
{
    NativeTypeEntry* pTab = (selection == m_nXdndSelection) ? aXdndConversionTab : aNativeConversionTab;
    int nTabEntries = (selection == m_nXdndSelection) ? SAL_N_ELEMENTS(aXdndConversionTab) : SAL_N_ELEMENTS(aNativeConversionTab);

    for( int i = 0; i < nTabEntries; i++ )
    {
        if( ! pTab[i].nAtom )
            pTab[i].nAtom = getAtom( OStringToOUString( pTab[i].pNativeType, RTL_TEXTENCODING_ISO_8859_1 ) );
        if( nType == pTab[i].nAtom )
        {
            rFormat = pTab[i].nFormat;
            return OStringToOUString( pTab[i].pType, RTL_TEXTENCODING_ISO_8859_1 );
        }
    }
    rFormat = 8;
    return getString( nType );
}

bool SelectionManager::getPasteData( Atom selection, Atom type, Sequence< sal_Int8 >& rData )
{
    osl::ResettableMutexGuard aGuard(m_aMutex);
    std::unordered_map< Atom, Selection* >::iterator it;
    bool bSuccess = false;

#if OSL_DEBUG_LEVEL > 1
    OUString aSelection( getString( selection ) );
    OUString aType( getString( type ) );
    fprintf( stderr, "getPasteData( %s, native: %s )\n",
             OUStringToOString( aSelection, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( aType, RTL_TEXTENCODING_ISO_8859_1 ).getStr()
             );
#endif

    if( ! m_pDisplay )
        return false;

    it = m_aSelections.find( selection );
    if( it == m_aSelections.end() )
        return false;

    ::Window aSelectionOwner = XGetSelectionOwner( m_pDisplay, selection );
    if( aSelectionOwner == None )
        return false;
    if( aSelectionOwner == m_aWindow )
    {
        // probably bad timing led us here
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Innere Nabelschau\n" );
#endif
        return false;
    }

    // ICCCM recommends to destroy property before convert request unless
    // parameters are transported; we do only in case of MULTIPLE,
    // so destroy property unless target is MULTIPLE
    if( type != m_nMULTIPLEAtom )
        XDeleteProperty( m_pDisplay, m_aWindow, selection );

    XConvertSelection( m_pDisplay, selection, type, selection, m_aWindow, selection == m_nXdndSelection ? m_nDropTime : CurrentTime );
    it->second->m_eState            = Selection::WaitingForResponse;
    it->second->m_aRequestedType    = type;
    it->second->m_aData             = Sequence< sal_Int8 >();
    it->second->m_aDataArrived.reset();
    // really start the request; if we don't flush the
    // queue the request won't leave it because there are no more
    // X calls after this until the data arrived or timeout
    XFlush( m_pDisplay );

    // do a reschedule
    struct timeval tv_last, tv_current;
    gettimeofday( &tv_last, nullptr );
    tv_current = tv_last;

    XEvent aEvent;
    do
    {
        bool bAdjustTime = false;
        {
            bool bHandle = false;

            if( XCheckTypedEvent( m_pDisplay,
                                  PropertyNotify,
                                  &aEvent
                                  ) )
            {
                bHandle = true;
                if( aEvent.xproperty.window == m_aWindow
                    && aEvent.xproperty.atom == selection )
                    bAdjustTime = true;
            }
            else if( XCheckTypedEvent( m_pDisplay,
                                  SelectionClear,
                                  &aEvent
                                  ) )
            {
                bHandle = true;
            }
            else if( XCheckTypedEvent( m_pDisplay,
                                  SelectionRequest,
                                  &aEvent
                                  ) )
            {
                bHandle = true;
            }
            else if( XCheckTypedEvent( m_pDisplay,
                                  SelectionNotify,
                                  &aEvent
                                  ) )
            {
                bHandle = true;
                if( aEvent.xselection.selection == selection
                    && ( aEvent.xselection.requestor == m_aWindow ||
                         aEvent.xselection.requestor == m_aCurrentDropWindow )
                    )
                    bAdjustTime = true;
            }
            else
            {
                aGuard.clear();
                osl::Thread::wait(std::chrono::milliseconds(100));
                aGuard.reset();
            }
            if( bHandle )
            {
                aGuard.clear();
                handleXEvent( aEvent );
                aGuard.reset();
            }
        }
        gettimeofday( &tv_current, nullptr );
        if( bAdjustTime )
            tv_last = tv_current;
    } while( ! it->second->m_aDataArrived.check() && (tv_current.tv_sec - tv_last.tv_sec) < getSelectionTimeout() );

#if OSL_DEBUG_LEVEL > 1
    if( (tv_current.tv_sec - tv_last.tv_sec) > getSelectionTimeout() )
        fprintf( stderr, "timed out\n" );
#endif
    if( it->second->m_aDataArrived.check() &&
        it->second->m_aData.getLength() )
    {
        rData = it->second->m_aData;
        bSuccess = true;
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "conversion unsuccessful\n" );
#endif
    return bSuccess;
}

bool SelectionManager::getPasteData( Atom selection, const OUString& rType, Sequence< sal_Int8 >& rData )
{
    bool bSuccess = false;

    std::unordered_map< Atom, Selection* >::iterator it;
    {
        osl::MutexGuard aGuard(m_aMutex);

        it = m_aSelections.find( selection );
        if( it == m_aSelections.end() )
            return false;
    }

    if( it->second->m_aTypes.getLength() == 0 )
    {
        Sequence< DataFlavor > aFlavors;
        getPasteDataTypes( selection, aFlavors );
        if( it->second->m_aTypes.getLength() == 0 )
            return false;
    }

    const Sequence< DataFlavor >& rTypes( it->second->m_aTypes );
    const std::vector< Atom >& rNativeTypes( it->second->m_aNativeTypes );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "getPasteData( \"%s\", \"%s\" )\n",
             OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    if( rType == "text/plain;charset=utf-16" )
    {
        // lets see if we have UTF16 else try to find something convertible
        if( it->second->m_aTypes.getLength() && ! it->second->m_bHaveUTF16 )
        {
            Sequence< sal_Int8 > aData;
            if( it->second->m_aUTF8Type != None &&
                getPasteData( selection,
                              it->second->m_aUTF8Type,
                              aData )
                )
            {
              OUString aRet( reinterpret_cast<const char*>(aData.getConstArray()), aData.getLength(), RTL_TEXTENCODING_UTF8 );
              rData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aRet.getStr()), (aRet.getLength()+1)*sizeof( sal_Unicode ) );
              bSuccess = true;
            }
            else if( it->second->m_bHaveCompound &&
                getPasteData( selection,
                              m_nCOMPOUNDAtom,
                              aData )
                )
            {
                OUString aRet( convertFromCompound( reinterpret_cast<const char*>(aData.getConstArray()), aData.getLength() ) );
                rData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aRet.getStr()), (aRet.getLength()+1)*sizeof( sal_Unicode ) );
                bSuccess = true;
            }
            else
            {
                for( int i = 0; i < rTypes.getLength(); i++ )
                {
                    rtl_TextEncoding aEncoding = getTextPlainEncoding( rTypes.getConstArray()[i].MimeType );
                    if( aEncoding != RTL_TEXTENCODING_DONTKNOW  &&
                        aEncoding != RTL_TEXTENCODING_UNICODE   &&
                        getPasteData( selection,
                                      rNativeTypes[i],
                                      aData )
                        )
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "using \"%s\" instead of \"%s\"\n",
                                 OUStringToOString( rTypes.getConstArray()[i].MimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                                 OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                                 );
#endif
                        OString aConvert( reinterpret_cast<char const *>(aData.getConstArray()), aData.getLength() );
                        OUString aUTF( OStringToOUString( aConvert, aEncoding ) );
                        rData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aUTF.getStr()), (aUTF.getLength()+1)*sizeof( sal_Unicode ) );
                        bSuccess = true;
                        break;
                    }
                }
            }
        }
    }
    else if( rType == "image/bmp" )
    {
        // #i83376# try if someone has the data in image/bmp already before
        // doing the PIXMAP stuff (e.g. the Gimp has this)
        bSuccess = getPasteData( selection, m_nImageBmpAtom, rData );
        #if OSL_DEBUG_LEVEL > 1
        if( bSuccess )
            fprintf( stderr, "got %d bytes of image/bmp\n", (int)rData.getLength() );
        #endif
        if( ! bSuccess )
        {
            Pixmap aPixmap = None;
            Colormap aColormap = None;

            // prepare property for MULTIPLE request
            Sequence< sal_Int8 > aData;
            Atom const pTypes[4] = { XA_PIXMAP, XA_PIXMAP, XA_COLORMAP, XA_COLORMAP };
            {
                osl::MutexGuard aGuard(m_aMutex);

                XChangeProperty( m_pDisplay,
                    m_aWindow,
                    selection,
                    XA_ATOM,
                    32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char const *>(pTypes),
                    4 );
            }

            // try MULTIPLE request
            if( getPasteData( selection, m_nMULTIPLEAtom, aData ) )
            {
                Atom* pReturnedTypes = reinterpret_cast<Atom*>(aData.getArray());
                if( pReturnedTypes[0] == XA_PIXMAP && pReturnedTypes[1] == XA_PIXMAP )
                {
                    osl::MutexGuard aGuard(m_aMutex);

                    Atom type = None;
                    int format = 0;
                    unsigned long nItems = 0;
                    unsigned long nBytes = 0;
                    unsigned char* pReturn = nullptr;
                    XGetWindowProperty( m_pDisplay, m_aWindow, XA_PIXMAP, 0, 1, True, XA_PIXMAP, &type, &format, &nItems, &nBytes, &pReturn );
                    if( pReturn )
                    {
                        if( type == XA_PIXMAP )
                            aPixmap = *reinterpret_cast<Pixmap*>(pReturn);
                        XFree( pReturn );
                        pReturn = nullptr;
                        if( pReturnedTypes[2] == XA_COLORMAP && pReturnedTypes[3] == XA_COLORMAP )
                        {
                            XGetWindowProperty( m_pDisplay, m_aWindow, XA_COLORMAP, 0, 1, True, XA_COLORMAP, &type, &format, &nItems, &nBytes, &pReturn );
                            if( pReturn )
                            {
                                if( type == XA_COLORMAP )
                                    aColormap = *reinterpret_cast<Colormap*>(pReturn);
                                XFree( pReturn );
                            }
                        }
                    }
                    #if OSL_DEBUG_LEVEL > 1
                    else
                    {
                        fprintf( stderr, "could not get PIXMAP property: type=%s, format=%d, items=%ld, bytes=%ld, ret=0x%p\n", OUStringToOString( getString( type ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(), format, nItems, nBytes, pReturn );
                    }
                    #endif
                }
            }

            if( aPixmap == None )
            {
                // perhaps two normal requests will work
                if( getPasteData( selection, XA_PIXMAP, aData ) )
                {
                    aPixmap = *reinterpret_cast<Pixmap*>(aData.getArray());
                    if( aColormap == None && getPasteData( selection, XA_COLORMAP, aData ) )
                        aColormap = *reinterpret_cast<Colormap*>(aData.getArray());
                }
            }

            // convert data if possible
            if( aPixmap != None )
            {
                osl::MutexGuard aGuard(m_aMutex);

                sal_Int32 nOutSize = 0;
                sal_uInt8* pBytes = X11_getBmpFromPixmap( m_pDisplay, aPixmap, aColormap, nOutSize );
                if( pBytes )
                {
                    if( nOutSize )
                    {
                        rData = Sequence< sal_Int8 >( nOutSize );
                        memcpy( rData.getArray(), pBytes, nOutSize );
                        bSuccess = true;
                    }
                    std::free( pBytes );
                }
            }
        }
    }

    if( ! bSuccess )
    {
        int nFormat;
        ::std::list< Atom > aTypes;
        convertTypeToNative( rType, selection, nFormat, aTypes );
        Atom nSelectedType = None;
        for (auto const& type : aTypes)
        {
            for( auto const & nativeType: rNativeTypes )
                if(nativeType == type)
                {
                    nSelectedType = type;
                    if (nSelectedType != None)
                        break;
                }
        }
        if( nSelectedType != None )
            bSuccess = getPasteData( selection, nSelectedType, rData );
    }
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "getPasteData for selection %s and data type %s returns %s, returned sequence has length %" SAL_PRIdINT32 "\n",
             OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             bSuccess ? "true" : "false",
             rData.getLength()
             );
#endif
    return bSuccess;
}

bool SelectionManager::getPasteDataTypes( Atom selection, Sequence< DataFlavor >& rTypes )
{
    std::unordered_map< Atom, Selection* >::iterator it;
    {
        osl::MutexGuard aGuard(m_aMutex);

        it = m_aSelections.find( selection );
        if( it != m_aSelections.end()                           &&
            it->second->m_aTypes.getLength()                    &&
            std::abs( it->second->m_nLastTimestamp - time( nullptr ) ) < 2
            )
        {
            rTypes = it->second->m_aTypes;
            return true;
        }
    }

    bool bSuccess = false;
    bool bHaveUTF16 = false;
    Atom aUTF8Type = None;
    bool bHaveCompound = false;
    Sequence< sal_Int8 > aAtoms;

    if( selection == m_nXdndSelection )
    {
        // xdnd sends first three types with XdndEnter
        // if more than three types are supported then the XDndTypeList
        // property on the source window is used
        if( m_aDropEnterEvent.data.l[0] && m_aCurrentDropWindow )
        {
            if( m_aDropEnterEvent.data.l[1] & 1 )
            {
                const unsigned int atomcount = 256;
                // more than three types; look in property
                osl::MutexGuard aGuard(m_aMutex);

                Atom nType;
                int nFormat;
                unsigned long nItems, nBytes;
                unsigned char* pBytes = nullptr;

                XGetWindowProperty( m_pDisplay, m_aDropEnterEvent.data.l[0],
                                    m_nXdndTypeList, 0, atomcount, False,
                                    XA_ATOM,
                                    &nType, &nFormat, &nItems, &nBytes, &pBytes );
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "have %ld data types in XdndTypeList\n", nItems );
#endif
                if( nItems == atomcount && nBytes > 0 )
                {
                    // wow ... more than 256 types !
                    aAtoms.realloc( sizeof( Atom )*atomcount+nBytes );
                    memcpy( aAtoms.getArray(), pBytes, sizeof( Atom )*atomcount );
                    XFree( pBytes );
                    pBytes = nullptr;
                    XGetWindowProperty( m_pDisplay, m_aDropEnterEvent.data.l[0],
                                        m_nXdndTypeList, atomcount, nBytes/sizeof(Atom),
                                        False, XA_ATOM,
                                        &nType, &nFormat, &nItems, &nBytes, &pBytes );
                    {
                        memcpy( aAtoms.getArray()+atomcount*sizeof(Atom), pBytes, nItems*sizeof(Atom) );
                        XFree( pBytes );
                    }
                }
                else
                {
                    aAtoms.realloc( sizeof(Atom)*nItems );
                    memcpy( aAtoms.getArray(), pBytes, nItems*sizeof(Atom) );
                    XFree( pBytes );
                }
            }
            else
            {
                // one to three types
                int n = 0, i;
                for( i = 0; i < 3; i++ )
                    if( m_aDropEnterEvent.data.l[2+i] )
                        n++;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "have %d data types in XdndEnter\n", n );
#endif
                aAtoms.realloc( sizeof(Atom)*n );
                for( i = 0, n = 0; i < 3; i++ )
                    if( m_aDropEnterEvent.data.l[2+i] )
                        reinterpret_cast<Atom*>(aAtoms.getArray())[n++] = m_aDropEnterEvent.data.l[2+i];
            }
        }
    }
    // get data of type TARGETS
    else if( ! getPasteData( selection, m_nTARGETSAtom, aAtoms ) )
        aAtoms = Sequence< sal_Int8 >();

    std::vector< Atom > aNativeTypes;
    if( aAtoms.getLength() )
    {
        sal_Int32 nAtoms = aAtoms.getLength() / sizeof(Atom);
        Atom* pAtoms = reinterpret_cast<Atom*>(aAtoms.getArray());
        rTypes.realloc( nAtoms );
        aNativeTypes.resize( nAtoms );
        DataFlavor* pFlavors = rTypes.getArray();
        sal_Int32 nNativeTypesIndex = 0;
        bool bHaveText = false;
        while( nAtoms-- )
        {
            SAL_INFO_IF(*pAtoms && *pAtoms < 0x01000000, "vcl.unx.dtrans",
                "getPasteDataTypes: available: \"" << getString(*pAtoms) << "\"");
            if( *pAtoms == m_nCOMPOUNDAtom )
                bHaveText = bHaveCompound = true;
            else if( *pAtoms && *pAtoms < 0x01000000 )
            {
                int nFormat;
                pFlavors->MimeType = convertTypeFromNative( *pAtoms, selection, nFormat );
                pFlavors->DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();
                sal_Int32 nIndex = 0;
                if( pFlavors->MimeType.getToken( 0, ';', nIndex ) == "text/plain" )
                {
                    OUString aToken(pFlavors->MimeType.getToken( 0, ';', nIndex ));
                    // omit text/plain;charset=unicode since it is not well defined
                    if( aToken == "charset=unicode" )
                    {
                        pAtoms++;
                        continue;
                    }
                    bHaveText = true;
                    if( aToken == "charset=utf-16" )
                    {
                        bHaveUTF16 = true;
                        pFlavors->DataType = cppu::UnoType<OUString>::get();
                    }
                    else if( aToken == "charset=utf-8" )
                    {
                        aUTF8Type = *pAtoms;
                    }
                }
                pFlavors++;
                aNativeTypes[ nNativeTypesIndex ] = *pAtoms;
                nNativeTypesIndex++;
            }
            pAtoms++;
        }
        if( (pFlavors - rTypes.getArray()) < rTypes.getLength() )
            rTypes.realloc(pFlavors - rTypes.getArray());
        bSuccess = rTypes.hasElements();
        if( bHaveText && ! bHaveUTF16 )
        {
            int i = 0;

            int nNewFlavors = rTypes.getLength()+1;
            Sequence< DataFlavor > aTemp( nNewFlavors );
            for( i = 0; i < nNewFlavors-1; i++ )
                aTemp.getArray()[i+1] = rTypes.getConstArray()[i];
            aTemp.getArray()[0].MimeType = "text/plain;charset=utf-16";
            aTemp.getArray()[0].DataType = cppu::UnoType<OUString>::get();
            rTypes = aTemp;

            std::vector< Atom > aNativeTemp( nNewFlavors );
            for( i = 0; i < nNewFlavors-1; i++ )
                aNativeTemp[ i + 1 ] = aNativeTypes[ i ];
            aNativeTemp[0] = None;
            aNativeTypes = aNativeTemp;
        }
    }

    {
        osl::MutexGuard aGuard(m_aMutex);

        it = m_aSelections.find( selection );
        if( it != m_aSelections.end() )
        {
            if( bSuccess )
            {
                it->second->m_aTypes            = rTypes;
                it->second->m_aNativeTypes      = aNativeTypes;
                it->second->m_nLastTimestamp    = time( nullptr );
                it->second->m_bHaveUTF16        = bHaveUTF16;
                it->second->m_aUTF8Type         = aUTF8Type;
                it->second->m_bHaveCompound     = bHaveCompound;
            }
            else
            {
                it->second->m_aTypes            = Sequence< DataFlavor >();
                it->second->m_aNativeTypes      = std::vector< Atom >();
                it->second->m_nLastTimestamp    = 0;
                it->second->m_bHaveUTF16        = false;
                it->second->m_aUTF8Type         = None;
                it->second->m_bHaveCompound     = false;
            }
        }
    }

#if OSL_DEBUG_LEVEL > 1
    {
        fprintf( stderr, "SelectionManager::getPasteDataTypes( %s ) = %s\n", OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(), bSuccess ? "true" : "false" );
        for( int i = 0; i < rTypes.getLength(); i++ )
            fprintf( stderr, "type: %s\n", OUStringToOString( rTypes.getConstArray()[i].MimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
    }
#endif

    return bSuccess;
}

PixmapHolder* SelectionManager::getPixmapHolder( Atom selection )
{
    std::unordered_map< Atom, Selection* >::const_iterator it = m_aSelections.find( selection );
    if( it == m_aSelections.end() )
        return nullptr;
    if( ! it->second->m_pPixmap )
        it->second->m_pPixmap = new PixmapHolder( m_pDisplay );
    return it->second->m_pPixmap;
}

static std::size_t GetTrueFormatSize(int nFormat)
{
    // http://mail.gnome.org/archives/wm-spec-list/2003-March/msg00067.html
    return nFormat == 32 ? sizeof(long) : nFormat/8;
}

bool SelectionManager::sendData( SelectionAdaptor* pAdaptor,
                                 ::Window requestor,
                                 Atom target,
                                 Atom property,
                                 Atom selection )
{
    osl::ResettableMutexGuard aGuard( m_aMutex );

    // handle targets related to image/bmp
    if( target == XA_COLORMAP || target == XA_PIXMAP || target == XA_BITMAP || target == XA_VISUALID )
    {
        PixmapHolder* pPixmap = getPixmapHolder( selection );
        if( ! pPixmap ) return false;
        XID nValue = None;

        // handle colormap request
        if( target == XA_COLORMAP )
            nValue = static_cast<XID>(pPixmap->getColormap());
        else if( target == XA_VISUALID )
            nValue = static_cast<XID>(pPixmap->getVisualID());
        else if( target == XA_PIXMAP || target == XA_BITMAP )
        {
            nValue = static_cast<XID>(pPixmap->getPixmap());
            if( nValue == None )
            {
                // first conversion
                Sequence< sal_Int8 > aData;
                int nFormat;
                aGuard.clear();
                bool bConverted = convertData( pAdaptor->getTransferable(), target, selection, nFormat, aData );
                aGuard.reset();
                if( bConverted )
                {
                    // get pixmap again since clearing the guard could have invalidated
                    // the pixmap in another thread
                    pPixmap = getPixmapHolder( selection );
                    // conversion succeeded, so aData contains image/bmp now
                    if( pPixmap->needsConversion( reinterpret_cast<const sal_uInt8*>(aData.getConstArray()) ) )
                    {
                        SAL_INFO( "vcl.unx.dtrans", "trying bitmap conversion" );
                        int depth = pPixmap->getDepth();
                        aGuard.clear();
                        aData = convertBitmapDepth(aData, depth);
                        aGuard.reset();
                    }
                    // get pixmap again since clearing the guard could have invalidated
                    // the pixmap in another thread
                    pPixmap = getPixmapHolder( selection );
                    nValue = static_cast<XID>(pPixmap->setBitmapData( reinterpret_cast<const sal_uInt8*>(aData.getConstArray()) ));
                }
                if( nValue == None )
                    return false;
            }
            if( target == XA_BITMAP )
                nValue = static_cast<XID>(pPixmap->getBitmap());
        }

        XChangeProperty( m_pDisplay,
                         requestor,
                         property,
                         target,
                         32,
                         PropModeReplace,
                         reinterpret_cast<const unsigned char*>(&nValue),
                         1);
        return true;
    }

    /*
     *  special target TEXT allows us to transfer
     *  the data in an encoding of our choice
     *  COMPOUND_TEXT will work with most applications
     */
    if( target == m_nTEXTAtom )
        target = m_nCOMPOUNDAtom;

    Sequence< sal_Int8 > aData;
    int nFormat;
    aGuard.clear();
    bool bConverted = convertData( pAdaptor->getTransferable(), target, selection, nFormat, aData );
    aGuard.reset();
    if( bConverted )
    {
        // conversion succeeded
        if( aData.getLength() > m_nIncrementalThreshold )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "using INCR protocol\n" );
            std::unordered_map< ::Window, std::unordered_map< Atom, IncrementalTransfer > >::const_iterator win_it = m_aIncrementals.find( requestor );
            if( win_it != m_aIncrementals.end() )
            {
                std::unordered_map< Atom, IncrementalTransfer >::const_iterator inc_it = win_it->second.find( property );
                if( inc_it != win_it->second.end() )
                {
                    const IncrementalTransfer& rInc = inc_it->second;
                    fprintf( stderr, "premature end and new start for INCR transfer for window 0x%lx, property %s, type %s\n",
                             rInc.m_aRequestor,
                             OUStringToOString( getString( rInc.m_aProperty ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                             OUStringToOString( getString( rInc.m_aTarget ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                             );
                }
            }
#endif

            // insert IncrementalTransfer
            IncrementalTransfer& rInc   = m_aIncrementals[ requestor ][ property ];
            rInc.m_aData                = aData;
            rInc.m_nBufferPos           = 0;
            rInc.m_aRequestor           = requestor;
            rInc.m_aProperty            = property;
            rInc.m_aTarget              = target;
            rInc.m_nFormat              = nFormat;
            rInc.m_nTransferStartTime   = time( nullptr );

            // use incr protocol, signal start to requestor
            long nMinSize = m_nIncrementalThreshold;
            XSelectInput( m_pDisplay, requestor, PropertyChangeMask );
            XChangeProperty( m_pDisplay, requestor, property,
                             m_nINCRAtom, 32,  PropModeReplace, reinterpret_cast<unsigned char*>(&nMinSize), 1 );
            XFlush( m_pDisplay );
        }
        else
        {
            std::size_t nUnitSize = GetTrueFormatSize(nFormat);
            XChangeProperty( m_pDisplay,
                             requestor,
                             property,
                             target,
                             nFormat,
                             PropModeReplace,
                             reinterpret_cast<const unsigned char*>(aData.getConstArray()),
                             aData.getLength()/nUnitSize );
            }
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "convertData failed for type: %s \n",
                 OUStringToOString( convertTypeFromNative( target, selection, nFormat ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    return bConverted;
}

bool SelectionManager::handleSelectionRequest( XSelectionRequestEvent& rRequest )
{
    osl::ResettableMutexGuard aGuard( m_aMutex );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "handleSelectionRequest for selection %s and target %s\n",
             OUStringToOString( getString( rRequest.selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( getString( rRequest.target ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
             );
#endif

    XEvent aNotify;

    aNotify.type                  = SelectionNotify;
    aNotify.xselection.display    = rRequest.display;
    aNotify.xselection.send_event = True;
    aNotify.xselection.requestor  = rRequest.requestor;
    aNotify.xselection.selection  = rRequest.selection;
    aNotify.xselection.time       = rRequest.time;
    aNotify.xselection.target     = rRequest.target;
    aNotify.xselection.property   = None;

    SelectionAdaptor* pAdaptor = getAdaptor( rRequest.selection );
    // ensure that we still own that selection
    if( pAdaptor &&
        XGetSelectionOwner( m_pDisplay, rRequest.selection ) == m_aWindow )
    {
        css::uno::Reference< XTransferable > xTrans( pAdaptor->getTransferable() );
        if( rRequest.target == m_nTARGETSAtom )
        {
            // someone requests our types
            if( xTrans.is() )
            {
                aGuard.clear();
                Sequence< DataFlavor > aFlavors = xTrans->getTransferDataFlavors();
                aGuard.reset();

                ::std::list< Atom > aConversions;
                getNativeTypeList( aFlavors, aConversions, rRequest.selection );

                int i, nTypes = aConversions.size();
                Atom* pTypes = static_cast<Atom*>(alloca( nTypes * sizeof( Atom ) ));
                std::list< Atom >::const_iterator it;
                for( i = 0, it = aConversions.begin(); i < nTypes; i++, ++it )
                    pTypes[i] = *it;
                XChangeProperty( m_pDisplay, rRequest.requestor, rRequest.property,
                                 XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(pTypes), nTypes );
                aNotify.xselection.property = rRequest.property;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "sending type list:\n" );
                for( int k = 0; k < nTypes; k++ )
                    fprintf( stderr, "   %s\n", pTypes[k] ? XGetAtomName( m_pDisplay, pTypes[k] ) : "<None>" );
#endif
            }
        }
        else if( rRequest.target == m_nTIMESTAMPAtom )
        {
            long nTimeStamp = static_cast<long>(m_aSelections[rRequest.selection]->m_nOrigTimestamp);
            XChangeProperty( m_pDisplay, rRequest.requestor, rRequest.property,
                             XA_INTEGER, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&nTimeStamp), 1 );
            aNotify.xselection.property = rRequest.property;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "sending timestamp: %d\n", (int)nTimeStamp );
#endif
        }
        else
        {
            bool bEventSuccess = false;
            if( rRequest.target == m_nMULTIPLEAtom )
            {
                // get all targets
                Atom nType = None;
                int nFormat = 0;
                unsigned long nItems = 0, nBytes = 0;
                unsigned char* pData = nullptr;

                // get number of atoms
                XGetWindowProperty( m_pDisplay,
                                    rRequest.requestor,
                                    rRequest.property,
                                    0, 0,
                                    False,
                                    AnyPropertyType,
                                    &nType, &nFormat,
                                    &nItems, &nBytes,
                                    &pData );
                if( nFormat == 32 && nBytes/4 )
                {
                    if( pData ) // ?? should not happen
                    {
                        XFree( pData );
                        pData = nullptr;
                    }
                    XGetWindowProperty( m_pDisplay,
                                        rRequest.requestor,
                                        rRequest.property,
                                        0, nBytes/4,
                                        False,
                                        nType,
                                        &nType, &nFormat,
                                        &nItems, &nBytes,
                                        &pData );
                    if( pData && nItems )
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "found %ld atoms in MULTIPLE request\n", nItems );
#endif
                        bEventSuccess = true;
                        bool bResetAtoms = false;
                        Atom* pAtoms = reinterpret_cast<Atom*>(pData);
                        aGuard.clear();
                        for( unsigned long i = 0; i < nItems; i += 2 )
                        {
#if OSL_DEBUG_LEVEL > 1
                            fprintf( stderr, "   %s => %s: ",
                                     OUStringToOString( getString( pAtoms[i] ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                                     OUStringToOString( getString( pAtoms[i+1] ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
                            bool bSuccess = sendData( pAdaptor, rRequest.requestor, pAtoms[i], pAtoms[i+1], rRequest.selection );
#if OSL_DEBUG_LEVEL > 1
                            fprintf( stderr, "%s\n", bSuccess ? "succeeded" : "failed" );
#endif
                            if( ! bSuccess )
                            {
                                pAtoms[i] = None;
                                bResetAtoms = true;
                            }
                        }
                        aGuard.reset();
                        if( bResetAtoms )
                            XChangeProperty( m_pDisplay,
                                             rRequest.requestor,
                                             rRequest.property,
                                             XA_ATOM,
                                             32,
                                             PropModeReplace,
                                             pData,
                                             nBytes/4 );
                    }
                    if( pData )
                        XFree( pData );
                }
#if OSL_DEBUG_LEVEL > 1
                else
                {
                    fprintf( stderr, "could not get type list from \"%s\" of type \"%s\" on requestor 0x%lx, requestor has properties:",
                             OUStringToOString( getString( rRequest.property ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                             OUStringToOString( getString( nType ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                             rRequest.requestor );
                    int nProps = 0;
                    Atom* pProps = XListProperties( m_pDisplay, rRequest.requestor, &nProps );
                    if( pProps )
                    {
                        for( int i = 0; i < nProps; i++ )
                            fprintf( stderr, " \"%s\"", OUStringToOString( getString( pProps[i]), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
                        XFree( pProps );
                    }
                    fprintf( stderr, "\n" );
                }
#endif
            }
            else
            {
                aGuard.clear();
                bEventSuccess = sendData( pAdaptor, rRequest.requestor, rRequest.target, rRequest.property, rRequest.selection );
                aGuard.reset();
            }
            if( bEventSuccess )
            {
                aNotify.xselection.target = rRequest.target;
                aNotify.xselection.property = rRequest.property;
            }
        }
        aGuard.clear();
        xTrans.clear();
        aGuard.reset();
    }
    XSendEvent( m_pDisplay, rRequest.requestor, False, 0, &aNotify );

    if( rRequest.selection == XA_PRIMARY    &&
        m_bWaitingForPrimaryConversion      &&
        m_xDragSourceListener.is() )
    {
        DragSourceDropEvent dsde;
        dsde.Source                 = static_cast< OWeakObject* >(this);
        dsde.DragSourceContext      = new DragSourceContext( m_aDropWindow, *this );
        dsde.DragSource             = static_cast< XDragSource* >(this);
        if( aNotify.xselection.property != None )
        {
            dsde.DropAction         = DNDConstants::ACTION_COPY;
            dsde.DropSuccess        = true;
        }
        else
        {
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = false;
        }
        css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
        m_xDragSourceListener.clear();
        aGuard.clear();
        if( xListener.is() )
            xListener->dragDropEnd( dsde );
    }

    // we handled the event in any case by answering
    return true;
}

bool SelectionManager::handleReceivePropertyNotify( XPropertyEvent const & rNotify )
{
    osl::MutexGuard aGuard( m_aMutex );
    // data we requested arrived
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "handleReceivePropertyNotify for property %s\n",
             OUStringToOString( getString( rNotify.atom ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    bool bHandled = false;

    std::unordered_map< Atom, Selection* >::iterator it =
          m_aSelections.find( rNotify.atom );
    if( it != m_aSelections.end() &&
        rNotify.state == PropertyNewValue &&
        ( it->second->m_eState == Selection::WaitingForResponse     ||
          it->second->m_eState == Selection::WaitingForData         ||
          it->second->m_eState == Selection::IncrementalTransfer
          )
        )
    {
        // MULTIPLE requests are only complete after selection notify
        if( it->second->m_aRequestedType == m_nMULTIPLEAtom &&
            ( it->second->m_eState == Selection::WaitingForResponse ||
              it->second->m_eState == Selection::WaitingForData ) )
            return false;

        bHandled = true;

        Atom nType = None;
        int nFormat = 0;
        unsigned long nItems = 0, nBytes = 0;
        unsigned char* pData = nullptr;

        // get type and length
        XGetWindowProperty( m_pDisplay,
                            rNotify.window,
                            rNotify.atom,
                            0, 0,
                            False,
                            AnyPropertyType,
                            &nType, &nFormat,
                            &nItems, &nBytes,
                            &pData );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %ld bytes data of type %s and format %d, items = %ld\n",
                 nBytes,
                 OUStringToOString( getString( nType ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                 nFormat, nItems );
#endif
        if( pData )
        {
            XFree( pData );
            pData = nullptr;
        }

        if( nType == m_nINCRAtom )
        {
            // start data transfer
            XDeleteProperty( m_pDisplay, rNotify.window, rNotify.atom );
            it->second->m_eState = Selection::IncrementalTransfer;
        }
        else if( nType != None )
        {
            XGetWindowProperty( m_pDisplay,
                                rNotify.window,
                                rNotify.atom,
                                0, nBytes/4 +1,
                                True,
                                nType,
                                &nType, &nFormat,
                                &nItems, &nBytes,
                                &pData );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "read %ld items data of type %s and format %d, %ld bytes left in property\n",
                     nItems,
                     OUStringToOString( getString( nType ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                     nFormat, nBytes );
#endif

            std::size_t nUnitSize = GetTrueFormatSize(nFormat);

            if( it->second->m_eState == Selection::WaitingForData ||
                it->second->m_eState == Selection::WaitingForResponse )
            {
                // copy data
                it->second->m_aData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8*>(pData), nItems*nUnitSize );
                it->second->m_eState = Selection::Inactive;
                it->second->m_aDataArrived.set();
            }
            else if( it->second->m_eState == Selection::IncrementalTransfer )
            {
                if( nItems )
                {
                    // append data
                    Sequence< sal_Int8 > aData( it->second->m_aData.getLength() + nItems*nUnitSize );
                    memcpy( aData.getArray(), it->second->m_aData.getArray(), it->second->m_aData.getLength() );
                    memcpy( aData.getArray() + it->second->m_aData.getLength(), pData, nItems*nUnitSize );
                    it->second->m_aData = aData;
                }
                else
                {
                    it->second->m_eState = Selection::Inactive;
                    it->second->m_aDataArrived.set();
                }
            }
            if( pData )
                XFree( pData );
        }
        else if( it->second->m_eState == Selection::IncrementalTransfer )
        {
            it->second->m_eState = Selection::Inactive;
            it->second->m_aDataArrived.set();
        }
    }
    return bHandled;
}

bool SelectionManager::handleSendPropertyNotify( XPropertyEvent const & rNotify )
{
    osl::MutexGuard aGuard( m_aMutex );

    // ready for next part of a IncrementalTransfer
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "handleSendPropertyNotify for property %s (%s)\n",
             OUStringToOString( getString( rNotify.atom ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             rNotify.state == PropertyNewValue ? "new value" : ( rNotify.state == PropertyDelete ? "deleted" : "unknown")
             );
#endif

    bool bHandled = false;
    // feed incrementals
    if( rNotify.state == PropertyDelete )
    {
        std::unordered_map< ::Window, std::unordered_map< Atom, IncrementalTransfer > >::iterator it;
        it = m_aIncrementals.find( rNotify.window );
        if( it != m_aIncrementals.end() )
        {
            bHandled = true;
            int nCurrentTime = time( nullptr );
            // throw out aborted transfers
            std::vector< Atom > aTimeouts;
            for (auto const& incrementalTransfer : it->second)
            {
                if( (nCurrentTime - incrementalTransfer.second.m_nTransferStartTime) > (getSelectionTimeout()+2) )
                {
                    aTimeouts.push_back( incrementalTransfer.first );
#if OSL_DEBUG_LEVEL > 1
                    const IncrementalTransfer& rInc = incrementalTransfer.second;
                    fprintf( stderr, "timeout on INCR transfer for window 0x%lx, property %s, type %s\n",
                             rInc.m_aRequestor,
                             OUStringToOString( getString( rInc.m_aProperty ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                             OUStringToOString( getString( rInc.m_aTarget ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                             );
#endif
                }
            }

            for (auto const& timeout : aTimeouts)
            {
                // transfer broken, might even be a new client with the
                // same window id
                it->second.erase( timeout );
            }
            aTimeouts.clear();

            auto inc_it = it->second.find( rNotify.atom );
            if( inc_it != it->second.end() )
            {
                IncrementalTransfer& rInc = inc_it->second;

                int nBytes = rInc.m_aData.getLength() - rInc.m_nBufferPos;
                nBytes = std::min(nBytes, m_nIncrementalThreshold);
                if( nBytes < 0 )  // sanity check
                    nBytes = 0;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "pushing %d bytes: \"%.*s\"...\n",
                         nBytes, std::min(nBytes, 32),
                         (const unsigned char*)rInc.m_aData.getConstArray()+rInc.m_nBufferPos );
#endif

                std::size_t nUnitSize = GetTrueFormatSize(rInc.m_nFormat);

                XChangeProperty( m_pDisplay,
                                 rInc.m_aRequestor,
                                 rInc.m_aProperty,
                                 rInc.m_aTarget,
                                 rInc.m_nFormat,
                                 PropModeReplace,
                                 reinterpret_cast<const unsigned char*>(rInc.m_aData.getConstArray())+rInc.m_nBufferPos,
                                 nBytes/nUnitSize );
                rInc.m_nBufferPos += nBytes;
                rInc.m_nTransferStartTime = nCurrentTime;

                if( nBytes == 0 ) // transfer finished
                {
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "finished INCR transfer for window 0x%lx, property %s, type %s\n",
                             rInc.m_aRequestor,
                             OUStringToOString( getString( rInc.m_aProperty ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                             OUStringToOString( getString( rInc.m_aTarget ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                             );
#endif
                    it->second.erase( inc_it );
                }

            }
            // eventually clean up the hash map
            if( it->second.empty() )
                m_aIncrementals.erase( it );
        }
    }
    return bHandled;
}

bool SelectionManager::handleSelectionNotify( XSelectionEvent const & rNotify )
{
    osl::MutexGuard aGuard( m_aMutex );

    bool bHandled = false;

    // notification about success/failure of one of our conversion requests
#if OSL_DEBUG_LEVEL > 1
    OUString aSelection( getString( rNotify.selection ) );
    OUString aProperty("None");
    if( rNotify.property )
        aProperty = getString( rNotify.property );
    fprintf( stderr, "handleSelectionNotify for selection %s and property %s (0x%lx)\n",
             OUStringToOString( aSelection, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( aProperty, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             rNotify.property
             );
    if( rNotify.requestor != m_aWindow && rNotify.requestor != m_aCurrentDropWindow )
        fprintf( stderr, "Warning: selection notify for unknown window 0x%lx\n", rNotify.requestor );
#endif
    std::unordered_map< Atom, Selection* >::iterator it =
          m_aSelections.find( rNotify.selection );
    if (
        (rNotify.requestor == m_aWindow || rNotify.requestor == m_aCurrentDropWindow) &&
        it != m_aSelections.end() &&
        (
         (it->second->m_eState == Selection::WaitingForResponse) ||
         (it->second->m_eState == Selection::WaitingForData)
        )
       )
    {
        bHandled = true;
        if( it->second->m_aRequestedType == m_nMULTIPLEAtom )
        {
            Atom nType = None;
            int nFormat = 0;
            unsigned long nItems = 0, nBytes = 0;
            unsigned char* pData = nullptr;

            // get type and length
            XGetWindowProperty( m_pDisplay,
                                rNotify.requestor,
                                rNotify.property,
                                0, 256,
                                False,
                                AnyPropertyType,
                                &nType, &nFormat,
                                &nItems, &nBytes,
                                &pData );
            if( nBytes ) // HUGE request !!!
            {
                if( pData )
                    XFree( pData );
                XGetWindowProperty( m_pDisplay,
                                    rNotify.requestor,
                                    rNotify.property,
                                    0, 256+(nBytes+3)/4,
                                    False,
                                    AnyPropertyType,
                                    &nType, &nFormat,
                                    &nItems, &nBytes,
                                    &pData );
            }
            it->second->m_eState        = Selection::Inactive;
            std::size_t nUnitSize = GetTrueFormatSize(nFormat);
            it->second->m_aData         = Sequence< sal_Int8 >(reinterpret_cast<sal_Int8*>(pData), nItems * nUnitSize);
            it->second->m_aDataArrived.set();
            if( pData )
                XFree( pData );
        }
        // WaitingForData can actually happen; some
        // applications (e.g. cmdtool on Solaris) first send
        // a success and then cancel it. Weird !
        else if( rNotify.property == None )
        {
            // conversion failed, stop transfer
            it->second->m_eState        = Selection::Inactive;
            it->second->m_aData         = Sequence< sal_Int8 >();
            it->second->m_aDataArrived.set();
        }
        // get the bytes, by INCR if necessary
        else
            it->second->m_eState = Selection::WaitingForData;
    }
#if OSL_DEBUG_LEVEL > 1
    else if( it != m_aSelections.end() )
        fprintf( stderr, "Warning: selection in state %d\n", it->second->m_eState );
#endif
    return bHandled;
}

bool SelectionManager::handleDropEvent( XClientMessageEvent const & rMessage )
{
    osl::ResettableMutexGuard aGuard(m_aMutex);

    // handle drop related events
    ::Window aSource = rMessage.data.l[0];
    ::Window aTarget = rMessage.window;

    bool bHandled = false;

    std::unordered_map< ::Window, DropTargetEntry >::iterator it =
          m_aDropTargets.find( aTarget );

#if OSL_DEBUG_LEVEL > 1
    if( rMessage.message_type == m_nXdndEnter     ||
        rMessage.message_type == m_nXdndLeave     ||
        rMessage.message_type == m_nXdndDrop      ||
        rMessage.message_type == m_nXdndPosition  )
    {
        fprintf( stderr, "got drop event %s, ", OUStringToOString( getString( rMessage.message_type ), RTL_TEXTENCODING_ASCII_US).getStr() );
        if( it == m_aDropTargets.end() )
            fprintf( stderr, "but no target found\n" );
        else if( ! it->second.m_pTarget->m_bActive )
            fprintf( stderr, "but target is inactive\n" );
        else if( m_aDropEnterEvent.data.l[0] != None && (::Window)m_aDropEnterEvent.data.l[0] != aSource )
            fprintf( stderr, "but source 0x%lx is unknown (expected 0x%lx or 0)\n", aSource, m_aDropEnterEvent.data.l[0] );
        else
            fprintf( stderr, "processing.\n" );
    }
#endif

    if( it != m_aDropTargets.end() && it->second.m_pTarget->m_bActive &&
        m_bDropWaitingForCompletion && m_aDropEnterEvent.data.l[0] )
    {
        bHandled = true;
        OSL_FAIL( "someone forgot to call dropComplete ?" );
        // some listener forgot to call dropComplete in the last operation
        // let us end it now and accept the new enter event
        aGuard.clear();
        dropComplete( false, m_aCurrentDropWindow );
        aGuard.reset();
    }

    if( it != m_aDropTargets.end() &&
        it->second.m_pTarget->m_bActive &&
       ( m_aDropEnterEvent.data.l[0] == None || ::Window(m_aDropEnterEvent.data.l[0]) == aSource )
        )
    {
        if( rMessage.message_type == m_nXdndEnter )
        {
            bHandled = true;
            m_aDropEnterEvent           = rMessage;
            m_bDropEnterSent            = false;
            m_aCurrentDropWindow        = aTarget;
            m_nCurrentProtocolVersion   = m_aDropEnterEvent.data.l[1] >> 24;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndEnter on 0x%lx\n", aTarget );
#endif
        }
        else if(
                rMessage.message_type == m_nXdndPosition &&
                aSource == ::Window(m_aDropEnterEvent.data.l[0])
                )
        {
            bHandled = true;
            m_nDropTime = m_nCurrentProtocolVersion > 0 ? rMessage.data.l[3] : CurrentTime;

            ::Window aChild;
            XTranslateCoordinates( m_pDisplay,
                                   it->second.m_aRootWindow,
                                   it->first,
                                   rMessage.data.l[2] >> 16,
                                   rMessage.data.l[2] & 0xffff,
                                   &m_nLastX, &m_nLastY,
                                   &aChild );

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndPosition on 0x%lx (%d, %d)\n", aTarget, m_nLastX, m_nLastY );
#endif
            DropTargetDragEnterEvent aEvent;
            aEvent.Source       = static_cast< XDropTarget* >(it->second.m_pTarget);
            aEvent.Context      = new DropTargetDragContext( m_aCurrentDropWindow, *this );
            aEvent.LocationX    = m_nLastX;
            aEvent.LocationY    = m_nLastY;
            aEvent.SourceActions = m_nSourceActions;
            if( m_nCurrentProtocolVersion < 2 )
                aEvent.DropAction = DNDConstants::ACTION_COPY;
            else if( Atom(rMessage.data.l[4]) == m_nXdndActionCopy )
                aEvent.DropAction = DNDConstants::ACTION_COPY;
            else if( Atom(rMessage.data.l[4]) == m_nXdndActionMove )
                aEvent.DropAction = DNDConstants::ACTION_MOVE;
            else if( Atom(rMessage.data.l[4]) == m_nXdndActionLink )
                aEvent.DropAction = DNDConstants::ACTION_LINK;
            else if( Atom(rMessage.data.l[4]) == m_nXdndActionAsk )
                // currently no interface to implement ask
                aEvent.DropAction = ~0;
            else
                aEvent.DropAction = DNDConstants::ACTION_NONE;

            m_nLastDropAction   = aEvent.DropAction;
            if( ! m_bDropEnterSent )
            {
                m_bDropEnterSent = true;
                aEvent.SupportedDataFlavors = m_xDropTransferable->getTransferDataFlavors();
                aGuard.clear();
                it->second->dragEnter( aEvent );
            }
            else
            {
                aGuard.clear();
                it->second->dragOver( aEvent );
            }
        }
        else if(
                rMessage.message_type == m_nXdndLeave  &&
                aSource == ::Window(m_aDropEnterEvent.data.l[0])
                )
        {
            bHandled = true;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndLeave on 0x%lx\n", aTarget );
#endif
            DropTargetEvent aEvent;
            aEvent.Source = static_cast< XDropTarget* >(it->second.m_pTarget);
            m_aDropEnterEvent.data.l[0] = None;
            if( m_aCurrentDropWindow == aTarget )
                m_aCurrentDropWindow = None;
            m_nCurrentProtocolVersion = nXdndProtocolRevision;
            aGuard.clear();
            it->second->dragExit( aEvent );
        }
        else if(
                rMessage.message_type == m_nXdndDrop &&
                aSource == ::Window(m_aDropEnterEvent.data.l[0])
                )
        {
            bHandled = true;
            m_nDropTime = m_nCurrentProtocolVersion > 0 ? rMessage.data.l[2] : CurrentTime;

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndDrop on 0x%lx (%d, %d)\n", aTarget, m_nLastX, m_nLastY );
#endif
            if( m_bLastDropAccepted )
            {
                DropTargetDropEvent aEvent;
                aEvent.Source       = static_cast< XDropTarget* >(it->second.m_pTarget);
                aEvent.Context      = new DropTargetDropContext( m_aCurrentDropWindow, *this );
                aEvent.LocationX    = m_nLastX;
                aEvent.LocationY    = m_nLastY;
                aEvent.DropAction   = m_nLastDropAction;
                // there is nothing corresponding to source supported actions
                // every source can do link, copy and move
                aEvent.SourceActions= m_nLastDropAction;
                aEvent.Transferable = m_xDropTransferable;

                m_bDropWaitingForCompletion = true;
                aGuard.clear();
                it->second->drop( aEvent );
            }
            else
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "XdndDrop canceled due to m_bLastDropAccepted = false\n" );
#endif
                DropTargetEvent aEvent;
                aEvent.Source = static_cast< XDropTarget* >(it->second.m_pTarget);
                aGuard.clear();
                it->second->dragExit( aEvent );
                // reset the drop status, notify source
                dropComplete( false, m_aCurrentDropWindow );
            }
        }
    }
    return bHandled;
}

/*
 *  methods for XDropTargetDropContext
 */

void SelectionManager::dropComplete( bool bSuccess, ::Window aDropWindow )
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    if( aDropWindow == m_aCurrentDropWindow )
    {
        if( m_xDragSourceListener.is() )
        {
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = getUserDragAction();
            dsde.DropSuccess        = bSuccess;
            css::uno::Reference< XDragSourceListener > xListener = m_xDragSourceListener;
            m_xDragSourceListener.clear();

            aGuard.clear();
            xListener->dragDropEnd( dsde );
        }
        else if( m_aDropEnterEvent.data.l[0] && m_aCurrentDropWindow )
        {
            XEvent aEvent;
            aEvent.xclient.type         = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = m_aDropEnterEvent.data.l[0];
            aEvent.xclient.message_type = m_nXdndFinished;
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = m_aCurrentDropWindow;
            aEvent.xclient.data.l[1]    = bSuccess ? 1 : 0;
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            if( bSuccess )
            {
                if( m_nLastDropAction & DNDConstants::ACTION_MOVE )
                    aEvent.xclient.data.l[2] = m_nXdndActionMove;
                else if( m_nLastDropAction & DNDConstants::ACTION_COPY )
                    aEvent.xclient.data.l[2] = m_nXdndActionCopy;
                else if( m_nLastDropAction & DNDConstants::ACTION_LINK )
                    aEvent.xclient.data.l[2] = m_nXdndActionLink;
            }

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Sending XdndFinished to 0x%lx\n",
                     m_aDropEnterEvent.data.l[0]
                     );
#endif

            XSendEvent( m_pDisplay, m_aDropEnterEvent.data.l[0],
                        False, NoEventMask, & aEvent );

            m_aDropEnterEvent.data.l[0] = None;
            m_aCurrentDropWindow        = None;
            m_nCurrentProtocolVersion   = nXdndProtocolRevision;
        }
        m_bDropWaitingForCompletion = false;
    }
    else
        OSL_FAIL( "dropComplete from invalid DropTargetDropContext" );
}

/*
 *  methods for XDropTargetDragContext
 */

void SelectionManager::sendDragStatus( Atom nDropAction )
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    if( m_xDragSourceListener.is() )
    {
        sal_Int8 nNewDragAction;
        if( nDropAction == m_nXdndActionMove )
            nNewDragAction = DNDConstants::ACTION_MOVE;
        else if( nDropAction == m_nXdndActionCopy )
            nNewDragAction = DNDConstants::ACTION_COPY;
        else if( nDropAction == m_nXdndActionLink )
            nNewDragAction = DNDConstants::ACTION_LINK;
        else
            nNewDragAction = DNDConstants::ACTION_NONE;
        nNewDragAction &= m_nSourceActions;

        if( nNewDragAction != m_nTargetAcceptAction )
        {
            setCursor( getDefaultCursor( nNewDragAction ), m_aDropWindow );
            m_nTargetAcceptAction = nNewDragAction;
        }

        DragSourceDragEvent dsde;
        dsde.Source             = static_cast< OWeakObject* >(this);
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = m_nSourceActions;
        dsde.UserAction         = getUserDragAction();

        css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
        // caution: do not change anything after this
        aGuard.clear();
        if( xListener.is() )
            xListener->dragOver( dsde );
    }
    else if( m_aDropEnterEvent.data.l[0] && m_aCurrentDropWindow )
    {
        XEvent aEvent;
        aEvent.xclient.type         = ClientMessage;
        aEvent.xclient.display      = m_pDisplay;
        aEvent.xclient.window       = m_aDropEnterEvent.data.l[0];
        aEvent.xclient.message_type = m_nXdndStatus;
        aEvent.xclient.format       = 32;
        aEvent.xclient.data.l[0]    = m_aCurrentDropWindow;
        aEvent.xclient.data.l[1]    = 2;
        if( nDropAction == m_nXdndActionMove    ||
            nDropAction == m_nXdndActionLink    ||
            nDropAction == m_nXdndActionCopy )
            aEvent.xclient.data.l[1] |= 1;
        aEvent.xclient.data.l[2] = 0;
        aEvent.xclient.data.l[3] = 0;
        aEvent.xclient.data.l[4] = m_nCurrentProtocolVersion > 1 ? nDropAction : 0;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Sending XdndStatus to 0x%lx with action %s\n",
                 m_aDropEnterEvent.data.l[0],
                 OUStringToOString( getString( nDropAction ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                 );
#endif

        XSendEvent( m_pDisplay, m_aDropEnterEvent.data.l[0],
                    False, NoEventMask, & aEvent );
        XFlush( m_pDisplay );
    }
}

sal_Int8 SelectionManager::getUserDragAction() const
{
    return (m_nTargetAcceptAction != DNDConstants::ACTION_DEFAULT) ? m_nTargetAcceptAction : m_nUserDragAction;
}

bool SelectionManager::updateDragAction( int modifierState )
{
    bool bRet = false;

    sal_Int8 nNewDropAction = DNDConstants::ACTION_MOVE;
    if( ( modifierState & ShiftMask ) && ! ( modifierState & ControlMask ) )
        nNewDropAction = DNDConstants::ACTION_MOVE;
    else if( ( modifierState & ControlMask ) && ! ( modifierState & ShiftMask ) )
        nNewDropAction = DNDConstants::ACTION_COPY;
    else if( ( modifierState & ShiftMask ) && ( modifierState & ControlMask ) )
        nNewDropAction = DNDConstants::ACTION_LINK;
    if( m_nCurrentProtocolVersion < 0 && m_aDropWindow != None )
        nNewDropAction = DNDConstants::ACTION_COPY;
    nNewDropAction &= m_nSourceActions;

    if( ! ( modifierState & ( ControlMask | ShiftMask ) ) )
    {
        if( ! nNewDropAction )
        {
            // default to an action so the user does not have to press
            // keys explicitly
            if( m_nSourceActions & DNDConstants::ACTION_MOVE )
                nNewDropAction = DNDConstants::ACTION_MOVE;
            else if( m_nSourceActions & DNDConstants::ACTION_COPY )
                nNewDropAction = DNDConstants::ACTION_COPY;
            else if( m_nSourceActions & DNDConstants::ACTION_LINK )
                nNewDropAction = DNDConstants::ACTION_LINK;
        }
        nNewDropAction |= DNDConstants::ACTION_DEFAULT;
    }

    if( nNewDropAction != m_nUserDragAction || m_nTargetAcceptAction != DNDConstants::ACTION_DEFAULT )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "updateDragAction: %x -> %x\n", (int)m_nUserDragAction, (int)nNewDropAction );
#endif
        bRet = true;
        m_nUserDragAction = nNewDropAction;

        DragSourceDragEvent dsde;
        dsde.Source             = static_cast< OWeakObject* >(this);
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = m_nUserDragAction;
        dsde.UserAction         = m_nUserDragAction;
        m_nTargetAcceptAction   = DNDConstants::ACTION_DEFAULT; // invalidate last accept
        m_xDragSourceListener->dropActionChanged( dsde );
    }
    return bRet;
}

void SelectionManager::sendDropPosition( bool bForce, Time eventTime )
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    if( m_bDropSent )
        return;

    std::unordered_map< ::Window, DropTargetEntry >::const_iterator it =
          m_aDropTargets.find( m_aDropWindow );
    if( it != m_aDropTargets.end() )
    {
        if( it->second.m_pTarget->m_bActive )
        {
            int x, y;
            ::Window aChild;
            XTranslateCoordinates( m_pDisplay, it->second.m_aRootWindow, m_aDropWindow, m_nLastDragX, m_nLastDragY, &x, &y, &aChild );
            DropTargetDragEvent dtde;
            dtde.Source         = static_cast< OWeakObject* >(it->second.m_pTarget );
            dtde.Context        = new DropTargetDragContext( m_aCurrentDropWindow, *this );
            dtde.LocationX      = x;
            dtde.LocationY      = y;
            dtde.DropAction     = getUserDragAction();
            dtde.SourceActions  = m_nSourceActions;
            aGuard.clear();
            it->second->dragOver( dtde );
        }
    }
    else if( bForce ||

             m_nLastDragX < m_nNoPosX || m_nLastDragX >= m_nNoPosX+m_nNoPosWidth ||
             m_nLastDragY < m_nNoPosY || m_nLastDragY >= m_nNoPosY+m_nNoPosHeight
             )
    {
        // send XdndPosition
        XEvent aEvent;
        aEvent.type = ClientMessage;
        aEvent.xclient.display      = m_pDisplay;
        aEvent.xclient.format       = 32;
        aEvent.xclient.message_type = m_nXdndPosition;
        aEvent.xclient.window       = m_aDropWindow;
        aEvent.xclient.data.l[0]    = m_aWindow;
        aEvent.xclient.data.l[1]    = 0;
        aEvent.xclient.data.l[2]    = m_nLastDragX << 16 | (m_nLastDragY&0xffff);
        aEvent.xclient.data.l[3]    = eventTime;

        if( m_nUserDragAction & DNDConstants::ACTION_COPY )
            aEvent.xclient.data.l[4]=m_nXdndActionCopy;
        else if( m_nUserDragAction & DNDConstants::ACTION_MOVE )
            aEvent.xclient.data.l[4]=m_nXdndActionMove;
        else if( m_nUserDragAction & DNDConstants::ACTION_LINK )
            aEvent.xclient.data.l[4]=m_nXdndActionLink;
        else
            aEvent.xclient.data.l[4]=m_nXdndActionCopy;
        XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
        m_nNoPosX = m_nNoPosY = m_nNoPosWidth = m_nNoPosHeight = 0;
    }
}

bool SelectionManager::handleDragEvent( XEvent const & rMessage )
{
    if( ! m_xDragSourceListener.is() )
        return false;

    osl::ResettableMutexGuard aGuard(m_aMutex);

    bool bHandled = false;

    // for shortcut
    std::unordered_map< ::Window, DropTargetEntry >::const_iterator it =
          m_aDropTargets.find( m_aDropWindow );
#if OSL_DEBUG_LEVEL > 1
    switch( rMessage.type )
    {
        case ClientMessage:
            fprintf( stderr, "handleDragEvent: %s\n", OUStringToOString( getString( rMessage.xclient.message_type ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
            break;
        case MotionNotify:
            break;
        case EnterNotify:
            fprintf( stderr, "handleDragEvent: EnterNotify\n" );
            break;
        case LeaveNotify:
            fprintf( stderr, "handleDragEvent: LeaveNotify\n" );
            break;
        case ButtonPress:
            fprintf( stderr, "handleDragEvent: ButtonPress %d (m_nDragButton = %d)\n", rMessage.xbutton.button, m_nDragButton );
            break;
        case ButtonRelease:
            fprintf( stderr, "handleDragEvent: ButtonRelease %d (m_nDragButton = %d)\n", rMessage.xbutton.button, m_nDragButton );
            break;
        case KeyPress:
            fprintf( stderr, "handleDragEvent: KeyPress\n" );
            break;
        case KeyRelease:
            fprintf( stderr, "handleDragEvent: KeyRelease\n" );
            break;
        default:
            fprintf( stderr, "handleDragEvent: <unknown type %d>\n", rMessage.type );
            break;
    }
#endif

    // handle drag related events
    if( rMessage.type == ClientMessage )
    {
        if( rMessage.xclient.message_type == m_nXdndStatus && Atom(rMessage.xclient.data.l[0]) == m_aDropWindow )
        {
            bHandled = true;
            DragSourceDragEvent dsde;
            dsde.Source                 = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext      = new DragSourceContext( m_aDropWindow, *this );
            dsde.DragSource             = static_cast< XDragSource* >( this );
            dsde.UserAction = getUserDragAction();
            dsde.DropAction = DNDConstants::ACTION_NONE;
            m_bDropSuccess = (rMessage.xclient.data.l[1] & 1) != 0;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "status drop action: accept = %s, %s\n",
                     m_bDropSuccess ? "true" : "false",
                     OUStringToOString( getString( rMessage.xclient.data.l[4] ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
            if( rMessage.xclient.data.l[1] & 1 )
            {
                if( m_nCurrentProtocolVersion > 1 )
                {
                    if( Atom(rMessage.xclient.data.l[4]) == m_nXdndActionCopy )
                        dsde.DropAction = DNDConstants::ACTION_COPY;
                    else if( Atom(rMessage.xclient.data.l[4]) == m_nXdndActionMove )
                        dsde.DropAction = DNDConstants::ACTION_MOVE;
                    else if( Atom(rMessage.xclient.data.l[4]) == m_nXdndActionLink )
                        dsde.DropAction = DNDConstants::ACTION_LINK;
                }
                else
                    dsde.DropAction = DNDConstants::ACTION_COPY;
            }
            m_nTargetAcceptAction = dsde.DropAction;

            if( ! ( rMessage.xclient.data.l[1] & 2 ) )
            {
                m_nNoPosX       = rMessage.xclient.data.l[2] >> 16;
                m_nNoPosY       = rMessage.xclient.data.l[2] & 0xffff;
                m_nNoPosWidth   = rMessage.xclient.data.l[3] >> 16;
                m_nNoPosHeight  = rMessage.xclient.data.l[3] & 0xffff;
            }
            else
                m_nNoPosX = m_nNoPosY = m_nNoPosWidth = m_nNoPosHeight = 0;

            setCursor( getDefaultCursor( dsde.DropAction ), m_aDropWindow );
            aGuard.clear();
            m_xDragSourceListener->dragOver( dsde );
        }
        else if( rMessage.xclient.message_type == m_nXdndFinished && m_aDropWindow == Atom(rMessage.xclient.data.l[0]) )
        {
            bHandled = true;
            // notify the listener
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = m_nTargetAcceptAction;
            dsde.DropSuccess        = m_bDropSuccess;
            css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
            m_xDragSourceListener.clear();
            aGuard.clear();
            xListener->dragDropEnd( dsde );
        }
    }
    else if( rMessage.type == MotionNotify ||
             rMessage.type == EnterNotify || rMessage.type == LeaveNotify
             )
    {
        bHandled = true;
        bool bForce = false;
        int root_x  = rMessage.type == MotionNotify ? rMessage.xmotion.x_root : rMessage.xcrossing.x_root;
        int root_y  = rMessage.type == MotionNotify ? rMessage.xmotion.y_root : rMessage.xcrossing.y_root;
        ::Window root = rMessage.type == MotionNotify ? rMessage.xmotion.root : rMessage.xcrossing.root;

        aGuard.clear();
        if( rMessage.type == MotionNotify )
        {
            bForce = updateDragAction( rMessage.xmotion.state );
        }
        updateDragWindow( root_x, root_y, root );
        aGuard.reset();

        if( m_nCurrentProtocolVersion >= 0 && m_aDropProxy != None )
        {
            aGuard.clear();
            sendDropPosition( bForce, rMessage.type == MotionNotify ? rMessage.xmotion.time : rMessage.xcrossing.time );
        }
    }
    else if( rMessage.type == KeyPress || rMessage.type == KeyRelease )
    {
        bHandled = true;
        KeySym aKey = XkbKeycodeToKeysym( m_pDisplay, rMessage.xkey.keycode, 0, 0 );
        if( aKey == XK_Escape )
        {
            // abort drag
            if( it != m_aDropTargets.end() )
            {
                DropTargetEvent dte;
                dte.Source = static_cast< OWeakObject* >( it->second.m_pTarget );
                aGuard.clear();
                it->second.m_pTarget->dragExit( dte );
            }
            else if( m_aDropProxy != None && m_nCurrentProtocolVersion >= 0 )
            {
                // send XdndLeave
                XEvent aEvent;
                aEvent.type = ClientMessage;
                aEvent.xclient.display      = m_pDisplay;
                aEvent.xclient.format       = 32;
                aEvent.xclient.message_type = m_nXdndLeave;
                aEvent.xclient.window       = m_aDropWindow;
                aEvent.xclient.data.l[0]    = m_aWindow;
                memset( aEvent.xclient.data.l+1, 0, sizeof(long)*4);
                m_aDropWindow = m_aDropProxy = None;
                XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
            }
            // notify the listener
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = false;
            css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
            m_xDragSourceListener.clear();
            aGuard.clear();
            xListener->dragDropEnd( dsde );
        }
        else
        {
            /*
             *  man page says: state is state immediate PRIOR to the
             *  event. It would seem that this is a somewhat arguable
             *  design decision.
             */
            int nState = rMessage.xkey.state;
            int nNewState = 0;
            switch( aKey )
            {
                case XK_Shift_R:
                case XK_Shift_L: nNewState = ShiftMask;break;
                case XK_Control_R:
                case XK_Control_L: nNewState = ControlMask;break;
                    // just interested in shift and ctrl for dnd
            }
            if( rMessage.type == KeyPress )
                nState |= nNewState;
            else
                nState &= ~nNewState;
            aGuard.clear();
            if( updateDragAction( nState ) )
                sendDropPosition( true, rMessage.xkey.time );
        }
    }
    else if(
            ( rMessage.type == ButtonPress || rMessage.type == ButtonRelease ) &&
            rMessage.xbutton.button == m_nDragButton )
    {
        bool bCancel = true;
        if( m_aDropWindow != None )
        {
            if( it != m_aDropTargets.end() )
            {
                if( it->second.m_pTarget->m_bActive && m_nUserDragAction != DNDConstants::ACTION_NONE && m_bLastDropAccepted )
                {
                    bHandled = true;
                    int x, y;
                    ::Window aChild;
                    XTranslateCoordinates( m_pDisplay, rMessage.xbutton.root, m_aDropWindow, rMessage.xbutton.x_root, rMessage.xbutton.y_root, &x, &y, &aChild );
                    DropTargetDropEvent dtde;
                    dtde.Source         = static_cast< OWeakObject* >(it->second.m_pTarget );
                    dtde.Context        = new DropTargetDropContext( m_aCurrentDropWindow, *this );
                    dtde.LocationX      = x;
                    dtde.LocationY      = y;
                    dtde.DropAction     = m_nUserDragAction;
                    dtde.SourceActions  = m_nSourceActions;
                    dtde.Transferable   = m_xDragSourceTransferable;
                    m_bDropSent                 = true;
                    m_nDropTimeout              = time( nullptr );
                    m_bDropWaitingForCompletion = true;
                    aGuard.clear();
                    it->second->drop( dtde );
                    bCancel = false;
                }
                else bCancel = true;
            }
            else if( m_nCurrentProtocolVersion >= 0 )
            {
                bHandled = true;

                XEvent aEvent;
                aEvent.type = ClientMessage;
                aEvent.xclient.display      = m_pDisplay;
                aEvent.xclient.format       = 32;
                aEvent.xclient.message_type = m_nXdndDrop;
                aEvent.xclient.window       = m_aDropWindow;
                aEvent.xclient.data.l[0]    = m_aWindow;
                aEvent.xclient.data.l[1]    = 0;
                aEvent.xclient.data.l[2]    = rMessage.xbutton.time;
                aEvent.xclient.data.l[3]    = 0;
                aEvent.xclient.data.l[4]    = 0;

                m_bDropSent                 = true;
                m_nDropTimeout              = time( nullptr );
                XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
                bCancel = false;
            }
            else
            {
                // dropping on non XdndWindows: acquire ownership of
                // PRIMARY and send a middle mouse button click down/up to
                // target window
                SelectionAdaptor* pAdaptor = getAdaptor( XA_PRIMARY );
                if( pAdaptor )
                {
                    bHandled = true;

                    ::Window aDummy;
                    XEvent aEvent;
                    aEvent.type = ButtonPress;
                    aEvent.xbutton.display      = m_pDisplay;
                    aEvent.xbutton.window       = m_aDropWindow;
                    aEvent.xbutton.root         = rMessage.xbutton.root;
                    aEvent.xbutton.subwindow    = m_aDropWindow;
                    aEvent.xbutton.time         = rMessage.xbutton.time+1;
                    aEvent.xbutton.x_root       = rMessage.xbutton.x_root;
                    aEvent.xbutton.y_root       = rMessage.xbutton.y_root;
                    aEvent.xbutton.state        = rMessage.xbutton.state;
                    aEvent.xbutton.button       = Button2;
                    aEvent.xbutton.same_screen  = True;
                    XTranslateCoordinates( m_pDisplay,
                                           rMessage.xbutton.root, m_aDropWindow,
                                           rMessage.xbutton.x_root, rMessage.xbutton.y_root,
                                           &aEvent.xbutton.x, &aEvent.xbutton.y,
                                           &aDummy );
                    XSendEvent( m_pDisplay, m_aDropWindow, False, ButtonPressMask, &aEvent );
                    aEvent.xbutton.type   = ButtonRelease;
                    aEvent.xbutton.time++;
                    aEvent.xbutton.state |= Button2Mask;
                    XSendEvent( m_pDisplay, m_aDropWindow, False, ButtonReleaseMask, &aEvent );

                    m_bDropSent                 = true;
                    m_nDropTimeout              = time( nullptr );
                    XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
                    m_bWaitingForPrimaryConversion  = true;
                    m_bDropSent                     = true;
                    m_nDropTimeout                  = time( nullptr );
                    // HACK :-)
                    aGuard.clear();
                    static_cast< X11Clipboard* >( pAdaptor )->setContents( m_xDragSourceTransferable, css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner >() );
                    aGuard.reset();
                    bCancel = false;
                }
            }
        }
        if( bCancel )
        {
            // cancel drag
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = false;
            css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
            m_xDragSourceListener.clear();
            aGuard.clear();
            xListener->dragDropEnd( dsde );
            bHandled = true;
        }
    }
    return bHandled;
}

void SelectionManager::accept( sal_Int8 dragOperation, ::Window aDropWindow )
{
    if( aDropWindow == m_aCurrentDropWindow )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "accept: %x\n", dragOperation );
#endif
        Atom nAction = None;
        dragOperation &= (DNDConstants::ACTION_MOVE | DNDConstants::ACTION_COPY | DNDConstants::ACTION_LINK);
        if( dragOperation & DNDConstants::ACTION_MOVE )
            nAction = m_nXdndActionMove;
        else if( dragOperation & DNDConstants::ACTION_COPY )
            nAction = m_nXdndActionCopy;
        else if( dragOperation & DNDConstants::ACTION_LINK )
            nAction = m_nXdndActionLink;
        m_bLastDropAccepted = true;
        sendDragStatus( nAction );
    }
}

void SelectionManager::reject( ::Window aDropWindow )
{
    if( aDropWindow == m_aCurrentDropWindow )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "reject\n" );
#endif
        m_bLastDropAccepted = false;
        sendDragStatus( None );
        if( m_bDropSent && m_xDragSourceListener.is() )
        {
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = false;
            m_xDragSourceListener->dragDropEnd( dsde );
            m_xDragSourceListener.clear();
        }
    }
}

/*
 *  XDragSource
 */

sal_Bool SelectionManager::isDragImageSupported()
{
    return false;
}

sal_Int32 SelectionManager::getDefaultCursor( sal_Int8 dragAction )
{
    Cursor aCursor = m_aNoneCursor;
    if( dragAction & DNDConstants::ACTION_MOVE )
        aCursor = m_aMoveCursor;
    else if( dragAction & DNDConstants::ACTION_COPY )
        aCursor = m_aCopyCursor;
    else if( dragAction & DNDConstants::ACTION_LINK )
        aCursor = m_aLinkCursor;
    return aCursor;
}

int SelectionManager::getXdndVersion( ::Window aWindow, ::Window& rProxy )
{
    Atom* pProperties = nullptr;
    int nProperties = 0;
    Atom nType;
    int nFormat;
    unsigned long nItems, nBytes;
    unsigned char* pBytes = nullptr;

    int nVersion = -1;
    rProxy = None;

    /*
     *  XListProperties is used here to avoid unnecessary XGetWindowProperty calls
     *  and therefore reducing latency penalty
     */
    pProperties = XListProperties( m_pDisplay, aWindow, &nProperties );
    // first look for proxy
    int i;
    for( i = 0; i < nProperties; i++ )
    {
        if( pProperties[i] == m_nXdndProxy )
        {
            XGetWindowProperty( m_pDisplay, aWindow, m_nXdndProxy, 0, 1, False, XA_WINDOW,
                                &nType, &nFormat, &nItems, &nBytes, &pBytes );
            if( pBytes )
            {
                if( nType == XA_WINDOW )
                    rProxy = *reinterpret_cast< ::Window* >(pBytes);
                XFree( pBytes );
                pBytes = nullptr;
                if( rProxy != None )
                {
                    // now check proxy whether it points to itself
                    XGetWindowProperty( m_pDisplay, rProxy, m_nXdndProxy, 0, 1, False, XA_WINDOW,
                                        &nType, &nFormat, &nItems, &nBytes, &pBytes );
                    if( pBytes )
                    {
                        if( nType == XA_WINDOW && *reinterpret_cast< ::Window* >(pBytes) != rProxy )
                            rProxy = None;
                        XFree( pBytes );
                        pBytes = nullptr;
                    }
                    else
                        rProxy = None;
                }
            }
            break;
        }
    }
    if ( pProperties )
        XFree (pProperties);

    ::Window aAwareWindow = rProxy != None ? rProxy : aWindow;

    XGetWindowProperty( m_pDisplay, aAwareWindow, m_nXdndAware, 0, 1, False, XA_ATOM,
                        &nType, &nFormat, &nItems, &nBytes, &pBytes );
    if( pBytes )
    {
        if( nType == XA_ATOM )
            nVersion = *reinterpret_cast<Atom*>(pBytes);
        XFree( pBytes );
    }

    nVersion = std::min<int>(nVersion, nXdndProtocolRevision);

    return nVersion;
}

void SelectionManager::updateDragWindow( int nX, int nY, ::Window aRoot )
{
    osl::ResettableMutexGuard aGuard( m_aMutex );

    css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );

    m_nLastDragX = nX;
    m_nLastDragY = nY;

    ::Window aParent = aRoot;
    ::Window aChild;
    ::Window aNewProxy = None, aNewCurrentWindow = None;
    int nNewProtocolVersion = -1;
    int nWinX, nWinY;

    // find the first XdndAware window or check if root window is
    // XdndAware or has XdndProxy
    do
    {
        XTranslateCoordinates( m_pDisplay, aRoot, aParent, nX, nY, &nWinX, &nWinY, &aChild );
        if( aChild != None )
        {
            if( aChild == m_aCurrentDropWindow && aChild != aRoot && m_nCurrentProtocolVersion >= 0 )
            {
                aParent = aChild;
                break;
            }
            nNewProtocolVersion = getXdndVersion( aChild, aNewProxy );
            aParent = aChild;
        }
    } while( aChild != None && nNewProtocolVersion < 0 );

    aNewCurrentWindow = aParent;
    if( aNewCurrentWindow == aRoot )
    {
        // no children, try root drop
        nNewProtocolVersion = getXdndVersion( aNewCurrentWindow, aNewProxy );
        if( nNewProtocolVersion < 3 )
        {
            aNewCurrentWindow = aNewProxy = None;
            nNewProtocolVersion = nXdndProtocolRevision;
        }
    }

    DragSourceDragEvent dsde;
    dsde.Source             = static_cast< OWeakObject* >(this);
    dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
    dsde.DragSource         = static_cast< XDragSource* >(this);
    dsde.DropAction         = nNewProtocolVersion >= 0 ? m_nUserDragAction : DNDConstants::ACTION_COPY;
    dsde.UserAction         = nNewProtocolVersion >= 0 ? m_nUserDragAction : DNDConstants::ACTION_COPY;

    std::unordered_map< ::Window, DropTargetEntry >::const_iterator it;
    if( aNewCurrentWindow != m_aDropWindow )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "drag left window 0x%lx (rev. %d), entered window 0x%lx (rev %d)\n", m_aDropWindow, m_nCurrentProtocolVersion, aNewCurrentWindow, nNewProtocolVersion );
#endif

        if( m_aDropWindow != None )
        {
            it = m_aDropTargets.find( m_aDropWindow );
            if( it != m_aDropTargets.end() )
                // shortcut for own drop targets
            {
                DropTargetEvent dte;
                dte.Source  = static_cast< OWeakObject* >( it->second.m_pTarget );
                aGuard.clear();
                it->second.m_pTarget->dragExit( dte );
                aGuard.reset();
            }
            else
            {
                // send old drop target a XdndLeave
                XEvent aEvent;
                aEvent.type = ClientMessage;
                aEvent.xclient.display          = m_pDisplay;
                aEvent.xclient.format           = 32;
                aEvent.xclient.message_type     = m_nXdndLeave;
                aEvent.xclient.window           = m_aDropWindow;
                aEvent.xclient.data.l[0]        = m_aWindow;
                aEvent.xclient.data.l[1]        = 0;
                XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
            }
            if( xListener.is() )
            {
                aGuard.clear();
                xListener->dragExit( dsde );
                aGuard.reset();
            }
        }

        m_nCurrentProtocolVersion   = nNewProtocolVersion;
        m_aDropWindow               = aNewCurrentWindow;
        m_aDropProxy                = aNewProxy != None ? aNewProxy : m_aDropWindow;

        it = m_aDropTargets.find( m_aDropWindow );
        if( it != m_aDropTargets.end() && ! it->second.m_pTarget->m_bActive )
            m_aDropProxy = None;

        if( m_aDropProxy != None && xListener.is() )
        {
            aGuard.clear();
            xListener->dragEnter( dsde );
            aGuard.reset();
        }
        // send XdndEnter
        if( m_aDropProxy != None && m_nCurrentProtocolVersion >= 0 )
        {
            it = m_aDropTargets.find( m_aDropWindow );
            if( it != m_aDropTargets.end() )
            {
                XTranslateCoordinates( m_pDisplay, aRoot, m_aDropWindow, nX, nY, &nWinX, &nWinY, &aChild );
                DropTargetDragEnterEvent dtde;
                dtde.Source                 = static_cast< OWeakObject* >( it->second.m_pTarget );
                dtde.Context                = new DropTargetDragContext( m_aCurrentDropWindow, *this );
                dtde.LocationX              = nWinX;
                dtde.LocationY              = nWinY;
                dtde.DropAction             = m_nUserDragAction;
                dtde.SourceActions          = m_nSourceActions;
                dtde.SupportedDataFlavors   = m_xDragSourceTransferable->getTransferDataFlavors();
                aGuard.clear();
                it->second.m_pTarget->dragEnter( dtde );
                aGuard.reset();
            }
            else
            {
                XEvent aEvent;
                aEvent.type = ClientMessage;
                aEvent.xclient.display          = m_pDisplay;
                aEvent.xclient.format           = 32;
                aEvent.xclient.message_type = m_nXdndEnter;
                aEvent.xclient.window       = m_aDropWindow;
                aEvent.xclient.data.l[0]    = m_aWindow;
                aEvent.xclient.data.l[1]    = m_nCurrentProtocolVersion << 24;
                memset( aEvent.xclient.data.l + 2, 0, sizeof( long )*3 );
                // fill in data types
                ::std::list< Atom > aConversions;
                getNativeTypeList( m_aDragFlavors, aConversions, m_nXdndSelection );
                if( aConversions.size() > 3 )
                    aEvent.xclient.data.l[1] |= 1;
                ::std::list< Atom >::const_iterator type_it = aConversions.begin();
                for( int i = 0; type_it != aConversions.end() && i < 3; i++, ++type_it )
                    aEvent.xclient.data.l[i+2] = *type_it;
                XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
            }
        }
        m_nNoPosX = m_nNoPosY = m_nNoPosWidth = m_nNoPosHeight = 0;
    }
    else if( m_aDropProxy != None && xListener.is() )
    {
        aGuard.clear();
        // drag over for XdndAware windows comes when receiving XdndStatus
        xListener->dragOver( dsde );
    }
}

void SelectionManager::startDrag(
                                 const DragGestureEvent& trigger,
                                 sal_Int8 sourceActions,
                                 sal_Int32,
                                 sal_Int32,
                                 const css::uno::Reference< XTransferable >& transferable,
                                 const css::uno::Reference< XDragSourceListener >& listener
                                 )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "startDrag( sourceActions = %x )\n", (int)sourceActions );
#endif

    DragSourceDropEvent aDragFailedEvent;
    aDragFailedEvent.Source             = static_cast< OWeakObject* >(this);
    aDragFailedEvent.DragSource         = static_cast< XDragSource* >(this);
    aDragFailedEvent.DragSourceContext  = new DragSourceContext( None, *this );
    aDragFailedEvent.DropAction         = DNDConstants::ACTION_NONE;
    aDragFailedEvent.DropSuccess        = false;

    if( m_aDragRunning.check() )
    {
        if( listener.is() )
            listener->dragDropEnd( aDragFailedEvent );

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "*** ERROR *** second drag and drop started.\n" );
        if( m_xDragSourceListener.is() )
            fprintf( stderr, "*** ERROR *** drag source listener already set.\n" );
        else
            fprintf( stderr, "*** ERROR *** drag thread already running.\n" );
#endif
        return;
    }

    SalFrame* pCaptureFrame = nullptr;

    {
        osl::ClearableMutexGuard aGuard(m_aMutex);

        // first get the current pointer position and the window that
        // the pointer is located in. since said window should be one
        // of our DropTargets at the time of executeDrag we can use
        // them for a start
        ::Window aRoot, aParent, aChild;
        int root_x(0), root_y(0), win_x(0), win_y(0);
        unsigned int mask(0);

        bool bPointerFound = false;
        for (auto const& dropTarget : m_aDropTargets)
        {
            if( XQueryPointer( m_pDisplay, dropTarget.second.m_aRootWindow,
                               &aRoot, &aParent,
                               &root_x, &root_y,
                               &win_x, &win_y,
                               &mask ) )
            {
                aParent = dropTarget.second.m_aRootWindow;
                aRoot = aParent;
                bPointerFound = true;
                break;
            }
        }

        // don't start DnD if there is none of our windows on the same screen as
        // the pointer or if no mouse button is pressed
        if( !bPointerFound || (mask & (Button1Mask|Button2Mask|Button3Mask)) == 0 )
        {
            aGuard.clear();
            if( listener.is() )
                listener->dragDropEnd( aDragFailedEvent );
            return;
        }

        // try to find which of our drop targets is the drag source
        // if that drop target is deregistered we should stop executing
        // the drag (actually this is a poor substitute for an "endDrag"
        // method ).
        m_aDragSourceWindow = None;
        do
        {
            XTranslateCoordinates( m_pDisplay, aRoot, aParent, root_x, root_y, &win_x, &win_y, &aChild );
            if( aChild != None && m_aDropTargets.find( aChild ) != m_aDropTargets.end() )
            {
                m_aDragSourceWindow = aChild;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "found drag source window 0x%lx\n", m_aDragSourceWindow );
#endif
                break;
            }
            aParent = aChild;
        } while( aChild != None );

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "try to grab pointer ... " );
#endif
        int nPointerGrabSuccess =
            XGrabPointer( m_pDisplay, aRoot, True,
                          DRAG_EVENT_MASK,
                          GrabModeAsync, GrabModeAsync,
                          None,
                          None,
                          CurrentTime );
        /* if we could not grab the pointer here, there is a chance
           that the pointer is grabbed by the other vcl display (the main loop)
           so let's break that grab an reset it later

           remark: this whole code should really be molten into normal vcl so only
           one display is used ....
        */
        if( nPointerGrabSuccess != GrabSuccess )
        {
            comphelper::SolarMutex& rSolarMutex( Application::GetSolarMutex() );
            if( rSolarMutex.tryToAcquire() )
            {
                pCaptureFrame = vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetCaptureFrame();
                if( pCaptureFrame )
                {
                    vcl_sal::getSalDisplay(GetGenericUnixSalData())->CaptureMouse( nullptr );
                    nPointerGrabSuccess =
                                XGrabPointer( m_pDisplay, aRoot, True,
                                              DRAG_EVENT_MASK,
                                              GrabModeAsync, GrabModeAsync,
                                              None,
                                              None,
                                              CurrentTime );
                }
            }
        }
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "%d\n", nPointerGrabSuccess );
#endif
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "try to grab keyboard ... " );
#endif
        int nKeyboardGrabSuccess =
            XGrabKeyboard( m_pDisplay, aRoot, True,
                           GrabModeAsync, GrabModeAsync, CurrentTime );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "%d\n", nKeyboardGrabSuccess );
#endif
        if( nPointerGrabSuccess != GrabSuccess || nKeyboardGrabSuccess != GrabSuccess )
        {
            if( nPointerGrabSuccess == GrabSuccess )
                XUngrabPointer( m_pDisplay, CurrentTime );
            if( nKeyboardGrabSuccess == GrabSuccess )
                XUngrabKeyboard( m_pDisplay, CurrentTime );
            XFlush( m_pDisplay );
            aGuard.clear();
            if( listener.is() )
                listener->dragDropEnd( aDragFailedEvent );
            if( pCaptureFrame )
            {
                comphelper::SolarMutex& rSolarMutex( Application::GetSolarMutex() );
                if( rSolarMutex.tryToAcquire() )
                    vcl_sal::getSalDisplay(GetGenericUnixSalData())->CaptureMouse( pCaptureFrame );
#if OSL_DEBUG_LEVEL > 0
                else
                    OSL_FAIL( "failed to acquire SolarMutex to reset capture frame" );
#endif
            }
            return;
        }

        m_xDragSourceTransferable   = transferable;
        m_xDragSourceListener       = listener;
        m_aDragFlavors              = transferable->getTransferDataFlavors();
        m_aCurrentCursor            = None;

        requestOwnership( m_nXdndSelection );

        ::std::list< Atom > aConversions;
        getNativeTypeList( m_aDragFlavors, aConversions, m_nXdndSelection );

        Atom* pTypes = static_cast<Atom*>(alloca( sizeof(Atom)*aConversions.size() ));
        int nTypes = 0;
        for (auto const& conversion : aConversions)
            pTypes[nTypes++] = conversion;

        XChangeProperty( m_pDisplay, m_aWindow, m_nXdndTypeList, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(pTypes), nTypes );

        m_nSourceActions                = sourceActions | DNDConstants::ACTION_DEFAULT;
        m_nUserDragAction               = DNDConstants::ACTION_MOVE & m_nSourceActions;
        if( ! m_nUserDragAction )
            m_nUserDragAction           = DNDConstants::ACTION_COPY & m_nSourceActions;
        if( ! m_nUserDragAction )
            m_nUserDragAction           = DNDConstants::ACTION_LINK & m_nSourceActions;
        m_nTargetAcceptAction           = DNDConstants::ACTION_DEFAULT;
        m_bDropSent                     = false;
        m_bDropSuccess                  = false;
        m_bWaitingForPrimaryConversion  = false;
        m_nDragButton                   = Button1; // default to left button
        css::awt::MouseEvent aEvent;
        if( trigger.Event >>= aEvent )
        {
            if( aEvent.Buttons & MouseButton::LEFT )
                m_nDragButton = Button1;
            else if( aEvent.Buttons & MouseButton::RIGHT )
                m_nDragButton = Button3;
            else if( aEvent.Buttons & MouseButton::MIDDLE )
                m_nDragButton = Button2;
        }
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "m_nUserDragAction = %x\n", (int)m_nUserDragAction );
#endif
        updateDragWindow( root_x, root_y, aRoot );
        m_nUserDragAction = ~0;
        updateDragAction( mask );
    }

    m_aDragRunning.set();
    m_aDragExecuteThread = osl_createSuspendedThread( call_SelectionManager_runDragExecute, this );
    if( m_aDragExecuteThread )
        osl_resumeThread( m_aDragExecuteThread );
    else
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "osl_createSuspendedThread failed for drag execute\n" );
#endif
        m_xDragSourceListener.clear();
        m_xDragSourceTransferable.clear();

        m_bDropSent                         = false;
        m_bDropSuccess                      = false;
        m_bWaitingForPrimaryConversion      = false;
        m_aDropWindow                       = None;
        m_aDropProxy                        = None;
        m_nCurrentProtocolVersion           = nXdndProtocolRevision;
        m_nNoPosX                           = 0;
        m_nNoPosY                           = 0;
        m_nNoPosWidth                       = 0;
        m_nNoPosHeight                      = 0;
        m_aCurrentCursor                    = None;

        XUngrabPointer( m_pDisplay, CurrentTime );
        XUngrabKeyboard( m_pDisplay, CurrentTime );
        XFlush( m_pDisplay );

        if( pCaptureFrame )
        {
            comphelper::SolarMutex& rSolarMutex( Application::GetSolarMutex() );
            if( rSolarMutex.tryToAcquire() )
                vcl_sal::getSalDisplay(GetGenericUnixSalData())->CaptureMouse( pCaptureFrame );
#if OSL_DEBUG_LEVEL > 0
            else
                OSL_FAIL( "failed to acquire SolarMutex to reset capture frame" );
#endif
        }

        m_aDragRunning.reset();

        if( listener.is() )
            listener->dragDropEnd( aDragFailedEvent );
    }
}

void SelectionManager::runDragExecute( void* pThis )
{
    SelectionManager* This = static_cast<SelectionManager*>(pThis);
    This->dragDoDispatch();
}

void SelectionManager::dragDoDispatch()
{

    // do drag
    // m_xDragSourceListener will be cleared on finished drop
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "begin executeDrag dispatching\n" );
#endif
    oslThread aThread = m_aDragExecuteThread;
    while( m_xDragSourceListener.is() && ( ! m_bDropSent || time(nullptr)-m_nDropTimeout < 5 ) && osl_scheduleThread( aThread ) )
    {
        // let the thread in the run method do the dispatching
        // just look occasionally here whether drop timed out or is completed
        osl::Thread::wait(std::chrono::milliseconds(200));
    }
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "end executeDrag dispatching\n" );
#endif
    {
        osl::ClearableMutexGuard aGuard(m_aMutex);

        css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
        css::uno::Reference< XTransferable > xTransferable( m_xDragSourceTransferable );
        m_xDragSourceListener.clear();
        m_xDragSourceTransferable.clear();

        DragSourceDropEvent dsde;
        dsde.Source             = static_cast< OWeakObject* >(this);
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = DNDConstants::ACTION_NONE;
        dsde.DropSuccess        = false;

        // cleanup after drag
        if( m_bWaitingForPrimaryConversion )
        {
            SelectionAdaptor* pAdaptor = getAdaptor( XA_PRIMARY );
            if (pAdaptor)
                pAdaptor->clearTransferable();
        }

        m_bDropSent                         = false;
        m_bDropSuccess                      = false;
        m_bWaitingForPrimaryConversion      = false;
        m_aDropWindow                       = None;
        m_aDropProxy                        = None;
        m_nCurrentProtocolVersion           = nXdndProtocolRevision;
        m_nNoPosX                           = 0;
        m_nNoPosY                           = 0;
        m_nNoPosWidth                       = 0;
        m_nNoPosHeight                      = 0;
        m_aCurrentCursor                    = None;

        XUngrabPointer( m_pDisplay, CurrentTime );
        XUngrabKeyboard( m_pDisplay, CurrentTime );
        XFlush( m_pDisplay );

        m_aDragExecuteThread = nullptr;
        m_aDragRunning.reset();

        aGuard.clear();
        if( xListener.is() )
        {
            xTransferable.clear();
            xListener->dragDropEnd( dsde );
        }
    }
    osl_destroyThread( aThread );
}

/*
 *  XDragSourceContext
 */


void SelectionManager::setCursor( sal_Int32 cursor, ::Window aDropWindow )
{
    osl::MutexGuard aGuard( m_aMutex );
    if( aDropWindow == m_aDropWindow && Cursor(cursor) != m_aCurrentCursor )
    {
        if( m_xDragSourceListener.is() && ! m_bDropSent )
        {
            m_aCurrentCursor = cursor;
            XChangeActivePointerGrab( m_pDisplay, DRAG_EVENT_MASK, cursor, CurrentTime );
            XFlush( m_pDisplay );
        }
    }
}

void SelectionManager::transferablesFlavorsChanged()
{
    osl::MutexGuard aGuard(m_aMutex);

    m_aDragFlavors = m_xDragSourceTransferable->getTransferDataFlavors();

    std::list< Atom > aConversions;

    getNativeTypeList( m_aDragFlavors, aConversions, m_nXdndSelection );

    Atom* pTypes = static_cast<Atom*>(alloca( sizeof(Atom)*aConversions.size() ));
    int nTypes = 0;
    for (auto const& conversion : aConversions)
        pTypes[nTypes++] = conversion;
    XChangeProperty( m_pDisplay, m_aWindow, m_nXdndTypeList, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(pTypes), nTypes );

    if( m_aCurrentDropWindow == None || m_nCurrentProtocolVersion < 0 )
        return;

    // send synthetic leave and enter events

    XEvent aEvent;

    aEvent.type = ClientMessage;
    aEvent.xclient.display          = m_pDisplay;
    aEvent.xclient.format           = 32;
    aEvent.xclient.window           = m_aDropWindow;
    aEvent.xclient.data.l[0]        = m_aWindow;

    aEvent.xclient.message_type     = m_nXdndLeave;
    aEvent.xclient.data.l[1]        = 0;
    XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );

    aEvent.xclient.message_type = m_nXdndEnter;
    aEvent.xclient.data.l[1]    = m_nCurrentProtocolVersion << 24;
    memset( aEvent.xclient.data.l + 2, 0, sizeof( long )*3 );
    // fill in data types
    if( nTypes > 3 )
        aEvent.xclient.data.l[1] |= 1;
    for( int j = 0; j < nTypes && j < 3; j++ )
        aEvent.xclient.data.l[j+2] = pTypes[j];

    XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );

}

/*
 *  dispatch loop
 */

bool SelectionManager::handleXEvent( XEvent& rEvent )
{
    /*
     *  since we are XConnectionListener to a second X display
     *  to get client messages it is essential not to dispatch
     *  events twice that we get on both connections
     *
     *  between dispatching ButtonPress and startDrag
     *  the user can already have released the mouse. The ButtonRelease
     *  will then be dispatched in VCLs queue and never turn up here.
     *  Which is not so good, since startDrag will XGrabPointer and
     *  XGrabKeyboard -> solid lock.
     */
    if( rEvent.xany.display != m_pDisplay
        && rEvent.type != ClientMessage
        && rEvent.type != ButtonPress
        && rEvent.type != ButtonRelease
        )
        return false;

    bool bHandled = false;
    switch (rEvent.type)
    {
        case SelectionClear:
        {
            osl::ClearableMutexGuard aGuard(m_aMutex);
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "SelectionClear for selection %s\n",
                     OUStringToOString( getString( rEvent.xselectionclear.selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                     );
#endif
            SelectionAdaptor* pAdaptor = getAdaptor( rEvent.xselectionclear.selection );
            std::unordered_map< Atom, Selection* >::iterator it( m_aSelections.find( rEvent.xselectionclear.selection ) );
            if( it != m_aSelections.end() )
                it->second->m_bOwner = false;
            aGuard.clear();
            if ( pAdaptor )
                pAdaptor->clearTransferable();
        }
        break;

        case SelectionRequest:
            bHandled = handleSelectionRequest( rEvent.xselectionrequest );
            break;
        case PropertyNotify:
            if( rEvent.xproperty.window == m_aWindow ||
                rEvent.xproperty.window == m_aCurrentDropWindow
                )
                bHandled = handleReceivePropertyNotify( rEvent.xproperty );
            else
                bHandled = handleSendPropertyNotify( rEvent.xproperty );
            break;
        case SelectionNotify:
            bHandled = handleSelectionNotify( rEvent.xselection );
            break;
        case ClientMessage:
            // messages from drag target
            if( rEvent.xclient.message_type == m_nXdndStatus ||
                rEvent.xclient.message_type == m_nXdndFinished )
                bHandled = handleDragEvent( rEvent );
            // messages from drag source
            else if(
                    rEvent.xclient.message_type == m_nXdndEnter     ||
                    rEvent.xclient.message_type == m_nXdndLeave     ||
                    rEvent.xclient.message_type == m_nXdndPosition  ||
                    rEvent.xclient.message_type == m_nXdndDrop
                    )
                bHandled = handleDropEvent( rEvent.xclient );
            break;
        case EnterNotify:
        case LeaveNotify:
        case MotionNotify:
        case ButtonPress:
        case ButtonRelease:
        case KeyPress:
        case KeyRelease:
            bHandled = handleDragEvent( rEvent );
            break;
        default:
            ;
    }
    return bHandled;
}

void SelectionManager::dispatchEvent( int millisec )
{
    // acquire the mutex to prevent other threads
    // from using the same X connection
    osl::ResettableMutexGuard aGuard(m_aMutex);

    if( !XPending( m_pDisplay ))
    {
        int nfds = 1;
        // wait for any events if none are already queued
        pollfd aPollFD[2];
        aPollFD[0].fd      = XConnectionNumber( m_pDisplay );
        aPollFD[0].events  = POLLIN;
        aPollFD[0].revents = 0;

        // on infinite timeout we need endthreadpipe monitoring too
        if (millisec < 0)
        {
            aPollFD[1].fd      = m_EndThreadPipe[0];
            aPollFD[1].events  = POLLIN | POLLERR;
            aPollFD[1].revents = 0;
            nfds = 2;
        }

        // release mutex for the time of waiting for possible data
        aGuard.clear();
        if( poll( aPollFD, nfds, millisec ) <= 0 )
            return;
        aGuard.reset();
    }
    while( XPending( m_pDisplay ))
    {
        XEvent event;
        XNextEvent( m_pDisplay, &event );
        aGuard.clear();
        handleXEvent( event );
        aGuard.reset();
    }
}

void SelectionManager::run( void* pThis )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "SelectionManager::run\n" );
#endif
    osl::Thread::setName("SelectionManager");
    // dispatch until the cows come home

    SelectionManager* This = static_cast<SelectionManager*>(pThis);

    timeval aLast;
    gettimeofday( &aLast, nullptr );

    css::uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    This->m_xDesktop.set( Desktop::create(xContext) );
    This->m_xDesktop->addTerminateListener(This);

    // if end thread pipe properly initialized, allow infinite wait in poll
    // otherwise, fallback on 1 sec timeout
    const int timeout = (This->m_EndThreadPipe[0] != This->m_EndThreadPipe[1]) ? -1 : 1000;

    while( osl_scheduleThread(This->m_aThread) )
    {
        This->dispatchEvent( timeout );

        timeval aNow;
        gettimeofday( &aNow, nullptr );

        if( (aNow.tv_sec - aLast.tv_sec) > 0 )
        {
            osl::ClearableMutexGuard aGuard(This->m_aMutex);
            std::vector< std::pair< SelectionAdaptor*, css::uno::Reference< XInterface > > > aChangeVector;

            for (auto const& selection : This->m_aSelections)
            {
                if( selection.first != This->m_nXdndSelection && ! selection.second->m_bOwner )
                {
                    ::Window aOwner = XGetSelectionOwner( This->m_pDisplay, selection.first );
                    if( aOwner != selection.second->m_aLastOwner )
                    {
                        selection.second->m_aLastOwner = aOwner;
                        std::pair< SelectionAdaptor*, css::uno::Reference< XInterface > >
                            aKeep( selection.second->m_pAdaptor, selection.second->m_pAdaptor->getReference() );
                        aChangeVector.push_back( aKeep );
                    }
                }
            }
            aGuard.clear();
            for (auto const& change : aChangeVector)
            {
                change.first->fireContentsChanged();
            }
            aLast = aNow;
        }
    }
    // close write end on exit so write() fails and other thread does not block
    // forever
    close(This->m_EndThreadPipe[1]);
    close(This->m_EndThreadPipe[0]);
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "SelectionManager::run end\n" );
#endif
}

void SelectionManager::shutdown() throw()
{
    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SelectionManager got app termination event\n" );
    #endif

    osl::ResettableMutexGuard aGuard(m_aMutex);

    if( m_bShutDown )
        return;
    m_bShutDown = true;

    if ( m_xDesktop.is() )
        m_xDesktop->removeTerminateListener(this);

    if( m_xDisplayConnection.is() )
        m_xDisplayConnection->removeEventHandler(Any(), this);

    // stop dispatching
    if( m_aThread )
    {
        osl_terminateThread( m_aThread );
        /*
         * Allow thread to finish before app exits to avoid pulling the carpet
         * out from under it if pasting is occurring during shutdown
         *
         * a) allow it to have the Mutex and
         * b) reschedule to allow it to complete callbacks to any
         * Application::GetSolarMutex protected regions, etc. e.g.
         * TransferableHelper::getTransferDataFlavors (via
         * SelectionManager::handleSelectionRequest) which it might
         * currently be trying to enter.
         *
         * Otherwise the thread may be left still waiting on a GlobalMutex
         * when that gets destroyed, letting the thread blow up and die
         * when enters the section in a now dead OOo instance.
         */
        aGuard.clear();
        while (osl_isThreadRunning(m_aThread))
        {
            {   // drop mutex before write - otherwise may deadlock
                SolarMutexGuard guard2;
                Application::Reschedule();
            }
            // trigger poll()'s wait end by writing a dummy value
            char dummy=0;
            dummy = write(m_EndThreadPipe[1], &dummy, 1);
        }
        osl_joinWithThread( m_aThread );
        osl_destroyThread( m_aThread );
        m_aThread = nullptr;
        aGuard.reset();
    }
    m_xDesktop.clear();
    m_xDisplayConnection.clear();
    m_xDropTransferable.clear();
}

sal_Bool SelectionManager::handleEvent(const Any& event)
{
    Sequence< sal_Int8 > aSeq;
    if( event >>= aSeq )
    {
        XEvent* pEvent = reinterpret_cast<XEvent*>(aSeq.getArray());
        Time nTimestamp = CurrentTime;
        if( pEvent->type == ButtonPress || pEvent->type == ButtonRelease )
            nTimestamp = pEvent->xbutton.time;
        else if( pEvent->type == KeyPress || pEvent->type == KeyRelease )
            nTimestamp = pEvent->xkey.time;
        else if( pEvent->type == MotionNotify )
            nTimestamp = pEvent->xmotion.time;
        else if( pEvent->type == PropertyNotify )
            nTimestamp = pEvent->xproperty.time;

        if( nTimestamp != CurrentTime )
        {
            osl::MutexGuard aGuard(m_aMutex);

            m_nSelectionTimestamp = nTimestamp;
        }

        return handleXEvent( *pEvent );
    }
    else
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "SelectionManager got downing event\n" );
        #endif
        shutdown();
    }
    return true;
}

void SAL_CALL SelectionManager::disposing( const css::lang::EventObject& rEvt )
{
    if (rEvt.Source == m_xDesktop || rEvt.Source == m_xDisplayConnection)
        shutdown();
}

void SAL_CALL SelectionManager::queryTermination( const css::lang::EventObject& )
{
}

/*
 * To be safe, shutdown needs to be called before the ~SfxApplication is called, waiting until
 * the downing event can be too late if paste are requested during shutdown and ~SfxApplication
 * has been called before vcl is shutdown
 */
void SAL_CALL SelectionManager::notifyTermination( const css::lang::EventObject& rEvent )
{
    disposing(rEvent);
}

void SelectionManager::registerHandler( Atom selection, SelectionAdaptor& rAdaptor )
{
    osl::MutexGuard aGuard(m_aMutex);

    Selection* pNewSelection    = new Selection();
    pNewSelection->m_pAdaptor   = &rAdaptor;
    m_aSelections[ selection ]  = pNewSelection;
}

void SelectionManager::deregisterHandler( Atom selection )
{
    osl::MutexGuard aGuard(m_aMutex);

    std::unordered_map< Atom, Selection* >::iterator it =
          m_aSelections.find( selection );
    if( it != m_aSelections.end() )
    {
        delete it->second->m_pPixmap;
        delete it->second;
        m_aSelections.erase( it );
    }
}

static bool bWasError = false;

extern "C"
{
    static int local_xerror_handler(Display* , XErrorEvent*)
    {
        bWasError = true;
        return 0;
    }
    typedef int(*xerror_hdl_t)(Display*,XErrorEvent*);
}

void SelectionManager::registerDropTarget( ::Window aWindow, DropTarget* pTarget )
{
    osl::MutexGuard aGuard(m_aMutex);

    // sanity check
    std::unordered_map< ::Window, DropTargetEntry >::const_iterator it =
          m_aDropTargets.find( aWindow );
    if( it != m_aDropTargets.end() )
        OSL_FAIL( "attempt to register window as drop target twice" );
    else if( aWindow && m_pDisplay )
    {
        DropTargetEntry aEntry( pTarget );
        bWasError=false;
        /* #i100000# ugly workaround: gtk sets its own XErrorHandler which is not suitable for us
           unfortunately XErrorHandler is not per display, so this is just and ugly hack
           Need to remove separate display and integrate clipboard/dnd into vcl's unx code ASAP
        */
        xerror_hdl_t pOldHandler = XSetErrorHandler( local_xerror_handler );
        XSelectInput( m_pDisplay, aWindow, PropertyChangeMask );
        if( ! bWasError )
        {
            // set XdndAware
            XChangeProperty( m_pDisplay, aWindow, m_nXdndAware, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char const *>(&nXdndProtocolRevision), 1 );
            if( ! bWasError )
            {
                // get root window of window (in 99.999% of all cases this will be
                // DefaultRootWindow( m_pDisplay )
                int x, y;
                unsigned int w, h, bw, d;
                XGetGeometry( m_pDisplay, aWindow, &aEntry.m_aRootWindow,
                              &x, &y, &w, &h, &bw, &d );
            }
        }
        XSetErrorHandler( pOldHandler );
        if(bWasError)
            return;
        m_aDropTargets[ aWindow ] = aEntry;
    }
    else
        OSL_FAIL( "attempt to register None as drop target" );
}

void SelectionManager::deregisterDropTarget( ::Window aWindow )
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    m_aDropTargets.erase( aWindow );
    if( aWindow != m_aDragSourceWindow || !m_aDragRunning.check() )
        return;

    // abort drag
    std::unordered_map< ::Window, DropTargetEntry >::const_iterator it =
        m_aDropTargets.find( m_aDropWindow );
    if( it != m_aDropTargets.end() )
    {
        DropTargetEvent dte;
        dte.Source = static_cast< OWeakObject* >( it->second.m_pTarget );
        aGuard.clear();
        it->second.m_pTarget->dragExit( dte );
    }
    else if( m_aDropProxy != None && m_nCurrentProtocolVersion >= 0 )
    {
        // send XdndLeave
        XEvent aEvent;
        aEvent.type = ClientMessage;
        aEvent.xclient.display      = m_pDisplay;
        aEvent.xclient.format       = 32;
        aEvent.xclient.message_type = m_nXdndLeave;
        aEvent.xclient.window       = m_aDropWindow;
        aEvent.xclient.data.l[0]    = m_aWindow;
        memset( aEvent.xclient.data.l+1, 0, sizeof(long)*4);
        m_aDropWindow = m_aDropProxy = None;
        XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
    }
    // notify the listener
    DragSourceDropEvent dsde;
    dsde.Source             = static_cast< OWeakObject* >(this);
    dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, *this );
    dsde.DragSource         = static_cast< XDragSource* >(this);
    dsde.DropAction         = DNDConstants::ACTION_NONE;
    dsde.DropSuccess        = false;
    css::uno::Reference< XDragSourceListener > xListener( m_xDragSourceListener );
    m_xDragSourceListener.clear();
    aGuard.clear();
    xListener->dragDropEnd( dsde );

}

/*
 *  SelectionAdaptor
 */

css::uno::Reference< XTransferable > SelectionManager::getTransferable() throw()
{
    return m_xDragSourceTransferable;
}

void SelectionManager::clearTransferable() throw()
{
    m_xDragSourceTransferable.clear();
}

void SelectionManager::fireContentsChanged() throw()
{
}

css::uno::Reference< XInterface > SelectionManager::getReference() throw()
{
    return css::uno::Reference< XInterface >( static_cast<OWeakObject*>(this) );
}

/*
 *  SelectionManagerHolder
 */

SelectionManagerHolder::SelectionManagerHolder() :
        ::cppu::WeakComponentImplHelper<
    XDragSource,
    XInitialization,
    XServiceInfo > (m_aMutex)
{
}

SelectionManagerHolder::~SelectionManagerHolder()
{
}

void SelectionManagerHolder::initialize( const Sequence< Any >& arguments )
{
    OUString aDisplayName;

    if( arguments.getLength() > 0 )
    {
        css::uno::Reference< XDisplayConnection > xConn;
        arguments.getConstArray()[0] >>= xConn;
        if( xConn.is() )
        {
            Any aIdentifier;
            aIdentifier >>= aDisplayName;
        }
    }

    SelectionManager& rManager = SelectionManager::get( aDisplayName );
    rManager.initialize( arguments );
    m_xRealDragSource = static_cast< XDragSource* >(&rManager);
}

/*
 *  XDragSource
 */

sal_Bool SelectionManagerHolder::isDragImageSupported()
{
    return m_xRealDragSource.is() && m_xRealDragSource->isDragImageSupported();
}

sal_Int32 SelectionManagerHolder::getDefaultCursor( sal_Int8 dragAction )
{
    return m_xRealDragSource.is() ? m_xRealDragSource->getDefaultCursor( dragAction ) : 0;
}

void SelectionManagerHolder::startDrag(
                                       const css::datatransfer::dnd::DragGestureEvent& trigger,
                                       sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
                                       const css::uno::Reference< css::datatransfer::XTransferable >& transferable,
                                       const css::uno::Reference< css::datatransfer::dnd::XDragSourceListener >& listener
                                       )
{
    if( m_xRealDragSource.is() )
        m_xRealDragSource->startDrag( trigger, sourceActions, cursor, image, transferable, listener );
}

/*
 *  XServiceInfo
 */

OUString SelectionManagerHolder::getImplementationName()
{
    return OUString(XDND_IMPLEMENTATION_NAME);
}

sal_Bool SelectionManagerHolder::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SelectionManagerHolder::getSupportedServiceNames()
{
    return Xdnd_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
