/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XISTRING_HXX
#define SC_XISTRING_HXX

#include "xlstring.hxx"

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
    /** Reads and appends formatting runs from an OBJ or TXO record. */
    inline void         ReadObjFormats( XclImpStream& rStrm, sal_uInt16 nFormatSize ) { ReadObjFormats( rStrm, maFormats, nFormatSize ); }

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
    /** Reads and appends formatting runs from an OBJ or TXO record. */
    static void         ReadObjFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats, sal_uInt16 nFormatSize );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
