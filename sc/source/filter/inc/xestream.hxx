/*************************************************************************
 *
 *  $RCSfile: xestream.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:05:34 $
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

#ifndef SC_XESTREAM_HXX
#define SC_XESTREAM_HXX

#ifndef SC_XLSTREAM_HXX
#include "xlstream.hxx"
#endif

/* ============================================================================
Output stream class for Excel export
- CONTINUE record handling
- ByteString and UniString support
============================================================================ */

class XclExpRoot;

/** This class is used to export Excel record streams.
    @descr  An instance is constructed with an SvStream and the maximum size of Excel
    record contents (in BIFF5: 2080 bytes, in BIFF8: 8224 bytes).

    To start writing a record call StartRecord(). Parameters are the record identifier
    and any calculated record size. This is for optimizing the write process: if the real
    written data has the same size as the calculated, the stream will not seek back and
    update the record size field. But it is not mandatory to calculate a size. Each
    record must be closed by calling EndRecord(). This will check (and update) the record
    size field.

    If some data exceeds the record size limit, a CONTINUE record is started automatically
    and the new data will be written to this record.

    If specific data pieces must not be splitted, use SetSliceLen(). For instance:
    To write a sequence of 16-bit values, where 4 values form a unit and cannot be
    split, call SetSliceLen( 8 ) first (4*2 bytes == 8).

    To write unicode character arrays, call WriteUnicodeBuffer(). It creates CONTINUE
    records and repeats the unicode string flag byte automatically. This function is used
    for instance from the class XclExpString which can write complete unicode strings.
*/
class XclExpStream
{
private:
    SvStream&                   mrStrm;         /// Reference to the system output stream.
    const XclExpRoot&           mrRoot;         /// Filter root data.

                                // length data
    sal_uInt32                  mnMaxRecSize;   /// Maximum size of record content.
    sal_uInt32                  mnMaxContSize;  /// Maximum size of CONTINUE content.
    sal_uInt32                  mnCurrMaxSize;  /// Current maximum, either mnMaxRecSize or mnMaxContSize.
    sal_uInt32                  mnMaxSliceSize; /// Maximum size of data slices (parts that cannot be split).
    sal_uInt32                  mnCalcSize;     /// Calculated size received from calling function.
    sal_uInt32                  mnHeaderSize;   /// Record size written in last record header.
    sal_uInt32                  mnCurrSize;     /// Count of bytes already written in current record.
    sal_uInt32                  mnSliceSize;    /// Count of bytes already written in current slice.

                                // stream position data
    sal_uInt32                  mnLastSizePos;  /// Stream position of size field in current header.
    bool                        mbInRec;        /// true = currently writing inside of a record.

public:
    /** Constructs the Excel record export stream.
        @param rOutStrm  The system output stream to write to.
        @param nMaxRecSize  The maximum allowed size of record content (depending on BIFF type).
        If 0 is passed, the record size will be set automatically, depending on the current BIFF type. */
                                XclExpStream(
                                    SvStream& rOutStrm,
                                    const XclExpRoot& rRoot,
                                    sal_uInt32 nMaxRecSize = 0 );

                                ~XclExpStream();

    /** Returns the filter root data. */
    inline const XclExpRoot&    GetRoot() const { return mrRoot; }

    /** Starts a new record: writes header data, stores calculated record size. */
    void                        StartRecord( sal_uInt16 nRecId, sal_uInt32 nRecSize );
    /** Checks and corrects real record length. Must be called everytime a record is finished. */
    void                        EndRecord();

    /** Returns the position inside of current record (starts by 0 in every CONTINUE). */
    inline sal_uInt32           GetRecPos() const                   { return mnCurrSize; }

    /** Returns the maximum size of a record. */
    inline sal_uInt32           GetMaxRecSize() const               { return mnMaxRecSize; }
    /** Sets maximum record size (valid only for current record). */
    inline void                 SetMaxRecSize( sal_uInt32 nMax )    { mnCurrMaxSize = nMax; }
    /** Sets maximum size of CONTINUE records (valid only for current record). */
    inline void                 SetMaxContSize( sal_uInt32 nMax )   { mnMaxContSize = nMax; }

    /** Sets data slice length. 0 = no slices. */
    void                        SetSliceSize( sal_uInt32 nSize );

    inline XclExpStream&        operator<<( sal_Int8 nValue );
    inline XclExpStream&        operator<<( sal_uInt8 nValue );
    inline XclExpStream&        operator<<( sal_Int16 nValue );
    inline XclExpStream&        operator<<( sal_uInt16 nValue );
    inline XclExpStream&        operator<<( sal_Int32 nValue );
    inline XclExpStream&        operator<<( sal_uInt32 nValue );
    inline XclExpStream&        operator<<( float fValue );
    inline XclExpStream&        operator<<( double fValue );

    /** Writes nBytes bytes from memory. */
    sal_uInt32                  Write( const void* pData, sal_uInt32 nBytes );
    /** Writes a sequence of nBytes zero bytes (respects slice setting). */
    void                        WriteZeroBytes( sal_uInt32 nBytes );
    /** Copies nBytes bytes from current position of the stream rInStrm.
        @descr  Omitting the second parameter means: read to end of stream. */
    sal_uInt32                  CopyFromStream( SvStream& rInStrm, sal_uInt32 nBytes = STREAM_SEEK_TO_END );

    // *** unicode string export is realized with helper class XclExpString ***
    // (slice length setting has no effect here -> disabled automatically)

    /** Writes Unicode buffer as 8/16 bit, repeats nFlags at start of a CONTINUE record. */
    void                        WriteUnicodeBuffer( const sal_uInt16* pBuffer, sal_uInt32 nChars, sal_uInt8 nFlags );
    /** Writes Unicode buffer as 8/16 bit, repeats nFlags at start of a CONTINUE record. */
    void                        WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags );

    // *** write 8-bit-strings ***
    // (slice length setting has no effect here -> disabled automatically)

    /** Writes ByteString buffer (without string length field). */
    void                        WriteByteStringBuffer(
                                    const ByteString& rString,
                                    sal_uInt16 nMaxLen = 0x00FF );
    /** Writes string length field and ByteString buffer. */
    void                        WriteByteString(
                                    const ByteString& rString,
                                    sal_uInt16 nMaxLen = 0x00FF,
                                    bool b16BitCount = false );

    /** Writes 8-bit character buffer. */
    void                        WriteCharBuffer( const ScfUInt8Vec& rBuffer );

    // *** SvStream access ***

    /** Sets position of system stream (only allowed outside of records). */
    sal_uInt32                  SetStreamPos( sal_uInt32 nPos );
    /** Returns the absolute position of the system stream. */
    inline sal_uInt32           GetStreamPos() const { return mrStrm.Tell(); }

private:
    /** Writes header data, internal setup. */
    void                        InitRecord( sal_uInt16 nRecId );
    /** Rewrites correct record length, if different from calculated. */
    void                        UpdateRecSize();
    /** Recalculates mnCurrSize and mnSliceSize. */
    void                        UpdateSizeVars( sal_uInt32 nSize );
    /** Writes CONTINUE header, internal setup. */
    void                        StartContinue();
    /** Refreshes counter vars, creates CONTINUE records. */
    void                        PrepareWrite( sal_uInt32 nSize );
    /** Creates CONTINUE record at end of record.
        @return  Maximum data block size remaining. */
    sal_uInt32                  PrepareWrite();

    /** Writes a raw sequence of zero bytes. */
    void                        WriteRawZeroBytes( sal_uInt32 nBytes );
};


inline XclExpStream& XclExpStream::operator<<( sal_Int8 nValue )
{
    PrepareWrite( 1 );
    mrStrm << nValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( sal_uInt8 nValue )
{
    PrepareWrite( 1 );
    mrStrm << nValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( sal_Int16 nValue )
{
    PrepareWrite( 2 );
    mrStrm << nValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( sal_uInt16 nValue )
{
    PrepareWrite( 2 );
    mrStrm << nValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( sal_Int32 nValue )
{
    PrepareWrite( 4 );
    mrStrm << nValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( sal_uInt32 nValue )
{
    PrepareWrite( 4 );
    mrStrm << nValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( float fValue )
{
    PrepareWrite( 4 );
    mrStrm << fValue;
    return *this;
}

inline XclExpStream& XclExpStream::operator<<( double fValue )
{
    PrepareWrite( 8 );
    mrStrm << fValue;
    return *this;
}


// ============================================================================

#endif

