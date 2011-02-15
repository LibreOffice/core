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
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif
#include <hintids.hxx>
#include <rtl/textenc.h>
#include <i18npool/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/lingucfg.hxx>
#include <fontcfg.hxx>
#include <swmodule.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/printer.hxx>
#include <editeng/flstitem.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <swtypes.hxx>
#include <ascfldlg.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <errhdl.hxx>

#ifndef _DIALOG_HRC
#include <dialog.hrc>
#endif
#ifndef _ASCFLDLG_HRC
#include <ascfldlg.hrc>
#endif

#include "vcl/metric.hxx"


using namespace ::com::sun::star;

const sal_Unicode cDialogExtraDataClose = '}';
const char __FAR_DATA sDialogImpExtraData[] = "EncImpDlg:{";
const char __FAR_DATA sDialogExpExtraData[] = "EncExpDlg:{";
const sal_uInt16 nDialogExtraDataLen = 11;      // 12345678901

SwAsciiFilterDlg::SwAsciiFilterDlg( Window* pParent, SwDocShell& rDocSh,
                                    SvStream* pStream )
    : SfxModalDialog( pParent, SW_RES( DLG_ASCII_FILTER )),
    aFL( this, SW_RES( FL_1 )),
    aCharSetFT( this, SW_RES( FT_CHARSET )),
    aCharSetLB( this, SW_RES( LB_CHARSET )),
    aFontFT( this, SW_RES( FT_FONT )),
    aFontLB( this, SW_RES( LB_FONT )),
    aLanguageFT( this, SW_RES( FT_LANGUAGE )),
    aLanguageLB( this, SW_RES( LB_LANGUAGE )),
    aCRLF_FT( this, SW_RES( FT_CRLF )),
    aCRLF_RB( this, SW_RES( RB_CRLF )),
    aCR_RB( this, SW_RES( RB_CR )),
    aLF_RB( this, SW_RES( RB_LF )),
    aOkPB( this, SW_RES( PB_OK )),
    aCancelPB( this, SW_RES( PB_CANCEL )),
    aHelpPB( this, SW_RES( PB_HELP )),
    sSystemCharSet( SW_RES( STR_SYS_CHARSET )),
    bSaveLineStatus( sal_True )
{
    FreeResource();

    SwAsciiOptions aOpt;
    {
        const String& rFindNm = String::CreateFromAscii(
                                    pStream ? sDialogImpExtraData
                                              : sDialogExpExtraData);
        sal_uInt16 nEnd, nStt = GetExtraData().Search( rFindNm );
        if( STRING_NOTFOUND != nStt )
        {
            nStt += nDialogExtraDataLen;
            nEnd = GetExtraData().Search( cDialogExtraDataClose, nStt );
            if( STRING_NOTFOUND != nEnd )
            {
                aOpt.ReadUserData( GetExtraData().Copy( nStt, nEnd - nStt ));
                nStt -= nDialogExtraDataLen;
                GetExtraData().Erase( nStt, nEnd - nStt + 1 );
            }
        }
    }

    // read the first chars and check the charset, (language - with L&H)
    if( pStream )
    {
        char aBuffer[ 4098 ];
        sal_uLong nOldPos = pStream->Tell();
        sal_uLong nBytesRead = pStream->Read( aBuffer, 4096 );
        pStream->Seek( nOldPos );

        if( nBytesRead <= 4096 )
        {
            aBuffer[ nBytesRead ] = '0';
            aBuffer[ nBytesRead+1 ] = '0';
            if( 0 != ( nBytesRead & 0x00000001 ) )
                aBuffer[ nBytesRead + 2 ] = '0';
        }

        sal_Bool bCR = sal_False, bLF = sal_False, bNoNormalChar = sal_False,
            bNullChar = sal_False;
        for( sal_uInt16 nCnt = 0; nCnt < nBytesRead; ++nCnt )
            switch( aBuffer[ nCnt ] )
            {
                case 0x0:   bNullChar = sal_True; break;
                case 0xA:   bLF = sal_True; break;
                case 0xD:   bCR = sal_True; break;
                case 0xC:
                case 0x1A:
                case 0x9:   break;
                default:
                    if( 0x20 > aBuffer[ nCnt ] )
                        bNoNormalChar = sal_True;
            }

        if( !bNullChar )
        {
            if( bCR )
            {
                if( bLF )
                {
                    aOpt.SetParaFlags( LINEEND_CRLF );
// have to check if of CharSet is type of ANSI
//                  aOpt.SetCharSet( CHARSET_ANSI );
                }
                else
                {
                    aOpt.SetParaFlags( LINEEND_CR );
// have to check if CharSet is type of MAC
//                  aOpt.SetCharSet( CHARSET_MAC );
                }
            }
            else if( bLF )
            {
                aOpt.SetParaFlags( LINEEND_LF );
// have to check if of CharSet is type of ANSI
//              aOpt.SetCharSet( CHARSET_ANSI );
            }
        }

        SwDoc* pDoc = rDocSh.GetDoc();

        sal_uInt16 nAppScriptType = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
        {
            sal_Bool bDelPrinter = sal_False;
            SfxPrinter* pPrt = pDoc ? pDoc->getPrinter(false) : 0;
            if( !pPrt )
            {
                SfxItemSet* pSet = new SfxItemSet( rDocSh.GetPool(),
                            SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                            0 );
                pPrt = new SfxPrinter( pSet );
                bDelPrinter = sal_True;
            }


            // get the set of disctinct available family names
            std::set< String > aFontNames;
            int nFontNames = pPrt->GetDevFontCount();
            for( int i = 0; i < nFontNames; i++ )
            {
                FontInfo aInf( pPrt->GetDevFont( i ) );
                aFontNames.insert( aInf.GetName() );
            }

            // insert to listbox
            for( std::set< String >::const_iterator it = aFontNames.begin();
                 it != aFontNames.end(); ++it )
            {
                aFontLB.InsertEntry( *it );
            }

            if( !aOpt.GetFontName().Len() )
            {
                if(pDoc)
                {
                    sal_uInt16 nFontRes = RES_CHRATR_FONT;
                    if(SCRIPTTYPE_ASIAN == nAppScriptType)
                        nFontRes = RES_CHRATR_CJK_FONT;
                    else if(SCRIPTTYPE_COMPLEX == nAppScriptType)
                        nFontRes = RES_CHRATR_CTL_FONT;

                    aOpt.SetFontName( ((SvxFontItem&)pDoc->GetDefault(
                                    nFontRes )).GetFamilyName() );
                }
                else
                {
                    sal_uInt16 nFontType = FONT_STANDARD;
                    if(SCRIPTTYPE_ASIAN == nAppScriptType)
                        nFontType = FONT_STANDARD_CJK;
                    else if(SCRIPTTYPE_COMPLEX == nAppScriptType)
                        nFontType = FONT_STANDARD_CTL;
                    aOpt.SetFontName(SW_MOD()->GetStdFontConfig()->GetFontFor(nFontType));
                }
            }
            aFontLB.SelectEntry( aOpt.GetFontName() );

            if( bDelPrinter )
                delete pPrt;
        }

        // initialisiere Sprache
        {
            if( !aOpt.GetLanguage() )
            {
                if(pDoc)
                {
                    sal_uInt16 nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
                    aOpt.SetLanguage( ((SvxLanguageItem&)pDoc->
                                GetDefault( nWhich )).GetLanguage());
                }
                else
                {
                    SvtLinguOptions aLinguOpt;
                    SvtLinguConfig().GetOptions( aLinguOpt );
                    switch(nAppScriptType)
                    {
                        case SCRIPTTYPE_ASIAN:
                            aOpt.SetLanguage(MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, SCRIPTTYPE_ASIAN));
                        break;
                        case SCRIPTTYPE_COMPLEX:
                            aOpt.SetLanguage(MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, SCRIPTTYPE_COMPLEX));
                        break;
                        //SCRIPTTYPE_LATIN:
                        default:
                            aOpt.SetLanguage(MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, SCRIPTTYPE_LATIN));
                    }
                }
            }

            aLanguageLB.SetLanguageList( LANG_LIST_ALL, sal_True, sal_False );
            aLanguageLB.SelectLanguage( aOpt.GetLanguage() );
        }
    }
    else
    {
        // hide the used Control for the Export and move the
        // other behind the charset controls
        aFontFT.Hide();
        aFontLB.Hide();
        aLanguageFT.Hide();
        aLanguageLB.Hide();

        long nY = aFontFT.GetPosPixel().Y() + 1;
        Point aPos( aCRLF_FT.GetPosPixel() );   aPos.Y() = nY;
        aCRLF_FT.SetPosPixel( aPos );

        aPos = aCRLF_RB.GetPosPixel();  aPos.Y() = nY;
        aCRLF_RB.SetPosPixel( aPos );

        aPos = aCR_RB.GetPosPixel();    aPos.Y() = nY;
        aCR_RB.SetPosPixel( aPos );

        aPos = aLF_RB.GetPosPixel();    aPos.Y() = nY;
        aLF_RB.SetPosPixel( aPos );

        Size aSize = GetSizePixel();
        Size aTmpSz( 6, 6 );
        aTmpSz = LogicToPixel(aTmpSz, MAP_APPFONT);
        aSize.Height() = aHelpPB.GetPosPixel().Y() +
                         aHelpPB.GetSizePixel().Height() + aTmpSz.Height();
        SetSizePixel( aSize );
    }

    // initialisiere Zeichensatz
    aCharSetLB.FillFromTextEncodingTable( pStream != NULL );
    aCharSetLB.SelectTextEncoding( aOpt.GetCharSet()  );

    aCharSetLB.SetSelectHdl( LINK( this, SwAsciiFilterDlg, CharSetSelHdl ));
    aCRLF_RB.SetToggleHdl( LINK( this, SwAsciiFilterDlg, LineEndHdl ));
    aLF_RB.SetToggleHdl( LINK( this, SwAsciiFilterDlg, LineEndHdl ));
    aCR_RB.SetToggleHdl( LINK( this, SwAsciiFilterDlg, LineEndHdl ));

    SetCRLF( aOpt.GetParaFlags() );

    aCRLF_RB.SaveValue();
    aLF_RB.SaveValue();
    aCR_RB.SaveValue();
}


SwAsciiFilterDlg::~SwAsciiFilterDlg()
{
}


void SwAsciiFilterDlg::FillOptions( SwAsciiOptions& rOptions )
{
    sal_uLong nCCode = aCharSetLB.GetSelectTextEncoding();
    String sFont;
    sal_uLong nLng = 0;
    if( aFontLB.IsVisible() )
    {
        sFont = aFontLB.GetSelectEntry();
        nLng = (sal_uLong)aLanguageLB.GetSelectLanguage();
    }

    rOptions.SetFontName( sFont );
    rOptions.SetCharSet( rtl_TextEncoding( nCCode ) );
    rOptions.SetLanguage( sal_uInt16( nLng ) );
    rOptions.SetParaFlags( GetCRLF() );

    // JP: Task #71802# save the user settings
    String sData;
    rOptions.WriteUserData( sData );
    if( sData.Len() )
    {
        const String& rFindNm = String::CreateFromAscii(
                                    aFontLB.IsVisible() ? sDialogImpExtraData
                                              : sDialogExpExtraData);
        sal_uInt16 nEnd, nStt = GetExtraData().Search( rFindNm );
        if( STRING_NOTFOUND != nStt )
        {
            // called twice, so remove "old" settings
            nEnd = GetExtraData().Search( cDialogExtraDataClose,
                                            nStt + nDialogExtraDataLen );
            if( STRING_NOTFOUND != nEnd )
                GetExtraData().Erase( nStt, nEnd - nStt + 1 );
        }
        String sTmp(GetExtraData());
        sTmp += rFindNm;
        sTmp += sData;
        sTmp += cDialogExtraDataClose;
        GetExtraData() = sTmp;
    }
}

void SwAsciiFilterDlg::SetCRLF( LineEnd eEnd )
{
    switch( eEnd )
    {
    case LINEEND_CR:    aCR_RB.Check();     break;
    case LINEEND_CRLF:  aCRLF_RB.Check();   break;
    case LINEEND_LF:    aLF_RB.Check();     break;
    }
}

LineEnd SwAsciiFilterDlg::GetCRLF() const
{
    LineEnd eEnd;
    if( aCR_RB.IsChecked() )
        eEnd = LINEEND_CR;
    else if( aLF_RB.IsChecked() )
        eEnd = LINEEND_LF;
    else
        eEnd = LINEEND_CRLF;
    return eEnd;
}

IMPL_LINK( SwAsciiFilterDlg, CharSetSelHdl, SvxTextEncodingBox*, pBox )
{
    LineEnd eOldEnd = GetCRLF(), eEnd = (LineEnd)-1;
    LanguageType nLng = aFontLB.IsVisible()
                    ? aLanguageLB.GetSelectLanguage()
                    : LANGUAGE_SYSTEM,
                nOldLng = nLng;

    rtl_TextEncoding nChrSet = pBox->GetSelectTextEncoding();
    if( nChrSet == gsl_getSystemTextEncoding() )
        eEnd = GetSystemLineEnd();
    else
    {
        switch( nChrSet )
        {
        case RTL_TEXTENCODING_MS_1252:
#ifdef UNX
            eEnd = LINEEND_LF;
#else
            eEnd = LINEEND_CRLF;                // ANSI
#endif
            break;

        case RTL_TEXTENCODING_APPLE_ROMAN:      // MAC
            eEnd = LINEEND_CR;
            break;

        case RTL_TEXTENCODING_IBM_850:          // DOS
            eEnd = LINEEND_CRLF;
            break;

        case RTL_TEXTENCODING_APPLE_ARABIC:
        case RTL_TEXTENCODING_APPLE_CENTEURO:
        case RTL_TEXTENCODING_APPLE_CROATIAN:
        case RTL_TEXTENCODING_APPLE_CYRILLIC:
        case RTL_TEXTENCODING_APPLE_DEVANAGARI:
        case RTL_TEXTENCODING_APPLE_FARSI:
        case RTL_TEXTENCODING_APPLE_GREEK:
        case RTL_TEXTENCODING_APPLE_GUJARATI:
        case RTL_TEXTENCODING_APPLE_GURMUKHI:
        case RTL_TEXTENCODING_APPLE_HEBREW:
        case RTL_TEXTENCODING_APPLE_ICELAND:
        case RTL_TEXTENCODING_APPLE_ROMANIAN:
        case RTL_TEXTENCODING_APPLE_THAI:
        case RTL_TEXTENCODING_APPLE_TURKISH:
        case RTL_TEXTENCODING_APPLE_UKRAINIAN:
        case RTL_TEXTENCODING_APPLE_CHINSIMP:
        case RTL_TEXTENCODING_APPLE_CHINTRAD:
        case RTL_TEXTENCODING_APPLE_JAPANESE:
        case RTL_TEXTENCODING_APPLE_KOREAN:
            eEnd = LINEEND_CR;
            break;
        }
    }

    bSaveLineStatus = sal_False;
    if( eEnd != (LineEnd)-1 )       // changed?
    {
        if( eOldEnd != eEnd )
            SetCRLF( eEnd );
    }
    else
    {
        // restore old user choise (not the automatic!)
        aCRLF_RB.Check( aCRLF_RB.GetSavedValue() );
        aCR_RB.Check( aCR_RB.GetSavedValue() );
        aLF_RB.Check( aLF_RB.GetSavedValue() );
    }
    bSaveLineStatus = sal_True;

    if( nOldLng != nLng && aFontLB.IsVisible() )
        aLanguageLB.SelectLanguage( nLng );

    return 0;
}

IMPL_LINK( SwAsciiFilterDlg, LineEndHdl, RadioButton*, pBtn )
{
    if( bSaveLineStatus )
        pBtn->SaveValue();
    return 0;
}


