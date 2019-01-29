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
#include <svtools/htmltokn.h>

#include <vector>

namespace com :: sun :: star :: uno { template <class interface_type> class Reference; }

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

class Color;
enum class HtmlOptionId;

#define HTMLFONTSZ1_DFLT 7
#define HTMLFONTSZ2_DFLT 10
#define HTMLFONTSZ3_DFLT 12
#define HTMLFONTSZ4_DFLT 14
#define HTMLFONTSZ5_DFLT 18
#define HTMLFONTSZ6_DFLT 24
#define HTMLFONTSZ7_DFLT 36

enum class HTMLTableFrame { Void, Above, Below, HSides, LHS, RHS, VSides, Box };

enum class HTMLTableRules { NONE, Groups, Rows, Cols, All };

enum class HTMLInputType
{
    Text =      1,
    Password,
    Checkbox,
    Radio,
    Range,
    Scribble,
    File,
    Hidden,
    Submit,
    Image,
    Reset,
    Button
};

enum class HTMLScriptLanguage
{
    StarBasic,
    JavaScript,
    Unknown
};

template<typename EnumT>
struct HTMLOptionEnum
{
    const sal_Char *pName;  // value of an HTML option
    EnumT const     nValue; // and corresponding value of an enum
};

/** Representation of an HTML option (=attribute in a start tag).
 * The values of the options are always stored as strings.
 * The methods GetNumber,... may only be called if the option
 * is actually numerical,...
 */
class SVT_DLLPUBLIC HTMLOption
{
    OUString const aValue;          // value of the option (always as string)
    OUString const aToken;          // name of the option as string
    HtmlOptionId const nToken;        // and respective token

public:

    HTMLOption( HtmlOptionId nTyp, const OUString& rToken, const OUString& rValue );

    // name of the option...
    HtmlOptionId GetToken() const { return nToken; }  // ... as enum
    const OUString& GetTokenString() const { return aToken; } // ... as string

    // value of the option ...
    const OUString& GetString() const { return aValue; }  // ... as string

    sal_uInt32 GetNumber() const;                           // ... as number
    sal_Int32 GetSNumber() const;                           // ... as number
    void GetNumbers( std::vector<sal_uInt32> &rNumbers ) const; // ... as numbers
    void GetColor( Color& ) const;                      // ... as color

    template<typename EnumT>
    EnumT GetEnum( const HTMLOptionEnum<EnumT> *pOptEnums,
                        EnumT nDflt = static_cast<EnumT>(0) ) const
    {
        while( pOptEnums->pName )
        {
            if( aValue.equalsIgnoreAsciiCaseAscii( pOptEnums->pName ) )
                return pOptEnums->nValue;
            pOptEnums++;
        }
        return nDflt;
    }

    template<typename EnumT>
    bool GetEnum( EnumT &rEnum, const HTMLOptionEnum<EnumT> *pOptEnums ) const
    {
        while( pOptEnums->pName )
        {
            if( aValue.equalsIgnoreAsciiCaseAscii( pOptEnums->pName ) )
            {
                rEnum = pOptEnums->nValue;
                return true;
            }
            pOptEnums++;
        }
        return false;
    }

    // ... and as a few special enums
    HTMLInputType GetInputType() const;                 // <INPUT TYPE=...>
    HTMLTableFrame GetTableFrame() const;               // <TABLE FRAME=...>
    HTMLTableRules GetTableRules() const;               // <TABLE RULES=...>
    //SvxAdjust GetAdjust() const;                      // <P,TH,TD ALIGN=>
};

typedef ::std::vector<HTMLOption> HTMLOptions;

class SVT_DLLPUBLIC HTMLParser : public SvParser<HtmlTokenId>
{
private:
    mutable HTMLOptions maOptions; // options of the start tag

    bool const bNewDoc  : 1;        // read new Doc?
    bool bIsInHeader    : 1;        // scan header section
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

    HtmlTokenId mnPendingOffToken;          ///< OFF token pending for a <XX.../> ON/OFF ON token

    OUString aEndToken;

    /// XML namespace, in case of XHTML.
    OUString maNamespace;

protected:
    OUString sSaveToken;             // the read tag as string

    HtmlTokenId ScanText( const sal_Unicode cBreak = 0U );

    HtmlTokenId GetNextRawToken();

    // scan next token
    virtual HtmlTokenId GetNextToken_() override;

    virtual ~HTMLParser() override;

    void FinishHeader() { bIsInHeader = false; }

    void SetNamespace(const OUString& rNamespace);

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
    inline void StartPRE();
    void FinishPRE() { bReadPRE = false; }
    HtmlTokenId FilterPRE( HtmlTokenId nToken );

    inline void StartListing();
    void FinishListing() { bReadListing = false; }
    HtmlTokenId FilterListing( HtmlTokenId nToken );

    inline void StartXMP();
    void FinishXMP() { bReadXMP = false; }
    HtmlTokenId FilterXMP( HtmlTokenId nToken );

    void FinishTextArea() { bReadTextArea = false; }

    // finish PRE-/LISTING- and XMP mode
    void FinishPREListingXMP() { bReadPRE = bReadListing = bReadXMP = false; }

    // Filter the current token according to the current mode
    // (PRE, XMP, ...) and set the flags. Is called by Continue before
    // NextToken is called. If you implement own loops or call
    // NextToken yourself, you should call this method beforehand.
    HtmlTokenId FilterToken( HtmlTokenId nToken );

    void ReadRawData( const OUString &rEndToken ) { aEndToken = rEndToken; }

    // Token without \-sequences
    void UnescapeToken();

    // Determine the options. pNoConvertToken is the optional token
    // of an option, for which the CR/LFs are not deleted from the value
    // of the option.
    const HTMLOptions& GetOptions( HtmlOptionId const *pNoConvertToken=nullptr );

    // for asynchronous reading from the SvStream
    virtual void Continue( HtmlTokenId nToken ) override;


protected:

    static rtl_TextEncoding GetEncodingByMIME( const OUString& rMime );

    /// template method: called when ParseMetaOptions adds a user-defined meta
    virtual void AddMetaUserDefined( OUString const & i_rMetaName );

private:
    /// parse meta options into XDocumentProperties and encoding
    bool ParseMetaOptionsImpl( const css::uno::Reference< css::document::XDocumentProperties>&,
            SvKeyValueIterator*,
            const HTMLOptions&,
            rtl_TextEncoding& rEnc );

public:
    /// overriding method must call this implementation!
    virtual bool ParseMetaOptions( const css::uno::Reference< css::document::XDocumentProperties>&,
            SvKeyValueIterator* );

    void ParseScriptOptions( OUString& rLangString, const OUString&, HTMLScriptLanguage& rLang,
                             OUString& rSrc, OUString& rLibrary, OUString& rModule );

    // Remove a comment around the content of <SCRIPT> or <STYLE>.
    // The whole line behind a "<!--" might be deleted (for JavaScript).
    static void RemoveSGMLComment( OUString &rString );

    static bool InternalImgToPrivateURL( OUString& rURL );
    static rtl_TextEncoding GetEncodingByHttpHeader( SvKeyValueIterator *pHTTPHeader );
    bool SetEncodingByHTTPHeader( SvKeyValueIterator *pHTTPHeader );
};

inline void HTMLParser::StartPRE()
{
    bReadPRE = true;
    bPre_IgnoreNewPara = true;
    nPre_LinePos = 0;
}

inline void HTMLParser::StartListing()
{
    bReadListing = true;
    bPre_IgnoreNewPara = true;
    nPre_LinePos = 0;
}

inline void HTMLParser::StartXMP()
{
    bReadXMP = true;
    bPre_IgnoreNewPara = true;
    nPre_LinePos = 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
