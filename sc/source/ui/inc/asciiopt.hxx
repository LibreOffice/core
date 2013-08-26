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

// ============================================================================

#ifndef SC_ASCIIOPT_HXX
#define SC_ASCIIOPT_HXX

#include <tools/string.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <tools/stream.hxx>
#include <svx/txencbox.hxx>
#include "csvtablebox.hxx"
#include "i18nlangtag/lang.h"

// ============================================================================

class ScAsciiOptions
{
private:
    bool        bFixedLen;
    String      aFieldSeps;
    bool        bMergeFieldSeps;
    bool        bQuotedFieldAsText;
    bool        bDetectSpecialNumber;
    sal_Unicode cTextSep;
    CharSet     eCharSet;
    LanguageType eLang;
    bool        bCharSetSystem;
    long        nStartRow;
    sal_uInt16      nInfoCount;
    sal_Int32* pColStart;  //! TODO replace with vector
    sal_uInt8*       pColFormat; //! TODO replace with vector

public:
                    ScAsciiOptions();
                    ScAsciiOptions(const ScAsciiOptions& rOpt);
                    ~ScAsciiOptions();

    static const sal_Unicode cDefaultTextSep = '"';

    ScAsciiOptions& operator=( const ScAsciiOptions& rCpy );

    bool            operator==( const ScAsciiOptions& rCmp ) const;

    void            ReadFromString( const String& rString );
    String          WriteToString() const;

    void            InterpretColumnList( const String& rString );

    CharSet             GetCharSet() const      { return eCharSet; }
    bool                GetCharSetSystem() const    { return bCharSetSystem; }
    const String&       GetFieldSeps() const    { return aFieldSeps; }
    bool                IsMergeSeps() const     { return bMergeFieldSeps; }
    bool                IsQuotedAsText() const  { return bQuotedFieldAsText; }
    bool                IsDetectSpecialNumber() const { return bDetectSpecialNumber; }
    sal_Unicode         GetTextSep() const      { return cTextSep; }
    bool                IsFixedLen() const      { return bFixedLen; }
    sal_uInt16          GetInfoCount() const    { return nInfoCount; }
    const sal_Int32*    GetColStart() const     { return pColStart; }
    const sal_uInt8*    GetColFormat() const    { return pColFormat; }
    long                GetStartRow() const     { return nStartRow; }
    LanguageType        GetLanguage() const     { return eLang; }

    void    SetCharSet( CharSet eNew )          { eCharSet = eNew; }
    void    SetCharSetSystem( bool bSet )       { bCharSetSystem = bSet; }
    void    SetFixedLen( bool bSet )            { bFixedLen = bSet; }
    void    SetFieldSeps( const String& rStr )  { aFieldSeps = rStr; }
    void    SetMergeSeps( bool bSet )           { bMergeFieldSeps = bSet; }
    void    SetQuotedAsText(bool bSet)          { bQuotedFieldAsText = bSet; }
    void    SetDetectSpecialNumber(bool bSet)   { bDetectSpecialNumber = bSet; }
    void    SetTextSep( sal_Unicode c )         { cTextSep = c; }
    void    SetStartRow( long nRow)             { nStartRow= nRow; }
    void    SetLanguage(LanguageType e)         { eLang = e; }

    void    SetColInfo( sal_uInt16 nCount, const sal_Int32* pStart, const sal_uInt8* pFormat );
    void    SetColumnInfo( const ScCsvExpDataVec& rDataVec );

    /** From the import field separators obtain the one most likely to be used
        for export, if multiple separators weighted comma, tab, semicolon,
        space and other.

        @param  bDecodeNumbers
                If TRUE, the separators are encoded as numbers and need to be
                decoded before characters can be extracted, for example "59/44"
                to ";,".
                If FALSE, the string is taken as is and each character is
                expected to be one separator.
     */
    static sal_Unicode  GetWeightedFieldSep( const String & rFieldSeps, bool bDecodeNumbers );
};

/// How ScImportAsciiDlg is called
enum ScImportAsciiCall {
        SC_IMPORTFILE,           // with File > Open: Text - CSV
        SC_PASTETEXT,            // with Paste > Unformatted Text
        SC_TEXTTOCOLUMNS };      // with Data > Text to Columns

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
