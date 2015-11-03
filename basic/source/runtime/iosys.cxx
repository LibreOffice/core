/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/security.h>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>

#include "runtime.hxx"

#ifdef _USE_UNO

// <-- encoding
#include <sal/alloca.h>

#include <ctype.h>
#include <rtl/byteseq.hxx>
#include <rtl/textenc.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <rtl/ustrbuf.hxx>
// encoding -->
#include <comphelper/processfactory.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>

using namespace comphelper;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;
using namespace com::sun::star::bridge;

#endif /* _USE_UNO */

#include "iosys.hxx"
#include "sbintern.hxx"

// Der Input-Dialog:

class SbiInputDialog : public ModalDialog {
    Edit aInput;
    OKButton aOk;
    CancelButton aCancel;
    String aText;
    DECL_LINK( Ok, Window * );
    DECL_LINK( Cancel, Window * );
public:
    SbiInputDialog( Window*, const String& );
    const String& GetInput() { return aText; }
};

SbiInputDialog::SbiInputDialog( Window* pParent, const String& rPrompt )
            :ModalDialog( pParent, WB_3DLOOK | WB_MOVEABLE | WB_CLOSEABLE ),
             aInput( this, WB_3DLOOK | WB_LEFT | WB_BORDER ),
             aOk( this ), aCancel( this )
{
    SetText( rPrompt );
    aOk.SetClickHdl( LINK( this, SbiInputDialog, Ok ) );
    aCancel.SetClickHdl( LINK( this, SbiInputDialog, Cancel ) );
    SetMapMode( MapMode( MAP_APPFONT ) );

    Point aPt = LogicToPixel( Point( 50, 50 ) );
    Size  aSz = LogicToPixel( Size( 145, 65 ) );
    SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 10, 10 ) );
    aSz = LogicToPixel( Size( 120, 12 ) );
    aInput.SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 15, 30 ) );
    aSz = LogicToPixel( Size( 45, 15) );
    aOk.SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 80, 30 ) );
    aSz = LogicToPixel( Size( 45, 15) );
    aCancel.SetPosSizePixel( aPt, aSz );

    aInput.Show();
    aOk.Show();
    aCancel.Show();
}

IMPL_LINK_INLINE_START( SbiInputDialog, Ok, Window *, pWindow )
{
    (void)pWindow;

    aText = aInput.GetText();
    EndDialog( 1 );
    return 0;
}
IMPL_LINK_INLINE_END( SbiInputDialog, Ok, Window *, pWindow )

IMPL_LINK_INLINE_START( SbiInputDialog, Cancel, Window *, pWindow )
{
    (void)pWindow;

    EndDialog( 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SbiInputDialog, Cancel, Window *, pWindow )

//////////////////////////////////////////////////////////////////////////

SbiStream::SbiStream()
    : pStrm( 0 )
{
}

SbiStream::~SbiStream()
{
    delete pStrm;
}

// Ummappen eines SvStream-Fehlers auf einen StarBASIC-Code

void SbiStream::MapError()
{
    if( pStrm )
     switch( pStrm->GetError() )
     {
        case SVSTREAM_OK:
            nError = 0; break;
        case SVSTREAM_FILE_NOT_FOUND:
            nError = SbERR_FILE_NOT_FOUND; break;
        case SVSTREAM_PATH_NOT_FOUND:
            nError = SbERR_PATH_NOT_FOUND; break;
        case SVSTREAM_TOO_MANY_OPEN_FILES:
            nError = SbERR_TOO_MANY_FILES; break;
        case SVSTREAM_ACCESS_DENIED:
            nError = SbERR_ACCESS_DENIED; break;
        case SVSTREAM_INVALID_PARAMETER:
            nError = SbERR_BAD_ARGUMENT; break;
        case SVSTREAM_OUTOFMEMORY:
            nError = SbERR_NO_MEMORY; break;
        default:
            nError = SbERR_IO_ERROR; break;
    }
}

#ifdef _USE_UNO

// TODO: Code is copied from daemons2/source/uno/asciiEncoder.cxx

::rtl::OUString findUserInDescription( const ::rtl::OUString& aDescription )
{
    ::rtl::OUString user;

    sal_Int32 index;
    sal_Int32 lastIndex = 0;

    do
    {
        index = aDescription.indexOf((sal_Unicode) ',', lastIndex);
        ::rtl::OUString token = (index == -1) ? aDescription.copy(lastIndex) : aDescription.copy(lastIndex, index - lastIndex);

        lastIndex = index + 1;

        sal_Int32 eindex = token.indexOf((sal_Unicode)'=');
        ::rtl::OUString left = token.copy(0, eindex).toAsciiLowerCase().trim();
        ::rtl::OUString right = INetURLObject::decode( token.copy(eindex + 1).trim(), '%',
                            INetURLObject::DECODE_WITH_CHARSET );

        if(left.equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user"))))
        {
            user = right;
            break;
        }
    }
    while(index != -1);

    return user;
}

#endif


// Hack for #83750
sal_Bool runsInSetup( void );

sal_Bool needSecurityRestrictions( void )
{
#ifdef _USE_UNO
    static sal_Bool bNeedInit = sal_True;
    static sal_Bool bRetVal = sal_True;

    if( bNeedInit )
    {
        // Hack for #83750, use internal flag until
        // setup provides own service manager
        if( runsInSetup() )
        {
            // Setup is not critical
            bRetVal = sal_False;
            return bRetVal;
        }

        bNeedInit = sal_False;

        // Get system user to compare to portal user
        oslSecurity aSecurity = osl_getCurrentSecurity();
        ::rtl::OUString aSystemUser;
        sal_Bool bRet = osl_getUserName( aSecurity, &aSystemUser.pData );
        if( !bRet )
        {
            // No valid security! -> Secure mode!
            return sal_True;
        }

        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( !xSMgr.is() )
            return sal_True;
        Reference< XBridgeFactory > xBridgeFac( xSMgr->createInstance
            ( ::rtl::OUString::createFromAscii( "com.sun.star.bridge.BridgeFactory" ) ), UNO_QUERY );

        Sequence< Reference< XBridge > > aBridgeSeq;
        sal_Int32 nBridgeCount = 0;
        if( xBridgeFac.is() )
        {
            aBridgeSeq = xBridgeFac->getExistingBridges();
            nBridgeCount = aBridgeSeq.getLength();
        }

        if( nBridgeCount == 0 )
        {
            // No bridges -> local
            bRetVal = sal_False;
            return bRetVal;
        }

        // Iterate through all bridges to find (portal) user property
        const Reference< XBridge >* pBridges = aBridgeSeq.getConstArray();
        bRetVal = sal_False;    // Now only sal_True if user different from portal user is found
        sal_Int32 i;
        for( i = 0 ; i < nBridgeCount ; i++ )
        {
            const Reference< XBridge >& rxBridge = pBridges[ i ];
            ::rtl::OUString aDescription = rxBridge->getDescription();
            ::rtl::OUString aPortalUser = findUserInDescription( aDescription );
            if( !aPortalUser.isEmpty() )
            {
                // User Found, compare to system user
                if( aPortalUser == aSystemUser )
                {
                    // Same user -> system security is ok, bRetVal stays FALSE
                    break;
                }
                else
                {
                    // Different user -> Secure mode!
                    bRetVal = sal_True;
                    break;
                }
            }
        }
        // No user found or PortalUser != SystemUser -> Secure mode! (Keep default value)
    }

    return bRetVal;
#else
    return sal_False;
#endif
}

// Returns sal_True if UNO is available, otherwise the old file
// system implementation has to be used
// #89378 New semantic: Don't just ask for UNO but for UCB
sal_Bool hasUno( void )
{
#ifdef _USE_UNO
    static sal_Bool bNeedInit = sal_True;
    static sal_Bool bRetVal = sal_True;

    if( bNeedInit )
    {
        bNeedInit = sal_False;
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( !xSMgr.is() )
        {
            // No service manager at all
            bRetVal = sal_False;
        }
        else
        {
            Reference< XContentProviderManager > xManager( xSMgr->createInstance( ::rtl::OUString::createFromAscii
                    ( "com.sun.star.ucb.UniversalContentBroker" ) ), UNO_QUERY );

            if ( !( xManager.is() && xManager->queryContentProvider( ::rtl::OUString::createFromAscii( "file:///" ) ).is() ) )
            {
                // No UCB
                bRetVal = sal_False;
            }
        }
    }
    return bRetVal;
#else
    return sal_False;
#endif
}



#ifndef _OLD_FILE_IMPL

class OslStream : public SvStream
{
    File maFile;
    short mnStrmMode;

public:
                    OslStream( const String& rName, short nStrmMode );
                    ~OslStream();
    virtual sal_uIntPtr GetData( void* pData, sal_uIntPtr nSize );
    virtual sal_uIntPtr PutData( const void* pData, sal_uIntPtr nSize );
    virtual sal_uIntPtr SeekPos( sal_uIntPtr nPos );
    virtual void    FlushData();
    virtual void    SetSize( sal_uIntPtr nSize );
};

OslStream::OslStream( const String& rName, short nStrmMode )
    : maFile( rName )
    , mnStrmMode( nStrmMode )
{
    sal_uInt32 nFlags;

    if( (nStrmMode & (STREAM_READ | STREAM_WRITE)) == (STREAM_READ | STREAM_WRITE) )
    {
        nFlags = OpenFlag_Read | OpenFlag_Write;
    }
    else if( nStrmMode & STREAM_WRITE )
    {
        nFlags = OpenFlag_Write;
    }
    else //if( nStrmMode & STREAM_READ )
    {
        nFlags = OpenFlag_Read;
    }

    FileBase::RC nRet = maFile.open( nFlags );
    if( nRet == FileBase::E_NOENT && nFlags != OpenFlag_Read )
    {
        nFlags |= OpenFlag_Create;
        nRet = maFile.open( nFlags );
    }

    if( nRet != FileBase::E_None )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}


OslStream::~OslStream()
{
    maFile.close();
}

sal_uIntPtr OslStream::GetData( void* pData, sal_uIntPtr nSize )
{
    sal_uInt64 nBytesRead = nSize;
    FileBase::RC nRet = FileBase::E_None;
    nRet = maFile.read( pData, nBytesRead, nBytesRead );
    return (sal_uIntPtr)nBytesRead;
}

sal_uIntPtr OslStream::PutData( const void* pData, sal_uIntPtr nSize )
{
    sal_uInt64 nBytesWritten;
    FileBase::RC nRet = FileBase::E_None;
    nRet = maFile.write( pData, (sal_uInt64)nSize, nBytesWritten );
    return (sal_uIntPtr)nBytesWritten;
}

sal_uIntPtr OslStream::SeekPos( sal_uIntPtr nPos )
{
    FileBase::RC nRet;
    if( nPos == STREAM_SEEK_TO_END )
    {
        nRet = maFile.setPos( Pos_End, 0 );
    }
    else
    {
        nRet = maFile.setPos( Pos_Absolut, (sal_uInt64)nPos );
    }
    sal_uInt64 nRealPos;
    nRet = maFile.getPos( nRealPos );
    return sal::static_int_cast<sal_uIntPtr>(nRealPos);
}

void OslStream::FlushData()
{
}

void OslStream::SetSize( sal_uIntPtr nSize )
{
    FileBase::RC nRet = FileBase::E_None;
    nRet = maFile.setSize( (sal_uInt64)nSize );
}

#endif


#ifdef _USE_UNO

class UCBStream : public SvStream
{
    Reference< XInputStream >   xIS;
    Reference< XOutputStream >  xOS;
    Reference< XStream >        xS;
    Reference< XSeekable >      xSeek;
public:
                    UCBStream( Reference< XInputStream > & xIS );
                    UCBStream( Reference< XOutputStream > & xOS );
                    UCBStream( Reference< XStream > & xS );
                    ~UCBStream();
    virtual sal_uIntPtr GetData( void* pData, sal_uIntPtr nSize );
    virtual sal_uIntPtr PutData( const void* pData, sal_uIntPtr nSize );
    virtual sal_uIntPtr SeekPos( sal_uIntPtr nPos );
    virtual void    FlushData();
    virtual void    SetSize( sal_uIntPtr nSize );
};

/*
sal_uIntPtr UCBErrorToSvStramError( ucb::IOErrorCode nError )
{
    sal_uIntPtr eReturn = ERRCODE_IO_GENERAL;
    switch( nError )
    {
        case ucb::IOErrorCode_ABORT:                eReturn = SVSTREAM_GENERALERROR; break;
        case ucb::IOErrorCode_NOT_EXISTING:         eReturn = SVSTREAM_FILE_NOT_FOUND; break;
        case ucb::IOErrorCode_NOT_EXISTING_PATH:    eReturn = SVSTREAM_PATH_NOT_FOUND; break;
        case ucb::IOErrorCode_OUT_OF_FILE_HANDLES:  eReturn = SVSTREAM_TOO_MANY_OPEN_FILES; break;
        case ucb::IOErrorCode_ACCESS_DENIED:        eReturn = SVSTREAM_ACCESS_DENIED; break;
        case ucb::IOErrorCode_LOCKING_VIOLATION:    eReturn = SVSTREAM_SHARING_VIOLATION; break;

        case ucb::IOErrorCode_INVALID_ACCESS:       eReturn = SVSTREAM_INVALID_ACCESS; break;
        case ucb::IOErrorCode_CANT_CREATE:          eReturn = SVSTREAM_CANNOT_MAKE; break;
        case ucb::IOErrorCode_INVALID_PARAMETER:    eReturn = SVSTREAM_INVALID_PARAMETER; break;

        case ucb::IOErrorCode_CANT_READ:            eReturn = SVSTREAM_READ_ERROR; break;
        case ucb::IOErrorCode_CANT_WRITE:           eReturn = SVSTREAM_WRITE_ERROR; break;
        case ucb::IOErrorCode_CANT_SEEK:            eReturn = SVSTREAM_SEEK_ERROR; break;
        case ucb::IOErrorCode_CANT_TELL:            eReturn = SVSTREAM_TELL_ERROR; break;

        case ucb::IOErrorCode_OUT_OF_MEMORY:        eReturn = SVSTREAM_OUTOFMEMORY; break;

        case SVSTREAM_FILEFORMAT_ERROR:             eReturn = SVSTREAM_FILEFORMAT_ERROR; break;
        case ucb::IOErrorCode_WRONG_VERSION:        eReturn = SVSTREAM_WRONGVERSION;
        case ucb::IOErrorCode_OUT_OF_DISK_SPACE:    eReturn = SVSTREAM_DISK_FULL; break;

        case ucb::IOErrorCode_BAD_CRC:              eReturn = ERRCODE_IO_BADCRC; break;
    }
    return eReturn;
}
*/

UCBStream::UCBStream( Reference< XInputStream > & rStm )
    : xIS( rStm )
    , xSeek( rStm, UNO_QUERY )
{
}

UCBStream::UCBStream( Reference< XOutputStream > & rStm )
    : xOS( rStm )
    , xSeek( rStm, UNO_QUERY )
{
}

UCBStream::UCBStream( Reference< XStream > & rStm )
    : xS( rStm )
    , xSeek( rStm, UNO_QUERY )
{
}


UCBStream::~UCBStream()
{
    try
    {
        if( xIS.is() )
            xIS->closeInput();
        else if( xOS.is() )
            xOS->closeOutput();
        else if( xS.is() )
        {
            Reference< XInputStream > xIS_ = xS->getInputStream();
            if( xIS_.is() )
                xIS_->closeInput();
        }
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}

sal_uIntPtr UCBStream::GetData( void* pData, sal_uIntPtr nSize )
{
    try
    {
        Reference< XInputStream > xISFromS;
        if( xIS.is() )
        {
            Sequence<sal_Int8> aData;
            nSize = xIS->readBytes( aData, nSize );
            rtl_copyMemory( pData, aData.getConstArray(), nSize );
            return nSize;
        }
        else if( xS.is() && (xISFromS = xS->getInputStream()).is() )
        {
            Sequence<sal_Int8> aData;
            nSize = xISFromS->readBytes( aData, nSize );
            rtl_copyMemory( pData, aData.getConstArray(), nSize );
            return nSize;
        }
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

sal_uIntPtr UCBStream::PutData( const void* pData, sal_uIntPtr nSize )
{
    try
    {
        Reference< XOutputStream > xOSFromS;
        if( xOS.is() )
        {
            Sequence<sal_Int8> aData( (const sal_Int8 *)pData, nSize );
            xOS->writeBytes( aData );
            return nSize;
        }
        else if( xS.is() && (xOSFromS = xS->getOutputStream()).is() )
        {
            Sequence<sal_Int8> aData( (const sal_Int8 *)pData, nSize );
            xOSFromS->writeBytes( aData );
            return nSize;
        }
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

sal_uIntPtr UCBStream::SeekPos( sal_uIntPtr nPos )
{
    try
    {
        if( xSeek.is() )
        {
            sal_uIntPtr nLen = sal::static_int_cast<sal_uIntPtr>( xSeek->getLength() );
            if( nPos > nLen )
                nPos = nLen;
            xSeek->seek( nPos );
            return nPos;
        }
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

void    UCBStream::FlushData()
{
    try
    {
        Reference< XOutputStream > xOSFromS;
        if( xOS.is() )
            xOS->flush();
        else if( xS.is() && (xOSFromS = xS->getOutputStream()).is() )
            xOSFromS->flush();
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}

void    UCBStream::SetSize( sal_uIntPtr nSize )
{
    (void)nSize;

    DBG_ERROR( "not allowed to call from basic" );
    SetError( ERRCODE_IO_GENERAL );
}

#endif

// Oeffnen eines Streams
SbError SbiStream::Open
( short nCh, const ByteString& rName, short nStrmMode, short nFlags, short nL )
{
    nMode   = nFlags;
    nLen    = nL;
    nChan   = nCh;
    nLine   = 0;
    nExpandOnWriteTo = 0;
    if( ( nStrmMode & ( STREAM_READ|STREAM_WRITE ) ) == STREAM_READ )
        nStrmMode |= STREAM_NOCREATE;
    String aStr( rName, gsl_getSystemTextEncoding() );
    String aNameStr = getFullPath( aStr );

#ifdef _USE_UNO
    if( hasUno() )
    {
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            Reference< XSimpleFileAccess >
                xSFI( xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
            if( xSFI.is() )
            {
                try
                {

                // #??? For write access delete file if it already exists (not for appending)
                if( (nStrmMode & STREAM_WRITE) != 0 && !IsAppend() && !IsBinary() && !IsRandom() &&
                    xSFI->exists( aNameStr ) && !xSFI->isFolder( aNameStr ) )
                {
                    xSFI->kill( aNameStr );
                }

                if( (nStrmMode & (STREAM_READ | STREAM_WRITE)) == (STREAM_READ | STREAM_WRITE) )
                {
                    Reference< XStream > xIS = xSFI->openFileReadWrite( aNameStr );
                    pStrm = new UCBStream( xIS );
                }
                else if( nStrmMode & STREAM_WRITE )
                {
                    Reference< XStream > xIS = xSFI->openFileReadWrite( aNameStr );
                    pStrm = new UCBStream( xIS );
                    // Open for writing is not implemented in ucb yet!!!
                    //Reference< XOutputStream > xIS = xSFI->openFileWrite( aNameStr );
                    //pStrm = new UCBStream( xIS );
                }
                else //if( nStrmMode & STREAM_READ )
                {
                    Reference< XInputStream > xIS = xSFI->openFileRead( aNameStr );
                    pStrm = new UCBStream( xIS );
                }

                }
                catch( Exception & )
                {
                    nError = ERRCODE_IO_GENERAL;
                }
            }
        }
    }

#endif
    if( !pStrm )
    {
#ifdef _OLD_FILE_IMPL
        pStrm = new SvFileStream( aNameStr, nStrmMode );
#else
        pStrm = new OslStream( aNameStr, nStrmMode );
#endif
    }
    if( IsAppend() )
        pStrm->Seek( STREAM_SEEK_TO_END );
    MapError();
    if( nError )
        delete pStrm, pStrm = NULL;
    return nError;
}

SbError SbiStream::Close()
{
    if( pStrm )
    {
        if( !hasUno() )
        {
#ifdef _OLD_FILE_IMPL
            ((SvFileStream *)pStrm)->Close();
#endif
        }
        MapError();
        delete pStrm;
        pStrm = NULL;
    }
    nChan = 0;
    return nError;
}

SbError SbiStream::Read( ByteString& rBuf, sal_uInt16 n, bool bForceReadingPerByte )
{
    nExpandOnWriteTo = 0;
    if( !bForceReadingPerByte && IsText() )
    {
        pStrm->ReadLine( rBuf );
        nLine++;
    }
    else
    {
        if( !n ) n = nLen;
        if( !n )
            return nError = SbERR_BAD_RECORD_LENGTH;
        rBuf.Fill( n, ' ' );
        pStrm->Read( (void*)rBuf.GetBuffer(), n );
    }
    MapError();
    if( !nError && pStrm->IsEof() )
        nError = SbERR_READ_PAST_EOF;
    return nError;
}

SbError SbiStream::Read( char& ch )
{
    nExpandOnWriteTo = 0;
    if( !aLine.Len() )
    {
        Read( aLine, 0 );
        aLine += '\n';
    }
    ch = aLine.GetBuffer()[0];
    aLine.Erase( 0, 1 );
    return nError;
}

void SbiStream::ExpandFile()
{
    if ( nExpandOnWriteTo )
    {
        sal_uIntPtr nCur = pStrm->Seek(STREAM_SEEK_TO_END);
        if( nCur < nExpandOnWriteTo )
        {
            sal_uIntPtr nDiff = nExpandOnWriteTo - nCur;
            char c = 0;
            while( nDiff-- )
                *pStrm << c;
        }
        else
        {
            pStrm->Seek( nExpandOnWriteTo );
        }
        nExpandOnWriteTo = 0;
    }
}

SbError SbiStream::Write( const ByteString& rBuf, sal_uInt16 n )
{
    ExpandFile();
    if( IsAppend() )
        pStrm->Seek( STREAM_SEEK_TO_END );

    if( IsText() )
    {
        aLine += rBuf;
        // Raus damit, wenn das Ende ein LF ist, aber CRLF vorher
        // strippen, da der SvStrm ein CRLF anfuegt!
        sal_uInt16 nLineLen = aLine.Len();
        if( nLineLen && aLine.GetBuffer()[ --nLineLen ] == 0x0A )
        {
            aLine.Erase( nLineLen );
            if( nLineLen && aLine.GetBuffer()[ --nLineLen ] == 0x0D )
                aLine.Erase( nLineLen );
            pStrm->WriteLines( aLine );
            aLine.Erase();
        }
    }
    else
    {
        if( !n ) n = nLen;
        if( !n )
            return nError = SbERR_BAD_RECORD_LENGTH;
        pStrm->Write( rBuf.GetBuffer(), n );
        MapError();
    }
    return nError;
}

//////////////////////////////////////////////////////////////////////////

// Zugriff auf das aktuelle I/O-System:

SbiIoSystem* SbGetIoSystem()
{
    SbiInstance* pInst = pINST;
    return pInst ? pInst->GetIoSystem() : NULL;
}

//////////////////////////////////////////////////////////////////////////

SbiIoSystem::SbiIoSystem()
{
    for( short i = 0; i < CHANNELS; i++ )
        pChan[ i ] = NULL;
    nChan  = 0;
    nError = 0;
}

SbiIoSystem::~SbiIoSystem()
{
    Shutdown();
}

SbError SbiIoSystem::GetError()
{
    SbError n = nError; nError = 0;
    return n;
}

void SbiIoSystem::Open
    ( short nCh, const ByteString& rName, short nMode, short nFlags, short nLen )
{
    nError = 0;
    if( nCh >= CHANNELS || !nCh )
        nError = SbERR_BAD_CHANNEL;
    else if( pChan[ nCh ] )
        nError = SbERR_FILE_ALREADY_OPEN;
    else
    {
        pChan[ nCh ] = new SbiStream;
        nError = pChan[ nCh ]->Open( nCh, rName, nMode, nFlags, nLen );
        if( nError )
            delete pChan[ nCh ], pChan[ nCh ] = NULL;
    }
    nChan = 0;
}

// Aktuellen Kanal schliessen

void SbiIoSystem::Close()
{
    if( !nChan )
        nError = SbERR_BAD_CHANNEL;
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
    {
        nError = pChan[ nChan ]->Close();
        delete pChan[ nChan ];
        pChan[ nChan ] = NULL;
    }
    nChan = 0;
}

// Shutdown nach Programmlauf

void SbiIoSystem::Shutdown()
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( pChan[ i ] )
        {
            SbError n = pChan[ i ]->Close();
            delete pChan[ i ];
            pChan[ i ] = NULL;
            if( n && !nError )
                nError = n;
        }
    }
    nChan = 0;
    // Noch was zu PRINTen?
    if( aOut.Len() )
    {
        String aOutStr( aOut, gsl_getSystemTextEncoding() );
#if defined GCC
        Window* pParent = Application::GetDefDialogParent();
        MessBox( pParent, WinBits( WB_OK ), String(), aOutStr ).Execute();
#else
        MessBox( GetpApp()->GetDefDialogParent(), WinBits( WB_OK ), String(), aOutStr ).Execute();
#endif
    }
    aOut.Erase();
}

// Aus aktuellem Kanal lesen

void SbiIoSystem::Read( ByteString& rBuf, short n )
{
    if( !nChan )
        ReadCon( rBuf );
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
        nError = pChan[ nChan ]->Read( rBuf, n );
}

char SbiIoSystem::Read()
{
    char ch = ' ';
    if( !nChan )
    {
        if( !aIn.Len() )
        {
            ReadCon( aIn );
            aIn += '\n';
        }
        ch = aIn.GetBuffer()[0];
        aIn.Erase( 0, 1 );
    }
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
        nError = pChan[ nChan ]->Read( ch );
    return ch;
}

void SbiIoSystem::Write( const ByteString& rBuf, short n )
{
    if( !nChan )
        WriteCon( rBuf );
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
        nError = pChan[ nChan ]->Write( rBuf, n );
}

short SbiIoSystem::NextChannel()
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( !pChan[ i ] )
            return i;
    }
    nError = SbERR_TOO_MANY_FILES;
    return CHANNELS;
}

// nChannel == 0..CHANNELS-1

SbiStream* SbiIoSystem::GetStream( short nChannel ) const
{
    SbiStream* pRet = 0;
    if( nChannel >= 0 && nChannel < CHANNELS )
        pRet = pChan[ nChannel ];
    return pRet;
}

void SbiIoSystem::CloseAll(void)
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( pChan[ i ] )
        {
            SbError n = pChan[ i ]->Close();
            delete pChan[ i ];
            pChan[ i ] = NULL;
            if( n && !nError )
                nError = n;
        }
    }
}

/***************************************************************************
*
*   Console Support
*
***************************************************************************/

// Einlesen einer Zeile von der Console

void SbiIoSystem::ReadCon( ByteString& rIn )
{
    String aPromptStr( aPrompt, gsl_getSystemTextEncoding() );
    SbiInputDialog aDlg( NULL, aPromptStr );
    if( aDlg.Execute() )
        rIn = ByteString( aDlg.GetInput(), gsl_getSystemTextEncoding() );
    else
        nError = SbERR_USER_ABORT;
    aPrompt.Erase();
}

// Ausgabe einer MessageBox, wenn im Console-Puffer ein CR ist

void SbiIoSystem::WriteCon( const ByteString& rText )
{
    aOut += rText;
    sal_uInt16 n1 = aOut.Search( '\n' );
    sal_uInt16 n2 = aOut.Search( '\r' );
    if( n1 != STRING_NOTFOUND || n2 != STRING_NOTFOUND )
    {
        if( n1 == STRING_NOTFOUND ) n1 = n2;
        else
        if( n2 == STRING_NOTFOUND ) n2 = n1;
        if( n1 > n2 ) n1 = n2;
        ByteString s( aOut.Copy( 0, n1 ) );
        aOut.Erase( 0, n1 );
        while( aOut.GetBuffer()[0] == '\n' || aOut.GetBuffer()[0] == '\r' )
            aOut.Erase( 0, 1 );
        String aStr( s, gsl_getSystemTextEncoding() );
        {
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            if( !MessBox( GetpApp()->GetDefDialogParent(),
                        WinBits( WB_OK_CANCEL | WB_DEF_OK ),
                        String(), aStr ).Execute() )
                nError = SbERR_USER_ABORT;
        }
    }
}

