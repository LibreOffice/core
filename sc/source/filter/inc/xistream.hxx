/*************************************************************************
 *
 *  $RCSfile: xistream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:38:00 $
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

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif
#ifndef SC_XLCONST_HXX
#include "xlconst.hxx"
#endif


/* ============================================================================
Input stream class for Excel import
- CONTINUE record handling
- ByteString and UniString support
============================================================================ */

// Decryption =================================================================

#define SC_XCL_USEDECR 0

#if SC_XCL_USEDECR
/** Base class for BIFF stream decryption. */
class XclImpDecrypter : ScfNoCopy
{
public:
    virtual                     ~XclImpDecrypter();

    /** Returns true, if the decrypter has been constructed successfully (especially
        with a valid password). */
    virtual bool                IsValid() const = 0;
    /** Initializes the key array offset which is dependent from record size. */
    virtual void                SetOffset( sal_uInt16 nRecSize ) = 0;
    /** Reads and decrypts nBytes bytes and stores data into the existing(!) buffer pData.
        @return  Count of bytes really read. */
    virtual sal_uInt32          ReadDecrypt( SvStream& rStrm, void* pData, sal_uInt32 nBytes ) = 0;

    /** Calculates the 16-bit hash value for the given password. */
    static sal_uInt16           GetHash( const ByteString& rPass );
    /** Calculates the BIFF2-BIFF7 decryption key for the given password. */
    static sal_uInt16           GetKey( const ByteString& rPass );
};


// ----------------------------------------------------------------------------

/** Decrypts BIFF5 stream contents using the given password and key. */
class XclImpBiff5Decrypter : public XclImpDecrypter
{
private:
    sal_uInt8                   mpKey[ 16 ];    /// Decryption key.
    sal_uInt32                  mnOffset;       /// Key/stream offset.
    bool                        mbIsValid;      /// true = Password is valid.

public:
    /** Constructs the decrypter.
        @param nKey  Password key from FILEPASS record.
        @param nHash  Password hash value from FILEPASS record. */
    explicit                    XclImpBiff5Decrypter( const String& rPass, sal_uInt16 nKey, sal_uInt16 nHash );

    /** Returns true, if the decrypter has been constructed successfully (especially
        with a valid password). */
    virtual bool                IsValid() const;
    /** Initializes the key array offset dependent from record size. */
    virtual void                SetOffset( sal_uInt16 nRecSize );
    /** Reads and decrypts nBytes bytes and stores data into the existing(!) buffer pData.
        @return  Count of bytes really read. */
    virtual sal_uInt32          ReadDecrypt( SvStream& rStrm, void* pData, sal_uInt32 nBytes );
};


// ----------------------------------------------------------------------------

/** Decrypts BIFF8 stream contents using the given password and key. */
class XclImpBiff8Decrypter : public XclImpDecrypter
{
public:
    explicit                    XclImpBiff8Decrypter( const String& rPass );

    /** Returns true, if the decrypter has been constructed successfully (especially
        with a valid password). */
    virtual bool                IsValid() const;
    /** Initializes the key array offset dependent from record size. */
    virtual void                SetOffset( sal_uInt16 nRecSize );
    /** Reads and decrypts nBytes bytes and stores data into the existing(!) buffer pData.
        @return  Count of bytes really read. */
    virtual sal_uInt32          ReadDecrypt( SvStream& rStrm, void* pData, sal_uInt32 nBytes );
};
#endif


// ============================================================================

/** This class represents an Excel stream position.
    @descr  It contains the relevant data for a stream position inside of a record
    (including CONTINUE records). */
class XclImpStreamPos
{
private:
    sal_uInt32                  mnPos;          /// Absolute position of the stream.
    sal_uInt32                  mnNextPos;      /// Absolute position of next record.
    sal_uInt32                  mnCurrSize;     /// Current calculated size of the record.
    sal_uInt32                  mnRecLeft;      /// Size left in current record.
    sal_uInt16                  mnRecSize;      /// Size of record (without CONTINUE).

public:
    /** Constructs the stream position data with the passed values. */
    explicit                    XclImpStreamPos(
                                    sal_uInt32 nStrmPos, sal_uInt32 nNextPos,
                                    sal_uInt32 nCurrSize, sal_uInt32 nRecLeft,
                                    sal_uInt16 nRecSize );

    /** Sets the stream position data to the passed values. */
    void                        Set(
                                    sal_uInt32 nStrmPos, sal_uInt32 nNextPos,
                                    sal_uInt32 nCurrSize, sal_uInt32 nRecLeft,
                                    sal_uInt16 nRecSize );

    /** Writes the contained stream position data to the given variables. */
    void                        Get(
                                    sal_uInt32& rnStrmPos, sal_uInt32& rnNextPos,
                                    sal_uInt32& rnCurrSize, sal_uInt32& rnRecLeft,
                                    sal_uInt16& rnRecSize ) const;
};


// ----------------------------------------------------------------------------

class XclImpRoot;

/** This class is used to import record oriented streams.
    @descr  An instance is constructed with an SvStream. The SvStream stream is
    reset to its start while constructing this stream.

    To start reading a record call StartNextRecord(). Now it is possible to read
    all contents of the record using operator>>() or any of the Read***() methods.
    If some data exceeds the record size limit, the stream looks for a following
    CONTINUE record and jumps automatically to it. It is NOT allowed that an atomic
    data type is split into two records (i.e. 4 bytes of a double in one record and
    the other 4 bytes in a following CONTINUE).

    Trying to read over the record limits results in a stream error. The IsValid()
    method indicates that with returning false. From now on it is undefined what data
    the read methods will return. The error state will be reset, if the record is
    reset (with the method InitializeRecord()) or if the next record is started.

    To switch off the automatic lookup of CONTINUE records, use InitializeRecord()
    with false parameter. This is useful i.e. on import of Escher objects, where
    sometimes solely CONTINUE records will occur. The automatic lookup keeps switched
    off until the method InitializeRecord() is called with parameter true.

    The import stream supports decrypting the stream data. The contents of a record
    (not the record header) will be encrypted by Excel if the file has been stored with
    password protection. The methods EnableDecryption() and UseDecryption() control
    the usage of the decryption algorithms. EnableDecryption() sets a decryption
    algorithm. UseDecryption() may be used to stop the usage of the decryption
    temporarily (sometimes record contents are never encrypted, i.e. all BOF records
    or the stream position in BOUNDSHEET). If decryption has been enabled
    with EnableDecryption(), the usage will be switched on automatically.

    It is possible to store several stream positions inside of a record (including
    its CONTINUE records). The positions are stored in a stack, which can be controlled
    with the methods PushPosition(), PopPosition() and RejectPosition(). The stack
    will be cleared whenever a new record is started (using the method
    StartNextRecord()).

    Additionally a single global stream position can be stored which keeps valid
    during the whole import process (methods StoreGlobalPosition(), SeekGlobalPosition()
    and DeleteGlobalPosition()). So it is possible to jump back to a previous record
    (that is a real jump without return).
*/
class XclImpStream
{
private:
#if SC_XCL_USEDECR
    typedef ::std::auto_ptr< XclImpDecrypter >  XclImpDecrypterPtr;
#endif
    typedef ScfDelStack< XclImpStreamPos >      XclImpStreamPosStack;

private:
    SvStream&                   mrStrm;         /// Reference to the system input stream.
    const XclImpRoot&           mrRoot;         /// Filter root data.

#if SC_XCL_USEDECR
    XclImpDecrypterPtr          mpDecrypter;    /// Provides methods to decrypt data.
#endif

    XclImpStreamPos             maFirstRec;     /// Start position of current record.
    XclImpStreamPosStack        maPosStack;     /// Stack for record positions.

    XclImpStreamPos             maGlobPos;      /// User defined position elsewhere in stream.
    sal_uInt16                  mnGlobRecId;    /// Record ID for user defined position.
    bool                        mbGlobValidRec; /// Was user position a valid record?
    bool                        mbHasGlobPos;   /// Is user position defined?

    sal_uInt32                  mnStreamSize;   /// Size of system stream.
    sal_uInt32                  mnNextRecPos;   /// Start of next record header.
    sal_uInt32                  mnCurrRecSize;  /// Helper for record position.
    sal_uInt32                  mnComplRecSize; /// Size of complete record data (with CONTINUEs).
    bool                        mbHasComplRec;  /// true = mnComplRecSize is valid.

    sal_uInt16                  mnRecId;        /// Current record ID (not the CONTINUE ID).
    sal_uInt16                  mnAltContId;    /// Alternative record ID for content continuation.
    sal_uInt16                  mnRecSize;      /// Size of current record content (without CONTINUE).
    sal_uInt32                  mnRecLeft;      /// Count of bytes left in current record.

    sal_Unicode                 mcNulSubst;     /// Replacement for NUL characters.

    bool                        mbCont;         /// Automatic CONTINUE lookup on/off.
#if SC_XCL_USEDECR
    bool                        mbUseDecr;      /// Usage of decryption.
#endif
    bool                        mbValidRec;     /// Read state: false = no record available.
    bool                        mbValid;        /// Read state: false = record overread.
    bool                        mbWarnings;     /// Enable/disable assertions.

public:
    /** Constructs the Excel record import stream.
        @param rInStrm  The system input stream. Will be set to its start position.
        @param bContHandling  Automatic CONTINUE lookup on/off. */
    explicit                    XclImpStream(
                                    SvStream& rInStrm,
                                    const XclImpRoot& rRoot,
                                    bool bContHandling = true );

                                ~XclImpStream();

    /** Returns the filter root data. */
    inline const XclImpRoot&    GetRoot() const { return mrRoot; }

    /** Sets stream pointer to the start of the next record content.
        @descr  Ignores all CONTINUE records of the current record, if automatic
        CONTINUE usage is switched on.
        @return  false = no record found (end of stream). */
    bool                        StartNextRecord();
    /** Sets stream pointer to begin of record content.
        @param bContHandling  Automatic CONTINUE lookup on/off.
        This setting is persistent until next call of this function.
        @param nAltContId  Sets an alternative record ID for content continuation.
        This value is reset automatically when a new record is started with
        StartNextRecord(). */
    void                        InitializeRecord( bool bContHandling, sal_uInt16 nAltContId = EXC_ID_UNKNOWN );

    /** Controls the appearance of overread warnings.
        @param bWarnMode  false = no overread assertions. */
    inline void                 SetWarningMode( bool bWarnMode ) { mbWarnings = bWarnMode; }

#if SC_XCL_USEDECR
    /** Enables decryption of record contents for the rest of the stream.
        @descr  Stream takes ownership of the decrypter object. */
    void                        EnableDecryption( XclImpDecrypter* pDecrypter );
    /** Switches usage of current decryption algorithm on/off. */
    void                        UseDecryption( bool bUse );
#endif

    /** Pushes current position on user position stack.
        @descr  This stack is emptied at every start of a new record. */
    void                        PushPosition();
    /** Seeks to last position from user position stack.
        @descr  This position will be removed from the stack. */
    void                        PopPosition();
    /** Removes last position from user position stack, but does not seek to it. */
    void                        RejectPosition();

    /** Stores current position. This position keeps valid in all records. */
    void                        StoreGlobalPosition();
    /** Seeks to the stored global user position. */
    void                        SeekGlobalPosition();
    /** Invalidates global user position. */
    inline void                 DeleteGlobalPosition() { mbHasGlobPos = false; }

    /** Returns read state: false = record overread. */
    inline bool                 IsValid() const { return mbValid; }
    /** Returns the current record ID. */
    inline sal_uInt16           GetRecId() const { return mnRecId; }
    /** Returns the position inside of the whole record content. */
    inline sal_uInt32           GetRecPos() const
                                    { return IsValid() ? mnCurrRecSize - mnRecLeft : 0; }
    /** Returns the data size of the whole record without record headers. */
    sal_uInt32                  GetRecSize();
    /** Returns remaining data size of the whole record without record headers. */
    inline sal_uInt32           GetRecLeft()
                                    { return IsValid() ? GetRecSize() - GetRecPos() : 0; }

    inline XclImpStream&        operator>>( sal_Int8& rnValue );
    inline XclImpStream&        operator>>( sal_uInt8& rnValue );
    inline XclImpStream&        operator>>( sal_Int16& rnValue );
    inline XclImpStream&        operator>>( sal_uInt16& rnValue );
    inline XclImpStream&        operator>>( sal_Int32& rnValue );
    inline XclImpStream&        operator>>( sal_uInt32& rnValue );
    inline XclImpStream&        operator>>( float& rfValue );
    inline XclImpStream&        operator>>( double& rfValue );

    sal_Int8                    ReadInt8();
    sal_uInt8                   ReaduInt8();
    sal_Int16                   ReadInt16();
    sal_uInt16                  ReaduInt16();
    sal_Int32                   ReadInt32();
    sal_uInt32                  ReaduInt32();
    float                       ReadFloat();
    double                      ReadDouble();

    /** Reads nBytes bytes to the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt32                  Read( void* pData, sal_uInt32 nBytes );
    /** Copies nBytes bytes to rOutStrm.
        @return  Count of bytes really written. */
    sal_uInt32                  CopyToStream( SvStream& rOutStrm, sal_uInt32 nBytes );
    /** Copies the entire record to rOutStrm. The current record position keeps unchanged.
        @return  Count of bytes really written. */
    sal_uInt32                  CopyRecordToStream( SvStream& rOutStrm );

    /** Seeks absolute in record content to the specified position.
        @descr  The value 0 means start of record, independent from physical stream position. */
    void                        Seek( sal_uInt32 nPos );
    /** Seeks forward inside the current record. */
    void                        Ignore( sal_uInt32 nBytes );


    // *** UNICODE STRINGS ***
    // structure of an Excel unicode string:
    // (1) 2 byte character count
    // (2) 1 byte flags (16-bit-characters, rich string, far east string)
    // (3) [2 byte rich string format run count]
    // (4) [4 byte far east data size]
    // (5) character array
    // (6) [4 * (rich string format run count) byte]
    // (7) [(far east data size) byte]
    // header = (1), (2)
    // ext. header = (3), (4)
    // ext. data = (6), (7)

    // *** special string functions ***

    /** Reads ext. header, detects 8/16 bit mode, sets all ext. info.
        @return  Size of ext. data. */
    sal_uInt32                  ReadUniStringExtHeader(
                                    bool& rb16Bit, bool& rbRich, bool& rbFareast,
                                    sal_uInt16& rnCrun, sal_uInt32& rnExtInf, sal_uInt8 nFlags );
    /** Seeks to begin of character array, detects 8/16 bit mode.
        @return  Size of ext. data. */
    sal_uInt32                  ReadUniStringExtHeader( bool& rb16Bit, sal_uInt8 nFlags );
    /** Skips ext. data after character array. */
    inline void                 SkipUniStringExtData( sal_uInt32 nExtSize )
                                    { Ignore( nExtSize ); }

    /** Sets a replacement character for NUL characters.
        @descr  NUL characters must be replaced, because Tools strings cannot handle them.
        @param cNulSubst  The character to use for NUL replacement. It is possible to specify
        NUL here. in this case strings are terminated when the first NUL occurs during string import. */
    inline void                 SetNulSubstChar( sal_Unicode cNulSubst = '?' ) { mcNulSubst = cNulSubst; }

    // *** read/ignore unicode strings ***
    // - look for CONTINUE records even if CONTINUE handling disabled
    //   (only if inside of a CONTINUE record - for TXO import)
    // - no overread assertions (for Applix wrong string length export bug)

    /** Reads nChars characters and appends string to rString. */
    void                        AppendRawUniString( String& rString, sal_uInt16 nChars, bool b16Bit );
    /** Reads ext. header, nChar characters, ext. data and appends string to rString. */
    void                        AppendUniString( String& rString, sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Reads 8 bit flags, ext. header, nChar characters, ext. data and appends string to rString. */
    inline void                 AppendUniString( String& rString, sal_uInt16 nChars );
    /** Reads 16 bit character count, 8 bit flags, ext. header, character array,
        ext. data and appends string to rString. */
    inline void                 AppendUniString( String& rString );

    /** Reads nChars characters and returns the string. */
    String                      ReadRawUniString( sal_uInt16 nChars, bool b16Bit );
    /** Reads ext. header, nChar characters, ext. data and returns the string. */
    String                      ReadUniString( sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Reads 8 bit flags, ext. header, nChar characters, ext. data and returns the string. */
    String                      ReadUniString( sal_uInt16 nChars );
    /** Reads 16 bit character count, 8 bit flags, ext. header, character array,
        ext. data and returns the string. */
    String                      ReadUniString();

    /** Ignores nChars characters. */
    void                        IgnoreRawUniString( sal_uInt16 nChars, bool b16Bit );
    /** Ignores ext. header, nChar characters, ext. data. */
    void                        IgnoreUniString( sal_uInt16 nChars, sal_uInt8 nFlags );
    /** Ignores 8 bit flags, ext. header, nChar characters, ext. data. */
    inline void                 IgnoreUniString( sal_uInt16 nChars );
    /** Ignores 16 bit character count, 8 bit flags, ext. header, character array, ext. data. */
    inline void                 IgnoreUniString();

    // *** read/ignore 8-bit-strings, store in String ***

    /** Reads nChar byte characters and appends string to rString. */
    inline void                 AppendRawByteString( String& rString, sal_uInt16 nChars );
    /** Reads 8/16 bit string length, character array and appends string to rString. */
    inline void                 AppendByteString( String& rString, bool b16BitLen );

    /** Reads nChar byte characters and returns the string. */
    String                      ReadRawByteString( sal_uInt16 nChars );
    /** Reads 8/16 bit string length, character array and returns the string. */
    String                      ReadByteString( bool b16BitLen );

    /** Ignores nChar byte characters. */
    inline void                 IgnoreRawByteString( sal_uInt16 nChars );
    /** Ignores 8/16 bit string length, character array. */
    inline void                 IgnoreByteString( bool b16BitLen );

    // *** SvStream functions ***

    /** Returns the absolute stream position. */
    sal_uInt32                  Tell() const;
    /** Returns the stream size. */
    inline sal_uInt32           GetStreamSize() const { return mnStreamSize; }

private:
#if SC_XCL_USEDECR
    /** Initializes the key/stream offset of the decrypter. */
    inline void                 SetDecrypterOffset( sal_uInt16 nRecSize );
#endif

    /** Reads and decrypts a sal_Int8 value. */
    void                        ReadAtom( sal_Int8& rnValue );
    /** Reads and decrypts a sal_uInt8 value. */
    void                        ReadAtom( sal_uInt8& rnValue );
    /** Reads and decrypts a sal_Int16 value. */
    void                        ReadAtom( sal_Int16& rnValue );
    /** Reads and decrypts a sal_uInt16 value. */
    void                        ReadAtom( sal_uInt16& rnValue );
    /** Reads and decrypts a sal_Int32 value. */
    void                        ReadAtom( sal_Int32& rnValue );
    /** Reads and decrypts a sal_uInt32 value. */
    void                        ReadAtom( sal_uInt32& rnValue );
    /** Reads and decrypts a float value. */
    void                        ReadAtom( float& rfValue );
    /** Reads and decrypts a double value. */
    void                        ReadAtom( double& rfValue );
    /** Reads and decrypts nBytes bytes to the existing(!) buffer pData.
        @return  Count of bytes really read. */
    sal_uInt32                  ReadData( void* pData, sal_uInt32 nBytes );

    /** Internal start of a new record, doesn't change mnNextRecPos and mbValid.
        @return  false = no record found (end of stream). */
    bool                        GetNextRecord( sal_uInt16& rnRecId, sal_uInt16& rnRecSize );
    /** Internal setup of a new record, expecting mnRecId and mnNextRecPos. */
    void                        SetupRecord();
    /** Returns true, if the passed ID is a real or an alternative continuation record ID. */
    bool                        IsContinueId( sal_uInt16 nRecId );
    /** Looks for and goes to a following CONTINUE record.
        @descr  Does not change mbValid, updates mnCurrRecSize. */
    bool                        GetContinue();

    /** Checks mnRecLeft and jumps into next CONTINUE record if necessary and mbCont is true. */
    bool                        CheckDataLeft( sal_uInt32 nBytes );

    /** Goes to start of the next CONTINUE record.
        @descr  Sets mbValid, mnNextRecPos and mnRecLeft. */
    void                        StartContinue();
    /** Goes to start of the next CONTINUE record while reading strings.
        @descr  Sets mbValid, mnNextRecPos and mnRecLeft, reads additional Unicode
        flag byte and sets/resets rb16Bit. */
    void                        StartStringContinue( bool& rb16Bit );

    /** Reads 8 bit/16 bit string length. */
    inline sal_uInt16           ReadByteStrLen( bool b16BitLen )
                                    { return b16BitLen ? ReaduInt16() : ReaduInt8(); }

    /** Restores stream position contained in rPos. */
    void                        RestorePosition( const XclImpStreamPos& rPos );
};


// ----------------------------------------------------------------------------

inline XclImpStream& XclImpStream::operator>>( sal_Int8& rnValue )
{
    if( CheckDataLeft( 1 ) ) ReadAtom( rnValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( sal_uInt8& rnValue )
{
    if( CheckDataLeft( 1 ) ) ReadAtom( rnValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( sal_Int16& rnValue )
{
    if( CheckDataLeft( 2 ) ) ReadAtom( rnValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( sal_uInt16& rnValue )
{
    if( CheckDataLeft( 2 ) ) ReadAtom( rnValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( sal_Int32& rnValue )
{
    if( CheckDataLeft( 4 ) ) ReadAtom( rnValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( sal_uInt32& rnValue )
{
    if( CheckDataLeft( 4 ) ) ReadAtom( rnValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( float& rfValue )
{
    if( CheckDataLeft( 4 ) ) ReadAtom( rfValue );
    return *this;
}

inline XclImpStream& XclImpStream::operator>>( double& rfValue )
{
    if( CheckDataLeft( 8 ) ) ReadAtom( rfValue );
    return *this;
}


// ----------------------------------------------------------------------------

inline void XclImpStream::AppendUniString( String& rString, sal_uInt16 nChars )
{
    AppendUniString( rString, nChars, ReaduInt8() );
}

inline void XclImpStream::AppendUniString( String& rString )
{
    AppendUniString( rString, ReaduInt16() );
}

inline void XclImpStream::IgnoreUniString( sal_uInt16 nChars )
{
    IgnoreUniString( nChars, ReaduInt8() );
}

inline void XclImpStream::IgnoreUniString()
{
    IgnoreUniString( ReaduInt16() );
}


// ----------------------------------------------------------------------------

inline void XclImpStream::AppendRawByteString( String& rString, sal_uInt16 nChars )
{
    AppendRawUniString( rString, nChars, false );
}

inline void XclImpStream::AppendByteString( String& rString, bool b16BitLen )
{
    AppendRawByteString( rString, ReadByteStrLen( b16BitLen ) );
}

inline void XclImpStream::IgnoreRawByteString( sal_uInt16 nChars )
{
    Ignore( nChars );
}

inline void XclImpStream::IgnoreByteString( bool b16BitLen )
{
    IgnoreRawByteString( ReadByteStrLen( b16BitLen ) );
}


// ----------------------------------------------------------------------------

#if SC_XCL_USEDECR
inline void XclImpStream::SetDecrypterOffset( sal_uInt16 nRecSize )
{
    if( mpDecrypter.get() )
        mpDecrypter->SetOffset( nRecSize );
}
#endif


// ============================================================================

#endif

