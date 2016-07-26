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

#include <string.h>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <osl/mutex.hxx>

#include "runtime.hxx"

#include <ctype.h>
#include <rtl/byteseq.hxx>
#include <rtl/textenc.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;
using namespace com::sun::star::bridge;

#include "iosys.hxx"
#include "sbintern.hxx"


class SbiInputDialog : public ModalDialog {
    VclPtr<Edit> aInput;
    VclPtr<OKButton> aOk;
    VclPtr<CancelButton> aCancel;
    OUString aText;
    DECL_LINK_TYPED( Ok, Button *, void );
    DECL_LINK_TYPED( Cancel, Button *, void );
public:
    SbiInputDialog( vcl::Window*, const OUString& );
    virtual ~SbiInputDialog() { disposeOnce(); }
    virtual void dispose() override;
    const OUString& GetInput() { return aText; }
};

SbiInputDialog::SbiInputDialog( vcl::Window* pParent, const OUString& rPrompt )
            :ModalDialog( pParent, WB_3DLOOK | WB_MOVEABLE | WB_CLOSEABLE ),
             aInput( VclPtr<Edit>::Create(this, WB_3DLOOK | WB_LEFT | WB_BORDER) ),
             aOk( VclPtr<OKButton>::Create(this) ), aCancel( VclPtr<CancelButton>::Create(this) )
{
    SetText( rPrompt );
    aOk->SetClickHdl( LINK( this, SbiInputDialog, Ok ) );
    aCancel->SetClickHdl( LINK( this, SbiInputDialog, Cancel ) );
    SetMapMode( MapMode( MAP_APPFONT ) );

    Point aPt = LogicToPixel( Point( 50, 50 ) );
    Size  aSz = LogicToPixel( Size( 145, 65 ) );
    SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 10, 10 ) );
    aSz = LogicToPixel( Size( 120, 12 ) );
    aInput->SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 15, 30 ) );
    aSz = LogicToPixel( Size( 45, 15) );
    aOk->SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 80, 30 ) );
    aSz = LogicToPixel( Size( 45, 15) );
    aCancel->SetPosSizePixel( aPt, aSz );

    aInput->Show();
    aOk->Show();
    aCancel->Show();
}

void SbiInputDialog::dispose()
{
    aInput.disposeAndClear();
    aOk.disposeAndClear();
    aCancel.disposeAndClear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED( SbiInputDialog, Ok, Button *, void )
{
    aText = aInput->GetText();
    EndDialog( 1 );
}

IMPL_LINK_NOARG_TYPED( SbiInputDialog, Cancel, Button *, void )
{
    EndDialog();
}

SbiStream::SbiStream()
    : pStrm(nullptr)
    , nExpandOnWriteTo(0)
    , nLine(0)
    , nLen(0)
    , nMode(SbiStreamFlags::NONE)
    , nChan(0)
    , nError(0)
{
}

SbiStream::~SbiStream()
{
    delete pStrm;
}

// map an SvStream-error to StarBASIC-code

void SbiStream::MapError()
{
    if( pStrm )
    {
        switch( pStrm->GetError() )
        {
        case SVSTREAM_OK:
            nError = 0;
            break;
        case SVSTREAM_FILE_NOT_FOUND:
            nError = ERRCODE_BASIC_FILE_NOT_FOUND;
            break;
        case SVSTREAM_PATH_NOT_FOUND:
            nError = ERRCODE_BASIC_PATH_NOT_FOUND;
            break;
        case SVSTREAM_TOO_MANY_OPEN_FILES:
            nError = ERRCODE_BASIC_TOO_MANY_FILES;
            break;
        case SVSTREAM_ACCESS_DENIED:
            nError = ERRCODE_BASIC_ACCESS_DENIED;
            break;
        case SVSTREAM_INVALID_PARAMETER:
            nError = ERRCODE_BASIC_BAD_ARGUMENT;
            break;
        case SVSTREAM_OUTOFMEMORY:
            nError = ERRCODE_BASIC_NO_MEMORY;
            break;
        default:
            nError = ERRCODE_BASIC_IO_ERROR;
            break;
        }
    }
}

// Returns sal_True if UNO is available, otherwise the old file
// system implementation has to be used
// #89378 New semantic: Don't just ask for UNO but for UCB
bool hasUno()
{
    static bool bNeedInit = true;
    static bool bRetVal = true;

    if( bNeedInit )
    {
        bNeedInit = false;
        Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
        if( !xContext.is() )
        {
            // No service manager at all
            bRetVal = false;
        }
        else
        {
            Reference< XUniversalContentBroker > xManager = UniversalContentBroker::create(xContext);

            if ( !( xManager->queryContentProvider( "file:///" ).is() ) )
            {
                // No UCB
                bRetVal = false;
            }
        }
    }
    return bRetVal;
}


class OslStream : public SvStream
{
    osl::File maFile;

public:
                        OslStream( const OUString& rName, StreamMode nStrmMode );
                       virtual ~OslStream();
    virtual sal_Size GetData( void* pData, sal_Size nSize ) override;
    virtual sal_Size PutData( const void* pData, sal_Size nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void        FlushData() override;
    virtual void        SetSize( sal_uInt64 nSize) override;
};

OslStream::OslStream( const OUString& rName, StreamMode nStrmMode )
    : maFile( rName )
{
    sal_uInt32 nFlags;

    if( (nStrmMode & (StreamMode::READ | StreamMode::WRITE)) == (StreamMode::READ | StreamMode::WRITE) )
    {
        nFlags = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;
    }
    else if( nStrmMode & StreamMode::WRITE )
    {
        nFlags = osl_File_OpenFlag_Write;
    }
    else //if( nStrmMode & StreamMode::READ )
    {
        nFlags = osl_File_OpenFlag_Read;
    }

    osl::FileBase::RC nRet = maFile.open( nFlags );
    if( nRet == osl::FileBase::E_NOENT && nFlags != osl_File_OpenFlag_Read )
    {
        nFlags |= osl_File_OpenFlag_Create;
        nRet = maFile.open( nFlags );
    }

    if( nRet != osl::FileBase::E_None )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}


OslStream::~OslStream()
{
    maFile.close();
}

sal_Size OslStream::GetData(void* pData, sal_Size nSize)
{
    sal_uInt64 nBytesRead = nSize;
    maFile.read( pData, nBytesRead, nBytesRead );
    return nBytesRead;
}

sal_Size OslStream::PutData(const void* pData, sal_Size nSize)
{
    sal_uInt64 nBytesWritten;
    maFile.write( pData, nSize, nBytesWritten );
    return nBytesWritten;
}

sal_uInt64 OslStream::SeekPos( sal_uInt64 nPos )
{
    ::osl::FileBase::RC rc = ::osl::FileBase::E_None;
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(nPos != SAL_MAX_UINT32);
    if( nPos == STREAM_SEEK_TO_END )
    {
        rc = maFile.setPos( osl_Pos_End, 0 );
    }
    else
    {
        rc = maFile.setPos( osl_Pos_Absolut, (sal_uInt64)nPos );
    }
    OSL_VERIFY(rc == ::osl::FileBase::E_None);
    sal_uInt64 nRealPos(0);
    maFile.getPos( nRealPos );
    return nRealPos;
}

void OslStream::FlushData()
{
}

void OslStream::SetSize( sal_uInt64 nSize )
{
    maFile.setSize( nSize );
}


class UCBStream : public SvStream
{
    Reference< XInputStream >   xIS;
    Reference< XStream >        xS;
    Reference< XSeekable >      xSeek;
public:
    explicit UCBStream( Reference< XInputStream > & xIS );
    explicit UCBStream( Reference< XStream > & xS );
                       virtual ~UCBStream();
    virtual sal_Size GetData( void* pData, sal_Size nSize ) override;
    virtual sal_Size PutData( const void* pData, sal_Size nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void        FlushData() override;
    virtual void        SetSize( sal_uInt64 nSize ) override;
};

UCBStream::UCBStream( Reference< XInputStream > & rStm )
    : xIS( rStm )
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
        {
            xIS->closeInput();
        }
        else if( xS.is() )
        {
            Reference< XInputStream > xIS_ = xS->getInputStream();
            if( xIS_.is() )
            {
                xIS_->closeInput();
            }
        }
    }
    catch(const Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}

sal_Size UCBStream::GetData(void* pData, sal_Size nSize)
{
    try
    {
        Reference< XInputStream > xISFromS;
        if( xIS.is() )
        {
            Sequence<sal_Int8> aData;
            nSize = xIS->readBytes( aData, nSize );
            memcpy( pData, aData.getConstArray(), nSize );
            return nSize;
        }
        else if( xS.is() && (xISFromS = xS->getInputStream()).is() )
        {
            Sequence<sal_Int8> aData;
            nSize = xISFromS->readBytes( aData, nSize );
            memcpy(pData, aData.getConstArray(), nSize );
            return nSize;
        }
        else
        {
            SetError( ERRCODE_IO_GENERAL );
        }
    }
    catch(const Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

sal_Size UCBStream::PutData(const void* pData, sal_Size nSize)
{
    try
    {
        Reference< XOutputStream > xOSFromS;
        if( xS.is() && (xOSFromS = xS->getOutputStream()).is() )
        {
            Sequence<sal_Int8> aData( static_cast<const sal_Int8 *>(pData), nSize );
            xOSFromS->writeBytes( aData );
            return nSize;
        }
        else
        {
            SetError( ERRCODE_IO_GENERAL );
        }
    }
    catch(const Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

sal_uInt64 UCBStream::SeekPos( sal_uInt64 nPos )
{
    try
    {
        if( xSeek.is() )
        {
            sal_uInt64 nLen = static_cast<sal_uInt64>( xSeek->getLength() );
            if( nPos > nLen )
            {
                nPos = nLen;
            }
            xSeek->seek( nPos );
            return nPos;
        }
        else
        {
            SetError( ERRCODE_IO_GENERAL );
        }
    }
    catch(const Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

void UCBStream::FlushData()
{
    try
    {
        Reference< XOutputStream > xOSFromS;
        if( xS.is() && (xOSFromS = xS->getOutputStream()).is() )
        {
            xOSFromS->flush();
        }
        else
        {
            SetError( ERRCODE_IO_GENERAL );
        }
    }
    catch(const Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}

void    UCBStream::SetSize( sal_uInt64 nSize )
{
    (void)nSize;

    SAL_WARN("basic", "UCBStream::SetSize not allowed to call from basic" );
    SetError( ERRCODE_IO_GENERAL );
}


SbError SbiStream::Open
( short nCh, const OString& rName, StreamMode nStrmMode, SbiStreamFlags nFlags, short nL )
{
    nMode   = nFlags;
    nLen    = nL;
    nChan   = nCh;
    nLine   = 0;
    nExpandOnWriteTo = 0;
    if( ( nStrmMode & ( StreamMode::READ|StreamMode::WRITE ) ) == StreamMode::READ )
    {
        nStrmMode |= StreamMode::NOCREATE;
    }
    OUString aStr(OStringToOUString(rName, osl_getThreadTextEncoding()));
    OUString aNameStr = getFullPath( aStr );

    if( hasUno() )
    {
        Reference< XSimpleFileAccess3 > xSFI( SimpleFileAccess::create( comphelper::getProcessComponentContext() ) );
        try
        {

        // #??? For write access delete file if it already exists (not for appending)
        if( (nStrmMode & StreamMode::WRITE) && !IsAppend() && !IsBinary() && !IsRandom() &&
            xSFI->exists( aNameStr ) && !xSFI->isFolder( aNameStr ) )
        {
            xSFI->kill( aNameStr );
        }

        if( (nStrmMode & (StreamMode::READ | StreamMode::WRITE)) == (StreamMode::READ | StreamMode::WRITE) )
        {
            Reference< XStream > xIS = xSFI->openFileReadWrite( aNameStr );
            pStrm = new UCBStream( xIS );
        }
        else if( nStrmMode & StreamMode::WRITE )
        {
            Reference< XStream > xIS = xSFI->openFileReadWrite( aNameStr );
            pStrm = new UCBStream( xIS );
        }
        else //if( nStrmMode & StreamMode::READ )
        {
            Reference< XInputStream > xIS = xSFI->openFileRead( aNameStr );
            pStrm = new UCBStream( xIS );
        }

        }
        catch(const Exception & )
        {
            nError = ERRCODE_IO_GENERAL;
        }
    }

    if( !pStrm )
    {
        pStrm = new OslStream( aNameStr, nStrmMode );
    }
    if( IsAppend() )
    {
        pStrm->Seek( STREAM_SEEK_TO_END );
    }
    MapError();
    if( nError )
    {
        delete pStrm;
        pStrm = nullptr;
    }
    return nError;
}

SbError SbiStream::Close()
{
    if( pStrm )
    {
        MapError();
        delete pStrm;
        pStrm = nullptr;
    }
    nChan = 0;
    return nError;
}

SbError SbiStream::Read(OString& rBuf, sal_uInt16 n, bool bForceReadingPerByte)
{
    nExpandOnWriteTo = 0;
    if( !bForceReadingPerByte && IsText() )
    {
        pStrm->ReadLine(rBuf);
        nLine++;
    }
    else
    {
        if( !n )
        {
            n = nLen;
        }
        if( !n )
        {
            return nError = ERRCODE_BASIC_BAD_RECORD_LENGTH;
        }
        OStringBuffer aBuffer(read_uInt8s_ToOString(*pStrm, n));
        //Pad it out with ' ' to the requested length on short read
        sal_Int32 nRequested = sal::static_int_cast<sal_Int32>(n);
        comphelper::string::padToLength(aBuffer, nRequested, ' ');
        rBuf = aBuffer.makeStringAndClear();
    }
    MapError();
    if( !nError && pStrm->IsEof() )
    {
        nError = ERRCODE_BASIC_READ_PAST_EOF;
    }
    return nError;
}

SbError SbiStream::Read( char& ch )
{
    nExpandOnWriteTo = 0;
    if (aLine.isEmpty())
    {
        Read( aLine );
        aLine = aLine + OString('\n');
    }
    ch = aLine[0];
    aLine = aLine.copy(1);
    return nError;
}

void SbiStream::ExpandFile()
{
    if ( nExpandOnWriteTo )
    {
        sal_uInt64 nCur = pStrm->Seek(STREAM_SEEK_TO_END);
        if( nCur < nExpandOnWriteTo )
        {
            sal_uInt64 nDiff = nExpandOnWriteTo - nCur;
            char c = 0;
            while( nDiff-- )
            {
                pStrm->WriteChar( c );
            }
        }
        else
        {
            pStrm->Seek( nExpandOnWriteTo );
        }
        nExpandOnWriteTo = 0;
    }
}

namespace
{
    void WriteLines(SvStream &rStream, const OString& rStr)
    {
        OString aStr(convertLineEnd(rStr, rStream.GetLineDelimiter()) );
        write_uInt8s_FromOString(rStream, aStr);
        endl( rStream );
    }
}

SbError SbiStream::Write( const OString& rBuf )
{
    ExpandFile();
    if( IsAppend() )
    {
        pStrm->Seek( STREAM_SEEK_TO_END );
    }
    if( IsText() )
    {
        aLine = aLine + rBuf;
        // Get it out, if the end is an LF, but strip CRLF before,
        // because the SvStream adds a CRLF!
        sal_Int32 nLineLen = aLine.getLength();
        if (nLineLen && aLine[--nLineLen] == 0x0A)
        {
            aLine = aLine.copy(0, nLineLen);
            if (nLineLen && aLine[--nLineLen] == 0x0D)
            {
                aLine = aLine.copy(0, nLineLen);
            }
            WriteLines(*pStrm, aLine);
            aLine.clear();
        }
    }
    else
    {
        if( !nLen )
        {
            return nError = ERRCODE_BASIC_BAD_RECORD_LENGTH;
        }
        pStrm->Write(rBuf.getStr(), nLen);
        MapError();
    }
    return nError;
}


SbiIoSystem::SbiIoSystem()
{
    for(SbiStream* & i : pChan)
    {
        i = nullptr;
    }
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

void SbiIoSystem::Open(short nCh, const OString& rName, StreamMode nMode, SbiStreamFlags nFlags, short nLen)
{
    nError = 0;
    if( nCh >= CHANNELS || !nCh )
    {
        nError = ERRCODE_BASIC_BAD_CHANNEL;
    }
    else if( pChan[ nCh ] )
    {
        nError = ERRCODE_BASIC_FILE_ALREADY_OPEN;
    }
    else
    {
        pChan[ nCh ] = new SbiStream;
        nError = pChan[ nCh ]->Open( nCh, rName, nMode, nFlags, nLen );
       if( nError )
       {
            delete pChan[ nCh ];
            pChan[ nCh ] = nullptr;
       }
    }
    nChan = 0;
}


void SbiIoSystem::Close()
{
    if( !nChan )
    {
        nError = ERRCODE_BASIC_BAD_CHANNEL;
    }
    else if( !pChan[ nChan ] )
    {
        nError = ERRCODE_BASIC_BAD_CHANNEL;
    }
    else
    {
        nError = pChan[ nChan ]->Close();
        delete pChan[ nChan ];
        pChan[ nChan ] = nullptr;
    }
    nChan = 0;
}


void SbiIoSystem::Shutdown()
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( pChan[ i ] )
        {
            SbError n = pChan[ i ]->Close();
            delete pChan[ i ];
            pChan[ i ] = nullptr;
            if( n && !nError )
            {
                nError = n;
            }
        }
    }
    nChan = 0;
    // anything left to PRINT?
    if( !aOut.isEmpty() )
    {
#if defined __GNUC__
        vcl::Window* pParent = Application::GetDefDialogParent();
        ScopedVclPtrInstance<MessBox>( pParent, WinBits( WB_OK ), OUString(), aOut )->Execute();
#else
        ScopedVclPtrInstance<MessBox>( GetpApp()->GetDefDialogParent(), WinBits( WB_OK ), OUString(), aOut )->Execute();
#endif
    }
    aOut.clear();
}


void SbiIoSystem::Read(OString& rBuf)
{
    if( !nChan )
    {
        ReadCon( rBuf );
    }
    else if( !pChan[ nChan ] )
    {
        nError = ERRCODE_BASIC_BAD_CHANNEL;
    }
    else
    {
        nError = pChan[ nChan ]->Read( rBuf );
    }
}

char SbiIoSystem::Read()
{
    char ch = ' ';
    if( !nChan )
    {
        if( aIn.isEmpty() )
        {
            ReadCon( aIn );
            aIn = aIn + OString('\n');
        }
        ch = aIn[0];
        aIn = aIn.copy(1);
    }
    else if( !pChan[ nChan ] )
    {
        nError = ERRCODE_BASIC_BAD_CHANNEL;
    }
    else
    {
        nError = pChan[ nChan ]->Read( ch );
    }
    return ch;
}

void SbiIoSystem::Write(const OUString& rBuf)
{
    if( !nChan )
    {
        WriteCon( rBuf );
    }
    else if( !pChan[ nChan ] )
    {
        nError = ERRCODE_BASIC_BAD_CHANNEL;
    }
    else
    {
        nError = pChan[ nChan ]->Write( OUStringToOString(rBuf, osl_getThreadTextEncoding()) );
    }
}

// nChannel == 0..CHANNELS-1

SbiStream* SbiIoSystem::GetStream( short nChannel ) const
{
    SbiStream* pRet = nullptr;
    if( nChannel >= 0 && nChannel < CHANNELS )
    {
        pRet = pChan[ nChannel ];
    }
    return pRet;
}

void SbiIoSystem::CloseAll()
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( pChan[ i ] )
        {
            SbError n = pChan[ i ]->Close();
            delete pChan[ i ];
            pChan[ i ] = nullptr;
            if( n && !nError )
            {
                nError = n;
            }
        }
    }
}

/***************************************************************************
*
*   Console Support
*
***************************************************************************/


void SbiIoSystem::ReadCon(OString& rIn)
{
    OUString aPromptStr(OStringToOUString(aPrompt, osl_getThreadTextEncoding()));
    ScopedVclPtrInstance< SbiInputDialog > aDlg(nullptr, aPromptStr);
    if( aDlg->Execute() )
    {
        rIn = OUStringToOString(aDlg->GetInput(), osl_getThreadTextEncoding());
    }
    else
    {
        nError = ERRCODE_BASIC_USER_ABORT;
    }
    aPrompt.clear();
}

// output of a MessageBox, if theres a CR in the console-buffer

void SbiIoSystem::WriteCon(const OUString& rText)
{
    aOut += rText;
    sal_Int32 n1 = aOut.indexOf('\n');
    sal_Int32 n2 = aOut.indexOf('\r');
    if( n1 != -1 || n2 != -1 )
    {
        if( n1 == -1 )
        {
            n1 = n2;
        }
        else if( n2 == -1 )
        {
            n2 = n1;
        }
        if( n1 > n2 )
        {
            n1 = n2;
        }
        OUString s(aOut.copy(0, n1));
        aOut = aOut.copy(n1);
        while ( !aOut.isEmpty() && (aOut[0] == '\n' || aOut[0] == '\r') )
        {
            aOut = aOut.copy(1);
        }
        {
            SolarMutexGuard aSolarGuard;
            if( !ScopedVclPtrInstance<MessBox>(
                        Application::GetDefDialogParent(),
                        WinBits( WB_OK_CANCEL | WB_DEF_OK ),
                        OUString(), s)->Execute() )
            {
                nError = ERRCODE_BASIC_USER_ABORT;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
