/*************************************************************************
 *
 *  $RCSfile: X11_selection.cxx,v $
 *
 *  $Revision: 1.64 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:33:25 $
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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#if defined(LINUX) || defined(NETBSD) || defined (FREEBSD)
#include <sys/poll.h>
#else
#include <poll.h>
#endif
#if !(defined FREEBSD || defined NETBSD || defined MACOSX)
#include <alloca.h>
#endif

#include <X11_selection.hxx>
#include <X11_clipboard.hxx>
#include <X11_transferable.hxx>
#include <X11_dndcontext.hxx>
#include <bmp.hxx>

// pointer bitmaps
#include <copydata_curs.h>
#include <copydata_mask.h>
#include <movedata_curs.h>
#include <movedata_mask.h>
#include <linkdata_curs.h>
#include <linkdata_mask.h>
#include <nodrop_curs.h>
#include <nodrop_mask.h>

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#define INCR_MIN_SIZE   1024
#define INCR_TIMEOUT        5

#define DRAG_EVENT_MASK ButtonPressMask         |\
                              ButtonReleaseMask     |\
                              PointerMotionMask     |\
                              EnterWindowMask           |\
                              LeaveWindowMask

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;

using namespace x11;

static const int nXdndProtocolRevision = 5;

// mapping between mime types (or what the office thinks of mime types)
// and X convention types
struct NativeTypeEntry
{
    Atom            nAtom;
    const char*     pType;              // Mime encoding on our side
    const char*     pNativeType;        // string corresponding to nAtom for the case of nAtom being uninitialized
    int             nFormat;            // the corresponding format
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
    if( aMimeType.getToken( 0, ';', nIndex ).equalsAsciiL( "text/plain" , 10 ) )
    {
        if( aMimeType.getLength() == 10 ) // only "text/plain"
            aEncoding = RTL_TEXTENCODING_ISO_8859_1;
        else
        {
            while( nIndex != -1 )
            {
                OUString aToken = aMimeType.getToken( 0, ';', nIndex );
                sal_Int32 nPos = 0;
                if( aToken.getToken( 0, '=', nPos ).equalsAsciiL( "charset", 7 ) )
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

// ------------------------------------------------------------------------

::std::hash_map< OUString, SelectionManager*, OUStringHash >& SelectionManager::getInstances()
{
    static ::std::hash_map< OUString, SelectionManager*, OUStringHash > aInstances;
    return aInstances;
}

// ------------------------------------------------------------------------

SelectionManager::IncrementalTransfer::IncrementalTransfer(
                                                           const Sequence< sal_Int8 >& rData,
                                                           Window aRequestor,
                                                           Atom aProperty,
                                                           Atom aTarget,
                                                           int nFormat
                                                           ) :
        m_aData( rData ),
        m_aRequestor( aRequestor ),
        m_aProperty( aProperty ),
        m_aTarget( aTarget ),
        m_nFormat( nFormat ),
        m_nBufferPos( 0 ),
        m_nTransferStartTime( time( NULL ) )
{
}

// ------------------------------------------------------------------------

SelectionManager::SelectionManager() :
        m_pDisplay( NULL ),
        m_aWindow( None ),
        m_aDropWindow( None ),
        m_aDropProxy( None ),
        m_aThread( NULL ),
        m_aDragExecuteThread( NULL ),
        m_nCurrentProtocolVersion( nXdndProtocolRevision ),
        m_nNoPosX( 0 ),
        m_nNoPosY( 0 ),
        m_nNoPosWidth( 0 ),
        m_nNoPosHeight( 0 ),
        m_bDropSent( false ),
        m_bWaitingForPrimaryConversion( false ),
        m_bDropSuccess( false ),
        m_aMoveCursor( None ),
        m_aCopyCursor( None ),
        m_aLinkCursor( None ),
        m_aNoneCursor( None ),
        m_aCurrentCursor( None ),
        m_bLastDropAccepted( false ),
        m_aDragSourceWindow( None ),
        m_bDropWaitingForCompletion( false )
{
    m_aDropEnterEvent.data.l[0] = None;
    m_bDropEnterSent            = true;
    m_aDragRunning.reset();
}

Cursor SelectionManager::createCursor( const char* pPointerData, const char* pMaskData, int width, int height, int hotX, int hotY )
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
                               pPointerData,
                               width,
                               height );
    aMask
        = XCreateBitmapFromData( m_pDisplay,
                                 m_aWindow,
                                 pMaskData,
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

void SelectionManager::initialize( const Sequence< Any >& arguments ) throw (::com::sun::star::uno::Exception)
{
    MutexGuard aGuard(m_aMutex);

    if( ! m_xDisplayConnection.is() )
    {
        /*
         *  first argument must be a ::com::sun::star::awt::XDisplayConnection
         *  from this we will get the XEvents of the vcl event loop by
         *  registering us as XEventHandler on it.
         *
         *  implementor's note:
         *  <RANT>
         *  Because XDND implementations send their ClientMessage events with
         *  XSendEvent and event mask 0 (which is perfectly sensible)
         *  these events get only sent to the connection that created
         *  the window in question: the vcl display.
         *
         *  So this whole service should be implemented in vcl where
         *  it belongs. But as usual the dogmatics win and the dogma says:
         *  "vcl bad, service good". So instead of making vcl a service
         *  providing library we make a new one that is "independent" of vcl,
         *  because it is not linked against it and dispatch every event of
         *  the vcl event loop to here via uno mechanisms (that is copying it,
         *  locking and unlocking mutexes). Well, the user has fast computers
         *  these days, so why do we need performance where more memory and
         *  a faster CPU will do the job ?
         *  And that is why this runs under the title:
         *      How NOT to implement a service
         *      or the victory of dogma over common sense.
         *  </RANT>
         */
        if( arguments.getLength() > 0 )
            arguments.getConstArray()[0] >>= m_xDisplayConnection;
        if( ! m_xDisplayConnection.is() )
        {
#if 0
            // for the time being try to live without XDisplayConnection
            // for the sake of clipboard service
            // clipboard service should be initialized with a XDisplayConnection
            // in the future
            Exception aExc;
            aExc.Message = OUString::createFromAscii( "initialize me with a valid XDisplayConnection" );
            aExc.Context = static_cast< OWeakObject* >(this);
            throw aExc;
#endif
        }
        else
            m_xDisplayConnection->addEventHandler( Any(), this, ~0 );
    }

    if( !m_xBitmapConverter.is() )
    {
        if( arguments.getLength() > 2 )
            arguments.getConstArray()[2] >>= m_xBitmapConverter;
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

        m_pDisplay = XOpenDisplay( aDisplayName.getLength() ? aDisplayName.getStr() : NULL );

        if( m_pDisplay )
        {
#ifdef SYNCHRONIZE
            XSynchronize( m_pDisplay, True );
#endif
            // clipboard selection
            m_nCLIPBOARDAtom    = getAtom( OUString::createFromAscii( "CLIPBOARD" ) );

            // special targets
            m_nTARGETSAtom      = getAtom( OUString::createFromAscii( "TARGETS" ) );
            m_nTEXTAtom         = getAtom( OUString::createFromAscii( "TEXT" ) );
            m_nINCRAtom         = getAtom( OUString::createFromAscii( "INCR" ) );
            m_nCOMPOUNDAtom     = getAtom( OUString::createFromAscii( "COMPOUND_TEXT" ) );
            m_nMULTIPLEAtom     = getAtom( OUString::createFromAscii( "MULTIPLE" ) );
            m_nUTF16Atom        = getAtom( OUString::createFromAscii( "ISO10646-1" ) );
//            m_nUTF16Atom      = getAtom( OUString::createFromAscii( "text/plain;charset=ISO-10646-UCS-2" ) );

            // Atoms for Xdnd protocol
            m_nXdndAware        = getAtom( OUString::createFromAscii( "XdndAware" ) );
            m_nXdndEnter        = getAtom( OUString::createFromAscii( "XdndEnter" ) );
            m_nXdndLeave        = getAtom( OUString::createFromAscii( "XdndLeave" ) );
            m_nXdndPosition     = getAtom( OUString::createFromAscii( "XdndPosition" ) );
            m_nXdndStatus       = getAtom( OUString::createFromAscii( "XdndStatus" ) );
            m_nXdndDrop         = getAtom( OUString::createFromAscii( "XdndDrop" ) );
            m_nXdndFinished     = getAtom( OUString::createFromAscii( "XdndFinished" ) );
            m_nXdndSelection    = getAtom( OUString::createFromAscii( "XdndSelection" ) );
            m_nXdndTypeList     = getAtom( OUString::createFromAscii( "XdndTypeList" ) );
            m_nXdndProxy        = getAtom( OUString::createFromAscii( "XdndProxy" ) );
            m_nXdndActionCopy   = getAtom( OUString::createFromAscii( "XdndActionCopy" ) );
            m_nXdndActionMove   = getAtom( OUString::createFromAscii( "XdndActionMove" ) );
            m_nXdndActionLink   = getAtom( OUString::createFromAscii( "XdndActionLink" ) );
            m_nXdndActionAsk    = getAtom( OUString::createFromAscii( "XdndActionAsk" ) );
            m_nXdndActionPrivate= getAtom( OUString::createFromAscii( "XdndActionPrivate" ) );

            // initialize map with member none
            m_aAtomToString[ 0 ]= OUString::createFromAscii( "None" );
            m_aAtomToString[ XA_PRIMARY ] = OUString::createFromAscii( "PRIMARY" );

            // create a (invisible) message window
            m_aWindow = XCreateSimpleWindow( m_pDisplay, DefaultRootWindow( m_pDisplay ),
                                             10, 10, 10, 10, 0, 0, 1 );

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
                m_xDropTransferable = new X11Transferable( *this, static_cast< OWeakObject* >(this), m_nXdndSelection );
                registerHandler( m_nXdndSelection, *this );

                m_aThread = osl_createSuspendedThread( run, this );
                if( m_aThread )
                    osl_resumeThread( m_aThread );
#if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "SelectionManager::initialize: creation of dispatch thread failed !\n" );
#endif
            }
        }
    }
}

// ------------------------------------------------------------------------

SelectionManager::~SelectionManager()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SelectionManager::~SelectionManager (%s)\n", m_pDisplay ? DisplayString(m_pDisplay) : "no display" );
#endif
    {
        MutexGuard aGuard( *Mutex::getGlobalMutex() );

        ::std::hash_map< OUString, SelectionManager*, OUStringHash >::iterator it;
        for( it = getInstances().begin(); it != getInstances().end(); ++it )
            if( it->second == this )
            {
                getInstances().erase( it );
                break;
            }
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
        m_aDragExecuteThread = NULL;
        // thread handle is freed in dragDoDispatch()
    }

    MutexGuard aGuard(m_aMutex);

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "shutting down SelectionManager\n" );
#endif

    if( m_pDisplay )
    {
        deregisterHandler( m_nXdndSelection );
        // destroy message window
        if( m_aWindow )
            XDestroyWindow( m_pDisplay, m_aWindow );

        // paranoia setting, the drag thread should have
        // done that already
        XUngrabPointer( m_pDisplay, CurrentTime );
        XUngrabKeyboard( m_pDisplay, CurrentTime );

        XCloseDisplay( m_pDisplay );
    }
}

// ------------------------------------------------------------------------

SelectionAdaptor* SelectionManager::getAdaptor( Atom selection )
{
    ::std::hash_map< Atom, Selection* >::iterator it =
          m_aSelections.find( selection );
    return it != m_aSelections.end() ? it->second->m_pAdaptor : NULL;
}

// ------------------------------------------------------------------------

OUString SelectionManager::convertFromCompound( const char* pText, int nLen )
{
    MutexGuard aGuard( m_aMutex );
    OUString aRet;
    if( nLen < 0 )
        nLen = strlen( pText );

    char** pTextList = NULL;
    int nTexts = 0;

    XTextProperty aProp;
    aProp.value     = (unsigned char*)pText;
    aProp.encoding  = m_nCOMPOUNDAtom;
    aProp.format    = 8;
    aProp.nitems    = nLen;
    XmbTextPropertyToTextList( m_pDisplay,
                               &aProp,
                               &pTextList,
                               &nTexts );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    for( int i = 0; i < nTexts; i++ )
        aRet += OStringToOUString( pTextList[i], aEncoding );

    if( pTextList )
        XFreeStringList( pTextList );

    return aRet;
}

// ------------------------------------------------------------------------

OString SelectionManager::convertToCompound( const OUString& rText )
{
    MutexGuard aGuard( m_aMutex );
    XTextProperty aProp;

    OString aRet( rText.getStr(), rText.getLength(), osl_getThreadTextEncoding() );
    char* pT = const_cast<char*>(aRet.getStr());

    XmbTextListToTextProperty( m_pDisplay,
                               &pT,
                               1,
                               XCompoundTextStyle,
                               &aProp );
    if( aProp.value )
    {
        aRet = (char*)aProp.value;
        XFree( aProp.value );
#ifdef SOLARIS
        /*  #97070#
         *  for currently unknown reasons XmbTextListToTextProperty on Solaris returns
         *  no data in ISO8859-n encodings (at least for n = 1, 15)
         *  in these encodings the directly converted text does the
         *  trick, also.
         */
        if( ! aRet.getLength() && rText.getLength() )
            aRet = OUStringToOString( rText, osl_getThreadTextEncoding() );
#endif
    }
    else
        aRet = OString();

    return aRet;
}

// ------------------------------------------------------------------------

bool SelectionManager::convertData(
                                   const Reference< XTransferable >& xTransferable,
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
        if( aFlavor.MimeType.getToken( 0, ';', nIndex ).compareToAscii( "text/plain" ) == 0 )
        {
            if( aFlavor.MimeType.getToken( 0, ';', nIndex ).compareToAscii( "charset=utf-16" ) == 0 )
                aFlavor.DataType = getCppuType( (OUString *) 0 );
            else
                aFlavor.DataType = getCppuType( (Sequence< sal_Int8 >*)0 );
        }
        else
            aFlavor.DataType = getCppuType( (Sequence< sal_Int8 >*)0 );

        if( xTransferable->isDataFlavorSupported( aFlavor ) )
        {
            Any aValue( xTransferable->getTransferData( aFlavor ) );
            if( aValue.getValueTypeClass() == TypeClass_STRING )
            {
                OUString aString;
                aValue >>= aString;
                rData = Sequence< sal_Int8 >( (sal_Int8*)aString.getStr(), aString.getLength() * sizeof( sal_Unicode ) );
                bSuccess = true;
            }
            else if( aValue.getValueType() == getCppuType( (Sequence< sal_Int8 >*)0 ) )
            {
                aValue >>= rData;
                bSuccess = true;
            }
        }
        else if( aFlavor.MimeType.compareToAscii( "text/plain", 10 ) == 0 )
        {
            rtl_TextEncoding aEncoding = RTL_TEXTENCODING_DONTKNOW;
            bool bCompoundText = false;
            if( nType == m_nCOMPOUNDAtom )
                bCompoundText = true;
            else
                aEncoding = getTextPlainEncoding( aFlavor.MimeType );
            if( aEncoding != RTL_TEXTENCODING_DONTKNOW || bCompoundText )
            {
                aFlavor.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
                aFlavor.DataType = getCppuType( (OUString *) 0 );
                if( xTransferable->isDataFlavorSupported( aFlavor ) )
                {
                    Any aValue( xTransferable->getTransferData( aFlavor ) );
                    OUString aString;
                    aValue >>= aString;
                    OString aByteString( bCompoundText ? convertToCompound( aString ) : OUStringToOString( aString, aEncoding ) );
                    rData = Sequence< sal_Int8 >( (sal_Int8*)aByteString.getStr(), aByteString.getLength() * sizeof( sal_Char ) );
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

// ------------------------------------------------------------------------

SelectionManager& SelectionManager::get( const OUString& rDisplayName )
{
    MutexGuard aGuard( *Mutex::getGlobalMutex() );

    OUString aDisplayName( rDisplayName );
    if( ! aDisplayName.getLength() )
        aDisplayName = OStringToOUString( getenv( "DISPLAY" ), RTL_TEXTENCODING_ISO_8859_1 );
    SelectionManager* pInstance = NULL;

    ::std::hash_map< OUString, SelectionManager*, OUStringHash >::iterator it = getInstances().find( aDisplayName );
    if( it != getInstances().end() )
        pInstance = it->second;
    else pInstance = getInstances()[ aDisplayName ] = new SelectionManager();

    return *pInstance;
}

// ------------------------------------------------------------------------

const OUString& SelectionManager::getString( Atom aAtom )
{
    MutexGuard aGuard(m_aMutex);

    ::std::hash_map< Atom, OUString >::const_iterator it;
    if( ( it = m_aAtomToString.find( aAtom ) ) == m_aAtomToString.end() )
    {
        static OUString aEmpty;
        char* pAtom = XGetAtomName( m_pDisplay, aAtom );
        if( ! pAtom )
            return aEmpty;
        OUString aString( OStringToOUString( pAtom, RTL_TEXTENCODING_ISO_8859_1 ) );
        XFree( pAtom );
        m_aStringToAtom[ aString ] = aAtom;
        m_aAtomToString[ aAtom ] = aString;
    }
    return m_aAtomToString[ aAtom ];
}

// ------------------------------------------------------------------------

Atom SelectionManager::getAtom( const OUString& rString )
{
    MutexGuard aGuard(m_aMutex);

    ::std::hash_map< OUString, Atom, OUStringHash >::const_iterator it;
    if( ( it = m_aStringToAtom.find( rString ) ) == m_aStringToAtom.end() )
    {
        Atom aAtom = XInternAtom( m_pDisplay, OUStringToOString( rString, RTL_TEXTENCODING_ISO_8859_1 ), False );
        m_aStringToAtom[ rString ] = aAtom;
        m_aAtomToString[ aAtom ] = rString;
    }
    return m_aStringToAtom[ rString ];
}

// ------------------------------------------------------------------------

bool SelectionManager::requestOwnership( Atom selection )
{
    bool bSuccess = false;
    if( m_pDisplay && m_aWindow )
    {
        MutexGuard aGuard(m_aMutex);

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
            pSel->m_pPixmap = NULL;
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

// ------------------------------------------------------------------------

void SelectionManager::convertTypeToNative( const OUString& rType, Atom selection, int& rFormat, ::std::list< Atom >& rConversions, bool bPushFront )
{
    NativeTypeEntry* pTab = selection == m_nXdndSelection ? aXdndConversionTab : aNativeConversionTab;
    int nTabEntries = selection == m_nXdndSelection
        ? sizeof(aXdndConversionTab)/sizeof(aXdndConversionTab[0]) :
        sizeof(aNativeConversionTab)/sizeof(aNativeConversionTab[0]);

    OString aType( OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ) );
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

// ------------------------------------------------------------------------

void SelectionManager::getNativeTypeList( const Sequence< DataFlavor >& rTypes, std::list< Atom >& rOutTypeList, Atom targetselection )
{
    rOutTypeList.clear();

    int nFormat;
    int nFlavors = rTypes.getLength();
    const DataFlavor* pFlavors = rTypes.getConstArray();
    bool bHaveText = false;
    for( int i = 0; i < nFlavors; i++ )
    {
        if( pFlavors[i].MimeType.compareToAscii( "text/plain", 10 ) == 0)
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
        convertTypeToNative( OUString::createFromAscii( "text/plain;charset=utf-8" ), targetselection, nFormat, rOutTypeList, true );
    }
    if( targetselection != m_nXdndSelection )
        rOutTypeList.push_back( m_nMULTIPLEAtom );
}

// ------------------------------------------------------------------------

OUString SelectionManager::convertTypeFromNative( Atom nType, Atom selection, int& rFormat )
{
    NativeTypeEntry* pTab = selection == m_nXdndSelection ? aXdndConversionTab : aNativeConversionTab;
    int nTabEntries = selection == m_nXdndSelection
        ? sizeof(aXdndConversionTab)/sizeof(aXdndConversionTab[0]) :
        sizeof(aNativeConversionTab)/sizeof(aNativeConversionTab[0]);

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

// ------------------------------------------------------------------------

bool SelectionManager::getPasteData( Atom selection, Atom type, Sequence< sal_Int8 >& rData )
{
    ::std::hash_map< Atom, Selection* >::iterator it;
    bool bSuccess = false;

#if OSL_DEBUG_LEVEL > 1
    OUString aSelection( getString( selection ) );
    OUString aType( getString( type ) );
    fprintf( stderr, "getPasteData( %s, native: %s )\n",
             OUStringToOString( aSelection, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( aType, RTL_TEXTENCODING_ISO_8859_1 ).getStr()
             );
#endif

    {
        MutexGuard aGuard(m_aMutex);

        it = m_aSelections.find( selection );
        if( it == m_aSelections.end() )
            return false;

        Window aSelectionOwner = XGetSelectionOwner( m_pDisplay, selection );
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
    }

    // do a reschedule
    time_t nBegin = time( NULL );
    XEvent aEvent;
    do
    {
        {
            ClearableMutexGuard aGuard(m_aMutex);
            bool bHandle = false;

            if( XCheckTypedEvent( m_pDisplay,
                                  PropertyNotify,
                                  &aEvent
                                  ) )
                bHandle = true;
            else
            if( XCheckTypedEvent( m_pDisplay,
                                  SelectionClear,
                                  &aEvent
                                  ) )
                bHandle = true;
            else
            if( XCheckTypedEvent( m_pDisplay,
                                  SelectionRequest,
                                  &aEvent
                                  ) )
                bHandle = true;
            else
            if( XCheckTypedEvent( m_pDisplay,
                                  SelectionNotify,
                                  &aEvent
                                  ) )
                bHandle = true;
            else
            {
                TimeValue aTVal;
                aTVal.Seconds = 0;
                aTVal.Nanosec = 200000000;
                osl_waitThread( &aTVal );
            }
            if( bHandle )
            {
                aGuard.clear();
                handleXEvent( aEvent );
            }
        }
    } while( ! it->second->m_aDataArrived.check() && time(NULL)-nBegin < 3 );

#if OSL_DEBUG_LEVEL > 1
    if( time(NULL)-nBegin >= 2 )
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
        fprintf( stderr, "conversion unsuccessfull\n" );
#endif
    return bSuccess;
}

// ------------------------------------------------------------------------

bool SelectionManager::getPasteData( Atom selection, const ::rtl::OUString& rType, Sequence< sal_Int8 >& rData )
{
    int nFormat;
    bool bSuccess = false;

    ::std::hash_map< Atom, Selection* >::iterator it;
    {
        MutexGuard aGuard(m_aMutex);

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
    const Sequence< Atom >& rNativeTypes( it->second->m_aNativeTypes );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "getPasteData( \"%s\", \"%s\" )\n",
             OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    if( rType.equalsAsciiL( "text/plain;charset=utf-16", 25 ) )
    {
        // lets see if we have UTF16 else try to find something convertible
        if( it->second->m_aTypes.getLength() && ! it->second->m_bHaveUTF16 )
        {
            Sequence< sal_Int8 > aData;
            if( it->second->m_bHaveCompound &&
                getPasteData( selection,
                              m_nCOMPOUNDAtom,
                              aData )
                )
            {
                OUString aRet( convertFromCompound( (const char*)aData.getConstArray(), aData.getLength() ) );
                rData = Sequence< sal_Int8 >( (sal_Int8*)aRet.getStr(), (aRet.getLength()+1)*sizeof( sal_Unicode ) );
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
                                      rNativeTypes.getConstArray()[i],
                                      aData )
                        )
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "using \"%s\" instead of \"%s\"\n",
                                 OUStringToOString( rTypes.getConstArray()[i].MimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                                 OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                                 );
#endif
                        OString aConvert( (sal_Char*)aData.getConstArray(), aData.getLength() );
                        OUString aUTF( OStringToOUString( aConvert, aEncoding ) );
                        rData = Sequence< sal_Int8 >( (sal_Int8*)aUTF.getStr(), (aUTF.getLength()+1)*sizeof( sal_Unicode ) );
                        bSuccess = true;
                        break;
                    }
                }
            }
        }
    }
    else if( rType.equalsAsciiL( "image/bmp", 9 ) )
    {
        Pixmap aPixmap = None;
        Colormap aColormap = None;

        // prepare property for MULTIPLE request
        Sequence< sal_Int8 > aData;
        Atom pTypes[4] = { XA_PIXMAP, XA_PIXMAP,
                           XA_COLORMAP, XA_COLORMAP };
        {
            MutexGuard aGuard(m_aMutex);

            XChangeProperty( m_pDisplay,
                             m_aWindow,
                             selection,
                             XA_ATOM,
                             32,
                             PropModeReplace,
                             (unsigned char*)pTypes,
                             4 );
        }

        // try MULTIPLE request
        if( getPasteData( selection, m_nMULTIPLEAtom, aData ) )
        {
            Atom* pReturnedTypes = (Atom*)aData.getArray();
            if( pReturnedTypes[0] == XA_PIXMAP && pReturnedTypes[1] == XA_PIXMAP )
            {
                MutexGuard aGuard(m_aMutex);

                Atom type = None;
                int format = 0;
                unsigned long nItems = 0;
                unsigned long nBytes = 0;
                unsigned char* pReturn = NULL;
                XGetWindowProperty( m_pDisplay, m_aWindow, XA_PIXMAP, 0, 1, True, XA_PIXMAP, &type, &format, &nItems, &nBytes, &pReturn );
                if( pReturn )
                {
                    if( type == XA_PIXMAP )
                        aPixmap = *(Pixmap*)pReturn;
                    XFree( pReturn );
                    pReturn = NULL;
                    if( pReturnedTypes[2] == XA_COLORMAP && pReturnedTypes[3] == XA_COLORMAP )
                    {
                        XGetWindowProperty( m_pDisplay, m_aWindow, XA_COLORMAP, 0, 1, True, XA_COLORMAP, &type, &format, &nItems, &nBytes, &pReturn );
                        if( pReturn )
                        {
                            if( type == XA_COLORMAP )
                                aColormap = *(Colormap*)pReturn;
                            XFree( pReturn );
                        }
                    }
                }
#if OSL_DEBUG_LEVEL > 1
                else
                {
                    fprintf( stderr, "could not get PIXMAP property: type=%s, format=%d, items=%d, bytes=%d, ret=0x%p\n", OUStringToOString( getString( type ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(), format, nItems, nBytes, pReturn );
                }
#endif
            }
        }

        if( aPixmap == None )
        {
            // perhaps two normal requests will work
            if( getPasteData( selection, XA_PIXMAP, aData ) )
            {
                aPixmap = *(Pixmap*)aData.getArray();
                if( aColormap == None && getPasteData( selection, XA_COLORMAP, aData ) )
                    aColormap = *(Colormap*)aData.getArray();
            }
        }

        // convert data if possible
        if( aPixmap != None )
        {
            MutexGuard aGuard(m_aMutex);

            sal_Int32 nOutSize = 0;
            sal_uInt8* pBytes = X11_getBmpFromPixmap( m_pDisplay, aPixmap, aColormap, nOutSize );
            if( pBytes && nOutSize )
            {
                rData = Sequence< sal_Int8 >( nOutSize );
                memcpy( rData.getArray(), pBytes, nOutSize );
                X11_freeBmp( pBytes );
                bSuccess = true;
            }
        }
    }

    if( ! bSuccess )
    {
        ::std::list< Atom > aTypes;
        convertTypeToNative( rType, selection, nFormat, aTypes );
        ::std::list< Atom >::const_iterator type_it;
        Atom nSelectedType = None;
        const Atom* pNativeTypes = rNativeTypes.getConstArray();
        for( type_it = aTypes.begin(); type_it != aTypes.end() && nSelectedType == None; ++type_it )
        {
            for( int i = 0; i < rNativeTypes.getLength() && nSelectedType == None; i++ )
                if( pNativeTypes[i] == *type_it )
                    nSelectedType = *type_it;
        }
        if( nSelectedType != None )
            bSuccess = getPasteData( selection, nSelectedType, rData );
    }
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "getPasteData for selection %s and data type %s returns %s, returned sequence has length %d\n",
             OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( rType, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             bSuccess ? "true" : "false",
             rData.getLength()
             );
#endif
    return bSuccess;
}

// ------------------------------------------------------------------------

bool SelectionManager::getPasteDataTypes( Atom selection, Sequence< DataFlavor >& rTypes )
{
    ::std::hash_map< Atom, Selection* >::iterator it;
    {
        MutexGuard aGuard(m_aMutex);

        it = m_aSelections.find( selection );
        if( it != m_aSelections.end()                           &&
            it->second->m_aTypes.getLength()                    &&
            abs( it->second->m_nLastTimestamp - time( NULL ) ) < 2
            )
        {
            rTypes = it->second->m_aTypes;
            return true;
        }
    }

    bool bSuccess = false;
    bool bHaveUTF16 = false;
    bool bHaveCompound = false;
    bool bHaveText = false;
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
                const int atomcount = 256;
                // more than three types; look in property
                MutexGuard aGuard(m_aMutex);

                Atom nType;
                int nFormat;
                unsigned long nItems, nBytes;
                unsigned char* pBytes = NULL;

                XGetWindowProperty( m_pDisplay, m_aDropEnterEvent.data.l[0],
                                    m_nXdndTypeList, 0, atomcount, False,
                                    XA_ATOM,
                                    &nType, &nFormat, &nItems, &nBytes, &pBytes );
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "have %d data types in XdndTypeList\n", nItems );
#endif
                if( nItems == atomcount && nBytes > 0 )
                {
                    // wow ... more than 256 types !
                    aAtoms.realloc( sizeof( Atom )*atomcount+nBytes );
                    memcpy( aAtoms.getArray(), pBytes, sizeof( Atom )*atomcount );
                    XFree( pBytes );
                    pBytes = NULL;
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
                        ((Atom*)aAtoms.getArray())[n++] = m_aDropEnterEvent.data.l[2+i];
            }
        }
    }
    // get data of type TARGETS
    else if( ! getPasteData( selection, m_nTARGETSAtom, aAtoms ) )
        aAtoms = Sequence< sal_Int8 >();

    Sequence< Atom > aNativeTypes;
    if( aAtoms.getLength() )
    {
        int nAtoms = aAtoms.getLength() / 4;
        Atom* pAtoms = (Atom*)aAtoms.getArray();
        rTypes.realloc( nAtoms );
        aNativeTypes.realloc( nAtoms );
        DataFlavor* pFlavors = rTypes.getArray();
        Atom* pNativeTypes = aNativeTypes.getArray();
        while( nAtoms-- )
        {
#if OSL_DEBUG_LEVEL > 1
            if( *pAtoms && *pAtoms < 0x01000000 )
                fprintf( stderr, "native type: %s\n", OUStringToOString( getString( *pAtoms ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
            if( *pAtoms == m_nCOMPOUNDAtom )
                bHaveText = bHaveCompound = true;
            else if( *pAtoms && *pAtoms < 0x01000000 )
            {
                int nFormat;
                pFlavors->MimeType = convertTypeFromNative( *pAtoms, selection, nFormat );
                pFlavors->DataType = getCppuType( (Sequence< sal_Int8 >*)0 );
                sal_Int32 nIndex = 0;
                if( pFlavors->MimeType.getToken( 0, ';', nIndex ).equalsAsciiL( "text/plain", 10 ) )
                {
                    // omit text/plain;charset=unicode since it is not well defined
                    if( pFlavors->MimeType.getToken( 0, ';', nIndex ).compareToAscii( "charset=unicode" ) == 0 )
                    {
                        pAtoms++;
                        continue;
                    }
                    bHaveText = true;
                    if( pFlavors->MimeType.getToken( 0, ';', nIndex ).compareToAscii( "charset=utf-16" ) == 0 )
                    {
                        bHaveUTF16 = true;
                        pFlavors->DataType = getCppuType( (OUString*)0 );
                    }
                }
                pFlavors++;
                *pNativeTypes++ = *pAtoms;
            }
            pAtoms++;
        }
        if( (pFlavors - rTypes.getArray()) < rTypes.getLength() )
            rTypes.realloc(pFlavors - rTypes.getArray());
        bSuccess = rTypes.getLength() ? true : false;
        if( bHaveText && ! bHaveUTF16 )
        {
            int nNewFlavors = rTypes.getLength()+1;
            Sequence< DataFlavor > aTemp( nNewFlavors );
            for( int i = 0; i < nNewFlavors-1; i++ )
                aTemp.getArray()[i+1] = rTypes.getConstArray()[i];
            aTemp.getArray()[0].MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
            aTemp.getArray()[0].DataType = getCppuType( (OUString*)0 );
            rTypes = aTemp;

            Sequence< Atom > aNativeTemp( nNewFlavors );
            memcpy( aNativeTemp.getArray()+1, aNativeTypes.getConstArray(), sizeof(Atom)*(nNewFlavors-1) );
            aNativeTemp.getArray()[0] = None;
            aNativeTypes = aNativeTemp;
        }
    }

    {
        MutexGuard aGuard(m_aMutex);

        it = m_aSelections.find( selection );
        if( it != m_aSelections.end() )
        {
            if( bSuccess )
            {
                it->second->m_aTypes            = rTypes;
                it->second->m_aNativeTypes      = aNativeTypes;
                it->second->m_nLastTimestamp    = time( NULL );
                it->second->m_bHaveUTF16        = bHaveUTF16;
                it->second->m_bHaveCompound     = bHaveCompound;
            }
            else
            {
                it->second->m_aTypes            = Sequence< DataFlavor >();
                it->second->m_aNativeTypes      = Sequence< Atom >();
                it->second->m_nLastTimestamp    = 0;
                it->second->m_bHaveUTF16        = false;
                it->second->m_bHaveCompound     = false;
            }
        }
    }

#if OSL_DEBUG_LEVEL > 1
//    if( selection != m_nCLIPBOARDAtom )
    {
        fprintf( stderr, "SelectionManager::getPasteDataTypes( %s ) = %s\n", OUStringToOString( getString( selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(), bSuccess ? "true" : "false" );
        for( int i = 0; i < rTypes.getLength(); i++ )
            fprintf( stderr, "type: %s\n", OUStringToOString( rTypes.getConstArray()[i].MimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
    }
#endif

    return bSuccess;
}

// ------------------------------------------------------------------------

PixmapHolder* SelectionManager::getPixmapHolder( Atom selection )
{
    std::hash_map< Atom, Selection* >::const_iterator it = m_aSelections.find( selection );
    if( it == m_aSelections.end() )
        return NULL;
    if( ! it->second->m_pPixmap )
        it->second->m_pPixmap = new PixmapHolder( m_pDisplay );
    return it->second->m_pPixmap;
}

bool SelectionManager::sendData( SelectionAdaptor* pAdaptor,
                                 Window requestor,
                                 Atom target,
                                 Atom property,
                                 Atom selection )
{
    ResettableMutexGuard aGuard( m_aMutex );

    // handle targets related to image/bmp
    if( target == XA_COLORMAP || target == XA_PIXMAP || target == XA_BITMAP || target == XA_VISUALID )
    {
        PixmapHolder* pPixmap = getPixmapHolder( selection );
        if( ! pPixmap ) return false;
        XID nValue = None;

        // handle colormap request
        if( target == XA_COLORMAP )
            nValue = (XID)pPixmap->getColormap();
        else if( target == XA_VISUALID )
            nValue = (XID)pPixmap->getVisualID();
        else if( target == XA_PIXMAP || target == XA_BITMAP )
        {
            nValue = (XID)pPixmap->getPixmap();
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
                    // conversion succeeded, so aData contains image/bmp now
                    if( pPixmap->needsConversion( (const sal_uInt8*)aData.getConstArray() )
                        && m_xBitmapConverter.is() )
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "trying bitmap conversion\n" );
#endif
                        Reference<XBitmap> xBM( new BmpTransporter( aData ) );
                        Sequence<Any> aArgs(2), aOutArgs;
                        Sequence<sal_Int16> aOutIndex;
                        aArgs.getArray()[0] = makeAny( xBM );
                        aArgs.getArray()[1] = makeAny( (sal_uInt16)pPixmap->getDepth() );
                        try
                        {
                            aGuard.clear();
                            Any aResult =
                                m_xBitmapConverter->invoke( OUString::createFromAscii( "convert-bitmap-depth" ),
                                                            aArgs, aOutIndex, aOutArgs );
                            aGuard.reset();
                            if( aResult >>= xBM )
                                aData = xBM->getDIB();
                        }
                        catch(...)
                        {
#if OSL_DEBUG_LEVEL > 1
                            fprintf( stderr, "exception in bitmap converter\n" );
#endif
                        }
                    }
                    nValue = (XID)pPixmap->setBitmapData( (const sal_uInt8*)aData.getConstArray() );
                }
                if( nValue == None )
                    return false;
            }
            if( target == XA_BITMAP )
                nValue = (XID)pPixmap->getBitmap();
        }

        XChangeProperty( m_pDisplay,
                         requestor,
                         property,
                         target,
                         32,
                         PropModeReplace,
                         (const unsigned char*)&nValue,
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
        if( aData.getLength() > INCR_MIN_SIZE )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "using INCR protocol\n" );
#endif
            // use incr protocol
            int nBufferPos = 0;
            int nMinSize = INCR_MIN_SIZE;
            XChangeProperty( m_pDisplay, requestor, property,
                             m_nINCRAtom, 32,  PropModeReplace, (unsigned char*)&nMinSize, 1 );
            XSelectInput( m_pDisplay, requestor, PropertyChangeMask );
            IncrementalTransfer aTransfer( aData,
                                           requestor,
                                           property,
                                           target,
                                           nFormat
                                           );
            m_aIncrementals[ requestor ].push_back( aTransfer );
        }
        else
            XChangeProperty( m_pDisplay,
                             requestor,
                             property,
                             target,
                             nFormat,
                             PropModeReplace,
                             (const unsigned char*)aData.getConstArray(),
                             aData.getLength()/(nFormat/8) );
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "convertData failed for type: %s \n",
                 OUStringToOString( convertTypeFromNative( target, selection, nFormat ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    return bConverted;
}

// ------------------------------------------------------------------------

void SelectionManager::handleSelectionRequest( XSelectionRequestEvent& rRequest )
{
    ResettableMutexGuard aGuard( m_aMutex );
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
        Reference< XTransferable > xTrans( pAdaptor->getTransferable() );
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
                Atom* pTypes = (Atom*)alloca( nTypes * sizeof( Atom ) );
                std::list< Atom >::const_iterator it;
                for( i = 0, it = aConversions.begin(); i < nTypes; i++, ++it )
                    pTypes[i] = *it;
                XChangeProperty( m_pDisplay, rRequest.requestor, rRequest.property,
                                 XA_ATOM, 32, PropModeReplace, (const unsigned char*)pTypes, nTypes );
                aNotify.xselection.property = rRequest.property;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "sending type list:\n" );
                for( int k = 0; k < nTypes; k++ )
                    fprintf( stderr, "   %s\n", pTypes[k] ? XGetAtomName( m_pDisplay, pTypes[k] ) : "<None>" );
#endif
            }
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
                unsigned char* pData = NULL;

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
                        pData = NULL;
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
                        fprintf( stderr, "found %d atoms in MULTIPLE request\n", nItems );
#endif
                        bEventSuccess = true;
                        bool bResetAtoms = false;
                        Atom* pAtoms = (Atom*)pData;
                        aGuard.clear();
                        for( int i = 0; i < nItems; i += 2 )
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
                    fprintf( stderr, "could not get type list from \"%s\" of type \"%s\" on requestor 0x%x, requestor has properties:",
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
        dsde.DragSourceContext      = new DragSourceContext( m_aDropWindow, rRequest.time, *this );
        dsde.DragSource             = static_cast< XDragSource* >(this);
        if( aNotify.xselection.property != None )
        {
            dsde.DropAction         = DNDConstants::ACTION_COPY;
            dsde.DropSuccess        = sal_True;
        }
        else
        {
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = sal_False;
        }
        Reference< XDragSourceListener > xListener( m_xDragSourceListener );
        m_xDragSourceListener.clear();
        aGuard.clear();
        if( xListener.is() )
            xListener->dragDropEnd( dsde );
    }
}

// ------------------------------------------------------------------------

void SelectionManager::handleReceivePropertyNotify( XPropertyEvent& rNotify )
{
    MutexGuard aGuard( m_aMutex );
    // data we requested arrived
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "handleReceivePropertyNotify for property %s\n",
             OUStringToOString( getString( rNotify.atom ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    ::std::hash_map< Atom, Selection* >::iterator it =
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
            return;

        Atom nType = None;
        int nFormat = 0;
        unsigned long nItems = 0, nBytes = 0;
        unsigned char* pData = NULL;

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
        fprintf( stderr, "found %d bytes data of type %s and format %d, items = %d\n",
                 nBytes,
                 OUStringToOString( getString( nType ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                 nFormat, nItems );
#endif
        if( pData )
        {
            XFree( pData );
            pData = NULL;
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
            fprintf( stderr, "read %d items data of type %s and format %d, %d bytes left in property\n",
                     nItems,
                     OUStringToOString( getString( nType ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                     nFormat, nBytes );
#endif
            if( it->second->m_eState == Selection::WaitingForData ||
                it->second->m_eState == Selection::WaitingForResponse )
            {
                // copy data
                it->second->m_aData = Sequence< sal_Int8 >( (sal_Int8*)pData, nItems*nFormat/8 );
                it->second->m_eState = Selection::Inactive;
                it->second->m_aDataArrived.set();
            }
            else if( it->second->m_eState == Selection::IncrementalTransfer )
            {
                if( nItems )
                {
                    // append data
                    Sequence< sal_Int8 > aData( it->second->m_aData.getLength() + nItems*nFormat/8 );
                    memcpy( aData.getArray(), it->second->m_aData.getArray(), it->second->m_aData.getLength() );
                    memcpy( aData.getArray() + it->second->m_aData.getLength(), pData, nItems*nFormat/8 );
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
}

// ------------------------------------------------------------------------

void SelectionManager::handleSendPropertyNotify( XPropertyEvent& rNotify )
{
    MutexGuard aGuard( m_aMutex );

    // ready for next part of a IncrementalTransfer
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "handleSendPropertyNotify for property %s (%s)\n",
             OUStringToOString( getString( rNotify.atom ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             rNotify.state == PropertyNewValue ? "new value" : ( rNotify.state == PropertyDelete ? "deleted" : "unknown")
             );
#endif

    // feed incrementals
    if( rNotify.state == PropertyDelete )
    {
        ::std::hash_map< Window, ::std::list< IncrementalTransfer > >::iterator it;
        it = m_aIncrementals.find( rNotify.window );
        int nCurrentTime = time( NULL );
        if( it != m_aIncrementals.end() )
        {
            ::std::list< IncrementalTransfer >::iterator inc_it = it->second.begin();
            while( inc_it != it->second.end() )
            {
                bool bDone = false;
                if( inc_it->m_aProperty == rNotify.atom )
                {
                    int nBytes = inc_it->m_aData.getLength() - inc_it->m_nBufferPos;
                    nBytes = nBytes > INCR_MIN_SIZE ? INCR_MIN_SIZE : nBytes;
                    XChangeProperty(
                                    m_pDisplay,
                                    inc_it->m_aRequestor,
                                    inc_it->m_aProperty,
                                    inc_it->m_aTarget,
                                    inc_it->m_nFormat,
                                    PropModeReplace,
                                    (const unsigned char*)inc_it->m_aData.getConstArray()+inc_it->m_nBufferPos,
                                    nBytes/(inc_it->m_nFormat/8) );
                    inc_it->m_nBufferPos += nBytes;
                    if( nBytes == 0 )
                        bDone = true;
                }
                else if( nCurrentTime - inc_it->m_nTransferStartTime > INCR_TIMEOUT )
                    bDone = true;
                if( bDone )
                {
                    ::std::list< IncrementalTransfer >::iterator temp_it = inc_it;
                    ++inc_it;
                    it->second.erase( temp_it );
                }
                else
                    ++inc_it;
            }
        }
    }
}

// ------------------------------------------------------------------------

void SelectionManager::handleSelectionNotify( XSelectionEvent& rNotify )
{
    MutexGuard aGuard( m_aMutex );

    // notification about success/failure of one of our conversion requests
#if OSL_DEBUG_LEVEL > 1
    OUString aSelection( getString( rNotify.selection ) );
    OUString aProperty( OUString::createFromAscii( "None" ) );
    if( rNotify.property )
        aProperty = getString( rNotify.property );
    fprintf( stderr, "handleSelectionNotify for selection %s and property %s (0x%x)\n",
             OUStringToOString( aSelection, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( aProperty, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             rNotify.property
             );
    if( rNotify.requestor != m_aWindow && rNotify.requestor != m_aCurrentDropWindow )
        fprintf( stderr, "Warning: selection notify for unknown window 0x%x\n", rNotify.requestor );
#endif
    ::std::hash_map< Atom, Selection* >::iterator it =
          m_aSelections.find( rNotify.selection );
    if( ( rNotify.requestor == m_aWindow || rNotify.requestor == m_aCurrentDropWindow )     &&
        it != m_aSelections.end()           &&
        ( it->second->m_eState == Selection::WaitingForResponse ) ||
        ( it->second->m_eState == Selection::WaitingForData ) )
    {
        if( it->second->m_aRequestedType == m_nMULTIPLEAtom )
        {
            Atom nType = None;
            int nFormat = 0;
            unsigned long nItems = 0, nBytes = 0;
            unsigned char* pData = NULL;

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
            it->second->m_aData         = Sequence< sal_Int8 >((sal_Int8*)pData, nFormat/8 * nItems );
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
}

// ------------------------------------------------------------------------

void SelectionManager::handleDropEvent( XClientMessageEvent& rMessage )
{
    ClearableMutexGuard aGuard(m_aMutex);

    // handle drop related events
    Window aSource = rMessage.data.l[0];
    Window aTarget = rMessage.window;

    ::std::hash_map< Window, DropTargetEntry >::iterator it =
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
        else if( m_aDropEnterEvent.data.l[0] != None && m_aDropEnterEvent.data.l[0] != aSource )
            fprintf( stderr, "but source 0x%x is unknown (expected 0x%x or 0)\n", aSource, m_aDropEnterEvent.data.l[0] );
        else
            fprintf( stderr, "processing.\n" );
    }
#endif

    if( it != m_aDropTargets.end() && it->second.m_pTarget->m_bActive &&
        m_bDropWaitingForCompletion && m_aDropEnterEvent.data.l[0] )
    {
        OSL_ENSURE( 0, "someone forgot to call dropComplete ?" );
        // some listener forgot to call dropComplete in the last operation
        // let us end it now and accept the new enter event
        dropComplete( sal_False, m_aCurrentDropWindow, m_nDropTime );
    }

    if( it != m_aDropTargets.end() &&
        it->second.m_pTarget->m_bActive &&
        ( m_aDropEnterEvent.data.l[0] == None || m_aDropEnterEvent.data.l[0] == aSource )
        )
    {
        if( rMessage.message_type == m_nXdndEnter )
        {
            m_aDropEnterEvent           = rMessage;
            m_bDropEnterSent            = false;
            m_aCurrentDropWindow        = aTarget;
            m_nCurrentProtocolVersion   = m_aDropEnterEvent.data.l[1] >> 24;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndEnter on 0x%x\n", aTarget );
#endif
        }
        else if(
                rMessage.message_type == m_nXdndPosition &&
                aSource == m_aDropEnterEvent.data.l[0]
                )
        {
            m_nDropTime = m_nCurrentProtocolVersion > 0 ? rMessage.data.l[3] : CurrentTime;
            if( ! m_bDropEnterSent )
                m_nDropTimestamp = m_nDropTime;

            Window aChild;
            XTranslateCoordinates( m_pDisplay,
                                   it->second.m_aRootWindow,
                                   it->first,
                                   rMessage.data.l[2] >> 16,
                                   rMessage.data.l[2] & 0xffff,
                                   &m_nLastX, &m_nLastY,
                                   &aChild );

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndPosition on 0x%x (%d, %d)\n", aTarget, m_nLastX, m_nLastY );
#endif
            DropTargetDragEnterEvent aEvent;
            aEvent.Source       = static_cast< XDropTarget* >(it->second.m_pTarget);
            aEvent.Context      = new DropTargetDragContext( m_aCurrentDropWindow, m_nDropTimestamp, *this );
            aEvent.LocationX    = m_nLastX;
            aEvent.LocationY    = m_nLastY;
            aEvent.SourceActions = m_nSourceActions;
            if( m_nCurrentProtocolVersion < 2 )
                aEvent.DropAction = DNDConstants::ACTION_COPY;
            else if( rMessage.data.l[4] == m_nXdndActionCopy )
                aEvent.DropAction = DNDConstants::ACTION_COPY;
            else if( rMessage.data.l[4] == m_nXdndActionMove )
                aEvent.DropAction = DNDConstants::ACTION_MOVE;
            else if( rMessage.data.l[4] == m_nXdndActionLink )
                aEvent.DropAction = DNDConstants::ACTION_LINK;
            else if( rMessage.data.l[4] == m_nXdndActionAsk )
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
                aSource == m_aDropEnterEvent.data.l[0]
                )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndLeave on 0x%x\n", aTarget );
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
                aSource == m_aDropEnterEvent.data.l[0]
                )
        {
            m_nDropTime = m_nCurrentProtocolVersion > 0 ? rMessage.data.l[2] : CurrentTime;

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "received XdndDrop on 0x%x (%d, %d)\n", aTarget, m_nLastX, m_nLastY );
#endif
            if( m_bLastDropAccepted )
            {
                DropTargetDropEvent aEvent;
                aEvent.Source       = static_cast< XDropTarget* >(it->second.m_pTarget);
                aEvent.Context      = new DropTargetDropContext( m_aCurrentDropWindow, m_nDropTimestamp, *this );
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
                fprintf( stderr, "XdndDrop canceled due to m_bLastDropAccepted = fale\n" );
#endif
                DropTargetEvent aEvent;
                aEvent.Source = static_cast< XDropTarget* >(it->second.m_pTarget);
                aGuard.clear();
                it->second->dragExit( aEvent );
                // reset the drop status, notify source
                dropComplete( sal_False, m_aCurrentDropWindow, m_nDropTime );
            }
        }
    }
}

/*
 *  methods for XDropTargetDropContext
 */

void SelectionManager::dropComplete( sal_Bool bSuccess, Window aDropWindow, Time aTimestamp )
{
    ClearableMutexGuard aGuard(m_aMutex);

    if( aDropWindow == m_aCurrentDropWindow )
    {
        if( m_xDragSourceListener.is() )
        {
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = getUserDragAction();
            dsde.DropSuccess        = bSuccess;
            Reference< XDragSourceListener > xListener = m_xDragSourceListener;
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
            fprintf( stderr, "Sending XdndFinished to 0x%x\n",
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
        OSL_ASSERT( "dropComplete from invalid DropTargetDropContext" );
}

/*
 *  methods for XDropTargetDragContext
 */

// ------------------------------------------------------------------------

void SelectionManager::sendDragStatus( Atom nDropAction )
{
    ClearableMutexGuard aGuard(m_aMutex);

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
            setCursor( getDefaultCursor( nNewDragAction ), m_aDropWindow, m_nDragTimestamp );
            m_nTargetAcceptAction = nNewDragAction;
        }

        DragSourceDragEvent dsde;
        dsde.Source             = static_cast< OWeakObject* >(this);
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = m_nSourceActions;
        dsde.UserAction         = getUserDragAction();

        Reference< XDragSourceListener > xListener( m_xDragSourceListener );
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
        fprintf( stderr, "Sending XdndStatus to 0x%x with action %s\n",
                 m_aDropEnterEvent.data.l[0],
                 OUStringToOString( getString( nDropAction ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                 );
#endif

        XSendEvent( m_pDisplay, m_aDropEnterEvent.data.l[0],
                    False, NoEventMask, & aEvent );
        XFlush( m_pDisplay );
    }
}

// ------------------------------------------------------------------------

sal_Int8 SelectionManager::getUserDragAction() const
{
    return (m_nTargetAcceptAction != DNDConstants::ACTION_DEFAULT) ? m_nTargetAcceptAction : m_nUserDragAction;
}

// ------------------------------------------------------------------------

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
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = m_nUserDragAction;
        dsde.UserAction         = m_nUserDragAction;
        m_nTargetAcceptAction   = DNDConstants::ACTION_DEFAULT; // invalidate last accept
        m_xDragSourceListener->dropActionChanged( dsde );
    }
    return bRet;
}

// ------------------------------------------------------------------------

void SelectionManager::sendDropPosition( bool bForce, Time eventTime )
{
    ClearableMutexGuard aGuard(m_aMutex);

    if( m_bDropSent )
        return;

    ::std::hash_map< Window, DropTargetEntry >::const_iterator it =
          m_aDropTargets.find( m_aDropWindow );
    if( it != m_aDropTargets.end() )
    {
        if( it->second.m_pTarget->m_bActive )
        {
            int x, y;
            Window aChild;
            XTranslateCoordinates( m_pDisplay, it->second.m_aRootWindow, m_aDropWindow, m_nLastDragX, m_nLastDragY, &x, &y, &aChild );
            DropTargetDragEvent dtde;
            dtde.Source         = static_cast< OWeakObject* >(it->second.m_pTarget );
            dtde.Context        = new DropTargetDragContext( m_aCurrentDropWindow, m_nDropTimestamp, *this );
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

// ------------------------------------------------------------------------

void SelectionManager::handleDragEvent( XEvent& rMessage )
{
    if( ! m_xDragSourceListener.is() )
        return;

    ResettableMutexGuard aGuard(m_aMutex);

    // for shortcut
    ::std::hash_map< Window, DropTargetEntry >::const_iterator it =
          m_aDropTargets.find( m_aDropWindow );
#if OSL_DEBUG_LEVEL > 1
    switch( rMessage.type )
    {
        case ClientMessage:
            fprintf( stderr, "handleDragEvent: %s\n", OUStringToOString( getString( rMessage.xclient.message_type ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
            break;
        case MotionNotify:
//          fprintf( stderr, "handleDragEvent: MotionNotify\n" );
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
        if( rMessage.xclient.message_type == m_nXdndStatus && rMessage.xclient.data.l[0] == m_aDropWindow )
        {
            DragSourceDragEvent dsde;
            dsde.Source                 = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext      = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
            dsde.DragSource             = static_cast< XDragSource* >( this );
            dsde.UserAction = getUserDragAction();
            dsde.DropAction = DNDConstants::ACTION_NONE;
            m_bDropSuccess = rMessage.xclient.data.l[1] & 1 ? true : false;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "status drop action: accept = %s, %s\n",
                     m_bDropSuccess ? "true" : "false",
                     OUStringToOString( getString( rMessage.xclient.data.l[4] ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
            if( rMessage.xclient.data.l[1] & 1 )
            {
                if( m_nCurrentProtocolVersion > 1 )
                {
                    if( rMessage.xclient.data.l[4] == m_nXdndActionCopy )
                        dsde.DropAction = DNDConstants::ACTION_COPY;
                    else if( rMessage.xclient.data.l[4] == m_nXdndActionMove )
                        dsde.DropAction = DNDConstants::ACTION_MOVE;
                    else if( rMessage.xclient.data.l[4] == m_nXdndActionLink )
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

            setCursor( getDefaultCursor( dsde.DropAction ), m_aDropWindow, m_nDragTimestamp );
            aGuard.clear();
            m_xDragSourceListener->dragOver( dsde );
        }
        else if( rMessage.xclient.message_type == m_nXdndFinished && m_aDropWindow == rMessage.xclient.data.l[0] )
        {
            // notify the listener
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = m_nTargetAcceptAction;
            dsde.DropSuccess        = m_bDropSuccess;
            Reference< XDragSourceListener > xListener( m_xDragSourceListener );
            m_xDragSourceListener.clear();
            aGuard.clear();
            xListener->dragDropEnd( dsde );
        }
    }
    else if( rMessage.type == MotionNotify ||
             rMessage.type == EnterNotify || rMessage.type == LeaveNotify
             )
    {
        bool bForce = false;
        int root_x  = rMessage.type == MotionNotify ? rMessage.xmotion.x_root : rMessage.xcrossing.x_root;
        int root_y  = rMessage.type == MotionNotify ? rMessage.xmotion.y_root : rMessage.xcrossing.y_root;
        Window root = rMessage.type == MotionNotify ? rMessage.xmotion.root : rMessage.xcrossing.root;
        m_nDragTimestamp = rMessage.type == MotionNotify ? rMessage.xmotion.time : rMessage.xcrossing.time;

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
        KeySym aKey = XKeycodeToKeysym( m_pDisplay, rMessage.xkey.keycode, 0 );
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
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = sal_False;
            Reference< XDragSourceListener > xListener( m_xDragSourceListener );
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
                    int x, y;
                    Window aChild;
                    XTranslateCoordinates( m_pDisplay, rMessage.xbutton.root, m_aDropWindow, rMessage.xbutton.x_root, rMessage.xbutton.y_root, &x, &y, &aChild );
                    DropTargetDropEvent dtde;
                    dtde.Source         = static_cast< OWeakObject* >(it->second.m_pTarget );
                    dtde.Context        = new DropTargetDropContext( m_aCurrentDropWindow, m_nDropTimestamp, *this );
                    dtde.LocationX      = x;
                    dtde.LocationY      = y;
                    dtde.DropAction     = m_nUserDragAction;
                    dtde.SourceActions  = m_nSourceActions;
                    dtde.Transferable   = m_xDragSourceTransferable;
                    m_bDropSent                 = true;
                    m_nDropTimeout              = time( NULL );
                    m_bDropWaitingForCompletion = true;
                    aGuard.clear();
                    it->second->drop( dtde );
                    bCancel = false;
                }
                else bCancel = true;
            }
            else if( m_nCurrentProtocolVersion >= 0 )
            {
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
                m_nDropTimeout              = time( NULL );
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
                    Window aDummy;
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
                    m_nDropTimeout              = time( NULL );
                    XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
                    m_bWaitingForPrimaryConversion  = true;
                    m_bDropSent                     = true;
                    m_nDropTimeout                  = time( NULL );
                    // HACK :-)
                    static_cast< X11Clipboard* >( pAdaptor )->setContents( m_xDragSourceTransferable, Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >() );
                    bCancel = false;
                }
            }
        }
        if( bCancel )
        {
            // cancel drag
            DragSourceDropEvent dsde;
            dsde.Source             = static_cast< OWeakObject* >(this);
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = sal_False;
            Reference< XDragSourceListener > xListener( m_xDragSourceListener );
            m_xDragSourceListener.clear();
            aGuard.clear();
            xListener->dragDropEnd( dsde );
        }
    }
}

// ------------------------------------------------------------------------

void SelectionManager::accept( sal_Int8 dragOperation, Window aDropWindow, Time aTimestamp )
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

// ------------------------------------------------------------------------

void SelectionManager::reject( Window aDropWindow, Time aTimestamp )
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
            dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
            dsde.DragSource         = static_cast< XDragSource* >(this);
            dsde.DropAction         = DNDConstants::ACTION_NONE;
            dsde.DropSuccess        = sal_False;
            m_xDragSourceListener->dragDropEnd( dsde );
            m_xDragSourceListener.clear();
        }
    }
}

/*
 *  XDragSource
 */

sal_Bool SelectionManager::isDragImageSupported() throw()
{
    return sal_False;
}

// ------------------------------------------------------------------------

sal_Int32 SelectionManager::getDefaultCursor( sal_Int8 dragAction ) throw()
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

// ------------------------------------------------------------------------

int SelectionManager::getXdndVersion( Window aWindow, Window& rProxy )
{
    Atom* pProperties = NULL;
    int nProperties = 0;
    Atom nType;
    int nFormat;
    unsigned long nItems, nBytes;
    unsigned char* pBytes = NULL;

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
                    rProxy = *(Window*)pBytes;
                XFree( pBytes );
                pBytes = NULL;
                if( rProxy != None )
                {
                    // now check proxy wether it points to itself
                    XGetWindowProperty( m_pDisplay, rProxy, m_nXdndProxy, 0, 1, False, XA_WINDOW,
                                        &nType, &nFormat, &nItems, &nBytes, &pBytes );
                    if( pBytes )
                    {
                        if( nType == XA_WINDOW && *(Window*)pBytes != rProxy )
                            rProxy = None;
                        XFree( pBytes );
                        pBytes = NULL;
                    }
                    else
                        rProxy = None;
                }
            }
            break;
        }
    }
    Window aAwareWindow = rProxy != None ? rProxy : aWindow;

    XGetWindowProperty( m_pDisplay, aAwareWindow, m_nXdndAware, 0, 1, False, XA_ATOM,
                        &nType, &nFormat, &nItems, &nBytes, &pBytes );
    if( pBytes )
    {
        if( nType == XA_ATOM )
            nVersion = *(Atom*)pBytes;
        XFree( pBytes );
    }

    nVersion = nVersion > nXdndProtocolRevision ? nXdndProtocolRevision : nVersion;

    return nVersion;
}

// ------------------------------------------------------------------------

void SelectionManager::updateDragWindow( int nX, int nY, Window aRoot )
{
    ResettableMutexGuard aGuard( m_aMutex );

    Reference< XDragSourceListener > xListener( m_xDragSourceListener );

    m_nLastDragX = nX;
    m_nLastDragY = nY;

    Window aParent = aRoot;
    Window aChild;
    Window aNewProxy = None, aNewCurrentWindow = None;
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
    dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
    dsde.DragSource         = static_cast< XDragSource* >(this);
    dsde.DropAction         = nNewProtocolVersion >= 0 ? m_nUserDragAction : DNDConstants::ACTION_COPY;
    dsde.UserAction         = nNewProtocolVersion >= 0 ? m_nUserDragAction : DNDConstants::ACTION_COPY;

    ::std::hash_map< Window, DropTargetEntry >::const_iterator it;
    if( aNewCurrentWindow != m_aDropWindow )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "drag left window 0x%x (rev. %d), entered window 0x%x (rev %d)\n", m_aDropWindow, m_nCurrentProtocolVersion, aNewCurrentWindow, nNewProtocolVersion );
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
                dtde.Context                = new DropTargetDragContext( m_aCurrentDropWindow, m_nDropTimestamp, *this );
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

// ------------------------------------------------------------------------

void SelectionManager::startDrag(
                                 const DragGestureEvent& trigger,
                                 sal_Int8 sourceActions,
                                 sal_Int32 cursor,
                                 sal_Int32 image,
                                 const Reference< XTransferable >& transferable,
                                 const Reference< XDragSourceListener >& listener
                                 ) throw()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "startDrag( sourceActions = %x )\n", (int)sourceActions );
#endif

    DragSourceDropEvent aDragFailedEvent;
    aDragFailedEvent.Source             = static_cast< OWeakObject* >(this);
    aDragFailedEvent.DragSource         = static_cast< XDragSource* >(this);
    aDragFailedEvent.DragSourceContext  = new DragSourceContext( None, CurrentTime, *this );
    aDragFailedEvent.DropAction         = DNDConstants::ACTION_NONE;
    aDragFailedEvent.DropSuccess        = sal_False;

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

    {
        MutexGuard aGuard(m_aMutex);

        // first get the current pointer position and the window that
        // the pointer is located in. since said window should be one
        // of our DropTargets at the time of executeDrag we can use
        // them for a start
        Window aRoot, aParent, aChild;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;

        ::std::hash_map< Window, DropTargetEntry >::const_iterator it;
        it = m_aDropTargets.begin();
        while( it != m_aDropTargets.end() )
        {
            if( XQueryPointer( m_pDisplay, it->second.m_aRootWindow,
                               &aRoot, &aParent,
                               &root_x, &root_y,
                               &win_x, &win_y,
                               &mask ) )
            {
                aParent = it->second.m_aRootWindow;
                break;
            }
            ++it;
        }

        // don't start DnD if there is none of our windows on the same screen as
        // the pointer or if no mouse button is pressed
        if( it == m_aDropTargets.end() || (mask & (Button1Mask|Button2Mask|Button3Mask)) == 0 )
        {
            if( listener.is() )
                listener->dragDropEnd( aDragFailedEvent );
            return;
        }

        // try to find which of our drop targets is the drag source
        // if that drop target is deregistered we should stop executing
        // the drag (actually this is a poor substitute for an "endDrag"
        // method ).
        m_aDragSourceWindow = None;
        aParent = aRoot = it->second.m_aRootWindow;
        do
        {
            XTranslateCoordinates( m_pDisplay, aRoot, aParent, root_x, root_y, &win_x, &win_y, &aChild );
            if( aChild != None && m_aDropTargets.find( aChild ) != m_aDropTargets.end() )
            {
                m_aDragSourceWindow = aChild;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "found drag source window 0x%x\n", m_aDragSourceWindow );
#endif
                break;
            }
            aParent = aChild;
        } while( aChild != None );


#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "try to grab pointer ... " );
#endif
        int nPointerGrabSuccess =
            XGrabPointer( m_pDisplay, it->second.m_aRootWindow, True,
                          DRAG_EVENT_MASK,
                          GrabModeAsync, GrabModeAsync,
                          None,
                          None,
                          CurrentTime );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "%d\n", nPointerGrabSuccess );
#endif
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "try to grab keyboard ... " );
#endif
        int nKeyboardGrabSuccess =
            XGrabKeyboard( m_pDisplay, it->second.m_aRootWindow, True,
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
            if( listener.is() )
                listener->dragDropEnd( aDragFailedEvent );
            return;
        }

        m_xDragSourceTransferable   = transferable;
        m_xDragSourceListener       = listener;
        m_aDragFlavors              = transferable->getTransferDataFlavors();
        m_aCurrentCursor            = None;

        requestOwnership( m_nXdndSelection );

        ::std::list< Atom > aConversions;
        ::std::list< Atom >::const_iterator type_it;
        getNativeTypeList( m_aDragFlavors, aConversions, m_nXdndSelection );

        int nTypes = aConversions.size();
        Atom* pTypes = (Atom*)alloca( sizeof(Atom)*nTypes );
        type_it = aConversions.begin();
        for( int n = 0; n < nTypes; n++, ++type_it )
            pTypes[n] = *type_it;

        XChangeProperty( m_pDisplay, m_aWindow, m_nXdndTypeList, XA_ATOM, 32, PropModeReplace, (unsigned char*)pTypes, nTypes );

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
        if( trigger.Event.getValueTypeName().equalsAsciiL( "com.sun.star.awt.MouseEvent", 27 ) )
        {
            MouseEvent aEvent;
            trigger.Event >>= aEvent;
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
    m_aDragExecuteThread = osl_createSuspendedThread( runDragExecute, this );
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

        m_aDragRunning.reset();

        if( listener.is() )
            listener->dragDropEnd( aDragFailedEvent );
    }
}

void SelectionManager::runDragExecute( void* pThis )
{
    SelectionManager* This = (SelectionManager*)pThis;
    This->dragDoDispatch();
}

void SelectionManager::dragDoDispatch()
{

    // do drag
    // m_xDragSourceListener will be cleared on finished drop
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "begin executeDrag dispatching\n" );
#endif
    TimeValue aTVal;
    aTVal.Seconds = 0;
    aTVal.Nanosec = 200000000;
    oslThread aThread = m_aDragExecuteThread;
    while( m_xDragSourceListener.is() && ( ! m_bDropSent || time(NULL)-m_nDropTimeout < 5 ) && osl_scheduleThread( aThread ) )
    {
        // let the thread in the run method do the dispatching
        // just look occasionally here whether drop timed out or is completed
        osl_waitThread( &aTVal );
    }
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "end executeDrag dispatching\n" );
#endif
    {
        ClearableMutexGuard aGuard(m_aMutex);

        Reference< XDragSourceListener > xListener( m_xDragSourceListener );
        Reference< XTransferable > xTransferable( m_xDragSourceTransferable );
        m_xDragSourceListener.clear();
        m_xDragSourceTransferable.clear();

        DragSourceDropEvent dsde;
        dsde.Source             = static_cast< OWeakObject* >(this);
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = DNDConstants::ACTION_NONE;
        dsde.DropSuccess        = sal_False;

        // cleanup after drag
        if( m_bWaitingForPrimaryConversion )
            getAdaptor( XA_PRIMARY )->clearTransferable();

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

        m_aDragExecuteThread = NULL;
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

sal_Int32 SelectionManager::getCurrentCursor()
{
    return m_aCurrentCursor;
}

// ------------------------------------------------------------------------

void SelectionManager::setCursor( sal_Int32 cursor, Window aDropWindow, Time aTimestamp )
{
    MutexGuard aGuard( m_aMutex );
    if( aDropWindow == m_aDropWindow && cursor != m_aCurrentCursor )
    {
        if( m_xDragSourceListener.is() && ! m_bDropSent )
        {
            m_aCurrentCursor = cursor;
            XChangeActivePointerGrab( m_pDisplay, DRAG_EVENT_MASK, cursor, CurrentTime );
            XFlush( m_pDisplay );
        }
    }
}

// ------------------------------------------------------------------------

void SelectionManager::setImage( sal_Int32 image, Window aDropWindow, Time aTimestamp )
{
}

// ------------------------------------------------------------------------

void SelectionManager::transferablesFlavorsChanged()
{
    MutexGuard aGuard(m_aMutex);

    m_aDragFlavors = m_xDragSourceTransferable->getTransferDataFlavors();
    int i;

    ::std::list< Atom > aConversions;
    ::std::list< Atom >::const_iterator type_it;

    getNativeTypeList( m_aDragFlavors, aConversions, m_nXdndSelection );

    int nTypes = aConversions.size();
    Atom* pTypes = (Atom*)alloca( sizeof(Atom)*aConversions.size() );
    for( i = 0, type_it = aConversions.begin(); type_it != aConversions.end(); ++type_it, i++ )
        pTypes[i] = *type_it;
    XChangeProperty( m_pDisplay, m_aWindow, m_nXdndTypeList, XA_ATOM, 32, PropModeReplace, (unsigned char*)pTypes, nTypes );

    if( m_aCurrentDropWindow != None && m_nCurrentProtocolVersion >= 0 )
    {
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
        for( int i = 0; i < nTypes && i < 3; i++ )
            aEvent.xclient.data.l[i+2] = pTypes[i];

        XSendEvent( m_pDisplay, m_aDropProxy, False, NoEventMask, &aEvent );
    }
}

/*
 *  dispatch loop
 */

// ------------------------------------------------------------------------

void SelectionManager::handleXEvent( XEvent& rEvent )
{
    /*
     *  since we are XConnectionListener to a second X display
     *  to get client messages it is essential not to dispatch
     *  events twice that we get on both connections
     *
     *  #95201# between dispatching ButtonPress and startDrag
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
        return;

    switch (rEvent.type)
    {
        case SelectionClear:
        {
            MutexGuard aGuard(m_aMutex);
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "SelectionClear for selection %s\n",
                     OUStringToOString( getString( rEvent.xselectionclear.selection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr()
                     );
#endif
            SelectionAdaptor* pAdaptor = getAdaptor( rEvent.xselectionclear.selection );
            if ( pAdaptor )
                pAdaptor->clearTransferable();
            ::std::hash_map< Atom, Selection* >::iterator it( m_aSelections.find( rEvent.xselectionclear.selection ) );
            if( it != m_aSelections.end() )
                it->second->m_bOwner = false;
        }
        break;

        case SelectionRequest:
            handleSelectionRequest( rEvent.xselectionrequest );
            break;
        case PropertyNotify:
            if( rEvent.xproperty.window == m_aWindow ||
                rEvent.xproperty.window == m_aCurrentDropWindow
                )
                handleReceivePropertyNotify( rEvent.xproperty );
            else
                handleSendPropertyNotify( rEvent.xproperty );
            break;
        case SelectionNotify:
            handleSelectionNotify( rEvent.xselection );
            break;
        case ClientMessage:
            // messages from drag target
            if( rEvent.xclient.message_type == m_nXdndStatus ||
                rEvent.xclient.message_type == m_nXdndFinished )
                handleDragEvent( rEvent );
            // messages from drag source
            else if(
                    rEvent.xclient.message_type == m_nXdndEnter     ||
                    rEvent.xclient.message_type == m_nXdndLeave     ||
                    rEvent.xclient.message_type == m_nXdndPosition  ||
                    rEvent.xclient.message_type == m_nXdndDrop
                    )
                handleDropEvent( rEvent.xclient );
            break;
        case EnterNotify:
        case LeaveNotify:
        case MotionNotify:
        case ButtonPress:
        case ButtonRelease:
        case KeyPress:
        case KeyRelease:
            handleDragEvent( rEvent );
            break;
        default:
            ;
    }
}

// ------------------------------------------------------------------------

void SelectionManager::dispatchEvent( int millisec )
{
    pollfd aPollFD;
    XEvent event;

    // query socket handle to poll on
    aPollFD.fd      = ConnectionNumber( m_pDisplay );
    aPollFD.events  = POLLIN;
    aPollFD.revents = 0;

    // wait for activity (outside the xlib)
    if( poll( &aPollFD, 1, millisec ) > 0 )
    {
        // now acquire the mutex to prevent other threads
        // from using the same X connection
        ResettableMutexGuard aGuard(m_aMutex);

        // prevent that another thread already ate the input
        // this can happen if e.g. another thread does
        // an X request getting a response. the response
        // would be removed from the queue and we would end up
        // with an empty socket here
        if( poll( &aPollFD, 1, 0 ) > 0 )
        {
            int nPending = 1;
            while( nPending )
            {
                nPending = XPending( m_pDisplay );
                if( nPending )
                {
                    XNextEvent( m_pDisplay, &event );
                    aGuard.clear();
                    handleXEvent( event );
                    aGuard.reset();
                }
            }
        }
    }
}

// ------------------------------------------------------------------------

void SelectionManager::run( void* pThis )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "SelectionManager::run\n" );
#endif
    // dispatch until the cows come home

    SelectionManager* This = (SelectionManager*)pThis;

    timeval aLast;
    gettimeofday( &aLast, 0 );

    while( osl_scheduleThread(This->m_aThread) )
    {
        This->dispatchEvent( 1000 );

        timeval aNow;
        gettimeofday( &aNow, 0 );

        if( (aNow.tv_sec - aLast.tv_sec) > 0 )
        {
            ClearableMutexGuard aGuard(This->m_aMutex);
            ::std::list< SelectionAdaptor* > aChangeList;

            for( ::std::hash_map< Atom, Selection* >::iterator it = This->m_aSelections.begin(); it != This->m_aSelections.end(); ++it )
            {
                if( it->first != This->m_nXdndSelection && ! it->second->m_bOwner )
                {
                    Window aOwner = XGetSelectionOwner( This->m_pDisplay, it->first );
                    if( aOwner != it->second->m_aLastOwner )
                    {
                        it->second->m_aLastOwner = aOwner;
                        aChangeList.push_back( it->second->m_pAdaptor );
                    }
                }
            }
            aGuard.clear();
            while( aChangeList.begin() != aChangeList.end() )
            {
                aChangeList.front()->fireContentsChanged();
                aChangeList.pop_front();
            }
            aLast = aNow;
        }
    }
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "SelectionManager::run end\n" );
#endif
}

// ------------------------------------------------------------------------

sal_Bool SelectionManager::handleEvent( const Any& event ) throw()
{
    Sequence< sal_Int8 > aSeq;
    event >>= aSeq;

    XEvent* pEvent = (XEvent*)aSeq.getArray();
    handleXEvent( *pEvent );

    return sal_False;
}

// ------------------------------------------------------------------------

void SelectionManager::registerHandler( Atom selection, SelectionAdaptor& rAdaptor )
{
    MutexGuard aGuard(m_aMutex);

    Selection* pNewSelection    = new Selection();
    pNewSelection->m_pAdaptor   = &rAdaptor;
    pNewSelection->m_aAtom      = selection;
    m_aSelections[ selection ]  = pNewSelection;
}

// ------------------------------------------------------------------------

void SelectionManager::deregisterHandler( Atom selection )
{
    MutexGuard aGuard(m_aMutex);

    ::std::hash_map< Atom, Selection* >::iterator it =
          m_aSelections.find( selection );
    if( it != m_aSelections.end() )
    {
        delete it->second->m_pPixmap;
        delete it->second;
        m_aSelections.erase( it );
    }
}

// ------------------------------------------------------------------------

void SelectionManager::registerDropTarget( Window aWindow, DropTarget* pTarget )
{
    MutexGuard aGuard(m_aMutex);

    // sanity check
    ::std::hash_map< Window, DropTargetEntry >::const_iterator it =
          m_aDropTargets.find( aWindow );
    if( it != m_aDropTargets.end() )
        OSL_ASSERT( "attempt to register window as drop target twice" );
    else if( aWindow )
    {
        XSelectInput( m_pDisplay, aWindow, PropertyChangeMask );

        // set XdndAware
        XChangeProperty( m_pDisplay, aWindow, m_nXdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&nXdndProtocolRevision, 1 );

        DropTargetEntry aEntry( pTarget );
        // get root window of window (in 99.999% of all cases this will be
        // DefaultRootWindow( m_pDisplay )
        int x, y;
        unsigned int w, h, bw, d;
        XGetGeometry( m_pDisplay, aWindow, &aEntry.m_aRootWindow,
                      &x, &y, &w, &h, &bw, &d );
        m_aDropTargets[ aWindow ] = aEntry;
    }
    else
        OSL_ASSERT( "attempt to register None as drop target" );
}

// ------------------------------------------------------------------------

void SelectionManager::deregisterDropTarget( Window aWindow )
{
    ClearableMutexGuard aGuard(m_aMutex);

    m_aDropTargets.erase( aWindow );
    if( aWindow == m_aDragSourceWindow && m_aDragRunning.check() )
    {
        // abort drag
        std::hash_map< Window, DropTargetEntry >::const_iterator it =
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
        dsde.DragSourceContext  = new DragSourceContext( m_aDropWindow, m_nDragTimestamp, *this );
        dsde.DragSource         = static_cast< XDragSource* >(this);
        dsde.DropAction         = DNDConstants::ACTION_NONE;
        dsde.DropSuccess        = sal_False;
        Reference< XDragSourceListener > xListener( m_xDragSourceListener );
        m_xDragSourceListener.clear();
        aGuard.clear();
        xListener->dragDropEnd( dsde );
    }
}

/*
 *  SelectionAdaptor
 */

Reference< XTransferable > SelectionManager::getTransferable() throw()
{
    return m_xDragSourceTransferable;
}

// ------------------------------------------------------------------------

void SelectionManager::clearTransferable() throw()
{
    m_xDragSourceTransferable.clear();
}

// ------------------------------------------------------------------------

void SelectionManager::fireContentsChanged() throw()
{
}

// ------------------------------------------------------------------------

/*
 *  SelectionManagerHolder
 */

SelectionManagerHolder::SelectionManagerHolder() :
        ::cppu::WeakComponentImplHelper3<
    XDragSource,
    XInitialization,
    XServiceInfo > (m_aMutex)
{
}

// ------------------------------------------------------------------------

SelectionManagerHolder::~SelectionManagerHolder()
{
}

// ------------------------------------------------------------------------

void SelectionManagerHolder::initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::Exception )
{
    OUString aDisplayName;

    if( arguments.getLength() > 0 )
    {
        Reference< XDisplayConnection > xConn;
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

sal_Bool SelectionManagerHolder::isDragImageSupported() throw()
{
    return m_xRealDragSource.is() ? m_xRealDragSource->isDragImageSupported() : sal_False;
}

// ------------------------------------------------------------------------

sal_Int32 SelectionManagerHolder::getDefaultCursor( sal_Int8 dragAction ) throw()
{
    return m_xRealDragSource.is() ? m_xRealDragSource->getDefaultCursor( dragAction ) : 0;
}

// ------------------------------------------------------------------------

void SelectionManagerHolder::startDrag(
                                       const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger,
                                       sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
                                       const Reference< ::com::sun::star::datatransfer::XTransferable >& transferable,
                                       const Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener
                                       ) throw()
{
    if( m_xRealDragSource.is() )
        m_xRealDragSource->startDrag( trigger, sourceActions, cursor, image, transferable, listener );
}

// ------------------------------------------------------------------------

/*
 *  XServiceInfo
 */

// ------------------------------------------------------------------------

OUString SelectionManagerHolder::getImplementationName() throw()
{
    return OUString::createFromAscii(XDND_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool SelectionManagerHolder::supportsService( const OUString& ServiceName ) throw()
{
    Sequence < OUString > SupportedServicesNames = Xdnd_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SelectionManagerHolder::getSupportedServiceNames() throw()
{
    return Xdnd_getSupportedServiceNames();
}


// ------------------------------------------------------------------------

