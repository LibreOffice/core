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
#pragma once

#include <config_options.h>
#include <tools/toolsdllapi.h>
#include <tools/lineend.hxx>
#include <tools/long.hxx>
#include <tools/ref.hxx>
#include <comphelper/errcode.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>
#include <string_view>

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
    COPY_ON_SYMLINK          = 0x0010,  ///< copy-on-write for symlinks (Unix-only)
    TEMPORARY                = 0x0020,  ///< temporary file attribute (Windows-only)
    DELETE_ON_CLOSE          = 0x0040,  ///< only for temporary files (Windows-only)
// sharing options
    SHARE_DENYNONE           = 0x0100,
    SHARE_DENYREAD           = 0x0200,  // overrides denynone
    SHARE_DENYWRITE          = 0x0400,  // overrides denynone
    SHARE_DENYALL            = 0x0800,  // overrides denyread,write,none
// masks
    READWRITE                = READ | WRITE,
    STD_READ                 = READ | SHARE_DENYNONE | NOCREATE,
    STD_WRITE                = WRITE | SHARE_DENYALL,
    STD_READWRITE            = READWRITE | SHARE_DENYALL
};
namespace o3tl
{
    template<> struct typed_flags<StreamMode> : is_typed_flags<StreamMode, 0x0f7f> {};
}

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

struct SvLockBytesStat
{
    std::size_t nSize;

    SvLockBytesStat() : nSize(0) {}
};

/** This is only extended by UcbLockBytes in ucb/ and appears to exist
to allow UCB to do delayed feeding of data into a SvStream i.e. a kind of a pipe
mechanism to allow asynchronous fetching of data.
*/
class UNLESS_MERGELIBS(TOOLS_DLLPUBLIC) SvLockBytes: public SvRefBase
{
    SvStream * m_pStream;
    bool m_bOwner;
    bool m_bSync;

protected:
    void close();

public:

    SvLockBytes() : m_pStream(nullptr), m_bOwner(false), m_bSync(false) {}

    virtual ~SvLockBytes() override { close(); }

    const SvStream * GetStream() const { return m_pStream; }

    void            SetSynchronMode(bool bTheSync = true) { m_bSync = bTheSync; }
    bool            IsSynchronMode() const { return m_bSync; }

    virtual ErrCode ReadAt(sal_uInt64 nPos, void * pBuffer, std::size_t nCount,
                           std::size_t * pRead) const;
    virtual ErrCode WriteAt(sal_uInt64 nPos, const void * pBuffer, std::size_t nCount,
                            std::size_t * pWritten);

    virtual ErrCode Flush() const;

    virtual ErrCode SetSize(sal_uInt64 nSize);

    virtual ErrCode Stat(SvLockBytesStat * pStat) const;
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
    std::unique_ptr<sal_uInt8[]>
                    m_pRWBuf;     ///< Points to read/write buffer
    sal_uInt8*      m_pBufPos;    ///< m_pRWBuf + m_nBufActualPos
    sal_uInt16      m_nBufSize;   ///< Allocated size of buffer
    sal_uInt16      m_nBufActualLen; ///< Length of used segment of buffer
                                  ///< = m_nBufSize, if EOF did not occur
    sal_uInt16      m_nBufActualPos; ///< current position in buffer (0..m_nBufSize-1)
    sal_uInt16      m_nBufFree;   ///< number of free slots in buffer to IO of type eIOMode
    bool            m_isIoRead;
    bool            m_isIoWrite;

    // Error codes, conversion, compression, ...
    bool            m_isDirty;  ///< true: Stream != buffer content
    bool            m_isSwap;
    bool            m_isEof;
    ErrCode         m_nError;
    SvStreamCompressFlags m_nCompressMode;
    LineEnd         m_eLineDelimiter;
    rtl_TextEncoding m_eStreamCharSet;

    // Encryption
    OString m_aCryptMaskKey;// aCryptMaskKey.getLength != 0  -> Encryption used
    unsigned char   m_nCryptMask;

    // Userdata
    sal_Int32       m_nVersion;   // for external use

                    SvStream ( const SvStream& rStream ) = delete;
    SvStream&       operator=( const SvStream& rStream ) = delete;

protected:
    sal_uInt64      m_nBufFilePos; ///< File position of pBuf[0]
    StreamMode      m_eStreamMode;
    bool            m_isWritable;

    virtual std::size_t GetData( void* pData, std::size_t nSize );
    virtual std::size_t PutData( const void* pData, std::size_t nSize );
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos );
    virtual void    FlushData();
    virtual void    SetSize(sal_uInt64 nSize);

    SAL_DLLPRIVATE void ClearError();
    SAL_DLLPRIVATE void ClearBuffer();

    // encrypt and write in blocks
    SAL_DLLPRIVATE std::size_t CryptAndWriteBuffer( const void* pStart, std::size_t nLen );
    SAL_DLLPRIVATE void EncryptBuffer( void* pStart, std::size_t nLen ) const;

public:
                    SvStream();
                    SvStream( SvLockBytes *pLockBytes);
    virtual         ~SvStream();

    SvLockBytes*    GetLockBytes() const { return m_xLockBytes.get(); }

    ErrCode         GetError() const { return m_nError.IgnoreWarning(); }
    ErrCode const & GetErrorCode() const { return m_nError; }
    void            SetError( ErrCode nErrorCode );
    virtual void    ResetError();

    void            SetEndian( SvStreamEndian SvStreamEndian );
    SvStreamEndian  GetEndian() const;
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
    SvStream&       WriteOString(std::string_view rStr)
                        { WriteBytes(rStr.data(), rStr.size()); return *this; }
    SvStream&       WriteStream( SvStream& rStream );
    sal_uInt64      WriteStream( SvStream& rStream, sal_uInt64 nSize );

    SvStream&       WriteBool( bool b )
                        { return WriteUChar(static_cast<unsigned char>(b)); }
    SvStream&       WriteSChar( signed char nChar );
    SvStream&       WriteChar( char nChar );
    SvStream&       WriteUChar( unsigned char nChar );
    SvStream&       WriteFloat( float nFloat );
    SvStream&       WriteDouble( double nDouble );

    template <typename N>
    SvStream&       WriteNumberAsString( N n ) { return WriteOString(OString::number(n)); }

    std::size_t     ReadBytes( void* pData, std::size_t nSize );
    std::size_t     WriteBytes( const void* pData, std::size_t nSize );
    sal_uInt64      Seek( sal_uInt64 nPos );
    sal_uInt64      SeekRel( sal_Int64 nPos );
    sal_uInt64      Tell() const { return m_nBufFilePos + m_nBufActualPos;  }
    virtual sal_uInt64 TellEnd();
    // length between current (Tell()) pos and end of stream
    sal_uInt64      remainingSize();
    /// If we have data in our internal buffers, write them out
    void            FlushBuffer();
    /// Call FlushBuffer() and then call flush on the underlying OS stream
    void            Flush();
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
    bool            ReadLine( OStringBuffer& rStr, sal_Int32 nMaxBytesToRead = 0xFFFE );
    bool            ReadLine( OString& rStr, sal_Int32 nMaxBytesToRead = 0xFFFE );
    bool            WriteLine( std::string_view rStr );

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
    bool            WriteByteStringLine( std::u16string_view rStr, rtl_TextEncoding eDestCharSet );

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
    SAL_DLLPRIVATE bool ReadUniStringLine(OUString& rStr, sal_Int32 nMaxCodepointsToRead);
    /** Read a 32bit length prefixed sequence of utf-16 if
        eSrcCharSet==RTL_TEXTENCODING_UNICODE, otherwise read a 16bit length
        prefixed sequence of bytes and convert from eSrcCharSet */
    OUString        ReadUniOrByteString(rtl_TextEncoding eSrcCharSet);
    /** Write a 32bit length prefixed sequence of utf-16 if
        eSrcCharSet==RTL_TEXTENCODING_UNICODE, otherwise convert to eSrcCharSet
        and write a 16bit length prefixed sequence of bytes */
    SvStream&       WriteUniOrByteString( std::u16string_view rStr, rtl_TextEncoding eDestCharSet );

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
        Bytecodes converted to eDestCharSet. Write trailing zero, if bZero is true. */
    bool            WriteUnicodeOrByteText(std::u16string_view rStr, rtl_TextEncoding eDestCharSet, bool bZero = false);
    bool            WriteUnicodeOrByteText(std::u16string_view rStr, bool bZero = false)
                    { return WriteUnicodeOrByteText(rStr, GetStreamCharSet(), bZero); }

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

    sal_Int32       GetVersion() const { return m_nVersion; }
    void            SetVersion( sal_Int32 n ) { m_nVersion = n; }

    friend SvStream& operator<<( SvStream& rStr, SvStrPtr f ); // for Manips

    /// end of input seen during previous i/o operation
    bool eof() const { return m_isEof; }

    /// stream is broken
    bool bad() const { return GetError() != ERRCODE_NONE; }

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

private:
    template <typename T> SvStream& ReadNumber(T& r);
    template <typename T> SvStream& WriteNumber(T n);

    template<typename T>
    void readNumberWithoutSwap(T& rDataDest)
    { readNumberWithoutSwap_(&rDataDest, sizeof(rDataDest)); }

    SAL_DLLPRIVATE void readNumberWithoutSwap_(void * pDataDest, int nDataSize);

    template<typename T>
    void writeNumberWithoutSwap(T const & rDataSrc)
    { writeNumberWithoutSwap_(&rDataSrc, sizeof(rDataSrc)); }

    SAL_DLLPRIVATE void writeNumberWithoutSwap_(const void * pDataSrc, int nDataSize);
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
    std::size_t nUnits);

/// Attempt to read nUnits 8bit units to an OUString
inline OUString read_uInt8s_ToOUString(SvStream& rStrm,
    std::size_t nUnits, rtl_TextEncoding eEnc)
{
    return OStringToOUString(read_uInt8s_ToOString(rStrm, nUnits), eEnc);
}

/// Attempt to read nUnits 16bit units to an OUString, returned
/// OUString's length is number of units successfully read
TOOLS_DLLPUBLIC OUString read_uInt16s_ToOUString(SvStream& rStrm,
    std::size_t nUnits);

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

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 16bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
std::size_t write_uInt32_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                std::u16string_view rStr);

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 16bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
UNLESS_MERGELIBS(TOOLS_DLLPUBLIC) std::size_t write_uInt16_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                std::u16string_view rStr);

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
inline OString read_uInt32_lenPrefixed_uInt8s_ToOString(SvStream& rStrm)
{
    sal_uInt32 nUnits = 0;
    rStrm.ReadUInt32(nUnits);
    return read_uInt8s_ToOString(rStrm, nUnits);
}
inline OString read_uInt16_lenPrefixed_uInt8s_ToOString(SvStream& rStrm)
{
    sal_uInt16 nUnits = 0;
    rStrm.ReadUInt16(nUnits);
    return read_uInt8s_ToOString(rStrm, nUnits);
}

inline OString read_uInt8_lenPrefixed_uInt8s_ToOString(SvStream& rStrm)
{
    sal_uInt8 nUnits = 0;
    rStrm.ReadUChar(nUnits);
    return read_uInt8s_ToOString(rStrm, nUnits);
}

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

/// Attempt to write a pascal-style length (of type prefix) prefixed
/// sequence of units from a string-type, returned value is number of bytes
/// written (including byte-count of prefix)
TOOLS_DLLPUBLIC std::size_t write_uInt16_lenPrefixed_uInt8s_FromOString(SvStream& rStrm,
                                              std::string_view rStr);

/// Attempt to write a pascal-style length (of type prefix) prefixed sequence
/// of 8bit units from an OUString, returned value is number of bytes written
/// (including byte-count of prefix)
inline std::size_t write_uInt16_lenPrefixed_uInt8s_FromOUString(SvStream& rStrm,
                                               std::u16string_view rStr,
                                               rtl_TextEncoding eEnc)
{
    return write_uInt16_lenPrefixed_uInt8s_FromOString(rStrm, OUStringToOString(rStr, eEnc));
}

[[nodiscard]] TOOLS_DLLPUBLIC bool checkSeek(SvStream &rSt, sal_uInt64 nOffset);

namespace tools
{
/// Is rUrl a file:// URL with no contents?
TOOLS_DLLPUBLIC bool isEmptyFileUrl(const OUString& rUrl);
}

// FileStream

class TOOLS_DLLPUBLIC SvFileStream final : public SvStream
{
private:
    void*           mxFileHandle = nullptr; // on windows, it is a HANDLE, otherwise, it is a oslFileHandle
#if defined(_WIN32)
    sal_uInt16      nLockCounter;
#endif
    OUString        aFilename;
    bool            bIsOpen;

    SvFileStream (const SvFileStream&) = delete;
    SvFileStream & operator= (const SvFileStream&) = delete;

    bool LockFile();
    void UnlockFile();

    virtual std::size_t GetData( void* pData, std::size_t nSize ) override;
    virtual std::size_t PutData( const void* pData, std::size_t nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void    SetSize( sal_uInt64 nSize ) override;
    virtual void    FlushData() override;

public:
                    // Switches to Read StreamMode on failed attempt of Write opening
                    SvFileStream( const OUString& rFileName, StreamMode eOpenMode );
                    SvFileStream();
                    virtual ~SvFileStream() override;

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
    std::size_t     nSize;
    std::size_t     nResize;
    std::size_t     nPos;
    std::size_t     nEndOfData;
    sal_uInt8*      pBuf;
    bool            bOwnsData;

    virtual std::size_t GetData( void* pData, std::size_t nSize ) override;
    virtual std::size_t PutData( const void* pData, std::size_t nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void    SetSize( sal_uInt64 nSize ) override;
    virtual void    FlushData() override;

    /// AllocateMemory must update pBuf accordingly
    /// - pBuf: Address of new block
    void    AllocateMemory( std::size_t nSize );

    /// ReAllocateMemory must update the following variables:
    /// - pBuf: Address of new block
    /// - nEndOfData: Set to nNewSize-1 , if outside of block
    ///               Set to 0 , if new block size is 0 bytes
    /// - nSize: New block size
    /// - nPos: Set to 0 if position outside of block
    bool    ReAllocateMemory( tools::Long nDiff );

    /// Is called when this stream allocated the buffer or the buffer is
    /// resized. FreeMemory may need to NULLify handles in derived classes.
    void    FreeMemory();

public:
                    SvMemoryStream( void* pBuf, std::size_t nSize, StreamMode eMode);
                    SvMemoryStream( std::size_t nInitSize=512, std::size_t nResize=64 );
                    virtual ~SvMemoryStream() override;

    virtual void    ResetError() override final;

    sal_uInt64      GetSize() { return TellEnd(); }
    std::size_t     GetEndOfData() const { return nEndOfData; }
    const void*     GetData() { FlushBuffer(); return pBuf; }

    // return the buffer currently in use, and allocate a new buffer internally
    void*           SwitchBuffer();
    // the buffer is not owned by this class
    void            SetBuffer( void* pBuf, std::size_t nSize, std::size_t nEOF );

    void            ObjectOwnsMemory( bool bOwn ) { bOwnsData = bOwn; }
    /// Makes the stream read-only after it was (possibly) initially writable,
    /// without having to copy the data or change buffers.
    /// @since LibreOffice 7.5
    void            MakeReadOnly();
    void            SetResizeOffset( std::size_t nNewResize ) { nResize = nNewResize; }
    virtual sal_uInt64 TellEnd() override final { FlushBuffer(); return nEndOfData; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
