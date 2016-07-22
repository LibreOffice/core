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
#ifndef INCLUDED_TOOLS_STREAM_HXX
#define INCLUDED_TOOLS_STREAM_HXX

#include <limits>
#include <osl/process.h>
#include <tools/toolsdllapi.h>
#include <tools/lineend.hxx>
#include <tools/errinf.hxx>
#include <tools/ref.hxx>
#include <rtl/string.hxx>
#include <o3tl/typed_flags_set.hxx>

class StreamData;

inline rtl_TextEncoding GetStoreCharSet( rtl_TextEncoding eEncoding )
{
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        return RTL_TEXTENCODING_MS_1252;
    else
        return eEncoding;
}

// StreamTypes

// read, write, create,... options
enum class StreamMode {
    NONE                     = 0x0000,
    READ                     = 0x0001,  ///< allow read accesses
    WRITE                    = 0x0002,  ///< allow write accesses
// file i/o
    NOCREATE                 = 0x0004,  ///< 1 == Don't create file
    TRUNC                    = 0x0008,  ///< Truncate _existing_ file to zero length
    COPY_ON_SYMLINK          = 0x0010,  ///< copy-on-write for symlinks (Unix)
// sharing options
    SHARE_DENYNONE           = 0x0100,
    SHARE_DENYREAD           = 0x0200,  // overrides denynone
    SHARE_DENYWRITE          = 0x0400,  // overrides denynone
    SHARE_DENYALL            = 0x0800,  // overrides denyread,write,none
};
namespace o3tl
{
    template<> struct typed_flags<StreamMode> : is_typed_flags<StreamMode, 0x0f1f> {};
}

#define STREAM_READWRITE                (StreamMode::READ | StreamMode::WRITE)
#define STREAM_SHARE_DENYREADWRITE      (StreamMode::SHARE_DENYREAD | StreamMode::SHARE_DENYWRITE)

#define STREAM_STD_READ                 (StreamMode::READ | StreamMode::SHARE_DENYNONE | StreamMode::NOCREATE)
#define STREAM_STD_WRITE                (StreamMode::WRITE | StreamMode::SHARE_DENYALL)
#define STREAM_STD_READWRITE            (STREAM_READWRITE | StreamMode::SHARE_DENYALL)

#define STREAM_SEEK_TO_BEGIN            0L
#define STREAM_SEEK_TO_END              SAL_MAX_UINT64

enum class SvStreamEndian { BIG, LITTLE };

enum class SvStreamCompressFlags {
    NONE     = 0x0000,
    ZBITMAP  = 0x0001,
    NATIVE   = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<SvStreamCompressFlags> : is_typed_flags<SvStreamCompressFlags, 0x0011> {};
}

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

    SvLockBytes() : m_pStream(nullptr), m_bOwner(false), m_bSync(false) {}

    SvLockBytes(SvStream * pTheStream, bool bTheOwner = false) :
        m_pStream(pTheStream), m_bOwner(bTheOwner), m_bSync(false) {}

    virtual ~SvLockBytes() { close(); }

    const SvStream * GetStream() const { return m_pStream; }

    virtual void    SetSynchronMode(bool bTheSync = true) { m_bSync = bTheSync; }
    bool            IsSynchronMode() const { return m_bSync; }

    virtual ErrCode ReadAt(sal_uInt64 nPos, void * pBuffer, sal_Size nCount,
                           sal_Size * pRead) const;
    virtual ErrCode WriteAt(sal_uInt64 nPos, const void * pBuffer, sal_Size nCount,
                            sal_Size * pWritten);

    virtual ErrCode Flush() const;

    virtual ErrCode SetSize(sal_uInt64 nSize);

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const;
};

typedef tools::SvRef<SvLockBytes> SvLockBytesRef;

// SvStream

class TOOLS_DLLPUBLIC SvStream
{
private:
    // LockBytes Interface
    SvLockBytesRef  m_xLockBytes; ///< Default implementation
    sal_uInt64      m_nActPos;

    // buffer management
    sal_uInt8*      m_pRWBuf;     ///< Points to read/write buffer
    sal_uInt8*      m_pBufPos;    ///< m_pRWBuf + m_nBufActualPos
    sal_uInt16      m_nBufSize;   ///< Allocated size of buffer
    sal_uInt16      m_nBufActualLen; ///< Length of used segment of puffer
                                  ///< = m_nBufSize, if EOF did not occur
    sal_uInt16      m_nBufActualPos; ///< current position in buffer (0..m_nBufSize-1)
    sal_uInt16      m_nBufFree;   ///< number of free slots in buffer to IO of type eIOMode
    bool            m_isIoRead;
    bool            m_isIoWrite;

    // Error codes, conversion, compression, ...
    bool            m_isDirty;  ///< true: Stream != buffer content
    bool            m_isConsistent; ///< false: Buffer contains data, which were
                                ///< NOT allowed to be written by PutData
                                ///< into the derived stream (cf. PutBack)
    bool            m_isSwap;
    bool            m_isEof;
    sal_uInt32      m_nError;
    SvStreamEndian  m_nEndian;
    SvStreamCompressFlags m_nCompressMode;
    LineEnd         m_eLineDelimiter;
    rtl_TextEncoding m_eStreamCharSet;

    // Encryption
    OString m_aCryptMaskKey;// aCryptMaskKey.getLength != 0  -> Encryption used
    unsigned char   m_nCryptMask;

    // Userdata
    long            m_nVersion;   // for external use

    // helper methods
    TOOLS_DLLPRIVATE void ImpInit();

                    SvStream ( const SvStream& rStream ) = delete;
    SvStream&       operator=( const SvStream& rStream ) = delete;

protected:
    sal_uInt64      m_nBufFilePos; ///< File position of pBuf[0]
    StreamMode      m_eStreamMode;
    bool            m_isWritable;

    virtual sal_Size GetData( void* pData, sal_Size nSize );
    virtual sal_Size PutData( const void* pData, sal_Size nSize );
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos );
    virtual void    FlushData();
    virtual void    SetSize(sal_uInt64 nSize);

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

    SvLockBytes*    GetLockBytes() const { return m_xLockBytes; }

    sal_uInt32      GetError() const { return ERRCODE_TOERROR(m_nError); }
    sal_uInt32      GetErrorCode() const { return m_nError; }

    void            SetError( sal_uInt32 nErrorCode );
    virtual void    ResetError();

    void            SetEndian( SvStreamEndian SvStreamEndian );
    SvStreamEndian  GetEndian() const { return m_nEndian; }
    /// returns status of endian swap flag
    bool            IsEndianSwap() const { return m_isSwap; }

    void            SetCompressMode( SvStreamCompressFlags nNewMode )
                        { m_nCompressMode = nNewMode; }
    SvStreamCompressFlags GetCompressMode() const { return m_nCompressMode; }

    void SetCryptMaskKey(const OString& rCryptMaskKey);

    void            SetStreamCharSet( rtl_TextEncoding eCharSet )
                        { m_eStreamCharSet = eCharSet; }
    rtl_TextEncoding GetStreamCharSet() const { return m_eStreamCharSet; }

    void            SetLineDelimiter( LineEnd eLineEnd )
                        { m_eLineDelimiter = eLineEnd; }
    LineEnd         GetLineDelimiter() const { return m_eLineDelimiter; }

    SvStream&       ReadUInt16( sal_uInt16& rUInt16 );
    SvStream&       ReadUInt32( sal_uInt32& rUInt32 );
    SvStream&       ReadUInt64( sal_uInt64& rUInt64 );
    SvStream&       ReadInt16( sal_Int16& rInt16 );
    SvStream&       ReadInt32( sal_Int32& rInt32 );
    SvStream&       ReadInt64(sal_Int64 & rInt64);
    SvStream&       ReadSChar( signed char& rChar );
    SvStream&       ReadChar( char& rChar );
    SvStream&       ReadUChar( unsigned char& rChar );
    SvStream&       ReadUtf16( sal_Unicode& rUtf16 );
    SvStream&       ReadCharAsBool( bool& rBool );
    SvStream&       ReadFloat( float& rFloat );
    SvStream&       ReadDouble( double& rDouble );
    SvStream&       ReadStream( SvStream& rStream );

    SvStream&       WriteUInt16( sal_uInt16 nUInt16 );
    SvStream&       WriteUInt32( sal_uInt32 nUInt32 );
    SvStream&       WriteUInt64( sal_uInt64 nuInt64 );
    SvStream&       WriteInt16( sal_Int16 nInt16 );
    SvStream&       WriteInt32( sal_Int32 nInt32 );
    SvStream&       WriteInt64( sal_Int64 nInt64 );
    SvStream&       WriteUInt8( sal_uInt8 nuInt8 );
    SvStream&       WriteUnicode( sal_Unicode );
    SvStream&       WriteOString(const OString& rStr)
                        { return WriteCharPtr(rStr.getStr()); }
    SvStream&       WriteStream( SvStream& rStream );

    SvStream&       WriteBool( bool b )
                        { return WriteUChar(static_cast<unsigned char>(b)); }
    SvStream&       WriteSChar( signed char nChar );
    SvStream&       WriteChar( char nChar );
    SvStream&       WriteUChar( unsigned char nChar );
    SvStream&       WriteFloat( float nFloat );
    SvStream&       WriteDouble( const double& rDouble );
    SvStream&       WriteCharPtr( const char* pBuf );

    SvStream&       WriteUInt32AsString( sal_uInt32 nUInt32 );
    SvStream&       WriteInt32AsString( sal_Int32 nInt32 );

    sal_Size        ReadBytes( void* pData, sal_Size nSize );
    sal_Size        WriteBytes( const void* pData, sal_Size nSize );
    sal_uInt64      Seek( sal_uInt64 nPos );
    sal_uInt64      SeekRel( sal_Int64 nPos );
    sal_uInt64      Tell() const { return m_nBufFilePos + m_nBufActualPos;  }
    // length between current (Tell()) pos and end of stream
    virtual sal_uInt64 remainingSize();
    void            Flush();
    bool            IsEof() const { return m_isEof; }
    // next Tell() <= nSize
    bool            SetStreamSize( sal_uInt64 nSize );

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
    virtual bool    ReadLine( OString& rStr, sal_Int32 nMaxBytesToRead = 0xFFFE );
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
    void            StartWritingUnicodeText();

    /** If eReadBomCharSet==RTL_TEXTENCODING_DONTKNOW: read 16bit, if 0xfeff do
        nothing (UTF-16), if 0xfffe switch endian swapping (UTF-16), if 0xefbb
        or 0xbbef read another byte and check for UTF-8. If no UTF-* BOM was
        detected put all read bytes back. This means that if 2 bytes were read
        it was an UTF-16 BOM, if 3 bytes were read it was an UTF-8 BOM. There
        is no UTF-7, UTF-32 or UTF-EBCDIC BOM detection!

        If eReadBomCharSet!=RTL_TEXTENCODING_DONTKNOW: only read a BOM of that
        encoding and switch endian swapping if UTF-16 and 0xfffe. */
    void            StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet );

    /** Read a line of Unicode.

        @param nMaxCodepointsToRead
                   Maximum of codepoints (UCS-2 or UTF-16 pairs, not bytes) to
                   read, if line is longer it will be truncated.
    */
    bool            ReadUniStringLine(OUString& rStr, sal_Int32 nMaxCodepointsToRead);
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

    void            SetBufferSize( sal_uInt16 m_nBufSize );
    sal_uInt16      GetBufferSize() const { return m_nBufSize; }

    void            RefreshBuffer();

    bool            IsWritable() const { return m_isWritable; }
    StreamMode      GetStreamMode() const { return m_eStreamMode; }

    long            GetVersion() { return m_nVersion; }
    void            SetVersion( long n ) { m_nVersion = n; }

    friend SvStream& operator<<( SvStream& rStr, SvStrPtr f ); // for Manips

    /// end of input seen during previous i/o operation
    bool eof() const { return m_isEof; }

    /// stream is broken
    bool bad() const { return GetError() != 0; }

    /** Get state

        If the state is good() the previous i/o operation succeeded.

        If the state is good(), the next input operation might succeed;
        otherwise, it will fail.

        Applying an input operation to a stream that is not in the good() state
        is a null operation as far as the variable being read into is concerned.

        If we try to read into a variable v and the operation fails, the value
        of v should be unchanged,
    */
    virtual bool good() const { return !(eof() || bad()); }
};

inline SvStream& operator<<( SvStream& rStr, SvStrPtr f )
{
    (*f)(rStr);
    return rStr;
}

TOOLS_DLLPUBLIC SvStream& endl( SvStream& rStr );
/// same as endl() but Unicode
TOOLS_DLLPUBLIC SvStream& endlu( SvStream& rStr );
/// call endlu() if m_eStreamCharSet==RTL_TEXTECODING_UNICODE otherwise endl()
TOOLS_DLLPUBLIC SvStream& endlub( SvStream& rStr );

/// Attempt to read nUnits 8bit units to an OString, returned OString's
/// length is number of units successfully read
TOOLS_DLLPUBLIC OString read_uInt8s_ToOString(SvStream& rStrm,
    sal_Size nUnits);

/// Attempt to read nUnits 8bit units to an OUString
inline OUString read_uInt8s_ToOUString(SvStream& rStrm,
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
inline OUString read_uInt16_lenPrefixed_uInt16s_ToOUString(SvStream& rStrm)
{
    sal_uInt16 nUnits = 0;
    rStrm.ReadUInt16( nUnits );
    return read_uInt16s_ToOUString(rStrm, nUnits);
}

inline OUString read_uInt32_lenPrefixed_uInt16s_ToOUString(SvStream& rStrm)
{
    sal_uInt32 nUnits = 0;
    rStrm.ReadUInt32( nUnits );
    return read_uInt16s_ToOUString(rStrm, nUnits);
}

/// Attempt to write a prefixed sequence of nUnits 16bit units from an OUString,
/// returned value is number of bytes written
TOOLS_DLLPUBLIC sal_Size write_uInt16s_FromOUString(SvStream& rStrm,
    const OUString& rStr, sal_Size nUnits);

inline sal_Size write_uInt16s_FromOUString(SvStream& rStrm,
    const OUString& rStr)
{
    return write_uInt16s_FromOUString(rStrm, rStr, rStr.getLength());
}

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 16bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
TOOLS_DLLPUBLIC sal_Size write_uInt32_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                const OUString &rStr);

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 16bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
TOOLS_DLLPUBLIC sal_Size write_uInt16_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                const OUString &rStr);

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
inline OString read_uInt16_lenPrefixed_uInt8s_ToOString(SvStream& rStrm)
{
    sal_uInt16 nUnits = 0;
    rStrm.ReadUInt16( nUnits );
    return read_uInt8s_ToOString(rStrm, nUnits);
}

inline OString read_uInt8_lenPrefixed_uInt8s_ToOString(SvStream& rStrm)
{
    sal_uInt8 nUnits = 0;
    rStrm.ReadUChar( nUnits );
    return read_uInt8s_ToOString(rStrm, nUnits);
}

/// Attempt to read a pascal-style length (of type prefix) prefixed sequence of
/// 8bit units to an OUString
inline OUString read_uInt16_lenPrefixed_uInt8s_ToOUString(SvStream& rStrm,
                                            rtl_TextEncoding eEnc)
{
    return OStringToOUString(read_uInt16_lenPrefixed_uInt8s_ToOString(rStrm), eEnc);
}

inline OUString read_uInt8_lenPrefixed_uInt8s_ToOUString(SvStream& rStrm,
                                            rtl_TextEncoding eEnc)
{
    return OStringToOUString(read_uInt8_lenPrefixed_uInt8s_ToOString(rStrm), eEnc);
}

/// Attempt to write a prefixed sequence of nUnits 8bit units from an OString,
/// returned value is number of bytes written
inline sal_Size write_uInt8s_FromOString(SvStream& rStrm, const OString& rStr,
                                                         sal_Size nUnits)
{
    return rStrm.WriteBytes(rStr.getStr(), nUnits);
}

inline sal_Size write_uInt8s_FromOString(SvStream& rStrm, const OString& rStr)
{
    return write_uInt8s_FromOString(rStrm, rStr, rStr.getLength());
}

/// Attempt to write a pascal-style length (of type prefix) prefixed
/// sequence of units from a string-type, returned value is number of bytes
/// written (including byte-count of prefix)
TOOLS_DLLPUBLIC sal_Size write_uInt16_lenPrefixed_uInt8s_FromOString(SvStream& rStrm,
                                              const OString &rStr);

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 8bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
inline sal_Size write_uInt16_lenPrefixed_uInt8s_FromOUString(SvStream& rStrm,
                                               const OUString &rStr,
                                               rtl_TextEncoding eEnc)
{
    return write_uInt16_lenPrefixed_uInt8s_FromOString(rStrm, OUStringToOString(rStr, eEnc));
}

// FileStream

class TOOLS_DLLPUBLIC SvFileStream : public SvStream
{
private:
    StreamData*     pInstanceData;
    OUString        aFilename;
    sal_uInt16      nLockCounter;
    bool            bIsOpen;

    SvFileStream (const SvFileStream&) = delete;
    SvFileStream & operator= (const SvFileStream&) = delete;

    bool LockRange( sal_Size nByteOffset, sal_Size nBytes );
    bool UnlockRange( sal_Size nByteOffset, sal_Size nBytes );
    bool LockFile();
    void UnlockFile();

protected:
    virtual sal_Size GetData( void* pData, sal_Size nSize ) override;
    virtual sal_Size PutData( const void* pData, sal_Size nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void    SetSize( sal_uInt64 nSize ) override;
    virtual void    FlushData() override;

public:
                    // Switches to Read StreamMode on failed attempt of Write opening
                    SvFileStream( const OUString& rFileName, StreamMode eOpenMode );
                    SvFileStream();
                    virtual ~SvFileStream();

    virtual void    ResetError() override;

    void            Open( const OUString& rFileName, StreamMode eOpenMode );
    void            Close();
    bool            IsOpen() const { return bIsOpen; }

    const OUString& GetFileName() const { return aFilename; }
};

// MemoryStream

class TOOLS_DLLPUBLIC SvMemoryStream : public SvStream
{
    SvMemoryStream (const SvMemoryStream&) = delete;
    SvMemoryStream & operator= (const SvMemoryStream&) = delete;

protected:
    sal_Size        nSize;
    sal_Size        nResize;
    sal_Size        nPos;
    sal_Size        nEndOfData;
    sal_uInt8*      pBuf;
    bool            bOwnsData;

    virtual sal_Size GetData( void* pData, sal_Size nSize ) override;
    virtual sal_Size PutData( const void* pData, sal_Size nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void    SetSize( sal_uInt64 nSize ) override;
    virtual void    FlushData() override;

    /// AllocateMemory must update pBuf accordingly
    /// - pBuf: Address of new block
    bool    AllocateMemory( sal_Size nSize );

    /// ReAllocateMemory must update the following variables:
    /// - pBuf: Address of new block
    /// - nEndOfData: Set to nNewSize-1L , if outside of block
    ///               Set to 0 , if new block size is 0 bytes
    /// - nSize: New block size
    /// - nPos: Set to 0 if position outside of block
    bool    ReAllocateMemory( long nDiff );

    /// Is called when this stream allocated the buffer or the buffer is
    /// resized. FreeMemory may need to NULLify handles in derived classes.
    void    FreeMemory();

public:
                    SvMemoryStream( void* pBuf, sal_Size nSize, StreamMode eMode);
                    SvMemoryStream( sal_Size nInitSize=512, sal_Size nResize=64 );
                    virtual ~SvMemoryStream();

    virtual void    ResetError() override;

    const void*    GetBuffer();
    sal_uInt64     GetSize();
    sal_Size        GetEndOfData() const { return nEndOfData; }
    const void*     GetData() { Flush(); return pBuf; }

    void*           SwitchBuffer();
    // the buffer is not owned by this class
    void            SetBuffer( void* pBuf, sal_Size nSize,
                               sal_Size nEOF=0 );

    void            ObjectOwnsMemory( bool bOwn ) { bOwnsData = bOwn; }
    void            SetResizeOffset( sal_Size nNewResize ) { nResize = nNewResize; }
    virtual sal_uInt64 remainingSize() override { return GetEndOfData() - Tell(); }
};

class TOOLS_DLLPUBLIC SvScriptStream: public SvStream
{
    oslProcess mpProcess;
    oslFileHandle mpHandle;

public:
    SvScriptStream(const OUString& rUrl);
    virtual ~SvScriptStream();

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
    virtual bool ReadLine(OString& rStr, sal_Int32 nMaxBytesToRead = 0xFFFE ) override;
    virtual bool good() const override;
};

/** Data Copy Stream

    This class is the foundation for all classes, using SvData
    (SO2\DTRANS.HXX/CXX) for transportation (e.g., graphics).
*/
class TOOLS_DLLPUBLIC SvDataCopyStream
{
public:
    // repeated execution of Load or Assign is allowed
    virtual         ~SvDataCopyStream(){}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
