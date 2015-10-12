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

#ifndef INCLUDED_SVTOOLS_PARHTML_HXX
#define INCLUDED_SVTOOLS_PARHTML_HXX

#include <svtools/svtdllapi.h>
#include <svtools/svparser.hxx>

#include <vector>

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

class Color;
class SvNumberFormatter;
class SvKeyValueIterator;

#define HTMLFONTSZ1_DFLT 7
#define HTMLFONTSZ2_DFLT 10
#define HTMLFONTSZ3_DFLT 12
#define HTMLFONTSZ4_DFLT 14
#define HTMLFONTSZ5_DFLT 18
#define HTMLFONTSZ6_DFLT 24
#define HTMLFONTSZ7_DFLT 36

enum HTMLTableFrame { HTML_TF_VOID, HTML_TF_ABOVE, HTML_TF_BELOW,
    HTML_TF_HSIDES, HTML_TF_LHS, HTML_TF_RHS, HTML_TF_VSIDES, HTML_TF_BOX };

enum HTMLTableRules { HTML_TR_NONE, HTML_TR_GROUPS, HTML_TR_ROWS,
    HTML_TR_COLS, HTML_TR_ALL };

enum HTMLInputType
{
    HTML_IT_TEXT =      0x01,
    HTML_IT_PASSWORD =  0x02,
    HTML_IT_CHECKBOX =  0x03,
    HTML_IT_RADIO =     0x04,
    HTML_IT_RANGE =     0x05,
    HTML_IT_SCRIBBLE =  0x06,
    HTML_IT_FILE =      0x07,
    HTML_IT_HIDDEN =    0x08,
    HTML_IT_SUBMIT =    0x09,
    HTML_IT_IMAGE =     0x0a,
    HTML_IT_RESET =     0x0b,
    HTML_IT_BUTTON =    0x0c
};

enum HTMLScriptLanguage
{
    HTML_SL_STARBASIC,
    HTML_SL_JAVASCRIPT,
    HTML_SL_UNKNOWN
};

struct HTMLOptionEnum
{
    const sal_Char *pName;  // value of an HTML option
    sal_uInt16 nValue;      // and corresponding value of an enum
};

/** Representation of an HTML option (=attribute in a start tag).
 * The values of the options are always stored as strings.
 * The methods GetNumber,... may only be called if the option
 * is actually numerical,...
 */
class SVT_DLLPUBLIC HTMLOption
{
    OUString aValue;          // value of the option (always as string)
    OUString aToken;          // name of the option as string
    sal_uInt16 nToken;        // and respective token

public:

    HTMLOption( sal_uInt16 nTyp, const OUString& rToken, const OUString& rValue );

    // name of the option...
    sal_uInt16 GetToken() const { return nToken; }  // ... as enum
    const OUString& GetTokenString() const { return aToken; } // ... as string

    // value of the option ...
    const OUString& GetString() const { return aValue; }  // ... as string

    sal_uInt32 GetNumber() const;                           // ... as number
    sal_Int32 GetSNumber() const;                           // ... as number
    void GetNumbers( std::vector<sal_uInt32> &rNumbers,                  // ... as numbers
                     bool bSpaceDelim=false ) const;
    void GetColor( Color& ) const;                      // ... as color

    // ... as enum; pOptEnums is an HTMLOptionEnum array
    sal_uInt16 GetEnum( const HTMLOptionEnum *pOptEnums,
                        sal_uInt16 nDflt=0 ) const;
    bool GetEnum( sal_uInt16 &rEnum, const HTMLOptionEnum *pOptEnums ) const;

    // ... and as a few special enums
    HTMLInputType GetInputType() const;                 // <INPUT TYPE=...>
    HTMLTableFrame GetTableFrame() const;               // <TABLE FRAME=...>
    HTMLTableRules GetTableRules() const;               // <TABLE RULES=...>
    //SvxAdjust GetAdjust() const;                      // <P,TH,TD ALIGN=>
};

typedef ::std::vector<HTMLOption> HTMLOptions;

class SVT_DLLPUBLIC HTMLParser : public SvParser
{
private:
    mutable HTMLOptions maOptions; // options of the start tag

    bool bNewDoc        : 1;        // read new Doc?
    bool bIsInHeader    : 1;        // scan header section
    bool bIsInBody      : 1;        // scan body section
    bool bReadListing   : 1;        // read listings
    bool bReadXMP       : 1;        // read XMP
    bool bReadPRE       : 1;        // read preformatted text
    bool bReadTextArea  : 1;        // read TEXTAREA
    bool bReadScript    : 1;        // read <SCRIPT>
    bool bReadStyle     : 1;        // read <STYLE>
    bool bEndTokenFound : 1;        // found </SCRIPT> or </STYLE>

    bool bPre_IgnoreNewPara : 1;    // flags for reading of PRE paragraphs
    bool bReadNextChar : 1;         // true: read NextChar again(JavaScript!)
    bool bReadComment : 1;          // true: read NextChar again (JavaScript!)

    sal_uInt32 nPre_LinePos;            // Pos in the line in the PRE-Tag

    int mnPendingOffToken;          ///< OFF token pending for a <XX.../> ON/OFF ON token

    OUString aEndToken;

protected:
    OUString sSaveToken;             // the read tag as string

    int ScanText( const sal_Unicode cBreak = 0U );

    int _GetNextRawToken();

    // scan next token
    virtual int _GetNextToken() override;

    virtual ~HTMLParser();

    void FinishHeader( bool bBody ) { bIsInHeader = false; bIsInBody = bBody; }

public:
    HTMLParser( SvStream& rIn, bool bReadNewDoc = true );

    virtual SvParserState CallParser() override;

    bool IsNewDoc() const       { return bNewDoc; }
    bool IsInHeader() const     { return bIsInHeader; }
    bool IsReadListing() const  { return bReadListing; }
    bool IsReadXMP() const      { return bReadXMP; }
    bool IsReadPRE() const      { return bReadPRE; }
    bool IsReadScript() const   { return bReadScript; }
    bool IsReadStyle() const    { return bReadStyle; }

    // start PRE-/LISTING or XMP mode or filter tags respectively
    inline void StartPRE( bool bRestart=false );
    void FinishPRE() { bReadPRE = false; }
    int FilterPRE( int nToken );

    inline void StartListing( bool bRestart=false );
    void FinishListing() { bReadListing = false; }
    int FilterListing( int nToken );

    inline void StartXMP( bool bRestart=false );
    void FinishXMP() { bReadXMP = false; }
    int FilterXMP( int nToken );

    void FinishTextArea() { bReadTextArea = false; }

    // finish PRE-/LISTING- and XMP mode
    void FinishPREListingXMP() { bReadPRE = bReadListing = bReadXMP = false; }

    // Filter the current token according to the current mode
    // (PRE, XMP, ...) and set the flags. Is called by Continue before
    // NextToken is called. If you implement own loops or call
    // NextToken yourself, you should call this method beforehand.
    int FilterToken( int nToken );

    void ReadRawData( const OUString &rEndToken ) { aEndToken = rEndToken; }

    // Token without \-sequences
    void UnescapeToken();

    // Determine the options. pNoConvertToken is the optional token
    // of an option, for which the CR/LFs are not deleted from the value
    // of the option.
    const HTMLOptions& GetOptions( sal_uInt16 *pNoConvertToken=0 );

    // for asynchronous reading from the SvStream
    virtual void Continue( int nToken ) override;


protected:

    static rtl_TextEncoding GetEncodingByMIME( const OUString& rMime );

    /// template method: called when ParseMetaOptions adds a user-defined meta
    virtual void AddMetaUserDefined( OUString const & i_rMetaName );

private:
    /// parse meta options into XDocumentProperties and encoding
    bool ParseMetaOptionsImpl( const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            SvKeyValueIterator*,
            const HTMLOptions&,
            rtl_TextEncoding& rEnc );

public:
    /// overriding method must call this implementation!
    virtual bool ParseMetaOptions( const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            SvKeyValueIterator* );

    bool ParseScriptOptions( OUString& rLangString, const OUString&, HTMLScriptLanguage& rLang,
                             OUString& rSrc, OUString& rLibrary, OUString& rModule );

    // remove a comment around the content of <SCRIPT> or <STYLE>
    // In case of 'bFull', the whole line behind a "<!--" might
    // be deleted (for JavaSript)
    static void RemoveSGMLComment( OUString &rString, bool bFull );

    static bool InternalImgToPrivateURL( OUString& rURL );
    static rtl_TextEncoding GetEncodingByHttpHeader( SvKeyValueIterator *pHTTPHeader );
    bool SetEncodingByHTTPHeader( SvKeyValueIterator *pHTTPHeader );
};

inline void HTMLParser::StartPRE( bool bRestart )
{
    bReadPRE = true;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

inline void HTMLParser::StartListing( bool bRestart )
{
    bReadListing = true;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

inline void HTMLParser::StartXMP( bool bRestart )
{
    bReadXMP = true;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
