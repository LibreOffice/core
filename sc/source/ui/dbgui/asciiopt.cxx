/*************************************************************************
 *
 *  $RCSfile: asciiopt.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 13:31:19 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "global.hxx"
#include "scresid.hxx"
#include "impex.hxx"
#include "asciiopt.hxx"
#include "asciiopt.hrc"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif


// ============================================================================

//CHINA001 //! TODO make dynamic
//CHINA001 #ifdef WIN
//CHINA001 const sal_Int32 ASCIIDLG_MAXROWS                = 10000;
//CHINA001 #else
//CHINA001 const sal_Int32 ASCIIDLG_MAXROWS                = 32000;
//CHINA001 #endif

static const sal_Char __FAR_DATA pStrFix[] = "FIX";
static const sal_Char __FAR_DATA pStrMrg[] = "MRG";


// ============================================================================

ScAsciiOptions::ScAsciiOptions() :
    bFixedLen       ( FALSE ),
    aFieldSeps      ( ';' ),
    bMergeFieldSeps ( FALSE ),
    cTextSep        ( 34 ),
    eCharSet        ( gsl_getSystemTextEncoding() ),
    bCharSetSystem  ( FALSE ),
    nStartRow       ( 1 ),
    nInfoCount      ( 0 ),
    pColStart       ( NULL ),
    pColFormat      ( NULL )
{
}


ScAsciiOptions::ScAsciiOptions(const ScAsciiOptions& rOpt) :
    bFixedLen       ( rOpt.bFixedLen ),
    aFieldSeps      ( rOpt.aFieldSeps ),
    bMergeFieldSeps ( rOpt.bMergeFieldSeps ),
    cTextSep        ( rOpt.cTextSep ),
    eCharSet        ( rOpt.eCharSet ),
    bCharSetSystem  ( rOpt.bCharSetSystem ),
    nStartRow       ( rOpt.nStartRow ),
    nInfoCount      ( rOpt.nInfoCount )
{
    if (nInfoCount)
    {
        pColStart = new xub_StrLen[nInfoCount];
        pColFormat = new BYTE[nInfoCount];
        for (USHORT i=0; i<nInfoCount; i++)
        {
            pColStart[i] = rOpt.pColStart[i];
            pColFormat[i] = rOpt.pColFormat[i];
        }
    }
    else
    {
        pColStart = NULL;
        pColFormat = NULL;
    }
}


ScAsciiOptions::~ScAsciiOptions()
{
    delete[] pColStart;
    delete[] pColFormat;
}


void ScAsciiOptions::SetColInfo( USHORT nCount, const xub_StrLen* pStart, const BYTE* pFormat )
{
    delete[] pColStart;
    delete[] pColFormat;

    nInfoCount = nCount;

    if (nInfoCount)
    {
        pColStart = new xub_StrLen[nInfoCount];
        pColFormat = new BYTE[nInfoCount];
        for (USHORT i=0; i<nInfoCount; i++)
        {
            pColStart[i] = pStart[i];
            pColFormat[i] = pFormat[i];
        }
    }
    else
    {
        pColStart = NULL;
        pColFormat = NULL;
    }
}


void ScAsciiOptions::SetColumnInfo( const ScCsvExpDataVec& rDataVec )
{
    delete[] pColStart;
    pColStart = NULL;
    delete[] pColFormat;
    pColFormat = NULL;

    nInfoCount = static_cast< sal_uInt16 >( rDataVec.size() );
    if( nInfoCount )
    {
        pColStart = new xub_StrLen[ nInfoCount ];
        pColFormat = new sal_uInt8[ nInfoCount ];
        for( sal_uInt16 nIx = 0; nIx < nInfoCount; ++nIx )
        {
            pColStart[ nIx ] = rDataVec[ nIx ].mnIndex;
            pColFormat[ nIx ] = rDataVec[ nIx ].mnType;
        }
    }
}


ScAsciiOptions& ScAsciiOptions::operator=( const ScAsciiOptions& rCpy )
{
    SetColInfo( rCpy.nInfoCount, rCpy.pColStart, rCpy.pColFormat );

    bFixedLen       = rCpy.bFixedLen;
    aFieldSeps      = rCpy.aFieldSeps;
    bMergeFieldSeps = rCpy.bMergeFieldSeps;
    cTextSep        = rCpy.cTextSep;
    eCharSet        = rCpy.eCharSet;
    bCharSetSystem  = rCpy.bCharSetSystem;
    nStartRow       = rCpy.nStartRow;

    return *this;
}


BOOL ScAsciiOptions::operator==( const ScAsciiOptions& rCmp ) const
{
    if ( bFixedLen       == rCmp.bFixedLen &&
         aFieldSeps      == rCmp.aFieldSeps &&
         bMergeFieldSeps == rCmp.bMergeFieldSeps &&
         cTextSep        == rCmp.cTextSep &&
         eCharSet        == rCmp.eCharSet &&
         bCharSetSystem  == rCmp.bCharSetSystem &&
         nStartRow       == rCmp.nStartRow &&
         nInfoCount      == rCmp.nInfoCount )
    {
        DBG_ASSERT( !nInfoCount || (pColStart && pColFormat && rCmp.pColStart && rCmp.pColFormat),
                     "0-Zeiger in ScAsciiOptions" );
        for (USHORT i=0; i<nInfoCount; i++)
            if ( pColStart[i] != rCmp.pColStart[i] ||
                 pColFormat[i] != rCmp.pColFormat[i] )
                return FALSE;

        return TRUE;
    }
    return FALSE;
}

//
//  Der Options-String darf kein Semikolon mehr enthalten (wegen Pickliste)
//  darum ab Version 336 Komma stattdessen
//


void ScAsciiOptions::ReadFromString( const String& rString )
{
    xub_StrLen nCount = rString.GetTokenCount(',');
    String aToken;
    xub_StrLen nSub;
    xub_StrLen i;

        //
        //  Feld-Trenner
        //

    if ( nCount >= 1 )
    {
        bFixedLen = bMergeFieldSeps = FALSE;
        aFieldSeps.Erase();

        aToken = rString.GetToken(0,',');
        if ( aToken.EqualsAscii(pStrFix) )
            bFixedLen = TRUE;
        nSub = aToken.GetTokenCount('/');
        for ( i=0; i<nSub; i++ )
        {
            String aCode = aToken.GetToken( i, '/' );
            if ( aCode.EqualsAscii(pStrMrg) )
                bMergeFieldSeps = TRUE;
            else
            {
                sal_Int32 nVal = aCode.ToInt32();
                if ( nVal )
                    aFieldSeps += (sal_Unicode) nVal;
            }
        }
    }

        //
        //  Text-Trenner
        //

    if ( nCount >= 2 )
    {
        aToken = rString.GetToken(1,',');
        sal_Int32 nVal = aToken.ToInt32();
        cTextSep = (sal_Unicode) nVal;
    }

        //
        //  Zeichensatz
        //

    if ( nCount >= 3 )
    {
        aToken = rString.GetToken(2,',');
        eCharSet = ScGlobal::GetCharsetValue( aToken );
    }

        //
        //  Startzeile
        //

    if ( nCount >= 4 )
    {
        aToken = rString.GetToken(3,',');
        nStartRow = aToken.ToInt32();
    }

        //
        //  Spalten-Infos
        //

    if ( nCount >= 5 )
    {
        delete[] pColStart;
        delete[] pColFormat;

        aToken = rString.GetToken(4,',');
        nSub = aToken.GetTokenCount('/');
        nInfoCount = nSub / 2;
        if (nInfoCount)
        {
            pColStart = new xub_StrLen[nInfoCount];
            pColFormat = new BYTE[nInfoCount];
            for (USHORT nInfo=0; nInfo<nInfoCount; nInfo++)
            {
                pColStart[nInfo]  = (xub_StrLen) aToken.GetToken( 2*nInfo, '/' ).ToInt32();
                pColFormat[nInfo] = (BYTE) aToken.GetToken( 2*nInfo+1, '/' ).ToInt32();
            }
        }
        else
        {
            pColStart = NULL;
            pColFormat = NULL;
        }
    }
}


String ScAsciiOptions::WriteToString() const
{
    String aOutStr;

        //
        //  Feld-Trenner
        //

    if ( bFixedLen )
        aOutStr.AppendAscii(pStrFix);
    else if ( !aFieldSeps.Len() )
        aOutStr += '0';
    else
    {
        xub_StrLen nLen = aFieldSeps.Len();
        for (xub_StrLen i=0; i<nLen; i++)
        {
            if (i)
                aOutStr += '/';
            aOutStr += String::CreateFromInt32(aFieldSeps.GetChar(i));
        }
        if ( bMergeFieldSeps )
        {
            aOutStr += '/';
            aOutStr.AppendAscii(pStrMrg);
        }
    }

    aOutStr += ',';                 // Token-Ende

        //
        //  Text-Trenner
        //

    aOutStr += String::CreateFromInt32(cTextSep);
    aOutStr += ',';                 // Token-Ende

        //
        //  Zeichensatz
        //

    if ( bCharSetSystem )           // force "SYSTEM"
        aOutStr += ScGlobal::GetCharsetString( RTL_TEXTENCODING_DONTKNOW );
    else
        aOutStr += ScGlobal::GetCharsetString( eCharSet );
    aOutStr += ',';                 // Token-Ende

        //
        //  Startzeile
        //

    aOutStr += String::CreateFromInt32(nStartRow);
    aOutStr += ',';                 // Token-Ende

        //
        //  Spalten-Infos
        //

    DBG_ASSERT( !nInfoCount || (pColStart && pColFormat), "0-Zeiger in ScAsciiOptions" );
    for (USHORT nInfo=0; nInfo<nInfoCount; nInfo++)
    {
        if (nInfo)
            aOutStr += '/';
        aOutStr += String::CreateFromInt32(pColStart[nInfo]);
        aOutStr += '/';
        aOutStr += String::CreateFromInt32(pColFormat[nInfo]);
    }

    return aOutStr;
}

#if 0
//  Code, um die Spalten-Liste aus einem Excel-kompatiblen String zu erzeugen:
//  (im Moment nicht benutzt)

void ScAsciiOptions::InterpretColumnList( const String& rString )
{
    //  Eingabe ist 1-basiert, pColStart fuer FixedLen ist 0-basiert

    //  Kommas durch Semikolon ersetzen

    String aSemiStr = rString;
    USHORT nPos = 0;
    do
        nPos = aSemiStr.SearchAndReplace( ',', ';', nPos );
    while ( nPos != STRING_NOTFOUND );

    //  Eintraege sortieren

    USHORT nCount = aSemiStr.GetTokenCount();
    USHORT* pTemp = new USHORT[nCount+1];
    pTemp[0] = 1;                                   // erste Spalte faengt immer bei 1 an
    USHORT nFound = 1;
    USHORT i,j;
    for (i=0; i<nCount; i++)
    {
        USHORT nVal = (USHORT) aSemiStr.GetToken(i);
        if (nVal)
        {
            BOOL bThere = FALSE;
            nPos = 0;
            for (j=0; j<nFound; j++)
            {
                if ( pTemp[j] == nVal )
                    bThere = TRUE;
                else if ( pTemp[j] < nVal )
                    nPos = j+1;
            }
            if ( !bThere )
            {
                if ( nPos < nFound )
                    memmove( &pTemp[nPos+1], &pTemp[nPos], (nFound-nPos)*sizeof(USHORT) );
                pTemp[nPos] = nVal;
                ++nFound;
            }
        }
    }

    //  Eintraege uebernehmen

    delete[] pColStart;
    delete[] pColFormat;
    nInfoCount = nFound;
    if (nInfoCount)
    {
        pColStart = new USHORT[nInfoCount];
        pColFormat = new BYTE[nInfoCount];
        for (i=0; i<nInfoCount; i++)
        {
            pColStart[i] = pTemp[i] - 1;
            pColFormat[i] = SC_COL_STANDARD;
        }
    }
    else
    {
        pColStart = NULL;
        pColFormat = NULL;
    }

    bFixedLen = TRUE;           // sonst macht's keinen Sinn

    //  aufraeumen

    delete[] pTemp;
}
#endif


// ============================================================================

//CHINA001 void lcl_FillCombo( ComboBox& rCombo, const String& rList, sal_Unicode cSelect )
//CHINA001 {
//CHINA001 xub_StrLen i;
//CHINA001 xub_StrLen nCount = rList.GetTokenCount('\t');
//CHINA001 for ( i=0; i<nCount; i+=2 )
//CHINA001 rCombo.InsertEntry( rList.GetToken(i,'\t') );
//CHINA001
//CHINA001 if ( cSelect )
//CHINA001 {
//CHINA001 String aStr;
//CHINA001 for ( i=0; i<nCount; i+=2 )
//CHINA001 if ( (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32() == cSelect )
//CHINA001 aStr = rList.GetToken(i,'\t');
//CHINA001 if (!aStr.Len())
//CHINA001 aStr = cSelect;          // Ascii
//CHINA001
//CHINA001 rCombo.SetText(aStr);
//CHINA001  }
//CHINA001 }
//CHINA001
//CHINA001 sal_Unicode lcl_CharFromCombo( ComboBox& rCombo, const String& rList )
//CHINA001 {
//CHINA001 sal_Unicode c = 0;
//CHINA001 String aStr = rCombo.GetText();
//CHINA001 if ( aStr.Len() )
//CHINA001 {
//CHINA001 xub_StrLen nCount = rList.GetTokenCount('\t');
//CHINA001 for ( xub_StrLen i=0; i<nCount; i+=2 )
//CHINA001      {
//CHINA001 if ( ScGlobal::pTransliteration->isEqual( aStr, rList.GetToken(i,'\t') ) )
//CHINA001 c = (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32();
//CHINA001      }
//CHINA001 if (!c)
//CHINA001 c = (sal_Unicode) aStr.ToInt32();        // Ascii
//CHINA001  }
//CHINA001 return c;
//CHINA001 }
//CHINA001
//CHINA001
//CHINA001 // ----------------------------------------------------------------------------
//CHINA001
//CHINA001 ScImportAsciiDlg::ScImportAsciiDlg( Window* pParent,String aDatName,
//CHINA001 SvStream* pInStream, sal_Unicode cSep ) :
//CHINA001 ModalDialog  ( pParent, ScResId( RID_SCDLG_ASCII ) ),
//CHINA001 pDatStream  ( pInStream ),
//CHINA001
//CHINA001 aRbFixed ( this, ScResId( RB_FIXED ) ),
//CHINA001 aRbSeparated( this, ScResId( RB_SEPARATED ) ),
//CHINA001
//CHINA001 aCkbTab      ( this, ScResId( CKB_TAB ) ),
//CHINA001 aCkbSemicolon(this, ScResId( CKB_SEMICOLON ) ),
//CHINA001 aCkbComma    ( this, ScResId( CKB_COMMA  ) ),
//CHINA001 aCkbSpace    ( this, ScResId( CKB_SPACE   ) ),
//CHINA001 aCkbOther    ( this, ScResId( CKB_OTHER ) ),
//CHINA001 aEdOther ( this, ScResId( ED_OTHER ) ),
//CHINA001
//CHINA001 aFtRow       ( this, ScResId( FT_AT_ROW  ) ),
//CHINA001 aNfRow       ( this, ScResId( NF_AT_ROW  ) ),
//CHINA001
//CHINA001 aFtCharSet   ( this, ScResId( FT_CHARSET ) ),
//CHINA001 aLbCharSet   ( this, ScResId( LB_CHARSET ) ),
//CHINA001 aFlSepOpt   ( this, ScResId( FL_SEPOPT ) ),
//CHINA001 aFtTextSep   ( this, ScResId( FT_TEXTSEP ) ),
//CHINA001 aCbTextSep   ( this, ScResId( CB_TEXTSEP ) ),
//CHINA001 aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
//CHINA001 aCkbAsOnce   ( this, ScResId( CB_ASONCE) ),
//CHINA001 aFtType      ( this, ScResId( FT_TYPE ) ),
//CHINA001 aLbType      ( this, ScResId( LB_TYPE1 ) ),
//CHINA001 maTableBox  ( this, ScResId( CTR_TABLEBOX ) ),
//CHINA001 aFlWidth    ( this, ScResId( FL_WIDTH ) ),
//CHINA001 aBtnOk       ( this, ScResId( BTN_OK ) ),
//CHINA001 aBtnCancel   ( this, ScResId( BTN_CANCEL ) ),
//CHINA001 aBtnHelp ( this, ScResId( BTN_HELP ) ),
//CHINA001 aCharSetUser( ScResId( SCSTR_CHARSET_USER ) ),
//CHINA001 aColumnUser  ( ScResId( SCSTR_COLUMN_USER ) ),
//CHINA001 aFldSepList  ( ScResId( SCSTR_FIELDSEP ) ),
//CHINA001 aTextSepList( ScResId( SCSTR_TEXTSEP ) ),
//CHINA001 pRowPosArray( NULL ),
//CHINA001 pRowPosArrayUnicode( NULL ),
//CHINA001 bVFlag      ( FALSE )
//CHINA001 {
//CHINA001 FreeResource();
//CHINA001
//CHINA001 String aName = GetText();
//CHINA001 aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" - ["));
//CHINA001 aName += aDatName;
//CHINA001 aName += ']';
//CHINA001 SetText( aName );
//CHINA001
//CHINA001 switch(cSep)
//CHINA001  {
//CHINA001      case '\t':  aCkbTab.Check();        break;
//CHINA001      case ';':   aCkbSemicolon.Check();  break;
//CHINA001      case ',':   aCkbComma.Check();      break;
//CHINA001      case ' ':   aCkbSpace.Check();      break;
//CHINA001      default:
//CHINA001 aCkbOther.Check();
//CHINA001 aEdOther.SetText( cSep );
//CHINA001  }
//CHINA001
//CHINA001 nArrayEndPos = nArrayEndPosUnicode = 0;
//CHINA001 USHORT nField;
//CHINA001 BOOL bPreselectUnicode = FALSE;
//CHINA001 if( pDatStream )
//CHINA001  {
//CHINA001 USHORT j;
//CHINA001 pRowPosArray=new ULONG[ASCIIDLG_MAXROWS+2];
//CHINA001 pRowPosArrayUnicode=new ULONG[ASCIIDLG_MAXROWS+2];
//CHINA001 ULONG *pPtrRowPos=pRowPosArray;
//CHINA001 ULONG *pPtrRowPosUnicode=pRowPosArrayUnicode;
//CHINA001 for(nField=0;nField<ASCIIDLG_MAXROWS;nField++)
//CHINA001      {
//CHINA001 *pPtrRowPos++=0;
//CHINA001 *pPtrRowPosUnicode++=0;
//CHINA001      }
//CHINA001 pDatStream->SetBufferSize(ASCIIDLG_MAXROWS);
//CHINA001 pDatStream->SetStreamCharSet( gsl_getSystemTextEncoding() ); //!???
//CHINA001 pDatStream->Seek( 0 );
//CHINA001 for ( j=0; j < CSV_PREVIEW_LINES; j++ )
//CHINA001      {
//CHINA001 pRowPosArray[nArrayEndPos++]=pDatStream->Tell();
//CHINA001 if(!pDatStream->ReadLine( aPreviewLine[j] ))
//CHINA001          {
//CHINA001 bVFlag=TRUE;
//CHINA001 maTableBox.Execute( CSVCMD_SETLINECOUNT, j );
//CHINA001 break;
//CHINA001          }
//CHINA001      }
//CHINA001 nStreamPos = pDatStream->Tell();
//CHINA001
//CHINA001 pDatStream->Seek( 0 );
//CHINA001 pDatStream->StartReadingUnicodeText();
//CHINA001 ULONG nUniPos = pDatStream->Tell();
//CHINA001 if ( nUniPos > 0 )
//CHINA001 bPreselectUnicode = TRUE;    // read 0xfeff/0xfffe
//CHINA001      else
//CHINA001      {
//CHINA001 UINT16 n;
//CHINA001 *pDatStream >> n;
//CHINA001 // Assume that normal ASCII/ANSI/ISO/etc. text doesn't start with
//CHINA001 // control characters except CR,LF,TAB
//CHINA001 if ( (n & 0xff00) < 0x2000 )
//CHINA001          {
//CHINA001 switch ( n & 0xff00 )
//CHINA001              {
//CHINA001                  case 0x0900 :
//CHINA001                  case 0x0a00 :
//CHINA001                  case 0x0d00 :
//CHINA001 break;
//CHINA001                  default:
//CHINA001 bPreselectUnicode = TRUE;
//CHINA001              }
//CHINA001          }
//CHINA001 pDatStream->Seek( nUniPos );
//CHINA001      }
//CHINA001 for ( j=0; j < CSV_PREVIEW_LINES; j++ )
//CHINA001      {
//CHINA001 pRowPosArrayUnicode[nArrayEndPosUnicode++] = pDatStream->Tell();
//CHINA001 if( !pDatStream->ReadUniStringLine( aPreviewLineUnicode[j] ) )
//CHINA001 break;
//CHINA001 // #84386# Reading Unicode on ASCII/ANSI data won't find any line
//CHINA001 // ends and therefor tries to read the whole file into strings.
//CHINA001 // Check if first line is completely filled and don't try any further.
//CHINA001 if ( j == 0 && aPreviewLineUnicode[j].Len() == STRING_MAXLEN )
//CHINA001 break;
//CHINA001      }
//CHINA001 nStreamPosUnicode = pDatStream->Tell();
//CHINA001
//CHINA001 //   #107455# If the file content isn't unicode, ReadUniStringLine
//CHINA001 //   may try to seek beyond the file's end and cause a CANTSEEK error
//CHINA001 //   (depending on the stream type). The error code has to be cleared,
//CHINA001 //   or further read operations (including non-unicode) will fail.
//CHINA001 if ( pDatStream->GetError() == ERRCODE_IO_CANTSEEK )
//CHINA001 pDatStream->ResetError();
//CHINA001  }
//CHINA001
//CHINA001 aNfRow.SetModifyHdl( LINK( this, ScImportAsciiDlg, FirstRowHdl ) );
//CHINA001
//CHINA001 // *** Separator characters ***
//CHINA001 lcl_FillCombo( aCbTextSep, aTextSepList, 34 );      // Default "
//CHINA001
//CHINA001 Link aSeparatorHdl =LINK( this, ScImportAsciiDlg, SeparatorHdl );
//CHINA001 aCbTextSep.SetSelectHdl( aSeparatorHdl );
//CHINA001 aCbTextSep.SetModifyHdl( aSeparatorHdl );
//CHINA001 aCkbTab.SetClickHdl( aSeparatorHdl );
//CHINA001 aCkbSemicolon.SetClickHdl( aSeparatorHdl );
//CHINA001 aCkbComma.SetClickHdl( aSeparatorHdl );
//CHINA001 aCkbAsOnce.SetClickHdl( aSeparatorHdl );
//CHINA001 aCkbSpace.SetClickHdl( aSeparatorHdl );
//CHINA001 aCkbOther.SetClickHdl( aSeparatorHdl );
//CHINA001 aEdOther.SetModifyHdl( aSeparatorHdl );
//CHINA001
//CHINA001 // *** text encoding ListBox ***
//CHINA001 // all encodings allowed, including Unicode, but subsets are excluded
//CHINA001 aLbCharSet.FillFromTextEncodingTable( TRUE );
//CHINA001 // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
//CHINA001 // independent document linkage.
//CHINA001 aLbCharSet.InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, aCharSetUser );
//CHINA001 aLbCharSet.SelectTextEncoding( bPreselectUnicode ?
//CHINA001 RTL_TEXTENCODING_UNICODE : gsl_getSystemTextEncoding() );
//CHINA001 SetSelectedCharSet();
//CHINA001 aLbCharSet.SetSelectHdl( LINK( this, ScImportAsciiDlg, CharSetHdl ) );
//CHINA001
//CHINA001 // *** column type ListBox ***
//CHINA001 xub_StrLen nCount = aColumnUser.GetTokenCount();
//CHINA001 for (xub_StrLen i=0; i<nCount; i++)
//CHINA001 aLbType.InsertEntry( aColumnUser.GetToken( i ) );
//CHINA001
//CHINA001 aLbType.SetSelectHdl( LINK( this, ScImportAsciiDlg, LbColTypeHdl ) );
//CHINA001 aFtType.Disable();
//CHINA001 aLbType.Disable();
//CHINA001
//CHINA001 // *** table box preview ***
//CHINA001 maTableBox.SetUpdateTextHdl( LINK( this, ScImportAsciiDlg, UpdateTextHdl ) );
//CHINA001 maTableBox.InitTypes( aLbType );
//CHINA001 maTableBox.SetColTypeHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );
//CHINA001
//CHINA001 if(!bVFlag)
//CHINA001 maTableBox.Execute( CSVCMD_SETLINECOUNT, ASCIIDLG_MAXROWS );
//CHINA001
//CHINA001 aRbSeparated.SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
//CHINA001 aRbFixed.SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
//CHINA001
//CHINA001 SetupSeparatorCtrls();
//CHINA001 RbSepFixHdl( &aRbFixed );
//CHINA001
//CHINA001 maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
//CHINA001 }
//CHINA001
//CHINA001
//CHINA001 ScImportAsciiDlg::~ScImportAsciiDlg()
//CHINA001 {
//CHINA001 delete[] pRowPosArray;
//CHINA001 delete[] pRowPosArrayUnicode;
//CHINA001
//CHINA001 }
//CHINA001
//CHINA001 void ScImportAsciiDlg::GetOptions( ScAsciiOptions& rOpt )
//CHINA001 {
//CHINA001 rOpt.SetCharSet( meCharSet );
//CHINA001 rOpt.SetCharSetSystem( mbCharSetSystem );
//CHINA001 rOpt.SetFixedLen( aRbFixed.IsChecked() );
//CHINA001 rOpt.SetStartRow( (USHORT)aNfRow.GetValue() );
//CHINA001 maTableBox.FillColumnData( rOpt );
//CHINA001 if( aRbSeparated.IsChecked() )
//CHINA001  {
//CHINA001 rOpt.SetFieldSeps( GetSeparators() );
//CHINA001 rOpt.SetMergeSeps( aCkbAsOnce.IsChecked() );
//CHINA001 rOpt.SetTextSep( lcl_CharFromCombo( aCbTextSep, aTextSepList ) );
//CHINA001  }
//CHINA001 }
//CHINA001
//CHINA001 void ScImportAsciiDlg::SetSelectedCharSet()
//CHINA001 {
//CHINA001 meCharSet = aLbCharSet.GetSelectTextEncoding();
//CHINA001 mbCharSetSystem = (meCharSet == RTL_TEXTENCODING_DONTKNOW);
//CHINA001 if( mbCharSetSystem )
//CHINA001 meCharSet = gsl_getSystemTextEncoding();
//CHINA001 }
//CHINA001
//CHINA001 String ScImportAsciiDlg::GetSeparators() const
//CHINA001 {
//CHINA001 String aSepChars;
//CHINA001 if( aCkbTab.IsChecked() )
//CHINA001 aSepChars += '\t';
//CHINA001 if( aCkbSemicolon.IsChecked() )
//CHINA001 aSepChars += ';';
//CHINA001 if( aCkbComma.IsChecked() )
//CHINA001 aSepChars += ',';
//CHINA001 if( aCkbSpace.IsChecked() )
//CHINA001 aSepChars += ' ';
//CHINA001 if( aCkbOther.IsChecked() )
//CHINA001 aSepChars += aEdOther.GetText();
//CHINA001 return aSepChars;
//CHINA001 }
//CHINA001
//CHINA001 void ScImportAsciiDlg::SetupSeparatorCtrls()
//CHINA001 {
//CHINA001 BOOL bEnable = aRbSeparated.IsChecked();
//CHINA001 aCkbTab.Enable( bEnable );
//CHINA001 aCkbSemicolon.Enable( bEnable );
//CHINA001 aCkbComma.Enable( bEnable );
//CHINA001 aCkbSpace.Enable( bEnable );
//CHINA001 aCkbOther.Enable( bEnable );
//CHINA001 aEdOther.Enable( bEnable );
//CHINA001 aCkbAsOnce.Enable( bEnable );
//CHINA001 aFtTextSep.Enable( bEnable );
//CHINA001 aCbTextSep.Enable( bEnable );
//CHINA001 }
//CHINA001
//CHINA001 void ScImportAsciiDlg::UpdateVertical( bool bSwitchToFromUnicode )
//CHINA001 {
//CHINA001 if ( bSwitchToFromUnicode )
//CHINA001  {
//CHINA001 bVFlag = FALSE;
//CHINA001 maTableBox.Execute( CSVCMD_SETLINECOUNT, ASCIIDLG_MAXROWS );
//CHINA001  }
//CHINA001 ULONG nNew = 0;
//CHINA001 if(!bVFlag)
//CHINA001  {
//CHINA001 // dragging the scrollbar -> read entire file
//CHINA001 bVFlag=TRUE;
//CHINA001 ULONG nRows = 0;
//CHINA001
//CHINA001 pDatStream->Seek(0);
//CHINA001 if ( meCharSet == RTL_TEXTENCODING_UNICODE )
//CHINA001      {
//CHINA001 String aStringUnicode;
//CHINA001 pDatStream->StartReadingUnicodeText();
//CHINA001 ULONG* pPtrRowPos = pRowPosArrayUnicode;
//CHINA001 *pPtrRowPos++ = 0;
//CHINA001 while( pDatStream->ReadUniStringLine( aStringUnicode ) )
//CHINA001          {
//CHINA001 nRows++;
//CHINA001 if( nRows > ASCIIDLG_MAXROWS )
//CHINA001 break;
//CHINA001 *pPtrRowPos++ = pDatStream->Tell();
//CHINA001          }
//CHINA001 nStreamPosUnicode = pDatStream->Tell();
//CHINA001
//CHINA001 if ( pDatStream->GetError() == ERRCODE_IO_CANTSEEK ) // #107455# reset seek error
//CHINA001 pDatStream->ResetError();
//CHINA001      }
//CHINA001      else
//CHINA001      {
//CHINA001 ByteString aString;
//CHINA001 ULONG* pPtrRowPos = pRowPosArray;
//CHINA001 *pPtrRowPos++ = 0;
//CHINA001 while( pDatStream->ReadLine( aString ) )
//CHINA001          {
//CHINA001 nRows++;
//CHINA001 if( nRows > ASCIIDLG_MAXROWS )
//CHINA001 break;
//CHINA001 *pPtrRowPos++ = pDatStream->Tell();
//CHINA001          }
//CHINA001 nStreamPos = pDatStream->Tell();
//CHINA001      }
//CHINA001
//CHINA001 maTableBox.Execute( CSVCMD_SETLINECOUNT, nRows );
//CHINA001  }
//CHINA001
//CHINA001 nNew = maTableBox.GetFirstVisLine();
//CHINA001
//CHINA001 if ( meCharSet == RTL_TEXTENCODING_UNICODE )
//CHINA001  {
//CHINA001 if ( bVFlag || nNew <= nArrayEndPosUnicode )
//CHINA001 pDatStream->Seek( pRowPosArrayUnicode[nNew] );
//CHINA001      else
//CHINA001 pDatStream->Seek( nStreamPosUnicode );
//CHINA001 for ( USHORT j=0; j < CSV_PREVIEW_LINES; j++ )
//CHINA001      {
//CHINA001 if( !bVFlag && nNew+j >= nArrayEndPos )
//CHINA001          {
//CHINA001 pRowPosArrayUnicode[nNew+j] = pDatStream->Tell();
//CHINA001 nArrayEndPosUnicode = (USHORT) nNew+j;
//CHINA001          }
//CHINA001 // #84386# Reading Unicode on ASCII/ANSI data won't find any line
//CHINA001 // ends and therefor tries to read the whole file into strings.
//CHINA001 // Check if first line is completely filled and don't try any further.
//CHINA001 if( (!pDatStream->ReadUniStringLine( aPreviewLineUnicode[j] ) ||
//CHINA001 (j == 0 && aPreviewLineUnicode[j].Len() == STRING_MAXLEN)) &&
//CHINA001 !bVFlag )
//CHINA001          {
//CHINA001 bVFlag = TRUE;
//CHINA001 maTableBox.Execute( CSVCMD_SETLINECOUNT, nArrayEndPosUnicode );
//CHINA001          }
//CHINA001      }
//CHINA001 nStreamPosUnicode = pDatStream->Tell();
//CHINA001
//CHINA001 if ( pDatStream->GetError() == ERRCODE_IO_CANTSEEK ) // #107455# reset seek error
//CHINA001 pDatStream->ResetError();
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001 if ( bVFlag || nNew <= nArrayEndPos )
//CHINA001 pDatStream->Seek( pRowPosArray[nNew] );
//CHINA001      else
//CHINA001 pDatStream->Seek( nStreamPos );
//CHINA001 for ( USHORT j=0; j < CSV_PREVIEW_LINES; j++ )
//CHINA001      {
//CHINA001 if( !bVFlag && nNew+j >= nArrayEndPos )
//CHINA001          {
//CHINA001 pRowPosArray[nNew+j] = pDatStream->Tell();
//CHINA001 nArrayEndPos = (USHORT) nNew+j;
//CHINA001          }
//CHINA001 if( !pDatStream->ReadLine( aPreviewLine[j] ) && !bVFlag )
//CHINA001          {
//CHINA001 bVFlag = TRUE;
//CHINA001 maTableBox.Execute( CSVCMD_SETLINECOUNT, nArrayEndPos );
//CHINA001          }
//CHINA001      }
//CHINA001 nStreamPos = pDatStream->Tell();
//CHINA001  }
//CHINA001 }
//CHINA001
//CHINA001
//CHINA001 // ----------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, RbSepFixHdl, RadioButton*, pButton )
//CHINA001 {
//CHINA001 DBG_ASSERT( pButton, "ScImportAsciiDlg::RbSepFixHdl - missing sender" );
//CHINA001
//CHINA001 if( (pButton == &aRbFixed) || (pButton == &aRbSeparated) )
//CHINA001  {
//CHINA001 SetPointer( Pointer( POINTER_WAIT ) );
//CHINA001 if( aRbFixed.IsChecked() )
//CHINA001 maTableBox.SetFixedWidthMode();
//CHINA001      else
//CHINA001 maTableBox.SetSeparatorsMode();
//CHINA001 SetPointer( Pointer( POINTER_ARROW ) );
//CHINA001
//CHINA001 SetupSeparatorCtrls();
//CHINA001  }
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, SeparatorHdl, Control*, pCtrl )
//CHINA001 {
//CHINA001 DBG_ASSERT( pCtrl, "ScImportAsciiDlg::SeparatorHdl - missing sender" );
//CHINA001 DBG_ASSERT( !aRbFixed.IsChecked(), "ScImportAsciiDlg::SeparatorHdl - not allowed in fixed width" );
//CHINA001
//CHINA001 if( (pCtrl == &aCkbOther) && aCkbOther.IsChecked() )
//CHINA001 aEdOther.GrabFocus();
//CHINA001  else if( pCtrl == &aEdOther )
//CHINA001 aCkbOther.Check( aEdOther.GetText().Len() > 0 );
//CHINA001 maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, CharSetHdl, SvxTextEncodingBox*, pCharSetBox )
//CHINA001 {
//CHINA001 DBG_ASSERT( pCharSetBox, "ScImportAsciiDlg::CharSetHdl - missing sender" );
//CHINA001
//CHINA001 if( (pCharSetBox == &aLbCharSet) && (pCharSetBox->GetSelectEntryCount() == 1) )
//CHINA001  {
//CHINA001 SetPointer( Pointer( POINTER_WAIT ) );
//CHINA001 CharSet eOldCharSet = meCharSet;
//CHINA001 SetSelectedCharSet();
//CHINA001 if( (meCharSet == RTL_TEXTENCODING_UNICODE) != (eOldCharSet == RTL_TEXTENCODING_UNICODE) )
//CHINA001      {
//CHINA001 // switching to/from Unicode invalidates all positions
//CHINA001 if( pDatStream )
//CHINA001 UpdateVertical( TRUE );
//CHINA001      }
//CHINA001 maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
//CHINA001 SetPointer( Pointer( POINTER_ARROW ) );
//CHINA001  }
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, FirstRowHdl, NumericField*, pNumField )
//CHINA001 {
//CHINA001 DBG_ASSERT( pNumField, "ScImportAsciiDlg::FirstRowHdl - missing sender" );
//CHINA001 maTableBox.Execute( CSVCMD_SETFIRSTIMPORTLINE, pNumField->GetValue() - 1 );
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, LbColTypeHdl, ListBox*, pListBox )
//CHINA001 {
//CHINA001 DBG_ASSERT( pListBox, "ScImportAsciiDlg::LbColTypeHdl - missing sender" );
//CHINA001 if( pListBox == &aLbType )
//CHINA001 maTableBox.Execute( CSVCMD_SETCOLUMNTYPE, pListBox->GetSelectEntryPos() );
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, UpdateTextHdl, ScCsvTableBox*, pTableBox )
//CHINA001 {
//CHINA001 DBG_ASSERT( pTableBox, "ScImportAsciiDlg::UpdateTextHdl - missing sender" );
//CHINA001
//CHINA001 BOOL bVFlag1 = bVFlag;
//CHINA001 if( pDatStream )
//CHINA001 UpdateVertical();
//CHINA001 if( bVFlag != bVFlag1 )
//CHINA001 UpdateVertical();
//CHINA001
//CHINA001 sal_Unicode cTextSep = lcl_CharFromCombo( aCbTextSep, aTextSepList );
//CHINA001 bool bMergeSep = (aCkbAsOnce.IsChecked() == TRUE);
//CHINA001
//CHINA001 if( meCharSet == RTL_TEXTENCODING_UNICODE )
//CHINA001 maTableBox.SetUniStrings( aPreviewLineUnicode, GetSeparators(), cTextSep, bMergeSep );
//CHINA001  else
//CHINA001 maTableBox.SetByteStrings( aPreviewLine, meCharSet, GetSeparators(), cTextSep, bMergeSep );
//CHINA001
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox*, pTableBox )
//CHINA001 {
//CHINA001 DBG_ASSERT( pTableBox, "ScImportAsciiDlg::ColTypeHdl - missing sender" );
//CHINA001
//CHINA001 sal_Int32 nType = pTableBox->GetSelColumnType();
//CHINA001 sal_Int32 nTypeCount = aLbType.GetEntryCount();
//CHINA001 bool bEmpty = (nType == CSV_TYPE_MULTI);
//CHINA001 bool bEnable = ((0 <= nType) && (nType < nTypeCount)) || bEmpty;
//CHINA001
//CHINA001 aFtType.Enable( bEnable );
//CHINA001 aLbType.Enable( bEnable );
//CHINA001     Link aSelHdl = aLbType.GetSelectHdl();
//CHINA001 aLbType.SetSelectHdl( Link() );
//CHINA001 if( bEmpty )
//CHINA001 aLbType.SetNoSelection();
//CHINA001  else if( bEnable )
//CHINA001 aLbType.SelectEntryPos( static_cast< sal_uInt16 >( nType ) );
//CHINA001 aLbType.SetSelectHdl( aSelHdl );
//CHINA001
//CHINA001 return 0;
//CHINA001 }

