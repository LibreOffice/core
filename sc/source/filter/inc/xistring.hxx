/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xistring.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-05-23 15:21:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XISTRING_HXX
#define SC_XISTRING_HXX

#ifndef SC_XLSTRING_HXX
#include "xlstring.hxx"
#endif

// Byte/Unicode strings =======================================================

class XclImpStream;

/** This class represents an unformatted or formatted string and provides importing from stream. */
class XclImpString
{
public:
    /** Constructs an empty string. */
    explicit            XclImpString();
    /** Constructs an unformatted string. */
    explicit            XclImpString( const String& rString );

                        ~XclImpString();

    /** Reads a complete string from the passed stream. */
    void                Read( XclImpStream& rStrm, XclStrFlags nFlags = EXC_STR_DEFAULT );

    /** Sets the passed string data. */
    inline void         SetText( const String& rText ) { maString = rText; }
    /** Sets the passed formatting buffer. */
    inline void         SetFormats( const XclFormatRunVec& rFormats ) { maFormats = rFormats; }
    /** Insert a formatting run to the format buffer. */
    inline void         AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIdx ) { AppendFormat( maFormats, nChar, nFontIdx ); }
    /** Reads and appends the formatting information (run count and runs) from stream. */
    inline void         ReadFormats( XclImpStream& rStrm ) { ReadFormats( rStrm, maFormats ); }
    /** Reads and appends nRunCount formatting runs from stream. */
    inline void         ReadFormats( XclImpStream& rStrm, sal_uInt16 nRunCount ) { ReadFormats( rStrm, maFormats, nRunCount ); }

    /** Returns true, if the string is empty. */
    inline bool         IsEmpty() const { return maString.Len() == 0; }
    /** Returns the pure text data of the string. */
    inline const String& GetText() const { return maString; }

    /** Returns true, if the string contains formatting information. */
    inline bool         IsRich() const { return !maFormats.empty(); }
    /** Returns the formatting run vector. */
    inline const XclFormatRunVec& GetFormats() const { return maFormats; }

    /** Insert a formatting run to the passed format buffer. */
    static void         AppendFormat( XclFormatRunVec& rFormats, sal_uInt16 nChar, sal_uInt16 nFontIdx );
    /** Reads and appends the formatting information (run count and runs) from stream. */
    static void         ReadFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats );
    /** Reads and appends nRunCount formatting runs from stream. */
    static void         ReadFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats, sal_uInt16 nRunCount );

private:
    String              maString;       /// The text data of the string.
    XclFormatRunVec     maFormats;      /// All formatting runs.
};

// String iterator ============================================================

/** Iterates over formatted string portions. */
class XclImpStringIterator
{
public:
    explicit            XclImpStringIterator( const XclImpString& rString );

    /** Returns true, if the iterator references a valid text portion. */
    inline bool         Is() const { return mnTextBeg < mrText.Len(); }
    /** Returns the index of the current text portion. */
    inline size_t       GetPortionIndex() const { return mnPortion; }
    /** Returns the string of the current text portion. */
    String              GetPortionText() const;
    /** Returns the font index of the current text portion. */
    sal_uInt16          GetPortionFont() const;

    /** Moves iterator to next text portion. */
    XclImpStringIterator& operator++();

private:
    const String&       mrText;         /// The processed string.
    const XclFormatRunVec& mrFormats;   /// The vector of formatting runs.
    size_t              mnPortion;      /// Current text portion.
    xub_StrLen          mnTextBeg;      /// First character of current portion.
    xub_StrLen          mnTextEnd;      /// First character of next portion.
    size_t              mnFormatsBeg;   /// Formatting run index for current portion.
    size_t              mnFormatsEnd;   /// Formatting run index for next portion.
};

// ============================================================================

#endif

