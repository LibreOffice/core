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

#include <comphelper/docpasswordhelper.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <filter/msfilter/mscodec.hxx>
#include <tools/stream.hxx>
#include <memory>
#include "xlstream.hxx"
#include "xlconst.hxx"

class XclImpRoot;

/* ============================================================================
Input stream class for Excel import
- CONTINUE record handling
- Decryption
============================================================================ */

// Decryption

class XclImpDecrypter;
typedef std::shared_ptr< XclImpDecrypter > XclImpDecrypterRef;

/** Base class for BIFF stream decryption. */
class XclImpDecrypter : public ::comphelper::IDocPasswordVerifier
{
public:
    explicit            XclImpDecrypter();
    virtual             ~XclImpDecrypter() override;

    /** Returns the current error code of the decrypter. */
    const ErrCode&      GetError() const { return mnError; }
    /** Returns true, if the decoder has been initialized correctly. */
    bool         IsValid() const { return mnError == ERRCODE_NONE; }

    /** Creates a (ref-counted) copy of this decrypter object. */
    XclImpDecrypterRef  Clone() const;

    /** Implementation of the ::comphelper::IDocPasswordVerifier interface */
    virtual ::comphelper::DocPasswordVerifierResult verifyPassword( const OUString& rPassword, css::uno::Sequence< css::beans::NamedValue >& o_rEncryptionData ) override;
    virtual ::comphelper::DocPasswordVerifierResult verifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) override;

    /** Updates the decrypter on start of a new record or after seeking stream. */
    void                Update( const SvStream& rStrm, sal_uInt16 nRecSize );
    /** Reads and decrypts nBytes bytes and stores data into the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt16          Read( SvStream& rStrm, void* pData, sal_uInt16 nBytes );

protected:
    /** Protected copy c'tor for OnClone(). */
    explicit            XclImpDecrypter( const XclImpDecrypter& rSrc );

private:
    /** Implementation of cloning this object. */
    virtual XclImpDecrypter* OnClone() const = 0;
    /** Derived classes implement password verification and initialization of
        the decoder. */
    virtual css::uno::Sequence< css::beans::NamedValue >
        OnVerifyPassword( const OUString& rPassword ) = 0;
    virtual bool OnVerifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) = 0;

    /** Implementation of updating the decrypter. */
    virtual void        OnUpdate( std::size_t nOldStrmPos, std::size_t nNewStrmPos, sal_uInt16 nRecSize ) = 0;
    /** Implementation of the decryption. */
    virtual sal_uInt16  OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes ) = 0;

private:
    ErrCode             mnError;        /// Decrypter error code.
    sal_uInt64          mnOldPos;       /// Last known stream position.
    sal_uInt16          mnRecSize;      /// Current record size.
};

/** Decrypts BIFF5 stream contents. */
class XclImpBiff5Decrypter : public XclImpDecrypter
{
public:
    explicit            XclImpBiff5Decrypter( sal_uInt16 nKey, sal_uInt16 nHash );

private:
    /** Private copy c'tor for OnClone(). */
    explicit            XclImpBiff5Decrypter( const XclImpBiff5Decrypter& rSrc );

    /** Implementation of cloning this object. */
    virtual XclImpBiff5Decrypter* OnClone() const override;
    /** Implements password verification and initialization of the decoder. */
    virtual css::uno::Sequence< css::beans::NamedValue >
        OnVerifyPassword( const OUString& rPassword ) override;
    virtual bool OnVerifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) override;
    /** Implementation of updating the decrypter. */
    virtual void        OnUpdate( std::size_t nOldStrmPos, std::size_t nNewStrmPos, sal_uInt16 nRecSize ) override;
    /** Implementation of the decryption. */
    virtual sal_uInt16  OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes ) override;

private:
    ::msfilter::MSCodec_XorXLS95 maCodec;       /// Crypto algorithm implementation.
    css::uno::Sequence< css::beans::NamedValue > maEncryptionData;
    sal_uInt16          mnKey;
    sal_uInt16          mnHash;
};

/** Decrypts BIFF8 stream contents using the given document identifier. */
class XclImpBiff8Decrypter : public XclImpDecrypter
{
private:
    /** Implements password verification and initialization of the decoder. */
    virtual css::uno::Sequence< css::beans::NamedValue >
        OnVerifyPassword( const OUString& rPassword ) override;
    virtual bool OnVerifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) override;
    /** Implementation of updating the decrypter. */
    virtual void        OnUpdate( std::size_t nOldStrmPos, std::size_t nNewStrmPos, sal_uInt16 nRecSize ) override;
    /** Implementation of the decryption. */
    virtual sal_uInt16  OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes ) override;

    /** Returns the block number corresponding to the passed stream position. */
    static sal_uInt32   GetBlock( std::size_t nStrmPos );
    /** Returns the block offset corresponding to the passed stream position. */
    static sal_uInt16   GetOffset( std::size_t nStrmPos );

protected:
    explicit  XclImpBiff8Decrypter(const std::vector<sal_uInt8>& rSalt,
                                   const std::vector<sal_uInt8>& rVerifier,
                                   const std::vector<sal_uInt8>& rVerifierHash);

    explicit  XclImpBiff8Decrypter(const XclImpBiff8Decrypter& rSrc);

    css::uno::Sequence< css::beans::NamedValue > maEncryptionData;
    std::vector< sal_uInt8 > maSalt;
    std::vector< sal_uInt8 > maVerifier;
    std::vector< sal_uInt8 > maVerifierHash;
    msfilter::MSCodec97* mpCodec;       /// Crypto algorithm implementation.
};

class XclImpBiff8StdDecrypter : public XclImpBiff8Decrypter
{
public:
    explicit XclImpBiff8StdDecrypter(const std::vector<sal_uInt8>& rSalt,
                                     const std::vector<sal_uInt8>& rVerifier,
                                     const std::vector<sal_uInt8>& rVerifierHash)
        : XclImpBiff8Decrypter(rSalt, rVerifier, rVerifierHash)
    {
        mpCodec = &maCodec;
    }

private:
    /** Private copy c'tor for OnClone(). */
    explicit XclImpBiff8StdDecrypter(const XclImpBiff8StdDecrypter& rSrc);

    /** Implementation of cloning this object. */
    virtual XclImpBiff8StdDecrypter* OnClone() const override;

private:
    ::msfilter::MSCodec_Std97 maCodec;       /// Crypto algorithm implementation.
};

class XclImpBiff8CryptoAPIDecrypter : public XclImpBiff8Decrypter
{
public:
    explicit XclImpBiff8CryptoAPIDecrypter(const std::vector<sal_uInt8>& rSalt,
                                           const std::vector<sal_uInt8>& rVerifier,
                                           const std::vector<sal_uInt8>& rVerifierHash)
        : XclImpBiff8Decrypter(rSalt, rVerifier, rVerifierHash)
    {
        mpCodec = &maCodec;
    }

private:
    /** Private copy c'tor for OnClone(). */
    explicit XclImpBiff8CryptoAPIDecrypter(const XclImpBiff8CryptoAPIDecrypter& rSrc);

    /** Implementation of cloning this object. */
    virtual XclImpBiff8CryptoAPIDecrypter* OnClone() const override;

private:
    ::msfilter::MSCodec_CryptoAPI maCodec;       /// Crypto algorithm implementation.
};

// Stream

/** This class represents an Excel stream position.
    @descr  It contains the relevant data for a stream position inside of a record
    (including CONTINUE records). */
class XclImpStreamPos
{
public:
    /** Constructs an invalid stream position data object. */
    explicit            XclImpStreamPos();

    /** Sets the stream position data to the passed values. */
    void                Set( const SvStream& rStrm, std::size_t nNextPos, std::size_t nCurrSize,
                            sal_uInt16 nRawRecId, sal_uInt16 nRawRecSize, sal_uInt16 nRawRecLeft,
                            bool bValid );

    /** Writes the contained stream position data to the given variables. */
    void                Get( SvStream& rStrm, std::size_t& rnNextPos, std::size_t& rnCurrSize,
                            sal_uInt16& rnRawRecId, sal_uInt16& rnRawRecSize, sal_uInt16& rnRawRecLeft,
                            bool& rbValid ) const;

    /** Returns the stored stream position. */
    std::size_t  GetPos() const { return mnPos; }

private:
    std::size_t         mnPos;          /// Absolute position of the stream.
    std::size_t         mnNextPos;      /// Absolute position of next record.
    std::size_t         mnCurrSize;     /// Current calculated size of the record.
    sal_uInt16          mnRawRecId;     /// Current raw record ID (including CONTINUEs).
    sal_uInt16          mnRawRecSize;   /// Current raw record size (without following CONTINUEs).
    sal_uInt16          mnRawRecLeft;   /// Bytes left in current raw record (without following CONTINUEs).
    bool                mbValid;        /// Read state: false = record overread.
};

/** This class is used to import record oriented streams.
    @descr  An instance is constructed with an SvStream. The SvStream stream is
    reset to its start while constructing this stream.

    To start reading a record call StartNextRecord(). Now it is possible to
    read all contents of the record using operator>>() or any of the Read***()
    functions. If some data exceeds the record size limit, the stream looks for
    a following CONTINUE record and jumps automatically to it. It is NOT
    allowed that an atomic data type is split into two records (i.e. 4 bytes of
    a double in one record and the other 4 bytes in a following CONTINUE).

    Trying to read over the record limits results in a stream error. The
    IsValid() function indicates that with returning false. From now on it is
    undefined what data the read functions will return. The error state will be
    reset, if the record is reset (with the method ResetRecord()) or if the
    next record is started.

    To switch off the automatic lookup of CONTINUE records, use ResetRecord()
    with false parameter. This is useful i.e. on import of Escher objects,
    where sometimes solely CONTINUE records will occur. The automatic lookup
    keeps switched off until the method ResetRecord() is called with parameter
    true. All other settings done on the stream (i.e. alternative CONTINUE
    record identifier, enabled decryption, NUL substitution character) will be
    reset to default values, if a new record is started.

    The import stream supports decrypting the stream data. The contents of a
    record (not the record header) will be encrypted by Excel if the file has
    been stored with password protection. The functions SetDecrypter(),
    EnableDecryption(), and DisableDecryption() control the usage of the
    decryption algorithms. SetDecrypter() sets a new decryption algorithm and
    initially enables it. DisableDecryption() may be used to stop the usage of
    the decryption temporarily (sometimes record contents are never encrypted,
    i.e. all BOF records or the stream position in BOUNDSHEET). Decryption will
    be re-enabled automatically, if a new record is started with the function
    StartNextRecord().

    It is possible to store several stream positions inside a record (including
    its CONTINUE records). The positions are stored on a stack, which can be
    controlled with the functions PushPosition(), PopPosition() and
    RejectPosition(). The stack will be cleared whenever a new record is
    started with the function StartNextRecord().

    Additionally a single global stream position can be stored which keeps
    valid during the whole import process (methods StoreGlobalPosition(),
    SeekGlobalPosition() and DeleteGlobalPosition()). This is the only way to
    jump back to a previous record (that is a real jump without return).
*/
class XclImpStream
{
public:
    /** Detects the BIFF version of the passed workbook stream. */
    static XclBiff      DetectBiffVersion( SvStream& rStrm );

    /** Constructs the Excel record import stream using a TOOLS stream object.
        @param rInStrm  The system input stream. Will be set to its start position.
        Must exist as long as this object exists */
    explicit            XclImpStream(
                            SvStream& rInStrm,
                            const XclImpRoot& rRoot );

                        ~XclImpStream();

    /** Returns the filter root data. */
    const XclImpRoot& GetRoot() const { return mrRoot; }

    /** Sets stream pointer to the start of the next record content.
        @descr  Ignores all CONTINUE records of the current record, if automatic
        CONTINUE usage is switched on.
        @return  false = no record found (end of stream). */
    bool                StartNextRecord();
    /** Sets stream pointer to the start of the record content for the record
        at the passed absolute stream position.
        @return  false = no record found (end of stream). */
    bool                StartNextRecord( std::size_t nNextRecPos );
    /** Sets stream pointer to begin of record content.
        @param bContLookup  Automatic CONTINUE lookup on/off. In difference
        to other stream settings, this setting is persistent until next call of
        this function (because it is wanted to receive the next CONTINUE
        records separately).
        @param nAltContId  Sets an alternative record ID for content
        continuation. This value is reset automatically when a new record is
        started with StartNextRecord(). */
    void                ResetRecord( bool bContLookup,
                            sal_uInt16 nAltContId = EXC_ID_UNKNOWN );
    /** Sets stream pointer before current record and invalidates stream.
        @descr  The next call to StartNextRecord() will start again the current
        record. This can be used in situations where a loop or a function
        leaves on a specific record, but the parent context expects to start
        this record by itself. The stream is invalid as long as the first
        record has not been started (it is not allowed to call any other stream
        operation then). */
    void                RewindRecord();

    /** Enables decryption of record contents for the rest of the stream. */
    void                SetDecrypter( XclImpDecrypterRef const & xDecrypter );
    /** Sets decrypter from another stream. */
    void                CopyDecrypterFrom( const XclImpStream& rStrm );
    /** Switches usage of current decryption algorithm on/off.
        @descr  Encryption is re-enabled automatically, if a new record is
        started using the function StartNextRecord(). */
    void                EnableDecryption( bool bEnable = true );
    /** Switches usage of current decryption algorithm off.
        @descr  This is a record-local setting. The function StartNextRecord()
        always enables decryption. */
    void         DisableDecryption() { EnableDecryption( false ); }

    /** Pushes current position on user position stack.
        @descr  This stack is emptied when starting a new record with
        StartNextRecord(). The decryption state (enabled/disabled) is not
        pushed onto the stack. */
    void                PushPosition();
    /** Seeks to last position from user position stack.
        @descr  This position will be removed from the stack. */
    void                PopPosition();

    /** Stores current position. This position keeps valid in all records. */
    void                StoreGlobalPosition();
    /** Seeks to the stored global user position. */
    void                SeekGlobalPosition();

    /** Returns record reading state: false = record overread. */
    bool         IsValid() const { return mbValid; }
    /** Returns the current record ID. */
    sal_uInt16   GetRecId() const { return mnRecId; }
    /** Returns the position inside of the whole record content. */
    std::size_t         GetRecPos() const;
    /** Returns the data size of the whole record without record headers. */
    std::size_t         GetRecSize();
    /** Returns remaining data size of the whole record without record headers. */
    std::size_t         GetRecLeft();
    /** Returns the record ID of the following record. */
    sal_uInt16          GetNextRecId();

    sal_uInt16          PeekRecId( std::size_t nPos );

    [[nodiscard]]
    sal_uInt8           ReaduInt8();
    [[nodiscard]]
    sal_Int16           ReadInt16();
    [[nodiscard]]
    sal_uInt16          ReaduInt16();
    [[nodiscard]]
    sal_Int32           ReadInt32();
    [[nodiscard]]
    sal_uInt32          ReaduInt32();
    [[nodiscard]]
    double              ReadDouble();

    /** Reads nBytes bytes to the existing(!) buffer pData.
        @return  Count of bytes really read. */
    std::size_t         Read( void* pData, std::size_t nBytes );
    /** Copies nBytes bytes to rOutStrm.
        @return  Count of bytes really written. */
    std::size_t         CopyToStream( SvStream& rOutStrm, std::size_t nBytes );

    /** Copies the entire record to rOutStrm. The current record position keeps unchanged. */
    void            CopyRecordToStream( SvStream& rOutStrm );

    /** Seeks absolute in record content to the specified position.
        @descr  The value 0 means start of record, independent from physical stream position. */
    void                Seek( std::size_t nPos );
    /** Seeks forward inside the current record. */
    void                Ignore( std::size_t nBytes );

    // *** special string functions *** ---------------------------------------

    // *** read/ignore unicode strings *** ------------------------------------
    /*  - look for CONTINUE records even if CONTINUE handling disabled
          (only if inside of a CONTINUE record - for TXO import)
        - no overread assertions (for Applix wrong string length export bug)

        structure of an Excel unicode string:
        (1) 2 byte character count
        (2) 1 byte flags (16-bit-characters, rich string, far east string)
        (3) [2 byte rich string format run count]
        (4) [4 byte far east data size]
        (5) character array
        (6) [4 * (rich string format run count) byte]
        (7) [(far east data size) byte]
        header = (1), (2)
        ext. header = (3), (4)
        ext. data = (6), (7)
     */

    /** Reads ext. header, detects 8/16 bit mode, sets all ext. info.
        @return  Total size of ext. data. */
    std::size_t         ReadUniStringExtHeader(
                            bool& rb16Bit, bool& rbRich, bool& rbFareast,
                            sal_uInt16& rnFormatRuns, sal_uInt32& rnExtInf, sal_uInt8 nFlags );
    /** Seeks to begin of character array, detects 8/16 bit mode.
        @return  Total size of ext. data. */
    std::size_t         ReadUniStringExtHeader( bool& rb16Bit, sal_uInt8 nFlags );

    /** Sets a replacement character for NUL characters.
        @descr  NUL characters must be replaced, because Tools strings cannot
        handle them. The substitution character is reset to '?' automatically,
        if a new record is started using the function StartNextRecord().
        @param cNulSubst  The character to use for NUL replacement. It is
        possible to specify NUL here. in this case strings are terminated when
        the first NUL occurs during string import. */
    void         SetNulSubstChar( sal_Unicode cNulSubst = '?' ) { mcNulSubst = cNulSubst; }

    /** Reads nChars characters and returns the string. */
    OUString            ReadRawUniString( sal_uInt16 nChars, bool b16Bit );
    /** Reads ext. header, nChar characters, ext. data and returns the string. */
    OUString            ReadUniString( sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Reads 8 bit flags, ext. header, nChar characters, ext. data and returns the string. */
    OUString            ReadUniString( sal_uInt16 nChars );
    /** Reads 16 bit character count, 8 bit flags, ext. header, character array,
        ext. data and returns the string. */
    OUString            ReadUniString();

    /** Ignores nChars characters. */
    void                IgnoreRawUniString( sal_uInt16 nChars, bool b16Bit );
    /** Ignores ext. header, nChar characters, ext. data. */
    void                IgnoreUniString( sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Ignores 8 bit flags, ext. header, nChar characters, ext. data. */
    void                IgnoreUniString( sal_uInt16 nChars );

    // *** read/ignore 8-bit-strings, store in String *** ---------------------

    /** Reads nChar byte characters and returns the string. */
    OUString            ReadRawByteString( sal_uInt16 nChars );
    /** Reads 8/16 bit string length, character array and returns the string. */
    OUString            ReadByteString( bool b16BitLen );

    // *** SvStream functions *** ---------------------------------------------

    /** Returns the absolute stream position. */
    std::size_t  GetSvStreamPos() const { return mrStrm.Tell(); }
    /** Returns the stream size. */
    std::size_t  GetSvStreamSize() const { return mnStreamSize; }

    /** Stores current stream position into rPos. */
    void                StorePosition( XclImpStreamPos& rPos );
    /** Restores stream position contained in rPos. */
    void                RestorePosition( const XclImpStreamPos& rPos );

    /** Set an SVSTREAM_..._ERROR. */
    void                SetSvStreamError( const ErrCode& rErrCode )
                            { mrStrm.SetError( rErrCode ); }

private:
    /** Seeks to next raw record header and reads record ID and size.
        @descr  This is a "raw" function, means that stream members are
        inconsistent after return. Does only change mnRawRecId, mnRawRecSize,
        and the base stream position, but no other members.
        @return  false = No record header found (end of stream). */
    bool                ReadNextRawRecHeader();

    /** Initializes the decrypter to read a new record. */
    void                SetupDecrypter();
    /** Initializes all members after base stream has been sought to new raw record. */
    void                SetupRawRecord();
    /** Initializes all members after base stream has been sought to new record. */
    void                SetupRecord();

    /** Returns true, if the passed ID is real or alternative continuation record ID. */
    bool                IsContinueId( sal_uInt16 nRecId ) const;

    /** Goes to start of the next CONTINUE record.
        @descr  Stream must be located at the end of a raw record, and handling
        of CONTINUE records must be enabled.
        @return  Copy of mbValid. */
    bool                JumpToNextContinue();
    /** Goes to start of the next CONTINUE record while reading strings.
        @descr  Stream must be located at the end of a raw record. If reading
        has been started in a CONTINUE record, jumps to an existing following
        CONTINUE record, even if handling of CONTINUE records is disabled (This
        is a special handling for TXO string data). Reads additional Unicode
        flag byte at start of the new raw record and sets or resets rb16Bit.
        @return  Copy of mbValid. */
    bool                JumpToNextStringContinue( bool& rb16Bit );

    /** Ensures that reading nBytes bytes is possible with next stream access.
        @descr  Stream must be located at the end of a raw record, and handling
        of CONTINUE records must be enabled.
        @return  Copy of mbValid. */
    bool                EnsureRawReadSize( sal_uInt16 nBytes );
    /** Returns the maximum size of raw data possible to read in one block. */
    sal_uInt16          GetMaxRawReadSize( std::size_t nBytes ) const;

    /** Reads and decrypts nBytes bytes to the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt16          ReadRawData( void* pData, sal_uInt16 nBytes );

private:
    SvStream&           mrStrm;         /// Reference to the system input stream.
    const XclImpRoot&   mrRoot;         /// Filter root data.

    XclImpDecrypterRef  mxDecrypter;    /// Provides methods to decrypt data.

    XclImpStreamPos     maFirstRec;     /// Start position of current record.
    std::vector< XclImpStreamPos >
                        maPosStack;    /// Stack for record positions.

    XclImpStreamPos     maGlobPos;      /// User defined position elsewhere in stream.
    sal_uInt16          mnGlobRecId;    /// Record ID for user defined position.
    bool                mbGlobValidRec; /// Was user position a valid record?
    bool                mbHasGlobPos;   /// Is user position defined?

    std::size_t         mnStreamSize;   /// Size of system stream.
    std::size_t         mnNextRecPos;   /// Start of next record header.
    std::size_t         mnCurrRecSize;  /// Helper for record position.
    std::size_t         mnComplRecSize; /// Size of complete record data (with CONTINUEs).
    bool                mbHasComplRec;  /// true = mnComplRecSize is valid.

    sal_uInt16          mnRecId;        /// Current record ID (not the CONTINUE ID).
    sal_uInt16          mnAltContId;    /// Alternative record ID for content continuation.

    sal_uInt16          mnRawRecId;     /// Current raw record ID (including CONTINUEs).
    sal_uInt16          mnRawRecSize;   /// Current raw record size (without following CONTINUEs).
    sal_uInt16          mnRawRecLeft;   /// Bytes left in current raw record (without following CONTINUEs).

    sal_Unicode         mcNulSubst;     /// Replacement for NUL characters.

    bool                mbCont;         /// Automatic CONTINUE lookup on/off.
    bool                mbUseDecr;      /// Usage of decryption.
    bool                mbValidRec;     /// false = No more records to read.
    bool                mbValid;        /// false = Record overread.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
