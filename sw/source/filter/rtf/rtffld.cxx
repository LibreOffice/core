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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include <ctype.h>
#include <hintids.hxx>

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#include <svl/urihelper.hxx>
#include <svtools/rtftoken.h>
#include <svl/zforlist.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/brkitem.hxx>
#include <fmtfld.hxx>
#include <fmtinfmt.hxx>
#include <swtypes.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <fldbas.hxx>
#include <swparrtf.hxx>
#include <txatbase.hxx>
#include <dbfld.hxx>
#include <usrfld.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <charfmt.hxx>
#ifndef _fmtruby_HXX
#include <fmtruby.hxx>
#endif
#include <breakit.hxx>
#include <reffld.hxx>
#include <SwStyleNameMapper.hxx>
#include <editeng/charhiddenitem.hxx>


// bestimme, ob es sich um ein IMPORT/TOC - Feld handelt.
// return:  0 - weder noch,
//          1 - TOC
//          2 - IMPORT
//          3 - INDEX
enum RTF_FLD_TYPES {
    RTFFLD_UNKNOWN = 0,
    RTFFLD_TOC,
    RTFFLD_IMPORT,
    RTFFLD_INDEX,
    RTFFLD_SYMBOL,
    RTFFLD_PAGE,
    RTFFLD_NUMPAGES,
    RTFFLD_DATE,
    RTFFLD_TIME,
    RTFFLD_DATA,
    RTFFLD_MERGEFLD,
    RTFFLD_HYPERLINK,
    RTFFLD_REF,
    RTFFLD_PAGEREF,
    RTFFLD_EQ,
    RTFFLD_INCLUDETEXT
};

static RTF_FLD_TYPES _WhichFld( String& rName, String& rNext )
{
    // Strings sind PascalStrings; Laenge steht an 1. Stellen, dadurch wird
    // sich der Aufruf von strlen erspart!!!
    sal_Char __READONLY_DATA sTOC[]=        "\x03""toc";
    sal_Char __READONLY_DATA sIMPORT[]=     "\x06""import";
    sal_Char __READONLY_DATA sINDEX[]=      "\x05""index";
    sal_Char __READONLY_DATA sSYMBOL[]=     "\x06""symbol";
    sal_Char __READONLY_DATA sPAGE[]=       "\x04""page";
    sal_Char __READONLY_DATA sNUMPAGES[]=   "\x08""numpages";
    sal_Char __READONLY_DATA sDATE[]=       "\x04""date";
    sal_Char __READONLY_DATA sTIME[]=       "\x04""time";
    sal_Char __READONLY_DATA sDATA[]=       "\x04""data";
    sal_Char __READONLY_DATA sMERGEFLD[]=   "\x0A""mergefield";
    sal_Char __READONLY_DATA sIMPORT2[]=    "\x0E""includepicture";
    sal_Char __READONLY_DATA sHYPERLINK[]=  "\x09""hyperlink";
    sal_Char __READONLY_DATA sREF[]=        "\x03""ref";
    sal_Char __READONLY_DATA sPAGEREF[]=    "\x07""pageref";
    sal_Char __READONLY_DATA sEQ[]=         "\x02""eq";
    sal_Char __READONLY_DATA sINCLUDETEXT[]="\x0B""includetext";

    struct _Dummy_RTF_FLD_TYPES
    {
        RTF_FLD_TYPES eFldType;
        const sal_Char* pFldNm;
    };
    __READONLY_DATA _Dummy_RTF_FLD_TYPES aFldNmArr[RTFFLD_INCLUDETEXT + 1] = {
            {RTFFLD_TOC,         sTOC},
            {RTFFLD_IMPORT,      sIMPORT},
            {RTFFLD_INDEX,       sINDEX},
            {RTFFLD_SYMBOL,      sSYMBOL},
            {RTFFLD_PAGE,        sPAGE},
            {RTFFLD_NUMPAGES,    sNUMPAGES},
            {RTFFLD_DATE,        sDATE},
            {RTFFLD_TIME,        sTIME},
            {RTFFLD_DATA,        sDATA},
            {RTFFLD_MERGEFLD,    sMERGEFLD},
            {RTFFLD_IMPORT,      sIMPORT2},
            {RTFFLD_HYPERLINK,   sHYPERLINK},
            {RTFFLD_REF,         sREF},
            {RTFFLD_PAGEREF,     sPAGEREF},
            {RTFFLD_EQ,          sEQ},
            {RTFFLD_INCLUDETEXT, sINCLUDETEXT}
    };


    if( !rName.Len() )
        return RTFFLD_UNKNOWN;

    String sNm( rName );
    sNm = sNm.EraseLeadingChars().GetToken(0, ' ');
    ASSERT( sNm.Len(), "Feldname hat keine Laenge!" );
    if( !sNm.Len() )
        return RTFFLD_UNKNOWN;

    xub_StrLen nTokenStt = rName.Search( sNm );
    sNm.ToLowerAscii();

    for (size_t n = 0; n < sizeof(aFldNmArr) / sizeof(aFldNmArr[0]); ++n)
    {
        const sal_Char* pCmp = aFldNmArr[n].pFldNm;
        int nLen = *pCmp++;
        xub_StrLen nFndPos = sNm.SearchAscii( pCmp );
        if( STRING_NOTFOUND != nFndPos &&
            ( !nFndPos || !isalpha(sNm.GetChar( static_cast< xub_StrLen >(nFndPos-1) )) ) &&
            ( nFndPos+nLen == sNm.Len() || !isalpha(sNm.GetChar( static_cast< xub_StrLen >(nFndPos+nLen) ) ) ) )
        {
//          rName = sNm.Copy( nFndPos, nLen );
            rName = rName.Copy( nFndPos, static_cast< xub_StrLen >(nLen) );
            nFndPos += nTokenStt + static_cast< xub_StrLen >(nLen);
            while( rNext.GetChar( nFndPos ) == ' ' )    ++nFndPos;
            rNext.Erase( 0, nFndPos );
            rNext.EraseTrailingChars();
            return aFldNmArr[n].eFldType;
        }
    }
    return RTFFLD_UNKNOWN;      // nichts gefunden.
}

static sal_uInt16 CheckNumberFmtStr( const String& rNStr )
{
    const static sal_Char* aNumberTypeTab[] =
    {
        "\x0A""ALPHABETIC",       /* CHARS_UPPER_LETTER*/
        "\x0A""alphabetic",       /* CHARS_LOWER_LETTER*/
        "\x05""ROMAN",            /* ROMAN_UPPER       */
        "\x05""roman",            /* ROMAN_LOWER       */
        "\x06""ARABIC",           /* ARABIC            */
        "\x04""NONE",             /* NUMBER_NONE       */
        "\x04""CHAR",             /* CHAR_SPECIAL      */
        "\x04""PAGE"              /* PAGEDESC          */
    };

    ASSERT(sizeof(aNumberTypeTab) / sizeof(sal_Char *)
           >= SVX_NUM_PAGEDESC - SVX_NUM_CHARS_UPPER_LETTER, "impossible");

    for (sal_uInt16 n = SVX_NUM_CHARS_UPPER_LETTER;  n <= SVX_NUM_PAGEDESC; ++n)
    {
        const sal_Char* pCmp = aNumberTypeTab[n - SVX_NUM_CHARS_UPPER_LETTER];
        int nLen = *pCmp++;
        if( rNStr.EqualsAscii( pCmp, 0, static_cast< xub_StrLen >(nLen) ))
            return static_cast< sal_uInt16 >(2 <= n ? n : (n + SVX_NUM_CHARS_UPPER_LETTER_N));
    }
    return SVX_NUM_PAGEDESC;        // default-Wert
}

class RtfFieldSwitch
{
    String sParam;
    xub_StrLen nCurPos;
public:
    RtfFieldSwitch( const String& rParam );
    sal_Unicode GetSwitch( String& rParam );

    sal_Bool IsAtEnd() const                { return nCurPos >= sParam.Len(); }
    xub_StrLen GetCurPos() const        { return nCurPos; }
    void Erase( xub_StrLen nEndPos )    { sParam.Erase( 0, nEndPos ); }
    void Insert( const String& rIns )   { sParam.Insert( rIns, 0 ); }
    const String& GetStr() const        { return sParam; }
};

RtfFieldSwitch::RtfFieldSwitch( const String& rParam )
    : sParam( rParam ), nCurPos( 0  )
{
    sParam.EraseTrailingChars().EraseLeadingChars();
}

sal_Unicode RtfFieldSwitch::GetSwitch( String& rParam )
{
    // beginnt ein Schalter?
    sal_Unicode c, cKey = 0;
    if( '\\' == (c = sParam.GetChar( nCurPos )) )
    {
        if( '\\' == ( c = sParam.GetChar( ++nCurPos )) )
            c = sParam.GetChar( ++nCurPos );

        cKey = c;

        while( ++nCurPos < sParam.Len() &&
                ' ' == ( c = sParam.GetChar( nCurPos )) )
            ;
    }

    // dann alles in Hochkommatas oder bis zum naechsten // als
    // Param returnen
    sal_uInt16 nOffset;
    if( '"' != c && '\'' != c )
        c = '\\', nOffset = 0;
    else
        nOffset = 1;

    sParam.Erase( 0, nCurPos + nOffset );
    rParam = sParam.GetToken( 0, c );
    sParam.Erase( 0, rParam.Len() + nOffset ).EraseLeadingChars();
    if( '\\' == c )
        rParam.EraseTrailingChars();
    nCurPos = 0;

    return cKey;
}

struct RTF_EquationData
{
    String sFontName, sUp, sDown, sText;
    sal_Int32 nJustificationCode, nFontSize, nUp, nDown, nStyleNo;

    inline RTF_EquationData()
        : nJustificationCode(0), nFontSize(0), nUp(0), nDown(0),
        nStyleNo( -1 )
    {}
};

xub_StrLen lcl_FindEndBracket( const String& rStr )
{
    xub_StrLen nEnd = rStr.Len(), nRet = STRING_NOTFOUND, nPos = 0;
    int nOpenCnt = 1;
    sal_Unicode cCh;
    for( ; nPos < nEnd; ++nPos )
        if( ')' == (cCh = rStr.GetChar( nPos )) && !--nOpenCnt )
        {
            nRet = nPos;
            break;
        }
        else if( '(' == cCh )
            ++nOpenCnt;

    return nRet;
}

void lcl_ScanEquationField( const String& rStr, RTF_EquationData& rData,
                            sal_Unicode nSttKey )
{
    int nSubSupFlag(0);
    RtfFieldSwitch aRFS( rStr );
    while( !aRFS.IsAtEnd() )
    {
        String sParam;
        sal_Unicode cKey = aRFS.GetSwitch( sParam );
        if( 1 == nSubSupFlag )
            ++nSubSupFlag;
        else if( 1 < nSubSupFlag )
            nSubSupFlag = 0;

        sal_Bool bCheckBracket = sal_False;
        switch( cKey )
        {
        case 0:
            switch( nSttKey )
            {
            case 'u':   rData.sUp += sParam;    break;
            case 'd':   rData.sDown += sParam;  break;
            default:    rData.sText += sParam;  break;
            }
            break;

        case '*':
            if( sParam.Len() )
            {
                if( sParam.EqualsIgnoreCaseAscii( "jc", 0, 2 ) )
                    rData.nJustificationCode = sParam.Copy( 2 ).ToInt32();
                else if( sParam.EqualsIgnoreCaseAscii( "hps", 0, 3 ) )
                    rData.nFontSize= sParam.Copy( 3 ).ToInt32();
                else if( sParam.EqualsIgnoreCaseAscii( "Font:", 0, 5 ) )
                    rData.sFontName = sParam.Copy( 5 );
                else if( sParam.EqualsIgnoreCaseAscii( "cs", 0, 2 ) )
                    rData.nStyleNo = sParam.Copy( 2 ).ToInt32();
            }
            break;
        case 's' :
            ++nSubSupFlag;
            break;

        case 'u':
            if( sParam.Len() && 'p' == sParam.GetChar( 0 ) &&
                2 == nSubSupFlag )
            {
                rData.nUp = sParam.Copy( 1 ).ToInt32();
                bCheckBracket = sal_True;
            }
            break;

        case 'd':
            if( sParam.Len() && 'o' == sParam.GetChar( 0 ) &&
                2 == nSubSupFlag )
            {
                rData.nDown = sParam.Copy( 1 ).ToInt32();
                bCheckBracket = sal_True;
            }
            break;

        default:
            bCheckBracket = sal_True;
            cKey = 0;
            break;
        }

        if( bCheckBracket && sParam.Len() )
        {
            xub_StrLen nEnd, nStt = sParam.Search( '(' ),
                        nLen = sParam.Len();
            if( STRING_NOTFOUND != nStt )
            {
                sParam.Erase( 0, nStt + 1 ) += aRFS.GetStr();
                if( STRING_NOTFOUND !=
                        (nEnd = ::lcl_FindEndBracket( sParam )) )
                {
                    // end in the added string?
                    if( (nLen - nStt - 1 ) < nEnd )
                        aRFS.Erase( nEnd + 1 - (nLen - nStt - 1));
                    else
                    {
                        // not all handled here, so set new into the RFS
                        aRFS.Insert( sParam.Copy( nEnd + 1,
                                                nLen - nStt - nEnd - 2 ));
                        sal_Unicode cCh;
                        if( aRFS.GetStr().Len() &&
                            ( ',' == (cCh = aRFS.GetStr().GetChar(0)) ||
                              ';' == cCh ))
                            aRFS.Erase( 1 );
                    }

                    ::lcl_ScanEquationField( sParam.Copy( 0, nEnd ),
                                                rData, cKey );
                }
            }
        }
    }
}

int SwRTFParser::MakeFieldInst( String& rFieldStr )
{
    // sicher den Original-String fuer die FeldNamen (User/Datenbank)
    String aSaveStr( rFieldStr );
    SwFieldType * pFldType;
    int nRet = _WhichFld(rFieldStr, aSaveStr);

    //Strip Mergeformat from fields
    xub_StrLen nPos=0;
    while (STRING_NOTFOUND != ( nPos = aSaveStr.SearchAscii("\\*", nPos)))
    {
        xub_StrLen nStartDel = nPos;
        nPos += 2;
        while (aSaveStr.GetChar(nPos) == ' ')
            ++nPos;
        if (aSaveStr.EqualsIgnoreCaseAscii("MERGEFORMAT", nPos, 11))
        {
            xub_StrLen nNoDel = (nPos + 11 ) - nStartDel;
            aSaveStr.Erase(nStartDel, nNoDel);
            nPos -= (nStartDel - nPos);
        }
    }

    nPos = 0;
    switch (nRet)
    {
    case RTFFLD_INCLUDETEXT:
        break;
    case RTFFLD_IMPORT:
        {
//JP 11.03.96: vertraegt sich nicht so ganz mit Internet!
//            if( STRING_NOTFOUND != ( nPos = aSaveStr.Search( '.' )))
//                aSaveStr.Erase( nPos+4 );

            aSaveStr.EraseLeadingAndTrailingChars();
            if( aSaveStr.Len() )
            {
                sal_Unicode c = aSaveStr.GetChar( 0 );
                if( '"' == c || '\'' == c )
                {
                    aSaveStr.Erase( 0, 1 );
                    aSaveStr = aSaveStr.GetToken( 0, c );
                }

                rFieldStr = URIHelper::SmartRel2Abs(
                    INetURLObject(GetBaseURL()), aSaveStr,
                    URIHelper::GetMaybeFileHdl() );
            }
//          SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_NUMPAGES:
        {
            SwDocStatField aFld( (SwDocStatFieldType*)pDoc->GetSysFldType( RES_DOCSTATFLD ),
                                  DS_PAGE, SVX_NUM_ARABIC );
            if( STRING_NOTFOUND != ( nPos = aSaveStr.SearchAscii( "\\*" )) )
            {
                nPos += 2;
                while( aSaveStr.GetChar(nPos) == ' ' ) nPos++;
                aSaveStr.Erase( 0, nPos );

                // steht jetzt geanu auf dem Format-Namen
                aFld.ChangeFormat( CheckNumberFmtStr( aSaveStr ));
            }
            pDoc->InsertPoolItem( *pPam, SwFmtFld( aFld ), 0 );
            SkipGroup();
        }
        break;

    case RTFFLD_PAGE:
        {
            pFldType = pDoc->GetSysFldType( RES_PAGENUMBERFLD );
            SwPageNumberField aPF( (SwPageNumberFieldType*)pFldType,
                                    PG_RANDOM, SVX_NUM_ARABIC);
            if( STRING_NOTFOUND != ( nPos = aSaveStr.SearchAscii( "\\*" )) )
            {
                nPos += 2;
                while( aSaveStr.GetChar(nPos) == ' ' ) nPos++;
                aSaveStr.Erase( 0, nPos );

                // steht jetzt geanu auf dem Format-Namen
                aPF.ChangeFormat( CheckNumberFmtStr( aSaveStr ));
            }
            pDoc->InsertPoolItem( *pPam, SwFmtFld( aPF ), 0 );
            SkipGroup();        // ueberlese den Rest
        }
        break;
    case RTFFLD_DATE:
    case RTFFLD_TIME:
        {
            if( STRING_NOTFOUND == ( nPos = aSaveStr.SearchAscii( "\\@" )) )
            {
                // es fehlt die Format - Angabe: defaulten auf Datum
                pFldType = pDoc->GetSysFldType( RES_DATETIMEFLD );
                pDoc->InsertPoolItem( *pPam, SwFmtFld( SwDateTimeField(
                    static_cast<SwDateTimeFieldType*>(pFldType), DATEFLD)), 0);
            }
            else
            {
                // versuche aus dem Formatstring zu erkennen, ob es ein
                // Datum oder Zeit oder Datum & Zeit Field ist
                // nur das Format interressiert
                aSaveStr.Erase( 0, aSaveStr.Search( '\"' )+1 );
                // alles hinter dem Format interressiert auch nicht mehr.
                aSaveStr.Erase( aSaveStr.Search( '\"' ) );
                aSaveStr.SearchAndReplaceAscii( "AM", aEmptyStr );
                aSaveStr.SearchAndReplaceAscii( "PM", aEmptyStr );

                // #117892# M.M. Put the word date and time formatter stuff in a common area
                // and get the rtf filter to use it
                SwField *pFld = 0;
                short nNumFmtType = NUMBERFORMAT_UNDEFINED;
                sal_uLong nFmtIdx = NUMBERFORMAT_UNDEFINED;

                sal_uInt16 rLang(0);
                RES_CHRATR eLang = maPageDefaults.mbRTLdoc ? RES_CHRATR_CTL_LANGUAGE : RES_CHRATR_LANGUAGE;
                const SvxLanguageItem *pLang = (SvxLanguageItem*)&pDoc->GetAttrPool().GetDefaultItem( static_cast< sal_uInt16 >(eLang) );
                rLang = pLang ? pLang->GetValue() : LANGUAGE_ENGLISH_US;

                SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
                bool bHijri = false;

                if( pFormatter )
                {
                    nFmtIdx = sw::ms::MSDateTimeFormatToSwFormat(aSaveStr, pFormatter, rLang, bHijri);
                    if (nFmtIdx)
                        nNumFmtType = pFormatter->GetType(nFmtIdx);
                }

                pFldType = pDoc->GetSysFldType( RES_DATETIMEFLD );

                if(nNumFmtType & NUMBERFORMAT_DATE)
                    pFld = new SwDateTimeField( (SwDateTimeFieldType*)pFldType, DATEFLD, nFmtIdx );
                else if(nNumFmtType == NUMBERFORMAT_TIME)
                    pFld = new SwDateTimeField( (SwDateTimeFieldType*)pFldType, TIMEFLD, nFmtIdx );

                if( pFld )
                {
                    pDoc->InsertPoolItem( *pPam, SwFmtFld( *pFld ), 0);
                    delete pFld;
                }
            }
            SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_DATA:
        {
            // Datenbank-FileName: nur der Filename interressiert
            // Zur Zeit werden nur SDF-Files verarbeitet, also suche nach
            // der Extension

            // im SWG geben die DATA Felder den Namen der Datenbank
            // an. Dieser kann als Field oder als DBInfo interpretiert
            // werden:
            //  \\data -> Datenbank-Name als Field
            //  DATA -> Datenbank-Info
            sal_Bool bField = rFieldStr.GetChar( 0 ) != 'D';

            // nur der Name interressiert
            if( STRING_NOTFOUND != (nPos = aSaveStr.Search( '.' )) )
                aSaveStr.Erase( nPos );
            SwDBData aData;
            aData.sDataSource = aSaveStr;
            if( bField )
            {
                pFldType = pDoc->GetSysFldType( RES_DBNAMEFLD );
                pDoc->InsertPoolItem( *pPam, SwFmtFld( SwDBNameField(
                    static_cast<SwDBNameFieldType*>(pFldType), SwDBData())), 0);
            }
            else
                pDoc->ChgDBData( aData );       // MS: Keine DBInfo verwenden
            SkipGroup();        // ueberlese den Rest
        }
        break;
    case RTFFLD_MERGEFLD:
        {
            // ein Datenbank - Feld: nur der Name interressiert
            // bis zum Ende vom String ist das der Feldname
            SwDBFieldType aTmp( pDoc, aSaveStr, SwDBData() );   //
            SwDBField aDBFld( (SwDBFieldType*)pDoc->InsertFldType( aTmp ));

            aDBFld.ChangeFormat( UF_STRING );
            pDoc->InsertPoolItem(*pPam, SwFmtFld( aDBFld ), 0);
            SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_SYMBOL:
        {
            // loesche fuehrende Blanks
            if( IsNewGroup() )
                GetAttrSet();
            SetNewGroup( sal_True );

            SfxItemSet& rSet = GetAttrSet();

            sal_Bool bCharIns = sal_False;
            RtfFieldSwitch aRFS( aSaveStr );
            while( !aRFS.IsAtEnd() )
            {
                String sParam;
                sal_Unicode cKey = aRFS.GetSwitch( sParam );
                if( sParam.Len() )
                    switch( cKey )
                    {
                    case 0:
                        if( !bCharIns )
                        {
                            sal_Unicode nChar = (sal_Unicode)sParam.ToInt32();
                            if( nChar )
                            {
                                pDoc->InsertString( *pPam, nChar );
                                bCharIns = sal_True;
                            }
                        }
                        break;

                    case 'f': case 'F':
                        // Font setzen
                        {
                            SvxRTFFontTbl& rTbl = GetFontTbl();
                            for( Font* pFont = rTbl.First(); pFont;
                                    pFont = rTbl.Next() )
                                if( pFont->GetName() == sParam )
                                {
                                    rSet.Put( SvxFontItem(
                                            pFont->GetFamily(),
                                            sParam,
                                            pFont->GetStyleName(),
                                            pFont->GetPitch(),
                                            pFont->GetCharSet(),
                                            RES_CHRATR_FONT ));
                                    break;
                                }
                        }
                        break;
                    case 'h': case 'H':
                        //??
                        break;
                    case 's': case 'S':
                        // Fontsize setzen
                        {
                            const sal_uInt16 nVal = (sal_uInt16)(sParam.ToInt32() * 20);
                            rSet.Put( SvxFontHeightItem( nVal,
                                                100, RES_CHRATR_FONTSIZE ));
                        }
                        break;
                    }
            }

            if( !IsNewGroup() ) AttrGroupEnd();
            SetNewGroup( sal_False );

            SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_HYPERLINK:
        rFieldStr.Erase();
        if( aSaveStr.Len() )
        {
            // return String ist URL, # Mark, \1 Frame
            String sMark, sFrame;
            RtfFieldSwitch aRFS( aSaveStr );
            while( !aRFS.IsAtEnd() )
            {
                String sParam;
                sal_Unicode cKey = aRFS.GetSwitch( sParam );
                if( sParam.Len() )
                    switch( cKey )
                    {
                    case 0:
                        if( !rFieldStr.Len() )
                            rFieldStr = URIHelper::SmartRel2Abs(
                                INetURLObject(GetBaseURL()), sParam,
                                URIHelper::GetMaybeFileHdl() );
                        break;

                    case 'l':   case 'L':   sMark = sParam;     break;
                    case 't':   case 'T':   sFrame = sParam;    break;
                    }
            }

            if( sMark.Len() )
                ( rFieldStr += INET_MARK_TOKEN ) += sMark;
            if( sFrame.Len() )
                ( rFieldStr += '\1' ) += sFrame;
        }
        break;

    case RTFFLD_EQ:
        rFieldStr.Erase();
        if( aSaveStr.Len() )
        {
            RTF_EquationData aData;
            ::lcl_ScanEquationField( aSaveStr, aData, 0 );

            // is it a ruby attr?
            if( aData.sText.Len() && aData.sFontName.Len() &&
                aData.nFontSize && aData.sUp.Len() && !aData.sDown.Len() )
            {
                //Translate and apply
                switch( aData.nJustificationCode )
                {
                case 0:     aData.nJustificationCode = 1;   break;
                case 1:     aData.nJustificationCode = 3;   break;
                case 2:     aData.nJustificationCode = 4;   break;
                case 4:     aData.nJustificationCode = 2;   break;
//              case 3:
                default:    aData.nJustificationCode = 0;   break;
                }

                SwFmtRuby aRuby( aData.sUp );
                SwCharFmt * pCharFmt = -1 != aData.nStyleNo
                                          ? aCharFmtTbl.Get( aData.nStyleNo )
                                          : 0;

                if( !pCharFmt )
                {
                    //Make a guess at which of asian of western we should be setting
                    sal_uInt16 nScript;
                    if (pBreakIt->GetBreakIter().is())
                        nScript = pBreakIt->GetBreakIter()->getScriptType( aData.sUp, 0);
                    else
                        nScript = i18n::ScriptType::ASIAN;

                    sal_uInt16 nFntHWhich = GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ),
                           nFntWhich = GetWhichOfScript( RES_CHRATR_FONT, nScript );

                    //Check to see if we already have a ruby charstyle that this fits
                    for(sal_uInt16 i=0; i < aRubyCharFmts.Count(); ++i )
                    {
                        SwCharFmt *pFmt = (SwCharFmt *)aRubyCharFmts[i];
                        const SvxFontHeightItem &rF = (const SvxFontHeightItem &)
                                                    pFmt->GetFmtAttr( nFntHWhich );
                        if( rF.GetHeight() == sal_uInt16(aData.nFontSize * 10 ))
                        {
                            const SvxFontItem &rFI = (const SvxFontItem &)
                                                    pFmt->GetFmtAttr( nFntWhich );
                            if( rFI.GetFamilyName().Equals( aData.sFontName ))
                            {
                                pCharFmt = pFmt;
                                break;
                            }
                        }
                    }

                    //Create a new char style if necessary
                    if( !pCharFmt )
                    {
                        String sNm;
                        //Take this as the base name
                        SwStyleNameMapper::FillUIName( RES_POOLCHR_RUBYTEXT, sNm );
                        sNm += String::CreateFromInt32( aRubyCharFmts.Count() + 1 );
                        pCharFmt = pDoc->MakeCharFmt( sNm,
                                            ( SwCharFmt*)pDoc->GetDfltCharFmt() );

                        SvxFontHeightItem aHeightItem( aData.nFontSize * 10, 100, RES_CHRATR_FONTSIZE );
                        aHeightItem.SetWhich( nFntHWhich );

                        SvxFontItem aFontItem( FAMILY_DONTKNOW, aData.sFontName,
                            aEmptyStr, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, nFntWhich );

                        pCharFmt->SetFmtAttr( aHeightItem );
                        pCharFmt->SetFmtAttr( aFontItem );
                        void* p = pCharFmt;
                        aRubyCharFmts.Insert( p, aRubyCharFmts.Count() );
                    }
                }

                //Set the charstyle and justification
                aRuby.SetCharFmtName( pCharFmt->GetName() );
                aRuby.SetCharFmtId( pCharFmt->GetPoolFmtId() );
                aRuby.SetAdjustment( (sal_uInt16)aData.nJustificationCode );

                // im FieldStr steht der anzuzeigenden Text, im
                pDoc->InsertString( *pPam, aData.sText );
                pPam->SetMark();
                pPam->GetMark()->nContent -= aData.sText.Len();
                pDoc->InsertPoolItem( *pPam, aRuby,
                    nsSetAttrMode::SETATTR_DONTEXPAND );
                pPam->DeleteMark();
            }
            // or a combined character field?
            else if( aData.sUp.Len() && aData.sDown.Len() &&
                    !aData.sText.Len() && !aData.sFontName.Len() &&
                    !aData.nFontSize )
            {
                String sFld( aData.sUp );
                sFld += aData.sDown;
                SwCombinedCharField aFld((SwCombinedCharFieldType*)pDoc->
                                GetSysFldType( RES_COMBINED_CHARS ), sFld );
                pDoc->InsertPoolItem( *pPam, SwFmtFld( aFld ), 0);

            }
            SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_PAGEREF:
        {
            String sOrigBkmName;
            RtfFieldSwitch aRFS( aSaveStr );
            while( !aRFS.IsAtEnd() )
            {
                String sParam;
                sal_Unicode cKey = aRFS.GetSwitch( sParam );
                switch( cKey )
                {
                    // In the case of pageref the only parameter we are
                    // interested in, is the name of the bookmark
                    case 0:
                        if( !sOrigBkmName.Len() ) // get name of bookmark
                            sOrigBkmName = sParam;
                        break;
                }
            }
            SwGetRefField aFld(
                    (SwGetRefFieldType*)pDoc->GetSysFldType( RES_GETREFFLD ),
                    sOrigBkmName,REF_BOOKMARK,0,REF_PAGE);

            if(!bNestedField)
            {
                pDoc->InsertPoolItem( *pPam, SwFmtFld( aFld ), 0 );
            }
            else
                bNestedField = false;
        }
        break;

    case RTFFLD_REF:
        {
            String sOrigBkmName;
            bool bChapterNr = false;
            bool bAboveBelow = false;

            RtfFieldSwitch aRFS( aSaveStr );
            while( !aRFS.IsAtEnd() )
            {
                String sParam;
                sal_Unicode cKey = aRFS.GetSwitch( sParam );
                switch( cKey )
                {
                    case 0:
                        if( !sOrigBkmName.Len() ) // get name of bookmark
                            sOrigBkmName = sParam;
                        break;

                    case 'n':
                    case 'r':
                    case 'w':
                        bChapterNr = true; // activate flag 'Chapter Number'
                        break;

                    case 'p':
                        bAboveBelow = true;
                        break;
                }
            }
            if (!bAboveBelow || bChapterNr)
            {
                if (bChapterNr)
                {
                    SwGetRefField aFld(
                        (SwGetRefFieldType*)pDoc->GetSysFldType( RES_GETREFFLD ),
                        sOrigBkmName,REF_BOOKMARK,0,REF_CHAPTER);
                    pDoc->InsertPoolItem( *pPam, SwFmtFld( aFld ), 0 );
                }
                else
                {
                    SwGetRefField aFld(
                        (SwGetRefFieldType*)pDoc->GetSysFldType( RES_GETREFFLD ),
                        sOrigBkmName,REF_BOOKMARK,0,REF_CONTENT);
                    pDoc->InsertPoolItem( *pPam, SwFmtFld( aFld ), 0 );
                }
            }

            if( bAboveBelow )
            {
                SwGetRefField aFld( (SwGetRefFieldType*)
                    pDoc->GetSysFldType( RES_GETREFFLD ), sOrigBkmName, REF_BOOKMARK, 0,
                    REF_UPDOWN );
                pDoc->InsertPoolItem(*pPam, SwFmtFld(aFld), 0);
            }
        }
        break;

    case RTFFLD_TOC:
    case RTFFLD_INDEX:
        break;

    default:
        {
            // keines von den bekannten Feldern, also eine neues UserField
            aSaveStr.EraseLeadingChars().EraseTrailingChars();
            SwUserFieldType aTmp( pDoc, aSaveStr );
            SwUserField aUFld( (SwUserFieldType*)pDoc->InsertFldType( aTmp ));
            aUFld.ChangeFormat( UF_STRING );
            pDoc->InsertPoolItem( *pPam, SwFmtFld( aUFld ), 0);
            nRet = RTFFLD_UNKNOWN;
        }
        break;
    }
    return nRet;
}


void SwRTFParser::ReadXEField()
{
    bReadSwFly = false; //#it may be that any uses of this need to be removed and replaced
    int nNumOpenBrakets = 1;
    String sFieldStr;
    sal_uInt8 cCh;

    int nToken;
    while (nNumOpenBrakets && IsParserWorking())
    {
        switch (nToken = GetNextToken())
        {
        case '}':
            {
                --nNumOpenBrakets;

                if( sFieldStr.Len())
                {
                    String sXE(sFieldStr);
                    sXE.Insert('\"', 0);
                    sXE.Append('\"');

                    // we have to make sure the hidden text flag is not on
                    // otherwise the index will not see this index mark
                    SfxItemSet& rSet = GetAttrSet();
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_HIDDEN, sal_True, &pItem ) )
                    {
                        SvxCharHiddenItem aCharHidden(*(SvxCharHiddenItem*)pItem);
                        aCharHidden.SetValue(sal_False);
                        rSet.Put(aCharHidden);
                    }

                    sw::ms::ImportXE(*pDoc, *pPam, sXE);

                    sFieldStr.Erase();
                }
            }
            break;

        case '{':
            if( RTF_IGNOREFLAG != GetNextToken() )
                SkipToken( -1 );
            // Unknown und alle bekannten nicht ausgewerteten Gruppen
            // sofort ueberspringen
            else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                SkipToken( -2 );
            else
            {
                // gleich herausfiltern
                ReadUnknownData();
                if( '}' != GetNextToken() )
                    eState = SVPAR_ERROR;
                break;
            }
            ++nNumOpenBrakets;
            break;

        case RTF_U:
            {
                if( nTokenValue )
                    sFieldStr += (sal_Unicode)nTokenValue;
                else
                    sFieldStr += aToken;
            }
            break;

        case RTF_LINE:          cCh = '\n'; goto INSINGLECHAR;
        case RTF_TAB:           cCh = '\t'; goto INSINGLECHAR;
        case RTF_SUBENTRYINDEX: cCh = ':';  goto INSINGLECHAR;
        case RTF_EMDASH:        cCh = 151;  goto INSINGLECHAR;
        case RTF_ENDASH:        cCh = 150;  goto INSINGLECHAR;
        case RTF_BULLET:        cCh = 149;  goto INSINGLECHAR;
        case RTF_LQUOTE:        cCh = 145;  goto INSINGLECHAR;
        case RTF_RQUOTE:        cCh = 146;  goto INSINGLECHAR;
        case RTF_LDBLQUOTE:     cCh = 147;  goto INSINGLECHAR;
        case RTF_RDBLQUOTE:     cCh = 148;  goto INSINGLECHAR;
INSINGLECHAR:
            sFieldStr += ByteString::ConvertToUnicode( cCh,
                                               RTL_TEXTENCODING_MS_1252 );
            break;

        // kein Break, aToken wird als Text gesetzt
        case RTF_TEXTTOKEN:
            sFieldStr += aToken;
            break;

        case RTF_BKMK_KEY:
        case RTF_TC:
        case RTF_NEXTFILE:
        case RTF_TEMPLATE:
        case RTF_SHPRSLT:
            SkipGroup();
            break;

        case RTF_PAR:
            sFieldStr.Append('\x0a');
            break;
        default:
            SvxRTFParser::NextToken( nToken );
            break;
        }
    }

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}


void SwRTFParser::ReadField()
{
    bReadSwFly = false; //#it may be that any uses of this need to be removed and replaced
    int nRet = 0;
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!
    int bFldInst = sal_False, bFldRslt = sal_False;
    String sFieldStr, sFieldNm;
    sal_uInt8 cCh;

    int nToken;
    while (nNumOpenBrakets && IsParserWorking())
    {
        switch (nToken = GetNextToken())
        {
        case '}':
            {
                --nNumOpenBrakets;
                if( 1 != nNumOpenBrakets || !bFldInst )
                    break;

                if( !bFldRslt )
                {
                    // FieldInst vollstaendig eingelesen, was ist es denn?
                    nRet = MakeFieldInst( sFieldStr );
                    switch ( nRet )
                    {
                    case RTFFLD_INCLUDETEXT:
                    case RTFFLD_TOC:
                    case RTFFLD_INDEX:
                        // erstmal Index/Inhaltsverzeichniss ueberspringen
                        // und als normalen Text einfuegen. Spaeter mal auch dem
                        // SwPaM darum aufspannen.
                        return ;

                    case RTFFLD_IMPORT:
                    case RTFFLD_HYPERLINK:
                        sFieldNm = sFieldStr;
                        break;
                    }
                    sFieldStr.Erase();
                }
                else if (RTFFLD_UNKNOWN == nRet)
                {
                    // FieldResult wurde eingelesen
                    if (SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode())
                    {
                        SwTxtAttr* const pFldAttr =
                            pTxtNd->GetTxtAttrForCharAt(
                                pPam->GetPoint()->nContent.GetIndex()-1 );

                        if (pFldAttr)
                        {
                            const SwField *pFld = pFldAttr->GetFld().GetFld();
                            SwFieldType *pTyp = pFld ? pFld->GetTyp() : 0;
                            ASSERT(pTyp->Which() == RES_USERFLD, "expected a user field");
                            if (pTyp->Which() == RES_USERFLD)
                            {
                                SwUserFieldType *pUsrTyp = (SwUserFieldType*)pTyp;
                                pUsrTyp->SetContent(sFieldStr);
                            }
                        }
                    }
                }
                else if( sFieldNm.Len() )
                {
                    switch ( nRet )
                    {
                    case RTFFLD_IMPORT:
                        // Grafik einfuegen
                        InsPicture( sFieldNm );
                        nRet = INT_MAX;
                        break;
                    case RTFFLD_HYPERLINK:
                        if( sFieldStr.Len() )
                        {
                            if(sNestedFieldStr.Len())
                                sFieldStr.Insert(sNestedFieldStr);

                            sNestedFieldStr.Erase();
                            // im FieldStr steht der anzuzeigenden Text, im
                            pDoc->InsertString( *pPam, sFieldStr );

                            String sTarget( sFieldNm.GetToken( 1, '\1' ));
                            if( sTarget.Len() )
                                sFieldNm.Erase( sFieldNm.Len() - sTarget.Len() -1 );

                            // oder ueber den Stack setzen??
                            pPam->SetMark();
                            pPam->GetMark()->nContent -= sFieldStr.Len();
                            pDoc->InsertPoolItem( *pPam,
                                            SwFmtINetFmt( sFieldNm, sTarget ),
                                            nsSetAttrMode::SETATTR_DONTEXPAND );
                            pPam->DeleteMark();

                        }
                        break;
                    }
                }
                else if(bNestedField)
                {
                    if(nRet == RTFFLD_PAGEREF)
                    {
                        // #17371 Nasty hack to get a pageref within a hyperlink working
                        sNestedFieldStr = sFieldStr;
                    }

                }

            }
            break;

        case '{':
            if( RTF_IGNOREFLAG != GetNextToken() )
                SkipToken( -1 );
            // Unknown und alle bekannten nicht ausgewerteten Gruppen
            // sofort ueberspringen
            else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                SkipToken( -2 );
            else
            {
                // gleich herausfiltern
                ReadUnknownData();
                if( '}' != GetNextToken() )
                    eState = SVPAR_ERROR;
                break;
            }
            ++nNumOpenBrakets;
            break;

        case RTF_DATAFIELD:
            SkipGroup();
            break;

        case RTF_FIELD:
            bNestedField = true;
            ReadField();
            break;

        case RTF_FLDINST:
            bFldInst = sal_True;
            break;

        case RTF_FLDRSLT:
            bFldRslt = sal_True;
            break;

        case RTF_U:
            {
                if( nTokenValue )
                    sFieldStr += (sal_Unicode)nTokenValue;
                else
                    sFieldStr += aToken;
            }
            break;

        case RTF_LINE:          cCh = '\n'; goto INSINGLECHAR;
        case RTF_TAB:           cCh = '\t'; goto INSINGLECHAR;
        case RTF_SUBENTRYINDEX: cCh = ':';  goto INSINGLECHAR;
        case RTF_EMDASH:        cCh = 151;  goto INSINGLECHAR;
        case RTF_ENDASH:        cCh = 150;  goto INSINGLECHAR;
        case RTF_BULLET:        cCh = 149;  goto INSINGLECHAR;
        case RTF_LQUOTE:        cCh = 145;  goto INSINGLECHAR;
        case RTF_RQUOTE:        cCh = 146;  goto INSINGLECHAR;
        case RTF_LDBLQUOTE:     cCh = 147;  goto INSINGLECHAR;
        case RTF_RDBLQUOTE:     cCh = 148;  goto INSINGLECHAR;
INSINGLECHAR:
            sFieldStr += ByteString::ConvertToUnicode( cCh,
                                               RTL_TEXTENCODING_MS_1252 );
            break;

        // kein Break, aToken wird als Text gesetzt
        case RTF_TEXTTOKEN:
            sFieldStr += aToken;
            break;

        case RTF_PICT:      // Pic-Daten einlesen!
            if( RTFFLD_IMPORT == nRet )
            {
                Graphic aGrf;
                SvxRTFPictureType aPicType;
                if( ReadBmpData( aGrf, aPicType ) )
                {
                    InsPicture( sFieldNm, &aGrf, &aPicType );
                    nRet = INT_MAX;
                }
                SkipGroup();
            }
            break;

        case RTF_BKMK_KEY:
        case RTF_XE:
        case RTF_TC:
        case RTF_NEXTFILE:
        case RTF_TEMPLATE:
        case RTF_SHPRSLT:
            SkipGroup();
            break;

        case RTF_CS:
            // we write every time "EQ "
            if( bFldInst && 0 == sFieldStr.SearchAscii( "EQ " ))
            {
                // insert behind the EQ the "\*cs<NO> " string. This is utilize
                // in the MakeFieldInst
                String sTmp;
                (sTmp.AssignAscii( "\\* cs" )
                    += String::CreateFromInt32( nTokenValue )) += ' ';
                sFieldStr.Insert( sTmp, 3 );
            }
            break;
        case RTF_FFNAME:
        case RTF_FORMFIELD:
            break;
        case RTF_PAR:
            sFieldStr.Append('\x0a');
            break;
        default:
            SvxRTFParser::NextToken( nToken );
            break;
        }
    }

    // Grafik einfuegen
    if (RTFFLD_IMPORT == nRet && sFieldNm.Len())
        InsPicture( sFieldNm );

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
