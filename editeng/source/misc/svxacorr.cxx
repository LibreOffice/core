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

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <tools/urlobj.hxx>
#include <i18npool/mslangid.hxx>
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
#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <sot/storage.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/unolingu.hxx>
#include "vcl/window.hxx"
#include <helpid.hrc>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <unotools/streamwrap.hxx>
#include <SvXMLAutoCorrectImport.hxx>
#include <SvXMLAutoCorrectExport.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <xmloff/xmltoken.hxx>
#include <vcl/help.hxx>
#include <rtl/logfile.hxx>

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

static const sal_Char pImplWrdStt_ExcptLstStr[]    = "WordExceptList";
static const sal_Char pImplCplStt_ExcptLstStr[]    = "SentenceExceptList";
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

void EncryptBlockName_Imp( String& rName );

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
    // Is called by the functions:
    //  - FnCptlSttWrd
    //  - FnCptlSttSntnc
    // after the exchange of characters. then the words can maybe be inserted
    // into the exception list.
void SvxAutoCorrDoc::SaveCpltSttWord( sal_uLong, xub_StrLen, const String&,
                                        sal_Unicode )
{
}

LanguageType SvxAutoCorrDoc::GetLanguage( xub_StrLen , sal_Bool ) const
{
    return LANGUAGE_SYSTEM;
}

static ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& GetProcessFact()
{
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();
    return xMSF;
}

static sal_uInt16 GetAppLang()
{
    return Application::GetSettings().GetLanguage();
}
static LocaleDataWrapper& GetLocaleDataWrapper( sal_uInt16 nLang )
{
    static LocaleDataWrapper aLclDtWrp( GetProcessFact(),
                                        SvxCreateLocale( GetAppLang() ) );
    ::com::sun::star::lang::Locale aLcl( SvxCreateLocale( nLang ));
    const ::com::sun::star::lang::Locale& rLcl = aLclDtWrp.getLoadedLocale();
    if( aLcl.Language != rLcl.Language ||
        aLcl.Country != rLcl.Country ||
        aLcl.Variant != rLcl.Variant )
        aLclDtWrp.setLocale( aLcl );
    return aLclDtWrp;
}
static TransliterationWrapper& GetIgnoreTranslWrapper()
{
    static int bIsInit = 0;
    static TransliterationWrapper aWrp( GetProcessFact(),
                ::com::sun::star::i18n::TransliterationModules_IGNORE_KANA |
                ::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH );
    if( !bIsInit )
    {
        aWrp.loadModuleIfNeeded( GetAppLang() );
        bIsInit = 1;
    }
    return aWrp;
}
static CollatorWrapper& GetCollatorWrapper()
{
    static int bIsInit = 0;
    static CollatorWrapper aCollWrp( GetProcessFact() );
    if( !bIsInit )
    {
        aCollWrp.loadDefaultCollator( SvxCreateLocale( GetAppLang() ), 0 );
        bIsInit = 1;
    }
    return aCollWrp;
}


// Keep the list sorted ...
bool CompareSvxAutocorrWordList::operator()( SvxAutocorrWord* const& lhs, SvxAutocorrWord* const& rhs ) const
{
    CollatorWrapper& rCmp = ::GetCollatorWrapper();
    return rCmp.compareString( lhs->GetShort(), rhs->GetShort() ) < 0;
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
            cChar == ':'  || cChar == '?' || cChar == '!' || cChar == '/';
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
    LanguageType eLang = GetAppLang();
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
    pCharClass = new CharClass( SvxCreateLocale( eLang ));
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

    // Two capital letters at the beginning of word?
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
            rtl::OUString sChar( cSave );
            sChar = rCC.lowercase( sChar );
            if( sChar[0] != cSave && rDoc.ReplaceRange( nSttPos, 1, sChar ))
            {
                if( SaveWordWrdSttLst & nFlags )
                    rDoc.SaveCpltSttWord( CptlSttWrd, nSttPos, sWord, cSave );
                bRet = sal_True;
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
        rtl::OUString sServiceName("com.sun.star.i18n.OrdinalSuffix");
        uno::Reference< i18n::XOrdinalSuffix > xOrdSuffix(
                comphelper::getProcessServiceFactory()->createInstance( sServiceName ),
                uno::UNO_QUERY );

        if ( xOrdSuffix.is( ) )
        {
            uno::Sequence< rtl::OUString > aSuffixes = xOrdSuffix->getOrdinalSuffix( nNum, rCC.getLocale( ) );
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
        eLang = GetAppLang();
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
                if( rCC.isLetterNumeric( rtl::OUString(cCh) ) )
                {
                    for( n = nSttPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt.GetChar( --n ))); )
                        ;

                    // found: "[A-z0-9][<AnyEndChars>] --[<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( rtl::OUString(cCh) ))
                    {
                        rDoc.Delete( nSttPos, nSttPos + 2 );
                        rDoc.Insert( nSttPos, bAlwaysUseEmDash ? rtl::OUString(cEmDash) : rtl::OUString(cEnDash) );
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
                if( rCC.isLetterNumeric( rtl::OUString(cCh) ) )
                {
                    cCh = ' ';
                    for( n = nTmpPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt.GetChar( --n ))); )
                            ;
                    // found: "[A-z0-9][<AnyEndChars>] - [<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( rtl::OUString(cCh) ))
                    {
                        rDoc.Delete( nTmpPos, nTmpPos + nLen );
                        rDoc.Insert( nTmpPos, bAlwaysUseEmDash ? rtl::OUString(cEmDash) : rtl::OUString(cEnDash) );
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
            rDoc.Insert( nSttPos, (bEnDash ? rtl::OUString(cEnDash) : rtl::OUString(cEmDash)) );
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
    const lang::Locale rLocale = rCC.getLocale( );

    if ( rLocale.Language == OUString( "fr" ) )
    {
        bool bFrCA = rLocale.Country == OUString( "CA" );
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
            xub_StrLen nStrPos = rTxt.Search( rtl::OUString( "://" ), nSttWdPos + 1 );
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
                        rDoc.Insert( nPos, rtl::OUString(CHAR_HARDBLANK) );
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
    String sURL( URIHelper::FindFirstURLInText( rTxt, nSttPos, nEndPos,
                                                GetCharClass( eLang ) ));
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
        // Span the Attribute over the area and delete the Character found at
        // the end.
        if( '*' == cInsChar )           // Bold
        {
            SvxWeightItem aSvxWeightItem( WEIGHT_BOLD, SID_ATTR_CHAR_WEIGHT );
            rDoc.SetAttr( nFndPos + 1, nEndPos,
                            SID_ATTR_CHAR_WEIGHT,
                            aSvxWeightItem);
        }
        else                            // underline
        {
            SvxUnderlineItem aSvxUnderlineItem( UNDERLINE_SINGLE, SID_ATTR_CHAR_UNDERLINE );
            rDoc.SetAttr( nFndPos + 1, nEndPos,
                            SID_ATTR_CHAR_UNDERLINE,
                            aSvxUnderlineItem);
        }
        rDoc.Delete( nEndPos, nEndPos + 1 );
        rDoc.Delete( nFndPos, nFndPos + 1 );
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
                (long)(pStart + 1) <= (long)pStr &&
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
        const String* pPrevPara = rDoc.GetPrevPara( bNormalPos );
        if( !pPrevPara )
        {
            // valid separator -> replace
            rtl::OUString sChar( *pWordStt );
            sChar = rCC.uppercase( sChar );
            return  !comphelper::string::equals(sChar, *pWordStt) &&
                    rDoc.ReplaceRange( xub_StrLen( pWordStt - pStart ), 1, sChar );
        }

        aText = *pPrevPara;
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
        sWord = rtl::OUString(pStr, pExceptStt - pStr + 1);
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
    rtl::OUString sChar( cSave );
    sChar = rCC.uppercase( sChar );
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
    aConverted.Append( rCC.uppercase(rtl::OUString(rTxt.GetChar(nSttPos))) );
    aConverted.Append( rCC.lowercase(rtl::OUString(rTxt.GetChar(nSttPos+1))) );

    for (xub_StrLen i = nSttPos+2; i < nEndPos; ++i)
    {
        if ( IsLowerLetter(rCC.getCharacterType(rTxt, i)) )
            // A lowercase letter disqualifies the whole text.
            return false;

        if ( IsUpperLetter(rCC.getCharacterType(rTxt, i)) )
            // Another uppercase letter.  Convert it.
            aConverted.Append(rCC.lowercase(rtl::OUString(rTxt.GetChar(i))));
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

    rtl::OUString sChg( cInsChar );
    if( bIns )
        rDoc.Insert( nInsPos, sChg );
    else
        rDoc.Replace( nInsPos, sChg );

    sChg = rtl::OUString(cRet);

    if( '\"' == cInsChar )
    {
        if( LANGUAGE_SYSTEM == eLang )
            eLang = GetAppLang();
        switch( eLang )
        {
        case LANGUAGE_FRENCH:
        case LANGUAGE_FRENCH_BELGIAN:
        case LANGUAGE_FRENCH_CANADIAN:
        case LANGUAGE_FRENCH_SWISS:
        case LANGUAGE_FRENCH_LUXEMBOURG:
            {
                rtl::OUString s( static_cast< sal_Unicode >(0xA0) );
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

    String sRet = rtl::OUString(cRet);

    if( '\"' == cInsChar )
    {
        if( LANGUAGE_SYSTEM == eLang )
            eLang = GetAppLang();
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

sal_uLong SvxAutoCorrect::AutoCorrect( SvxAutoCorrDoc& rDoc, const String& rTxt,
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
                rDoc.Insert( nInsPos, rtl::OUString(cChar) );
            else
                rDoc.Replace( nInsPos, rtl::OUString(cChar) );

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
            const String* pPara = 0;
            const String** ppPara = IsAutoCorrFlag(CptlSttSntnc) ? &pPara : 0;

            sal_Bool bChgWord = rDoc.ChgAutoCorrWord( nCapLttrPos, nInsPos,
                                                    *this, ppPara );
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
                    rDoc.ChgAutoCorrWord( nCapLttrPos1, nInsPos1, *this, ppPara ))
                {
                    bChgWord = sal_True;
                    nCapLttrPos = nCapLttrPos1;
                }
            }

            if( bChgWord )
            {
                nRet = Autocorrect;
                if( pPara )
                {
                    xub_StrLen nEnd = nCapLttrPos;
                    while( nEnd < pPara->Len() &&
                            !IsWordDelim( pPara->GetChar( nEnd )))
                        ++nEnd;

                    // Capital letter at beginning of paragraph?
                    if( IsAutoCorrFlag( CptlSttSntnc ) &&
                        FnCptlSttSntnc( rDoc, *pPara, sal_False,
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
            if( IsAutoCorrFlag( CptlSttSntnc ) &&
                FnCptlSttSntnc( rDoc, rTxt, sal_True, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CptlSttSntnc;

            // Two capital letters at beginning of word ??
            if( IsAutoCorrFlag( CptlSttWrd ) &&
                FnCptlSttWrd( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CptlSttWrd;

            if( IsAutoCorrFlag( ChgToEnEmDash ) &&
                FnChgToEnEmDash( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
                nRet |= ChgToEnEmDash;
        }

    } while( sal_False );

    if( nRet )
    {
        const char* aHelpIds[] =
        {
            HID_AUTOCORR_HELP_WORD,
            HID_AUTOCORR_HELP_SENT,
            HID_AUTOCORR_HELP_SENTWORD,
            HID_AUTOCORR_HELP_ACORWORD,
            "",
            HID_AUTOCORR_HELP_ACORSENTWORD,
            "",
            HID_AUTOCORR_HELP_CHGTOENEMDASH,
            HID_AUTOCORR_HELP_WORDENEMDASH,
            HID_AUTOCORR_HELP_SENTENEMDASH,
            HID_AUTOCORR_HELP_SENTWORDENEMDASH,
            HID_AUTOCORR_HELP_ACORWORDENEMDASH,
            "",
            HID_AUTOCORR_HELP_ACORSENTWORDENEMDASH,
            "",
            HID_AUTOCORR_HELP_CHGQUOTES,
            HID_AUTOCORR_HELP_CHGSGLQUOTES,
            HID_AUTOCORR_HELP_SETINETATTR,
            HID_AUTOCORR_HELP_INGNOREDOUBLESPACE,
            HID_AUTOCORR_HELP_CHGWEIGHTUNDERL,
            HID_AUTOCORR_HELP_CHGFRACTIONSYMBOL,
            HID_AUTOCORR_HELP_CHGORDINALNUMBER
        };

        sal_uLong nHelpId = 0;
        if( nRet & ( Autocorrect|CptlSttSntnc|CptlSttWrd|ChgToEnEmDash ) )
        {
            // from 0 - 15
            if( nRet & ChgToEnEmDash )
                nHelpId += 8;
            if( nRet & Autocorrect )
                nHelpId += 4;
            if( nRet & CptlSttSntnc )
                nHelpId += 2;
            if( nRet & CptlSttWrd )
                nHelpId += 1;
        }
        else
        {
                 if( nRet & ChgQuotes)          nHelpId = 16;
            else if( nRet & ChgSglQuotes)       nHelpId = 17;
            else if( nRet & SetINetAttr)        nHelpId = 18;
            else if( nRet & IgnoreDoubleSpace)  nHelpId = 19;
            else if( nRet & ChgWeightUnderl)    nHelpId = 20;
            else if( nRet & AddNonBrkSpace)     nHelpId = 21;
            else if( nRet & ChgOrdinalNumber)   nHelpId = 22;
        }

        if( nHelpId )
        {
            nHelpId -= 1;
            Application::GetHelp()->OpenHelpAgent( aHelpIds[nHelpId] );
        }
    }


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
        OSL_FAIL("Save an empty list? ");
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
        OSL_FAIL("Save an empty list? ");
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
        nTmpVal = pLangTable->find(LANGUAGE_DONTKNOW);
        if(nTmpVal != pLangTable->end())
            pLists = nTmpVal->second;
        else if(CreateLanguageFile(LANGUAGE_DONTKNOW, sal_True))
            pLists = pLangTable->find(LANGUAGE_DONTKNOW)->second;
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
        nTmpVal = pLangTable->find(LANGUAGE_DONTKNOW);
        if(nTmpVal != pLangTable->end())
            pLists = nTmpVal->second;
        else if(CreateLanguageFile(LANGUAGE_DONTKNOW, sal_True))
            pLists = pLangTable->find(LANGUAGE_DONTKNOW)->second;
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

    String sUserDirFile( GetAutoCorrFileName( eLang, sal_True, sal_False ));
    String sShareDirFile( sUserDirFile );

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

    //  - Delete an entry
sal_Bool SvxAutoCorrect::DeleteText( const String& rShort, LanguageType eLang )
{
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>::iterator nTmpVal = pLangTable->find(eLang);
    if(nTmpVal != pLangTable->end())
        return nTmpVal->second->DeleteText(rShort);
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
sal_Bool SvxAutoCorrect::GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String&, const String& , String& )
{
    return sal_False;
}

    // Text with attribution (only the SWG - SWG format!)
sal_Bool SvxAutoCorrect::PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String&, const String&, SfxObjectShell&,
                                String& )
{
    return sal_False;
}

void EncryptBlockName_Imp( String& rName )
{
    xub_StrLen nLen, nPos = 1;
    rName.Insert( '#', 0 );
    sal_Unicode* pName = rName.GetBufferAccess();
    for ( nLen = rName.Len(), ++pName; nPos < nLen; ++nPos, ++pName )
    {
        if( lcl_IsInAsciiArr( "!/:.\\", *pName ))
            *pName &= 0x0f;
    }
}

/* This code is copied from SwXMLTextBlocks::GeneratePackageName */
static void GeneratePackageName ( const String& rShort, String& rPackageName )
{
    rPackageName = rShort;
    xub_StrLen nPos = 0;
    sal_Unicode pDelims[] = { '!', '/', ':', '.', '\\', 0 };
    rtl::OString sByte(rtl::OUStringToOString(rPackageName, RTL_TEXTENCODING_UTF7));
    rPackageName = rtl::OStringToOUString(sByte, RTL_TEXTENCODING_ASCII_US);
    while( STRING_NOTFOUND != ( nPos = rPackageName.SearchChar( pDelims, nPos )))
    {
        rPackageName.SetChar( nPos, '_' );
        ++nPos;
    }
}

static const SvxAutocorrWord* lcl_SearchWordsInList(
                SvxAutoCorrectLanguageListsPtr pList, const String& rTxt,
                xub_StrLen& rStt, xub_StrLen nEndPos, SvxAutoCorrDoc& )
{
    const SvxAutocorrWordList* pAutoCorrWordList = pList->GetAutocorrWordList();
    TransliterationWrapper& rCmp = GetIgnoreTranslWrapper();
    for( SvxAutocorrWordList::const_iterator it = pAutoCorrWordList->begin(); it != pAutoCorrWordList->end(); ++it )
    {
        const SvxAutocorrWord* pFnd = *it;
        const String& rChk = pFnd->GetShort();
        if( nEndPos >= rChk.Len() )
        {
            xub_StrLen nCalcStt = nEndPos - rChk.Len();
            if( ( !nCalcStt || nCalcStt == rStt ||
                ( nCalcStt < rStt &&
                    IsWordDelim( rTxt.GetChar(nCalcStt - 1 ) ))) )
            {
                rtl::OUString sWord(rTxt.GetBuffer() + nCalcStt, rChk.Len());
                if( rCmp.isEqual( rChk, sWord ))
                {
                    rStt = nCalcStt;
                    return pFnd;
                }
            }
        }
    }
    return 0;
}


// the search for the words in the substitution table
const SvxAutocorrWord* SvxAutoCorrect::SearchWordsInList(
                const String& rTxt, xub_StrLen& rStt, xub_StrLen nEndPos,
                SvxAutoCorrDoc& rDoc, LanguageType& rLang )
{
    LanguageType eLang = rLang;
    const SvxAutocorrWord* pRet = 0;
    if( LANGUAGE_SYSTEM == eLang )
        eLang = MsLangId::getSystemLanguage();

    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_DONTKNOW
    if(pLangTable->find(eLang) != pLangTable->end() || CreateLanguageFile(eLang, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(eLang)->second;
        pRet = lcl_SearchWordsInList(  pList, rTxt, rStt, nEndPos, rDoc );
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
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos, rDoc);
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
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos, rDoc);
        if( pRet )
        {
            rLang = nTmpKey2;
            return pRet;
        }
    }

    if(pLangTable->find(LANGUAGE_DONTKNOW) != pLangTable->end() || CreateLanguageFile(LANGUAGE_DONTKNOW, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(LANGUAGE_DONTKNOW)->second;
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos, rDoc);
        if( pRet )
        {
            rLang = LANGUAGE_DONTKNOW;
            return pRet;
        }
    }
    return 0;
}

sal_Bool SvxAutoCorrect::FindInWrdSttExceptList( LanguageType eLang,
                                             const String& sWord )
{
    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_DONTKNOW
    LanguageType nTmpKey1 = eLang & 0x7ff, // the main language in many cases DE
                 nTmpKey2 = eLang & 0x3ff; // otherwise for example EN
    String sTemp(sWord);

    if(pLangTable->find(eLang) != pLangTable->end() || CreateLanguageFile(eLang, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(eLang)->second;
        String _sTemp(sWord);
        if(pList->GetWrdSttExceptList()->find(&_sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }

    // If it still could not be found here, then keep on searching
    if(nTmpKey1 != eLang && (pLangTable->find(nTmpKey1) != pLangTable->end() || CreateLanguageFile(nTmpKey1, sal_False)))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(nTmpKey1)->second;
        if(pList->GetWrdSttExceptList()->find(&sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }

    if(nTmpKey2 != eLang && (pLangTable->find(nTmpKey2) != pLangTable->end() || CreateLanguageFile(nTmpKey2, sal_False)))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(nTmpKey2)->second;
        if(pList->GetWrdSttExceptList()->find(&sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }

    if(pLangTable->find(LANGUAGE_DONTKNOW) != pLangTable->end() || CreateLanguageFile(LANGUAGE_DONTKNOW, sal_False))
    {
        //the language is available - so bring it on
        SvxAutoCorrectLanguageLists* pList = pLangTable->find(LANGUAGE_DONTKNOW)->second;
        if(pList->GetWrdSttExceptList()->find(&sTemp) != pList->GetWrdSttExceptList()->end() )
            return sal_True;
    }
    return sal_False;
}

static sal_Bool lcl_FindAbbreviation( const SvStringsISortDtor* pList, const String& sWord)
{
    String sAbk(rtl::OUString('~'));
    SvStringsISortDtor::const_iterator it = pList->find( &sAbk );
    sal_uInt16 nPos = it - pList->begin();
    if( nPos < pList->size() )
    {
        String sLowerWord( sWord ); sLowerWord.ToLowerAscii();
        const String* pAbk;
        for( sal_uInt16 n = nPos;
                n < pList->size() &&
                '~' == ( pAbk = (*pList)[ n ])->GetChar( 0 );
            ++n )
        {
            // ~ and ~. are not allowed!
            if( 2 < pAbk->Len() && pAbk->Len() - 1 <= sWord.Len() )
            {
                String sLowerAbk( *pAbk ); sLowerAbk.ToLowerAscii();
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
    OSL_ENSURE( !(nPos && '~' == (*pList)[ --nPos ]->GetChar( 0 ) ),
            "Wrongly sorted exception list?" );
    return sal_False;
}

sal_Bool SvxAutoCorrect::FindInCplSttExceptList(LanguageType eLang,
                                const String& sWord, sal_Bool bAbbreviation)
{
    // First search for eLang, then US-English -> English
    // and last in LANGUAGE_DONTKNOW
    LanguageType nTmpKey1 = eLang & 0x7ff, // the main language in many cases DE
                 nTmpKey2 = eLang & 0x3ff; // otherwise for example EN
    String sTemp( sWord );

    if(pLangTable->find(eLang) != pLangTable->end() || CreateLanguageFile(eLang, sal_False))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = pLangTable->find(eLang)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(&sTemp) != pList->end() )
            return sal_True;
    }

    // If it still could not be found here, then keep on searching
    if(nTmpKey1 != eLang && (pLangTable->find(nTmpKey1) != pLangTable->end() || CreateLanguageFile(nTmpKey1, sal_False)))
    {
        const SvStringsISortDtor* pList = pLangTable->find(nTmpKey1)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(&sTemp) != pList->end() )
            return sal_True;
    }

    if(nTmpKey2 != eLang && (pLangTable->find(nTmpKey2) != pLangTable->end() || CreateLanguageFile(nTmpKey2, sal_False)))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = pLangTable->find(nTmpKey2)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(&sTemp) != pList->end() )
            return sal_True;
    }

    if(pLangTable->find(LANGUAGE_DONTKNOW) != pLangTable->end() || CreateLanguageFile(LANGUAGE_DONTKNOW, sal_False))
    {
        //the language is available - so bring it on
        const SvStringsISortDtor* pList = pLangTable->find(LANGUAGE_DONTKNOW)->second->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation(pList, sWord) : pList->find(&sTemp) != pList->end() )
            return sal_True;
    }
    return sal_False;
}

String SvxAutoCorrect::GetAutoCorrFileName( LanguageType eLang,
                                            sal_Bool bNewFile, sal_Bool bTst ) const
{
    String sRet, sExt( MsLangId::convertLanguageToIsoString( eLang ) );
    sExt.Insert('_', 0);
    sExt.AppendAscii( ".dat" );
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
        rpLst->DeleteAndDestroyAll();
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
                uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
                    comphelper::getProcessServiceFactory();
                OSL_ENSURE( xServiceFactory.is(),
                    "XMLReader::Read: got no service manager" );
                if( !xServiceFactory.is() )
                {
                    // Throw an exception ?
                }

                xml::sax::InputSource aParserInput;
                aParserInput.sSystemId = sStrmName;

                xStrm->Seek( 0L );
                xStrm->SetBufferSize( 8 * 1024 );
                aParserInput.aInputStream = new utl::OInputStreamWrapper( *xStrm );

                // get filter
                uno::Reference< xml::sax::XDocumentHandler > xFilter = new SvXMLExceptionListImport ( xServiceFactory, *rpLst );

                // connect parser and filter
                uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( comphelper::getComponentContext(xServiceFactory) );
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
                xStrm->SetProperty( rtl::OUString("MediaType"), aAny );


                uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
                    comphelper::getProcessServiceFactory();
                OSL_ENSURE( xServiceFactory.is(),
                            "XMLReader::Read: got no service manager" );
                if( !xServiceFactory.is() )
                {
                    // Throw an exception ?
                }

                    uno::Reference < XInterface > xWriter (xServiceFactory->createInstance(
                        OUString("com.sun.star.xml.sax.Writer")));
                    OSL_ENSURE(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
                uno::Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *xStrm );
                    uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
                    xSrc->setOutputStream(xOut);

                    uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

                    SvXMLExceptionListExport aExp( xServiceFactory, rLst, sStrmName, xHandler );

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
        uno::Reference< lang::XMultiServiceFactory > xServiceFactory = comphelper::getProcessServiceFactory();
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aXMLWordListName;
        aParserInput.aInputStream = xStrm->getInputStream();

        // get parser
        uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);
        RTL_LOGFILE_PRODUCT_CONTEXT( aLog, "AutoCorrect Import" );
        uno::Reference< xml::sax::XDocumentHandler > xFilter = new SvXMLAutoCorrectImport( xServiceFactory, pAutocorr_List, rAutoCorrect, xStg );

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
    String* pNew = new String( rNew );
    if( rNew.Len() && GetCplSttExceptList()->insert( pNew ).second )
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
    else
        delete pNew, pNew = 0;
    return 0 != pNew;
}

sal_Bool SvxAutoCorrectLanguageLists::AddToWrdSttExceptList(const String& rNew)
{
    String* pNew = new String( rNew );
    SvStringsISortDtor* pExceptList = LoadWrdSttExceptList();
    if( rNew.Len() && pExceptList && pExceptList->insert( pNew ).second )
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
    else
        delete pNew, pNew = 0;
    return 0 != pNew;
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
            aDest.SetExtension ( rtl::OUString("bak") );
            bConvert = sal_True;
        }
        bCopy = sal_True;
    }
    else if ( SotStorage::IsOLEStorage ( sUserAutoCorrFile ) )
    {
        aSource = INetURLObject ( sUserAutoCorrFile );
        aDest = INetURLObject ( sUserAutoCorrFile );
        aDest.SetExtension ( rtl::OUString("bak") );
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
            String sWord        ( RTL_CONSTASCII_USTRINGPARAM ( pImplWrdStt_ExcptLstStr ) );
            String sSentence    ( RTL_CONSTASCII_USTRINGPARAM ( pImplCplStt_ExcptLstStr ) );
            String sXMLWord     ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplWrdStt_ExcptLstStr ) );
            String sXMLSentence ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplCplStt_ExcptLstStr ) );
            SvStringsISortDtor  *pTmpWordList = NULL;

            if (xSrcStg->IsContained( sXMLWord ) )
                LoadXMLExceptList_Imp( pTmpWordList, pXMLImplWrdStt_ExcptLstStr, xSrcStg );

            if (pTmpWordList)
            {
                SaveExceptList_Imp( *pTmpWordList, pXMLImplWrdStt_ExcptLstStr, xDstStg, sal_True );
                pTmpWordList->DeleteAndDestroyAll();
                pTmpWordList = NULL;
            }


            if (xSrcStg->IsContained( sXMLSentence ) )
                LoadXMLExceptList_Imp( pTmpWordList, pXMLImplCplStt_ExcptLstStr, xSrcStg );

            if (pTmpWordList)
            {
                SaveExceptList_Imp( *pTmpWordList, pXMLImplCplStt_ExcptLstStr, xDstStg, sal_True );
                pTmpWordList->DeleteAndDestroyAll();
            }

            GetAutocorrWordList();
            MakeBlocklist_Imp( *xDstStg );
            sShareAutoCorrFile = sUserAutoCorrFile;
            xDstStg = 0;
            try
            {
                ::ucbhelper::Content aContent ( aDest.GetMainURL( INetURLObject::DECODE_TO_IURI ), uno::Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                aContent.executeCommand ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "delete" ) ), makeAny ( sal_Bool (sal_True ) ) );
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
            String aPropName( rtl::OUString( "MediaType" ) );
            OUString aMime( "text/xml" );
            uno::Any aAny;
            aAny <<= aMime;
            refList->SetProperty( aPropName, aAny );

            uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
            OSL_ENSURE( xServiceFactory.is(),
                        "XMLReader::Read: got no service manager" );
            if( !xServiceFactory.is() )
            {
                // Throw an exception ?
            }

                uno::Reference < XInterface > xWriter (xServiceFactory->createInstance(
                    OUString("com.sun.star.xml.sax.Writer")));
                OSL_ENSURE(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
            uno::Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *refList );
                uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
                xSrc->setOutputStream(xOut);

                uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

                SvXMLAutoCorrectExport aExp( xServiceFactory, pAutocorr_List, sStrmName, xHandler );

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
            SvxAutocorrWordList::iterator iterator = pAutocorr_List->find( &aWordToDelete );
            if( iterator != pAutocorr_List->end() )
            {
                SvxAutocorrWord* pFoundEntry = *iterator;
                if( !pFoundEntry->IsTextOnly() )
                {
                    String aName( aWordToDelete.GetShort() );
                    if (xStorage->IsOLEStorage())
                        EncryptBlockName_Imp( aName );
                    else
                        GeneratePackageName ( aWordToDelete.GetShort(), aName );

                    if( xStorage->IsContained( aName ) )
                    {
                        xStorage->Remove( aName );
                        bRet = xStorage->Commit();
                    }
                }
                // Update the word list
                delete pFoundEntry;
                pAutocorr_List->erase( iterator );
            }
        }

        for ( sal_uInt32 i=0; i < aNewEntries.size(); i++ )
        {
            SvxAutocorrWord* aWordToAdd = new SvxAutocorrWord( aNewEntries[i].GetShort(), aNewEntries[i].GetLong(), sal_True );
            SvxAutocorrWordList::iterator iterator = pAutocorr_List->find( aWordToAdd );
            if( iterator != pAutocorr_List->end() )
            {
                if( !(*iterator)->IsTextOnly() )
                {
                    // Still have to remove the Storage
                    String sStorageName( aWordToAdd->GetShort() );
                    if (xStorage->IsOLEStorage())
                    {
                        EncryptBlockName_Imp( sStorageName );
                    }
                    else
                    {
                        GeneratePackageName ( aWordToAdd->GetShort(), sStorageName);
                    }

                    if( xStorage->IsContained( sStorageName ) )
                        xStorage->Remove( sStorageName );
                }
                delete *iterator;
                pAutocorr_List->erase( iterator );
            }
            bRet = pAutocorr_List->insert( aWordToAdd ).second;

            if ( !bRet )
            {
                delete aWordToAdd;
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
        SvxAutocorrWordList::iterator it = pAutocorr_List->find( pNew );
        if( it != pAutocorr_List->end() )
        {
            if( !(*it)->IsTextOnly() )
            {
                // Still have to remove the Storage
                String sStgNm( rShort );
                if (xStg->IsOLEStorage())
                    EncryptBlockName_Imp( sStgNm );
                else
                    GeneratePackageName ( rShort, sStgNm);

                if( xStg->IsContained( sStgNm ) )
                    xStg->Remove( sStgNm );
            }
            delete *it;
            pAutocorr_List->erase( it );
        }

        if( pAutocorr_List->insert( pNew ).second )
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
    String sLong;
    try
    {
        uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetStorageFromURL( sUserAutoCorrFile, embed::ElementModes::READWRITE );
        bRet = rAutoCorrect.PutText( xStg, sUserAutoCorrFile, rShort, rShell, sLong );
        xStg = 0;

        // Update the word list
        if( bRet )
        {
            SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, sLong, sal_False );
            if( pAutocorr_List->insert( pNew ).second )
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
        SvxAutocorrWordList::iterator it = pAutocorr_List->find( &aTmp );
        if( it != pAutocorr_List->end() )
        {
            SvxAutocorrWord* pFnd = *it;
            if( !pFnd->IsTextOnly() )
            {
                String aName( rShort );
                if (xStg->IsOLEStorage())
                    EncryptBlockName_Imp( aName );
                else
                    GeneratePackageName ( rShort, aName );
                if( xStg->IsContained( aName ) )
                {
                    xStg->Remove( aName );
                    bRet = xStg->Commit();
                }

            }
            // Update the word list
            delete pFnd;
            pAutocorr_List->erase( it );
            MakeBlocklist_Imp( *xStg );
            xStg = 0;
        }
        else
            bRet = sal_False;
    }
    return bRet;
}

SvxAutocorrWordList::~SvxAutocorrWordList()
{
    DeleteAndDestroyAll();
}

void SvxAutocorrWordList::DeleteAndDestroyAll()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
    clear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
