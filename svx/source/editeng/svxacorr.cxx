/*************************************************************************
 *
 *  $RCSfile: svxacorr.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: mtg $ $Date: 2001-02-16 09:57:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define ITEMID_UNDERLINE    0
#define ITEMID_WEIGHT       0
#define ITEMID_ESCAPEMENT   0
#define ITEMID_CHARSETCOLOR 0
#define ITEMID_COLOR        0

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _STORINFO_HXX //autogen
#include <sot/storinfo.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFX_DOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
// fuer die Sort-String-Arrays aus dem SVMEM.HXX
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svtools/fstathelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HDL_
#include <com/sun/star/i18n/UnicodeType.hdl>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif

#include "udlnitem.hxx"
#include "wghtitem.hxx"
#include "escpitem.hxx"
#include "svxacorr.hxx"
#include "unolingu.hxx"

#ifndef _SVX_HELPID_HRC
#include <helpid.hrc>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _SV_XMLAUTOCORRECTIMPORT_HXX
#include "SvXMLAutoCorrectImport.hxx"
#endif
#ifndef _SV_XMLAUTOCORRECTEXPORT_HXX
#include "SvXMLAutoCorrectExport.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

const int C_NONE                = 0x00;
const int C_FULL_STOP           = 0x01;
const int C_EXCLAMATION_MARK    = 0x02;
const int C_QUESTION_MARK       = 0x04;

static const sal_Char pImplWrdStt_ExcptLstStr[]    = "WordExceptList";
static const sal_Char pImplCplStt_ExcptLstStr[]    = "SentenceExceptList";
static const sal_Char pImplAutocorr_ListStr[]      = "DocumentList";
static const sal_Char pXMLImplWrdStt_ExcptLstStr[] = "WordExceptList.xml";
static const sal_Char pXMLImplCplStt_ExcptLstStr[] = "SentenceExceptList.xml";
static const sal_Char pXMLImplAutocorr_ListStr[]   = "DocumentList.xml";

static const sal_Char
    /* auch bei diesen Anfaengen - Klammern auf und alle Arten von Anf.Zei. */
    sImplSttSkipChars[] = "\"\'([{\x83\x84\x89\x91\x92\x93\x94",
    /* auch bei diesen Ende - Klammern auf und alle Arten von Anf.Zei. */
    sImplEndSkipChars[] = "\"\')]}\x83\x84\x89\x91\x92\x93\x94";

// diese Zeichen sind in Worten erlaubt: (fuer FnCptlSttSntnc)
static const sal_Char sImplWordChars[] = "-'";

void EncryptBlockName_Imp( String& rName );
void DecryptBlockName_Imp( String& rName );


// FileVersions Nummern fuer die Ersetzungs-/Ausnahmelisten getrennt
#define WORDLIST_VERSION_358    1
#define EXEPTLIST_VERSION_358   0


_SV_IMPL_SORTAR_ALG( SvxAutocorrWordList, SvxAutocorrWordPtr )
TYPEINIT0(SvxAutoCorrect)

typedef SvxAutoCorrectLanguageLists* SvxAutoCorrectLanguageListsPtr;
DECLARE_TABLE( SvxAutoCorrLanguageTable_Impl,  SvxAutoCorrectLanguageListsPtr);

DECLARE_TABLE( SvxAutoCorrLastFileAskTable_Impl, long );


inline int IsWordDelim( const sal_Unicode c )
{
    return ' ' == c || '\t' == c || 0x0a == c ||
            0xA0 == c || 0x2011 == c || 0x1 == c;
}

inline int IsLowerLetter( sal_Int32 nCharType )
{
    return CharClass::isLetterType( nCharType ) &&
            0 == ( ::com::sun::star::i18n::KCharacterType::UPPER & nCharType);
}
inline int IsUpperLetter( sal_Int32 nCharType )
{
    return CharClass::isLetterType( nCharType ) &&
            0 == ( ::com::sun::star::i18n::KCharacterType::LOWER & nCharType);
}

BOOL lcl_IsSymbolChar( CharClass& rCC, const String& rTxt,
                           xub_StrLen nStt, xub_StrLen nEnd )
{
    for( ; nStt < nEnd; ++nStt )
    {
#ifdef DEBUG
        sal_Int32 nCharType = rCC.getCharacterType( rTxt, nStt );
        sal_Int32 nChType = rCC.getType( rTxt, nStt );
#endif
        if( ::com::sun::star::i18n::UnicodeType::PRIVATE_USE ==
                rCC.getType( rTxt, nStt ))
            return TRUE;
    }
    return FALSE;
}


static BOOL lcl_IsInAsciiArr( const sal_Char* pArr, const sal_Unicode c )
{
    BOOL bRet = FALSE;
    for( ; *pArr; ++pArr )
        if( *pArr == c )
        {
            bRet = TRUE;
            break;
        }
    return bRet;
}

SvxAutoCorrDoc::~SvxAutoCorrDoc()
{
}


    // wird nach dem austauschen der Zeichen von den Funktionen
    //  - FnCptlSttWrd
    //  - FnCptlSttSntnc
    // gerufen. Dann koennen die Worte ggfs. in die Ausnahmelisten
    // aufgenommen werden.
void SvxAutoCorrDoc::SaveCpltSttWord( ULONG, xub_StrLen, const String&,
                                        sal_Unicode )
{
}

LanguageType SvxAutoCorrDoc::GetLanguage( xub_StrLen , BOOL ) const
{
    return LANGUAGE_SYSTEM;
}


void SvxAutocorrWordList::DeleteAndDestroy( USHORT nP, USHORT nL )
{
    if( nL )
    {
        DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
        for( USHORT n=nP; n < nP + nL; n++ )
            delete *((SvxAutocorrWordPtr*)pData+n);
        SvPtrarr::Remove( nP, nL );
    }
}


BOOL SvxAutocorrWordList::Seek_Entry( const SvxAutocorrWordPtr aE, USHORT* pP ) const
{
    register USHORT nO  = SvxAutocorrWordList_SAR::Count(),
            nM,
            nU = 0;
    const International& rInter = Application::GetAppInternational();
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = rInter.Compare( aE->GetShort(),
                        (*((SvxAutocorrWordPtr*)pData + nM))->GetShort() );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( COMPARE_GREATER == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}

/* -----------------18.11.98 15:28-------------------
 *
 * --------------------------------------------------*/
void lcl_ClearTable(SvxAutoCorrLanguageTable_Impl& rLangTable)
{
    SvxAutoCorrectLanguageListsPtr pLists = rLangTable.Last();
    while(pLists)
    {
        delete pLists;
        pLists = rLangTable.Prev();
    }
    rLangTable.Clear();
}

/* -----------------19.11.98 10:15-------------------
 *
 * --------------------------------------------------*/
long SvxAutoCorrect::GetDefaultFlags()
{
    long nRet = Autocorrect
                    | CptlSttSntnc
                    | CptlSttWrd
                    | ChgFractionSymbol
                    | ChgOrdinalNumber
                    | ChgToEnEmDash
                    | ChgWeightUnderl
                    | SetINetAttr
                    | ChgQuotes
                    | SaveWordCplSttLst
                    | SaveWordWrdSttLst;
    LanguageType eLang = Application::GetAppInternational().GetLanguage();
    if( LANGUAGE_SYSTEM == eLang )
        eLang = System::GetLanguage();
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
    cStartSQuote( 0 ), cEndSQuote( 0 ), cStartDQuote( 0 ), cEndDQuote( 0 ),
    pLangTable( new SvxAutoCorrLanguageTable_Impl ),
    pLastFileTable( new SvxAutoCorrLastFileAskTable_Impl ),
    pCharClass( 0 )
{
    nFlags = SvxAutoCorrect::GetDefaultFlags();

    c1Div2 = ByteString::ConvertToUnicode( '\xBD', RTL_TEXTENCODING_MS_1252 );
    c1Div4 = ByteString::ConvertToUnicode( '\xBC', RTL_TEXTENCODING_MS_1252 );
    c3Div4 = ByteString::ConvertToUnicode( '\xBE', RTL_TEXTENCODING_MS_1252 );
    cEmDash = ByteString::ConvertToUnicode( '\x97', RTL_TEXTENCODING_MS_1252 );
    cEnDash = ByteString::ConvertToUnicode( '\x96', RTL_TEXTENCODING_MS_1252 );
}

SvxAutoCorrect::SvxAutoCorrect( const SvxAutoCorrect& rCpy )
    : nFlags( rCpy.nFlags & ~(ChgWordLstLoad|CplSttLstLoad|WrdSttLstLoad)),
    aSwFlags( rCpy.aSwFlags ),
/* Die Sprachentabelle wird neu aufgebaut, da sie im Dtor von rCpy abgeraeumt wird!
 */
    sShareAutoCorrFile( rCpy.sShareAutoCorrFile ),
    sUserAutoCorrFile( rCpy.sUserAutoCorrFile ),
    cStartSQuote( rCpy.cStartSQuote ), cEndSQuote( rCpy.cEndSQuote ),
    cStartDQuote( rCpy.cStartDQuote ), cEndDQuote( rCpy.cEndDQuote ),
    c1Div2( rCpy.c1Div2 ), c1Div4( rCpy.c1Div4 ), c3Div4( rCpy.c3Div4 ),
    cEmDash( rCpy.cEmDash ), cEnDash( rCpy.cEnDash ),
    pLangTable( new SvxAutoCorrLanguageTable_Impl ),
    pLastFileTable( new SvxAutoCorrLastFileAskTable_Impl ),
    pCharClass( 0 )
{
}


SvxAutoCorrect::~SvxAutoCorrect()
{
    lcl_ClearTable(*pLangTable);
    delete pLangTable;
    delete pLastFileTable;
    delete pCharClass;
}

void SvxAutoCorrect::_GetCharClass( LanguageType eLang )
{
    delete pCharClass;
    pCharClass = new CharClass( SvxCreateLocale( eLang ));
    eCharClassLang = eLang;
}

void SvxAutoCorrect::SetAutoCorrFlag( long nFlag, BOOL bOn )
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


    // Zwei Grossbuchstaben am Wort-Anfang ??
BOOL SvxAutoCorrect::FnCptlSttWrd( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                    xub_StrLen nSttPos, xub_StrLen nEndPos,
                                    LanguageType eLang )
{
    BOOL bRet = FALSE;
    CharClass& rCC = GetCharClass( eLang );

    // loesche alle nicht alpanum. Zeichen am Wortanfang/-ende und
    // teste dann ( erkennt: "(min.", "/min.", usw.)
    for( ; nSttPos < nEndPos; ++nSttPos )
        if( rCC.isLetterNumeric( rTxt, nSttPos ))
            break;
    for( ; nSttPos < nEndPos; --nEndPos )
        if( rCC.isLetterNumeric( rTxt, nEndPos - 1 ))
            break;

    // Zwei Grossbuchstaben am Wort-Anfang ??
    if( nSttPos+2 < nEndPos &&
        IsUpperLetter( rCC.getCharacterType( rTxt, nSttPos )) &&
        IsUpperLetter( rCC.getCharacterType( rTxt, ++nSttPos )) &&
        // ist das 3. Zeichen ein klein geschiebenes Alpha-Zeichen
        IsLowerLetter( rCC.getCharacterType( rTxt, nSttPos +1 )) &&
        // keine Sonder-Attribute ersetzen
        0x1 != rTxt.GetChar( nSttPos ) && 0x2 != rTxt.GetChar( nSttPos ))
    {
        // teste ob das Wort in einer Ausnahmeliste steht
        String sWord( rTxt.Copy( nSttPos - 1, nEndPos - nSttPos + 1 ));
        if( !FindInWrdSttExceptList(eLang, sWord) )
        {
            sal_Unicode cSave = rTxt.GetChar( nSttPos );
            String sChar( cSave );
            rCC.toLower( sChar );
            if( rDoc.Replace( nSttPos, sChar ))
            {
                if( SaveWordWrdSttLst & nFlags )
                    rDoc.SaveCpltSttWord( CptlSttWrd, nSttPos, sWord, cSave );
                bRet = TRUE;
            }
        }
    }
    return bRet;
}


BOOL SvxAutoCorrect::FnChgFractionSymbol(
                                SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nSttPos, xub_StrLen nEndPos )
{
    sal_Unicode cChar = 0;

    for( ; nSttPos < nEndPos; ++nSttPos )
        if( !lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nSttPos ) ))
            break;
    for( ; nSttPos < nEndPos; --nEndPos )
        if( !lcl_IsInAsciiArr( sImplEndSkipChars, rTxt.GetChar( nEndPos - 1 ) ))
            break;

    // 1/2, 1/4, ... ersetzen durch das entsprechende Zeichen vom Font
    if( 3 == nEndPos - nSttPos && '/' == rTxt.GetChar( nSttPos+1 ))
    {
        switch( ( rTxt.GetChar( nSttPos )) * 256 + rTxt.GetChar( nEndPos-1 ))
        {
        case '1' * 256 + '2':       cChar = c1Div2;     break;
        case '1' * 256 + '4':       cChar = c1Div4;     break;
        case '3' * 256 + '4':       cChar = c3Div4;     break;
        }

        if( cChar )
        {
            // also austauschen:
            rDoc.Delete( nSttPos+1, nEndPos );
            rDoc.Replace( nSttPos, cChar );
        }
    }
    return 0 != cChar;
}


BOOL SvxAutoCorrect::FnChgOrdinalNumber(
                                SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang )
{
// 1st, 2nd, 3rd, 4 - 0th
// 201th oder 201st
// 12th oder 12nd
    CharClass& rCC = GetCharClass( eLang );
    BOOL bChg = FALSE;

    for( ; nSttPos < nEndPos; ++nSttPos )
        if( !lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nSttPos ) ))
            break;
    for( ; nSttPos < nEndPos; --nEndPos )
        if( !lcl_IsInAsciiArr( sImplEndSkipChars, rTxt.GetChar( nEndPos - 1 ) ))
            break;

    if( 2 < nEndPos - nSttPos &&
        rCC.isDigit( rTxt, nEndPos - 3 ) )
    {
        static sal_Char __READONLY_DATA
            sAll[]      = "th",         /* rest */
            sFirst[]    = "st",         /* 1 */
            sSecond[]   = "nd",         /* 2 */
            sThird[]    = "rd";         /* 3 */
        static const sal_Char* __READONLY_DATA aNumberTab[ 4 ] =
        {
            sAll, sFirst, sSecond, sThird
        };

        sal_Unicode c = rTxt.GetChar( nEndPos - 3 );
        if( ( c -= '0' ) > 3 )
            c = 0;

        bChg = ( ((sal_Unicode)*((aNumberTab[ c ])+0)) ==
                                        rTxt.GetChar( nEndPos - 2 ) &&
                 ((sal_Unicode)*((aNumberTab[ c ])+1)) ==
                                         rTxt.GetChar( nEndPos - 1 )) ||
               ( 3 < nEndPos - nSttPos &&
                ( ((sal_Unicode)*(sAll+0)) == rTxt.GetChar( nEndPos - 2 ) &&
                  ((sal_Unicode)*(sAll+1)) == rTxt.GetChar( nEndPos - 1 )));

        if( bChg )
        {
            // dann pruefe mal, ob alle bis zum Start alle Zahlen sind
            for( xub_StrLen n = nEndPos - 3; nSttPos < n; )
                if( !rCC.isDigit( rTxt, --n ) )
                {
                    bChg = !rCC.isLetter( rTxt, n );
                    break;
                }

            if( bChg )      // dann setze mal das Escapement Attribut
            {
                SvxEscapementItem aSvxEscapementItem( DFLT_ESC_AUTO_SUPER,
                                                    DFLT_ESC_PROP );
                rDoc.SetAttr( nEndPos - 2, nEndPos,
                                SID_ATTR_CHAR_ESCAPEMENT,
                                aSvxEscapementItem);
            }
        }

    }
    return bChg;
}


BOOL SvxAutoCorrect::FnChgToEnEmDash(
                                SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang )
{
    BOOL bRet = FALSE;
    CharClass& rCC = GetCharClass( eLang );

    // ersetze " - " oder " --" durch "enDash"
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
                if( rCC.isLetterNumeric( cCh ) )
                {
                    for( n = nSttPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt.GetChar( --n ))); )
                        ;

                    // found: "[A-z0-9][<AnyEndChars>] --[<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( cCh ))
                    {
                        rDoc.Delete( nSttPos, nSttPos + 2 );
                        rDoc.Insert( nSttPos, cEnDash );
                        bRet = TRUE;
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
                if( rCC.isLetterNumeric( cCh ) )
                {
                    cCh = ' ';
                    for( n = nTmpPos-1; n && lcl_IsInAsciiArr(
                            sImplEndSkipChars,(cCh = rTxt.GetChar( --n ))); )
                            ;
                    // found: "[A-z0-9][<AnyEndChars>] - [<AnySttChars>][A-z0-9]
                    if( rCC.isLetterNumeric( cCh ))
                    {
                        rDoc.Delete( nTmpPos, nTmpPos + nLen );
                        rDoc.Insert( nTmpPos, cEnDash );
                        bRet = TRUE;
                    }
                }
            }
        }
    }

    // ersetze [A-z0-9]--[A-z0-9] durch "emDash"
    if( cEmDash && 4 <= nEndPos - nSttPos )
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
            nSttPos += nFndPos;
            rDoc.Delete( nSttPos, nSttPos + 2 );
            rDoc.Insert( nSttPos, cEmDash );
            bRet = TRUE;
        }
    }
    return bRet;
}


BOOL SvxAutoCorrect::FnSetINetAttr( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                    xub_StrLen nSttPos, xub_StrLen nEndPos,
                                    LanguageType eLang )
{
#ifdef SKIP_QUOTES
    // 1. Schritt: alle NICHT alphanumerischen Zeichen am Anfang entfernen
    //              und alle festgelegten Zeichen am Ende (laut Netscape!)
    String sQuotes;
    if( !GetEndDoubleQuote() || !GetEndSingleQuote() )
    {
        // dann ueber die Language das richtige Zeichen heraussuchen
        if( LANGUAGE_NONE != eLang )
        {
            const International& rInter = Application::GetAppInternational();
            International* pIntl = (International*)&rInter;

            if( eLang != pIntl->GetLanguage() && LANGUAGE_SYSTEM != eLang )
                pIntl = new International( eLang );

            sQuotes = GetEndDoubleQuote()
                        ? GetEndDoubleQuote()
                        : pIntl->GetDoubleQuotationMarkEndChar();

            sQuotes += GetEndSingleQuote()
                        ? GetEndSingleQuote()
                        : pIntl->GetDoubleQuotationMarkEndChar();

            if( pIntl != &rInter )
                delete pIntl;
        }
    }
    else
        ( sQuotes = GetEndDoubleQuote() ) += GetEndSingleQuote();

    String sURL( URIHelper::FindFirstURLInText( rTxt, nSttPos, nEndPos,
                                GetCharClass( eLang ), &sQuotes ));
#else
    String sURL( URIHelper::FindFirstURLInText( rTxt, nSttPos, nEndPos,
                                                GetCharClass( eLang ) ));
#endif
    BOOL bRet = 0 != sURL.Len();
    if( bRet )          // also Attribut setzen:
        rDoc.SetINetAttr( nSttPos, nEndPos, sURL );
    return bRet;
}


BOOL SvxAutoCorrect::FnChgWeightUnderl( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                        xub_StrLen nSttPos, xub_StrLen nEndPos,
                                        LanguageType eLang )
{
    // Bedingung:
    //  Am Anfang:  _ oder * hinter Space mit nachfolgenden !Space
    //  Am Ende:    _ oder * vor Space (Worttrenner?)

    sal_Unicode c, cInsChar = rTxt.GetChar( nEndPos );  // unterstreichen oder fett
    if( ++nEndPos != rTxt.Len() &&
        !IsWordDelim( rTxt.GetChar( nEndPos ) ) )
        return FALSE;

    --nEndPos;

    BOOL bAlphaNum = FALSE;
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
                    // Bedingung ist nicht erfuellt, also abbrechen
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
        // ueber den gefundenen Bereich das Attribut aufspannen und
        // das gefunde und am Ende stehende Zeichen loeschen
        if( '*' == cInsChar )           // Fett
        {
            SvxWeightItem aSvxWeightItem( WEIGHT_BOLD );
            rDoc.SetAttr( nFndPos + 1, nEndPos,
                            SID_ATTR_CHAR_WEIGHT,
                            aSvxWeightItem);
        }
        else                            // unterstrichen
        {
            SvxUnderlineItem aSvxUnderlineItem( UNDERLINE_SINGLE );
            rDoc.SetAttr( nFndPos + 1, nEndPos,
                            SID_ATTR_CHAR_UNDERLINE,
                            aSvxUnderlineItem);
        }
        rDoc.Delete( nEndPos, nEndPos + 1 );
        rDoc.Delete( nFndPos, nFndPos + 1 );
    }

    return STRING_NOTFOUND != nFndPos;
}


BOOL SvxAutoCorrect::FnCptlSttSntnc( SvxAutoCorrDoc& rDoc,
                                    const String& rTxt, BOOL bNormalPos,
                                    xub_StrLen nSttPos, xub_StrLen nEndPos,
                                    LanguageType eLang )
{
    // Grossbuchstabe am Satz-Anfang ??
    if( !rTxt.Len() )
        return FALSE;

    CharClass& rCC = GetCharClass( eLang );
    String aText( rTxt );
    const sal_Unicode *pStart = aText.GetBuffer(),
                      *pStr = pStart + nEndPos,
                      *pWordStt = 0,
                      *pDelim;

    BOOL bAtStart = FALSE, bPrevPara = FALSE;
    do {
        --pStr;
        if( rCC.isLetter( aText, pStr - pStart ) )
        {
            if( !pWordStt )
                pDelim = pStr+1;
            pWordStt = pStr;
        }
        else if( pWordStt && !rCC.isDigit( aText, pStr - pStart ))
        {
            if( lcl_IsInAsciiArr( sImplWordChars, *pStr ) &&
                pWordStt - 1 == pStr &&
                (long)(pStart + 1) < (long)pStr &&
                rCC.isLetter( aText, pStr-1 - pStart ) )
                pWordStt = --pStr;
            else
                break;
        }
    } while( 0 == ( bAtStart = (pStart == pStr)) );


    if( !pWordStt ||
        rCC.isDigit( aText, pStr - pStart ) ||
        IsUpperLetter( rCC.getCharacterType( aText, pWordStt - pStart )) ||
        0x1 == *pWordStt || 0x2 == *pWordStt )
        return FALSE;       // kein zu ersetzendes Zeichen, oder schon ok

    // JP 27.10.97: wenn das Wort weniger als 3 Zeichen hat und der Trenner
    //              ein "Num"-Trenner ist, dann nicht ersetzen!
    //              Damit wird ein "a.",  "a)", "a-a" nicht ersetzt!
    if( *pDelim && 2 >= pDelim - pWordStt &&
        lcl_IsInAsciiArr( ".-)>", *pDelim ) )
        return FALSE;

    if( !bAtStart ) // noch kein Absatz Anfang ?
    {
        if( !IsWordDelim( *pStr ))
            return FALSE;       // kein gueltiger Trenner -> keine Ersetzung

        while( 0 == ( bAtStart = (pStart == pStr--) ) && IsWordDelim( *pStr ))
            ;
    }

    if( bAtStart )  // am Absatz Anfang ?
    {
        // Ueberpruefe den vorherigen Absatz, wenn es diesen gibt.
        // Wenn ja, dann pruefe auf SatzTrenner am Ende.
        const String* pPrevPara = rDoc.GetPrevPara( bNormalPos );
        if( !pPrevPara )
        {
            // gueltiger Trenner -> Ersetze
            String sChar( *pWordStt );
            rCC.toUpper( sChar );
            return rDoc.Replace( xub_StrLen( pWordStt - pStart ), sChar );
        }

        aText = *pPrevPara;
        bPrevPara = TRUE;
        bAtStart = FALSE;
        pStart = aText.GetBuffer();
        pStr = pStart + aText.Len();

        do {            // alle Blanks ueberlesen
            --pStr;
            if( !IsWordDelim( *pStr ))
                break;
        } while( 0 == ( bAtStart = (pStart == pStr)) );

        if( bAtStart )
            return FALSE;       // kein gueltiger Trenner -> keine Ersetzung
    }

    // bis hierhier wurde [ \t]+[A-Z0-9]+ gefunden. Test jetzt auf den
    // Satztrenner. Es koennen alle 3 vorkommen, aber nicht mehrfach !!
    const sal_Unicode* pExceptStt = 0;
    if( !bAtStart )
    {
        BOOL bWeiter = TRUE;
        int nFlag = C_NONE;
        do {
            switch( *pStr )
            {
            case '.':
                {
                    if( nFlag & C_FULL_STOP )
                        return FALSE;       // kein gueltiger Trenner -> keine Ersetzung
                    nFlag |= C_FULL_STOP;
                    pExceptStt = pStr;
                }
                break;
            case '!':
                {
                    if( nFlag & C_EXCLAMATION_MARK )
                        return FALSE;   // kein gueltiger Trenner -> keine Ersetzung
                    nFlag |= C_EXCLAMATION_MARK;
                }
                break;
            case '?':
                {
                    if( nFlag & C_QUESTION_MARK)
                        return FALSE;       // kein gueltiger Trenner -> keine Ersetzung
                    nFlag |= C_QUESTION_MARK;
                }
                break;
            default:
                if( !nFlag )
                    return FALSE;       // kein gueltiger Trenner -> keine Ersetzung
                else
                    bWeiter = FALSE;
                break;
            }

            if( bWeiter && pStr-- == pStart )
            {
// !!! wenn am Anfang, dann nie ersetzen.
//              if( !nFlag )
                    return FALSE;       // kein gueltiger Trenner -> keine Ersetzung
//              ++pStr;
//              break;      // Schleife beenden
            }
        } while( bWeiter );
        if( C_FULL_STOP != nFlag )
            pExceptStt = 0;
    }

    if( 2 > ( pStr - pStart ) )
        return FALSE;

    if( !rCC.isLetterNumeric( aText, pStr-- - pStart ) )
    {
        BOOL bValid = FALSE, bAlphaFnd = FALSE;
        const sal_Unicode* pTmpStr = pStr;
        while( !bValid )
        {
            if( rCC.isDigit( aText, pTmpStr - pStart ) )
            {
                bValid = TRUE;
                pStr = pTmpStr - 1;
            }
            else if( rCC.isLetter( aText, pTmpStr - pStart ) )
            {
                if( bAlphaFnd )
                {
                    bValid = TRUE;
                    pStr = pTmpStr;
                }
                else
                    bAlphaFnd = TRUE;
            }
            else if( bAlphaFnd || IsWordDelim( *pTmpStr ) )
                break;

            if( pTmpStr == pStart )
                break;

            --pTmpStr;
        }

        if( !bValid )
            return FALSE;       // kein gueltiger Trenner -> keine Ersetzung
    }

    BOOL bNumericOnly = '0' <= *(pStr+1) && *(pStr+1) <= '9';

    // suche den Anfang vom Wort
    while( !IsWordDelim( *pStr ))
    {
        if( bNumericOnly && rCC.isLetter( aText, pStr - pStart ))
            bNumericOnly = FALSE;

        if( pStart == pStr )
            break;

        --pStr;
    }

    if( bNumericOnly )      // besteht nur aus Zahlen, dann nicht
        return FALSE;

    if( IsWordDelim( *pStr ))
        ++pStr;

    String sWord;

    // ueberpruefe anhand der Exceptionliste
    if( pExceptStt )
    {
        sWord = String( pStr, pExceptStt - pStr + 1 );
        if( FindInCplSttExceptList(eLang, sWord) )
            return FALSE;

        // loesche alle nicht alpanum. Zeichen am Wortanfang/-ende und
        // teste dann noch mal ( erkennt: "(min.", "/min.", usw.)
        String sTmp( sWord );
        while( sTmp.Len() &&
                !rCC.isLetterNumeric( sTmp, 0 ) )
            sTmp.Erase( 0, 1 );

        // alle hinteren nicht alphanumerische Zeichen bis auf das
        // Letzte entfernen
        xub_StrLen nLen = sTmp.Len();
        while( nLen && !rCC.isLetterNumeric( sTmp, nLen-1 ) )
            --nLen;
        if( nLen + 1 < sTmp.Len() )
            sTmp.Erase( nLen + 1 );

        if( sTmp.Len() && sTmp.Len() != sWord.Len() &&
            FindInCplSttExceptList(eLang, sTmp))
            return FALSE;

        if(FindInCplSttExceptList(eLang, sWord, TRUE))
            return FALSE;
    }

    // Ok, dann ersetze mal
    sal_Unicode cSave = *pWordStt;
    nSttPos = pWordStt - rTxt.GetBuffer();
    String sChar( cSave );
    rCC.toUpper( sChar );
    BOOL bRet = rDoc.Replace( nSttPos, sChar );

    // das Wort will vielleicht jemand haben
    if( bRet && SaveWordCplSttLst & nFlags )
        rDoc.SaveCpltSttWord( CptlSttSntnc, nSttPos, sWord, cSave );

    return bRet;
}

sal_Unicode SvxAutoCorrect::_GetQuote( sal_Unicode cInsChar, BOOL bSttQuote,
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
        // dann ueber die Language das richtige Zeichen heraussuchen
        if( LANGUAGE_NONE == eLang )
            cRet = cInsChar;
        else
        {
            International* pIntl = (International*)&Application::GetAppInternational();

            if( eLang != pIntl->GetLanguage() && LANGUAGE_SYSTEM != eLang )
                pIntl = new International( eLang );

            cRet = bSttQuote
                ? ( '\"' == cInsChar
                    ? pIntl->GetDoubleQuotationMarkStartChar()
                    : pIntl->GetQuotationMarkStartChar() )
                : ( '\"' == cInsChar
                    ? pIntl->GetDoubleQuotationMarkEndChar()
                    : pIntl->GetQuotationMarkEndChar() );

            if( pIntl != &Application::GetAppInternational() )
                delete pIntl;
        }
    }
    return cRet;
}

void SvxAutoCorrect::InsertQuote( SvxAutoCorrDoc& rDoc, xub_StrLen nInsPos,
                                    sal_Unicode cInsChar, BOOL bSttQuote,
                                    BOOL bIns )
{
    LanguageType eLang = rDoc.GetLanguage( nInsPos, FALSE );
    sal_Unicode cRet = _GetQuote( cInsChar, bSttQuote, eLang );

    //JP 13.02.99: damit beim Undo das "einfuegte" Zeichen wieder erscheint,
    //              wird es erstmal eingefuegt und dann ueberschrieben
    String sChg( cInsChar );
    if( bIns )
        rDoc.Insert( nInsPos, sChg );
    else
        rDoc.Replace( nInsPos, sChg );

    //JP 13.08.97: Bug 42477 - bei doppelten Anfuehrungszeichen muss bei
    //              franzoesischer Sprache an Anfang ein Leerzeichen dahinter
    //              und am Ende ein Leerzeichen dahinter eingefuegt werden.
    sChg = cRet;

    if( '\"' == cInsChar )
    {
        if( LANGUAGE_SYSTEM == eLang )
            eLang = Application::GetAppInternational().GetLanguage();
        switch( eLang )
        {
        case LANGUAGE_FRENCH:
        case LANGUAGE_FRENCH_BELGIAN:
        case LANGUAGE_FRENCH_CANADIAN:
        case LANGUAGE_FRENCH_SWISS:
        case LANGUAGE_FRENCH_LUXEMBOURG:
            // JP 09.02.99: das zusaetzliche Zeichen immer per Insert einfuegen.
            //              Es ueberschreibt nichts!
            {
                String s( 0xA0 );       // UNICODE code for no break space
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
                                sal_Unicode cInsChar, BOOL bSttQuote )
{
    LanguageType eLang = rDoc.GetLanguage( nInsPos, FALSE );
    sal_Unicode cRet = _GetQuote( cInsChar, bSttQuote, eLang );

    String sRet( cRet );
    //JP 13.08.97: Bug 42477 - bei doppelten Anfuehrungszeichen muss bei
    //              franzoesischer Sprache an Anfang ein Leerzeichen dahinter
    //              und am Ende ein Leerzeichen dahinter eingefuegt werden.
    if( '\"' == cInsChar )
    {
        if( LANGUAGE_SYSTEM == eLang )
            eLang = Application::GetAppInternational().GetLanguage();
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

ULONG SvxAutoCorrect::AutoCorrect( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                    xub_StrLen nInsPos, sal_Unicode cChar,
                                    BOOL bInsert )
{
    ULONG nRet = 0;
    do{                                 // only for middle check loop !!
        if( cChar )
        {
            //JP 10.02.97: doppelte Spaces verhindern
            if( nInsPos && ' ' == cChar &&
                IsAutoCorrFlag( IngnoreDoubleSpace ) &&
                ' ' == rTxt.GetChar( nInsPos - 1 ) )
            {
                nRet = IngnoreDoubleSpace;
                break;
            }

            BOOL bSingle = '\'' == cChar;
            BOOL bIsReplaceQuote =
                        (IsAutoCorrFlag( ChgQuotes ) && ('\"' == cChar )) ||
                        (IsAutoCorrFlag( ChgSglQuotes ) && bSingle );
            if( bIsReplaceQuote )
            {
                sal_Unicode cPrev;
                BOOL bSttQuote = !nInsPos ||
                        IsWordDelim( ( cPrev = rTxt.GetChar( nInsPos-1 ))) ||
// os: #56034# - Warum kein schliessendes Anfuehrungszeichen nach dem Bindestrich?
//                      strchr( "-([{", cPrev ) ||
                        lcl_IsInAsciiArr( "([{", cPrev ) ||
                        ( cEmDash && cEmDash == cPrev ) ||
                        ( cEnDash && cEnDash == cPrev );

                InsertQuote( rDoc, nInsPos, cChar, bSttQuote, bInsert );
                nRet = bSingle ? ChgSglQuotes : ChgQuotes;
                break;
            }

            if( bInsert )
                rDoc.Insert( nInsPos, cChar );
            else
                rDoc.Replace( nInsPos, cChar );
        }

        if( !nInsPos )
            break;

        xub_StrLen nPos = nInsPos - 1;

        // Bug 19286: nur direkt hinter dem "Wort" aufsetzen
        if( IsWordDelim( rTxt.GetChar( nPos )))
            break;

        // automatisches Fett oder Unterstreichen setzen?
        if( '*' == cChar || '_' == cChar )
        {
            if( IsAutoCorrFlag( ChgWeightUnderl ) &&
                FnChgWeightUnderl( rDoc, rTxt, 0, nPos+1 ) )
                nRet = ChgWeightUnderl;
            break;
        }

        while( nPos && !IsWordDelim( rTxt.GetChar( --nPos )))
            ;

        // Absatz-Anfang oder ein Blank gefunden, suche nach dem Wort
        // Kuerzel im Auto
        xub_StrLen nCapLttrPos = nPos+1;        // auf das 1. Zeichen
        if( !nPos && !IsWordDelim( rTxt.GetChar( 0 )))
            --nCapLttrPos;          // Absatz Anfang und kein Blank !

        LanguageType eLang = rDoc.GetLanguage( nCapLttrPos, FALSE );
        if( LANGUAGE_SYSTEM == eLang )
            eLang = System::GetLanguage();
        CharClass& rCC = GetCharClass( eLang );

        // Bug 19285: Symbolzeichen nicht anfassen
        if( lcl_IsSymbolChar( rCC, rTxt, nCapLttrPos, nInsPos ))
            break;

        if( IsAutoCorrFlag( Autocorrect ) )
        {
            const String* pPara = 0;
            const String** ppPara = IsAutoCorrFlag(CptlSttSntnc) ? &pPara : 0;

            BOOL bChgWord = rDoc.ChgAutoCorrWord( nCapLttrPos, nInsPos,
                                                    *this, ppPara );
            if( !bChgWord )
            {
                // JP 16.06.98: dann versuche mal alle !AlphaNum. Zeichen los zu
                //              werden und teste dann nochmals
                //JP 22.04.99: Bug 63883 - entferne nur die "Klammern Start/-Anfaenge",
                //              alle anderen Zeichen muessen drin bleiben.
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
                    bChgWord = TRUE;
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

                    // Grossbuchstabe am Satz-Anfang ??
                    if( FnCptlSttSntnc( rDoc, *pPara, FALSE, nCapLttrPos, nEnd, eLang ) )
                        nRet |= CptlSttSntnc;
                }
                break;
            }
        }

        if( ( IsAutoCorrFlag( nRet = ChgFractionSymbol ) &&
                FnChgFractionSymbol( rDoc, rTxt, nCapLttrPos, nInsPos ) ) ||
            ( IsAutoCorrFlag( nRet = ChgOrdinalNumber ) &&
                FnChgOrdinalNumber( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) ||
            ( IsAutoCorrFlag( nRet = ChgToEnEmDash ) &&
                FnChgToEnEmDash( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) ||
            ( IsAutoCorrFlag( nRet = SetINetAttr ) &&
                ( ' ' == cChar || '\t' == cChar || 0x0a == cChar || !cChar ) &&
                FnSetINetAttr( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) ) )
            ;
        else
        {
            nRet = 0;
            // Grossbuchstabe am Satz-Anfang ??
            if( IsAutoCorrFlag( CptlSttSntnc ) &&
                FnCptlSttSntnc( rDoc, rTxt, TRUE, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CptlSttSntnc;

            // Zwei Grossbuchstaben am Wort-Anfang ??
            if( IsAutoCorrFlag( CptlSttWrd ) &&
                FnCptlSttWrd( rDoc, rTxt, nCapLttrPos, nInsPos, eLang ) )
                nRet |= CptlSttWrd;
        }
    } while( FALSE );

    return nRet;
}

SvxAutoCorrectLanguageLists& SvxAutoCorrect::_GetLanguageList(
                                                        LanguageType eLang )
{
    if( !pLangTable->IsKeyValid( ULONG( eLang )))
        CreateLanguageFile( eLang, TRUE);
    return *pLangTable->Seek( ULONG( eLang ) );
}

void SvxAutoCorrect::SaveCplSttExceptList( LanguageType eLang )
{
    if( pLangTable->IsKeyValid( ULONG( eLang )))
    {
        SvxAutoCorrectLanguageListsPtr pLists = pLangTable->Seek(ULONG(eLang));
        if( pLists )
            pLists->SaveCplSttExceptList();
    }
#ifndef PRODUCT
    else
    {
        DBG_ERROR("speichern einer leeren Liste?")
    }
#endif
}

void SvxAutoCorrect::SaveWrdSttExceptList(LanguageType eLang)
{
    if(pLangTable->IsKeyValid(ULONG(eLang)))
    {
        SvxAutoCorrectLanguageListsPtr pLists = pLangTable->Seek(ULONG(eLang));
        if(pLists)
            pLists->SaveWrdSttExceptList();
    }
#ifndef PRODUCT
    else
    {
        DBG_ERROR("speichern einer leeren Liste?")
    }
#endif
}


    // fuegt ein einzelnes Wort hinzu. Die Liste wird sofort
    // in die Datei geschrieben!
BOOL SvxAutoCorrect::AddCplSttException( const String& rNew,
                                        LanguageType eLang )
{
    SvxAutoCorrectLanguageListsPtr pLists = 0;
    //entweder die richtige Sprache ist vorhanden oder es kommt in die allg. Liste
    if( pLangTable->IsKeyValid(ULONG(eLang)))
        pLists = pLangTable->Seek(ULONG(eLang));
    else if(pLangTable->IsKeyValid(ULONG(LANGUAGE_DONTKNOW))||
            CreateLanguageFile(LANGUAGE_DONTKNOW, TRUE))
    {
        pLists = pLangTable->Seek(ULONG(LANGUAGE_DONTKNOW));
    }
    DBG_ASSERT(pLists, "keine Autokorrekturdatei")
    return pLists->AddToCplSttExceptList(rNew);
}


    // fuegt ein einzelnes Wort hinzu. Die Liste wird sofort
    // in die Datei geschrieben!
BOOL SvxAutoCorrect::AddWrtSttException( const String& rNew,
                                         LanguageType eLang )
{
    SvxAutoCorrectLanguageListsPtr pLists = 0;
    //entweder die richtige Sprache ist vorhanden oder es kommt in die allg. Liste
    if(pLangTable->IsKeyValid(ULONG(eLang)))
        pLists = pLangTable->Seek(ULONG(eLang));
    else if(pLangTable->IsKeyValid(ULONG(LANGUAGE_DONTKNOW))||
            CreateLanguageFile(LANGUAGE_DONTKNOW, TRUE))
        pLists = pLangTable->Seek(ULONG(LANGUAGE_DONTKNOW));
    DBG_ASSERT(pLists, "keine Autokorrekturdatei")
    return pLists->AddToWrdSttExceptList(rNew);
}




void SvxAutoCorrect::SetUserAutoCorrFileName( const String& rNew )
{
    if( sUserAutoCorrFile != rNew )
    {
        sUserAutoCorrFile = rNew;

        // sind die Listen gesetzt sind, so muessen sie jetzt geloescht
        // werden
        lcl_ClearTable(*pLangTable);
        nFlags &= ~(CplSttLstLoad | WrdSttLstLoad | ChgWordLstLoad );
    }
}

void SvxAutoCorrect::SetShareAutoCorrFileName( const String& rNew )
{
    if( sShareAutoCorrFile != rNew )
    {
        sShareAutoCorrFile = rNew;

        // sind die Listen gesetzt sind, so muessen sie jetzt geloescht
        // werden
        lcl_ClearTable(*pLangTable);
        nFlags &= ~(CplSttLstLoad | WrdSttLstLoad | ChgWordLstLoad );
    }
}


BOOL SvxAutoCorrect::GetPrevAutoCorrWord( SvxAutoCorrDoc& rDoc,
                                        const String& rTxt, xub_StrLen nPos,
                                        String& rWord ) const
{
    if( !nPos )
        return FALSE;

    xub_StrLen nEnde = nPos;

    // dahinter muss ein Blank oder Tab folgen!
    if( ( nPos < rTxt.Len() &&
        !IsWordDelim( rTxt.GetChar( nPos ))) ||
        IsWordDelim( rTxt.GetChar( --nPos )))
        return FALSE;

    while( nPos && !IsWordDelim( rTxt.GetChar( --nPos )))
        ;

    // Absatz-Anfang oder ein Blank gefunden, suche nach dem Wort
    // Kuerzel im Auto
    xub_StrLen nCapLttrPos = nPos+1;        // auf das 1. Zeichen
    if( !nPos && !IsWordDelim( rTxt.GetChar( 0 )))
        --nCapLttrPos;          // Absatz Anfang und kein Blank !

    while( lcl_IsInAsciiArr( sImplSttSkipChars, rTxt.GetChar( nCapLttrPos )) )
        if( ++nCapLttrPos >= nEnde )
            return FALSE;

    // Bug 19285: Symbolzeichen nicht anfassen
    // Interresant erst ab 3 Zeichen
    if( 3 > nEnde - nCapLttrPos )
        return FALSE;

    LanguageType eLang = rDoc.GetLanguage( nCapLttrPos, FALSE );
    if( LANGUAGE_SYSTEM == eLang )
        eLang = System::GetLanguage();

    SvxAutoCorrect* pThis = (SvxAutoCorrect*)this;
    CharClass& rCC = pThis->GetCharClass( eLang );

    if( lcl_IsSymbolChar( rCC, rTxt, nCapLttrPos, nEnde ))
        return FALSE;

    rWord = rTxt.Copy( nCapLttrPos, nEnde - nCapLttrPos );
    return TRUE;
}

BOOL SvxAutoCorrect::CreateLanguageFile( LanguageType eLang, BOOL bNewFile )
{
    DBG_ASSERT(!pLangTable->IsKeyValid(ULONG(eLang)), "Sprache ist bereits vorhanden")

    String sUserDirFile( GetAutoCorrFileName( eLang, TRUE, FALSE )),
           sShareDirFile( sUserDirFile );
    SvxAutoCorrectLanguageListsPtr pLists = 0;

    Time nMinTime( 0, 2 ), nAktTime, nLastCheckTime;
    ULONG nFndPos;
    if( TABLE_ENTRY_NOTFOUND !=
                    pLastFileTable->SearchKey( ULONG( eLang ), &nFndPos ) &&
        ( nLastCheckTime.SetTime( pLastFileTable->GetObject( nFndPos )),
            nLastCheckTime < nAktTime ) &&
        ( nAktTime - nLastCheckTime ) < nMinTime )
    {
        // no need to test the file, because the last check is not older then
        // 2 minutes.
        if( bNewFile )
        {
            sShareDirFile = sUserDirFile;
            pLists = new SvxAutoCorrectLanguageLists( *this, sShareDirFile,
                                                        sUserDirFile, eLang );
            pLangTable->Insert( ULONG(eLang), pLists );
            pLastFileTable->Remove( ULONG( eLang ) );
        }
    }
    else if( ( FStatHelper::IsDocument( sUserDirFile ) ||
                FStatHelper::IsDocument( sShareDirFile =
                              GetAutoCorrFileName( eLang, FALSE, FALSE ) ) ) ||
        ( sShareDirFile = sUserDirFile, bNewFile ))
    {
        pLists = new SvxAutoCorrectLanguageLists( *this, sShareDirFile,
                                                    sUserDirFile, eLang );
        pLangTable->Insert( ULONG(eLang), pLists );
        pLastFileTable->Remove( ULONG( eLang ) );
    }
    else if( !bNewFile )
    {
        if( !pLastFileTable->Insert( ULONG( eLang ), nAktTime.GetTime() ))
            pLastFileTable->Replace( ULONG( eLang ), nAktTime.GetTime() );
    }
    return pLists != 0;
}

BOOL SvxAutoCorrect::PutText( const String& rShort, const String& rLong,
                                LanguageType eLang )
{
    BOOL bRet = FALSE;
    if( pLangTable->IsKeyValid( ULONG(eLang)) || CreateLanguageFile(eLang) )
        bRet = pLangTable->Seek( ULONG(eLang) )->PutText(rShort, rLong);
    return bRet;
}


    //  - loesche einen Eintrag
BOOL SvxAutoCorrect::DeleteText( const String& rShort, LanguageType eLang )
{
    BOOL bRet = FALSE;
    if( pLangTable->IsKeyValid( ULONG( eLang )) )
        bRet = pLangTable->Seek( ULONG( eLang ))->DeleteText( rShort );
    return bRet;
}


    //  - return den Ersetzungstext (nur fuer SWG-Format, alle anderen
    //      koennen aus der Wortliste herausgeholt werden!)
BOOL SvxAutoCorrect::GetLongText( SvStorage&, const String& , String& )
{
    return FALSE;
}

    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
BOOL SvxAutoCorrect::PutText( SvStorage&, const String&, SfxObjectShell&,
                                String& )
{
    return FALSE;
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

void DecryptBlockName_Imp( String& rName )
{
    if( '#' == rName.GetChar( 0 ) )
    {
        rName.Erase( 0, 1 );
        sal_Unicode* pName = rName.GetBufferAccess();
        xub_StrLen nLen, nPos;
        for ( nLen = rName.Len(), nPos = 0; nPos < nLen; ++nPos, ++pName )
            switch( *pName )
            {
            case 0x01:  *pName = '!';   break;
            case 0x0A:  *pName = ':';   break;
            case 0x0C:  *pName = '\\';  break;
            case 0x0E:  *pName = '.';   break;
            case 0x0F:  *pName = '/';   break;
            }
    }
}


/* -----------------18.11.98 16:00-------------------
 *
 * --------------------------------------------------*/
const SvxAutocorrWord* lcl_SearchWordsInList(
                SvxAutoCorrectLanguageListsPtr pList, const String& rTxt,
                xub_StrLen& rStt, xub_StrLen nEndPos, SvxAutoCorrDoc& rDoc )
{
    const SvxAutocorrWordList* pAutoCorrWordList = pList->GetAutocorrWordList();
    const International& rInter = Application::GetAppInternational();
    for( xub_StrLen nPos = 0; nPos < pAutoCorrWordList->Count(); ++nPos )
    {
        const SvxAutocorrWord* pFnd = (*pAutoCorrWordList)[ nPos ];
        const String& rChk = pFnd->GetShort();
        if( nEndPos >= rChk.Len() )
        {
            xub_StrLen nCalcStt = nEndPos - rChk.Len();
            if( ( !nCalcStt || nCalcStt == rStt ||
                ( nCalcStt < rStt &&
                    IsWordDelim( rTxt.GetChar(nCalcStt - 1 ) ))) )
            {
                String sWord( rTxt.GetBuffer() + nCalcStt, rChk.Len() );
                if( COMPARE_EQUAL == rInter.Compare( rChk, sWord,
                                        INTN_COMPARE_IGNORECASE ) )
                {
                    rStt = nCalcStt;
                    return pFnd;
                }
            }
        }
    }
    return 0;
}


// suche das oder die Worte in der ErsetzungsTabelle
const SvxAutocorrWord* SvxAutoCorrect::SearchWordsInList(
                const String& rTxt, xub_StrLen& rStt, xub_StrLen nEndPos,
                SvxAutoCorrDoc& rDoc, LanguageType& rLang )
{
    LanguageType eLang = rLang;
    const SvxAutocorrWord* pRet = 0;
    if( LANGUAGE_SYSTEM == eLang )
        eLang = System::GetLanguage();

    // zuerst nach eLang suchen, dann nach der Obersprache
    // US-Englisch -> Englisch und zuletzt in LANGUAGE_DONTKNOW

    if( pLangTable->IsKeyValid( ULONG( eLang ) ) ||
        CreateLanguageFile( eLang, FALSE ))
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pList = pLangTable->Seek(ULONG(eLang));
        pRet = lcl_SearchWordsInList(  pList, rTxt, rStt, nEndPos, rDoc );
        if( pRet )
        {
            rLang = eLang;
            return pRet;
        }
    }

    // wenn es hier noch nicht gefunden werden konnte, dann weitersuchen
    ULONG nTmpKey1 = eLang & 0x7ff, // die Hauptsprache in vielen Faellen u.B. DE
          nTmpKey2 = eLang & 0x3ff, // sonst z.B. EN
          nTmp;

    if( ((nTmp = nTmpKey1) != (ULONG)eLang &&
         ( pLangTable->IsKeyValid( nTmpKey1 ) ||
           CreateLanguageFile( LanguageType( nTmpKey1 ), FALSE ) )) ||
        (( nTmp = nTmpKey2) != (ULONG)eLang &&
         ( pLangTable->IsKeyValid( nTmpKey2 ) ||
           CreateLanguageFile( LanguageType( nTmpKey2 ), FALSE ) )) )
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pList = pLangTable->Seek( nTmp );
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos, rDoc);
        if( pRet )
        {
            rLang = LanguageType( nTmp );
            return pRet;
        }
    }
    if( pLangTable->IsKeyValid( ULONG( LANGUAGE_DONTKNOW ) ) ||
        CreateLanguageFile( LANGUAGE_DONTKNOW, FALSE ) )
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pList = pLangTable->Seek(ULONG(LANGUAGE_DONTKNOW));
        pRet = lcl_SearchWordsInList( pList, rTxt, rStt, nEndPos, rDoc);
        if( pRet )
        {
            rLang = LANGUAGE_DONTKNOW;
            return pRet;
        }
    }
    return 0;
}
/* -----------------18.11.98 13:46-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrect::FindInWrdSttExceptList( LanguageType eLang,
                                             const String& sWord )
{
    //zuerst nach eLang suchen, dann nach der Obersprace US-Englisch -> Englisch
    //und zuletzt in LANGUAGE_DONTKNOW
    ULONG nTmpKey1 = eLang & 0x7ff; // die Hauptsprache in vielen Faellen u.B. DE
    ULONG nTmpKey2 = eLang & 0x3ff; // sonst z.B. EN
    String sTemp(sWord);
    if( pLangTable->IsKeyValid( ULONG( eLang )) ||
        CreateLanguageFile( eLang, FALSE ) )
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pList = pLangTable->Seek(ULONG(eLang));
        String sTemp(sWord);
        if(pList->GetWrdSttExceptList()->Seek_Entry(&sTemp))
            return TRUE;

    }
    // wenn es hier noch nicht gefunden werden konnte, dann weitersuchen
    ULONG nTmp;
    if( ((nTmp = nTmpKey1) != (ULONG)eLang &&
         ( pLangTable->IsKeyValid( nTmpKey1 ) ||
           CreateLanguageFile( LanguageType( nTmpKey1 ), FALSE ) )) ||
        (( nTmp = nTmpKey2) != (ULONG)eLang &&
         ( pLangTable->IsKeyValid( nTmpKey2 ) ||
           CreateLanguageFile( LanguageType( nTmpKey2 ), FALSE ) )) )
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pList = pLangTable->Seek(nTmp);
        if(pList->GetWrdSttExceptList()->Seek_Entry(&sTemp))
            return TRUE;
    }
    if(pLangTable->IsKeyValid(ULONG(LANGUAGE_DONTKNOW))|| CreateLanguageFile(LANGUAGE_DONTKNOW, FALSE))
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pList = pLangTable->Seek(ULONG(LANGUAGE_DONTKNOW));
        if(pList->GetWrdSttExceptList()->Seek_Entry(&sTemp))
            return TRUE;
    }
    return FALSE;
}
/* -----------------18.11.98 14:28-------------------
 *
 * --------------------------------------------------*/
BOOL lcl_FindAbbreviation( const SvStringsISortDtor* pList, const String& sWord)
{
    String sAbk( '~' );
    USHORT nPos;
    pList->Seek_Entry( &sAbk, &nPos );
    if( nPos < pList->Count() )
    {
        String sLowerWord( sWord ); sLowerWord.ToLowerAscii();
        const String* pAbk;
        for( USHORT n = nPos;
                n < pList->Count() &&
                '~' == ( pAbk = (*pList)[ n ])->GetChar( 0 );
            ++n )
        {
            // ~ und ~. sind nicht erlaubt!
            if( 2 < pAbk->Len() && pAbk->Len() - 1 <= sWord.Len() )
            {
                String sLowerAbk( *pAbk ); sLowerAbk.ToLowerAscii();
                for( xub_StrLen i = sLowerAbk.Len(), ii = sLowerWord.Len(); i; )
                {
                    if( !--i )      // stimmt ueberein
                        return TRUE;

                    if( sLowerAbk.GetChar( i ) != sLowerWord.GetChar( --ii ))
                        break;
                }
            }
        }
    }
    DBG_ASSERT( !(nPos && '~' == (*pList)[ --nPos ]->GetChar( 0 ) ),
            "falsch sortierte ExeptionListe?" );
    return FALSE;
}
/* -----------------18.11.98 14:49-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrect::FindInCplSttExceptList(LanguageType eLang,
                                const String& sWord, BOOL bAbbreviation)
{
    //zuerst nach eLang suchen, dann nach der Obersprace US-Englisch -> Englisch
    //und zuletzt in LANGUAGE_DONTKNOW
    ULONG nTmpKey1 = eLang & 0x7ff; // die Hauptsprache in vielen Faellen u.B. DE
    ULONG nTmpKey2 = eLang & 0x3ff; // sonst z.B. EN
    String sTemp( sWord );
    if( pLangTable->IsKeyValid( ULONG( eLang )) ||
        CreateLanguageFile( eLang, FALSE ))
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pLists = pLangTable->Seek(ULONG(eLang));
        const SvStringsISortDtor* pList = pLists->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation( pList, sWord)
                         : pList->Seek_Entry( &sTemp ) )
            return TRUE;
    }
    // wenn es hier noch nicht gefunden werden konnte, dann weitersuchen
    ULONG nTmp;

    if( ((nTmp = nTmpKey1) != (ULONG)eLang &&
         ( pLangTable->IsKeyValid( nTmpKey1 ) ||
           CreateLanguageFile( LanguageType( nTmpKey1 ), FALSE ) )) ||
        (( nTmp = nTmpKey2) != (ULONG)eLang &&
         ( pLangTable->IsKeyValid( nTmpKey2 ) ||
           CreateLanguageFile( LanguageType( nTmpKey2 ), FALSE ) )) )
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pLists = pLangTable->Seek(nTmp);
        const SvStringsISortDtor* pList = pLists->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation( pList, sWord)
                         : pList->Seek_Entry( &sTemp ) )
            return TRUE;
    }
    if(pLangTable->IsKeyValid(ULONG(LANGUAGE_DONTKNOW))|| CreateLanguageFile(LANGUAGE_DONTKNOW, FALSE))
    {
        //die Sprache ist vorhanden - also her damit
        SvxAutoCorrectLanguageListsPtr pLists = pLangTable->Seek(LANGUAGE_DONTKNOW);
        const SvStringsISortDtor* pList = pLists->GetCplSttExceptList();
        if(bAbbreviation ? lcl_FindAbbreviation( pList, sWord)
                         : pList->Seek_Entry( &sTemp ) )
            return TRUE;
    }
    return FALSE;

}

/* -----------------20.11.98 11:53-------------------
 *
 * --------------------------------------------------*/
String SvxAutoCorrect::GetAutoCorrFileName( LanguageType eLang,
                                            BOOL bNewFile, BOOL bTst ) const
{
    String sRet, sExt( String::CreateFromInt32( eLang ));
    sExt.AppendAscii( ".dat" );

    if( bNewFile )
        ( sRet = sUserAutoCorrFile ) += sExt;
    else if( !bTst )
        ( sRet = sShareAutoCorrFile ) += sExt;
    else
    {
        // test first in the user directory - if not exist, then
        ( sRet = sUserAutoCorrFile ) += sExt;
        if( !FStatHelper::IsDocument( sRet ))
            ( sRet = sShareAutoCorrFile ) += sExt;
    }
    return sRet;
}

/* -----------------18.11.98 11:16-------------------
 *
 * --------------------------------------------------*/
SvxAutoCorrectLanguageLists::SvxAutoCorrectLanguageLists(
                SvxAutoCorrect& rParent,
                const String& rShareAutoCorrectFile,
                const String& rUserAutoCorrectFile,
                LanguageType eLang)
    : rAutoCorrect(rParent),
    eLanguage(eLang),
    sShareAutoCorrFile( rShareAutoCorrectFile ),
    sUserAutoCorrFile( rUserAutoCorrectFile ),
    nFlags(0),
    pCplStt_ExcptLst( 0 ),
    pWrdStt_ExcptLst( 0 ),
    pAutocorr_List( 0 )
{
}

/* -----------------18.11.98 11:16-------------------
 *
 * --------------------------------------------------*/
SvxAutoCorrectLanguageLists::~SvxAutoCorrectLanguageLists()
{
    delete pCplStt_ExcptLst;
    delete pWrdStt_ExcptLst;
    delete pAutocorr_List;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrectLanguageLists::IsFileChanged_Imp()
{
    // nur alle 2 Minuten aufs FileSystem zugreifen um den
    // Dateistempel zu ueberpruefen
    BOOL bRet = FALSE;

    Time nMinTime( 0, 2 );
    Time nAktTime;
    if( aLastCheckTime > nAktTime ||                    // ueberlauf ?
        ( nAktTime -= aLastCheckTime ) > nMinTime )     // min Zeit vergangen
    {
        Date aTstDate; Time aTstTime;
        if( FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                            &aTstDate, &aTstTime ) &&
            ( aModifiedDate != aTstDate || aModifiedTime != aTstTime ))
        {
            bRet = TRUE;
            // dann mal schnell alle Listen entfernen!
            if( CplSttLstLoad & nFlags && pCplStt_ExcptLst )
                delete pCplStt_ExcptLst, pCplStt_ExcptLst = 0;
            if( WrdSttLstLoad & nFlags && pWrdStt_ExcptLst )
                delete pWrdStt_ExcptLst, pWrdStt_ExcptLst = 0;
            if( ChgWordLstLoad & nFlags && pAutocorr_List )
                delete pAutocorr_List, pAutocorr_List = 0;
            nFlags &= ~(CplSttLstLoad | WrdSttLstLoad | ChgWordLstLoad );
        }
        aLastCheckTime = Time();
    }
    return bRet;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::LoadExceptList_Imp(
                                        SvStringsISortDtor*& rpLst,
                                        const sal_Char* pStrmName,
                                        SvStorageRef &rStg)
{
    if( rpLst )
        rpLst->DeleteAndDestroy( 0, rpLst->Count() );
    else
        rpLst = new SvStringsISortDtor( 16, 16 );

    {

        String sStrmName( pStrmName, RTL_TEXTENCODING_MS_1252 );
        String sTmp( sStrmName );

        if( rStg.Is() && ( rStg->IsStream( sStrmName ) ||
            // "alte" Listen konvertieren!
            ( pCplStt_ExcptLst == rpLst &&
                rStg->IsStream( sTmp.AssignAscii( "ExceptionList" ))) ))
        {
            SvStorageStreamRef xStrm = rStg->OpenStream( sTmp,
                ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE ) );
            if( SVSTREAM_OK != xStrm->GetError())
            {
                xStrm.Clear();
                rStg.Clear();
                RemoveStream_Imp( sStrmName );
            }
            else
            {
                xStrm->SetBufferSize( 8192 );

                BYTE cLen, cSet;
                USHORT nVersion, nCount;
                *xStrm >> cLen >> nVersion >> cSet >> nCount;

                for( USHORT i = 0; i < nCount; ++i)
                {
                    String* pNew = new String;
                    xStrm->ReadByteString( *pNew, cSet );

                    if( xStrm->IsEof() || SVSTREAM_OK != xStrm->GetError() )
                    {
//                      nErr = ( pExceptLst->GetError() == SVSTREAM_OK )
//                                  ? 0 : ERR_SWG_READ_ERROR;
                        xStrm.Clear();
                        delete pNew;
                        break;
                    }

                    if( !rpLst->Insert( pNew ) )
                        delete pNew;
                }

                if( sTmp != sStrmName )
                {
                    xStrm.Clear();
                    rStg.Clear();
                    RemoveStream_Imp( sTmp );
                }
            }
        }

        // Zeitstempel noch setzen
        FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                        &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time();
    }
}
void SvxAutoCorrectLanguageLists::LoadXMLExceptList_Imp(
                                        SvStringsISortDtor*& rpLst,
                                        const sal_Char* pStrmName,
                                        SvStorageRef &rStg)
{
    if( rpLst )
        rpLst->DeleteAndDestroy( 0, rpLst->Count() );
    else
        rpLst = new SvStringsISortDtor( 16, 16 );

    {
        String sStrmName( pStrmName, RTL_TEXTENCODING_MS_1252 );
        String sTmp( sStrmName );

        if( rStg.Is() && rStg->IsStream( sStrmName ) )
        {
            SvStorageStreamRef xStrm = rStg->OpenStream( sTmp,
                ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE ) );
            if( SVSTREAM_OK != xStrm->GetError())
            {
                xStrm.Clear();
                rStg.Clear();
                RemoveStream_Imp( sStrmName );
            }
            else
            {
                Reference< lang::XMultiServiceFactory > xServiceFactory =
                    comphelper::getProcessServiceFactory();
                DBG_ASSERT( xServiceFactory.is(),
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

                // get parser
                Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
                DBG_ASSERT( xXMLParser.is(),
                    "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
                if( !xXMLParser.is() )
                {
                    // Maybe throw an exception?
                }

                // get filter
                Reference< xml::sax::XDocumentHandler > xFilter = new SvXMLExceptionListImport ( *rpLst );

                // connect parser and filter
                Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
                xParser->setDocumentHandler( xFilter );

                // parse
                try
                {
                    xParser->parseStream( aParserInput );
                }
                catch( xml::sax::SAXParseException&  )
                {
                    // re throw ?
                }
                catch( xml::sax::SAXException&  )
                {
                    // re throw ?
                }
                catch( io::IOException& )
                {
                    // re throw ?
                }
            }
        }

        // Zeitstempel noch setzen
        FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                        &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time();
    }

}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::SaveExceptList_Imp(
                            const SvStringsISortDtor& rLst,
                            const sal_Char* pStrmName )
{
    MakeUserStorage_Impl();

    SfxMedium aMedium( sUserAutoCorrFile, STREAM_READWRITE, TRUE );
    SvStorageRef xStg = aMedium.GetOutputStorage();

    if( xStg.Is() )
    {
        String sStrmName( pStrmName, RTL_TEXTENCODING_MS_1252 );
        if( !rLst.Count() )
        {
            xStg->Remove( sStrmName );
            xStg->Commit();
        }
        else
        {
            SvStorageStreamRef xStrm = xStg->OpenStream( sStrmName,
                    ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE ) );
            if( xStrm.Is() )
            {
                xStrm->SetSize( 0 );
                xStrm->SetBufferSize( 8192 );
                Reference< lang::XMultiServiceFactory > xServiceFactory =
                    comphelper::getProcessServiceFactory();
                DBG_ASSERT( xServiceFactory.is(),
                            "XMLReader::Read: got no service manager" );
                if( !xServiceFactory.is() )
                {
                    // Throw an exception ?
                }

                    Reference < XInterface > xWriter (xServiceFactory->createInstance(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
                    DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
                Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *xStrm );
                    uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
                    xSrc->setOutputStream(xOut);

                    uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

                    SvXMLExceptionListExport aExp(rLst, sStrmName, xHandler);
                aExp.exportDoc( sXML_block_list );

                xStrm->Commit();
                if( xStrm->GetError() == SVSTREAM_OK )
                {
                    xStrm.Clear();
                    xStg->Commit();
                    if( SVSTREAM_OK != xStg->GetError() )
                    {
                        xStg->Remove( sStrmName );
                        xStg->Commit();
                    }
                }
            }
        }

        xStg = 0;
        aMedium.Commit();

        // Zeitstempel noch setzen
        FStatHelper::GetModifiedDateTimeOfFile( sUserAutoCorrFile,
                                        &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time();
    }
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
SvxAutocorrWordList* SvxAutoCorrectLanguageLists::LoadAutocorrWordList()
{


    if( pAutocorr_List )
        pAutocorr_List->DeleteAndDestroy( 0, pAutocorr_List->Count() );
    else
        pAutocorr_List = new SvxAutocorrWordList( 16, 16 );

    SfxMedium aMedium( sShareAutoCorrFile,
                        STREAM_READ | STREAM_SHARE_DENYNONE, TRUE );
    SvStorageRef xStg = aMedium.GetStorage();

    SvStringsDtor aRemoveArr;
    String aWordListName( pImplAutocorr_ListStr, RTL_TEXTENCODING_MS_1252 );
    String aXMLWordListName( pXMLImplAutocorr_ListStr, RTL_TEXTENCODING_MS_1252 );
    if( xStg.Is() && xStg->IsStream( aWordListName ) )
    {
        SvStorageStreamRef xStrm = xStg->OpenStream( aWordListName,
            ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE ) );
        if( SVSTREAM_OK != xStrm->GetError())
        {
            xStrm.Clear();
            xStg.Clear();
            RemoveStream_Imp( aWordListName );
        }
        else
        {
            xStrm->SetBufferSize( 8192 );

//!!!!!!!!!!!!!
            // JP 10.10.96: AutocorrDatei vom Writer hat in einer alten
            //              Version keine CharSet Information sondern
            //              beginnt sofort mit einem String.
            //              Diese Dateien koennen wir ERSTMAL nicht
            //              verarbeiten.
//!!!!!!!!!!!!!
            String sShort, sLong;
            xStrm->ReadByteString( sShort, RTL_TEXTENCODING_MS_1252 );
            if( !sShort.Len() )
            {
                // neuer Stream (mit Header)
                ULONG nOld = xStrm->Tell();

                BYTE cLen, cSet;
                USHORT nVersion;
                *xStrm >> cLen >> nVersion >> cSet;

                SvStorageInfoList* pInfoList = 0;
                if( WORDLIST_VERSION_358 > nVersion )
                {
                    pInfoList = new SvStorageInfoList;
                    xStg->FillInfoList( pInfoList );
                }

                ULONG nNew = xStrm->Tell();
                nOld += cLen;
                if( nOld != nNew )
                    xStrm->Seek( nOld );

                const International& rInter = Application::GetAppInternational();

                // dann lese mal alle Ersetzungen:
                while( TRUE )
                {
                    xStrm->ReadByteString( sShort, cSet ).
                           ReadByteString( sLong, cSet );
                    if( xStrm->IsEof() ||  SVSTREAM_OK != xStrm->GetError() )
                        break;

                    BOOL bOnlyTxt = COMPARE_EQUAL != rInter.Compare(
                                    sShort, sLong, INTN_COMPARE_IGNORECASE );
                    if( !bOnlyTxt )
                    {
                        String sLongSave( sLong );
                        if( !rAutoCorrect.GetLongText( *xStg, sShort, sLong ) &&
                            sLongSave.Len() )
                        {
                            sLong = sLongSave;
                            bOnlyTxt = TRUE;
                        }
                    }

                    SvxAutocorrWordPtr pNew = new SvxAutocorrWord(
                                                sShort, sLong, bOnlyTxt );

                    if( !pAutocorr_List->Insert( pNew ) )
                        delete pNew;

                    if( pInfoList )
                    {
                        // dann ggfs aus den alten Storages die
                        // ueberfluessigen Streams entfernen
                        EncryptBlockName_Imp( sShort );
                        const SvStorageInfo* pInfo = pInfoList->Get( sShort );
                        if( pInfo && ( pInfo->IsStream() ||
                            (bOnlyTxt && pInfo->IsStorage() ) ))
                        {
                            String* pNew = new String( sShort );
                            aRemoveArr.Insert( pNew, aRemoveArr.Count() );
                        }
                    }
                }
                if( pInfoList )
                    delete pInfoList;
            }
        }


        if( aRemoveArr.Count() && sShareAutoCorrFile == sUserAutoCorrFile )
        {
            xStrm.Clear();
            xStg.Clear();
            xStg = new SvStorage( sShareAutoCorrFile,
                                STREAM_STD_READWRITE, STORAGE_TRANSACTED );

            if( xStg.Is() && SVSTREAM_OK == xStg->GetError() )
            {
                String* pStr;
                for( USHORT n = aRemoveArr.Count(); n; )
                    if( xStg->IsContained( *( pStr = aRemoveArr[ --n ] ) ) )
                        xStg->Remove( *pStr  );

                // die neue Liste mit der neuen Versionsnummer speichern
                MakeBlocklist_Imp( *xStg );
                xStg->Commit();
            }
        }

        // Zeitstempel noch setzen
        FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                        &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time();
    }
    else if( xStg.Is() && xStg->IsStream( aXMLWordListName ) )
    {
        SvStorageStreamRef xStrm = xStg->OpenStream( aXMLWordListName,
            ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE ) );
        if( SVSTREAM_OK != xStrm->GetError())
        {
            xStrm.Clear();
            xStg.Clear();
            RemoveStream_Imp( aWordListName );
        }
        else
        {
            Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
            DBG_ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
            if( !xServiceFactory.is() )
            {
                // Throw an exception ?
            }

            xml::sax::InputSource aParserInput;
            aParserInput.sSystemId = aXMLWordListName;

            xStrm->Seek( 0L );
            xStrm->SetBufferSize( 8 * 1024 );
            aParserInput.aInputStream = new utl::OInputStreamWrapper( *xStrm );

            // get parser
            Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
            DBG_ASSERT( xXMLParser.is(),
                "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
            if( !xXMLParser.is() )
            {
                // Maybe throw an exception?
            }

            // get filter
            Reference< xml::sax::XDocumentHandler > xFilter = new SvXMLAutoCorrectImport( pAutocorr_List, rAutoCorrect, xStg );

            // connect parser and filter
            Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
            xParser->setDocumentHandler( xFilter );

            // parse
            try
            {
                xParser->parseStream( aParserInput );
            }
            catch( xml::sax::SAXParseException&  )
            {
                // re throw ?
            }
            catch( xml::sax::SAXException&  )
            {
                // re throw ?
            }
            catch( io::IOException& )
            {
                // re throw ?
            }
        }


        if( aRemoveArr.Count() && sShareAutoCorrFile == sUserAutoCorrFile )
        {
            xStrm.Clear();
            xStg.Clear();
            xStg = new SvStorage( sShareAutoCorrFile,
                                STREAM_STD_READWRITE, STORAGE_TRANSACTED );

            if( xStg.Is() && SVSTREAM_OK == xStg->GetError() )
            {
                String* pStr;
                for( USHORT n = aRemoveArr.Count(); n; )
                    if( xStg->IsContained( *( pStr = aRemoveArr[ --n ] ) ) )
                        xStg->Remove( *pStr  );

                // die neue Liste mit der neuen Versionsnummer speichern
                MakeBlocklist_Imp( *xStg );
                xStg->Commit();
            }
        }

        // Zeitstempel noch setzen
        FStatHelper::GetModifiedDateTimeOfFile( sShareAutoCorrFile,
                                        &aModifiedDate, &aModifiedTime );
        aLastCheckTime = Time();
    }
    return pAutocorr_List;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/

void SvxAutoCorrectLanguageLists::SetAutocorrWordList( SvxAutocorrWordList* pList )
{
    if( pAutocorr_List && pList != pAutocorr_List )
        delete pAutocorr_List;
    pAutocorr_List = pList;
    if( !pAutocorr_List )
    {
        DBG_ASSERT( !this, "keine gueltige Liste" );
        pAutocorr_List = new SvxAutocorrWordList( 16, 16 );
    }
    nFlags |= ChgWordLstLoad;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
const SvxAutocorrWordList* SvxAutoCorrectLanguageLists::GetAutocorrWordList()
{
    if( !( ChgWordLstLoad & nFlags ) || IsFileChanged_Imp() )
        SetAutocorrWordList( LoadAutocorrWordList() );
    return pAutocorr_List;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
const SvStringsISortDtor* SvxAutoCorrectLanguageLists::GetCplSttExceptList()
{
    if( !( CplSttLstLoad & nFlags ) || IsFileChanged_Imp() )
        SetCplSttExceptList( LoadCplSttExceptList() );
    return pCplStt_ExcptLst;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrectLanguageLists::AddToCplSttExceptList(const String& rNew)
{
    String* pNew = new String( rNew );
    if( rNew.Len() && LoadCplSttExceptList()->Insert( pNew ) )
        SaveExceptList_Imp( *pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr );
    else
        delete pNew, pNew = 0;
    return 0 != pNew;
}
/* -----------------18.11.98 15:20-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrectLanguageLists::AddToWrdSttExceptList(const String& rNew)
{
    String* pNew = new String( rNew );
    if( rNew.Len() && LoadWrdSttExceptList()->Insert( pNew ) )
        SaveExceptList_Imp( *pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr );
    else
        delete pNew, pNew = 0;
    return 0 != pNew;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
SvStringsISortDtor* SvxAutoCorrectLanguageLists::LoadCplSttExceptList()
{
    SfxMedium aMedium( sShareAutoCorrFile,
                            STREAM_READ | STREAM_SHARE_DENYNONE, TRUE );
    SvStorageRef xStg = aMedium.GetStorage();
    String sTemp ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplCplStt_ExcptLstStr ) );
    if( xStg.Is() && xStg->IsContained( sTemp ) )
        LoadXMLExceptList_Imp( pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr, xStg );
    else
        LoadExceptList_Imp( pCplStt_ExcptLst, pImplCplStt_ExcptLstStr, xStg );

    return pCplStt_ExcptLst;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::SaveCplSttExceptList()
{
    SaveExceptList_Imp( *pCplStt_ExcptLst, pXMLImplCplStt_ExcptLstStr );
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::SetCplSttExceptList( SvStringsISortDtor* pList )
{
    if( pCplStt_ExcptLst && pList != pCplStt_ExcptLst )
        delete pCplStt_ExcptLst;

    pCplStt_ExcptLst = pList;
    if( !pCplStt_ExcptLst )
    {
        DBG_ASSERT( !this, "keine gueltige Liste" );
        pCplStt_ExcptLst = new SvStringsISortDtor( 16, 16 );
    }
    nFlags |= CplSttLstLoad;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
SvStringsISortDtor* SvxAutoCorrectLanguageLists::LoadWrdSttExceptList()
{
    SfxMedium aMedium( sShareAutoCorrFile,
                            STREAM_READ | STREAM_SHARE_DENYNONE, TRUE );
    SvStorageRef xStg = aMedium.GetStorage();
    String sTemp ( RTL_CONSTASCII_USTRINGPARAM ( pXMLImplWrdStt_ExcptLstStr ) );
    if( xStg.Is() && xStg->IsContained( sTemp ) )
        LoadXMLExceptList_Imp( pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr, xStg );
    else
        LoadExceptList_Imp( pWrdStt_ExcptLst, pImplWrdStt_ExcptLstStr, xStg );
    return pWrdStt_ExcptLst;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::SaveWrdSttExceptList()
{
    SaveExceptList_Imp( *pWrdStt_ExcptLst, pXMLImplWrdStt_ExcptLstStr );
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::SetWrdSttExceptList( SvStringsISortDtor* pList )
{
    if( pWrdStt_ExcptLst && pList != pWrdStt_ExcptLst )
        delete pWrdStt_ExcptLst;
    pWrdStt_ExcptLst = pList;
    if( !pWrdStt_ExcptLst )
    {
        DBG_ASSERT( !this, "keine gueltige Liste" );
        pWrdStt_ExcptLst = new SvStringsISortDtor( 16, 16 );
    }
    nFlags |= WrdSttLstLoad;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
const SvStringsISortDtor* SvxAutoCorrectLanguageLists::GetWrdSttExceptList()
{
    if( !( WrdSttLstLoad & nFlags ) || IsFileChanged_Imp() )
        SetWrdSttExceptList( LoadWrdSttExceptList() );
    return pWrdStt_ExcptLst;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
void SvxAutoCorrectLanguageLists::RemoveStream_Imp( const String& rName )
{
    if( sShareAutoCorrFile != sUserAutoCorrFile )
    {
        SfxMedium aMedium( sUserAutoCorrFile, STREAM_STD_READWRITE, TRUE );
        SvStorageRef xStg = aMedium.GetStorage();

        if( xStg.Is() && SVSTREAM_OK == xStg->GetError() &&
            xStg->IsStream( rName ) )
        {
            xStg->Remove( rName );
            xStg->Commit();

            xStg = 0;
            aMedium.Commit();
        }
    }
}

void SvxAutoCorrectLanguageLists::MakeUserStorage_Impl()
{
    if( sUserAutoCorrFile != sShareAutoCorrFile )
    {
        // at first copy the complete storage file from the share to
        // user directory
        SfxMedium aSrcMedium( sShareAutoCorrFile, STREAM_STD_READ, TRUE );
        SvStorageRef xSrcStg = aSrcMedium.GetStorage();
        SfxMedium aDstMedium( sUserAutoCorrFile, STREAM_STD_WRITE, TRUE );
        // Copy it to a UCBStorage
        SvStorageRef xDstStg = aDstMedium.GetOutputStorage(  );

        if( xSrcStg.Is() && xDstStg.Is() )
        {
            xSrcStg->CopyTo( &xDstStg );
            sShareAutoCorrFile = sUserAutoCorrFile;
            xDstStg = 0;
            aDstMedium.Commit();
        }
    }
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrectLanguageLists::MakeBlocklist_Imp( SvStorage& rStg )
{
    String sStrmName( pXMLImplAutocorr_ListStr, RTL_TEXTENCODING_MS_1252 );
    BOOL bRet = TRUE, bRemove = !pAutocorr_List || !pAutocorr_List->Count();
    if( !bRemove )
    {
        if ( rStg.IsContained( sStrmName) )
        {
            rStg.Remove ( sStrmName );
            rStg.Commit();
        }
        SvStorageStreamRef refList = rStg.OpenStream( sStrmName,
                    ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE ) );
        if( refList.Is() )
        {
            refList->SetSize( 0 );
            refList->SetBufferSize( 8192 );
            Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
            DBG_ASSERT( xServiceFactory.is(),
                        "XMLReader::Read: got no service manager" );
            if( !xServiceFactory.is() )
            {
                // Throw an exception ?
            }

                Reference < XInterface > xWriter (xServiceFactory->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
                DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
            Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper( *refList );
                uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
                xSrc->setOutputStream(xOut);

                uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

                SvXMLAutoCorrectExport aExp(pAutocorr_List, sStrmName, xHandler);
            aExp.exportDoc( sXML_block_list );

            refList->Commit();
            bRet = SVSTREAM_OK == refList->GetError();
            if( bRet )
            {
                refList.Clear();
                rStg.Commit();
                if( SVSTREAM_OK != rStg.GetError() )
                {
                    bRemove = TRUE;
                    bRet = FALSE;
                }
            }

            /*
            refList->SetSize( 0 );
            refList->SetBufferSize( 8192 );
            rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();

            String aDummy;              // Erkennungszeichen fuer neue Streams
            refList->WriteByteString( aDummy, RTL_TEXTENCODING_MS_1252 )
                     << (BYTE)  4       // Laenge des Headers (ohne den Leerstring)
                     << (USHORT)WORDLIST_VERSION_358    // Version des Streams
                     << (BYTE)eEncoding;                // der Zeichensatz

            for( USHORT i = 0; i < pAutocorr_List->Count() &&
                                SVSTREAM_OK == refList->GetError(); ++i )
            {
                SvxAutocorrWord* p = pAutocorr_List->GetObject( i );
                refList->WriteByteString( p->GetShort(), eEncoding ).
                        WriteByteString(  p->IsTextOnly()
                                            ? p->GetLong()
                                            : p->GetShort(), eEncoding );
            }
            refList->Commit();
            bRet = SVSTREAM_OK == refList->GetError();
            if( bRet )
            {
                refList.Clear();
                rStg.Commit();
                if( SVSTREAM_OK != rStg.GetError() )
                {
                    bRemove = TRUE;
                    bRet = FALSE;
                }
            }
            */
        }
        else
            bRet = FALSE;
    }

    if( bRemove )
    {
        rStg.Remove( sStrmName );
        rStg.Commit();
    }

    return bRet;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
BOOL SvxAutoCorrectLanguageLists::PutText( const String& rShort,
                                           const String& rLong )
{
    // erstmal akt. Liste besorgen!
    GetAutocorrWordList();

    MakeUserStorage_Impl();

    SfxMedium aMedium( sUserAutoCorrFile, STREAM_STD_READWRITE, TRUE );
    SvStorageRef xStg = aMedium.GetOutputStorage();
    BOOL bRet = xStg.Is() && SVSTREAM_OK == xStg->GetError();

/*  if( bRet )
    {
        // PutText( *xStg, rShort );
    }
*/
    // die Wortliste aktualisieren
    if( bRet )
    {
        USHORT nPos;
        SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, rLong, TRUE );
        if( pAutocorr_List->Seek_Entry( pNew, &nPos ) )
        {
            if( !(*pAutocorr_List)[ nPos ]->IsTextOnly() )
            {
                // dann ist der Storage noch zu entfernen
                String sStgNm( rShort );
                EncryptBlockName_Imp( sStgNm );
                if( xStg->IsContained( sStgNm ) )
                    xStg->Remove( sStgNm );
            }
            pAutocorr_List->DeleteAndDestroy( nPos );
        }

        if( pAutocorr_List->Insert( pNew ) )
        {
            bRet = MakeBlocklist_Imp( *xStg );
            xStg = 0;
            aMedium.Commit();
        }
        else
        {
            delete pNew;
            bRet = FALSE;
        }
    }
    return bRet;
}
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
BOOL SvxAutoCorrectLanguageLists::PutText( const String& rShort,
                                        SfxObjectShell& rShell )
{
    // erstmal akt. Liste besorgen!
    GetAutocorrWordList();

    MakeUserStorage_Impl();

    String sLong;
    SfxMedium aMedium( sUserAutoCorrFile, STREAM_STD_READWRITE, TRUE );
    SvStorageRef xStg = aMedium.GetOutputStorage();
    BOOL bRet = xStg.Is() && SVSTREAM_OK == xStg->GetError();

    if( bRet )
    {
//      String aName( rShort );
//      EncryptBlockName_Imp( aName );
//      bRet = PutText( *xStg, aName, rShell, sLong );
        bRet = rAutoCorrect.PutText( *xStg, rShort, rShell, sLong );
    }

    // die Wortliste aktualisieren
    if( bRet )
    {
        SvxAutocorrWord* pNew = new SvxAutocorrWord( rShort, sLong, FALSE );
        if( pAutocorr_List->Insert( pNew ) )
        {
            MakeBlocklist_Imp( *xStg );
            xStg = 0;
            aMedium.Commit();
        }
        else
            delete pNew;
    }
    return bRet;
}

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
    //  - loesche einen Eintrag
BOOL SvxAutoCorrectLanguageLists::DeleteText( const String& rShort )
{
    // erstmal akt. Liste besorgen!
    GetAutocorrWordList();

    MakeUserStorage_Impl();

    SfxMedium aMedium( sUserAutoCorrFile, STREAM_STD_READWRITE, TRUE );
    SvStorageRef xStg = aMedium.GetStorage();
    BOOL bRet = xStg.Is() && SVSTREAM_OK == xStg->GetError();
    if( bRet )
    {
        USHORT nPos;
        SvxAutocorrWord aTmp( rShort, rShort );
        if( pAutocorr_List->Seek_Entry( &aTmp, &nPos ) )
        {
            SvxAutocorrWord* pFnd = (*pAutocorr_List)[ nPos ];
            if( !pFnd->IsTextOnly() )
            {
                String aName( rShort );
                EncryptBlockName_Imp( aName );
                if( xStg->IsContained( aName ) )
                {
                    xStg->Remove( aName );
                    bRet = xStg->Commit();
                }
            }
            // die Wortliste aktualisieren
            pAutocorr_List->DeleteAndDestroy( nPos );
            MakeBlocklist_Imp( *xStg );
            xStg = 0;
            aMedium.Commit();
        }
        else
            bRet = FALSE;
    }
    return bRet;
}


