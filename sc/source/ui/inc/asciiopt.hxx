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
