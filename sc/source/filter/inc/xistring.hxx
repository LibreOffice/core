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

#include <rtl/ustring.hxx>
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
    explicit            XclImpString( const OUString& rString );

    /** Reads a complete string from the passed stream. */
    void                Read( XclImpStream& rStrm, XclStrFlags nFlags = XclStrFlags::NONE );

    /** Sets the passed string data. */
    void         SetText( const OUString& rText ) { maString = rText; }
    /** Sets the passed formatting buffer. */
    void         SetFormats( const XclFormatRunVec& rFormats ) { maFormats = rFormats; }
    /** Reads and appends the formatting information (run count and runs) from stream. */
    void         ReadFormats( XclImpStream& rStrm ) { ReadFormats( rStrm, maFormats ); }
    /** Reads and appends formatting runs from an OBJ or TXO record. */
    void         ReadObjFormats( XclImpStream& rStrm, sal_uInt16 nFormatSize ) { ReadObjFormats( rStrm, maFormats, nFormatSize ); }

    /** Returns true, if the string is empty. */
    bool         IsEmpty() const { return maString.isEmpty(); }
    /** Returns the pure text data of the string. */
    const OUString& GetText() const { return maString; }

    /** Returns true, if the string contains formatting information. */
    bool         IsRich() const { return !maFormats.empty(); }
    /** Returns the formatting run vector. */
    const XclFormatRunVec& GetFormats() const { return maFormats; }

    /** Insert a formatting run to the passed format buffer. */
    static void         AppendFormat( XclFormatRunVec& rFormats, sal_uInt16 nChar, sal_uInt16 nFontIdx );
    /** Reads and appends the formatting information (run count and runs) from stream. */
    static void         ReadFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats );
    /** Reads and appends nRunCount formatting runs from stream. */
    static void         ReadFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats, sal_uInt16 nRunCount );
    /** Reads and appends formatting runs from an OBJ or TXO record. */
    static void         ReadObjFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats, sal_uInt16 nFormatSize );

private:
    OUString            maString;       /// The text data of the string.
    XclFormatRunVec     maFormats;      /// All formatting runs.
};

// String iterator ============================================================

/** Iterates over formatted string portions. */
class XclImpStringIterator
{
public:
    explicit            XclImpStringIterator( const XclImpString& rString );

    /** Returns true, if the iterator references a valid text portion. */
    bool         Is() const { return mnTextBeg < mrText.getLength(); }
    /** Returns the index of the current text portion. */
    size_t       GetPortionIndex() const { return mnPortion; }
    /** Returns the string of the current text portion. */
    OUString            GetPortionText() const;
    /** Returns the font index of the current text portion. */
    sal_uInt16          GetPortionFont() const;

    /** Moves iterator to next text portion. */
    XclImpStringIterator& operator++();

private:
    const OUString&     mrText;         /// The processed string.
    const XclFormatRunVec& mrFormats;   /// The vector of formatting runs.
    size_t              mnPortion;      /// Current text portion.
    sal_Int32           mnTextBeg;      /// First character of current portion.
    sal_Int32           mnTextEnd;      /// First character of next portion.
    size_t              mnFormatsBeg;   /// Formatting run index for current portion.
    size_t              mnFormatsEnd;   /// Formatting run index for next portion.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
