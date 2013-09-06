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
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <unotools/streamwrap.hxx>
#include <SvXMLAutoCorrectImport.hxx>
#include <SvXMLAutoCorrectExport.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <xmloff/xmltoken.hxx>
#include <vcl/help.hxx>

#define CHAR_HARDBLANK      ((sal_Unicode)0x00A0)

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;
using namespace ::utl;

static const int C_NONE             = 0x00;
static const int C_FULL_STOP        = 0x01;
static const int C_EXCLAMATION_MARK = 0x02;
static const int C_QUESTION_MARK    = 0x04;
static const int C_ASTERISK         = 0x2A;

static const sal_Char pImplAutocorr_ListStr[]      = "DocumentList";
static const sal_Char pXMLImplWrdStt_ExcptLstStr[] = "WordExceptList.xml";
static const sal_Char pXMLImplCplStt_ExcptLstStr[] = "SentenceExceptList.xml";
static const sal_Char pXMLImplAutocorr_ListStr[]   = "DocumentList.xml";

static const sal_Char
    /* also at these beginnings - Brackets and all kinds of begin characters */
    sImplSttSkipChars[] = "\"\'([{\x83\x84\x89\x91\x92\x93\x94",
    /* also at these ends - Brackets and all kinds of begin characters */
    sImplEndSkipChars[] = "\"\')]}\x83\x84\x89\x91\x92\x93\x94";

// These characters are allowed in words: (for FnCptlSttSntnc)
static const sal_Char sImplWordChars[] = "-'";

OUString EncryptBlockName_Imp(const OUString& rName);

TYPEINIT0(SvxAutoCorrect)

typedef SvxAutoCorrectLanguageLists* SvxAutoCorrectLanguageListsPtr;

static inline int IsWordDelim( const sal_Unicode c )
{
    return ' ' == c || '\t' == c || 0x0a == c ||
            0xA0 == c || 0x2011 == c || 0x1 == c;
}

static inline int IsLowerLetter( sal_Int32 nCharType )
{
    return CharClass::isLetterType( nCharType ) &&
            0 == ( ::com::sun::star::i18n::KCharacterType::UPPER & nCharType);
}

static inline int IsUpperLetter( sal_Int32 nCharType )
{
    return CharClass::isLetterType( nCharType ) &&
            0 == ( ::com::sun::star::i18n::KCharacterType::LOWER & nCharType);
}

bool lcl_IsUnsupportedUnicodeChar( CharClass& rCC, const String& rTxt,
                                   xub_StrLen nStt, xub_StrLen nEnd )
{
    for( ; nStt < nEnd; ++nStt )
    {
        short nScript = rCC.getScript( rTxt, nStt );
        switch( nScript )
        {
            case ::com::sun::star::i18n::UnicodeScript_kCJKRadicalsSupplement:
            case ::com::sun::star::i18n::UnicodeScript_kHangulJamo:
            case ::com::sun::star::i18n::UnicodeScript_kCJKSymbolPunctuation:
            case ::com::sun::star::i18n::UnicodeScript_kHiragana:
            case ::com::sun::star::i18n::UnicodeScript_kKatakana:
            case ::com::sun::star::i18n::UnicodeScript_kHangulCompatibilityJamo:
            case ::com::sun::star::i18n::UnicodeScript_kEnclosedCJKLetterMonth:
            case ::com::sun::star::i18n::UnicodeScript_kCJKCompatibility:
            case ::com::sun::star::i18n::UnicodeScript_k_CJKUnifiedIdeographsExtensionA:
            case ::com::sun::star::i18n::UnicodeScript_kCJKUnifiedIdeograph:
            case ::com::sun::star::i18n::UnicodeScript_kHangulSyllable:
            case ::com::sun::star::i18n::UnicodeScript_kCJKCompatibilityIdeograph:
            case ::com::sun::star::i18n::UnicodeScript_kHalfwidthFullwidthForm:
                return true;
            default: ; //do nothing
        }
    }
    return false;
}

static sal_Bool lcl_IsSymbolChar( CharClass& rCC, const String& rTxt,
                                  xub_StrLen nStt, xub_StrLen nEnd )
{
    for( ; nStt < nEnd; ++nStt )
    {
        if( ::com::sun::star::i18n::UnicodeType::PRIVATE_USE ==
                rCC.getType( rTxt, nStt ))
            return sal_True;
    }
    return sal_False;
}

static sal_Bool lcl_IsInAsciiArr( const sal_Char* pArr, const sal_Unicode c )
{
    sal_Bool bRet = sal_False;
    for( ; *pArr; ++pArr )
        if( *pArr == c )
        {
            bRet = sal_True;
            break;
        }
    return bRet;
}

SvxAutoCorrDoc::~SvxAutoCorrDoc()
{
}

// Called by the functions:
//  - FnCptlSttWrd
//  - FnCptlSttSntnc
// after the exchange of characters. Then the words, if necessary, can be inserted
// into the exception list.
void SvxAutoCorrDoc::SaveCpltSttWord( sal_uLong, xub_StrLen, const OUString&,
                                        sal_Unicode )
{
}

LanguageType SvxAutoCorrDoc::GetLanguage( xub_StrLen , sal_Bool ) const
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
                ::com::sun::star::i18n::TransliterationModules_IGNORE_KANA |
                ::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH );
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

static void lcl_ClearTable(boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>& rLangTable)
{
    rLangTable.clear();
}

sal_Bool SvxAutoCorrect::IsAutoCorrectChar( sal_Unicode cChar )
{
    return  cChar == '\0' || cChar == '\t' || cChar == 0x0a ||
            cChar == ' '  || cChar == '\'' || cChar == '\"' ||
            cChar == '*'  || cChar == '_'  || cChar == '%' ||
            cChar == '.'  || cChar == ','  || cChar == ';' ||
            cChar == ':'  || cChar == '?' || cChar == '!' ||
            cChar == '/'  || cChar == '-';
}

sal_Bool SvxAutoCorrect::NeedsHardspaceAutocorr( sal_Unicode cChar )
{
    return cChar == '%' || cChar == ';' || cChar == ':'  || cChar == '?' || cChar == '!' ||
        cChar == '/' /*case for the urls exception*/;
}

long SvxAutoCorrect::GetDefaultFlags()
{
    long nRet = Autocorrect
                    | CptlSttSntnc
                    | CptlSttWrd
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


SvxAutoCorrect::SvxAutoCorrect( const String& rShareAutocorrFile,
                                const String& rUserAutocorrFile )
    : sShareAutoCorrFile( rShareAutocorrFile ),
    sUserAutoCorrFile( rUserAutocorrFile ),
    pLangTable( new boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists> ),
    pCharClass( 0 ), bRunNext( false ),
    cStartDQuote( 0 ), cEndDQuote( 0 ), cStartSQuote( 0 ), cEndSQuote( 0 )
{
    nFlags = SvxAutoCorrect::GetDefaultFlags();

    cEmDash = 0x2014;
    cEnDash = 0x2013;
}

SvxAutoCorrect::SvxAutoCorrect( const SvxAutoCorrect& rCpy )
:   sShareAutoCorrFile( rCpy.sShareAutoCorrFile ),
    sUserAutoCorrFile( rCpy.sUserAutoCorrFile ),

    aSwFlags( rCpy.aSwFlags ),

    pLangTable( new boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists> ),
    pCharClass( 0 ), bRunNext( false ),

    nFlags( rCpy.nFlags & ~(ChgWordLstLoad|CplSttLstLoad|WrdSttLstLoad)),
    cStartDQuote( rCpy.cStartDQuote ), cEndDQuote( rCpy.cEndDQuote ),
    cStartSQuote( rCpy.cStartSQuote ), cEndSQuote( rCpy.cEndSQuote ),
    cEmDash( rCpy.cEmDash ), cEnDash( rCpy.cEnDash )
{
}


SvxAutoCorrect::~SvxAutoCorrect()
{
    lcl_ClearTable(*pLangTable);
    delete pLangTable;
    delete pCharClass;
}

void SvxAutoCorrect::_GetCharClass( LanguageType eLang )
{
    delete pCharClass;
    pCharClass = new CharClass( LanguageTag( eLang ));
    eCharClassLang = eLang;
}

void SvxAutoCorrect::SetAutoCorrFlag( long nFlag, sal_Bool bOn )
{
    long nOld = nFlags;
    nFlags = bOn ? nFlags | nFlag
                 : nFlags & ~nFlag;

    if( !bOn )
    {
        if( (nOld & CptlSttSntnc) != (nFlags & CptlSttSntnc) )
            nFlags &= ~CplSttLstLoad;
        if( (nOld & CptlSttWrd) != (nFlags & CptlSttWrd) )
            nFlags &= ~WrdSttLstLoad;
        if( (nOld & Autocorrect) != (nFlags & Autocorrect) )
            nFlags &= ~ChgWordLstLoad;
    }
}


    // Two capital letters at the beginning of word?
sal_Bool SvxAutoCorrect::FnCptlSttWrd( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                    xub_StrLen nSttPos, xub_StrLen nEndPos,
                                    LanguageType eLang )
{
    sal_Bool bRet = sal_False;
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
    xub_StrLen n = 0;
    std::deque<xub_StrLen> aDelimiters;

    // Always check for two capitals at the beginning
    // of the entire word, so start at nSttPos.
    aDelimiters.push_back(nSttPos);

    // Find all compound word delimiters
    for (n = nSttPos; n < nEndPos; n++)
    {
        if (IsAutoCorrectChar(rTxt.GetChar( n )))
        {
            aDelimiters.push_back( n + 1 ); // Get position of char after delimiter
        }

    }

    // Decide where to put the terminating delimiter.
    // If the last AutoCorrect char was a newline, then the AutoCorrect
    // char will not be included in rTxt.
    // If the last AutoCorrect char was not a newline, then the AutoCorrect
    // character will be the last character in rTxt.
    if (!IsAutoCorrectChar(rTxt.GetChar(nEndPos-1)))
        aDelimiters.push_back(nEndPos);

    // Iterate through the word and all words that compose it.
    n = aDelimiters.size();

    // Two capital letters at the beginning of word?
    for(n = 0; n < aDelimiters.size() - 1; n++)
    {
        nSttPos = aDelimiters[n];
        nEndPos = aDelimiters[n + 1];

        if( nSttPos+2 < nEndPos &&
            IsUpperLetter( rCC.getCharacterType( rTxt, nSttPos )) &&
            IsUpperLetter( rCC.getCharacterType( rTxt, ++nSttPos )) &&
            // Is the third character a lower case
            IsLowerLetter( rCC.getCharacterType( rTxt, nSttPos +1 )) &&
            // Do not replace special attributes
            0x1 != rTxt.GetChar( nSttPos ) && 0x2 != rTxt.GetChar( nSttPos ))
        {
            // test if the word is in an exception list
            String sWord( rTxt.Copy( nSttPos - 1, nEndPos - nSttPos + 1 ));
            if( !FindInWrdSttExceptList(eLang, sWord) )
            {
                // Check that word isn't correctly spelled before correcting:
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XSpellChecker1 > xSpeller =
                    SvxGetSpellChecker();
                if( xSpeller->hasLanguage(eLang) )
                {
                    Sequence< ::com::sun::star::beans::PropertyValue > aEmptySeq;
                    if (!xSpeller->spell(sWord, eLang, aEmptySeq).is())
                    {
                        return false;
                    }
                }
                sal_Unicode cSave = rTxt.GetChar( nSttPos );
                OUString sChar( cSave );
                sChar = rCC.lowercase( sChar );
                if( sChar[0] != cSave && rDoc.ReplaceRange( nSttPos, 1, sChar ))
                {
                    if( SaveWordWrdSttLst & nFlags )
                        rDoc.SaveCpltSttWord( CptlSttWrd, nSttPos, sWord, cSave );
                    bRet = sal_True;
                }
            }
        }
    }
    return bRet;
}


sal_Bool SvxAutoCorrect::FnChgOrdinalNumber(
                                SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang )
{
// 1st, 2nd, 3rd, 4 - 0th
// 201th or 201st
// 12th or 12nd
    CharClass& rCC = GetCharClass( eLang );
    sal_Bool bChg = sal_False;

    for( ; nSttPos < nEndPos; ++nSttPos )
        if( !lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nSttPos ) ))
            break;
    for( ; nSttPos < nEndPos; --nEndPos )
        if( !lcl_IsInAsciiArr( sImplEndSkipChars, rTxt.GetChar( nEndPos - 1 ) ))
            break;


    // Get the last number in the string to check
    xub_StrLen nNumEnd = nEndPos;
    bool foundEnd = false;
    bool validNumber = true;
    xub_StrLen i = nEndPos;

    while ( i > nSttPos )
    {
        i--;
        bool isDigit = rCC.isDigit( rTxt, i );
        if ( foundEnd )
            validNumber |= isDigit;

        if ( isDigit && !foundEnd )
        {
            foundEnd = true;
            nNumEnd = i;
        }
    }

    if ( foundEnd && validNumber ) {
        sal_Int32 nNum = rTxt.Copy( nSttPos, nNumEnd - nSttPos + 1 ).ToInt32( );

        // Check if the characters after that number correspond to the ordinal suffix
        uno::Reference< i18n::XOrdinalSuffix > xOrdSuffix
                = i18n::OrdinalSuffix::create( comphelper::getProcessComponentContext() );

        uno::Sequence< OUString > aSuffixes = xOrdSuffix->getOrdinalSuffix( nNum, rCC.getLanguageTag().getLocale( ) );
        for ( sal_Int32 nSuff = 0; nSuff < aSuffixes.getLength(); nSuff++ )
        {
            String sSuffix( aSuffixes[ nSuff ] );
            String sEnd = rTxt.Copy( nNumEnd + 1, nEndPos - nNumEnd - 1 );

            if ( sSuffix == sEnd )
            {
                // Check if the ordinal suffix has to be set as super script
                if ( rCC.isLetter( sSuffix ) )
                {
                    // Do the change
                    SvxEscapementItem aSvxEscapementItem( DFLT_ESC_AUTO_SUPER,
                                                        DFLT_ESC_PROP, SID_ATTR_CHAR_ESCAPEMENT );
                    rDoc.SetAttr( nNumEnd + 1 , nEndPos,
                                    SID_ATTR_CHAR_ESCAPEMENT,
                                    aSvxEscapementItem);
                }
            }
        }
    }
    return bChg;
}


sal_Bool SvxAutoCorrect::FnChgToEnEmDash(
                                SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang )
{
    sal_Bool bRet = sal_False;
    CharClass& rCC = GetCharClass( eLang );
    if (eLang == LANGUAGE_SYSTEM)
        eLang = GetAppLang().getLanguageType();
    bool bAlwaysUseEmDash = (cEmDash && (eLang == LANGUAGE_RUSSIAN || eLang == LANGUAGE_UKRAINIAN));

    // replace " - " or " --" with "enDash"
    if( cEnDash && 1 < nSttPos && 1 <= nEndPos - nSttPos )
    {
        sal_Unicode cCh = rTxt.GetChar( nSttPos );
        if( '-' == cCh )
        {
            if( ' ' == rTxt.GetChar( nSttPos-1 ) &&
                '-' == rTxt.GetChar( nSttPos+1 ))
            {
                xub_StrLen n;
                for( n = nSttPos+2; n < nEndPos && lcl_IsInAsciiArr(
                            sImplSttSkipChars,(cCh = rTxt.GetChar( n )));
                        ++n )
                    ;

                // found: " --[<AnySttChars>][A-z0-9]
                if( rCC.isLetterNumeric( OUString(cCh) ) )
                {
                    for( n = nSttPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt.GetChar( --n ))); )
                        ;

                    // found: "[A-z0-9][<AnyEndChars>] --[<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( OUString(cCh) ))
                    {
                        rDoc.Delete( nSttPos, nSttPos + 2 );
                        rDoc.Insert( nSttPos, bAlwaysUseEmDash ? OUString(cEmDash) : OUString(cEnDash) );
                        bRet = sal_True;
                    }
                }
            }
        }
        else if( 3 < nSttPos &&
                 ' ' == rTxt.GetChar( nSttPos-1 ) &&
                 '-' == rTxt.GetChar( nSttPos-2 ))
        {
            xub_StrLen n, nLen = 1, nTmpPos = nSttPos - 2;
            if( '-' == ( cCh = rTxt.GetChar( nTmpPos-1 )) )
            {
                --nTmpPos;
                ++nLen;
                cCh = rTxt.GetChar( nTmpPos-1 );
            }
            if( ' ' == cCh )
            {
                for( n = nSttPos; n < nEndPos && lcl_IsInAsciiArr(
                            sImplSttSkipChars,(cCh = rTxt.GetChar( n )));
                        ++n )
                    ;

                // found: " - [<AnySttChars>][A-z0-9]
                if( rCC.isLetterNumeric( OUString(cCh) ) )
                {
                    cCh = ' ';
                    for( n = nTmpPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt.GetChar( --n ))); )
                            ;
                    // found: "[A-z0-9][<AnyEndChars>] - [<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( OUString(cCh) ))
                    {
                        rDoc.Delete( nTmpPos, nTmpPos + nLen );
                        rDoc.Insert( nTmpPos, bAlwaysUseEmDash ? OUString(cEmDash) : OUString(cEnDash) );
                        bRet = sal_True;
                    }
                }
            }
        }
    }

    // Replace [A-z0-9]--[A-z0-9] double dash with "emDash" or "enDash".
    // Finnish and Hungarian use enDash instead of emDash.
    bool bEnDash = (eLang == LANGUAGE_HUNGARIAN || eLang == LANGUAGE_FINNISH);
    if( ((cEmDash && !bEnDash) || (cEnDash && bEnDash)) && 4 <= nEndPos - nSttPos )
    {
        String sTmp( rTxt.Copy( nSttPos, nEndPos - nSttPos ) );
        xub_StrLen nFndPos = sTmp.SearchAscii( "--" );
        if( STRING_NOTFOUND != nFndPos && nFndPos &&
            nFndPos + 2 < sTmp.Len() &&
            ( rCC.isLetterNumeric( sTmp, nFndPos - 1 ) ||
              lcl_IsInAsciiArr( sImplEndSkipChars, rTxt.GetChar( nFndPos - 1 ) )) &&
            ( rCC.isLetterNumeric( sTmp, nFndPos + 2 ) ||
            lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nFndPos + 2 ) )))
        {
            nSttPos = nSttPos + nFndPos;
            rDoc.Delete( nSttPos, nSttPos + 2 );
            rDoc.Insert( nSttPos, (bEnDash ? OUString(cEnDash) : OUString(cEmDash)) );
            bRet = sal_True;
        }
    }
    return bRet;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4706) // assignment within conditional expression
#endif

sal_Bool SvxAutoCorrect::FnAddNonBrkSpace(
                                SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen, xub_StrLen nEndPos,
                                LanguageType eLang )
{
    bool bRet = false;

    CharClass& rCC = GetCharClass( eLang );

    if ( rCC.getLanguageTag().getLanguage() == "fr" )
    {
        bool bFrCA = (rCC.getLanguageTag().getCountry() == "CA");
        OUString allChars = OUString( ":;?!%" );
        OUString chars( allChars );
        if ( bFrCA )
            chars = OUString( ":" );

        sal_Unicode cChar = rTxt.GetChar( nEndPos );
        bool bHasSpace = chars.indexOf( cChar ) != -1;
        bool bIsSpecial = allChars.indexOf( cChar ) != -1;
        if ( bIsSpecial )
        {
            // Get the last word delimiter position
            xub_StrLen nSttWdPos = nEndPos;
            bool bWasWordDelim = false;
            while( nSttWdPos && !(bWasWordDelim = IsWordDelim( rTxt.GetChar( --nSttWdPos ))))
                ;

            if(INetURLObject::CompareProtocolScheme(rTxt.Copy(nSttWdPos + (bWasWordDelim ? 1 : 0), nEndPos - nSttWdPos + 1)) != INET_PROT_NOT_VALID) {
                return sal_False;
            }


            // Check the presence of "://" in the word
            xub_StrLen nStrPos = rTxt.Search( OUString( "://" ), nSttWdPos + 1 );
            if ( STRING_NOTFOUND == nStrPos && nEndPos > 0 )
            {
                // Check the previous char
                sal_Unicode cPrevChar = rTxt.GetChar( nEndPos - 1 );
                if ( ( chars.indexOf( cPrevChar ) == -1 ) && cPrevChar != '\t' )
                {
                    // Remove any previous normal space
                    xub_StrLen nPos = nEndPos - 1;
                    while ( cPrevChar == ' ' || cPrevChar == CHAR_HARDBLANK )
                    {
                        if ( nPos == 0 ) break;
                        nPos--;
                        cPrevChar = rTxt.GetChar( nPos );
                    }

                    nPos++;
                    if ( nEndPos - nPos > 0 )
                        rDoc.Delete( nPos, nEndPos );

                    // Add the non-breaking space at the end pos
                    if ( bHasSpace )
                        rDoc.Insert( nPos, OUString(CHAR_HARDBLANK) );
                    bRunNext = true;
                    bRet = true;
                }
                else if ( chars.indexOf( cPrevChar ) != -1 )
                    bRunNext = true;
            }
        }
        else if ( cChar == '/' && nEndPos > 1 && rTxt.Len() > (nEndPos - 1) )
        {
            // Remove the hardspace right before to avoid formatting URLs
            sal_Unicode cPrevChar = rTxt.GetChar( nEndPos - 1 );
            sal_Unicode cMaybeSpaceChar = rTxt.GetChar( nEndPos - 2 );
            if ( cPrevChar == ':' && cMaybeSpaceChar == CHAR_HARDBLANK )
            {
                rDoc.Delete( nEndPos - 2, nEndPos - 1 );
                bRet = true;
            }
        }
    }

    return bRet;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

sal_Bool SvxAutoCorrect::FnSetINetAttr( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                    xub_StrLen nSttPos, xub_StrLen nEndPos,
                                    LanguageType eLang )
{
    sal_Int32 nStart(nSttPos);
    sal_Int32 nEnd(nEndPos);

    String sURL( URIHelper::FindFirstURLInText( rTxt, nStart, nEnd,
                                                GetCharClass( eLang ) ));
    nSttPos = (xub_StrLen)nStart;
    nEndPos = (xub_StrLen)nEnd;
    sal_Bool bRet = 0 != sURL.Len();
    if( bRet )          // also Attribut setzen:
        rDoc.SetINetAttr( nSttPos, nEndPos, sURL );
    return bRet;
}


sal_Bool SvxAutoCorrect::FnChgWeightUnderl( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                        xub_StrLen, xub_StrLen nEndPos,
                                        LanguageType eLang )
{
    // Condition:
    //  at the beginning:   _ or * after Space with the folloeing !Space
    //  at the end:         _ or * before Space (word delimiter?)

    sal_Unicode c, cInsChar = rTxt.GetChar( nEndPos );  // underline or bold
    if( ++nEndPos != rTxt.Len() &&
        !IsWordDelim( rTxt.GetChar( nEndPos ) ) )
        return sal_False;

    --nEndPos;

    sal_Bool bAlphaNum = sal_False;
    xub_StrLen nPos = nEndPos, nFndPos = STRING_NOTFOUND;
    CharClass& rCC = GetCharClass( eLang );

    while( nPos )
    {
        switch( c = rTxt.GetChar( --nPos ) )
        {
        case '_':
        case '*':
            if( c == cInsChar )
            {
                if( bAlphaNum && nPos+1 < nEndPos && ( !nPos ||
                    IsWordDelim( rTxt.GetChar( nPos-1 ))) &&
                    !IsWordDelim( rTxt.GetChar( nPos+1 )))
                        nFndPos = nPos;
                else
                    // Condition is not satisfied, so cancel
                    nFndPos = STRING_NOTFOUND;
                nPos = 0;
            }
            break;
        default:
            if( !bAlphaNum )
                bAlphaNum = rCC.isLetterNumeric( rTxt, nPos );
        }
    }

    if( STRING_NOTFOUND != nFndPos )
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

    return STRING_NOTFOUND != nFndPos;
}


sal_Bool SvxAutoCorrect::FnCptlSttSntnc( SvxAutoCorrDoc& rDoc,
                                    const String& rTxt, sal_Bool bNormalPos,
                                    xub_StrLen nSttPos, xub_StrLen nEndPos,
                                    LanguageType eLang )
{

    if( !rTxt.Len() || nEndPos <= nSttPos )
        return sal_False;

    CharClass& rCC = GetCharClass( eLang );
    String aText( rTxt );
    const sal_Unicode *pStart = aText.GetBuffer(),
                      *pStr = pStart + nEndPos,
                      *pWordStt = 0,
                      *pDelim = 0;

    sal_Bool bAtStart = sal_False;
    do {
        --pStr;
        if( rCC.isLetter(
                aText, sal::static_int_cast< xub_StrLen >( pStr - pStart ) ) )
        {
            if( !pWordStt )
                pDelim = pStr+1;
            pWordStt = pStr;
        }
        else if( pWordStt &&
                 !rCC.isDigit(
                     aText,
                     sal::static_int_cast< xub_StrLen >( pStr - pStart ) ) )
        {
            if( lcl_IsInAsciiArr( sImplWordChars, *pStr ) &&
                pWordStt - 1 == pStr &&
                // Installation at beginning of paragraph. Replaced < by <= (#i38971#)
                (sal_IntPtr)(pStart + 1) <= (sal_IntPtr)pStr &&
                rCC.isLetter(
                    aText,
                    sal::static_int_cast< xub_StrLen >( pStr-1 - pStart ) ) )
                pWordStt = --pStr;
            else
                break;
        }
    } while( 0 == ( bAtStart = (pStart == pStr)) );

    if( !pWordStt ||
        rCC.isDigit(
            aText, sal::static_int_cast< xub_StrLen >( pStr - pStart ) ) ||
        IsUpperLetter(
            rCC.getCharacterType(
                aText,
                sal::static_int_cast< xub_StrLen >( pWordStt - pStart ) ) ) ||
        INetURLObject::CompareProtocolScheme(rTxt.Copy(pWordStt - pStart, pDelim - pWordStt + 1)) != INET_PROT_NOT_VALID ||
        0x1 == *pWordStt || 0x2 == *pWordStt )
        return sal_False;       // no character to be replaced, or already ok

    if( *pDelim && 2 >= pDelim - pWordStt &&
        lcl_IsInAsciiArr( ".-)>", *pDelim ) )
        return sal_False;

    if( !bAtStart ) // Still no beginning of a paragraph?
    {
        if ( IsWordDelim( *pStr ) )
        {
            while( 0 == ( bAtStart = (pStart == pStr--) ) && IsWordDelim( *pStr ))
                ;
        }
        // Asian full stop, full width full stop, full width exclamation mark
        // and full width question marks are treated as word delimiters
        else if ( 0x3002 != *pStr && 0xFF0E != *pStr && 0xFF01 != *pStr &&
                  0xFF1F != *pStr )
            return sal_False; // no valid separator -> no replacement
    }

    if( bAtStart )  // at the beginning of a paragraph?
    {
        // Check out the previous paragraph, if it exists.
        // If so, then check to paragraph separator at the end.
        OUString aPrevPara = rDoc.GetPrevPara( bNormalPos );
        if( !aPrevPara.isEmpty() )
        {
            // valid separator -> replace
            OUString sChar( *pWordStt );
            sChar = rCC.titlecase(sChar); //see fdo#56740
            return  !comphelper::string::equals(sChar, *pWordStt) &&
                    rDoc.ReplaceRange( xub_StrLen( pWordStt - pStart ), 1, sChar );
        }

        aText = aPrevPara;
        bAtStart = sal_False;
        pStart = aText.GetBuffer();
        pStr = pStart + aText.Len();

        do {            // overwrite all blanks
            --pStr;
            if( !IsWordDelim( *pStr ))
                break;
        } while( 0 == ( bAtStart = (pStart == pStr)) );

        if( bAtStart )
            return sal_False;  // no valid separator -> no replacement
    }

    // Found [ \t]+[A-Z0-9]+ until here. Test now on the paragraph separator.
    // all three can happen, but not more than once!
    const sal_Unicode* pExceptStt = 0;
    if( !bAtStart )
    {
        sal_Bool bWeiter = sal_True;
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
                        return sal_False;
                    }
                    if( nFlag & C_FULL_STOP )
                        return sal_False;  // no valid separator -> no replacement
                    nFlag |= C_FULL_STOP;
                    pExceptStt = pStr;
                }
                break;
            case '!':
            case 0xFF01 :
                {
                    if( nFlag & C_EXCLAMATION_MARK )
                        return sal_False;   // no valid separator -> no replacement
                    nFlag |= C_EXCLAMATION_MARK;
                }
                break;
            case '?':
            case 0xFF1F :
                {
                    if( nFlag & C_QUESTION_MARK)
                        return sal_False;   // no valid separator -> no replacement
                    nFlag |= C_QUESTION_MARK;
                }
                break;
            default:
                if( !nFlag )
                    return sal_False;       // no valid separator -> no replacement
                else
                    bWeiter = sal_False;
                break;
            }

            if( bWeiter && pStr-- == pStart )
            {
                return sal_False;       // no valid separator -> no replacement
            }
        } while( bWeiter );
        if( C_FULL_STOP != nFlag )
            pExceptStt = 0;
    }

    if( 2 > ( pStr - pStart ) )
        return sal_False;

    if( !rCC.isLetterNumeric(
            aText, sal::static_int_cast< xub_StrLen >( pStr-- - pStart ) ) )
    {
        sal_Bool bValid = sal_False, bAlphaFnd = sal_False;
        const sal_Unicode* pTmpStr = pStr;
        while( !bValid )
        {
            if( rCC.isDigit(
                    aText,
                    sal::static_int_cast< xub_StrLen >( pTmpStr - pStart ) ) )
            {
                bValid = sal_True;
                pStr = pTmpStr - 1;
            }
            else if( rCC.isLetter(
                         aText,
                         sal::static_int_cast< xub_StrLen >(
                             pTmpStr - pStart ) ) )
            {
                if( bAlphaFnd )
                {
                    bValid = sal_True;
                    pStr = pTmpStr;
                }
                else
                    bAlphaFnd = sal_True;
            }
            else if( bAlphaFnd || IsWordDelim( *pTmpStr ) )
                break;

            if( pTmpStr == pStart )
                break;

            --pTmpStr;
        }

        if( !bValid )
            return sal_False;       // no valid separator -> no replacement
    }

    sal_Bool bNumericOnly = '0' <= *(pStr+1) && *(pStr+1) <= '9';

    // Search for the beginning of the word
    while( !IsWordDelim( *pStr ))
    {
        if( bNumericOnly &&
            rCC.isLetter(
                aText, sal::static_int_cast< xub_StrLen >( pStr - pStart ) ) )
            bNumericOnly = sal_False;

        if( pStart == pStr )
            break;

        --pStr;
    }

    if( bNumericOnly )      // consists of only numbers, then not
        return sal_False;

    if( IsWordDelim( *pStr ))
        ++pStr;

    String sWord;

    // check on the basis of the exception list
    if( pExceptStt )
    {
        sWord = OUString(pStr, pExceptStt - pStr + 1);
        if( FindInCplSttExceptList(eLang, sWord) )
            return sal_False;

        // Delete all non alphanumeric. Test the characters at the
        // beginning/end of the word ( recognizes: "(min.", "/min.", and so on.)
        String sTmp( sWord );
        while( sTmp.Len() &&
                !rCC.isLetterNumeric( sTmp, 0 ) )
            sTmp.Erase( 0, 1 );

        // Remove all non alphanumeric characters towards the end up until
        // the last one.
        xub_StrLen nLen = sTmp.Len();
        while( nLen && !rCC.isLetterNumeric( sTmp, nLen-1 ) )
            --nLen;
        if( nLen + 1 < sTmp.Len() )
            sTmp.Erase( nLen + 1 );

        if( sTmp.Len() && sTmp.Len() != sWord.Len() &&
            FindInCplSttExceptList(eLang, sTmp))
            return sal_False;

        if(FindInCplSttExceptList(eLang, sWord, sal_True))
            return sal_False;
    }

    // Ok, then replace
    sal_Unicode cSave = *pWordStt;
    nSttPos = sal::static_int_cast< xub_StrLen >( pWordStt - rTxt.GetBuffer() );
    OUString sChar( cSave );
    sChar = rCC.titlecase(sChar); //see fdo#56740
    sal_Bool bRet = sChar[0] != cSave && rDoc.ReplaceRange( nSttPos, 1, sChar );

    // Parahaps someone wants to have the word
    if( bRet && SaveWordCplSttLst & nFlags )
        rDoc.SaveCpltSttWord( CptlSttSntnc, nSttPos, sWord, cSave );

    return bRet;
}

bool SvxAutoCorrect::FnCorrectCapsLock( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                        xub_StrLen nSttPos, xub_StrLen nEndPos,
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

    String aConverted;
    aConverted.Append( rCC.uppercase(OUString(rTxt.GetChar(nSttPos))) );
    aConverted.Append( rCC.lowercase(OUString(rTxt.GetChar(nSttPos+1))) );

    for (xub_StrLen i = nSttPos+2; i < nEndPos; ++i)
    {
        if ( IsLowerLetter(rCC.getCharacterType(rTxt, i)) )
            // A lowercase letter disqualifies the whole text.
            return false;

        if ( IsUpperLetter(rCC.getCharacterType(rTxt, i)) )
            // Another uppercase letter.  Convert it.
            aConverted.Append(rCC.lowercase(OUString(rTxt.GetChar(i))));
        else
            // This is not an alphabetic letter.  Leave it as-is.
            aConverted.Append(rTxt.GetChar(i));
    }

    // Replace the word.
    rDoc.Delete(nSttPos, nEndPos);
    rDoc.Insert(nSttPos, aConverted);

    return true;
}


sal_Unicode SvxAutoCorrect::GetQuote( sal_Unicode cInsChar, sal_Bool bSttQuote,
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
            String sRet( bSttQuote
                            ? ( '\"' == cInsChar
                                ? rLcl.getDoubleQuotationMarkStart()
                                : rLcl.getQuotationMarkStart() )
                            : ( '\"' == cInsChar
                                ? rLcl.getDoubleQuotationMarkEnd()
                                : rLcl.getQuotationMarkEnd() ));
            cRet = sRet.Len() ? sRet.GetChar( 0 ) : cInsChar;
        }
    }
    return cRet;
}

void SvxAutoCorrect::InsertQuote( SvxAutoCorrDoc& rDoc, xub_StrLen nInsPos,
                                    sal_Unicode cInsChar, sal_Bool bSttQuote,
                                    sal_Bool bIns )
{
    LanguageType eLang = rDoc.GetLanguage( nInsPos, sal_False );
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
                OUString s( static_cast< sal_Unicode >(0xA0) );
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

String SvxAutoCorrect::GetQuote( SvxAutoCorrDoc& rDoc, xub_StrLen nInsPos,
                                sal_Unicode cInsChar, sal_Bool bSttQuote )
{
    LanguageType eLang = rDoc.GetLanguage( nInsPos, sal_False );
    sal_Unicode cRet = GetQuote( cInsChar, bSttQuote, eLang );

    String sRet = OUString(cRet);

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
                sRet += ' ';
            else
                sRet.Insert( ' ', 0 );
            break;
        }
    }
    return sRet;
}

sal_uLong
SvxAutoCorrect::DoAutoCorrect( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                    xub_StrLen nInsPos, sal_Unicode cChar,
                                    sal_Bool bInsert, Window* pFrameWin )
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
                ' ' == rTxt.GetChar( nInsPos - 1 ) )
            {
                nRet = IgnoreDoubleSpace;
                break;
            }

            sal_Bool bSingle = '\'' == cChar;
            sal_Bool bIsReplaceQuote =
                        (IsAutoCorrFlag( ChgQuotes ) && ('\"' == cChar )) ||
                        (IsAutoCorrFlag( ChgSglQuotes ) && bSingle );
            if( bIsReplaceQuote )
            {
                sal_Unicode cPrev;
                sal_Bool bSttQuote = !nInsPos ||
                        IsWordDelim( ( cPrev = rTxt.GetChar( nInsPos-1 ))) ||
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
                    FnAddNonBrkSpace( rDoc, rTxt, 0, nInsPos, rDoc.GetLanguage( nInsPos, sal_False ) ) )
                {
                    nRet = AddNonBrkSpace;
                }
                else if ( bIsNextRun && !IsAutoCorrectChar( cChar ) )
                {
                    // Remove the NBSP if it wasn't an autocorrection
                    if ( nInsPos != 0 && NeedsHardspaceAutocorr( rTxt.GetChar( nInsPos - 1 ) ) &&
                            cChar != ' ' && cChar != '\t' && cChar != CHAR_HARDBLANK )
                    {
                        // Look for the last HARD_SPACE
                        xub_StrLen nPos = nInsPos - 1;
                        bool bContinue = true;
                        while ( bContinue )
                        {
                            const sal_Unicode cTmpChar = rTxt.GetChar( nPos );
                            if ( cTmpChar == CHAR_HARDBLANK )
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

        xub_StrLen nPos = nInsPos - 1;

        if( IsWordDelim( rTxt.GetChar( nPos )))
            break;

        // Set bold or underline automatically?
        if( '*' == cChar || '_' == cChar )
        {
            if( IsAutoCorrFlag( ChgWeightUnderl ) &&
                FnChgWeightUnderl( rDoc, rTxt, 0, nPos+1 ) )
                nRet = ChgWeightUnderl;
            break;
        }

        while( nPos && !IsWordDelim( rTxt.GetChar( --nPos )))
            ;

        // Found a Paragraph-start or a Blank, search for the word shortcut in
        // auto.
        xub_StrLen nCapLttrPos = nPos+1;        // on the 1st Character
        if( !nPos && !IsWordDelim( rTxt.GetChar( 0 )))
            --nCapLttrPos;          // Absatz Anfang und kein Blank !

        LanguageType eLang = rDoc.GetLanguage( nCapLttrPos, sal_False );
        if( LANGUAGE_SYSTEM == eLang )
            eLang = MsLangId::getSystemLanguage();
        CharClass& rCC = GetCharClass( eLang );

        // no symbol characters
        if( lcl_IsSymbolChar( rCC, rTxt, nCapLttrPos, nInsPos ))
            break;

        if( IsAutoCorrFlag( Autocorrect ) )
        {
            OUString aPara;
            OUString* pPara = IsAutoCorrFlag(CptlSttSntnc) ? &aPara : 0;

            sal_Bool bChgWord = rDoc.ChgAutoCorrWord( nCapLttrPos, nInsPos,
                                                    *this, pPara );
            if( !bChgWord )
            {
                xub_StrLen nCapLttrPos1 = nCapLttrPos, nInsPos1 = nInsPos;
                while( nCapLttrPos1 < nInsPos &&
                        lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nCapLttrPos1 ) )
                        )
                        ++nCapLttrPos1;
                while( nCapLttrPos1 < nInsPos1 && nInsPos1 &&
                        lcl_IsInAsciiArr( sImplEndSkipChars, rTxt.GetChar( nInsPos1-1 ) )
                        )
                        --nInsPos1;

                if( (nCapLttrPos1 != nCapLttrPos || nInsPos1 != nInsPos ) &&
                    nCapLttrPos1 < nInsPos1 &&
                    rDoc.ChgAutoCorrWord( nCapLttrPos1, nInsPos1, *this, pPara ))
                {
                    bChgWord = sal_True;
                    nCapLttrPos = nCapLttrPos1;
                }
            }

            if( bChgWord )
            {
                nRet = Autocorrect;
                if( !aPara.isEmpty() )
                {
                    xub_StrLen nEnd = nCapLttrPos;
                    while( nEnd < aPara.getLength() &&
                            !IsWordDelim( aPara[ nEnd ]))
                        ++nEnd;

                    // Capital letter at beginning of paragraph?
                    if( IsAutoCorrFlag( CptlSttSntnc ) &&
                        FnCptlSttSntnc( rDoc, aPara, sal_False,
                                                nCapLttrPos, nEnd, eLang ) )
                        nRet |= CptlSttSntnc;

                    if( IsAutoCorrFlag( ChgToEnEmDash ) &&
                        FnChgToEnEmDash( rDoc, rTxt, nCapLttrPos, nEnd, eLang ) )
                        nRet |= ChgToEnEmDash;
                }
                break;
            }
        }

        if( ( IsAutoCorrFlag( nRet = ChgOrdinalNumber ) &&
                FnChgOrdinalNumber( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) ||
            ( IsAutoCorrFlag( nRet = SetINetAttr ) &&
                ( ' ' == cChar || '\t' == cChar || 0x0a == cChar || !cChar ) &&
                FnSetINetAttr( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) )
            ;
        else
        {
            bool bLockKeyOn = pFrameWin && (pFrameWin->GetIndicatorState() & INDICATOR_CAPSLOCK);
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
                IsAutoCorrFlag( CptlSttSntnc ) &&
                FnCptlSttSntnc( rDoc, rTxt, sal_True, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CptlSttSntnc;

            // Two capital letters at beginning of word ??
            if( !bUnsupported &&
                IsAutoCorrFlag( CptlSttWrd ) &&
                FnCptlSttWrd( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CptlSttWrd;

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
    if(pLangTable->find(eLang) == pLangTable->end())
        CreateLanguageFile(eLang, sal_True);
    return *(pLangTable->find(eLang)->second);
}

void SvxAutoCorrect::SaveCplSttExceptList( LanguageType eLang )
{
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end() && nTmpVal->second)
        nTmpVal->second->SaveCplSttExceptList();
#ifdef DBG_UTIL
    else
    {
        SAL_WARN("editeng", "Save an empty list? ");
    }
#endif
}

void SvxAutoCorrect::SaveWrdSttExceptList(LanguageType eLang)
{
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end() && nTmpVal->second)
        nTmpVal->second->SaveWrdSttExceptList();
#ifdef DBG_UTIL
    else
    {
        SAL_WARN("editeng", "Save an empty list? ");
    }
#endif
}

    // Adds a single word. The list will immediately be written to the file!
sal_Bool SvxAutoCorrect::AddCplSttException( const String& rNew,
                                        LanguageType eLang )
{
    SvxAutoCorrectLanguageLists* pLists = 0;
    // either the right language is present or it will be this in the general list
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end())
        pLists = nTmpVal->second;
    else
    {
        nTmpVal = pLangTable->find(LANGUAGE_UNDETERMINED);
        if(nTmpVal != pLangTable->end())
            pLists = nTmpVal->second;
        else if(CreateLanguageFile(LANGUAGE_UNDETERMINED, sal_True))
            pLists = pLangTable->find(LANGUAGE_UNDETERMINED)->second;
    }
    OSL_ENSURE(pLists, "No auto correction data");
    return pLists->AddToCplSttExceptList(rNew);
}

// Adds a single word. The list will immediately be written to the file!
sal_Bool SvxAutoCorrect::AddWrtSttException( const String& rNew,
                                         LanguageType eLang )
{
    SvxAutoCorrectLanguageLists* pLists = 0;
    //either the right language is present or it is set in the general list
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end())
        pLists = nTmpVal->second;
    else
    {
        nTmpVal = pLangTable->find(LANGUAGE_UNDETERMINED);
        if(nTmpVal != pLangTable->end())
            pLists = nTmpVal->second;
        else if(CreateLanguageFile(LANGUAGE_UNDETERMINED, sal_True))
            pLists = pLangTable->find(LANGUAGE_UNDETERMINED)->second;
    }
    OSL_ENSURE(pLists, "No auto correction file!");
    return pLists->AddToWrdSttExceptList(rNew);
}

sal_Bool SvxAutoCorrect::GetPrevAutoCorrWord( SvxAutoCorrDoc& rDoc,
                                        const String& rTxt, xub_StrLen nPos,
                                        String& rWord ) const
{
    if( !nPos )
        return sal_False;

    xub_StrLen nEnde = nPos;

    // it must be followed by a blank or tab!
    if( ( nPos < rTxt.Len() &&
        !IsWordDelim( rTxt.GetChar( nPos ))) ||
        IsWordDelim( rTxt.GetChar( --nPos )))
        return sal_False;

    while( nPos && !IsWordDelim( rTxt.GetChar( --nPos )))
        ;

    // Found a Paragraph-start or a Blank, search for the word shortcut in
    // auto.
    xub_StrLen nCapLttrPos = nPos+1;        // on the 1st Character
    if( !nPos && !IsWordDelim( rTxt.GetChar( 0 )))
        --nCapLttrPos;          // Beginning of pargraph and no Blank!

    while( lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nCapLttrPos )) )
        if( ++nCapLttrPos >= nEnde )
            return sal_False;

    if( 3 > nEnde - nCapLttrPos )
        return sal_False;

    LanguageType eLang = rDoc.GetLanguage( nCapLttrPos, sal_False );
    if( LANGUAGE_SYSTEM == eLang )
        eLang = MsLangId::getSystemLanguage();

    SvxAutoCorrect* pThis = (SvxAutoCorrect*)this;
    CharClass& rCC = pThis->GetCharClass( eLang );

    if( lcl_IsSymbolChar( rCC, rTxt, nCapLttrPos, nEnde ))
        return sal_False;

    rWord = rTxt.Copy( nCapLttrPos, nEnde - nCapLttrPos );
    return sal_True;
}

sal_Bool SvxAutoCorrect::CreateLanguageFile( LanguageType eLang, sal_Bool bNewFile )
{
    OSL_ENSURE(pLangTable->find(eLang) == pLangTable->end(), "Language already exists ");

    OUString sUserDirFile( GetAutoCorrFileName( eLang, sal_True, sal_False ));
    OUString sShareDirFile( sUserDirFile );

    SvxAutoCorrectLanguageListsPtr pLists = 0;

    Time nMinTime( 0, 2 ), nAktTime( Time::SYSTEM ), nLastCheckTime( Time::EMPTY );

    std::map<LanguageType, long>::iterator nFndPos = aLastFileTable.find(eLang);
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
            pLangTable->insert(eLang, pLists);
            aLastFileTable.erase(nFndPos);
        }
    }
    else if( ( FStatHelper::IsDocument( sUserDirFile ) ||
                FStatHelper::IsDocument( sShareDirFile =
                              GetAutoCorrFileName( eLang, sal_False, sal_False ) ) ) ||
        ( sShareDirFile = sUserDirFile, bNewFile ))
    {
        pLists = new SvxAutoCorrectLanguageLists( *this, sShareDirFile, sUserDirFile );
        pLangTable->insert(eLang, pLists);
        if (nFndPos != aLastFileTable.end())
            aLastFileTable.erase(nFndPos);
    }
    else if( !bNewFile )
    {
        aLastFileTable[eLang] = nAktTime.GetTime();
    }
    return pLists != 0;
}

sal_Bool SvxAutoCorrect::PutText( const String& rShort, const String& rLong,
                                LanguageType eLang )
{
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end())
        return nTmpVal->second->PutText(rShort, rLong);
    if(CreateLanguageFile(eLang))
        return pLangTable->find(eLang)->second->PutText(rShort, rLong);
    return sal_False;
}

sal_Bool SvxAutoCorrect::MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries,
                                              std::vector<SvxAutocorrWord>& aDeleteEntries,
                                              LanguageType eLang )
{
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end())
    {
        return nTmpVal->second->MakeCombinedChanges( aNewEntries, aDeleteEntries );
    }
    else if(CreateLanguageFile( eLang ))
    {
        return pLangTable->find( eLang )->second->MakeCombinedChanges( aNewEntries, aDeleteEntries );
    }
    return sal_False;

}


    //  - return the replacement text (only for SWG-Format, all other
    //      can be taken from the word list!)
sal_Bool SvxAutoCorrect::GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                      const String&, const String&, OUString& )
{
    return sal_False;
}

    // Text with attribution (only the SWG - SWG format!)
sal_Bool SvxAutoCorrect::PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                  const String&, const String&, SfxObjectShell&, OUString& )
{
    return sal_False;
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
static void GeneratePackageName ( const String& rShort, String& rPackageName )
{
    rPackageName = rShort;
    xub_StrLen nPos = 0;
    sal_Unicode pDelims[] = { '!', '/', ':', '.', '\\', 0 };
    OString sByte(OUStringToOString(rPackageName, RTL_TEXTENCODING_UTF7));
    rPackageName = OStringToOUString(sByte, RTL_TEXTENCODING_ASCII_US);
    while( STRING_NOTFOUND != ( nPos = rPackageName.SearchChar( pDelims, nPos )))
    {
        rPackageName.SetChar( nPos, '_' );
        ++nPos;
    }
}

static const SvxAutocorrWord* lcl_SearchWordsInList(
                SvxAutoCorrectLanguageListsPtr pList, const String& rTxt,
                xub_StrLen& rStt, xub_StrLen nEndPos)
{
    const SvxAutocorrWordList* pAutoCorrWordList = pList->GetAutocorrWordList();
    return pAutoCorrWordList->SearchWordsInList( rTxt, rStt, nEndPos );
}

// the search for the words in the substitution table
const SvxAutocorrWord* SvxAutoCorrect::SearchWordsInList(
                const String& rTxt, xub_StrLen& rStt, xub_StrLen nEndPos,
                SvxAutoCorrDoc&, LanguageType& rLang )
{
    LanguageType eLang = rLang;
    const SvxAutocorrWord* pRet = 0;
    if( LANGUAGE_SYSTEM == eLang )
        eLang = MsLangId::getSystemLanguage();

    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_UNDETERMINED
    if(pLangTable->find(eLang) != pLangTable->end() || CreateLanguageFile(eLang, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(eLang)->second;
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = eLang;
            return pRet;
        }
    }

    // If it still could not be found here, then keep on searching

    LanguageType nTmpKey1 = eLang & 0x7ff, // the main language in many cases DE
                 nTmpKey2 = eLang & 0x3ff; // otherwise for example EN
    if(nTmpKey1 != eLang && (pLangTable->find(nTmpKey1) != pLangTable->end() || CreateLanguageFile(nTmpKey1, sal_False)))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(nTmpKey1)->second;
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = nTmpKey1;
            return pRet;
        }
    }

    if(nTmpKey2 != eLang && (pLangTable->find(nTmpKey2) != pLangTable->end() || CreateLanguageFile(nTmpKey2, sal_False)))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(nTmpKey2)->second;
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = nTmpKey2;
            return pRet;
        }
    }

    if(pLangTable->find(LANGUAGE_UNDETERMINED) != pLangTable->end() || CreateLanguageFile(LANGUAGE_UNDETERMINED, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(LANGUAGE_UNDETERMINED)->second;
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos );
        if( pRet )
        {
            rLang = LANGUAGE_UNDETERMINED;
            return pRet;
        }
    }
    return 0;
}

sal_Bool SvxAutoCorrect::FindInWrdSttExceptList( LanguageType eLang,
                                             const String& sWord )
{
    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_UNDETERMINED
    LanguageType nTmpKey1 = eLang & 0x7ff, // the main language in many cases DE
                 nTmpKey2 = eLang & 0x3ff; // otherwise for example EN
    OUString sTemp(sWord);

    if(pLangTable->find(eLang) != pLangTable->end() || CreateLanguageFile(eLang, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(eLang)->second;
        OUString _sTemp(sWord);
        if(pList->GetWrdSttExceptList()->find(_sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }

    // If it still could not be found here, then keep on searching
    if(nTmpKey1 != eLang && (pLangTable->find(nTmpKey1) != pLangTable->end() || CreateLanguageFile(nTmpKey1, sal_False)))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(nTmpKey1)->second;
        if(pList->GetWrdSttExceptList()->find(sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }

    if(nTmpKey2 != eLang && (pLangTable->find(nTmpKey2) != pLangTable->end() || CreateLanguageFile(nTmpKey2, sal_False)))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(nTmpKey2)->second;
        if(pList->GetWrdSttExceptList()->find(sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }

    if(pLangTable->find(LANGUAGE_UNDETERMINED) != pLangTable->end() || CreateLanguageFile(LANGUAGE_UNDETERMINED, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(LANGUAGE_UNDETERMINED)->second;
        if(pList->GetWrdSttExceptList()->find(sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }
    return sal_False;
}

static sal_Bool lcl_FindAbbreviation( const SvStringsISortDtor* pList, const String& sWord)
{
    OUString sAbk('~');
    SvStringsISortDtor::const_iterator it = pList->find( sAbk );
    sal_uInt16 nPos = it - pList->begin();
    if( nPos < pList->size() )
    {
        String sLowerWord( sWord ); sLowerWord.ToLowerAscii();
        OUString pAbk;
        for( sal_uInt16 n = nPos;
                n < pList->size() &&
                '~' == ( pAbk = (*pList)[ n ])[ 0 ];
            ++n )
        {
            // ~ and ~. are not allowed!
            if( 2 < pAbk.getLength() && pAbk.getLength() - 1 <= sWord.Len() )
            {
                String sLowerAbk( pAbk ); sLowerAbk.ToLowerAscii();
                for( xub_StrLen i = sLowerAbk.Len(), ii = sLowerWord.Len(); i; )
                {
                    if( !--i )      // agrees
                        return sal_True;

                    if( sLowerAbk.GetChar( i ) != sLowerWord.GetChar( --ii ))
                        break;
                }
            }
        }
    }
    OSL_ENSURE( !(nPos && '~' == (*pList)[ --nPos ][ 0 ] ),
            "Wrongly sorted exception list?" );
    return sal_False;
}

sal_Bool SvxAutoCorrect::FindInCplSttExceptList(LanguageType eLang,
                                const String& sWord, sal_Bool bAbbreviation)
{
    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_UNDETERMINED
    LanguageType nTmpKey1 = eLang & 0x7ff, // the main language in many cases DE
                 nTmpKey2 = eLang & 0x3ff; // otherwise for example EN
    OUString sTemp( sWord );

    if(pLangTable->find(eLang) != pLangTable->end() || CreateLanguageFile(eLang, sal_False))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = pLangTable->find(eLang)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return sal_True;
    }

    // If it still could not be found here, then keep on searching
    if(nTmpKey1 != eLang && (pLangTable->find(nTmpKey1) != pLangTable->end() || CreateLanguageFile(nTmpKey1, sal_False)))
    {
        const SvStringsISortDtor* pList = pLangTable->find(nTmpKey1)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return sal_True;
    }

    if(nTmpKey2 != eLang && (pLangTable->find(nTmpKey2) != pLangTable->end() || CreateLanguageFile(nTmpKey2, sal_False)))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = pLangTable->find(nTmpKey2)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return sal_True;
    }

    if(pLangTable->find(LANGUAGE_UNDETERMINED) != pLangTable->end() || CreateLanguageFile(LANGUAGE_UNDETERMINED, sal_False))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = pLangTable->find(LANGUAGE_UNDETERMINED)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(sTemp) != pList->end() )
            return sal_True;
    }
    return sal_False;
}

OUString SvxAutoCorrect::GetAutoCorrFileName( LanguageType eLang,
                                            sal_Bool bNewFile, sal_Bool bTst ) const
{
    OUString sRet, sExt( LanguageTag::convertToBcp47( eLang ) );

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
                const String& rShareAutoCorrectFile,
                const String& rUserAutoCorrectFile)
:   sShareAutoCorrFile( rShareAutoCorrectFile ),
    sUserAutoCorrFile( rUserAutoCorrectFile ),
    aModifiedDate( Date::EMPTY ),
    aModifiedTime( Time::EMPTY ),
    aLastCheckTime( Time::EMPTY ),
    pCplStt_ExcptLst( 0 ),
    pWrdStt_ExcptLst( 0 ),
    pAutocorr_List( 0 ),
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

sal_Bool SvxAutoCorrectLanguageLists::IsFileChanged_Imp()
{
    // Access the file system only every 2 minutes to check the date stamp
    sal_Bool bRet = sal_False;

    Time nMinTime( 0, 2 );
    Time nAktTime( Time::SYSTEM );
    if( aLastCheckTime > nAktTime ||                    // overflow?
        ( nAktTime -= aLastCheckTime ) > nMinTime )     // min time past
    {
        Date aTstDate( Date::EMPTY ); Time aTstTime( Time::EMPTY );
        if( FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                            &aTstDate, &aTstTime ) &&
            ( aModifiedDate != aTstDate || aModifiedTime != aTstTime ))
        {
            bRet = sal_True;
            // then remove all the lists fast!
            if( CplSttLstLoad & nFlags && pCplStt_ExcptLst )
                delete pCplStt_ExcptLst, pCplStt_ExcptLst = 0;
            if( WrdSttLstLoad & nFlags && pWrdStt_ExcptLst )
                delete pWrdStt_ExcptLst, pWrdStt_ExcptLst = 0;
            if( ChgWordLstLoad & nFlags && pAutocorr_List )
                delete pAutocorr_List, pAutocorr_List = 0;
            nFlags &= ~(CplSttLstLoad | WrdSttLstLoad | ChgWordLstLoad );
        }
        aLastCheckTime = Time( Time::SYSTEM );
    }
    return bRet;
}

void SvxAutoCorrectLanguageLists::LoadXMLExceptList_Imp(
                                        SvStringsISortDtor*& rpLst,
                                        const sal_Char* pStrmName,
                                        SotStorageRef& rStg)
{
    if( rpLst )
        rpLst->clear();
    else
        rpLst = new SvStringsISortDtor;

    {
        String sStrmName( pStrmName, RTL_TEXTENCODING_MS_1252 );
        String sTmp( sStrmName );

        if( rStg.Is() && rStg->IsStream( sStrmName ) )
        {
            SvStorageStreamRef xStrm = rStg->OpenSotStream( sTmp,
                ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE ) );
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
                uno::Reference< xml::sax::XDocumentHandler > xFilter = new SvXMLExceptionListImport ( xContext, *rpLst );

                // connect parser and filter
                uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );
                xParser->setDocumentHandler( xFilter );

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
        aLastCheckTime = Time( Time::SYSTEM );
    }

}

void SvxAutoCorrectLanguageLists::SaveExceptList_Imp(
                            const SvStringsISortDtor& rLst,
                            const sal_Char* pStrmName,
                            SotStorageRef &rStg,
                            sal_Bool bConvert )
{
    if( rStg.Is() )
    {
        String sStrmName( pStrmName, RTL_TEXTENCODING_MS_1252 );
        if( rLst.empty() )
        {
            rStg->Remove( sStrmName );
            rStg->Commit();
        }
        else
        {
            SotStorageStreamRef xStrm = rStg->OpenSotStream( sStrmName,
                    ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE ) );
            if( xStrm.Is() )
            {
                xStrm->SetSize( 0 );
                xStrm->SetBufferSize( 8192 );
                OUString aMime( "text/xml" );
                uno::Any aAny;
                aAny <<= aMime;
                xStrm->SetProperty( OUString("MediaType"), aAny );


                uno::Reference< uno::XComponentContext > xContext =
                    comphelper::getProcessComponentContext();

                uno::Reference < xml::sax::XWriter > xWriter  = xml::sax::Writer::create(xContext);
                uno::Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *xStrm );
                xWriter->setOutputStream(xOut);

                uno::Reference < xml::sax::XDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
                SvXMLExceptionListExport aExp( xContext, rLst, sStrmName, xHandler );

                aExp.exportDoc( XML_BLOCK_LIST );

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
        String aXMLWordListName( pXMLImplAutocorr_ListStr, RTL_TEXTENCODING_MS_1252 );
        uno::Reference < io::XStream > xStrm = xStg->openStreamElement( aXMLWordListName, embed::ElementModes::READ );
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aXMLWordListName;
        aParserInput.aInputStream = xStrm->getInputStream();

        // get parser
        uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);
        SAL_INFO("editeng", "AutoCorrect Import" );
        uno::Reference< xml::sax::XDocumentHandler > xFilter = new SvXMLAutoCorrectImport( xContext, pAutocorr_List, rAutoCorrect, xStg );

        // connect parser and filter
        xParser->setDocumentHandler( xFilter );

        // parse
        xParser->parseStream( aParserInput );
    }
    catch ( const uno::Exception& )
    {
    }

    // Set time stamp
    FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                    &aModifiedDate, &aModifiedTime );
    aLastCheckTime = Time( Time::SYSTEM );

    return pAutocorr_List;
}

void SvxAutoCorrectLanguageLists::SetAutocorrWordList( SvxAutocorrWordList* pList )
{
    if( pAutocorr_List && pList != pAutocorr_List )
        delete pAutocorr_List;
    pAutocorr_List = pList;
    if( !pAutocorr_List )
    {
        OSL_ENSURE( !this, "No valid list" );
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

sal_Bool SvxAutoCorrectLanguageLists::AddToCplSttExceptList(const String& rNew)
{
    sal_Bool aRet = sal_False;
    if( rNew.Len() && GetCplSttExceptList()->insert( rNew ).second )
    {
        MakeUserStorage_Impl();
        SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );

        SaveExceptList_Imp( *pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );

        xStg = 0;
        // Set time stamp
        FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time( Time::SYSTEM );
        aRet = sal_True;
    }
    return aRet;
}

sal_Bool SvxAutoCorrectLanguageLists::AddToWrdSttExceptList(const String& rNew)
{
    sal_Bool aRet = sal_False;
    SvStringsISortDtor* pExceptList = LoadWrdSttExceptList();
    if( rNew.Len() && pExceptList && pExceptList->insert( rNew ).second )
    {
        MakeUserStorage_Impl();
        SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );

        SaveExceptList_Imp( *pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );

        xStg = 0;
        // Set time stamp
        FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time( Time::SYSTEM );
        aRet = sal_True;
    }
    return aRet;
}

SvStringsISortDtor* SvxAutoCorrectLanguageLists::LoadCplSttExceptList()
{
    SotStorageRef xStg = new SotStorage( sShareAutoCorrFile, STREAM_READ | STREAM_SHARE_DENYNONE, sal_True );
    String sTemp ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplCplStt_ExcptLstStr ) );
    if( xStg.Is() && xStg->IsContained( sTemp ) )
        LoadXMLExceptList_Imp( pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );

    return pCplStt_ExcptLst;
}

void SvxAutoCorrectLanguageLists::SaveCplSttExceptList()
{
    MakeUserStorage_Impl();
    SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );

    SaveExceptList_Imp( *pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );

    xStg = 0;

    // Set time stamp
    FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
    aLastCheckTime = Time( Time::SYSTEM );
}

void SvxAutoCorrectLanguageLists::SetCplSttExceptList( SvStringsISortDtor* pList )
{
    if( pCplStt_ExcptLst && pList != pCplStt_ExcptLst )
        delete pCplStt_ExcptLst;

    pCplStt_ExcptLst = pList;
    if( !pCplStt_ExcptLst )
    {
        OSL_ENSURE( !this, "No valid list" );
        pCplStt_ExcptLst = new SvStringsISortDtor;
    }
    nFlags |= CplSttLstLoad;
}

SvStringsISortDtor* SvxAutoCorrectLanguageLists::LoadWrdSttExceptList()
{
    SotStorageRef xStg = new SotStorage( sShareAutoCorrFile, STREAM_READ | STREAM_SHARE_DENYNONE, sal_True );
    String sTemp ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplWrdStt_ExcptLstStr ) );
    if( xStg.Is() && xStg->IsContained( sTemp ) )
        LoadXMLExceptList_Imp( pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );
    return pWrdStt_ExcptLst;
}

void SvxAutoCorrectLanguageLists::SaveWrdSttExceptList()
{
    MakeUserStorage_Impl();
    SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );

    SaveExceptList_Imp( *pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );

    xStg = 0;
    // Set time stamp
    FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                            &aModifiedDate, &aModifiedTime );
    aLastCheckTime = Time( Time::SYSTEM );
}

void SvxAutoCorrectLanguageLists::SetWrdSttExceptList( SvStringsISortDtor* pList )
{
    if( pWrdStt_ExcptLst && pList != pWrdStt_ExcptLst )
        delete pWrdStt_ExcptLst;
    pWrdStt_ExcptLst = pList;
    if( !pWrdStt_ExcptLst )
    {
        OSL_ENSURE( !this, "No valid list" );
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

void SvxAutoCorrectLanguageLists::RemoveStream_Imp( const String& rName )
{
    if( sShareAutoCorrFile != sUserAutoCorrFile )
    {
        SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );
        if( xStg.Is() && SVSTREAM_OK == xStg->GetError() &&
            xStg->IsStream( rName ) )
        {
            xStg->Remove( rName );
            xStg->Commit();

            xStg = 0;
        }
    }
}

void SvxAutoCorrectLanguageLists::MakeUserStorage_Impl()
{
    // The conversion needs to happen if the file is already in the user
    // directory and is in the old format. Additionally it needs to
    // happen when the file is being copied from share to user.

    sal_Bool bError = sal_False, bConvert = sal_False, bCopy = sal_False;
    INetURLObject aDest;
    INetURLObject aSource;

    if (sUserAutoCorrFile != sShareAutoCorrFile )
    {
        aSource = INetURLObject ( sShareAutoCorrFile );
        aDest = INetURLObject ( sUserAutoCorrFile );
        if ( SotStorage::IsOLEStorage ( sShareAutoCorrFile ) )
        {
            aDest.SetExtension ( OUString("bak") );
            bConvert = sal_True;
        }
        bCopy = sal_True;
    }
    else if ( SotStorage::IsOLEStorage ( sUserAutoCorrFile ) )
    {
        aSource = INetURLObject ( sUserAutoCorrFile );
        aDest = INetURLObject ( sUserAutoCorrFile );
        aDest.SetExtension ( OUString("bak") );
        bCopy = bConvert = sal_True;
    }
    if (bCopy)
    {
        try
        {
            String sMain(aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ));
            sal_Unicode cSlash = '/';
            xub_StrLen nSlashPos = sMain.SearchBackward(cSlash);
            sMain.Erase(nSlashPos);
            ::ucbhelper::Content aNewContent( sMain, uno::Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            Any aAny;
            TransferInfo aInfo;
            aInfo.NameClash = NameClash::OVERWRITE;
            aInfo.NewTitle  = aDest.GetName();
            aInfo.SourceURL = aSource.GetMainURL( INetURLObject::DECODE_TO_IURI );
            aInfo.MoveData  = sal_False;
            aAny <<= aInfo;
            aNewContent.executeCommand( OUString (  "transfer"  ), aAny);
        }
        catch (...)
        {
            bError = sal_True;
        }
    }
    if (bConvert && !bError)
    {
        SotStorageRef xSrcStg = new SotStorage( aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ), STREAM_READ, sal_True );
        SotStorageRef xDstStg = new SotStorage( sUserAutoCorrFile, STREAM_WRITE, sal_True );

        if( xSrcStg.Is() && xDstStg.Is() )
        {
            String sXMLWord     ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplWrdStt_ExcptLstStr ) );
            String sXMLSentence ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplCplStt_ExcptLstStr ) );
            SvStringsISortDtor  *pTmpWordList = NULL;

            if (xSrcStg->IsContained( sXMLWord ) )
                LoadXMLExceptList_Imp( pTmpWordList, pXMLImplWrdStt_ExcptLstStr, xSrcStg );

            if (pTmpWordList)
            {
                SaveExceptList_Imp( *pTmpWordList, pXMLImplWrdStt_ExcptLstStr, xDstStg, sal_True );
                pTmpWordList->clear();
                pTmpWordList = NULL;
            }


            if (xSrcStg->IsContained( sXMLSentence ) )
                LoadXMLExceptList_Imp( pTmpWordList, pXMLImplCplStt_ExcptLstStr, xSrcStg );

            if (pTmpWordList)
            {
                SaveExceptList_Imp( *pTmpWordList, pXMLImplCplStt_ExcptLstStr, xDstStg, sal_True );
                pTmpWordList->clear();
            }

            GetAutocorrWordList();
            MakeBlocklist_Imp( *xDstStg );
            sShareAutoCorrFile = sUserAutoCorrFile;
            xDstStg = 0;
            try
            {
                ::ucbhelper::Content aContent ( aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ), uno::Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                aContent.executeCommand ( OUString( "delete" ), makeAny ( sal_Bool (sal_True ) ) );
            }
            catch (...)
            {
            }
        }
    }
    else if( bCopy && !bError )
        sShareAutoCorrFile = sUserAutoCorrFile;
}

sal_Bool SvxAutoCorrectLanguageLists::MakeBlocklist_Imp( SvStorage& rStg )
{
    String sStrmName( pXMLImplAutocorr_ListStr, RTL_TEXTENCODING_MS_1252 );
    sal_Bool bRet = sal_True, bRemove = !pAutocorr_List || pAutocorr_List->empty();
    if( !bRemove )
    {
        SvStorageStreamRef refList = rStg.OpenSotStream( sStrmName,
                    ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE ) );
        if( refList.Is() )
        {
            refList->SetSize( 0 );
            refList->SetBufferSize( 8192 );
            String aPropName( OUString( "MediaType" ) );
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
            SvXMLAutoCorrectExport aExp( xContext, pAutocorr_List, sStrmName, xHandler );

            aExp.exportDoc( XML_BLOCK_LIST );

            refList->Commit();
            bRet = SVSTREAM_OK == refList->GetError();
            if( bRet )
            {
                refList.Clear();
                rStg.Commit();
                if( SVSTREAM_OK != rStg.GetError() )
                {
                    bRemove = sal_True;
                    bRet = sal_False;
                }
            }
        }
        else
            bRet = sal_False;
    }

    if( bRemove )
    {
        rStg.Remove( sStrmName );
        rStg.Commit();
    }

    return bRet;
}

sal_Bool SvxAutoCorrectLanguageLists::MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries, std::vector<SvxAutocorrWord>& aDeleteEntries )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();
    SotStorageRef xStorage = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );

    sal_Bool bRet = xStorage.Is() && SVSTREAM_OK == xStorage->GetError();

    if( bRet )
    {
        for ( sal_uInt32 i=0; i < aDeleteEntries.size(); i++ )
        {
            SvxAutocorrWord aWordToDelete = aDeleteEntries[i];
            SvxAutocorrWord *pFoundEntry = pAutocorr_List->FindAndRemove( &aWordToDelete );
            if( pFoundEntry )
            {
                if( !pFoundEntry->IsTextOnly() )
                {
                    String aName( aWordToDelete.GetShort() );
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

        for ( sal_uInt32 i=0; i < aNewEntries.size(); i++ )
        {
            SvxAutocorrWord *pWordToAdd = new SvxAutocorrWord( aNewEntries[i].GetShort(), aNewEntries[i].GetLong(), sal_True );
            SvxAutocorrWord *pRemoved = pAutocorr_List->FindAndRemove( pWordToAdd );
            if( pRemoved )
            {
                if( !pRemoved->IsTextOnly() )
                {
                    // Still have to remove the Storage
                    String sStorageName( pWordToAdd->GetShort() );
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

sal_Bool SvxAutoCorrectLanguageLists::PutText( const String& rShort, const String& rLong )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();
    SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );

    sal_Bool bRet = xStg.Is() && SVSTREAM_OK == xStg->GetError();

    // Update the word list
    if( bRet )
    {
        SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, rLong, sal_True );
        SvxAutocorrWord *pRemove = pAutocorr_List->FindAndRemove( pNew );
        if( pRemove )
        {
            if( !pRemove->IsTextOnly() )
            {
                // Still have to remove the Storage
                String sStgNm( rShort );
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
            xStg = 0;
        }
        else
        {
            delete pNew;
            bRet = sal_False;
        }
    }
    return bRet;
}

sal_Bool SvxAutoCorrectLanguageLists::PutText( const String& rShort,
                                               SfxObjectShell& rShell )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();

    sal_Bool bRet = sal_False;
    OUString sLong;
    try
    {
        uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetStorageFromURL( sUserAutoCorrFile, embed::ElementModes::READWRITE );
        bRet = rAutoCorrect.PutText( xStg, sUserAutoCorrFile, rShort, rShell, sLong );
        xStg = 0;

        // Update the word list
        if( bRet )
        {
            SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, sLong, sal_False );
            if( pAutocorr_List->Insert( pNew ) )
            {
                SotStorageRef xStor = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );
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

// Delete an entry
sal_Bool SvxAutoCorrectLanguageLists::DeleteText( const String& rShort )
{
    // First get the current list!
    GetAutocorrWordList();

    MakeUserStorage_Impl();

    SotStorageRef xStg = new SotStorage( sUserAutoCorrFile, STREAM_READWRITE, sal_True );
    sal_Bool bRet = xStg.Is() && SVSTREAM_OK == xStg->GetError();
    if( bRet )
    {
        SvxAutocorrWord aTmp( rShort, rShort );
        SvxAutocorrWord *pFnd = pAutocorr_List->FindAndRemove( &aTmp );
        if( pFnd )
        {
            if( !pFnd->IsTextOnly() )
            {
                String aName( rShort );
                if (xStg->IsOLEStorage())
                    aName = EncryptBlockName_Imp(aName);
                else
                    GeneratePackageName ( rShort, aName );
                if( xStg->IsContained( aName ) )
                {
                    xStg->Remove( aName );
                    bRet = xStg->Commit();
                }

            }
            delete pFnd;
            MakeBlocklist_Imp( *xStg );
            xStg = 0;
        }
        else
            bRet = sal_False;
    }
    return bRet;
}

// Keep the list sorted ...
bool CompareSvxAutocorrWordList::operator()( SvxAutocorrWord* const& lhs, SvxAutocorrWord* const& rhs ) const
{
    CollatorWrapper& rCmp = ::GetCollatorWrapper();
    return rCmp.compareString( lhs->GetShort(), rhs->GetShort() ) < 0;
}

SvxAutocorrWordList::~SvxAutocorrWordList()
{
    DeleteAndDestroyAll();
}

void SvxAutocorrWordList::DeleteAndDestroyAll()
{
    for( SvxAutocorrWordList_Hash::const_iterator it = maHash.begin(); it != maHash.end(); ++it )
        delete it->second;
    maHash.clear();

    for( SvxAutocorrWordList_Set::const_iterator it2 = maSet.begin(); it2 != maSet.end(); ++it2 )
        delete *it2;
    maSet.clear();
}

// returns true if inserted
bool SvxAutocorrWordList::Insert(SvxAutocorrWord *pWord) const
{
    if ( maSet.empty() ) // use the hash
    {
        OUString aShort( pWord->GetShort() );
        return maHash.insert( std::pair<OUString, SvxAutocorrWord *>( aShort, pWord ) ).second;
    }
    else
        return maSet.insert( pWord ).second;
}

void SvxAutocorrWordList::LoadEntry(const OUString& sWrong, const OUString& sRight, sal_Bool bOnlyTxt)
{
    SvxAutocorrWord* pNew = new SvxAutocorrWord( sWrong, sRight, bOnlyTxt );
    if( !Insert( pNew ) )
        delete pNew;
}

bool SvxAutocorrWordList::empty() const
{
    return maHash.empty() && maSet.empty();
}

SvxAutocorrWord *SvxAutocorrWordList::FindAndRemove(SvxAutocorrWord *pWord)
{
    SvxAutocorrWord *pMatch = NULL;

    if ( maSet.empty() ) // use the hash
    {
        SvxAutocorrWordList_Hash::iterator it = maHash.find( pWord->GetShort() );
        if( it != maHash.end() )
        {
            pMatch = it->second;
            maHash.erase (it);
        }
    }
    else
    {
        SvxAutocorrWordList_Set::iterator it = maSet.find( pWord );
        if( it != maSet.end() )
        {
            pMatch = *it;
            maSet.erase (it);
        }
    }
    return pMatch;
}

// return the sorted contents - defer sorting until we have to.
SvxAutocorrWordList::Content SvxAutocorrWordList::getSortedContent() const
{
    Content aContent;

    // convert from hash to set permanantly
    if ( maSet.empty() )
    {
        // This beasty has some O(N log(N)) in a terribly slow ICU collate fn.
        for( SvxAutocorrWordList_Hash::const_iterator it = maHash.begin(); it != maHash.end(); ++it )
            maSet.insert( it->second );
        maHash.clear();
    }
    for( SvxAutocorrWordList_Set::const_iterator it = maSet.begin(); it != maSet.end(); ++it )
        aContent.push_back( *it );

    return aContent;
}

const SvxAutocorrWord* SvxAutocorrWordList::WordMatches(const SvxAutocorrWord *pFnd,
                                      const OUString &rTxt,
                                      xub_StrLen &rStt,
                                      xub_StrLen nEndPos) const
{
    const String& rChk = pFnd->GetShort();
    xub_StrLen left_wildcard = ( rChk.GetChar( 0 ) == C_ASTERISK ) ? 1 : 0; // "*word" pattern?
    xub_StrLen nSttWdPos = nEndPos;
    if( nEndPos >= rChk.Len() - left_wildcard)
    {

        bool bWasWordDelim = false;
        xub_StrLen nCalcStt = nEndPos - rChk.Len() + left_wildcard;
        if( ( !nCalcStt || nCalcStt == rStt || left_wildcard ||
              ( nCalcStt < rStt &&
                IsWordDelim( rTxt[ nCalcStt - 1 ] ))) )
        {
            TransliterationWrapper& rCmp = GetIgnoreTranslWrapper();
            OUString sWord = rTxt.copy(nCalcStt, rChk.Len() - left_wildcard);
            if( (!left_wildcard && rCmp.isEqual( rChk, sWord )) || (left_wildcard && rCmp.isEqual( rChk.Copy(1), sWord) ))
            {
                rStt = nCalcStt;
                if (!left_wildcard) return pFnd;
                // get the first word delimiter position before the matching "*word" pattern
                while( rStt && !(bWasWordDelim = IsWordDelim( rTxt[ --rStt ])))
                    ;
                if (bWasWordDelim) rStt++;
                SvxAutocorrWord* pNew = new SvxAutocorrWord(rTxt.copy(rStt, nEndPos - rStt), rTxt.copy(rStt, nEndPos - rStt - rChk.Len() + 1) + pFnd->GetLong());
                if( Insert( pNew ) ) return pNew; else delete pNew;
            }
        }
        // match "word*" patterns, eg. "i18n*"
        if ( rChk.GetChar( rChk.Len() - 1) == C_ASTERISK )
        {
            String sTmp( rChk.Copy( 0, rChk.Len() - 1 ) );
            // Get the last word delimiter position
            bool not_suffix;
            while( nSttWdPos && !(bWasWordDelim = IsWordDelim( rTxt[ --nSttWdPos ])))
                ;
            // search the first occurance with a left word delimitation
            sal_Int32 nFndPos = -1;
            do {
                nFndPos = rTxt.indexOf( sTmp, nFndPos + 1);
                not_suffix = (bWasWordDelim && (nSttWdPos >= nFndPos + sTmp.Len()));
            } while ( nFndPos != -1 && (!(!nFndPos || IsWordDelim( rTxt[ nFndPos - 1 ])) || not_suffix));
            if ( nFndPos != -1 )
            {
                // store matching pattern and its replacement as a new list item, eg. "i18ns" -> "internationalizations"
                OUString aShort = rTxt.copy(nFndPos, nEndPos - nFndPos + ((rTxt[nEndPos] != 0x20) ? 1: 0));
                OUString aLong = pFnd->GetLong() + rTxt.copy(nFndPos + sTmp.Len(), nEndPos - nFndPos - sTmp.Len());
                SvxAutocorrWord* pNew = new SvxAutocorrWord(aShort, aLong);
                if( Insert( pNew ) )
                {
                    rStt = nFndPos;
                    return pNew;
                } else delete pNew;
            }
        }
    }
    return NULL;
}

const SvxAutocorrWord* SvxAutocorrWordList::SearchWordsInList(const OUString& rTxt, xub_StrLen& rStt,
                                                              xub_StrLen nEndPos) const
{
    for( SvxAutocorrWordList_Hash::const_iterator it = maHash.begin(); it != maHash.end(); ++it )
    {
        if( const SvxAutocorrWord *aTmp = WordMatches( it->second, rTxt, rStt, nEndPos ) )
            return aTmp;
    }

    for( SvxAutocorrWordList_Set::const_iterator it2 = maSet.begin(); it2 != maSet.end(); ++it2 )
    {
        if( const SvxAutocorrWord *aTmp = WordMatches( *it2, rTxt, rStt, nEndPos ) )
            return aTmp;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
