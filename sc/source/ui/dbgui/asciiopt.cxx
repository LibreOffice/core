/*************************************************************************
 *
 *  $RCSfile: asciiopt.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:51 $
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

//! TODO make dynamic
#ifdef WIN
const sal_Int32 ASCIIDLG_MAXROWS                = 10000;
#else
const sal_Int32 ASCIIDLG_MAXROWS                = 32000;
#endif

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

void lcl_FillCombo( ComboBox& rCombo, const String& rList, sal_Unicode cSelect )
{
    xub_StrLen i;
    xub_StrLen nCount = rList.GetTokenCount('\t');
    for ( i=0; i<nCount; i+=2 )
        rCombo.InsertEntry( rList.GetToken(i,'\t') );

    if ( cSelect )
    {
        String aStr;
        for ( i=0; i<nCount; i+=2 )
            if ( (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32() == cSelect )
                aStr = rList.GetToken(i,'\t');
        if (!aStr.Len())
            aStr = cSelect;         // Ascii

        rCombo.SetText(aStr);
    }
}

sal_Unicode lcl_CharFromCombo( ComboBox& rCombo, const String& rList )
{
    sal_Unicode c = 0;
    String aStr = rCombo.GetText();
    if ( aStr.Len() )
    {
        xub_StrLen nCount = rList.GetTokenCount('\t');
        for ( xub_StrLen i=0; i<nCount; i+=2 )
        {
            if ( ScGlobal::pTransliteration->isEqual( aStr, rList.GetToken(i,'\t') ) )
                c = (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32();
        }
        if (!c)
            c = (sal_Unicode) aStr.ToInt32();       // Ascii
    }
    return c;
}


// ----------------------------------------------------------------------------

ScImportAsciiDlg::ScImportAsciiDlg( Window* pParent,String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep ) :
        ModalDialog ( pParent, ScResId( RID_SCDLG_ASCII ) ),
        pDatStream  ( pInStream ),

        aRbFixed    ( this, ScResId( RB_FIXED ) ),
        aRbSeparated( this, ScResId( RB_SEPARATED ) ),

        aCkbTab     ( this, ScResId( CKB_TAB ) ),
        aCkbSemicolon(this, ScResId( CKB_SEMICOLON ) ),
        aCkbComma   ( this, ScResId( CKB_COMMA  ) ),
        aCkbSpace   ( this, ScResId( CKB_SPACE   ) ),
        aCkbOther   ( this, ScResId( CKB_OTHER ) ),
        aEdOther    ( this, ScResId( ED_OTHER ) ),

        aFtRow      ( this, ScResId( FT_AT_ROW  ) ),
        aNfRow      ( this, ScResId( NF_AT_ROW  ) ),

        aFtCharSet  ( this, ScResId( FT_CHARSET ) ),
        aLbCharSet  ( this, ScResId( LB_CHARSET ) ),
        aFlSepOpt   ( this, ScResId( FL_SEPOPT ) ),
        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aCbTextSep  ( this, ScResId( CB_TEXTSEP ) ),
        aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
        aCkbAsOnce  ( this, ScResId( CB_ASONCE) ),
        aFtType     ( this, ScResId( FT_TYPE ) ),
        aLbType     ( this, ScResId( LB_TYPE1 ) ),
        maTableBox  ( this, ScResId( CTR_TABLEBOX ) ),
        aFlWidth    ( this, ScResId( FL_WIDTH ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),
        aCharSetUser( ScResId( SCSTR_CHARSET_USER ) ),
        aColumnUser ( ScResId( SCSTR_COLUMN_USER ) ),
        aFldSepList ( ScResId( SCSTR_FIELDSEP ) ),
        aTextSepList( ScResId( SCSTR_TEXTSEP ) ),
        pRowPosArray( NULL ),
        pRowPosArrayUnicode( NULL ),
        bVFlag      ( FALSE )
{
    FreeResource();

    String aName = GetText();
    aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" - ["));
    aName += aDatName;
    aName += ']';
    SetText( aName );

    switch(cSep)
    {
        case '\t':  aCkbTab.Check();        break;
        case ';':   aCkbSemicolon.Check();  break;
        case ',':   aCkbComma.Check();      break;
        case ' ':   aCkbSpace.Check();      break;
        default:
            aCkbOther.Check();
            aEdOther.SetText( cSep );
    }

    nArrayEndPos = nArrayEndPosUnicode = 0;
    USHORT nField;
    BOOL bPreselectUnicode = FALSE;
    if( pDatStream )
    {
        USHORT j;
        pRowPosArray=new ULONG[ASCIIDLG_MAXROWS+2];
        pRowPosArrayUnicode=new ULONG[ASCIIDLG_MAXROWS+2];
        ULONG *pPtrRowPos=pRowPosArray;
        ULONG *pPtrRowPosUnicode=pRowPosArrayUnicode;
        for(nField=0;nField<ASCIIDLG_MAXROWS;nField++)
        {
            *pPtrRowPos++=0;
            *pPtrRowPosUnicode++=0;
        }
        pDatStream->SetBufferSize(ASCIIDLG_MAXROWS);
        pDatStream->SetStreamCharSet( gsl_getSystemTextEncoding() );    //!???
        pDatStream->Seek( 0 );
        for ( j=0; j < CSV_PREVIEW_LINES; j++ )
        {
            pRowPosArray[nArrayEndPos++]=pDatStream->Tell();
            if(!pDatStream->ReadLine( aPreviewLine[j] ))
            {
                bVFlag=TRUE;
                maTableBox.Execute( CSVCMD_SETLINECOUNT, j );
                break;
            }
        }
        nStreamPos = pDatStream->Tell();

        pDatStream->Seek( 0 );
        pDatStream->StartReadingUnicodeText();
        ULONG nUniPos = pDatStream->Tell();
        if ( nUniPos > 0 )
            bPreselectUnicode = TRUE;   // read 0xfeff/0xfffe
        else
        {
            UINT16 n;
            *pDatStream >> n;
            // Assume that normal ASCII/ANSI/ISO/etc. text doesn't start with
            // control characters except CR,LF,TAB
            if ( (n & 0xff00) < 0x2000 )
            {
                switch ( n & 0xff00 )
                {
                    case 0x0900 :
                    case 0x0a00 :
                    case 0x0d00 :
                    break;
                    default:
                        bPreselectUnicode = TRUE;
                }
            }
            pDatStream->Seek( nUniPos );
        }
        for ( j=0; j < CSV_PREVIEW_LINES; j++ )
        {
            pRowPosArrayUnicode[nArrayEndPosUnicode++] = pDatStream->Tell();
            if( !pDatStream->ReadUniStringLine( aPreviewLineUnicode[j] ) )
                break;
            // #84386# Reading Unicode on ASCII/ANSI data won't find any line
            // ends and therefor tries to read the whole file into strings.
            // Check if first line is completely filled and don't try any further.
            if ( j == 0 && aPreviewLineUnicode[j].Len() == STRING_MAXLEN )
                break;
        }
        nStreamPosUnicode = pDatStream->Tell();

        //  #107455# If the file content isn't unicode, ReadUniStringLine
        //  may try to seek beyond the file's end and cause a CANTSEEK error
        //  (depending on the stream type). The error code has to be cleared,
        //  or further read operations (including non-unicode) will fail.
        if ( pDatStream->GetError() == ERRCODE_IO_CANTSEEK )
            pDatStream->ResetError();
    }

    aNfRow.SetModifyHdl( LINK( this, ScImportAsciiDlg, FirstRowHdl ) );

    // *** Separator characters ***
    lcl_FillCombo( aCbTextSep, aTextSepList, 34 );      // Default "

    Link aSeparatorHdl =LINK( this, ScImportAsciiDlg, SeparatorHdl );
    aCbTextSep.SetSelectHdl( aSeparatorHdl );
    aCbTextSep.SetModifyHdl( aSeparatorHdl );
    aCkbTab.SetClickHdl( aSeparatorHdl );
    aCkbSemicolon.SetClickHdl( aSeparatorHdl );
    aCkbComma.SetClickHdl( aSeparatorHdl );
    aCkbAsOnce.SetClickHdl( aSeparatorHdl );
    aCkbSpace.SetClickHdl( aSeparatorHdl );
    aCkbOther.SetClickHdl( aSeparatorHdl );
    aEdOther.SetModifyHdl( aSeparatorHdl );

    // *** text encoding ListBox ***
    // all encodings allowed, including Unicode, but subsets are excluded
    aLbCharSet.FillFromTextEncodingTable( TRUE );
    // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
    // independent document linkage.
    aLbCharSet.InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, aCharSetUser );
    aLbCharSet.SelectTextEncoding( bPreselectUnicode ?
        RTL_TEXTENCODING_UNICODE : gsl_getSystemTextEncoding() );
    SetSelectedCharSet();
    aLbCharSet.SetSelectHdl( LINK( this, ScImportAsciiDlg, CharSetHdl ) );

    // *** column type ListBox ***
    xub_StrLen nCount = aColumnUser.GetTokenCount();
    for (xub_StrLen i=0; i<nCount; i++)
        aLbType.InsertEntry( aColumnUser.GetToken( i ) );

    aLbType.SetSelectHdl( LINK( this, ScImportAsciiDlg, LbColTypeHdl ) );
    aFtType.Disable();
    aLbType.Disable();

    // *** table box preview ***
    maTableBox.SetUpdateTextHdl( LINK( this, ScImportAsciiDlg, UpdateTextHdl ) );
    maTableBox.InitTypes( aLbType );
    maTableBox.SetColTypeHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );

    if(!bVFlag)
        maTableBox.Execute( CSVCMD_SETLINECOUNT, ASCIIDLG_MAXROWS );

    aRbSeparated.SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    aRbFixed.SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );

    SetupSeparatorCtrls();
    RbSepFixHdl( &aRbFixed );

    maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
}


ScImportAsciiDlg::~ScImportAsciiDlg()
{
    delete[] pRowPosArray;
    delete[] pRowPosArrayUnicode;

}

void ScImportAsciiDlg::GetOptions( ScAsciiOptions& rOpt )
{
    rOpt.SetCharSet( meCharSet );
    rOpt.SetCharSetSystem( mbCharSetSystem );
    rOpt.SetFixedLen( aRbFixed.IsChecked() );
    rOpt.SetStartRow( (USHORT)aNfRow.GetValue() );
    maTableBox.FillColumnData( rOpt );
    if( aRbSeparated.IsChecked() )
    {
        rOpt.SetFieldSeps( GetSeparators() );
        rOpt.SetMergeSeps( aCkbAsOnce.IsChecked() );
        rOpt.SetTextSep( lcl_CharFromCombo( aCbTextSep, aTextSepList ) );
    }
}

void ScImportAsciiDlg::SetSelectedCharSet()
{
    meCharSet = aLbCharSet.GetSelectTextEncoding();
    mbCharSetSystem = (meCharSet == RTL_TEXTENCODING_DONTKNOW);
    if( mbCharSetSystem )
        meCharSet = gsl_getSystemTextEncoding();
}

String ScImportAsciiDlg::GetSeparators() const
{
    String aSepChars;
    if( aCkbTab.IsChecked() )
        aSepChars += '\t';
    if( aCkbSemicolon.IsChecked() )
        aSepChars += ';';
    if( aCkbComma.IsChecked() )
        aSepChars += ',';
    if( aCkbSpace.IsChecked() )
        aSepChars += ' ';
    if( aCkbOther.IsChecked() )
        aSepChars += aEdOther.GetText();
    return aSepChars;
}

void ScImportAsciiDlg::SetupSeparatorCtrls()
{
    BOOL bEnable = aRbSeparated.IsChecked();
    aCkbTab.Enable( bEnable );
    aCkbSemicolon.Enable( bEnable );
    aCkbComma.Enable( bEnable );
    aCkbSpace.Enable( bEnable );
    aCkbOther.Enable( bEnable );
    aEdOther.Enable( bEnable && aCkbOther.IsChecked() );
    aCkbAsOnce.Enable( bEnable );
    aFtTextSep.Enable( bEnable );
    aCbTextSep.Enable( bEnable );
}

void ScImportAsciiDlg::UpdateVertical( bool bSwitchToFromUnicode )
{
    if ( bSwitchToFromUnicode )
    {
        bVFlag = FALSE;
        maTableBox.Execute( CSVCMD_SETLINECOUNT, ASCIIDLG_MAXROWS );
    }
    ULONG nNew = 0;
    if(!bVFlag)
    {
        // dragging the scrollbar -> read entire file
        bVFlag=TRUE;
        ULONG nRows = 0;

        pDatStream->Seek(0);
        if ( meCharSet == RTL_TEXTENCODING_UNICODE )
        {
            String aStringUnicode;
            pDatStream->StartReadingUnicodeText();
            ULONG* pPtrRowPos = pRowPosArrayUnicode;
            *pPtrRowPos++ = 0;
            while( pDatStream->ReadUniStringLine( aStringUnicode ) )
            {
                nRows++;
                if( nRows > ASCIIDLG_MAXROWS )
                    break;
                *pPtrRowPos++ = pDatStream->Tell();
            }
            nStreamPosUnicode = pDatStream->Tell();

            if ( pDatStream->GetError() == ERRCODE_IO_CANTSEEK )    // #107455# reset seek error
                pDatStream->ResetError();
        }
        else
        {
            ByteString aString;
            ULONG* pPtrRowPos = pRowPosArray;
            *pPtrRowPos++ = 0;
            while( pDatStream->ReadLine( aString ) )
            {
                nRows++;
                if( nRows > ASCIIDLG_MAXROWS )
                    break;
                *pPtrRowPos++ = pDatStream->Tell();
            }
            nStreamPos = pDatStream->Tell();
        }

        maTableBox.Execute( CSVCMD_SETLINECOUNT, nRows );
    }

    nNew = maTableBox.GetFirstVisLine();

    if ( meCharSet == RTL_TEXTENCODING_UNICODE )
    {
        if ( bVFlag || nNew <= nArrayEndPosUnicode )
            pDatStream->Seek( pRowPosArrayUnicode[nNew] );
        else
            pDatStream->Seek( nStreamPosUnicode );
        for ( USHORT j=0; j < CSV_PREVIEW_LINES; j++ )
        {
            if( !bVFlag && nNew+j >= nArrayEndPos )
            {
                pRowPosArrayUnicode[nNew+j] = pDatStream->Tell();
                nArrayEndPosUnicode = (USHORT) nNew+j;
            }
            // #84386# Reading Unicode on ASCII/ANSI data won't find any line
            // ends and therefor tries to read the whole file into strings.
            // Check if first line is completely filled and don't try any further.
            if( (!pDatStream->ReadUniStringLine( aPreviewLineUnicode[j] ) ||
                 (j == 0 && aPreviewLineUnicode[j].Len() == STRING_MAXLEN)) &&
                 !bVFlag )
            {
                bVFlag = TRUE;
                maTableBox.Execute( CSVCMD_SETLINECOUNT, nArrayEndPosUnicode );
            }
        }
        nStreamPosUnicode = pDatStream->Tell();

        if ( pDatStream->GetError() == ERRCODE_IO_CANTSEEK )    // #107455# reset seek error
            pDatStream->ResetError();
    }
    else
    {
        if ( bVFlag || nNew <= nArrayEndPos )
            pDatStream->Seek( pRowPosArray[nNew] );
        else
            pDatStream->Seek( nStreamPos );
        for ( USHORT j=0; j < CSV_PREVIEW_LINES; j++ )
        {
            if( !bVFlag && nNew+j >= nArrayEndPos )
            {
                pRowPosArray[nNew+j] = pDatStream->Tell();
                nArrayEndPos = (USHORT) nNew+j;
            }
            if( !pDatStream->ReadLine( aPreviewLine[j] ) && !bVFlag )
            {
                bVFlag = TRUE;
                maTableBox.Execute( CSVCMD_SETLINECOUNT, nArrayEndPos );
            }
        }
        nStreamPos = pDatStream->Tell();
    }
}


// ----------------------------------------------------------------------------

IMPL_LINK( ScImportAsciiDlg, RbSepFixHdl, RadioButton*, pButton )
{
    DBG_ASSERT( pButton, "ScImportAsciiDlg::RbSepFixHdl - missing sender" );

    if( (pButton == &aRbFixed) || (pButton == &aRbSeparated) )
    {
        SetPointer( Pointer( POINTER_WAIT ) );
        if( aRbFixed.IsChecked() )
            maTableBox.SetFixedWidthMode();
        else
            maTableBox.SetSeparatorsMode();
        SetPointer( Pointer( POINTER_ARROW ) );

        SetupSeparatorCtrls();
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, SeparatorHdl, Control*, pCtrl )
{
    DBG_ASSERT( pCtrl, "ScImportAsciiDlg::SeparatorHdl - missing sender" );
    DBG_ASSERT( !aRbFixed.IsChecked(), "ScImportAsciiDlg::SeparatorHdl - not allowed in fixed width" );

    aEdOther.Enable( aCkbOther.IsChecked() );
    if( (pCtrl == static_cast< Control* >( &aCkbOther )) && aEdOther.IsEnabled() )
        aEdOther.GrabFocus();
    maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, CharSetHdl, SvxTextEncodingBox*, pCharSetBox )
{
    DBG_ASSERT( pCharSetBox, "ScImportAsciiDlg::CharSetHdl - missing sender" );

    if( (pCharSetBox == &aLbCharSet) && (pCharSetBox->GetSelectEntryCount() == 1) )
    {
        SetPointer( Pointer( POINTER_WAIT ) );
        CharSet eOldCharSet = meCharSet;
        SetSelectedCharSet();
        if( (meCharSet == RTL_TEXTENCODING_UNICODE) != (eOldCharSet == RTL_TEXTENCODING_UNICODE) )
        {
            // switching to/from Unicode invalidates all positions
            if( pDatStream )
                UpdateVertical( TRUE );
        }
        maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
        SetPointer( Pointer( POINTER_ARROW ) );
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, FirstRowHdl, NumericField*, pNumField )
{
    DBG_ASSERT( pNumField, "ScImportAsciiDlg::FirstRowHdl - missing sender" );
    maTableBox.Execute( CSVCMD_SETFIRSTIMPORTLINE, pNumField->GetValue() - 1 );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, LbColTypeHdl, ListBox*, pListBox )
{
    DBG_ASSERT( pListBox, "ScImportAsciiDlg::LbColTypeHdl - missing sender" );
    if( pListBox == &aLbType )
        maTableBox.Execute( CSVCMD_SETCOLUMNTYPE, pListBox->GetSelectEntryPos() );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, UpdateTextHdl, ScCsvTableBox*, pTableBox )
{
    DBG_ASSERT( pTableBox, "ScImportAsciiDlg::UpdateTextHdl - missing sender" );

    BOOL bVFlag1 = bVFlag;
    if( pDatStream )
        UpdateVertical();
    if( bVFlag != bVFlag1 )
        UpdateVertical();

    sal_Unicode cTextSep = lcl_CharFromCombo( aCbTextSep, aTextSepList );
    bool bMergeSep = (aCkbAsOnce.IsChecked() == TRUE);

    if( meCharSet == RTL_TEXTENCODING_UNICODE )
        maTableBox.SetUniStrings( aPreviewLineUnicode, GetSeparators(), cTextSep, bMergeSep );
    else
        maTableBox.SetByteStrings( aPreviewLine, meCharSet, GetSeparators(), cTextSep, bMergeSep );

    return 0;
}

IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox*, pTableBox )
{
    DBG_ASSERT( pTableBox, "ScImportAsciiDlg::ColTypeHdl - missing sender" );

    sal_Int32 nType = pTableBox->GetSelColumnType();
    sal_Int32 nTypeCount = aLbType.GetEntryCount();
    bool bEmpty = (nType == CSV_TYPE_MULTI);
    bool bEnable = ((0 <= nType) && (nType < nTypeCount)) || bEmpty;

    aFtType.Enable( bEnable );
    aLbType.Enable( bEnable );

    Link aSelHdl = aLbType.GetSelectHdl();
    aLbType.SetSelectHdl( Link() );
    if( bEmpty )
        aLbType.SetNoSelection();
    else if( bEnable )
        aLbType.SelectEntryPos( static_cast< sal_uInt16 >( nType ) );
    aLbType.SetSelectHdl( aSelHdl );

    return 0;
}

