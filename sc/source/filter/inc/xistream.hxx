/*************************************************************************
 *
 *  $RCSfile: xistream.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:47:40 $
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

// ============================================================================

#ifndef SC_XISTREAM_HXX
#define SC_XISTREAM_HXX

#ifndef SVX_MSCODEC_HXX
#include <svx/mscodec.hxx>
#endif
#ifndef SC_XLSTREAM_HXX
#include "xlstream.hxx"
#endif

class XclImpRoot;

/* ============================================================================
Input stream class for Excel import
- CONTINUE record handling
- ByteString and UniString support
- Decryption
============================================================================ */

// ============================================================================
// Decryption
// ============================================================================

class XclImpDecrypter;
typedef ::boost::shared_ptr< XclImpDecrypter > XclImpDecrypterRef;

/** Base class for BIFF stream decryption. */
class XclImpDecrypter : ScfNoCopy
{
public:
    explicit            XclImpDecrypter();
    virtual             ~XclImpDecrypter();

    /** Returns the current error code of the decrypter. */
    inline ErrCode      GetError() const { return mnError; }
    /** Returns true, if the decrypter has been constructed successfully.
        This means especially that construction happened with a valid password. */
    inline bool         IsValid() const { return mnError == ERRCODE_NONE; }

    /** Creates a (ref-counted) copy of this decrypter object. */
    XclImpDecrypterRef  Clone() const;

    /** Updates the decrypter on start of a new record or after seeking stream. */
    void                Update( SvStream& rStrm, sal_uInt16 nRecSize );
    /** Reads and decrypts nBytes bytes and stores data into the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt16          Read( SvStream& rStrm, void* pData, sal_uInt16 nBytes );

protected:
    /** Protected copy c'tor for OnClone(). */
    explicit            XclImpDecrypter( const XclImpDecrypter& rSrc );

    /** Sets the decrypter to a state showing whether the password was correct. */
    void                SetHasValidPassword( bool bValid );

private:
    /** Implementation of cloning this object. */
    virtual XclImpDecrypter* OnClone() const = 0;
    /** Implementation of updating the decrypter. */
    virtual void        OnUpdate( ULONG nOldStrmPos, ULONG nNewStrmPos, sal_uInt16 nRecSize ) = 0;
    /** Implementation of the decryption. */
    virtual sal_uInt16  OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes ) = 0;

private:
    ErrCode             mnError;        /// Decrypter error code.
    ULONG               mnOldPos;       /// Last known stream position.
    sal_uInt16          mnRecSize;      /// Current record size.
};

// ----------------------------------------------------------------------------

/** Decrypts BIFF5 stream contents. */
class XclImpBiff5Decrypter : public XclImpDecrypter
{
public:
    /** Constructs the decrypter.
        @descr  Checks if the passed key and hash specify workbook protection.
        Asks for a password otherwise.
        @param nKey  Password key from FILEPASS record to verify password.
        @param nHash  Password hash value from FILEPASS record to verify password. */
    explicit            XclImpBiff5Decrypter( const XclImpRoot& rRoot, sal_uInt16 nKey, sal_uInt16 nHash );

private:
    /** Private copy c'tor for OnClone(). */
    explicit            XclImpBiff5Decrypter( const XclImpBiff5Decrypter& rSrc );

    /** Implementation of cloning this object. */
    virtual XclImpBiff5Decrypter* OnClone() const;
    /** Implementation of updating the decrypter. */
    virtual void        OnUpdate( ULONG nOldStrmPos, ULONG nNewStrmPos, sal_uInt16 nRecSize );
    /** Implementation of the decryption. */
    virtual sal_uInt16  OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes );

    /** Initializes the members.
        @postcond  Internal status is set and can be querried with IsValid(). */
    void                Init( const ByteString& rPass, sal_uInt16 nKey, sal_uInt16 nHash );

private:
    ::svx::MSCodec_XorXLS95 maCodec;       /// Crypto algorithm implementation.
    sal_uInt8           mpnPassw[ 16 ]; /// Cached password data for copy construction.
};

// ----------------------------------------------------------------------------

/** Decrypts BIFF8 stream contents using the given document identifier. */
class XclImpBiff8Decrypter : public XclImpDecrypter
{
public:
    /** Constructs the decrypter.
        @descr  Checks if the passed salt data specifies workbook protection.
        Asks for a password otherwise.
        @param pnDocId  Unique document identifier from FILEPASS record.
        @param pnSaltData  Salt data from FILEPASS record.
        @param pnSaltHash  Salt hash value from FILEPASS record. */
    explicit            XclImpBiff8Decrypter( const XclImpRoot& rRoot, sal_uInt8 pnDocId[ 16 ],
                            sal_uInt8 pnSaltData[ 16 ], sal_uInt8 pnSaltHash[ 16 ] );

private:
    /** Private copy c'tor for OnClone(). */
    explicit            XclImpBiff8Decrypter( const XclImpBiff8Decrypter& rSrc );

    /** Implementation of cloning this object. */
    virtual XclImpBiff8Decrypter* OnClone() const;
    /** Implementation of updating the decrypter. */
    virtual void        OnUpdate( ULONG nOldStrmPos, ULONG nNewStrmPos, sal_uInt16 nRecSize );
    /** Implementation of the decryption. */
    virtual sal_uInt16  OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes );

    /** Initializes the internal codec.
        @postcond  Internal status is set and can be querried with IsValid(). */
    void                Init( const String& rPass, sal_uInt8 pnDocId[ 16 ],
                            sal_uInt8 pnSaltData[ 16 ], sal_uInt8 pnSaltHash[ 16 ] );

    /** Returns the block number corresponding to the passed stream position. */
    sal_uInt32          GetBlock( ULONG nStrmPos ) const;
    /** Returns the block offset corresponding to the passed stream position. */
    sal_uInt16          GetOffset( ULONG nStrmPos ) const;

private:
    ::svx::MSCodec_Std97 maCodec;       /// Crypto algorithm implementation.
    sal_uInt16          mpnPassw[ 16 ]; /// Cached password data for copy construction.
    sal_uInt8           mpnDocId[ 16 ]; /// Cached document ID for copy construction.
};

// ============================================================================
// Stream
// ============================================================================

/** This class represents an Excel stream position.
    @descr  It contains the relevant data for a stream position inside of a record
    (including CONTINUE records). */
class XclImpStreamPos
{
public:
    /** Constructs an invalid stream position data object. */
    explicit            XclImpStreamPos();

    /** Sets the stream position data to the passed values. */
    void                Set( const SvStream& rStrm, ULONG nNextPos, sal_uInt32 nCurrSize,
                            sal_uInt16 nRawRecId, sal_uInt16 nRawRecSize, sal_uInt16 nRawRecLeft,
                            bool bValid );

    /** Writes the contained stream position data to the given variables. */
    void                Get( SvStream& rStrm, ULONG& rnNextPos, sal_uInt32& rnCurrSize,
                            sal_uInt16& rnRawRecId, sal_uInt16& rnRawRecSize, sal_uInt16& rnRawRecLeft,
                            bool& rbValid ) const;

private:
    ULONG               mnPos;          /// Absolute position of the stream.
    ULONG               mnNextPos;      /// Absolute position of next record.
    sal_uInt32          mnCurrSize;     /// Current calculated size of the record.
    sal_uInt16          mnRawRecId;     /// Current raw record ID (including CONTINUEs).
    sal_uInt16          mnRawRecSize;   /// Current raw record size (without following CONTINUEs).
    sal_uInt16          mnRawRecLeft;   /// Bytes left in current raw record (without following CONTINUEs).
    bool                mbValid;        /// Read state: false = record overread.
};

// ============================================================================

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
    be reenabled automatically, if a new record is started with the function
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
        Must exist as long as this object exists.
        @param bContLookup  Automatic CONTINUE lookup on/off. */
    explicit            XclImpStream(
                            SvStream& rInStrm,
                            const XclImpRoot& rRoot,
                            bool bContLookup = true );

                        ~XclImpStream();

    /** Returns the filter root data. */
    inline const XclImpRoot& GetRoot() const { return mrRoot; }

    /** Sets stream pointer to the start of the next record content.
        @descr  Ignores all CONTINUE records of the current record, if automatic
        CONTINUE usage is switched on.
        @return  false = no record found (end of stream). */
    bool                StartNextRecord();
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

    /** Enables decryption of record contents for the rest of the stream. */
    void                SetDecrypter( XclImpDecrypterRef xDecrypter );
    /** Sets decrypter from another stream. */
    void                CopyDecrypterFrom( const XclImpStream& rStrm );
    /** Returns true, if a valid decrypter is set at the stream. */
    bool                HasValidDecrypter() const;
    /** Switches usage of current decryption algorithm on/off.
        @descr  Encryption is re-enabled automatically, if a new record is
        started using the function StartNextRecord(). */
    void                EnableDecryption( bool bEnable = true );
    /** Switches usage of current decryption algorithm off.
        @descr  This is a record-local setting. The function StartNextRecord()
        always enables decryption. */
    inline void         DisableDecryption() { EnableDecryption( false ); }

    /** Pushes current position on user position stack.
        @descr  This stack is emptied when starting a new record with
        StartNextRecord(). The decryption state (enabled/disabled) is not
        pushed onto the stack. */
    void                PushPosition();
    /** Seeks to last position from user position stack.
        @descr  This position will be removed from the stack. */
    void                PopPosition();
    /** Removes last position from user position stack, but does not seek to it. */
    void                RejectPosition();

    /** Stores current position. This position keeps valid in all records. */
    void                StoreGlobalPosition();
    /** Seeks to the stored global user position. */
    void                SeekGlobalPosition();
    /** Invalidates global user position. */
    inline void         DeleteGlobalPosition() { mbHasGlobPos = false; }

    /** Returns record reading state: false = record overread. */
    inline bool         IsValid() const { return mbValid; }
    /** Returns the current record ID. */
    inline sal_uInt16   GetRecId() const { return mnRecId; }
    /** Returns the position inside of the whole record content. */
    sal_uInt32          GetRecPos() const;
    /** Returns the data size of the whole record without record headers. */
    sal_uInt32          GetRecSize();
    /** Returns remaining data size of the whole record without record headers. */
    sal_uInt32          GetRecLeft();

    XclImpStream&       operator>>( sal_Int8& rnValue );
    XclImpStream&       operator>>( sal_uInt8& rnValue );
    XclImpStream&       operator>>( sal_Int16& rnValue );
    XclImpStream&       operator>>( sal_uInt16& rnValue );
    XclImpStream&       operator>>( sal_Int32& rnValue );
    XclImpStream&       operator>>( sal_uInt32& rnValue );
    XclImpStream&       operator>>( float& rfValue );
    XclImpStream&       operator>>( double& rfValue );

    sal_Int8            ReadInt8();
    sal_uInt8           ReaduInt8();
    sal_Int16           ReadInt16();
    sal_uInt16          ReaduInt16();
    sal_Int32           ReadInt32();
    sal_uInt32          ReaduInt32();
    float               ReadFloat();
    double              ReadDouble();

    /** Reads nBytes bytes to the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt32          Read( void* pData, sal_uInt32 nBytes );
    /** Copies nBytes bytes to rOutStrm.
        @return  Count of bytes really written. */
    sal_uInt32          CopyToStream( SvStream& rOutStrm, sal_uInt32 nBytes );
    /** Copies the entire record to rOutStrm. The current record position keeps unchanged.
        @return  Count of bytes really written. */
    sal_uInt32          CopyRecordToStream( SvStream& rOutStrm );

    /** Seeks absolute in record content to the specified position.
        @descr  The value 0 means start of record, independent from physical stream position. */
    void                Seek( sal_uInt32 nPos );
    /** Seeks forward inside the current record. */
    void                Ignore( sal_uInt32 nBytes );

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
    sal_uInt32          ReadUniStringExtHeader(
                            bool& rb16Bit, bool& rbRich, bool& rbFareast,
                            sal_uInt16& rnFormatRuns, sal_uInt32& rnExtInf, sal_uInt8 nFlags );
    /** Seeks to begin of character array, detects 8/16 bit mode.
        @return  Total size of ext. data. */
    sal_uInt32          ReadUniStringExtHeader( bool& rb16Bit, sal_uInt8 nFlags );
    /** Skips ext. data after character array. */
    inline void         SkipUniStringExtData( sal_uInt32 nExtSize ) { Ignore( nExtSize ); }

    /** Sets a replacement character for NUL characters.
        @descr  NUL characters must be replaced, because Tools strings cannot
        handle them. The substitution character is reset to '?' automatically,
        if a new record is started using the function StartNextRecord().
        @param cNulSubst  The character to use for NUL replacement. It is
        possible to specify NUL here. in this case strings are terminated when
        the first NUL occurs during string import. */
    inline void         SetNulSubstChar( sal_Unicode cNulSubst = '?' ) { mcNulSubst = cNulSubst; }

    /** Reads nChars characters and returns the string. */
    String              ReadRawUniString( sal_uInt16 nChars, bool b16Bit );
    /** Reads ext. header, nChar characters, ext. data and returns the string. */
    String              ReadUniString( sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Reads 8 bit flags, ext. header, nChar characters, ext. data and returns the string. */
    String              ReadUniString( sal_uInt16 nChars );
    /** Reads 16 bit character count, 8 bit flags, ext. header, character array,
        ext. data and returns the string. */
    String              ReadUniString();

    /** Ignores nChars characters. */
    void                IgnoreRawUniString( sal_uInt16 nChars, bool b16Bit );
    /** Ignores ext. header, nChar characters, ext. data. */
    void                IgnoreUniString( sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Ignores 8 bit flags, ext. header, nChar characters, ext. data. */
    void                IgnoreUniString( sal_uInt16 nChars );
    /** Ignores 16 bit character count, 8 bit flags, ext. header, character array, ext. data. */
    void                IgnoreUniString();

    // *** read/ignore 8-bit-strings, store in String *** ---------------------

    /** Reads nChar byte characters and returns the string. */
    String              ReadRawByteString( sal_uInt16 nChars );
    /** Reads 8/16 bit string length, character array and returns the string. */
    String              ReadByteString( bool b16BitLen );

    /** Ignores nChar byte characters. */
    void                IgnoreRawByteString( sal_uInt16 nChars );
    /** Ignores 8/16 bit string length, character array. */
    void                IgnoreByteString( bool b16BitLen );

    // *** SvStream functions *** ---------------------------------------------

    /** Returns the absolute stream position. */
    inline ULONG        GetSvStreamPos() const { return mrStrm.Tell(); }
    /** Returns the stream size. */
    inline ULONG        GetSvStreamSize() const { return mnStreamSize; }

private:
    /** Stores current stream position into rPos. */
    void                StorePosition( XclImpStreamPos& rPos );
    /** Restores stream position contained in rPos. */
    void                RestorePosition( const XclImpStreamPos& rPos );

    /** Seeks to next raw record header and reads record ID and size.
        @descr  This is a "raw" function, means that stream members are
        inconsistent after return. Does only change mnRawRecId, mnRawRecSize,
        and the base stream position, but no other members.
        @return  false = No record header found (end of stream). */
    bool                ReadNextRawRecHeader();

    /** Initializes the decrypter to read a new record. */
    void                SetupDecrypter();
    /** Initializes all members after base stream has been seeked to new raw record. */
    void                SetupRawRecord();
    /** Initializes all members after base stream has been seeked to new record. */
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
    sal_uInt16          GetMaxRawReadSize( sal_uInt32 nBytes ) const;

    /** Reads and decrypts nBytes bytes to the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt16          ReadRawData( void* pData, sal_uInt16 nBytes );

    /** Reads 8 bit/16 bit string length. */
    inline sal_uInt16   ReadByteStrLen( bool b16BitLen )
                            { return b16BitLen ? ReaduInt16() : ReaduInt8(); }

private:
    typedef ::std::vector< XclImpStreamPos > XclImpStreamPosStack;

    SvStream&           mrStrm;         /// Reference to the system input stream.
    const XclImpRoot&   mrRoot;         /// Filter root data.

    XclImpDecrypterRef  mxDecrypter;    /// Provides methods to decrypt data.

    XclImpStreamPos     maFirstRec;     /// Start position of current record.
    XclImpStreamPosStack maPosStack;    /// Stack for record positions.

    XclImpStreamPos     maGlobPos;      /// User defined position elsewhere in stream.
    sal_uInt16          mnGlobRecId;    /// Record ID for user defined position.
    bool                mbGlobValidRec; /// Was user position a valid record?
    bool                mbHasGlobPos;   /// Is user position defined?

    ULONG               mnStreamSize;   /// Size of system stream.
    ULONG               mnNextRecPos;   /// Start of next record header.
    sal_uInt32          mnCurrRecSize;  /// Helper for record position.
    sal_uInt32          mnComplRecSize; /// Size of complete record data (with CONTINUEs).
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

// ============================================================================

#endif

