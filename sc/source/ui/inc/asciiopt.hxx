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

#pragma once
#if 1

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
#include "i18npool/lang.h"

// ============================================================================

class ScAsciiOptions
{
private:
    sal_Bool        bFixedLen;
    String      aFieldSeps;
    sal_Bool        bMergeFieldSeps;
    bool        bQuotedFieldAsText;
    bool        bDetectSpecialNumber;
    sal_Unicode cTextSep;
    CharSet     eCharSet;
    LanguageType eLang;
    sal_Bool        bCharSetSystem;
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

    sal_Bool            operator==( const ScAsciiOptions& rCmp ) const;

    void            ReadFromString( const String& rString );
    String          WriteToString() const;

    void            InterpretColumnList( const String& rString );

    CharSet             GetCharSet() const      { return eCharSet; }
    sal_Bool                GetCharSetSystem() const    { return bCharSetSystem; }
    const String&       GetFieldSeps() const    { return aFieldSeps; }
    sal_Bool                IsMergeSeps() const     { return bMergeFieldSeps; }
    bool                IsQuotedAsText() const  { return bQuotedFieldAsText; }
    bool                IsDetectSpecialNumber() const { return bDetectSpecialNumber; }
    sal_Unicode         GetTextSep() const      { return cTextSep; }
    sal_Bool                IsFixedLen() const      { return bFixedLen; }
    sal_uInt16              GetInfoCount() const    { return nInfoCount; }
    const sal_Int32*   GetColStart() const     { return pColStart; }
    const sal_uInt8*            GetColFormat() const    { return pColFormat; }
    long                GetStartRow() const     { return nStartRow; }
    LanguageType        GetLanguage() const     { return eLang; }

    void    SetCharSet( CharSet eNew )          { eCharSet = eNew; }
    void    SetCharSetSystem( sal_Bool bSet )       { bCharSetSystem = bSet; }
    void    SetFixedLen( sal_Bool bSet )            { bFixedLen = bSet; }
    void    SetFieldSeps( const String& rStr )  { aFieldSeps = rStr; }
    void    SetMergeSeps( sal_Bool bSet )           { bMergeFieldSeps = bSet; }
    void    SetQuotedAsText(bool bSet)          { bQuotedFieldAsText = bSet; }
    void    SetDetectSpecialNumber(bool bSet)   { bDetectSpecialNumber = bSet; }
    void    SetTextSep( sal_Unicode c )         { cTextSep = c; }
    void    SetStartRow( long nRow)             { nStartRow= nRow; }
    void    SetLanguage(LanguageType e)         { eLang = e; }

    void    SetColInfo( sal_uInt16 nCount, const sal_Int32* pStart, const sal_uInt8* pFormat );
    void    SetColumnInfo( const ScCsvExpDataVec& rDataVec );
};

/// How ScImportAsciiDlg is called
enum ScImportAsciiCall {
        SC_IMPORTFILE,           // with File > Open: Text - CSV
        SC_PASTETEXT,            // with Paste > Unformatted Text
        SC_TEXTTOCOLUMNS };      // with Data > Text to Columns

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
