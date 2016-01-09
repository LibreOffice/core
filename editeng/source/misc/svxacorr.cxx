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

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <tools/urlobj.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sot/storinfo.hxx>
#include <svl/fstathelper.hxx>
#include <svtools/helpopt.hxx>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/OrdinalSuffix.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <sot/storage.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/unolingu.hxx>
#include "vcl/window.hxx"
#include <helpid.hrc>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/FastTokenHandler.hpp>
#include <unotools/streamwrap.hxx>
#include <SvXMLAutoCorrectImport.hxx>
#include <SvXMLAutoCorrectExport.hxx>
#include <SvXMLAutoCorrectTokenHandler.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <xmloff/xmltoken.hxx>
#include <vcl/help.hxx>
#include <set>
#include <unordered_map>

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::utl;

static const int C_NONE             = 0x00;
static const int C_FULL_STOP        = 0x01;
static const int C_EXCLAMATION_MARK = 0x02;
static const int C_QUESTION_MARK    = 0x04;
static const sal_Unicode cNonBreakingSpace = 0xA0;

static const sal_Char pXMLImplWrdStt_ExcptLstStr[] = "WordExceptList.xml";
static const sal_Char pXMLImplCplStt_ExcptLstStr[] = "SentenceExceptList.xml";
static const sal_Char pXMLImplAutocorr_ListStr[]   = "DocumentList.xml";

static const sal_Char
    /* also at these beginnings - Brackets and all kinds of begin characters */
    sImplSttSkipChars[] = "\"\'([{\x83\x84\x89\x91\x92\x93\x94",
    /* also at these ends - Brackets and all kinds of begin characters */
    sImplEndSkipChars[] = "\"\')]}\x83\x84\x89\x91\x92\x93\x94";

// These characters are allowed in words: (for FnCapitalStartSentence)
static const sal_Char sImplWordChars[] = "-'";

OUString EncryptBlockName_Imp(const OUString& rName);

static inline bool IsWordDelim( const sal_Unicode c )
{
    return ' ' == c || '\t' == c || 0x0a == c ||
            cNonBreakingSpace == c || 0x2011 == c || 0x1 == c;
}

static inline bool IsLowerLetter( sal_Int32 nCharType )
{
    return CharClass::isLetterType( nCharType ) &&
            0 == ( css::i18n::KCharacterType::UPPER & nCharType);
}

static inline bool IsUpperLetter( sal_Int32 nCharType )
{
    return CharClass::isLetterType( nCharType ) &&
            0 == ( css::i18n::KCharacterType::LOWER & nCharType);
}

bool lcl_IsUnsupportedUnicodeChar( CharClass& rCC, const OUString& rTxt,
                                   sal_Int32 nStt, sal_Int32 nEnd )
{
    for( ; nStt < nEnd; ++nStt )
    {
        short nScript = rCC.getScript( rTxt, nStt );
        switch( nScript )
        {
            case css::i18n::UnicodeScript_kCJKRadicalsSupplement:
            case css::i18n::UnicodeScript_kHangulJamo:
            case css::i18n::UnicodeScript_kCJKSymbolPunctuation:
            case css::i18n::UnicodeScript_kHiragana:
            case css::i18n::UnicodeScript_kKatakana:
            case css::i18n::UnicodeScript_kHangulCompatibilityJamo:
            case css::i18n::UnicodeScript_kEnclosedCJKLetterMonth:
            case css::i18n::UnicodeScript_kCJKCompatibility:
            case css::i18n::UnicodeScript_k_CJKUnifiedIdeographsExtensionA:
            case css::i18n::UnicodeScript_kCJKUnifiedIdeograph:
            case css::i18n::UnicodeScript_kHangulSyllable:
            case css::i18n::UnicodeScript_kCJKCompatibilityIdeograph:
            case css::i18n::UnicodeScript_kHalfwidthFullwidthForm:
                return true;
            default: ; //do nothing
        }
    }
    return false;
}

static bool lcl_IsSymbolChar( CharClass& rCC, const OUString& rTxt,
                                  sal_Int32 nStt, sal_Int32 nEnd )
{
    for( ; nStt < nEnd; ++nStt )
    {
        if( css::i18n::UnicodeType::PRIVATE_USE == rCC.getType( rTxt, nStt ))
            return true;
    }
    return false;
}

static bool lcl_IsInAsciiArr( const sal_Char* pArr, const sal_Unicode c )
{
    bool bRet = false;
    for( ; *pArr; ++pArr )
        if( *pArr == c )
        {
            bRet = true;
            break;
        }
    return bRet;
}

SvxAutoCorrDoc::~SvxAutoCorrDoc()
{
}

// Called by the functions:
//  - FnCapitalStartWord
//  - FnCapitalStartSentence
// after the exchange of characters. Then the words, if necessary, can be inserted
// into the exception list.
void SvxAutoCorrDoc::SaveCpltSttWord( sal_uLong, sal_Int32, const OUString&,
                                        sal_Unicode )
{
}

LanguageType SvxAutoCorrDoc::GetLanguage( sal_Int32, bool ) const
{
    return LANGUAGE_SYSTEM;
}

static const LanguageTag& GetAppLang()
{
    return Application::GetSettings().GetLanguageTag();
}
static LocaleDataWrapper& GetLocaleDataWrapper( sal_uInt16 nLang )
{
    static LocaleDataWrapper aLclDtWrp( GetAppLang() );
    LanguageTag aLcl( nLang );
    const LanguageTag& rLcl = aLclDtWrp.getLoadedLanguageTag();
    if( aLcl != rLcl )
        aLclDtWrp.setLanguageTag( aLcl );
    return aLclDtWrp;
}
static TransliterationWrapper& GetIgnoreTranslWrapper()
{
    static int bIsInit = 0;
    static TransliterationWrapper aWrp( ::comphelper::getProcessComponentContext(),
                css::i18n::TransliterationModules_IGNORE_KANA |
                css::i18n::TransliterationModules_IGNORE_WIDTH );
    if( !bIsInit )
    {
        aWrp.loadModuleIfNeeded( GetAppLang().getLanguageType() );
        bIsInit = 1;
    }
    return aWrp;
}
static CollatorWrapper& GetCollatorWrapper()
{
    static int bIsInit = 0;
    static CollatorWrapper aCollWrp( ::comphelper::getProcessComponentContext() );
    if( !bIsInit )
    {
        aCollWrp.loadDefaultCollator( GetAppLang().getLocale(), 0 );
        bIsInit = 1;
    }
    return aCollWrp;
}

bool SvxAutoCorrect::IsAutoCorrectChar( sal_Unicode cChar )
{
    return  cChar == '\0' || cChar == '\t' || cChar == 0x0a ||
            cChar == ' '  || cChar == '\'' || cChar == '\"' ||
            cChar == '*'  || cChar == '_'  || cChar == '%' ||
            cChar == '.'  || cChar == ','  || cChar == ';' ||
            cChar == ':'  || cChar == '?' || cChar == '!' ||
            cChar == '/'  || cChar == '-';
}

bool SvxAutoCorrect::NeedsHardspaceAutocorr( sal_Unicode cChar )
{
    return cChar == '%' || cChar == ';' || cChar == ':'  || cChar == '?' || cChar == '!' ||
        cChar == '/' /*case for the urls exception*/;
}

long SvxAutoCorrect::GetDefaultFlags()
{
    long nRet = Autocorrect
                    | CapitalStartSentence
                    | CapitalStartWord
                    | ChgOrdinalNumber
                    | ChgToEnEmDash
                    | AddNonBrkSpace
                    | ChgWeightUnderl
                    | SetINetAttr
                    | ChgQuotes
                    | SaveWordCplSttLst
                    | SaveWordWrdSttLst
                    | CorrectCapsLock;
    LanguageType eLang = GetAppLang().getLanguageType();
    switch( eLang )
    {
    case LANGUAGE_ENGLISH:
    case LANGUAGE_ENGLISH_US:
    case LANGUAGE_ENGLISH_UK:
    case LANGUAGE_ENGLISH_AUS:
    case LANGUAGE_ENGLISH_CAN:
    case LANGUAGE_ENGLISH_NZ:
    case LANGUAGE_ENGLISH_EIRE:
    case LANGUAGE_ENGLISH_SAFRICA:
    case LANGUAGE_ENGLISH_JAMAICA:
    case LANGUAGE_ENGLISH_CARRIBEAN:
        nRet &= ~(ChgQuotes|ChgSglQuotes);
        break;
    }
    return nRet;
}


SvxAutoCorrect::SvxAutoCorrect( const OUString& rShareAutocorrFile,
                                const OUString& rUserAutocorrFile )
    : sShareAutoCorrFile( rShareAutocorrFile )
    , sUserAutoCorrFile( rUserAutocorrFile )
    , m_pLangTable( new std::map<LanguageTag, std::unique_ptr<SvxAutoCorrectLanguageLists>> )
    , pCharClass( nullptr )
    , bRunNext( false )
    , eCharClassLang( LANGUAGE_DONTKNOW )
    , nFlags(SvxAutoCorrect::GetDefaultFlags())
    , cStartDQuote( 0 )
    , cEndDQuote( 0 )
    , cStartSQuote( 0 )
    , cEndSQuote( 0 )
    , cEmDash( 0x2014 )
    , cEnDash( 0x2013)
{
}

SvxAutoCorrect::SvxAutoCorrect( const SvxAutoCorrect& rCpy )
    : sShareAutoCorrFile( rCpy.sShareAutoCorrFile )
    , sUserAutoCorrFile( rCpy.sUserAutoCorrFile )
    , aSwFlags( rCpy.aSwFlags )
    , m_pLangTable( new std::map<LanguageTag, std::unique_ptr<SvxAutoCorrectLanguageLists>> )
    , pCharClass( nullptr )
    , bRunNext( false )
    , eCharClassLang(rCpy.eCharClassLang)
    , nFlags( rCpy.nFlags & ~(ChgWordLstLoad|CplSttLstLoad|WrdSttLstLoad))
    , cStartDQuote( rCpy.cStartDQuote )
    , cEndDQuote( rCpy.cEndDQuote )
    , cStartSQuote( rCpy.cStartSQuote )
    , cEndSQuote( rCpy.cEndSQuote )
    , cEmDash( rCpy.cEmDash )
    , cEnDash( rCpy.cEnDash )
{
}


SvxAutoCorrect::~SvxAutoCorrect()
{
    delete m_pLangTable;
    delete pCharClass;
}

void SvxAutoCorrect::_GetCharClass( LanguageType eLang )
{
    delete pCharClass;
    pCharClass = new CharClass( LanguageTag( eLang));
    eCharClassLang = eLang;
}

void SvxAutoCorrect::SetAutoCorrFlag( long nFlag, bool bOn )
{
    long nOld = nFlags;
    nFlags = bOn ? nFlags | nFlag
                 : nFlags & ~nFlag;

    if( !bOn )
    {
        if( (nOld & CapitalStartSentence) != (nFlags & CapitalStartSentence) )
            nFlags &= ~CplSttLstLoad;
        if( (nOld & CapitalStartWord) != (nFlags & CapitalStartWord) )
            nFlags &= ~WrdSttLstLoad;
        if( (nOld & Autocorrect) != (nFlags & Autocorrect) )
            nFlags &= ~ChgWordLstLoad;
    }
}


    // Two capital letters at the beginning of word?
bool SvxAutoCorrect::FnCapitalStartWord( SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                    sal_Int32 nSttPos, sal_Int32 nEndPos,
                                    LanguageType eLang )
{
    bool bRet = false;
    CharClass& rCC = GetCharClass( eLang );

    // Delete all non alphanumeric. Test the characters at the beginning/end of
    // the word ( recognizes: "(min.", "/min.", and so on.)
    for( ; nSttPos < nEndPos; ++nSttPos )
        if( rCC.isLetterNumeric( rTxt, nSttPos ))
            break;
    for( ; nSttPos < nEndPos; --nEndPos )
        if( rCC.isLetterNumeric( rTxt, nEndPos - 1 ))
            break;

    // Is the word a compounded word separated by delimiters?
    // If so, keep track of all delimiters so each constituent
    // word can be checked for two initial capital letters.
    std::deque<sal_Int32> aDelimiters;

    // Always check for two capitals at the beginning
    // of the entire word, so start at nSttPos.
    aDelimiters.push_back(nSttPos);

    // Find all compound word delimiters
    for (sal_Int32 n = nSttPos; n < nEndPos; ++n)
    {
        if (IsAutoCorrectChar(rTxt[ n ]))
        {
            aDelimiters.push_back( n + 1 ); // Get position of char after delimiter
        }
    }

    // Decide where to put the terminating delimiter.
    // If the last AutoCorrect char was a newline, then the AutoCorrect
    // char will not be included in rTxt.
    // If the last AutoCorrect char was not a newline, then the AutoCorrect
    // character will be the last character in rTxt.
    if (!IsAutoCorrectChar(rTxt[nEndPos-1]))
        aDelimiters.push_back(nEndPos);

    // Iterate through the word and all words that compose it.
    // Two capital letters at the beginning of word?
    for (size_t nI = 0; nI < aDelimiters.size() - 1; ++nI)
    {
        nSttPos = aDelimiters[nI];
        nEndPos = aDelimiters[nI + 1];

        if( nSttPos+2 < nEndPos &&
            IsUpperLetter( rCC.getCharacterType( rTxt, nSttPos )) &&
            IsUpperLetter( rCC.getCharacterType( rTxt, ++nSttPos )) &&
            // Is the third character a lower case
            IsLowerLetter( rCC.getCharacterType( rTxt, nSttPos +1 )) &&
            // Do not replace special attributes
            0x1 != rTxt[ nSttPos ] && 0x2 != rTxt[ nSttPos ])
        {
            // test if the word is in an exception list
            OUString sWord( rTxt.copy( nSttPos - 1, nEndPos - nSttPos + 1 ));
            if( !FindInWrdSttExceptList(eLang, sWord) )
            {
                // Check that word isn't correctly spelled before correcting:
                css::uno::Reference< css::linguistic2::XSpellChecker1 > xSpeller =
                    SvxGetSpellChecker();
                if( xSpeller->hasLanguage(eLang) )
                {
                    Sequence< css::beans::PropertyValue > aEmptySeq;
                    if (!xSpeller->spell(sWord, eLang, aEmptySeq).is())
                    {
                        return false;
                    }
                }
                sal_Unicode cSave = rTxt[ nSttPos ];
                OUString sChar( cSave );
                sChar = rCC.lowercase( sChar );
                if( sChar[0] != cSave && rDoc.ReplaceRange( nSttPos, 1, sChar ))
                {
                    if( SaveWordWrdSttLst & nFlags )
                        rDoc.SaveCpltSttWord( CapitalStartWord, nSttPos, sWord, cSave );
                    bRet = true;
                }
            }
        }
    }
    return bRet;
}


bool SvxAutoCorrect::FnChgOrdinalNumber(
    SvxAutoCorrDoc& rDoc, const OUString& rTxt,
    sal_Int32 nSttPos, sal_Int32 nEndPos,
    LanguageType eLang)
{
    // 1st, 2nd, 3rd, 4 - 0th
    // 201th or 201st
    // 12th or 12nd
    bool bChg = false;

    // In some languages ordinal suffixes should never be
    // changed to superscript. Let's break for those languages.
    switch (eLang)
    {
    case LANGUAGE_SWEDISH:
    case LANGUAGE_SWEDISH_FINLAND:
        break;
    default:
        CharClass& rCC = GetCharClass(eLang);

        for (; nSttPos < nEndPos; ++nSttPos)
            if (!lcl_IsInAsciiArr(sImplSttSkipChars, rTxt[nSttPos]))
                break;
        for (; nSttPos < nEndPos; --nEndPos)
            if (!lcl_IsInAsciiArr(sImplEndSkipChars, rTxt[nEndPos - 1]))
                break;


        // Get the last number in the string to check
        sal_Int32 nNumEnd = nEndPos;
        bool bFoundEnd = false;
        bool isValidNumber = true;
        sal_Int32 i = nEndPos;

        while (i > nSttPos)
        {
            i--;
            bool isDigit = rCC.isDigit(rTxt, i);
            if (bFoundEnd)
                isValidNumber |= isDigit;

            if (isDigit && !bFoundEnd)
            {
                bFoundEnd = true;
                nNumEnd = i;
            }
        }

        if (bFoundEnd && isValidNumber) {
            sal_Int32 nNum = rTxt.copy(nSttPos, nNumEnd - nSttPos + 1).toInt32();

            // Check if the characters after that number correspond to the ordinal suffix
            uno::Reference< i18n::XOrdinalSuffix > xOrdSuffix
                = i18n::OrdinalSuffix::create(comphelper::getProcessComponentContext());

            uno::Sequence< OUString > aSuffixes = xOrdSuffix->getOrdinalSuffix(nNum, rCC.getLanguageTag().getLocale());
            for (sal_Int32 nSuff = 0; nSuff < aSuffixes.getLength(); nSuff++)
            {
                OUString sSuffix(aSuffixes[nSuff]);
                OUString sEnd = rTxt.copy(nNumEnd + 1, nEndPos - nNumEnd - 1);

                if (sSuffix == sEnd)
                {
                    // Check if the ordinal suffix has to be set as super script
                    if (rCC.isLetter(sSuffix))
                    {
                        // Do the change
                        SvxEscapementItem aSvxEscapementItem(DFLT_ESC_AUTO_SUPER,
                            DFLT_ESC_PROP, SID_ATTR_CHAR_ESCAPEMENT);
                        rDoc.SetAttr(nNumEnd + 1, nEndPos,
                            SID_ATTR_CHAR_ESCAPEMENT,
                            aSvxEscapementItem);
                        bChg = true;
                    }
                }
            }
        }
    }
    return bChg;
}


bool SvxAutoCorrect::FnChgToEnEmDash(
                                SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                sal_Int32 nSttPos, sal_Int32 nEndPos,
                                LanguageType eLang )
{
    bool bRet = false;
    CharClass& rCC = GetCharClass( eLang );
    if (eLang == LANGUAGE_SYSTEM)
        eLang = GetAppLang().getLanguageType();
    bool bAlwaysUseEmDash = (cEmDash && (eLang == LANGUAGE_RUSSIAN || eLang == LANGUAGE_UKRAINIAN));

    // replace " - " or " --" with "enDash"
    if( cEnDash && 1 < nSttPos && 1 <= nEndPos - nSttPos )
    {
        sal_Unicode cCh = rTxt[ nSttPos ];
        if( '-' == cCh )
        {
            if( ' ' == rTxt[ nSttPos-1 ] &&
                '-' == rTxt[ nSttPos+1 ])
            {
                sal_Int32 n;
                for( n = nSttPos+2; n < nEndPos && lcl_IsInAsciiArr(
                            sImplSttSkipChars,(cCh = rTxt[ n ]));
                        ++n )
                    ;

                // found: " --[<AnySttChars>][A-z0-9]
                if( rCC.isLetterNumeric( OUString(cCh) ) )
                {
                    for( n = nSttPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt[ --n ])); )
                        ;

                    // found: "[A-z0-9][<AnyEndChars>] --[<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( OUString(cCh) ))
                    {
                        rDoc.Delete( nSttPos, nSttPos + 2 );
                        rDoc.Insert( nSttPos, bAlwaysUseEmDash ? OUString(cEmDash) : OUString(cEnDash) );
                        bRet = true;
                    }
                }
            }
        }
        else if( 3 < nSttPos &&
                 ' ' == rTxt[ nSttPos-1 ] &&
                 '-' == rTxt[ nSttPos-2 ])
        {
            sal_Int32 n, nLen = 1, nTmpPos = nSttPos - 2;
            if( '-' == ( cCh = rTxt[ nTmpPos-1 ]) )
            {
                --nTmpPos;
                ++nLen;
                cCh = rTxt[ nTmpPos-1 ];
            }
            if( ' ' == cCh )
            {
                for( n = nSttPos; n < nEndPos && lcl_IsInAsciiArr(
                            sImplSttSkipChars,(cCh = rTxt[ n ]));
                        ++n )
                    ;

                // found: " - [<AnySttChars>][A-z0-9]
                if( rCC.isLetterNumeric( OUString(cCh) ) )
                {
                    cCh = ' ';
                    for( n = nTmpPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt[ --n ])); )
                            ;
                    // found: "[A-z0-9][<AnyEndChars>] - [<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( OUString(cCh) ))
                    {
                        rDoc.Delete( nTmpPos, nTmpPos + nLen );
                        rDoc.Insert( nTmpPos, bAlwaysUseEmDash ? OUString(cEmDash) : OUString(cEnDash) );
                        bRet = true;
                    }
                }
            }
        }
    }

    // Replace [A-z0-9]--[A-z0-9] double dash with "emDash" or "enDash"
    // [0-9]--[0-9] double dash always replaced with "enDash"
    // Finnish and Hungarian use enDash instead of emDash.
    bool bEnDash = (eLang == LANGUAGE_HUNGARIAN || eLang == LANGUAGE_FINNISH);
    if( ((cEmDash && !bEnDash) || (cEnDash && bEnDash)) && 4 <= nEndPos - nSttPos )
    {
        OUString sTmp( rTxt.copy( nSttPos, nEndPos - nSttPos ) );
        sal_Int32 nFndPos = sTmp.indexOf("--");
        if( nFndPos != -1 && nFndPos &&
            nFndPos + 2 < sTmp.getLength() &&
            ( rCC.isLetterNumeric( sTmp, nFndPos - 1 ) ||
              lcl_IsInAsciiArr( sImplEndSkipChars, rTxt[ nFndPos - 1 ] )) &&
            ( rCC.isLetterNumeric( sTmp, nFndPos + 2 ) ||
            lcl_IsInAsciiArr( sImplSttSkipChars, rTxt[ nFndPos + 2 ] )))
        {
            nSttPos = nSttPos + nFndPos;
            rDoc.Delete( nSttPos, nSttPos + 2 );
            rDoc.Insert( nSttPos, (bEnDash || (rCC.isDigit( sTmp, nFndPos - 1 ) &&
                rCC.isDigit( sTmp, nFndPos + 2 )) ? OUString(cEnDash) : OUString(cEmDash)) );
            bRet = true;
        }
    }
    return bRet;
}


bool SvxAutoCorrect::FnAddNonBrkSpace(
                                SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                sal_Int32, sal_Int32 nEndPos,
                                LanguageType eLang )
{
    bool bRet = false;

    CharClass& rCC = GetCharClass( eLang );

    if ( rCC.getLanguageTag().getLanguage() == "fr" )
    {
        bool bFrCA = (rCC.getLanguageTag().getCountry() == "CA");
        OUString allChars = ":;?!%";
        OUString chars( allChars );
        if ( bFrCA )
            chars = ":";

        sal_Unicode cChar = rTxt[ nEndPos ];
        bool bHasSpace = chars.indexOf( cChar ) != -1;
        bool bIsSpecial = allChars.indexOf( cChar ) != -1;
        if ( bIsSpecial )
        {
            // Get the last word delimiter position
            sal_Int32 nSttWdPos = nEndPos;
            bool bWasWordDelim = false;
            while( nSttWdPos && !(bWasWordDelim = IsWordDelim( rTxt[ --nSttWdPos ])))
                ;

            //See if the text is the start of a protocol string, e.g. have text of
            //"http" see if it is the start of "http:" and if so leave it alone
            sal_Int32 nIndex = nSttWdPos + (bWasWordDelim ? 1 : 0);
            sal_Int32 nProtocolLen = nEndPos - nSttWdPos + 1;
            if (nIndex + nProtocolLen <= rTxt.getLength())
            {
                if (INetURLObject::CompareProtocolScheme(rTxt.copy(nIndex, nProtocolLen)) != INetProtocol::NotValid)
                    return false;
            }

            // Check the presence of "://" in the word
            sal_Int32 nStrPos = rTxt.indexOf( "://", nSttWdPos + 1 );
            if ( nStrPos == -1 && nEndPos > 0 )
            {
                // Check the previous char
                sal_Unicode cPrevChar = rTxt[ nEndPos - 1 ];
                if ( ( chars.indexOf( cPrevChar ) == -1 ) && cPrevChar != '\t' )
                {
                    // Remove any previous normal space
                    sal_Int32 nPos = nEndPos - 1;
                    while ( cPrevChar == ' ' || cPrevChar == cNonBreakingSpace )
                    {
                        if ( nPos == 0 ) break;
                        nPos--;
                        cPrevChar = rTxt[ nPos ];
                    }

                    nPos++;
                    if ( nEndPos - nPos > 0 )
                        rDoc.Delete( nPos, nEndPos );

                    // Add the non-breaking space at the end pos
                    if ( bHasSpace )
                        rDoc.Insert( nPos, OUString(cNonBreakingSpace) );
                    bRunNext = true;
                    bRet = true;
                }
                else if ( chars.indexOf( cPrevChar ) != -1 )
                    bRunNext = true;
            }
        }
        else if ( cChar == '/' && nEndPos > 1 && rTxt.getLength() > (nEndPos - 1) )
        {
            // Remove the hardspace right before to avoid formatting URLs
            sal_Unicode cPrevChar = rTxt[ nEndPos - 1 ];
            sal_Unicode cMaybeSpaceChar = rTxt[ nEndPos - 2 ];
            if ( cPrevChar == ':' && cMaybeSpaceChar == cNonBreakingSpace )
            {
                rDoc.Delete( nEndPos - 2, nEndPos - 1 );
                bRet = true;
            }
        }
    }

    return bRet;
}


bool SvxAutoCorrect::FnSetINetAttr( SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                    sal_Int32 nSttPos, sal_Int32 nEndPos,
                                    LanguageType eLang )
{
    OUString sURL( URIHelper::FindFirstURLInText( rTxt, nSttPos, nEndPos,
                                                GetCharClass( eLang ) ));
    bool bRet = !sURL.isEmpty();
    if( bRet )          // also Attribut setzen:
        rDoc.SetINetAttr( nSttPos, nEndPos, sURL );
    return bRet;
}


bool SvxAutoCorrect::FnChgWeightUnderl( SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                        sal_Int32 , sal_Int32 nEndPos,
                                        LanguageType eLang )
{
    // Condition:
    //  at the beginning:   _ or * after Space with the following !Space
    //  at the end:         _ or * before Space (word delimiter?)

    sal_Unicode cInsChar = rTxt[ nEndPos ];  // underline or bold
    if( ++nEndPos != rTxt.getLength() &&
        !IsWordDelim( rTxt[ nEndPos ] ) )
        return false;

    --nEndPos;

    bool bAlphaNum = false;
    sal_Int32 nPos = nEndPos;
    sal_Int32  nFndPos = -1;
    CharClass& rCC = GetCharClass( eLang );

    while( nPos )
    {
        switch( sal_Unicode c = rTxt[ --nPos ] )
        {
        case '_':
        case '*':
            if( c == cInsChar )
            {
                if( bAlphaNum && nPos+1 < nEndPos && ( !nPos ||
                    IsWordDelim( rTxt[ nPos-1 ])) &&
                    !IsWordDelim( rTxt[ nPos+1 ]))
                        nFndPos = nPos;
                else
                    // Condition is not satisfied, so cancel
                    nFndPos = -1;
                nPos = 0;
            }
            break;
        default:
            if( !bAlphaNum )
                bAlphaNum = rCC.isLetterNumeric( rTxt, nPos );
        }
    }

    if( -1 != nFndPos )
    {
        // first delete the Character at the end - this allows insertion
        // of an empty hint in SetAttr which would be removed by Delete
        // (fdo#62536, AUTOFMT in Writer)
        rDoc.Delete( nEndPos, nEndPos + 1 );
        rDoc.Delete( nFndPos, nFndPos + 1 );
        // Span the Attribute over the area
        // the end.
        if( '*' == cInsChar )           // Bold
        {
            SvxWeightItem aSvxWeightItem( WEIGHT_BOLD, SID_ATTR_CHAR_WEIGHT );
            rDoc.SetAttr( nFndPos, nEndPos - 1,
                            SID_ATTR_CHAR_WEIGHT,
                            aSvxWeightItem);
        }
        else                            // underline
        {
            SvxUnderlineItem aSvxUnderlineItem( UNDERLINE_SINGLE, SID_ATTR_CHAR_UNDERLINE );
            rDoc.SetAttr( nFndPos, nEndPos - 1,
                            SID_ATTR_CHAR_UNDERLINE,
                            aSvxUnderlineItem);
        }
    }

    return -1 != nFndPos;
}


bool SvxAutoCorrect::FnCapitalStartSentence( SvxAutoCorrDoc& rDoc,
                                    const OUString& rTxt, bool bNormalPos,
                                    sal_Int32 nSttPos, sal_Int32 nEndPos,
                                    LanguageType eLang )
{

    if( rTxt.isEmpty() || nEndPos <= nSttPos )
        return false;

    CharClass& rCC = GetCharClass( eLang );
    OUString aText( rTxt );
    const sal_Unicode *pStart = aText.getStr(),
                      *pStr = pStart + nEndPos,
                      *pWordStt = nullptr,
                      *pDelim = nullptr;

    bool bAtStart = false;
    do {
        --pStr;
        if (rCC.isLetter(aText, pStr - pStart))
        {
            if( !pWordStt )
                pDelim = pStr+1;
            pWordStt = pStr;
        }
        else if (pWordStt && !rCC.isDigit(aText, pStr - pStart))
        {
            if( lcl_IsInAsciiArr( sImplWordChars, *pStr ) &&
                pWordStt - 1 == pStr &&
                // Installation at beginning of paragraph. Replaced < by <= (#i38971#)
                (pStart + 1) <= pStr &&
                rCC.isLetter(aText, pStr-1 - pStart))
                pWordStt = --pStr;
            else
                break;
        }
    } while( ! ( bAtStart = (pStart == pStr) ) );

    if (!pWordStt)
        return false;    // no character to be replaced


    if (rCC.isDigit(aText, pStr - pStart))
        return false; // already ok

    if (IsUpperLetter(rCC.getCharacterType(aText, pWordStt - pStart)))
        return false; // already ok

    //See if the text is the start of a protocol string, e.g. have text of
    //"http" see if it is the start of "http:" and if so leave it alone
    sal_Int32 nIndex = pWordStt - pStart;
    sal_Int32 nProtocolLen = pDelim - pWordStt + 1;
    if (nIndex + nProtocolLen <= rTxt.getLength())
    {
        if (INetURLObject::CompareProtocolScheme(rTxt.copy(nIndex, nProtocolLen)) != INetProtocol::NotValid)
            return false; // already ok
    }

    if (0x1 == *pWordStt || 0x2 == *pWordStt)
        return false; // already ok

    if( *pDelim && 2 >= pDelim - pWordStt &&
        lcl_IsInAsciiArr( ".-)>", *pDelim ) )
        return false;

    if( !bAtStart ) // Still no beginning of a paragraph?
    {
        if ( IsWordDelim( *pStr ) )
        {
            while( ! ( bAtStart = (pStart == pStr--) ) && IsWordDelim( *pStr ) )
                ;
        }
        // Asian full stop, full width full stop, full width exclamation mark
        // and full width question marks are treated as word delimiters
        else if ( 0x3002 != *pStr && 0xFF0E != *pStr && 0xFF01 != *pStr &&
                  0xFF1F != *pStr )
            return false; // no valid separator -> no replacement
    }

    if( bAtStart )  // at the beginning of a paragraph?
    {
        // Check out the previous paragraph, if it exists.
        // If so, then check to paragraph separator at the end.
        OUString const*const pPrevPara = rDoc.GetPrevPara(bNormalPos);
        if (!pPrevPara)
        {
            // valid separator -> replace
            OUString sChar( *pWordStt );
            sChar = rCC.titlecase(sChar); //see fdo#56740
            return  !comphelper::string::equals(sChar, *pWordStt) &&
                    rDoc.ReplaceRange( pWordStt - pStart, 1, sChar );
        }

        aText = *pPrevPara;
        bAtStart = false;
        pStart = aText.getStr();
        pStr = pStart + aText.getLength();

        do {            // overwrite all blanks
            --pStr;
            if( !IsWordDelim( *pStr ))
                break;
        } while( ! ( bAtStart = (pStart == pStr) ) );

        if( bAtStart )
            return false;  // no valid separator -> no replacement
    }

    // Found [ \t]+[A-Z0-9]+ until here. Test now on the paragraph separator.
    // all three can happen, but not more than once!
    const sal_Unicode* pExceptStt = nullptr;
    if( !bAtStart )
    {
        bool bContinue = true;
        int nFlag = C_NONE;
        do {
            switch( *pStr )
            {
            // Western and Asian full stop
            case '.':
            case 0x3002 :
            case 0xFF0E :
                {
                    if (pStr >= pStart + 2 && *(pStr-2) == '.')
                    {
                        //e.g. text "f.o.o. word": Now currently considering
                        //capitalizing word but second last character of
                        //previous word is a .  So probably last word is an
                        //anagram that ends in . and not truly the end of a
                        //previous sentence, so don't autocapitalize this word
                        return false;
                    }
                    if( nFlag & C_FULL_STOP )
                        return false;  // no valid separator -> no replacement
                    nFlag |= C_FULL_STOP;
                    pExceptStt = pStr;
                }
                break;
            case '!':
            case 0xFF01 :
                {
                    if( nFlag & C_EXCLAMATION_MARK )
                        return false;   // no valid separator -> no replacement
                    nFlag |= C_EXCLAMATION_MARK;
                }
                break;
            case '?':
            case 0xFF1F :
                {
                    if( nFlag & C_QUESTION_MARK)
                        return false;   // no valid separator -> no replacement
                    nFlag |= C_QUESTION_MARK;
                }
                break;
            default:
                if( !nFlag )
                    return false;       // no valid separator -> no replacement
                else
                    bContinue = false;
                break;
            }

            if( bContinue && pStr-- == pStart )
            {
                return false;       // no valid separator -> no replacement
            }
        } while( bContinue );
        if( C_FULL_STOP != nFlag )
            pExceptStt = nullptr;
    }

    if( 2 > ( pStr - pStart ) )
        return false;

    if (!rCC.isLetterNumeric(aText, pStr-- - pStart))
    {
        bool bValid = false, bAlphaFnd = false;
        const sal_Unicode* pTmpStr = pStr;
        while( !bValid )
        {
            if( rCC.isDigit( aText, pTmpStr - pStart ) )
            {
                bValid = true;
                pStr = pTmpStr - 1;
            }
            else if( rCC.isLetter( aText, pTmpStr - pStart ) )
            {
                if( bAlphaFnd )
                {
                    bValid = true;
                    pStr = pTmpStr;
                }
                else
                    bAlphaFnd = true;
            }
            else if( bAlphaFnd || IsWordDelim( *pTmpStr ) )
                break;

            if( pTmpStr == pStart )
                break;

            --pTmpStr;
        }

        if( !bValid )
            return false;       // no valid separator -> no replacement
    }

    bool bNumericOnly = '0' <= *(pStr+1) && *(pStr+1) <= '9';

    // Search for the beginning of the word
    while( !IsWordDelim( *pStr ))
    {
        if( bNumericOnly && rCC.isLetter( aText, pStr - pStart ) )
            bNumericOnly = false;

        if( pStart == pStr )
            break;

        --pStr;
    }

    if( bNumericOnly )      // consists of only numbers, then not
        return false;

    if( IsWordDelim( *pStr ))
        ++pStr;

    OUString sWord;

    // check on the basis of the exception list
    if( pExceptStt )
    {
        sWord = OUString(pStr, pExceptStt - pStr + 1);
        if( FindInCplSttExceptList(eLang, sWord) )
            return false;

        // Delete all non alphanumeric. Test the characters at the
        // beginning/end of the word ( recognizes: "(min.", "/min.", and so on.)
        OUString sTmp( sWord );
        while( !sTmp.isEmpty() &&
                !rCC.isLetterNumeric( sTmp, 0 ) )
            sTmp = sTmp.copy(1);

        // Remove all non alphanumeric characters towards the end up until
        // the last one.
        sal_Int32 nLen = sTmp.getLength();
        while( nLen && !rCC.isLetterNumeric( sTmp, nLen-1 ) )
            --nLen;
        if( nLen + 1 < sTmp.getLength() )
            sTmp = sTmp.copy( 0, nLen + 1 );

        if( !sTmp.isEmpty() && sTmp.getLength() != sWord.getLength() &&
            FindInCplSttExceptList(eLang, sTmp))
            return false;

        if(FindInCplSttExceptList(eLang, sWord, true))
            return false;
    }

    // Ok, then replace
    sal_Unicode cSave = *pWordStt;
    nSttPos = pWordStt - rTxt.getStr();
    OUString sChar( cSave );
    sChar = rCC.titlecase(sChar); //see fdo#56740
    bool bRet = sChar[0] != cSave && rDoc.ReplaceRange( nSttPos, 1, sChar );

    // Perhaps someone wants to have the word
    if( bRet && SaveWordCplSttLst & nFlags )
        rDoc.SaveCpltSttWord( CapitalStartSentence, nSttPos, sWord, cSave );

    return bRet;
}

bool SvxAutoCorrect::FnCorrectCapsLock( SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                        sal_Int32 nSttPos, sal_Int32 nEndPos,
                                        LanguageType eLang )
{
    if (nEndPos - nSttPos < 2)
        // string must be at least 2-character long.
        return false;

    CharClass& rCC = GetCharClass( eLang );

    // Check the first 2 letters.
    if ( !IsLowerLetter(rCC.getCharacterType(rTxt, nSttPos)) )
        return false;

    if ( !IsUpperLetter(rCC.getCharacterType(rTxt, nSttPos+1)) )
        return false;

    OUString aConverted;
    aConverted += rCC.uppercase(OUString(rTxt[nSttPos]));
    aConverted += rCC.lowercase(OUString(rTxt[nSttPos+1]));

    for( sal_Int32 i = nSttPos+2; i < nEndPos; ++i )
    {
        if ( IsLowerLetter(rCC.getCharacterType(rTxt, i)) )
            // A lowercase letter disqualifies the whole text.
            return false;

        if ( IsUpperLetter(rCC.getCharacterType(rTxt, i)) )
            // Another uppercase letter.  Convert it.
            aConverted += rCC.lowercase(OUString(rTxt[i]));
        else
            // This is not an alphabetic letter.  Leave it as-is.
            aConverted += OUString( rTxt[i] );
    }

    // Replace the word.
    rDoc.Delete(nSttPos, nEndPos);
    rDoc.Insert(nSttPos, aConverted);

    return true;
}


sal_Unicode SvxAutoCorrect::GetQuote( sal_Unicode cInsChar, bool bSttQuote,
                                        LanguageType eLang ) const
{
    sal_Unicode cRet = bSttQuote ? ( '\"' == cInsChar
                                    ? GetStartDoubleQuote()
                                    : GetStartSingleQuote() )
                                   : ( '\"' == cInsChar
                                    ? GetEndDoubleQuote()
                                    : GetEndSingleQuote() );
    if( !cRet )
    {
        // then through the Language find the right character
        if( LANGUAGE_NONE == eLang )
            cRet = cInsChar;
        else
        {
            LocaleDataWrapper& rLcl = GetLocaleDataWrapper( eLang );
            OUString sRet( bSttQuote
                            ? ( '\"' == cInsChar
                                ? rLcl.getDoubleQuotationMarkStart()
                                : rLcl.getQuotationMarkStart() )
                            : ( '\"' == cInsChar
                                ? rLcl.getDoubleQuotationMarkEnd()
                                : rLcl.getQuotationMarkEnd() ));
            cRet = !sRet.isEmpty() ? sRet[0] : cInsChar;
        }
    }
    return cRet;
}

void SvxAutoCorrect::InsertQuote( SvxAutoCorrDoc& rDoc, sal_Int32 nInsPos,
                                    sal_Unicode cInsChar, bool bSttQuote,
                                    bool bIns )
{
    LanguageType eLang = rDoc.GetLanguage( nInsPos );
    sal_Unicode cRet = GetQuote( cInsChar, bSttQuote, eLang );

    OUString sChg( cInsChar );
    if( bIns )
        rDoc.Insert( nInsPos, sChg );
    else
        rDoc.Replace( nInsPos, sChg );

    sChg = OUString(cRet);

    if( '\"' == cInsChar )
    {
        if( LANGUAGE_SYSTEM == eLang )
            eLang = GetAppLang().getLanguageType();
        switch( eLang )
        {
        case LANGUAGE_FRENCH:
        case LANGUAGE_FRENCH_BELGIAN:
        case LANGUAGE_FRENCH_CANADIAN:
        case LANGUAGE_FRENCH_SWISS:
        case LANGUAGE_FRENCH_LUXEMBOURG:
            {
                OUString s( cNonBreakingSpace );
                    // UNICODE code for no break space
                if( rDoc.Insert( bSttQuote ? nInsPos+1 : nInsPos, s ))
                {
                    if( !bSttQuote )
                        ++nInsPos;
                }
            }
            break;
        }
    }

    rDoc.Replace( nInsPos, sChg );
}

OUString SvxAutoCorrect::GetQuote( SvxAutoCorrDoc& rDoc, sal_Int32 nInsPos,
                                sal_Unicode cInsChar, bool bSttQuote )
{
    LanguageType eLang = rDoc.GetLanguage( nInsPos );
    sal_Unicode cRet = GetQuote( cInsChar, bSttQuote, eLang );

    OUString sRet = OUString(cRet);

    if( '\"' == cInsChar )
    {
        if( LANGUAGE_SYSTEM == eLang )
            eLang = GetAppLang().getLanguageType();
        switch( eLang )
        {
        case LANGUAGE_FRENCH:
        case LANGUAGE_FRENCH_BELGIAN:
        case LANGUAGE_FRENCH_CANADIAN:
        case LANGUAGE_FRENCH_SWISS:
        case LANGUAGE_FRENCH_LUXEMBOURG:
            if( bSttQuote )
                sRet += " ";
            else
                sRet = " " + sRet;
            break;
        }
    }
    return sRet;
}

sal_uLong
SvxAutoCorrect::DoAutoCorrect( SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                                    sal_Int32 nInsPos, sal_Unicode cChar,
                                    bool bInsert, vcl::Window* pFrameWin )
{
    sal_uLong nRet = 0;
    bool bIsNextRun = bRunNext;
    bRunNext = false;  // if it was set, then it has to be turned off

    do{                                 // only for middle check loop !!
        if( cChar )
        {
            // Prevent double space
            if( nInsPos && ' ' == cChar &&
                IsAutoCorrFlag( IgnoreDoubleSpace ) &&
                ' ' == rTxt[ nInsPos - 1 ])
            {
                nRet = IgnoreDoubleSpace;
                break;
            }

            bool bSingle = '\'' == cChar;
            bool bIsReplaceQuote =
                        (IsAutoCorrFlag( ChgQuotes ) && ('\"' == cChar )) ||
                        (IsAutoCorrFlag( ChgSglQuotes ) && bSingle );
            if( bIsReplaceQuote )
            {
                sal_Unicode cPrev;
                bool bSttQuote = !nInsPos ||
                        IsWordDelim( ( cPrev = rTxt[ nInsPos-1 ])) ||
                        lcl_IsInAsciiArr( "([{", cPrev ) ||
                        ( cEmDash && cEmDash == cPrev ) ||
                        ( cEnDash && cEnDash == cPrev );

                InsertQuote( rDoc, nInsPos, cChar, bSttQuote, bInsert );
                nRet = bSingle ? ChgSglQuotes : ChgQuotes;
                break;
            }

            if( bInsert )
                rDoc.Insert( nInsPos, OUString(cChar) );
            else
                rDoc.Replace( nInsPos, OUString(cChar) );

            // Hardspaces autocorrection
            if ( IsAutoCorrFlag( AddNonBrkSpace ) )
            {
                if ( NeedsHardspaceAutocorr( cChar ) &&
                    FnAddNonBrkSpace( rDoc, rTxt, 0, nInsPos, rDoc.GetLanguage( nInsPos ) ) )
                {
                    nRet = AddNonBrkSpace;
                }
                else if ( bIsNextRun && !IsAutoCorrectChar( cChar ) )
                {
                    // Remove the NBSP if it wasn't an autocorrection
                    if ( nInsPos != 0 && NeedsHardspaceAutocorr( rTxt[ nInsPos - 1 ] ) &&
                            cChar != ' ' && cChar != '\t' && cChar != cNonBreakingSpace )
                    {
                        // Look for the last HARD_SPACE
                        sal_Int32 nPos = nInsPos - 1;
                        bool bContinue = true;
                        while ( bContinue )
                        {
                            const sal_Unicode cTmpChar = rTxt[ nPos ];
                            if ( cTmpChar == cNonBreakingSpace )
                            {
                                rDoc.Delete( nPos, nPos + 1 );
                                nRet = AddNonBrkSpace;
                                bContinue = false;
                            }
                            else if ( !NeedsHardspaceAutocorr( cTmpChar ) || nPos == 0 )
                                bContinue = false;
                            nPos--;
                        }
                    }
                }
            }
        }

        if( !nInsPos )
            break;

        sal_Int32 nPos = nInsPos - 1;

        if( IsWordDelim( rTxt[ nPos ]))
            break;

        // Set bold or underline automatically?
        if (('*' == cChar || '_' == cChar) && (nPos+1 < rTxt.getLength()))
        {
            if( IsAutoCorrFlag( ChgWeightUnderl ) &&
                FnChgWeightUnderl( rDoc, rTxt, 0, nPos+1 ) )
                nRet = ChgWeightUnderl;
            break;
        }

        while( nPos && !IsWordDelim( rTxt[ --nPos ]))
            ;

        // Found a Paragraph-start or a Blank, search for the word shortcut in
        // auto.
        sal_Int32 nCapLttrPos = nPos+1;        // on the 1st Character
        if( !nPos && !IsWordDelim( rTxt[ 0 ]))
            --nCapLttrPos;          // Absatz Anfang und kein Blank !

        LanguageType eLang = rDoc.GetLanguage( nCapLttrPos );
        if( LANGUAGE_SYSTEM == eLang )
            eLang = MsLangId::getSystemLanguage();
        CharClass& rCC = GetCharClass( eLang );

        // no symbol characters
        if( lcl_IsSymbolChar( rCC, rTxt, nCapLttrPos, nInsPos ))
            break;

        if( IsAutoCorrFlag( Autocorrect ) )
        {
            OUString aPara;
            OUString* pPara = IsAutoCorrFlag(CapitalStartSentence) ? &aPara : nullptr;

            // since LibO 4.1, '-' is a word separator
            // fdo#67742 avoid "--" to be replaced by "–" if next is "-"
            if( rTxt.endsWith( "---" ) )
                    break;
            bool bChgWord = rDoc.ChgAutoCorrWord( nCapLttrPos, nInsPos,
                                                    *this, pPara );
            if( !bChgWord )
            {
                sal_Int32 nCapLttrPos1 = nCapLttrPos, nInsPos1 = nInsPos;
                while( nCapLttrPos1 < nInsPos &&
                        lcl_IsInAsciiArr( sImplSttSkipChars, rTxt[ nCapLttrPos1 ] )
                        )
                        ++nCapLttrPos1;
                while( nCapLttrPos1 < nInsPos1 && nInsPos1 &&
                        lcl_IsInAsciiArr( sImplEndSkipChars, rTxt[ nInsPos1-1 ] )
                        )
                        --nInsPos1;

                if( (nCapLttrPos1 != nCapLttrPos || nInsPos1 != nInsPos ) &&
                    nCapLttrPos1 < nInsPos1 &&
                    rDoc.ChgAutoCorrWord( nCapLttrPos1, nInsPos1, *this, pPara ))
                {
                    bChgWord = true;
                    nCapLttrPos = nCapLttrPos1;
                }
            }

            if( bChgWord )
            {
                nRet = Autocorrect;
                if( !aPara.isEmpty() )
                {
                    sal_Int32 nEnd = nCapLttrPos;
                    while( nEnd < aPara.getLength() &&
                            !IsWordDelim( aPara[ nEnd ]))
                        ++nEnd;

                    // Capital letter at beginning of paragraph?
                    if( IsAutoCorrFlag( CapitalStartSentence ) &&
                        FnCapitalStartSentence( rDoc, aPara, false,
                                                nCapLttrPos, nEnd, eLang ) )
                        nRet |= CapitalStartSentence;

                    if( IsAutoCorrFlag( ChgToEnEmDash ) &&
                        FnChgToEnEmDash( rDoc, rTxt, nCapLttrPos, nEnd, eLang ) )
                        nRet |= ChgToEnEmDash;
                }
                break;
            }
        }

        if( ( IsAutoCorrFlag( nRet = ChgOrdinalNumber ) &&
                (nInsPos >= 2 ) &&       // fdo#69762 avoid autocorrect for 2e-3
                ( '-' != cChar || 'E' != toupper(rTxt[nInsPos-1]) || '0' > rTxt[nInsPos-2] || '9' < rTxt[nInsPos-2] ) &&
                FnChgOrdinalNumber( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) ||
            ( IsAutoCorrFlag( nRet = SetINetAttr ) &&
                ( ' ' == cChar || '\t' == cChar || 0x0a == cChar || !cChar ) &&
                FnSetINetAttr( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) )
            ;
        else
        {
            bool bLockKeyOn = pFrameWin && (pFrameWin->GetIndicatorState() & KeyIndicatorState::CAPSLOCK);
            bool bUnsupported = lcl_IsUnsupportedUnicodeChar( rCC, rTxt, nCapLttrPos, nInsPos );

            nRet = 0;
            if ( bLockKeyOn && IsAutoCorrFlag( CorrectCapsLock ) &&
                 FnCorrectCapsLock( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
            {
                // Correct accidental use of cAPS LOCK key (do this only when
                // the caps or shift lock key is pressed).  Turn off the caps
                // lock afterwords.
                nRet |= CorrectCapsLock;
                pFrameWin->SimulateKeyPress( KEY_CAPSLOCK );
            }

            // Capital letter at beginning of paragraph ?
            if( !bUnsupported &&
                IsAutoCorrFlag( CapitalStartSentence ) &&
                FnCapitalStartSentence( rDoc, rTxt, true, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CapitalStartSentence;

            // Two capital letters at beginning of word ??
            if( !bUnsupported &&
                IsAutoCorrFlag( CapitalStartWord ) &&
                FnCapitalStartWord( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CapitalStartWord;

            if( IsAutoCorrFlag( ChgToEnEmDash ) &&
                FnChgToEnEmDash( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
                nRet |= ChgToEnEmDash;
        }

    } while( false );

    return nRet;
}

SvxAutoCorrectLanguageLists& SvxAutoCorrect::_GetLanguageList(
                                                        LanguageType eLang )
{
    LanguageTag aLanguageTag( eLang);
    if (m_pLangTable->find(aLanguageTag) == m_pLangTable->end())
        (void)CreateLanguageFile(aLanguageTag);
    return *(m_pLangTable->find(aLanguageTag)->second);
}

void SvxAutoCorrect::SaveCplSttExceptList( LanguageType eLang )
{
    auto const iter = m_pLangTable->find(LanguageTag(eLang));
    if (iter != m_pLangTable->end() && iter->second)
        iter->second->SaveCplSttExceptList();
#ifdef DBG_UTIL
    else
    {
        SAL_WARN("editeng", "Save an empty list? ");
    }
#endif
}

void SvxAutoCorrect::SaveWrdSttExceptList(LanguageType eLang)
{
    auto const iter = m_pLangTable->find(LanguageTag(eLang));
    if (iter != m_pLangTable->end() && iter->second)
        iter->second->SaveWrdSttExceptList();
#ifdef DBG_UTIL
    else
    {
        SAL_WARN("editeng", "Save an empty list? ");
    }
#endif
}

// Adds a single word. The list will immediately be written to the file!
bool SvxAutoCorrect::AddCplSttException( const OUString& rNew,
                                        LanguageType eLang )
{
    SvxAutoCorrectLanguageLists* pLists = nullptr;
    // either the right language is present or it will be this in the general list
    auto iter = m_pLangTable->find(LanguageTag(eLang));
    if (iter != m_pLangTable->end())
        pLists = iter->second.get();
    else
    {
        LanguageTag aLangTagUndetermined( LANGUAGE_UNDETERMINED);
        iter = m_pLangTable->find(aLangTagUndetermined);
        if (iter != m_pLangTable->end())
            pLists = iter->second.get();
        else if(CreateLanguageFile(aLangTagUndetermined))
            pLists = m_pLangTable->find(aLangTagUndetermined)->second.get();
    }
    OSL_ENSURE(pLists, "No auto correction data");
    return pLists && pLists->AddToCplSttExceptList(rNew);
}

// Adds a single word. The list will immediately be written to the file!
bool SvxAutoCorrect::AddWrtSttException( const OUString& rNew,
                                         LanguageType eLang )
{
    SvxAutoCorrectLanguageLists* pLists = nullptr;
    //either the right language is present or it is set in the general list
    auto iter = m_pLangTable->find(LanguageTag(eLang));
    if (iter != m_pLangTable->end())
        pLists = iter->second.get();
    else
    {
        LanguageTag aLangTagUndetermined( LANGUAGE_UNDETERMINED);
        iter = m_pLangTable->find(aLangTagUndetermined);
        if (iter != m_pLangTable->end())
            pLists = iter->second.get();
        else if(CreateLanguageFile(aLangTagUndetermined))
            pLists = m_pLangTable->find(aLangTagUndetermined)->second.get();
    }
    OSL_ENSURE(pLists, "No auto correction file!");
    return pLists && pLists->AddToWrdSttExceptList(rNew);
}

bool SvxAutoCorrect::GetPrevAutoCorrWord( SvxAutoCorrDoc& rDoc,
                                        const OUString& rTxt, sal_Int32 nPos,
                                        OUString& rWord ) const
{
    if( !nPos )
        return false;

    sal_Int32 nEnde = nPos;

    // it must be followed by a blank or tab!
    if( ( nPos < rTxt.getLength() &&
        !IsWordDelim( rTxt[ nPos ])) ||
        IsWordDelim( rTxt[ --nPos ]))
        return false;

    while( nPos && !IsWordDelim( rTxt[ --nPos ]))
        ;

    // Found a Paragraph-start or a Blank, search for the word shortcut in
    // auto.
    sal_Int32 nCapLttrPos = nPos+1;        // on the 1st Character
    if( !nPos && !IsWordDelim( rTxt[ 0 ]))
        --nCapLttrPos;          // Beginning of pargraph and no Blank!

    while( lcl_IsInAsciiArr( sImplSttSkipChars, rTxt[ nCapLttrPos ]) )
        if( ++nCapLttrPos >= nEnde )
            return false;

    if( 3 > nEnde - nCapLttrPos )
        return false;

    LanguageType eLang = rDoc.GetLanguage( nCapLttrPos );
    if( LANGUAGE_SYSTEM == eLang )
        eLang = MsLangId::getSystemLanguage();

    SvxAutoCorrect* pThis = const_cast<SvxAutoCorrect*>(this);
    CharClass& rCC = pThis->GetCharClass( eLang );

    if( lcl_IsSymbolChar( rCC, rTxt, nCapLttrPos, nEnde ))
        return false;

    rWord = rTxt.copy( nCapLttrPos, nEnde - nCapLttrPos );
    return true;
}

bool SvxAutoCorrect::CreateLanguageFile( const LanguageTag& rLanguageTag, bool bNewFile )
{
    OSL_ENSURE(m_pLangTable->find(rLanguageTag) == m_pLangTable->end(), "Language already exists ");

    OUString sUserDirFile( GetAutoCorrFileName( rLanguageTag, true ));
    OUString sShareDirFile( sUserDirFile );

    SvxAutoCorrectLanguageLists* pLists = nullptr;

    tools::Time nMinTime( 0, 2 ), nAktTime( tools::Time::SYSTEM ), nLastCheckTime( tools::Time::EMPTY );

    std::map<LanguageTag, long>::iterator nFndPos = aLastFileTable.find(rLanguageTag);
    if(nFndPos != aLastFileTable.end() &&
       (nLastCheckTime.SetTime(nFndPos->second), nLastCheckTime < nAktTime) &&
       nAktTime - nLastCheckTime < nMinTime)
    {
        // no need to test the file, because the last check is not older then
        // 2 minutes.
        if( bNewFile )
        {
            sShareDirFile = sUserDirFile;
            pLists = new SvxAutoCorrectLanguageLists( *this, sShareDirFile, sUserDirFile );
            LanguageTag aTmp(rLanguageTag);     // this insert() needs a non-const reference
            m_pLangTable->insert(std::make_pair(aTmp, std::unique_ptr<SvxAutoCorrectLanguageLists>(pLists)));
            aLastFileTable.erase(nFndPos);
        }
    }
    else if(
             ( FStatHelper::IsDocument( sUserDirFile ) ||
               FStatHelper::IsDocument( sShareDirFile =
                   GetAutoCorrFileName( rLanguageTag ) ) ||
               FStatHelper::IsDocument( sShareDirFile =
                   GetAutoCorrFileName( rLanguageTag, false, false, true) )
             ) ||
        ( sShareDirFile = sUserDirFile, bNewFile )
          )
    {
        pLists = new SvxAutoCorrectLanguageLists( *this, sShareDirFile, sUserDirFile );
        LanguageTag aTmp(rLanguageTag);     // this insert() needs a non-const reference
        m_pLangTable->insert(std::make_pair(aTmp, std::unique_ptr<SvxAutoCorrectLanguageLists>(pLists)));
        if (nFndPos != aLastFileTable.end())
            aLastFileTable.erase(nFndPos);
    }
    else if( !bNewFile )
    {
        aLastFileTable[rLanguageTag] = nAktTime.GetTime();
    }
    return pLists != nullptr;
}

bool SvxAutoCorrect::PutText( const OUString& rShort, const OUString& rLong,
                                LanguageType eLang )
{
    LanguageTag aLanguageTag( eLang);
    auto const iter = m_pLangTable->find(aLanguageTag);
    if (iter != m_pLangTable->end())
        return iter->second->PutText(rShort, rLong);
    if(CreateLanguageFile(aLanguageTag))
        return m_pLangTable->find(aLanguageTag)->second->PutText(rShort, rLong);
    return false;
}

bool SvxAutoCorrect::MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries,
                                              std::vector<SvxAutocorrWord>& aDeleteEntries,
                                              LanguageType eLang )
{
    LanguageTag aLanguageTag( eLang);
    auto const iter = m_pLangTable->find(aLanguageTag);
    if (iter != m_pLangTable->end())
    {
        return iter->second->MakeCombinedChanges( aNewEntries, aDeleteEntries );
    }
    else if(CreateLanguageFile( aLanguageTag ))
    {
        return m_pLangTable->find( aLanguageTag )->second->MakeCombinedChanges( aNewEntries, aDeleteEntries );
    }
    return false;

}

//  - return the replacement text (only for SWG-Format, all other
//    can be taken from the word list!)
bool SvxAutoCorrect::GetLongText( const OUString&, OUString& )
{
    return false;
}

void SvxAutoCorrect::refreshBlockList( const uno::Reference< embed::XStorage >& )
{
}

// Text with attribution (only the SWG - SWG format!)
bool SvxAutoCorrect::PutText( const css::uno::Reference < css::embed::XStorage >&,
                              const OUString&, const OUString&, SfxObjectShell&, OUString& )
{
    return false;
}

OUString EncryptBlockName_Imp(const OUString& rName)
{
    OUStringBuffer aName;
    aName.append('#').append(rName);
    for (sal_Int32 nLen = rName.getLength(), nPos = 1; nPos < nLen; ++nPos)
    {
        if (lcl_IsInAsciiArr( "!/:.\\", aName[nPos]))
            aName[nPos] &= 0x0f;
    }
    return aName.makeStringAndClear();
}

/* This code is copied from SwXMLTextBlocks::GeneratePackageName */
static void GeneratePackageName ( const OUString& rShort, OUString& rPackageName )
{
    OString sByte(OUStringToOString(rShort, RTL_TEXTENCODING_UTF7));
    OUStringBuffer aBuf(OStringToOUString(sByte, RTL_TEXTENCODING_ASCII_US));

    for (sal_Int32 nPos = 0; nPos < aBuf.getLength(); ++nPos)
    {
        switch (aBuf[nPos])
        {
            case '!':
            case '/':
            case ':':
            case '.':
            case '\\':
                aBuf[nPos] = '_';
                break;
            default:
                break;
        }
    }

    rPackageName = aBuf.makeStringAndClear();
}

static const SvxAutocorrWord* lcl_SearchWordsInList(
                SvxAutoCorrectLanguageLists* pList, const OUString& rTxt,
                sal_Int32& rStt, sal_Int32 nEndPos)
{
    const SvxAutocorrWordList* pAutoCorrWordList = pList->GetAutocorrWordList();
    return pAutoCorrWordList->SearchWordsInList( rTxt, rStt, nEndPos );
}

// the search for the words in the substitution table
const SvxAutocorrWord* SvxAutoCorrect::SearchWordsInList(
                const OUString& rTxt, sal_Int32& rStt, sal_Int32 nEndPos,
                SvxAutoCorrDoc&, LanguageTag& rLang )
{
    const SvxAutocorrWord* pRet = nullptr;
    LanguageTag aLanguageTag( rLang);
    if( aLanguageTag.isSystemLocale() )
        aLanguageTag.reset( MsLangId::getSystemLanguage());

    /* TODO-BCP47: this is so ugly, should all maybe be a proper fallback
     * list instead? */

    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_UNDETERMINED
    if (m_pLangTable->find(aLanguageTag) != m_pLangTable->end() || CreateLanguageFile(aLanguageTag, false))
    {
        //the language is available - so bring it on
        std::unique_ptr<SvxAutoCorrectLanguageLists> const& pList = m_pLangTable->find(aLanguageTag)->second;
        pRet = lcl_SearchWordsInList( pList.get(), rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = aLanguageTag;
            return pRet;
        }
    }

    // If it still could not be found here, then keep on searching
    LanguageType eLang = aLanguageTag.getLanguageType();
    // the primary language for example EN
    aLanguageTag.reset(aLanguageTag.getLanguage());
    LanguageType nTmpKey = aLanguageTag.getLanguageType(false);
    if (nTmpKey != eLang && nTmpKey != LANGUAGE_UNDETERMINED &&
                (m_pLangTable->find(aLanguageTag) != m_pLangTable->end() ||
                 CreateLanguageFile(aLanguageTag, false)))
    {
        //the language is available - so bring it on
        std::unique_ptr<SvxAutoCorrectLanguageLists> const& pList = m_pLangTable->find(aLanguageTag)->second;
        pRet = lcl_SearchWordsInList( pList.get(), rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = aLanguageTag;
            return pRet;
        }
    }

    if (m_pLangTable->find(aLanguageTag.reset(LANGUAGE_UNDETERMINED)) != m_pLangTable->end() ||
            CreateLanguageFile(aLanguageTag, false))
    {
        //the language is available - so bring it on
        std::unique_ptr<SvxAutoCorrectLanguageLists> const& pList = m_pLangTable->find(aLanguageTag)->second;
        pRet = lcl_SearchWordsInList( pList.get(), rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = aLanguageTag;
            return pRet;
        }
    }
    return nullptr;
}

bool SvxAutoCorrect::FindInWrdSttExceptList( LanguageType eLang,
                                             const OUString& sWord )
{
    LanguageTag aLanguageTag( eLang);

    /* TODO-BCP47: again horrible uglyness */

    // First search for eLang, then primary language of eLang
    // and last in LANGUAGE_UNDETERMINED
    OUString sTemp(sWord);

    if (m_pLangTable->find(aLanguageTag) != m_pLangTable->end() || CreateLanguageFile(aLanguageTag, false))
    {
        //the language is available - so bring it on
        auto const& pList = m_pLangTable->find(aLanguageTag)->second;
        OUString _sTemp(sWord);
        if(pList->GetWrdSttExceptList()->find(_sTemp) != pList->GetWrdSttExceptList()->end() )
            return true;
    }

    // If it still could not be found here, then keep on searching
    // the primary language for example EN
    aLanguageTag.reset(aLanguageTag.getLanguage());
    LanguageType nTmpKey = aLanguageTag.getLanguageType(false);
    if (nTmpKey != eLang && nTmpKey != LANGUAGE_UNDETERMINED &&
                (m_pLangTable->find(aLanguageTag) != m_pLangTable->end() ||
                 CreateLanguageFile(aLanguageTag, false)))
    {
        //the language is available - so bring it on
        auto const& pList = m_pLangTable->find(aLanguageTag)->second;
        if(pList->GetWrdSttExceptList()->find(sTemp) != pList->GetWrdSttExceptList()->end() )
            return true;
    }

    if (m_pLangTable->find(aLanguageTag.reset(LANGUAGE_UNDETERMINED)) != m_pLangTable->end() ||
            CreateLanguageFile(aLanguageTag, false))
    {
        //the language is available - so bring it on
        auto const& pList = m_pLangTable->find(aLanguageTag)->second;
        if(pList->GetWrdSttExceptList()->find(sTemp) != pList->GetWrdSttExceptList()->end() )
            return true;
    }
    return false;
}

static bool lcl_FindAbbreviation(const SvStringsISortDtor* pList, const OUString& sWord)
{
    OUString sAbk('~');
    SvStringsISortDtor::const_iterator it = pList->find( sAbk );
    sal_uInt16 nPos = it - pList->begin();
    if( nPos < pList->size() )
    {
        OUString sLowerWord(sWord.toAsciiLowerCase());
        OUString pAbk;
        for( sal_uInt16 n = nPos;
                n < pList->size() &&
                '~' == ( pAbk = (*pList)[ n ])[ 0 ];
            ++n )
        {
            // ~ and ~. are not allowed!
            if( 2 < pAbk.getLength() && pAbk.getLength() - 1 <= sWord.getLength() )
            {
                OUString sLowerAbk(pAbk.toAsciiLowerCase());
                for (sal_Int32 i = sLowerAbk.getLength(), ii = sLowerWord.getLength(); i;)
                {
                    if( !--i )      // agrees
                        return true;

                    if( sLowerAbk[i] != sLowerWord[--ii])
                        break;
                }
            }
        }
    }
    OSL_ENSURE( !(nPos && '~' == (*pList)[ --nPos ][ 0 ] ),
            "Wrongly sorted exception list?" );
    return false;
}

bool SvxAutoCorrect::FindInCplSttExceptList(LanguageType eLang,
                                const OUString& sWord, bool bAbbreviation)
{
    LanguageTag aLanguageTag( eLang);

    /* TODO-BCP47: did I mention terrible horrible uglyness? */

    // First search for eLang, then primary language of eLang
    // and last in LANGUAGE_UNDETERMINED
    OUString sTemp( sWord );

    if (m_pLangTable->find(aLanguageTag) != m_pLangTable->end() || CreateLanguageFile(aLanguageTag, false))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = m_pLangTable->find(aLanguageTag)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return true;
    }

    // If it still could not be found here, then keep on searching
    // the primary language for example EN
    aLanguageTag.reset(aLanguageTag.getLanguage());
    LanguageType nTmpKey = aLanguageTag.getLanguageType(false);
    if (nTmpKey != eLang && nTmpKey != LANGUAGE_UNDETERMINED &&
                (m_pLangTable->find(aLanguageTag) != m_pLangTable->end() ||
                 CreateLanguageFile(aLanguageTag, false)))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = m_pLangTable->find(aLanguageTag)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return true;
    }

    if (m_pLangTable->find(aLanguageTag.reset(LANGUAGE_UNDETERMINED)) != m_pLangTable->end() ||
            CreateLanguageFile(aLanguageTag, false))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = m_pLangTable->find(aLanguageTag)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return true;
    }
    return false;
}

OUString SvxAutoCorrect::GetAutoCorrFileName( const LanguageTag& rLanguageTag,
                                            bool bNewFile, bool bTst, bool bUnlocalized ) const
{
    OUString sRet, sExt( rLanguageTag.getBcp47() );
    if (bUnlocalized)
    {
        // we don't want variant, so we'll take "fr" instead of "fr-CA" for example
        ::std::vector< OUString > vecFallBackStrings = rLanguageTag.getFallbackStrings(false);
        if (!vecFallBackStrings.empty())
           sExt = vecFallBackStrings[0];
    }

    sExt = "_" + sExt + ".dat";
    if( bNewFile )
        ( sRet = sUserAutoCorrFile )  += sExt;
    else if( !bTst )
        ( sRet = sShareAutoCorrFile )  += sExt;
    else
    {
        // test first in the user directory - if not exist, then
        ( sRet = sUserAutoCorrFile ) += sExt;
        if( !FStatHelper::IsDocument( sRet ))
            ( sRet = sShareAutoCorrFile ) += sExt;
    }
    return sRet;
}

SvxAutoCorrectLanguageLists::SvxAutoCorrectLanguageLists(
                SvxAutoCorrect& rParent,
                const OUString& rShareAutoCorrectFile,
                const OUString& rUserAutoCorrectFile)
:   sShareAutoCorrFile( rShareAutoCorrectFile ),
    sUserAutoCorrFile( rUserAutoCorrectFile ),
    aModifiedDate( Date::EMPTY ),
    aModifiedTime( tools::Time::EMPTY ),
    aLastCheckTime( tools::Time::EMPTY ),
    pCplStt_ExcptLst( nullptr ),
    pWrdStt_ExcptLst( nullptr ),
    pAutocorr_List( nullptr ),
    rAutoCorrect(rParent),
    nFlags(0)
{
}

SvxAutoCorrectLanguageLists::~SvxAutoCorrectLanguageLists()
{
    delete pCplStt_ExcptLst;
    delete pWrdStt_ExcptLst;
    delete pAutocorr_List;
}

bool SvxAutoCorrectLanguageLists::IsFileChanged_Imp()
{
    // Access the file system only every 2 minutes to check the date stamp
    bool bRet = false;

    tools::Time nMinTime( 0, 2 );
    tools::Time nAktTime( tools::Time::SYSTEM );
    if( aLastCheckTime > nAktTime ||                    // overflow?
        ( nAktTime -= aLastCheckTime ) > nMinTime )     // min time past
    {
        Date aTstDate( Date::EMPTY ); tools::Time aTstTime( tools::Time::EMPTY );
        if( FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                            &aTstDate, &aTstTime ) &&
            ( aModifiedDate != aTstDate || aModifiedTime != aTstTime ))
        {
            bRet = true;
            // then remove all the lists fast!
            if( CplSttLstLoad & nFlags && pCplStt_ExcptLst )
                delete pCplStt_ExcptLst, pCplStt_ExcptLst = nullptr;
            if( WrdSttLstLoad & nFlags && pWrdStt_ExcptLst )
                delete pWrdStt_ExcptLst, pWrdStt_ExcptLst = nullptr;
            if( ChgWordLstLoad & nFlags && pAutocorr_List )
                delete pAutocorr_List, pAutocorr_List = nullptr;
            nFlags &= ~(CplSttLstLoad | WrdSttLstLoad | ChgWordLstLoad );
        }
        aLastCheckTime = tools::Time( tools::Time::SYSTEM );
    }
    return bRet;
}

void SvxAutoCorrectLanguageLists::LoadXMLExceptList_Imp(
                                        SvStringsISortDtor*& rpLst,
                                        const sal_Char* pStrmName,
                                        tools::SvRef<SotStorage>& rStg)
{
    if( rpLst )
        rpLst->clear();
    else
        rpLst = new SvStringsISortDtor;

    {
        OUString sStrmName( pStrmName, strlen(pStrmName), RTL_TEXTENCODING_MS_1252 );
        OUString sTmp( sStrmName );

        if( rStg.Is() && rStg->IsStream( sStrmName ) )
        {
            tools::SvRef<SotStorageStream> xStrm = rStg->OpenSotStream( sTmp,
                ( StreamMode::READ | StreamMode::SHARE_DENYWRITE | StreamMode::NOCREATE ) );
            if( SVSTREAM_OK != xStrm->GetError())
            {
                xStrm.Clear();
                rStg.Clear();
                RemoveStream_Imp( sStrmName );
            }
            else
            {
                uno::Reference< uno::XComponentContext > xContext =
                    comphelper::getProcessComponentContext();

                xml::sax::InputSource aParserInput;
                aParserInput.sSystemId = sStrmName;

                xStrm->Seek( 0L );
                xStrm->SetBufferSize( 8 * 1024 );
                aParserInput.aInputStream = new utl::OInputStreamWrapper( *xStrm );

                // get filter
                uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SvXMLExceptionListImport ( xContext, *rpLst );

                // connect parser and filter
                uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create( xContext );
                uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = static_cast< xml::sax::XFastTokenHandler* >( new SvXMLAutoCorrectTokenHandler );
                xParser->setFastDocumentHandler( xFilter );
                xParser->registerNamespace( "http://openoffice.org/2001/block-list", SvXMLAutoCorrectToken::NAMESPACE );
                xParser->setTokenHandler( xTokenHandler );

                // parse
                try
                {
                    xParser->parseStream( aParserInput );
                }
                catch( const xml::sax::SAXParseException& )
                {
                    // re throw ?
                }
                catch( const xml::sax::SAXException& )
                {
                    // re throw ?
                }
                catch( const io::IOException& )
                {
                    // re throw ?
                }
            }
        }

        // Set time stamp
        FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                        &aModifiedDate, &aModifiedTime );
        aLastCheckTime = tools::Time( tools::Time::SYSTEM );
    }

}

void SvxAutoCorrectLanguageLists::SaveExceptList_Imp(
                            const SvStringsISortDtor& rLst,
                            const sal_Char* pStrmName,
                            tools::SvRef<SotStorage> &rStg,
                            bool bConvert )
{
    if( rStg.Is() )
    {
        OUString sStrmName( pStrmName, strlen(pStrmName), RTL_TEXTENCODING_MS_1252 );
        if( rLst.empty() )
        {
            rStg->Remove( sStrmName );
            rStg->Commit();
        }
        else
        {
            tools::SvRef<SotStorageStream> xStrm = rStg->OpenSotStream( sStrmName,
                    ( StreamMode::READ | StreamMode::WRITE | StreamMode::SHARE_DENYWRITE ) );
            if( xStrm.Is() )
            {
                xStrm->SetSize( 0 );
                xStrm->SetBufferSize( 8192 );
                OUString aMime( "text/xml" );
                uno::Any aAny;
                aAny <<= aMime;
                xStrm->SetProperty( "MediaType", aAny );


                uno::Reference< uno::XComponentContext > xContext =
                    comphelper::getProcessComponentContext();

                uno::Reference < xml::sax::XWriter > xWriter  = xml::sax::Writer::create(xContext);
                uno::Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *xStrm );
                xWriter->setOutputStream(xOut);

                uno::Reference < xml::sax::XDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
                uno::Reference< SvXMLExceptionListExport > xExp( new SvXMLExceptionListExport( xContext, rLst, sStrmName, xHandler ) );

                xExp->exportDoc( XML_BLOCK_LIST );

                xStrm->Commit();
                if( xStrm->GetError() == SVSTREAM_OK )
                {
                    xStrm.Clear();
                    if (!bConvert)
                    {
                        rStg->Commit();
                        if( SVSTREAM_OK != rStg->GetError() )
                        {
                            rStg->Remove( sStrmName );
                            rStg->Commit();
                        }
                    }
                }
            }
        }
    }
}

SvxAutocorrWordList* SvxAutoCorrectLanguageLists::LoadAutocorrWordList()
{
    if( pAutocorr_List )
        pAutocorr_List->DeleteAndDestroyAll();
    else
        pAutocorr_List = new SvxAutocorrWordList();

    try
    {
        uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetStorageFromURL( sShareAutoCorrFile, embed::ElementModes::READ );
        OUString aXMLWordListName( pXMLImplAutocorr_ListStr, strlen(pXMLImplAutocorr_ListStr), RTL_TEXTENCODING_MS_1252 );
        uno::Reference < io::XStream > xStrm = xStg->openStreamElement( aXMLWordListName, embed::ElementModes::READ );
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aXMLWordListName;
        aParserInput.aInputStream = xStrm->getInputStream();

        // get parser
        uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create(xContext);
        SAL_INFO("editeng", "AutoCorrect Import" );
        uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SvXMLAutoCorrectImport( xContext, pAutocorr_List, rAutoCorrect, xStg );
        uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = static_cast< xml::sax::XFastTokenHandler* >( new SvXMLAutoCorrectTokenHandler );

        // connect parser and filter
        xParser->setFastDocumentHandler( xFilter );
        xParser->registerNamespace( "http://openoffice.org/2001/block-list", SvXMLAutoCorrectToken::NAMESPACE );
        xParser->setTokenHandler(xTokenHandler);

        // parse
        xParser->parseStream( aParserInput );
    }
    catch ( const uno::Exception& )
    {
    }

    // Set time stamp
    FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                    &aModifiedDate, &aModifiedTime );
    aLastCheckTime = tools::Time( tools::Time::SYSTEM );

    return pAutocorr_List;
}

void SvxAutoCorrectLanguageLists::SetAutocorrWordList( SvxAutocorrWordList* pList )
{
    if( pAutocorr_List && pList != pAutocorr_List )
        delete pAutocorr_List;
    pAutocorr_List = pList;
    if( !pAutocorr_List )
    {
        OSL_ENSURE( false, "No valid list" );
        pAutocorr_List = new SvxAutocorrWordList();
    }
    nFlags |= ChgWordLstLoad;
}

const SvxAutocorrWordList* SvxAutoCorrectLanguageLists::GetAutocorrWordList()
{
    if( !( ChgWordLstLoad & nFlags ) || IsFileChanged_Imp() )
        SetAutocorrWordList( LoadAutocorrWordList() );
    return pAutocorr_List;
}

SvStringsISortDtor* SvxAutoCorrectLanguageLists::GetCplSttExceptList()
{
    if( !( CplSttLstLoad & nFlags ) || IsFileChanged_Imp() )
        SetCplSttExceptList( LoadCplSttExceptList() );
    return pCplStt_ExcptLst;
}

bool SvxAutoCorrectLanguageLists::AddToCplSttExceptList(const OUString& rNew)
{
    bool bRet = false;
    if( !rNew.isEmpty() && GetCplSttExceptList()->insert( rNew ).second )
    {
        MakeUserStorage_Impl();
        tools::SvRef<SotStorage> xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );

        SaveExceptList_Imp( *pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );

        xStg = nullptr;
        // Set time stamp
        FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
        aLastCheckTime = tools::Time( tools::Time::SYSTEM );
        bRet = true;
    }
    return bRet;
}

bool SvxAutoCorrectLanguageLists::AddToWrdSttExceptList(const OUString& rNew)
{
    bool bRet = false;
    SvStringsISortDtor* pExceptList = LoadWrdSttExceptList();
    if( !rNew.isEmpty() && pExceptList && pExceptList->insert( rNew ).second )
    {
        MakeUserStorage_Impl();
        tools::SvRef<SotStorage> xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );

        SaveExceptList_Imp( *pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );

        xStg = nullptr;
        // Set time stamp
        FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
        aLastCheckTime = tools::Time( tools::Time::SYSTEM );
        bRet = true;
    }
    return bRet;
}

SvStringsISortDtor* SvxAutoCorrectLanguageLists::LoadCplSttExceptList()
{
    try
    {
        tools::SvRef<SotStorage> xStg = new SotStorage( sShareAutoCorrFile, StreamMode::READ | StreamMode::SHARE_DENYNONE );
        OUString sTemp ( pXMLImplCplStt_ExcptLstStr );
        if( xStg.Is() && xStg->IsContained( sTemp ) )
            LoadXMLExceptList_Imp( pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );
    }
    catch (const css::ucb::ContentCreationException&)
    {
    }
    return pCplStt_ExcptLst;
}

void SvxAutoCorrectLanguageLists::SaveCplSttExceptList()
{
    MakeUserStorage_Impl();
    tools::SvRef<SotStorage> xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );

    SaveExceptList_Imp( *pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );

    xStg = nullptr;

    // Set time stamp
    FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
    aLastCheckTime = tools::Time( tools::Time::SYSTEM );
}

void SvxAutoCorrectLanguageLists::SetCplSttExceptList( SvStringsISortDtor* pList )
{
    if( pCplStt_ExcptLst && pList != pCplStt_ExcptLst )
        delete pCplStt_ExcptLst;

    pCplStt_ExcptLst = pList;
    if( !pCplStt_ExcptLst )
    {
        OSL_ENSURE( false, "No valid list" );
        pCplStt_ExcptLst = new SvStringsISortDtor;
    }
    nFlags |= CplSttLstLoad;
}

SvStringsISortDtor* SvxAutoCorrectLanguageLists::LoadWrdSttExceptList()
{
    try
    {
        tools::SvRef<SotStorage> xStg = new SotStorage( sShareAutoCorrFile, StreamMode::READ | StreamMode::SHARE_DENYNONE );
        OUString sTemp ( pXMLImplWrdStt_ExcptLstStr );
        if( xStg.Is() && xStg->IsContained( sTemp ) )
            LoadXMLExceptList_Imp( pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );
    }
    catch (const css::ucb::ContentCreationException &e)
    {
        SAL_WARN("editeng", "SvxAutoCorrectLanguageLists::LoadWrdSttExceptList: Caught exception: " << e.Message);
    }
    return pWrdStt_ExcptLst;
}

void SvxAutoCorrectLanguageLists::SaveWrdSttExceptList()
{
    MakeUserStorage_Impl();
    tools::SvRef<SotStorage> xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );

    SaveExceptList_Imp( *pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );

    xStg = nullptr;
    // Set time stamp
    FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
    aLastCheckTime = tools::Time( tools::Time::SYSTEM );
}

void SvxAutoCorrectLanguageLists::SetWrdSttExceptList( SvStringsISortDtor* pList )
{
    if( pWrdStt_ExcptLst && pList != pWrdStt_ExcptLst )
        delete pWrdStt_ExcptLst;
    pWrdStt_ExcptLst = pList;
    if( !pWrdStt_ExcptLst )
    {
        OSL_ENSURE( false, "No valid list" );
        pWrdStt_ExcptLst = new SvStringsISortDtor;
    }
    nFlags |= WrdSttLstLoad;
}

SvStringsISortDtor* SvxAutoCorrectLanguageLists::GetWrdSttExceptList()
{
    if( !( WrdSttLstLoad & nFlags ) || IsFileChanged_Imp() )
        SetWrdSttExceptList( LoadWrdSttExceptList() );
    return pWrdStt_ExcptLst;
}

void SvxAutoCorrectLanguageLists::RemoveStream_Imp( const OUString& rName )
{
    if( sShareAutoCorrFile != sUserAutoCorrFile )
    {
        tools::SvRef<SotStorage> xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );
        if( xStg.Is() && SVSTREAM_OK == xStg->GetError() &&
            xStg->IsStream( rName ) )
        {
            xStg->Remove( rName );
            xStg->Commit();

            xStg = nullptr;
        }
    }
}

void SvxAutoCorrectLanguageLists::MakeUserStorage_Impl()
{
    // The conversion needs to happen if the file is already in the user
    // directory and is in the old format. Additionally it needs to
    // happen when the file is being copied from share to user.

    bool bError = false, bConvert = false, bCopy = false;
    INetURLObject aDest;
    INetURLObject aSource;

    if (sUserAutoCorrFile != sShareAutoCorrFile )
    {
        aSource = INetURLObject ( sShareAutoCorrFile );
        aDest = INetURLObject ( sUserAutoCorrFile );
        if ( SotStorage::IsOLEStorage ( sShareAutoCorrFile ) )
        {
            aDest.SetExtension ( "bak" );
            bConvert = true;
        }
        bCopy = true;
    }
    else if ( SotStorage::IsOLEStorage ( sUserAutoCorrFile ) )
    {
        aSource = INetURLObject ( sUserAutoCorrFile );
        aDest = INetURLObject ( sUserAutoCorrFile );
        aDest.SetExtension ( "bak" );
        bCopy = bConvert = true;
    }
    if (bCopy)
    {
        try
        {
            OUString sMain(aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ));
            sal_Unicode cSlash = '/';
            sal_Int32 nSlashPos = sMain.lastIndexOf(cSlash);
            sMain = sMain.copy(0, nSlashPos);
            ::ucbhelper::Content aNewContent( sMain, uno::Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            Any aAny;
            TransferInfo aInfo;
            aInfo.NameClash = NameClash::OVERWRITE;
            aInfo.NewTitle  = aDest.GetName();
            aInfo.SourceURL = aSource.GetMainURL( INetURLObject::DECODE_TO_IURI );
            aInfo.MoveData  = sal_False;
            aAny <<= aInfo;
            aNewContent.executeCommand( "transfer", aAny);
        }
        catch (...)
        {
            bError = true;
        }
    }
    if (bConvert && !bError)
    {
        tools::SvRef<SotStorage> xSrcStg = new SotStorage( aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ), StreamMode::READ );
        tools::SvRef<SotStorage> xDstStg = new SotStorage( sUserAutoCorrFile, StreamMode::WRITE );

        if( xSrcStg.Is() && xDstStg.Is() )
        {
            OUString sXMLWord     ( pXMLImplWrdStt_ExcptLstStr );
            OUString sXMLSentence ( pXMLImplCplStt_ExcptLstStr );
            SvStringsISortDtor  *pTmpWordList = nullptr;

            if (xSrcStg->IsContained( sXMLWord ) )
                LoadXMLExceptList_Imp( pTmpWordList, pXMLImplWrdStt_ExcptLstStr, xSrcStg );

            if (pTmpWordList)
            {
                SaveExceptList_Imp( *pTmpWordList, pXMLImplWrdStt_ExcptLstStr, xDstStg, true );
                pTmpWordList->clear();
                pTmpWordList = nullptr;
            }


            if (xSrcStg->IsContained( sXMLSentence ) )
                LoadXMLExceptList_Imp( pTmpWordList, pXMLImplCplStt_ExcptLstStr, xSrcStg );

            if (pTmpWordList)
            {
                SaveExceptList_Imp( *pTmpWordList, pXMLImplCplStt_ExcptLstStr, xDstStg, true );
                pTmpWordList->clear();
            }

            GetAutocorrWordList();
            MakeBlocklist_Imp( *xDstStg );
            sShareAutoCorrFile = sUserAutoCorrFile;
            xDstStg = nullptr;
            try
            {
                ::ucbhelper::Content aContent ( aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ), uno::Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                aContent.executeCommand ( "delete", makeAny ( true ) );
            }
            catch (...)
            {
            }
        }
    }
    else if( bCopy && !bError )
        sShareAutoCorrFile = sUserAutoCorrFile;
}

bool SvxAutoCorrectLanguageLists::MakeBlocklist_Imp( SotStorage& rStg )
{
    OUString sStrmName( pXMLImplAutocorr_ListStr, strlen(pXMLImplAutocorr_ListStr), RTL_TEXTENCODING_MS_1252 );
    bool bRet = true, bRemove = !pAutocorr_List || pAutocorr_List->empty();
    if( !bRemove )
    {
        tools::SvRef<SotStorageStream> refList = rStg.OpenSotStream( sStrmName,
                    ( StreamMode::READ | StreamMode::WRITE | StreamMode::SHARE_DENYWRITE ) );
        if( refList.Is() )
        {
            refList->SetSize( 0 );
            refList->SetBufferSize( 8192 );
            OUString aPropName( "MediaType" );
            OUString aMime( "text/xml" );
            uno::Any aAny;
            aAny <<= aMime;
            refList->SetProperty( aPropName, aAny );

            uno::Reference< uno::XComponentContext > xContext =
                comphelper::getProcessComponentContext();

            uno::Reference < xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);
            uno::Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *refList );
            xWriter->setOutputStream(xOut);

            uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);
            uno::Reference< SvXMLAutoCorrectExport > xExp( new SvXMLAutoCorrectExport( xContext, pAutocorr_List, sStrmName, xHandler ) );

            xExp->exportDoc( XML_BLOCK_LIST );

            refList->Commit();
            bRet = SVSTREAM_OK == refList->GetError();
            if( bRet )
            {
                refList.Clear();
                rStg.Commit();
                if( SVSTREAM_OK != rStg.GetError() )
                {
                    bRemove = true;
                    bRet = false;
                }
            }
        }
        else
            bRet = false;
    }

    if( bRemove )
    {
        rStg.Remove( sStrmName );
        rStg.Commit();
    }

    return bRet;
}

bool SvxAutoCorrectLanguageLists::MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries, std::vector<SvxAutocorrWord>& aDeleteEntries )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();
    tools::SvRef<SotStorage> xStorage = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );

    bool bRet = xStorage.Is() && SVSTREAM_OK == xStorage->GetError();

    if( bRet )
    {
        for ( size_t i=0; i < aDeleteEntries.size(); i++ )
        {
            SvxAutocorrWord aWordToDelete = aDeleteEntries[i];
            SvxAutocorrWord *pFoundEntry = pAutocorr_List->FindAndRemove( &aWordToDelete );
            if( pFoundEntry )
            {
                if( !pFoundEntry->IsTextOnly() )
                {
                    OUString aName( aWordToDelete.GetShort() );
                    if (xStorage->IsOLEStorage())
                        aName = EncryptBlockName_Imp(aName);
                    else
                        GeneratePackageName ( aWordToDelete.GetShort(), aName );

                    if( xStorage->IsContained( aName ) )
                    {
                        xStorage->Remove( aName );
                        bRet = xStorage->Commit();
                    }
                }
                delete pFoundEntry;
            }
        }

        for ( size_t i=0; i < aNewEntries.size(); i++ )
        {
            SvxAutocorrWord *pWordToAdd = new SvxAutocorrWord( aNewEntries[i].GetShort(), aNewEntries[i].GetLong(), true );
            SvxAutocorrWord *pRemoved = pAutocorr_List->FindAndRemove( pWordToAdd );
            if( pRemoved )
            {
                if( !pRemoved->IsTextOnly() )
                {
                    // Still have to remove the Storage
                    OUString sStorageName( pWordToAdd->GetShort() );
                    if (xStorage->IsOLEStorage())
                        sStorageName = EncryptBlockName_Imp(sStorageName);
                    else
                        GeneratePackageName ( pWordToAdd->GetShort(), sStorageName);

                    if( xStorage->IsContained( sStorageName ) )
                        xStorage->Remove( sStorageName );
                }
                delete pRemoved;
            }
            bRet = pAutocorr_List->Insert( pWordToAdd );

            if ( !bRet )
            {
                delete pWordToAdd;
                break;
            }
        }

        if ( bRet )
        {
            bRet = MakeBlocklist_Imp( *xStorage );
        }
    }
    return bRet;
}

bool SvxAutoCorrectLanguageLists::PutText( const OUString& rShort, const OUString& rLong )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();
    tools::SvRef<SotStorage> xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );

    bool bRet = xStg.Is() && SVSTREAM_OK == xStg->GetError();

    // Update the word list
    if( bRet )
    {
        SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, rLong, true );
        SvxAutocorrWord *pRemove = pAutocorr_List->FindAndRemove( pNew );
        if( pRemove )
        {
            if( !pRemove->IsTextOnly() )
            {
                // Still have to remove the Storage
                OUString sStgNm( rShort );
                if (xStg->IsOLEStorage())
                    sStgNm = EncryptBlockName_Imp(sStgNm);
                else
                    GeneratePackageName ( rShort, sStgNm);

                if( xStg->IsContained( sStgNm ) )
                    xStg->Remove( sStgNm );
            }
            delete pRemove;
        }

        if( pAutocorr_List->Insert( pNew ) )
        {
            bRet = MakeBlocklist_Imp( *xStg );
            xStg = nullptr;
        }
        else
        {
            delete pNew;
            bRet = false;
        }
    }
    return bRet;
}

bool SvxAutoCorrectLanguageLists::PutText( const OUString& rShort,
                                               SfxObjectShell& rShell )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();

    bool bRet = false;
    OUString sLong;
    try
    {
        uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetStorageFromURL( sUserAutoCorrFile, embed::ElementModes::READWRITE );
        bRet = rAutoCorrect.PutText( xStg, sUserAutoCorrFile, rShort, rShell, sLong );
        xStg = nullptr;

        // Update the word list
        if( bRet )
        {
            SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, sLong, false );
            if( pAutocorr_List->Insert( pNew ) )
            {
                tools::SvRef<SotStorage> xStor = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE );
                MakeBlocklist_Imp( *xStor );
            }
            else
                delete pNew;
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return bRet;
}

// Keep the list sorted ...
struct CompareSvxAutocorrWordList
{
    bool operator()( SvxAutocorrWord* const& lhs, SvxAutocorrWord* const& rhs ) const
    {
        CollatorWrapper& rCmp = ::GetCollatorWrapper();
        return rCmp.compareString( lhs->GetShort(), rhs->GetShort() ) < 0;
    }
};

namespace {

typedef std::set<SvxAutocorrWord*, CompareSvxAutocorrWordList> AutocorrWordSetType;
typedef std::unordered_map<OUString, SvxAutocorrWord*, OUStringHash> AutocorrWordHashType;

}

struct SvxAutocorrWordList::Impl
{

    // only one of these contains the data
    mutable AutocorrWordSetType maSet;
    mutable AutocorrWordHashType maHash; // key is 'Short'

    void DeleteAndDestroyAll()
    {
        for (AutocorrWordHashType::const_iterator it = maHash.begin(); it != maHash.end(); ++it)
            delete it->second;
        maHash.clear();

        for (AutocorrWordSetType::const_iterator it2 = maSet.begin(); it2 != maSet.end(); ++it2)
            delete *it2;
        maSet.clear();
    }
};

SvxAutocorrWordList::SvxAutocorrWordList() : mpImpl(new Impl) {}

SvxAutocorrWordList::~SvxAutocorrWordList()
{
    mpImpl->DeleteAndDestroyAll();
}

void SvxAutocorrWordList::DeleteAndDestroyAll()
{
    mpImpl->DeleteAndDestroyAll();
}

// returns true if inserted
bool SvxAutocorrWordList::Insert(SvxAutocorrWord *pWord) const
{
    if ( mpImpl->maSet.empty() ) // use the hash
    {
        OUString aShort( pWord->GetShort() );
        return mpImpl->maHash.insert( std::pair<OUString, SvxAutocorrWord *>( aShort, pWord ) ).second;
    }
    else
        return mpImpl->maSet.insert( pWord ).second;
}

void SvxAutocorrWordList::LoadEntry(const OUString& sWrong, const OUString& sRight, bool bOnlyTxt)
{
    SvxAutocorrWord* pNew = new SvxAutocorrWord( sWrong, sRight, bOnlyTxt );
    if( !Insert( pNew ) )
        delete pNew;
}

bool SvxAutocorrWordList::empty() const
{
    return mpImpl->maHash.empty() && mpImpl->maSet.empty();
}

SvxAutocorrWord *SvxAutocorrWordList::FindAndRemove(SvxAutocorrWord *pWord)
{
    SvxAutocorrWord *pMatch = nullptr;

    if ( mpImpl->maSet.empty() ) // use the hash
    {
        AutocorrWordHashType::iterator it = mpImpl->maHash.find( pWord->GetShort() );
        if( it != mpImpl->maHash.end() )
        {
            pMatch = it->second;
            mpImpl->maHash.erase (it);
        }
    }
    else
    {
        AutocorrWordSetType::iterator it = mpImpl->maSet.find( pWord );
        if( it != mpImpl->maSet.end() )
        {
            pMatch = *it;
            mpImpl->maSet.erase (it);
        }
    }
    return pMatch;
}

// return the sorted contents - defer sorting until we have to.
SvxAutocorrWordList::Content SvxAutocorrWordList::getSortedContent() const
{
    Content aContent;

    // convert from hash to set permanantly
    if ( mpImpl->maSet.empty() )
    {
        // This beast has some O(N log(N)) in a terribly slow ICU collate fn.
        for (AutocorrWordHashType::const_iterator it = mpImpl->maHash.begin(); it != mpImpl->maHash.end(); ++it)
            mpImpl->maSet.insert( it->second );
        mpImpl->maHash.clear();
    }
    for (AutocorrWordSetType::const_iterator it = mpImpl->maSet.begin(); it != mpImpl->maSet.end(); ++it)
        aContent.push_back( *it );

    return aContent;
}

const SvxAutocorrWord* SvxAutocorrWordList::WordMatches(const SvxAutocorrWord *pFnd,
                                      const OUString &rTxt,
                                      sal_Int32 &rStt,
                                      sal_Int32 nEndPos) const
{
    const OUString& rChk = pFnd->GetShort();

    sal_Int32 left_wildcard = rChk.startsWith( ".*" ) ? 2 : 0; // ".*word" pattern?
    sal_Int32 right_wildcard = rChk.endsWith( ".*" ) ? 2 : 0; // "word.*" pattern?
    sal_Int32 nSttWdPos = nEndPos;

    // direct replacement of keywords surrounded by colons (for example, ":name:")
    bool bColonNameColon = rTxt.getLength() > nEndPos &&
        rTxt[nEndPos] == ':' && rChk[0] == ':' && rChk.endsWith(":");
    if ( nEndPos + (bColonNameColon ? 1 : 0) >= rChk.getLength() - left_wildcard - right_wildcard )
    {

        bool bWasWordDelim = false;
        sal_Int32 nCalcStt = nEndPos - rChk.getLength() + left_wildcard;
        if (bColonNameColon)
            nCalcStt++;
        if( !right_wildcard && ( !nCalcStt || nCalcStt == rStt || left_wildcard || bColonNameColon ||
              ( nCalcStt < rStt &&
                IsWordDelim( rTxt[ nCalcStt - 1 ] ))) )
        {
            TransliterationWrapper& rCmp = GetIgnoreTranslWrapper();
            OUString sWord = rTxt.copy(nCalcStt, rChk.getLength() - left_wildcard);
            if( (!left_wildcard && rCmp.isEqual( rChk, sWord )) || (left_wildcard && rCmp.isEqual( rChk.copy(left_wildcard), sWord) ))
            {
                rStt = nCalcStt;
                if (!left_wildcard)
                {
                    // fdo#33899 avoid "1/2", "1/3".. to be replaced by fractions in dates, eg. 1/2/14
                    if (rTxt.getLength() > nEndPos && rTxt[nEndPos] == '/' && rChk.indexOf('/') != -1)
                        return nullptr;
                    return pFnd;
                }
                // get the first word delimiter position before the matching ".*word" pattern
                while( rStt && !(bWasWordDelim = IsWordDelim( rTxt[ --rStt ])))
                    ;
                if (bWasWordDelim) rStt++;
                OUString left_pattern = rTxt.copy(rStt, nEndPos - rStt - rChk.getLength() + left_wildcard);
                // avoid double spaces before simple "word" replacement
                left_pattern += (left_pattern.getLength() == 0 && pFnd->GetLong()[0] == 0x20) ? pFnd->GetLong().copy(1) : pFnd->GetLong();
                SvxAutocorrWord* pNew = new SvxAutocorrWord(rTxt.copy(rStt, nEndPos - rStt), left_pattern);
                if( Insert( pNew ) ) return pNew; else delete pNew;
            }
        } else
        // match "word.*" or ".*word.*" patterns, eg. "i18n.*", ".*---.*", TODO: add transliteration support
        if ( right_wildcard )
        {

            OUString sTmp( rChk.copy( left_wildcard, rChk.getLength() - left_wildcard - right_wildcard ) );
            // Get the last word delimiter position
            bool not_suffix;

            while( nSttWdPos && !(bWasWordDelim = IsWordDelim( rTxt[ --nSttWdPos ])))
                ;
            // search the first occurrence (with a left word delimitation, if needed)
            sal_Int32 nFndPos = -1;
            do {
                nFndPos = rTxt.indexOf( sTmp, nFndPos + 1);
                if (nFndPos == -1)
                    break;
                not_suffix = bWasWordDelim && (nSttWdPos >= (nFndPos + sTmp.getLength()));
            } while ( (!left_wildcard && nFndPos && !IsWordDelim( rTxt[ nFndPos - 1 ])) || not_suffix );

            if ( nFndPos != -1 )
            {
                sal_Int32 extra_repl = nFndPos + sTmp.getLength() > nEndPos ? 1: 0; // for patterns with terminating characters, eg. "a:"

                if ( left_wildcard )
                {
                    // get the first word delimiter position before the matching ".*word.*" pattern
                    while( nFndPos && !(bWasWordDelim = IsWordDelim( rTxt[ --nFndPos ])))
                        ;
                    if (bWasWordDelim) nFndPos++;
                }
                if (nEndPos + extra_repl <= nFndPos)
                {
                    return nullptr;
                }
                // store matching pattern and its replacement as a new list item, eg. "i18ns" -> "internationalizations"
                OUString aShort = rTxt.copy(nFndPos, nEndPos - nFndPos + extra_repl);

                OUString aLong;
                rStt = nFndPos;
                if ( !left_wildcard )
                {
                    sal_Int32 siz = nEndPos - nFndPos - sTmp.getLength();
                    aLong = pFnd->GetLong() + (siz > 0 ? rTxt.copy(nFndPos + sTmp.getLength(), siz) : "");
                } else {
                    OUStringBuffer buf;
                    do {
                        nSttWdPos = rTxt.indexOf( sTmp, nFndPos);
                        if (nSttWdPos != -1)
                        {
                            buf.append(rTxt.copy(nFndPos, nSttWdPos - nFndPos)).append(pFnd->GetLong());
                            nFndPos = nSttWdPos + sTmp.getLength();
                        }
                    } while (nSttWdPos != -1);
                    if (nEndPos - nFndPos > extra_repl) buf.append(rTxt.copy(nFndPos, nEndPos - nFndPos));
                    aLong = buf.makeStringAndClear();
                }
                SvxAutocorrWord* pNew = new SvxAutocorrWord(aShort, aLong);
                if ( Insert( pNew ) )
                {
                    if ( IsWordDelim(rTxt[nEndPos]) ) return pNew;
                } else delete pNew;
            }
        }
    }
    return nullptr;
}

const SvxAutocorrWord* SvxAutocorrWordList::SearchWordsInList(const OUString& rTxt, sal_Int32& rStt,
                                                              sal_Int32 nEndPos) const
{
    for (AutocorrWordHashType::const_iterator it = mpImpl->maHash.begin(); it != mpImpl->maHash.end(); ++it)
    {
        if( const SvxAutocorrWord *aTmp = WordMatches( it->second, rTxt, rStt, nEndPos ) )
            return aTmp;
    }

    for (AutocorrWordSetType::const_iterator it2 = mpImpl->maSet.begin(); it2 != mpImpl->maSet.end(); ++it2)
    {
        if( const SvxAutocorrWord *aTmp = WordMatches( *it2, rTxt, rStt, nEndPos ) )
            return aTmp;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
