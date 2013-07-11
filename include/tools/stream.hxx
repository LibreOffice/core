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
#ifndef _STREAM_HXX
#define _STREAM_HXX

#include <limits>
#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/lineend.hxx>
#include <tools/errinf.hxx>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <rtl/string.hxx>

class StreamData;

inline rtl_TextEncoding GetStoreCharSet( rtl_TextEncoding eEncoding )
{
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        return RTL_TEXTENCODING_MS_1252;
    else
        return eEncoding;
}

// StreamTypes

typedef sal_uInt16 StreamMode;

// read, write, create,... options
#define STREAM_READ                     0x0001  ///< allow read accesses
#define STREAM_WRITE                    0x0002  ///< allow write accesses
// file i/o
#define STREAM_NOCREATE                 0x0004  ///< 1 == Dont create file
#define STREAM_TRUNC                    0x0008  ///< Truncate _existing_ file to zero length
#define STREAM_COPY_ON_SYMLINK          0x0010  ///< copy-on-write for symlinks (UNX)

#define STREAM_READWRITEBITS            (STREAM_READ | STREAM_WRITE | \
                                         STREAM_NOCREATE | STREAM_TRUNC)

// sharing options
#define STREAM_SHARE_DENYNONE           0x0100
#define STREAM_SHARE_DENYREAD           0x0200  // overrides denynone
#define STREAM_SHARE_DENYWRITE          0x0400  // overrides denynone
#define STREAM_SHARE_DENYALL            0x0800  // overrides denyread,write,none

#define STREAM_SHAREBITS                (STREAM_SHARE_DENYNONE | STREAM_SHARE_DENYREAD |\
                                         STREAM_SHARE_DENYWRITE | STREAM_SHARE_DENYALL)

#define STREAM_READWRITE                (STREAM_READ | STREAM_WRITE)
#define STREAM_SHARE_DENYREADWRITE      (STREAM_SHARE_DENYREAD | STREAM_SHARE_DENYWRITE)

#define STREAM_STD_READ                 (STREAM_READ | STREAM_SHARE_DENYNONE | STREAM_NOCREATE)
#define STREAM_STD_WRITE                (STREAM_WRITE | STREAM_SHARE_DENYALL)
#define STREAM_STD_READWRITE            (STREAM_READWRITE | STREAM_SHARE_DENYALL)

#define STREAM_SEEK_TO_BEGIN            0L
#define STREAM_SEEK_TO_END              ULONG_MAX

#define NUMBERFORMAT_INT_BIGENDIAN      (sal_uInt16)0x0000
#define NUMBERFORMAT_INT_LITTLEENDIAN   (sal_uInt16)0xFFFF

#define COMPRESSMODE_NONE               (sal_uInt16)0x0000
#define COMPRESSMODE_ZBITMAP            (sal_uInt16)0x0001
#define COMPRESSMODE_NATIVE             (sal_uInt16)0x0010

#define ID_STREAM                       1
#define ID_FILESTREAM                   2
#define ID_MEMORYSTREAM                 3
#define ID_SHAREDMEMORYSTREAM           4
#define ID_STORAGESTREAM                5
#define ID_PERSISTSTREAM                6

class SvStream;

typedef SvStream& (*SvStrPtr)( SvStream& );

inline SvStream& operator<<( SvStream& rStr, SvStrPtr f );

// SvLockBytes

enum LockType {};

struct SvLockBytesStat
{
    sal_Size nSize;

    SvLockBytesStat() : nSize(0) {}
};

enum SvLockBytesStatFlag { SVSTATFLAG_DEFAULT };

class TOOLS_DLLPUBLIC SvLockBytes: public virtual SvRefBase
{
    SvStream * m_pStream;
    bool m_bOwner;
    bool m_bSync;

protected:
    void close();

public:
    TYPEINFO();

    SvLockBytes() : m_pStream(0), m_bOwner(false), m_bSync(false) {}

    SvLockBytes(SvStream * pTheStream, bool bTheOwner = false) :
        m_pStream(pTheStream), m_bOwner(bTheOwner), m_bSync(false) {}

    virtual ~SvLockBytes() { close(); }

    virtual const SvStream * GetStream() const { return m_pStream; }

    virtual void    SetSynchronMode(bool bTheSync = true) { m_bSync = bTheSync; }
    virtual bool    IsSynchronMode() const { return m_bSync; }

    virtual ErrCode ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                           sal_Size * pRead) const;
    virtual ErrCode WriteAt(sal_Size nPos, const void * pBuffer, sal_Size nCount,
                            sal_Size * pWritten);

    virtual ErrCode Flush() const;

    virtual ErrCode SetSize(sal_Size nSize);

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const;
};

SV_DECL_IMPL_REF(SvLockBytes);

// SvOpenLockBytes

class TOOLS_DLLPUBLIC SvOpenLockBytes: public SvLockBytes
{
public:
    TYPEINFO();

    SvOpenLockBytes() : SvLockBytes(0, false) {}
    SvOpenLockBytes(SvStream * pStream, bool bOwner):
        SvLockBytes(pStream, bOwner) {}

    virtual ErrCode FillAppend(const void * pBuffer, sal_Size nCount,
                               sal_Size * pWritten) = 0;

    virtual sal_Size Tell() const = 0;

    virtual sal_Size Seek(sal_Size nPos) = 0;

    virtual void    Terminate() = 0;
};

SV_DECL_IMPL_REF(SvOpenLockBytes);

// SvAsyncLockBytes

class SvAsyncLockBytes: public SvOpenLockBytes
{
    sal_Size m_nSize;
    bool m_bTerminated;

public:
    TYPEINFO();

    SvAsyncLockBytes(SvStream * pStream, bool bOwner):
        SvOpenLockBytes(pStream, bOwner), m_nSize(0), m_bTerminated(false) {}

    virtual ErrCode ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                           sal_Size * pRead) const;
    virtual ErrCode WriteAt(sal_Size nPos, const void * pBuffer, sal_Size nCount,
                            sal_Size * pWritten);

    virtual ErrCode FillAppend(const void * pBuffer, sal_Size nCount,
                               sal_Size * pWritten);

    virtual sal_Size Tell() const { return m_nSize; }

    virtual sal_Size Seek(sal_Size nPos);

    virtual void    Terminate() { m_bTerminated = true; }
};

SV_DECL_IMPL_REF(SvAsyncLockBytes);

// SvStream

class TOOLS_DLLPUBLIC SvStream
{
private:
    // LockBytes Interface
    void*           pImp;           // unused
    SvLockBytesRef  xLockBytes;  // Default implementation
    sal_Size        nActPos;

    // Puffer-Verwaltung
    sal_uInt8*      pRWBuf;         // Points to read/write buffer
    sal_uInt8*      pBufPos;        // pRWBuf + nBufActualPos
    sal_uInt16      nBufSize;       // Allocated size of buffer
    sal_uInt16      nBufActualLen;  // Length of used segment of puffer
                                // = nBufSize, if EOF did not occur
    sal_uInt16      nBufActualPos;  // current position in buffer (0..nBufSize-1)
    sal_uInt16      nBufFree;       // number of free slots in buffer to IO of type eIOMode
    bool            bIoRead;
    bool            bIoWrite;

    // Error codes, conversion, compression, ...
    bool            bIsDirty; // true: Stream != buffer content
    bool            bIsConsistent;// false: Buffer contains data, which were
                                // NOT allowed to be written by PutData
                                // into the derived stream (cf. PutBack)
    bool            bSwap;
    bool            bIsEof;
    sal_uInt32      nError;
    sal_uInt16      nNumberFormatInt;
    sal_uInt16      nCompressMode;
    LineEnd         eLineDelimiter;
    rtl_TextEncoding eStreamCharSet;

    // Encryption
    OString m_aCryptMaskKey;// aCryptMaskKey.getLength != 0  -> Encryption used
    unsigned char   nCryptMask;

    // Userdata
    long            nVersion;   // for external use

    // helper methods
    TOOLS_DLLPRIVATE void ImpInit();

                    SvStream ( const SvStream& rStream ); // not implemented
    SvStream&       operator=( const SvStream& rStream ); // not implemented

protected:
    sal_Size        nBufFilePos;///< File position of pBuf[0]
    sal_uInt16      eStreamMode;
    bool            bIsWritable;

    virtual sal_Size GetData( void* pData, sal_Size nSize );
    virtual sal_Size PutData( const void* pData, sal_Size nSize );
    virtual sal_Size SeekPos( sal_Size nPos );
    virtual void    FlushData();
    virtual void    SetSize( sal_Size nSize );

    void            ClearError();
    void            ClearBuffer();

    // encrypt and write in blocks
    sal_Size        CryptAndWriteBuffer( const void* pStart, sal_Size nLen );
    bool            EncryptBuffer( void* pStart, sal_Size nLen );

    void            SyncSvStream( sal_Size nNewStreamPos ); ///< SvStream <- Medium
    void            SyncSysStream(); ///< SvStream -> Medium

public:
                    SvStream();
                    SvStream( SvLockBytes *pLockBytes);
    virtual         ~SvStream();

    SvLockBytes*    GetLockBytes() const { return xLockBytes; }

    sal_uInt32      GetError() const { return ERRCODE_TOERROR(nError); }
    sal_uInt32      GetErrorCode() const { return nError; }

    void            SetError( sal_uInt32 nErrorCode );
    virtual void    ResetError();

    void            SetNumberFormatInt( sal_uInt16 nNewFormat );
    sal_uInt16      GetNumberFormatInt() const { return nNumberFormatInt; }
    /// Enable/disable swapping of endians, may be needed for Unicode import/export
    inline void     SetEndianSwap( bool bVal );
    /// returns status of endian swap flag
    bool            IsEndianSwap() const { return 0 != bSwap; }

    void            SetCompressMode( sal_uInt16 nNewMode )
                        { nCompressMode = nNewMode; }
    sal_uInt16      GetCompressMode() const { return nCompressMode; }

    void SetCryptMaskKey(const OString& rCryptMaskKey);
    const OString& GetCryptMaskKey() const { return m_aCryptMaskKey; }

    void            SetStreamCharSet( rtl_TextEncoding eCharSet )
                        { eStreamCharSet = eCharSet; }
    rtl_TextEncoding GetStreamCharSet() const { return eStreamCharSet; }

    void            SetLineDelimiter( LineEnd eLineEnd )
                        { eLineDelimiter = eLineEnd; }
    LineEnd         GetLineDelimiter() const { return eLineDelimiter; }

    SvStream&       operator>>( sal_uInt16& rUInt16 );
    SvStream&       operator>>( sal_uInt32& rUInt32 );
    SvStream&       operator>>( sal_uInt64& rUInt64 );
    SvStream&       operator>>( sal_Int16& rInt16 );
    SvStream&       operator>>( sal_Int32& rInt32 );
    SvStream&       operator>>( sal_Int64& rInt64 ) SAL_DELETED_FUNCTION;
    SvStream&       ReadInt64(sal_Int64 & rInt64);

    SvStream&       operator>>( signed char& rChar );
    SvStream&       operator>>( char& rChar );
    SvStream&       operator>>( unsigned char& rChar );
    SvStream&       operator>>( float& rFloat );
    SvStream&       operator>>( double& rDouble );
    SvStream&       operator>>( SvStream& rStream );

    SvStream&       operator<<( sal_uInt16 nUInt16 );
    SvStream&       operator<<( sal_uInt32 nUInt32 );
    SvStream&       operator<<( sal_uInt64 nuInt64 );
    SvStream&       operator<<( sal_Int16 nInt16 );
    SvStream&       operator<<( sal_Int32 nInt32 );
    SvStream&       operator<<( sal_Int64 nInt64 ) SAL_DELETED_FUNCTION;
    SvStream&       WriteInt64(sal_Int64 nInt64);

    SvStream&       operator<<( bool b )
                        { return operator<<(static_cast< sal_Bool >(b)); }
    SvStream&       operator<<( signed char nChar );
    SvStream&       operator<<( char nChar );
    SvStream&       operator<<( unsigned char nChar );
    SvStream&       operator<<( float nFloat );
    SvStream&       operator<<( const double& rDouble );
    SvStream&       operator<<( const char* pBuf );
    SvStream&       operator<<( const unsigned char* pBuf );
    SvStream&       operator<<( SvStream& rStream );

    SvStream&       WriteNumber( sal_uInt32 nUInt32 );
    SvStream&       WriteNumber( sal_Int32 nInt32 );

    sal_Size        Read( void* pData, sal_Size nSize );
    sal_Size        Write( const void* pData, sal_Size nSize );
    sal_Size        Seek( sal_Size nPos );
    sal_Size        SeekRel( sal_sSize nPos );
    sal_Size        Tell() const { return nBufFilePos+nBufActualPos;  }
    // length between current (Tell()) pos and end of stream
    virtual sal_Size remainingSize();
    void            Flush();
    bool            IsEof() const { return bIsEof; }
    // next Tell() <= nSize
    bool            SetStreamSize( sal_Size nSize );

    /** Read a line of bytes.

        @param nMaxBytesToRead
                   Maximum of bytes to read, if line is longer it will be
                   truncated.

        @note NOTE that the default is one character less than STRING_MAXLEN to
              prevent problems after conversion to String that may be lurking
              in various places doing something like
              @code
                for (sal_uInt16 i=0; i < aString.Len(); ++i)
              @endcode
              causing endless loops ...
    */
    bool            ReadLine( OString& rStr, sal_Int32 nMaxBytesToRead = 0xFFFE );
    bool            WriteLine( const OString& rStr );

    /** Read a line of bytes.

        @param nMaxBytesToRead
                   Maximum of bytes to read, if line is longer it will be
                   truncated.

        @note NOTE that the default is one character less than STRING_MAXLEN to
              prevent problems after conversion to String that may be lurking
              in various places doing something like
              @code
                for (sal_uInt16 i=0; i < aString.Len(); ++i)
              @endcode
              causing endless loops ...
    */
    bool            ReadByteStringLine( OUString& rStr, rtl_TextEncoding eSrcCharSet,
                                        sal_Int32 nMaxBytesToRead = 0xFFFE );
    bool            WriteByteStringLine( const OUString& rStr, rtl_TextEncoding eDestCharSet );

    /// Switch to no endian swapping and write 0xfeff
    bool        StartWritingUnicodeText();

    /** If eReadBomCharSet==RTL_TEXTENCODING_DONTKNOW: read 16bit, if 0xfeff do
        nothing (UTF-16), if 0xfffe switch endian swapping (UTF-16), if 0xefbb
        or 0xbbef read another byte and check for UTF-8. If no UTF-* BOM was
        detected put all read bytes back. This means that if 2 bytes were read
        it was an UTF-16 BOM, if 3 bytes were read it was an UTF-8 BOM. There
        is no UTF-7, UTF-32 or UTF-EBCDIC BOM detection!

        If eReadBomCharSet!=RTL_TEXTENCODING_DONTKNOW: only read a BOM of that
        encoding and switch endian swapping if UTF-16 and 0xfffe. */
    bool        StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet );

    /** Read a line of Unicode.

        @param nMaxCodepointsToRead
                   Maximum of codepoints (UCS-2 or UTF-16 pairs, not bytes) to
                   read, if line is longer it will be truncated.

        @note NOTE that the default is one character less than STRING_MAXLEN to
              prevent problems after conversion to String that may be lurking in
              various places doing something like
              @code
                for (sal_uInt16 i=0; i < aString.Len(); ++i)
              @endcode
              causing endless loops ...
    */
    bool            ReadUniStringLine( OUString& rStr, sal_Int32 nMaxCodepointsToRead = 0xFFFE );
    /** Read a 32bit length prefixed sequence of utf-16 if
        eSrcCharSet==RTL_TEXTENCODING_UNICODE, otherwise read a 16bit length
        prefixed sequence of bytes and convert from eSrcCharSet */
    OUString        ReadUniOrByteString(rtl_TextEncoding eSrcCharSet);
    /** Write a 32bit length prefixed sequence of utf-16 if
        eSrcCharSet==RTL_TEXTENCODING_UNICODE, otherwise convert to eSrcCharSet
        and write a 16bit length prefixed sequence of bytes */
    SvStream&       WriteUniOrByteString( const OUString& rStr, rtl_TextEncoding eDestCharSet );

    /** Read a line of Unicode if eSrcCharSet==RTL_TEXTENCODING_UNICODE,
        otherwise read a line of Bytecode and convert from eSrcCharSet

        @param nMaxCodepointsToRead
                   Maximum of codepoints (2 bytes if Unicode, bytes if not
                   Unicode) to read, if line is longer it will be truncated.

        @note NOTE that the default is one character less than STRING_MAXLEN to
              prevent problems after conversion to String that may be lurking in
              various places doing something like
              @code
                for (sal_uInt16 i=0; i < aString.Len(); ++i)
              @endcode
              causing endless loops ...
    */
    bool            ReadUniOrByteStringLine( OUString& rStr, rtl_TextEncoding eSrcCharSet,
                                             sal_Int32 nMaxCodepointsToRead = 0xFFFE );
    /** Write a sequence of Unicode characters if
        eDestCharSet==RTL_TEXTENCODING_UNICODE, otherwise write a sequence of
        Bytecodes converted to eDestCharSet */
    bool            WriteUnicodeOrByteText( const OUString& rStr, rtl_TextEncoding eDestCharSet );
    bool            WriteUnicodeOrByteText( const OUString& rStr )
                    { return WriteUnicodeOrByteText( rStr, GetStreamCharSet() ); }

    /** Write a Unicode character if eDestCharSet==RTL_TEXTENCODING_UNICODE,
        otherwise write as Bytecode converted to eDestCharSet.

        This may result in more than one byte being written if a multi byte
        encoding (e.g. UTF7, UTF8) is chosen. */
    bool            WriteUniOrByteChar( sal_Unicode ch, rtl_TextEncoding eDestCharSet );
    bool            WriteUniOrByteChar( sal_Unicode ch )
                    { return WriteUniOrByteChar( ch, GetStreamCharSet() ); }

    void            SetBufferSize( sal_uInt16 nBufSize );
    sal_uInt16      GetBufferSize() const { return nBufSize; }

    void            RefreshBuffer();
    SvStream&       PutBack( char aCh );

    bool            IsWritable() const { return bIsWritable; }
    StreamMode      GetStreamMode() const { return eStreamMode; }
    virtual sal_uInt16 IsA() const;

    long            GetVersion() { return nVersion; }
    void            SetVersion( long n ) { nVersion = n; }

    friend SvStream& operator<<( SvStream& rStr, SvStrPtr f ); // for Manips

    /// end of input seen during previous i/o operation
    bool            eof() const { return bIsEof; }

    /// stream is broken
    bool            bad() const { return GetError() != 0; }

    /** Get state

        If the state is good() the previous i/o operation succeeded.

        If the state is good(), the next input operation might succeed;
        otherwise, it will fail.

        Applying an input operation to a stream that is not in the good() state
        is a null operation as far as the variable being read into is concerned.

        If we try to read into a variable v and the operation fails, the value
        of v should be unchanged,
    */
    bool good() const { return !(eof() || bad()); }
};

inline SvStream& operator<<( SvStream& rStr, SvStrPtr f )
{
    (*f)(rStr);
    return rStr;
}

inline void SvStream::SetEndianSwap( bool bVal )
{
#ifdef OSL_BIGENDIAN
    SetNumberFormatInt( bVal ? NUMBERFORMAT_INT_LITTLEENDIAN : NUMBERFORMAT_INT_BIGENDIAN );
#else
    SetNumberFormatInt( bVal ? NUMBERFORMAT_INT_BIGENDIAN : NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
}

TOOLS_DLLPUBLIC SvStream& endl( SvStream& rStr );
/// same as endl() but Unicode
TOOLS_DLLPUBLIC SvStream& endlu( SvStream& rStr );
/// call endlu() if eStreamCharSet==RTL_TEXTECODING_UNICODE otherwise endl()
TOOLS_DLLPUBLIC SvStream& endlub( SvStream& rStr );

/// Attempt to read nUnits 8bit units to an OString, returned OString's
/// length is number of units successfully read
TOOLS_DLLPUBLIC OString read_uInt8s_ToOString(SvStream& rStrm,
    sal_Size nUnits);

/// Attempt to read nUnits 8bit units to an OUString
TOOLS_DLLPUBLIC inline OUString read_uInt8s_ToOUString(SvStream& rStrm,
    sal_Size nUnits, rtl_TextEncoding eEnc)
{
    return OStringToOUString(read_uInt8s_ToOString(rStrm, nUnits), eEnc);
}

/// Attempt to read nUnits 16bit units to an OUString, returned
/// OUString's length is number of units successfully read
TOOLS_DLLPUBLIC OUString read_uInt16s_ToOUString(SvStream& rStrm,
    sal_Size nUnits);

/// Attempt to read a pascal-style length (of type prefix) prefixed sequence of
/// 16bit units to an OUString, returned OString's length is number of
/// units successfully read.
template<typename prefix>
OUString read_lenPrefixed_uInt16s_ToOUString(SvStream& rStrm)
{
    prefix nUnits = 0;
    rStrm >> nUnits;
    return read_uInt16s_ToOUString(rStrm, nUnits);
}

/// Attempt to write a prefixed sequence of nUnits 16bit units from an OUString,
/// returned value is number of bytes written
TOOLS_DLLPUBLIC sal_Size write_uInt16s_FromOUString(SvStream& rStrm,
    const OUString& rStr, sal_Size nUnits);

TOOLS_DLLPUBLIC inline sal_Size write_uInt16s_FromOUString(SvStream& rStrm,
    const OUString& rStr)
{
    return write_uInt16s_FromOUString(rStrm, rStr, rStr.getLength());
}

namespace streamdetail
{
    /// Attempt to write a pascal-style length (of type prefix) prefixed
    /// sequence of units from a string-type, returned value is number of bytes
    /// written (including byte-count of prefix)
    template<typename prefix, typename S, sal_Size (*writeOper)(SvStream&, const S&, sal_Size)>
        sal_Size write_lenPrefixed_seq_From_str(SvStream& rStrm, const S &rStr)
    {
        sal_Size nWritten = 0;
        prefix nUnits = std::min<sal_Size>(rStr.getLength(), std::numeric_limits<prefix>::max());
        SAL_WARN_IF(static_cast<sal_Size>(nUnits) != static_cast<sal_Size>(rStr.getLength()),
            "tools.stream",
            "string too long for prefix count to fit in output type");
        rStrm << nUnits;
        if (rStrm.good())
        {
            nWritten += sizeof(prefix);
            nWritten += writeOper(rStrm, rStr, nUnits);
        }
        return nWritten;
    }
}

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 16bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
template<typename prefix>
sal_Size write_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                const OUString &rStr)
{
    return streamdetail::write_lenPrefixed_seq_From_str<prefix, OUString, write_uInt16s_FromOUString>(rStrm, rStr);
}

/// Attempt to read 8bit units to an OString until a zero terminator is
/// encountered, returned OString's length is number of units *definitely*
/// successfully read, check SvStream::good() to see if null terminator was
/// successfully read
TOOLS_DLLPUBLIC OString read_zeroTerminated_uInt8s_ToOString(SvStream& rStrm);

/// Attempt to read 8bit units assuming source encoding eEnc to an OUString
/// until a zero terminator is encountered. Check SvStream::good() to see if
/// null terminator was successfully read
TOOLS_DLLPUBLIC OUString read_zeroTerminated_uInt8s_ToOUString(SvStream& rStrm, rtl_TextEncoding eEnc);

/// Attempt to read a pascal-style length (of type prefix) prefixed sequence of
/// 8bit units to an OString, returned OString's length is number of units
/// successfully read.
template<typename prefix>
OString read_lenPrefixed_uInt8s_ToOString(SvStream& rStrm)
{
    prefix nUnits = 0;
    rStrm >> nUnits;
    return read_uInt8s_ToOString(rStrm, nUnits);
}

/// Attempt to read a pascal-style length (of type prefix) prefixed sequence of
/// 8bit units to an OUString
template<typename prefix>
OUString read_lenPrefixed_uInt8s_ToOUString(SvStream& rStrm,
                                            rtl_TextEncoding eEnc)
{
    return OStringToOUString(read_lenPrefixed_uInt8s_ToOString<prefix>(rStrm), eEnc);
}

/// Attempt to write a prefixed sequence of nUnits 8bit units from an OString,
/// returned value is number of bytes written
TOOLS_DLLPUBLIC inline sal_Size write_uInt8s_FromOString(SvStream& rStrm, const OString& rStr,
                                                         sal_Size nUnits)
{
    return rStrm.Write(rStr.getStr(), nUnits);
}

TOOLS_DLLPUBLIC inline sal_Size write_uInt8s_FromOString(SvStream& rStrm, const OString& rStr)
{
    return write_uInt8s_FromOString(rStrm, rStr, rStr.getLength());
}

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 8bit units from an OString, returned value is number of bytes written
/// (including byte-count of prefix)
template<typename prefix>
sal_Size write_lenPrefixed_uInt8s_FromOString(SvStream& rStrm,
                                              const OString &rStr)
{
    return streamdetail::write_lenPrefixed_seq_From_str<prefix, OString, write_uInt8s_FromOString>(rStrm, rStr);
}

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 8bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
template<typename prefix>
sal_Size write_lenPrefixed_uInt8s_FromOUString(SvStream& rStrm,
                                               const OUString &rStr,
                                               rtl_TextEncoding eEnc)
{
    return write_lenPrefixed_uInt8s_FromOString<prefix>(rStrm, OUStringToOString(rStr, eEnc));
}

// FileStream

class TOOLS_DLLPUBLIC SvFileStream : public SvStream
{
private:
    StreamData*     pInstanceData;
    OUString        aFilename;
    sal_uInt16      nLockCounter;
    bool            bIsOpen;
#ifdef UNX
    sal_uInt32      GetFileHandle() const;
#endif
    // Forbidden and not implemented.
    SvFileStream (const SvFileStream&);
    SvFileStream & operator= (const SvFileStream&);

    bool LockRange( sal_Size nByteOffset, sal_Size nBytes );
    bool UnlockRange( sal_Size nByteOffset, sal_Size nBytes );
    bool LockFile();
    bool UnlockFile();

protected:
    virtual sal_Size GetData( void* pData, sal_Size nSize );
    virtual sal_Size PutData( const void* pData, sal_Size nSize );
    virtual sal_Size SeekPos( sal_Size nPos );
    virtual void    SetSize( sal_Size nSize );
    virtual void    FlushData();

public:
                    // Switches to Read StreamMode on failed attempt of Write opening
                    SvFileStream( const OUString& rFileName, StreamMode eOpenMode );
                    SvFileStream();
                    ~SvFileStream();

    virtual void    ResetError();

    void            Open( const OUString& rFileName, StreamMode eOpenMode );
    void            Close();
    bool            IsOpen() const { return bIsOpen; }
    bool            IsLocked() const { return ( nLockCounter!=0 ); }
    virtual sal_uInt16 IsA() const;

    const OUString& GetFileName() const { return aFilename; }
};

// MemoryStream

class TOOLS_DLLPUBLIC SvMemoryStream : public SvStream
{
    // Forbidden and not implemented.
    SvMemoryStream (const SvMemoryStream&);
    SvMemoryStream & operator= (const SvMemoryStream&);

    sal_Size        GetBufSize() const { return nSize; }

protected:
    sal_Size        nSize;
    sal_Size        nResize;
    sal_Size        nPos;
    sal_Size        nEndOfData;
    sal_uInt8*      pBuf;
    bool            bOwnsData;

    virtual sal_Size GetData( void* pData, sal_Size nSize );
    virtual sal_Size PutData( const void* pData, sal_Size nSize );
    virtual sal_Size SeekPos( sal_Size nPos );
    virtual void    SetSize( sal_Size nSize );
    virtual void    FlushData();

    /// AllocateMemory must update pBuf accordingly
    /// - pBuf: Address of new block
    virtual bool    AllocateMemory( sal_Size nSize );

    /// ReAllocateMemory must update the following variables:
    /// - pBuf: Address of new block
    /// - nEndOfData: Set to nNewSize-1L , if outside of block
    ///               Set to 0 , if new block size is 0 bytes
    /// - nSize: New block size
    /// - nPos: Set to 0 if position outside of block
    virtual bool    ReAllocateMemory( long nDiff );

    /// Is called when this stream allocated the buffer or the buffer is
    /// resized. FreeMemory may need to NULLify handles in derived classes.
    virtual void    FreeMemory();

                    SvMemoryStream(void*) { } // for sub-classes

public:
                    SvMemoryStream( void* pBuf, sal_Size nSize, StreamMode eMode);
                    SvMemoryStream( sal_Size nInitSize=512, sal_Size nResize=64 );
                    ~SvMemoryStream();

    virtual void    ResetError();

    const void*    GetBuffer();
    sal_uIntPtr     GetSize();
    sal_Size        GetEndOfData() const { return nEndOfData; }
    const void*     GetData() { Flush(); return pBuf; }
    operator const  void*() { Flush(); return pBuf; }
    virtual sal_uInt16  IsA() const;

    void*           SwitchBuffer( sal_Size nInitSize=512, sal_Size nResize=64 );
    void*           SetBuffer( void* pBuf, sal_Size nSize,
                               bool bOwnsData=true, sal_Size nEOF=0 );

    void            ObjectOwnsMemory( bool bOwn ) { bOwnsData = bOwn; }
    bool            IsObjectMemoryOwner() { return bOwnsData; }
    void            SetResizeOffset( sal_Size nNewResize ) { nResize = nNewResize; }
    sal_Size        GetResizeOffset() const { return nResize; }
    virtual sal_Size remainingSize() { return GetBufSize() - Tell(); }
};

/** Data Copy Stream

    This class is the foundation for all classes, using SvData
    (SO2\DTRANS.HXX/CXX) for transportation (e.g., graphics).
*/
class TOOLS_DLLPUBLIC SvDataCopyStream
{
public:
    // repeated execution of Load or Assign is allowed
                    TYPEINFO();
    virtual         ~SvDataCopyStream(){}
    virtual void    Load( SvStream & ) = 0;
    virtual void    Save( SvStream & ) = 0;
    virtual void    Assign( const SvDataCopyStream & );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
