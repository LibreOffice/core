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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ASCIIOPT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ASCIIOPT_HXX

#include <rtl/ustring.hxx>
#include <i18nlangtag/lang.h>

#include "csvcontrol.hxx"

class ScAsciiOptions
{
private:
    bool        bFixedLen;
    OUString    aFieldSeps;
    bool        bMergeFieldSeps;
    bool        bRemoveSpace;
    bool        bQuotedFieldAsText;
    bool        bDetectSpecialNumber;
    bool        bSkipEmptyCells;
    bool        bSaveAsShown;
    bool        bSaveFormulas;
    sal_Unicode cTextSep;
    rtl_TextEncoding eCharSet;
    LanguageType eLang;
    bool        bCharSetSystem;
    long        nStartRow;
    std::vector<sal_Int32> mvColStart;
    std::vector<sal_uInt8> mvColFormat;

public:
                    ScAsciiOptions();

    static const sal_Unicode cDefaultTextSep = '"';

    void            ReadFromString( const OUString& rString );
    OUString        WriteToString() const;

    rtl_TextEncoding    GetCharSet() const      { return eCharSet; }
    const OUString&     GetFieldSeps() const    { return aFieldSeps; }
    bool                IsMergeSeps() const     { return bMergeFieldSeps; }
    bool                IsRemoveSpace() const   { return bRemoveSpace; }
    bool                IsQuotedAsText() const  { return bQuotedFieldAsText; }
    bool                IsDetectSpecialNumber() const { return bDetectSpecialNumber; }
    bool                IsSkipEmptyCells() const      { return bSkipEmptyCells; }
    sal_Unicode         GetTextSep() const      { return cTextSep; }
    bool                IsFixedLen() const      { return bFixedLen; }
    sal_uInt16          GetInfoCount() const    { return mvColStart.size(); }
    const sal_Int32*    GetColStart() const     { return mvColStart.data(); }
    const sal_uInt8*    GetColFormat() const    { return mvColFormat.data(); }
    long                GetStartRow() const     { return nStartRow; }
    LanguageType        GetLanguage() const     { return eLang; }

    void    SetCharSet( rtl_TextEncoding eNew ) { eCharSet = eNew; }
    void    SetCharSetSystem( bool bSet )       { bCharSetSystem = bSet; }
    void    SetFixedLen( bool bSet )            { bFixedLen = bSet; }
    void    SetFieldSeps( const OUString& rStr )  { aFieldSeps = rStr; }
    void    SetMergeSeps( bool bSet )           { bMergeFieldSeps = bSet; }
    void    SetRemoveSpace( bool bSet )         { bRemoveSpace = bSet; }
    void    SetQuotedAsText(bool bSet)          { bQuotedFieldAsText = bSet; }
    void    SetDetectSpecialNumber(bool bSet)   { bDetectSpecialNumber = bSet; }
    void    SetSkipEmptyCells(bool bSet)        { bSkipEmptyCells = bSet; }
    void    SetTextSep( sal_Unicode c )         { cTextSep = c; }
    void    SetStartRow( long nRow)             { nStartRow= nRow; }
    void    SetLanguage(LanguageType e)         { eLang = e; }

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
    static sal_Unicode  GetWeightedFieldSep( const OUString & rFieldSeps, bool bDecodeNumbers );
};

/// How ScImportAsciiDlg is called
enum ScImportAsciiCall {
        SC_IMPORTFILE,           // with File > Open: Text - CSV
        SC_PASTETEXT,            // with Paste > Unformatted Text
        SC_TEXTTOCOLUMNS };      // with Data > Text to Columns

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
