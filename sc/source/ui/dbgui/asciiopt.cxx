/*************************************************************************
 *
 *  $RCSfile: asciiopt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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

#ifndef PCH
#include "segmentc.hxx"
#endif

#include "global.hxx"
#include "scresid.hxx"
#include "impex.hxx"
#include "asciiopt.hxx"
#include "asciiopt.hrc"

#include <vcl/system.hxx>


//------------------------------------------------------------------------

#define ASCIIDLG_MAXFIELDS      256

#ifdef WIN
    #define ASCIIDLG_MAXROWS                10000
#else
    #define ASCIIDLG_MAXROWS                32000
#endif

 // da max. 5 Decimalstellen
#define ASCIIDLG_1COL_SIZE          5
#define ASCIIDLG_LASTCOL_SIZE       10000

static const sal_Char __FAR_DATA pStrFix[] = "FIX";
static const sal_Char __FAR_DATA pStrMrg[] = "MRG";

//  Liste der CharSet-Werte passend zum String SCSTR_CHARSET_USER

static CharSet eCharSetList[] =
    { RTL_TEXTENCODING_MS_1252,         // "Ansi"
      RTL_TEXTENCODING_APPLE_ROMAN,     // "Mac"
      RTL_TEXTENCODING_IBM_850,         // "IBMPC"
      RTL_TEXTENCODING_IBM_437,         // "IBMPC (437)"
      RTL_TEXTENCODING_IBM_850,         // "IBMPC (850)"
      RTL_TEXTENCODING_IBM_860,         // "IBMPC (860)"
      RTL_TEXTENCODING_IBM_861,         // "IBMPC (861)"
      RTL_TEXTENCODING_IBM_863,         // "IBMPC (863)"
      RTL_TEXTENCODING_IBM_865,         // "IBMPC (865)"
      RTL_TEXTENCODING_DONTKNOW };      // "System" (handled in GetCharSetFromList)


//  Liste der Spaltentypen passend zum String SCSTR_COLUMN_USER

static BYTE nColTypeList[] =
    { SC_COL_STANDARD, SC_COL_TEXT, SC_COL_DMY, SC_COL_MDY, SC_COL_YMD,
        SC_COL_ENGLISH, SC_COL_SKIP };


SEG_EOFGLOBALS()

#pragma SEG_FUNCDEF(asciiopt_07)

String lcl_GetCharsetString(CharSet eCharSet)       //! nach global oder so verschieben
{
    const sal_Char* pChar;
    switch (eCharSet)
    {
        case RTL_TEXTENCODING_MS_1252:      pChar = "ANSI";         break;
        case RTL_TEXTENCODING_APPLE_ROMAN:  pChar = "MAC";          break;
        // IBMPC == IBMPC_850
        case RTL_TEXTENCODING_IBM_437:      pChar = "IBMPC_437";    break;
        case RTL_TEXTENCODING_IBM_850:      pChar = "IBMPC_850";    break;
        case RTL_TEXTENCODING_IBM_860:      pChar = "IBMPC_860";    break;
        case RTL_TEXTENCODING_IBM_861:      pChar = "IBMPC_861";    break;
        case RTL_TEXTENCODING_IBM_863:      pChar = "IBMPC_863";    break;
        case RTL_TEXTENCODING_IBM_865:      pChar = "IBMPC_865";    break;
        default:                            pChar = "SYSTEM";       break;
    }
    return String::CreateFromAscii(pChar);
}

CharSet lcl_GetCharSetFromList( USHORT nPos )
{
    CharSet eRet = eCharSetList[nPos];
    if (eRet == RTL_TEXTENCODING_DONTKNOW)
        eRet = gsl_getSystemTextEncoding();
    return eRet;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(asciiopt_01)

ScAsciiOptions::ScAsciiOptions() :
    bFixedLen       ( FALSE ),
    aFieldSeps      ( ';' ),
    bMergeFieldSeps ( FALSE ),
    cTextSep        ( 34 ),
    eCharSet        ( gsl_getSystemTextEncoding() ),
    nStartRow       ( 1 ),
    nInfoCount      ( 0 ),
    pColStart       ( NULL ),
    pColFormat      ( NULL )
{
}

#pragma SEG_FUNCDEF(asciiopt_02)

ScAsciiOptions::ScAsciiOptions(const ScAsciiOptions& rOpt) :
    bFixedLen       ( rOpt.bFixedLen ),
    aFieldSeps      ( rOpt.aFieldSeps ),
    bMergeFieldSeps ( rOpt.bMergeFieldSeps ),
    cTextSep        ( rOpt.cTextSep ),
    eCharSet        ( rOpt.eCharSet ),
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

#pragma SEG_FUNCDEF(asciiopt_03)

ScAsciiOptions::~ScAsciiOptions()
{
    delete[] pColStart;
    delete[] pColFormat;
}

#pragma SEG_FUNCDEF(asciiopt_0a)

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

#pragma SEG_FUNCDEF(asciiopt_04)

ScAsciiOptions& ScAsciiOptions::operator=( const ScAsciiOptions& rCpy )
{
    SetColInfo( rCpy.nInfoCount, rCpy.pColStart, rCpy.pColFormat );

    bFixedLen       = rCpy.bFixedLen;
    aFieldSeps      = rCpy.aFieldSeps;
    bMergeFieldSeps = rCpy.bMergeFieldSeps;
    cTextSep        = rCpy.cTextSep;
    eCharSet        = rCpy.eCharSet;
    nStartRow       = rCpy.nStartRow;

    return *this;
}

#pragma SEG_FUNCDEF(asciiopt_08)

BOOL ScAsciiOptions::operator==( const ScAsciiOptions& rCmp ) const
{
    if ( bFixedLen       == rCmp.bFixedLen &&
         aFieldSeps      == rCmp.aFieldSeps &&
         bMergeFieldSeps == rCmp.bMergeFieldSeps &&
         cTextSep        == rCmp.cTextSep &&
         eCharSet        == rCmp.eCharSet &&
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

#pragma SEG_FUNCDEF(asciiopt_05)

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
        eCharSet = GetCharsetValue( aToken );
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

#pragma SEG_FUNCDEF(asciiopt_06)

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

    aOutStr += lcl_GetCharsetString( eCharSet );
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
#pragma SEG_FUNCDEF(asciiopt_09)

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

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(asciiopt_0b)

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

#pragma SEG_FUNCDEF(asciiopt_0c)

sal_Unicode lcl_CharFromCombo( ComboBox& rCombo, const String& rList )
{
    sal_Unicode c = 0;
    String aStr = rCombo.GetText();
    if ( aStr.Len() )
    {
        xub_StrLen nCount = rList.GetTokenCount('\t');
        for ( xub_StrLen i=0; i<nCount; i+=2 )
            if ( ScGlobal::pScInternational->CompareEqual(
                    aStr, rList.GetToken(i,'\t'), INTN_COMPARE_IGNORECASE ) )
            {
                c = (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32();
            }
        if (!c)
            c = (sal_Unicode) aStr.ToInt32();       // Ascii
    }
    return c;
}

#pragma SEG_FUNCDEF(asciiopt_0d)

ScImportAsciiDlg::ScImportAsciiDlg( Window* pParent,String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep ) :
        ModalDialog ( pParent, ScResId( RID_SCDLG_ASCII ) ),
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

        aTableBox   ( this, ScResId( CTR_TABLE  ) ),
        aFtCharSet  ( this, ScResId( FT_CHARSET ) ),
        aLbCharSet  ( this, ScResId( LB_CHARSET ) ),
        aGbSepOpt   ( this, ScResId( GB_SEPOPT ) ),
        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aCbTextSep  ( this, ScResId( CB_TEXTSEP ) ),
        aGbFieldOpt ( this, ScResId( GB_FIELDOPT ) ),
        aCkbAsOnce  ( this, ScResId( CB_ASONCE) ),
        aFtType     ( this, ScResId( FT_TYPE ) ),
        aLbType     ( this, ScResId( LB_TYPE1 ) ),
        aScrollbar  ( this, ScResId( SB_COLUMN ) ),
        aVScroll    ( this, ScResId( SB_ROW ) ),
        aGbWidth    ( this, ScResId( GB_WIDTH ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),
        aStringCol  ( ScResId( STR_COL ) ),
        aStringTo   ( ScResId( STR_TO ) ),
        aCharSetUser( ScResId( SCSTR_CHARSET_USER ) ),
        aColumnUser ( ScResId( SCSTR_COLUMN_USER ) ),
        aFldSepList ( ScResId( SCSTR_FIELDSEP ) ),
        aTextSepList( ScResId( SCSTR_TEXTSEP ) )
{
    String aName = GetText();
    aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" - ["));
    aName += aDatName;
    aName += ']';
    SetText( aName );

    FreeResource();
    bVFlag=FALSE;
    String aSizeString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("00000"));
    Font aTBFont=System::GetStandardFont( STDFONT_FIXED );
    Size aTBSize=aTableBox.GetFont().GetSize();
    aTBSize.Width()=aTBFont.GetSize().Width();
    aTBFont.SetSize(aTBSize);
    aTableBox.SetFont(aTBFont);

    pDatStream=pInStream;
    aTableBox.InsertCol(0,2);
    aTableBox.InsertRow(0,4);

    aTableBox.SetNumOfCharsForCol(0,aSizeString.Len());

    switch(cSep)
    {
        case '\t'   :aCkbTab        .Check();break;
        case ';'    :aCkbSemicolon  .Check();break;
        case ','    :aCkbComma      .Check();break;
        case ' '    :aCkbSpace      .Check();break;
        default     :aCkbOther      .Check();
                     aEdOther.SetText(cSep);
                     break;
    }

    nArrayEndPos=0;
    USHORT nField;
    if(pDatStream!=NULL)
    {
        pRowPosArray=new ULONG[ASCIIDLG_MAXROWS+2];
        if(pRowPosArray!=NULL)
        {
            ULONG *pPtrRowPos=pRowPosArray;
            for(nField=0;nField<ASCIIDLG_MAXROWS;nField++)
            {
                *pPtrRowPos++=0;
            }
        }
        pDatStream->SetBufferSize(ASCIIDLG_MAXROWS);
        pDatStream->SetStreamCharSet( gsl_getSystemTextEncoding() );    //!???
        pDatStream->Seek( 0 );
        for ( USHORT j=0; j < SC_ASCIIOPT_PREVIEW_LINES; j++ )
        {
            pRowPosArray[nArrayEndPos++]=pDatStream->Tell();
            if(!pDatStream->ReadLine( aPreviewLine[j] ))
            {
                bVFlag=TRUE;
                aVScroll.SetPageSize( aTableBox.GetYMaxVisChars()-1);
                aVScroll.SetThumbPos(0);
                aVScroll.SetRange( Range( 0, j) );
                aVScroll.SetVisibleSize(j);
                break;
            }
        }
    }
    nScrollPos = 0;
    nUsedCols = 0;
    pEndValues  = new USHORT[ASCIIDLG_MAXFIELDS];
    pFlags      = new BYTE[ASCIIDLG_MAXFIELDS];
    for (nField=0; nField<ASCIIDLG_MAXFIELDS; nField++)
    {
        pEndValues[nField] = 0;
        pFlags[nField] = SC_COL_STANDARD;
    }

    lcl_FillCombo( aCbTextSep, aTextSepList, 34 );      // Default "

    Link aVarSepLink=LINK( this, ScImportAsciiDlg, VarSepHdl );
    aCbTextSep.SetSelectHdl( aVarSepLink);
    aCbTextSep.SetModifyHdl( aVarSepLink);

    aCkbTab.SetClickHdl( aVarSepLink);
    aCkbSemicolon.SetClickHdl(aVarSepLink);
    aCkbComma.SetClickHdl( aVarSepLink);
    aCkbAsOnce.SetClickHdl( aVarSepLink);
    aCkbSpace.SetClickHdl( aVarSepLink);
    aCkbOther.SetClickHdl( aVarSepLink);
    aEdOther.SetModifyHdl( aVarSepLink);

    aTableBox.SetSelectionHdl(LINK( this, ScImportAsciiDlg, SelectHdl ));

    //  Zeichensatz - Listbox

    xub_StrLen nCount = aCharSetUser.GetTokenCount();
    xub_StrLen i;
    for (i=0; i<nCount; i++)
        aLbCharSet.InsertEntry(aCharSetUser.GetToken(i));
    aLbCharSet.SelectEntryPos( nCount - 1 );                // "System" ganz hinten
    GetCharSet();
    aLbCharSet.SetSelectHdl( LINK( this, ScImportAsciiDlg, CharSetHdl ) );

    //  Spaltentyp - Listboxen

    ListBox* pType =&aLbType;
    nCount = aColumnUser.GetTokenCount();
    for (i=0; i<nCount; i++)
    {
        String aToken = aColumnUser.GetToken(i);
        pType->InsertEntry(aToken);
        aTableBox.InsertContextEntry(100+i,aToken);
    }
    pType->SetSelectHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );
    aTableBox.SetModifyColHdl(LINK( this, ScImportAsciiDlg, ColTypeHdl ) );

    aScrollbar.SetRange( Range( 0, ASCIIDLG_MAXFIELDS ) );      // initial getrennt
    aScrollbar.SetPageSize( aTableBox.GetXMaxVisChars() );
    //aScrollbar.SetVisibleSize( 3 );
    aScrollbar.SetScrollHdl( LINK( this, ScImportAsciiDlg, ScrollHdl ) );

    if(!bVFlag)
    {
        aVScroll.SetRange( Range( 0, ASCIIDLG_MAXROWS ) );      // initial getrennt
        aVScroll.SetPageSize( aTableBox.GetYMaxVisChars()-2 );
        aVScroll.SetVisibleSize(aTableBox.GetYMaxVisChars()-1);
    }
    aVScroll.SetScrollHdl( LINK( this, ScImportAsciiDlg, ScrollHdl ) );
    aRbSeparated.SetClickHdl( LINK( this, ScImportAsciiDlg, VarFixHdl ) );
    aRbFixed.SetClickHdl( LINK( this, ScImportAsciiDlg, VarFixHdl ) );

    CheckScrollPos();
    CheckColTypes(FALSE);
    CheckValues();
    CheckDisable();
    VarFixHdl(&aRbFixed);
}

#pragma SEG_FUNCDEF(asciiopt_0e)

ScImportAsciiDlg::~ScImportAsciiDlg()
{
    delete[] pEndValues;
    delete[] pFlags;
    delete[] pRowPosArray;

}

#pragma SEG_FUNCDEF(asciiopt_0f)

IMPL_LINK( ScImportAsciiDlg, VarFixHdl, void *, pCtr )
{
    if((RadioButton *)pCtr==&aRbFixed ||(RadioButton *)pCtr==&aRbSeparated )
    {
        aTableBox.ClearTable();
        aTableBox.InsertRow(0,4);
        aTableBox.InsertCol(0,2);
        aTableBox.SetNumOfCharsForCol(0,ASCIIDLG_1COL_SIZE);
        for (int i=0; i<ASCIIDLG_MAXFIELDS; i++)
        {
            pFlags[i] = SC_COL_STANDARD;
        }
        aLbType.SelectEntryPos(0);
        CheckValues();
        CheckScrollRange();
        ScrollHdl(&aScrollbar);
        CheckDisable();
        aTableBox.SetSelectedCol(1);
    }
    else
    {
        CheckValues();
        CheckScrollRange();
        ScrollHdl(&aScrollbar);
        CheckDisable();
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, VarSepHdl, void*, pCtr )
{
    if (!aRbFixed.IsChecked())
    {
        aTableBox.ClearTable();
        aTableBox.InsertRow(0,4);
        aTableBox.InsertCol(0,2);
        aTableBox.SetNumOfCharsForCol(0,5); // da max. 5 Decimalstellen
        if((Edit *)pCtr==&aEdOther)
        {
            aCkbOther.Check((aEdOther.GetText().Len()>0));
        }

        DelimitedPreview();         // Preview mit neuen Einstellungen
        CheckScrollRange();
        ScrollHdl(&aScrollbar);
    }
    return 0;
}

#pragma SEG_FUNCDEF(asciiopt_10)

IMPL_LINK( ScImportAsciiDlg, ScrollHdl, void*, pScroll )
{
    if((ScrollBar *)pScroll==&aScrollbar)
    {
        long nNew = aScrollbar.GetThumbPos();
        if ( nNew >= 0)
        {
            nScrollPos = (USHORT) nNew;

            CheckScrollPos();
            CheckColTypes(FALSE);
            CheckValues();
            String aSizeString('W');
            aTableBox.SetXTablePos((short)(nNew *
                aTableBox.GetTextWidth(aSizeString)));
        }
    }
    else if((ScrollBar *)pScroll==&aVScroll)
    {
        BOOL bVFlag1=bVFlag;
        if(pDatStream !=NULL && pRowPosArray!=NULL) UpdateVertical();
        if(bVFlag!=bVFlag1) UpdateVertical();
        CheckScrollPos();
        CheckColTypes(FALSE);
        CheckValues();
        CheckScrollRange();
        //CheckDisable();
    }
    return 0;
}

#pragma SEG_FUNCDEF(asciiopt_11)

IMPL_LINK( ScImportAsciiDlg, CharSetHdl, void*, EMPTY_ARG )
{
    if ( aLbCharSet.GetSelectEntryCount() == 1 )
    {
        USHORT nSel = aLbCharSet.GetSelectEntryPos();
        eCharSet = lcl_GetCharSetFromList(nSel);

        CheckValues();              // Preview anpassen
    }

    return 0;
}

#pragma SEG_FUNCDEF(asciiopt_12)

IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, void*, pCtr )
{
    CheckColTypes( TRUE, pCtr);

    return 0;
}

#pragma SEG_FUNCDEF(asciiopt_13)

IMPL_LINK( ScImportAsciiDlg, SelectHdl, ScTableWithRuler*, pModified )
{
    long nPos=aTableBox.GetSelectedCol();

    if(aTableBox.GetSelectedCol()>0)
    {

        if ( nPos-1 < ASCIIDLG_MAXFIELDS )
        {
            BYTE nType = pFlags[nPos-1];
            USHORT nCount = sizeof(nColTypeList) / sizeof(BYTE);
            USHORT nPos1 = nCount-1;
            for (USHORT j=0; j<nCount; j++)
                if ( nColTypeList[j] == nType )
                    nPos1 = j;
            aLbType.SelectEntryPos(nPos1);
        }
    }
    return 0;
}

void ScImportAsciiDlg::CheckScrollRange()
{
    USHORT nScrollEnd = nUsedCols + 1;
    if ( nScrollEnd < 3 )
        nScrollEnd = 3;
    String aSizeString('W');

    long nMaxSize=aTableBox.GetXMaxTableSize();

    xub_StrLen nNumOfChars=aTableBox.GetNumOfCharsForCol(0);
    if(nMaxSize<0) nMaxSize=0;
    aScrollbar.SetRange(Range(0,nNumOfChars+nMaxSize/aTableBox.GetTextWidth(aSizeString)));
    aScrollbar.SetPageSize((long)(aTableBox.GetXMaxVisChars()-nNumOfChars));
    aScrollbar.SetVisibleSize((long)(aTableBox.GetXMaxVisChars()-nNumOfChars));

    if(aScrollbar.GetThumbPos()>nMaxSize)
    {
        aScrollbar.SetThumbPos(0);
    }
}

#pragma SEG_FUNCDEF(asciiopt_14)

void ScImportAsciiDlg::CheckValues( BOOL bReadVal, USHORT nEditField )
{
//  String aSizeString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("999"));
    USHORT i;

    if ( !aRbFixed.IsChecked() )
    {
        DelimitedPreview();     // nur Preview anpassen
        return;
    }

    aTableBox.RulerEnabled();
    //  Dialog anpassen
    USHORT  nNumOfCols=aTableBox.GetNumberOfCols();
    xub_StrLen nVal = 0;
    USHORT nRowNum=aVScroll.GetThumbPos()+1;
    String aField;

    for ( USHORT j=0; j < SC_ASCIIOPT_PREVIEW_LINES; j++ )
    {
        aField = String::CreateFromInt32( nRowNum++ );

        String aPreviewConv( aPreviewLine[j], eCharSet );
        aTableBox.SetDataAtRowCol(j+1,0,aField);
        //aTableBox.SetNumOfCharsForCol(0,aField.Len()+2);

        String aPrevStr;

        if(nNumOfCols>2) nVal= 0;
        xub_StrLen nPrevious = 0;
        for (i=0; i<nNumOfCols-1; i++)
        {
            USHORT nPos = i+1;
            if ( nPos < ASCIIDLG_MAXFIELDS )
            {
                if(nNumOfCols<=2)
                {
                    if( nVal < aPreviewConv.Len())
                    {
                        nVal=aPreviewConv.Len();
                        if(nVal<aTableBox.GetXMaxVisChars()-ASCIIDLG_1COL_SIZE)
                            nVal=aTableBox.GetXMaxVisChars();
                    }
                }
                else
                {
                    nVal += aTableBox.GetNumOfCharsForCol(nPos);
                }

                String aStart = String::CreateFromInt32(nPrevious);
                aStart += ' ';
                aStart += aStringTo;
                if(i+1<nNumOfCols-1)
                {
                    aPrevStr = aPreviewConv.Copy( nPrevious, nVal-nPrevious );
                    aTableBox.SetNumOfCharsForCol(i+1, nVal-nPrevious );
                }
                else
                {
                    aPrevStr = aPreviewConv.Copy(nPrevious);
                    if(aPrevStr.Len()>aTableBox.GetNumOfCharsForCol(i+1))
                    {
                        aTableBox.SetNumOfCharsForCol(i+1, aPrevStr.Len());
                    }
                }
                aTableBox.SetDataAtRowCol(j+1,i+1,aPrevStr);
                nPrevious  += aTableBox.GetNumOfCharsForCol(nPos);
            }
        }
    }
    aTableBox.SetNumOfCharsForCol(0,aField.Len()+2);
    nNumOfCols=aTableBox.GetNumberOfCols();
    for (i=0; i<nNumOfCols-1; i++)
    {
        if ( i < ASCIIDLG_MAXFIELDS )
        {
            BYTE nType = pFlags[i];
            USHORT nCount = sizeof(nColTypeList) / sizeof(BYTE);
            USHORT nPos1 = nCount-1;
            for (USHORT j=0; j<nCount; j++)
                if ( nColTypeList[j] == nType )
                    nPos1 = j;
            aTableBox.SetDataAtRowCol(0,i+1,aLbType.GetEntry(nPos1));
        }
    }
}

void ScImportAsciiDlg::DelimitedPreview()
{
    aTableBox.RulerEnabled(FALSE);
    String aSeps;
    if(aCkbTab.IsChecked())         aSeps+='\t';
    if(aCkbSemicolon.IsChecked())   aSeps+=';';
    if(aCkbComma.IsChecked())       aSeps+=',';
    if(aCkbSpace.IsChecked())       aSeps+=' ';
    if(aCkbOther.IsChecked())
    {
        aSeps+=aEdOther.GetText().GetChar(0);
    }
    BOOL bMerge = FALSE;
    if(aCkbAsOnce.IsChecked()) bMerge=TRUE;

    sal_Unicode cTextSep = lcl_CharFromCombo( aCbTextSep, aTextSepList );

    //  Preview-String von vorne her aufteilen...
    USHORT nPos,nMaxCol;
    xub_StrLen nMaxWidth[ASCIIDLG_MAXFIELDS];
    int i;
    for (i=0; i<ASCIIDLG_MAXFIELDS; i++)
    {
        nMaxWidth[i]=5;
    }
    nMaxCol=0;
    USHORT nRowNum=aVScroll.GetThumbPos()+1;
    USHORT  nNumOfCols=aTableBox.GetNumberOfCols();
    String aField;

    const sal_Unicode* pSeps = aSeps.GetBuffer();
    for ( USHORT j=0; j < SC_ASCIIOPT_PREVIEW_LINES; j++ )
    {
        String aPreviewConv( aPreviewLine[j], eCharSet );

        const sal_Unicode* p = aPreviewConv.GetBuffer();
        String aPrevStr;
        aField = String::CreateFromInt32( nRowNum++ );
        aTableBox.SetDataAtRowCol(j+1,0,aField);
        //aTableBox.SetNumOfCharsForCol(0,aField.Len()+2);

        BOOL bCont = TRUE;
        for (nPos=0; nPos < ASCIIDLG_MAXFIELDS && bCont; nPos++)
        {
            if ( *p )
            {
                p = ScImportExport::ScanNextFieldFromString( p, aField, cTextSep, pSeps, bMerge );
                if ( !*p && aField.Len() && (ScGlobal::UnicodeStrChr( pSeps, *(p-1) ) == NULL) )
                    bCont = FALSE;      // letztes Feld, mit Inhalt
            }
            else
            {
                bCont = FALSE;          // letztes Feld, ohne Inhalt
                aField.Erase();
            }

            if(nMaxWidth[nPos+1]<aField.Len())
                nMaxWidth[nPos+1]=aField.Len();

            aTableBox.SetDataAtRowCol(j+1,nPos+1,aField);
        }
        if(nMaxCol<nPos+1) nMaxCol=nPos+1;
        aField.Erase();
        for(;nPos<nNumOfCols-1;nPos++)
            aTableBox.SetDataAtRowCol(j+1,nPos+1,aField);
    }
    aField = String::CreateFromInt32( nRowNum-1 );
    aTableBox.SetNumOfCharsForCol(0,aField.Len()+2);

    if(nMaxCol<nNumOfCols)
            aTableBox.DeleteCol(nMaxCol,nNumOfCols-nMaxCol);

    for (i=1; i<nMaxCol; i++)
    {
        aTableBox.SetNumOfCharsForCol(i,nMaxWidth[i]);
    }

    for (i=0; i<nMaxCol-1; i++)
    {
        if ( i < ASCIIDLG_MAXFIELDS )
        {
            BYTE nType = pFlags[i];
            USHORT nCount = sizeof(nColTypeList) / sizeof(BYTE);
            USHORT nPos1 = nCount-1;
            for (USHORT j=0; j<nCount; j++)
                if ( nColTypeList[j] == nType )
                    nPos1 = j;
            aTableBox.SetDataAtRowCol(0,i+1,aLbType.GetEntry(nPos1));
        }
    }
}

#pragma SEG_FUNCDEF(asciiopt_15)

void ScImportAsciiDlg::CheckScrollPos()
{
    /*
    FixedText* pTitle[3] = { &aFtCol1, &aFtCol2, &aFtCol3 };

    for (USHORT i=0; i<3; i++)
    {
        String aStr = aStringCol;
        aStr += ' ';
        aStr += ( nScrollPos + i + 1 );
        pTitle[i]->SetText(aStr);
    }
    */
}

void ScImportAsciiDlg::UpdateVertical()
{
    ByteString aString;
    ULONG   nRows=0;
    ULONG   nNew=0;
    double fNew;
    if(!bVFlag && aVScroll.GetType()==SCROLL_DRAG)
    {
        bVFlag=TRUE;
        pDatStream->Seek(0);

        ULONG *pPtrRowPos=pRowPosArray;
        *pPtrRowPos++=0;
        SetPointer(Pointer(POINTER_WAIT));
        while(pDatStream->ReadLine(aString))
        {
            nRows++;

            if(nRows>ASCIIDLG_MAXROWS) break;

            *pPtrRowPos++=pDatStream->Tell();
        }
        fNew=((double) (aVScroll.GetThumbPos())*nRows)/ASCIIDLG_MAXROWS;
        nNew =(ULONG) fNew;
        aVScroll.SetPageSize( aTableBox.GetYMaxVisChars()-1);
        aVScroll.SetThumbPos(nNew);
        aVScroll.SetRange( Range( 0, nRows) );
        SetPointer(Pointer(POINTER_ARROW));
    }

    nNew=aVScroll.GetThumbPos();

    if(bVFlag || nNew<=nArrayEndPos) pDatStream->Seek(pRowPosArray[nNew]);


    for ( USHORT j=0; j < SC_ASCIIOPT_PREVIEW_LINES; j++ )
    {
        if(!bVFlag && nNew+j>=nArrayEndPos)
        {
            pRowPosArray[nNew+j]=pDatStream->Tell();
            nArrayEndPos=(USHORT) nNew+j;
        }
        if(!pDatStream->ReadLine( aPreviewLine[j] )&& !bVFlag)
        {
            bVFlag=TRUE;
            aVScroll.SetPageSize( aTableBox.GetYMaxVisChars()-1);
            aVScroll.SetThumbPos(nNew-1);
            aVScroll.SetRange( Range( 0, nArrayEndPos) );
        }
    }
}


#pragma SEG_FUNCDEF(asciiopt_16)

void ScImportAsciiDlg::CheckColTypes(BOOL bReadVal,void *pCtr)
{
    USHORT i, nPos;
    BYTE nType = SC_COL_STANDARD;

    if((ScTableWithRuler*) pCtr== &aTableBox)
    {
        USHORT  nNumOfCols=aTableBox.GetNumberOfCols();
        for (i=1; i<nNumOfCols; i++)
        {
            nPos =i-1;
            if ( nPos < ASCIIDLG_MAXFIELDS )
            {
                String aString=aTableBox.GetDataAtRowCol(0,i);
                USHORT nSel = aLbType.GetEntryPos(aString);
                nType = nColTypeList[nSel];
                pFlags[nPos] = nType;
            }
        }
    }
    else
    {
        nPos = aTableBox.GetSelectedCol()-1;
        if ( nPos < ASCIIDLG_MAXFIELDS )
        {
            if ( aLbType.GetSelectEntryCount() == 1 )
            {
                USHORT nSel = aLbType.GetSelectEntryPos();
                nType = nColTypeList[nSel];
                aTableBox.SetDataAtRowCol(0,nPos+1,aLbType.GetSelectEntry());
                aTableBox.Update();
            }
            pFlags[nPos] = nType;
        }
    }
}

#pragma SEG_FUNCDEF(asciiopt_17)

void lcl_DoEnable( Window& rWin, BOOL bEnable )
{
    if (bEnable)
        rWin.Enable();
    else
        rWin.Disable();
    rWin.Invalidate();
    rWin.Update();

}

#pragma SEG_FUNCDEF(asciiopt_18)

void ScImportAsciiDlg::CheckDisable()
{
    BOOL bFix = aRbFixed.IsChecked();

    BOOL bVar = !bFix;
    lcl_DoEnable( aCkbTab,      bVar );
    lcl_DoEnable( aCkbSemicolon,bVar );
    lcl_DoEnable( aCkbComma,    bVar );
    lcl_DoEnable( aCkbSpace,    bVar );
    lcl_DoEnable( aCkbOther,    bVar );
    lcl_DoEnable( aEdOther,     bVar );
    lcl_DoEnable( aCkbAsOnce,   bVar );
    lcl_DoEnable( aFtTextSep,   bVar );
    lcl_DoEnable( aCbTextSep,   bVar );
}

#pragma SEG_FUNCDEF(asciiopt_19)

void ScImportAsciiDlg::GetCharSet()
{
    String aUser = aLbCharSet.GetSelectEntry();
    xub_StrLen nPos = 0;
    xub_StrLen nCount = aCharSetUser.GetTokenCount();
    for (xub_StrLen i=0; i<nCount; i++)
        if ( ScGlobal::pScInternational->CompareEqual(
                aUser, aCharSetUser.GetToken(i), INTN_COMPARE_IGNORECASE ) )
        {
            nPos = i;
        }
    eCharSet = lcl_GetCharSetFromList(nPos);
}

#pragma SEG_FUNCDEF(asciiopt_1a)

void ScImportAsciiDlg::GetOptions( ScAsciiOptions& rOpt )
{
    BOOL bFix = aRbFixed.IsChecked();

    rOpt.SetCharSet( eCharSet );
    rOpt.SetFixedLen( bFix );
    USHORT nRow=(USHORT)aNfRow.GetText().ToInt32();

    rOpt.SetStartRow(nRow);

    if ( bFix )
    {
        nUsedCols=aTableBox.GetNumberOfCols();
        xub_StrLen* pOptStart = new xub_StrLen[ nUsedCols+1 ];
        BYTE*   pOptType  = new BYTE[ nUsedCols+1 ];
        USHORT nCount = 0;
        xub_StrLen nLastEnd = 0;
        xub_StrLen nStart = 0;
        xub_StrLen nEnd   = 0;
        for (USHORT i=1; i<nUsedCols; i++)
        {
            nEnd+=aTableBox.GetNumOfCharsForCol(i);
            if ( nEnd > nStart )
            {
                pOptStart[nCount] = nStart;
                pOptType[nCount] = pFlags[i-1];
                ++nCount;
                nLastEnd = nEnd;
            }
            nStart +=aTableBox.GetNumOfCharsForCol(i);
        }
        pOptStart[nCount] = ASCIIDLG_LASTCOL_SIZE;//fuer den Rest
        //pOptStart[nCount] = nLastEnd;         // den Rest ueberspringen
        pOptType[nCount] = SC_COL_SKIP;
        ++nCount;

        rOpt.SetColInfo( nCount, pOptStart, pOptType );
        delete[] pOptStart;
        delete[] pOptType;
    }
    else
    {
        sal_Unicode cTextSep = lcl_CharFromCombo( aCbTextSep, aTextSepList );


        String aSeps;
        if(aCkbTab.IsChecked())         aSeps+='\t';
        if(aCkbSemicolon.IsChecked())   aSeps+=';';
        if(aCkbComma.IsChecked())       aSeps+=',';
        if(aCkbSpace.IsChecked())       aSeps+=' ';
        if(aCkbOther.IsChecked())
        {
            aSeps+=aEdOther.GetText().GetChar(0);
        }
        rOpt.SetMergeSeps(aCkbAsOnce.IsChecked());
        rOpt.SetFieldSeps( aSeps );
        rOpt.SetTextSep( cTextSep );

        //  Spaltentypen

        xub_StrLen* pColNum  = new xub_StrLen[ASCIIDLG_MAXFIELDS];
        BYTE*       pColType = new BYTE[ASCIIDLG_MAXFIELDS];

        USHORT nCount = 0;
        for (USHORT i=0; i<ASCIIDLG_MAXFIELDS; i++)
            if (pFlags[i] != SC_COL_STANDARD)
            {
                pColNum[nCount] = i+1;          // 1-based
                pColType[nCount] = pFlags[i];
                ++nCount;
            }

        rOpt.SetColInfo( nCount, pColNum, pColType );

        delete[] pColType;
        delete[] pColNum;
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.25  2000/09/17 14:08:55  willem.vandorp
    OpenOffice header added.

    Revision 1.24  2000/08/31 16:38:19  willem.vandorp
    Header and footer replaced

    Revision 1.23  2000/05/22 18:05:11  nn
    DelimitedPreview: right Field len

    Revision 1.22  2000/05/19 18:35:33  nn
    CreateFromInt32

    Revision 1.21  2000/04/28 18:55:41  nn
    include

    Revision 1.20  2000/04/14 17:38:02  nn
    unicode changes

    Revision 1.19  2000/02/11 12:22:37  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.18  1998/12/02 10:17:56  ANK
    #59981# Horizontale Begrenzung aufgehoben


      Rev 1.17   02 Dec 1998 11:17:56   ANK
   #59981# Horizontale Begrenzung aufgehoben

      Rev 1.16   19 Nov 1998 11:48:30   ER
   #59519# hmblgrmbl.. BSE und Alzheimer

      Rev 1.15   16 Nov 1998 20:51:54   ER
   #59519# DelimitedPreview: letzte Spalte auch anzeigen wenn gerade leer

      Rev 1.14   26 Aug 1998 17:11:36   ANK
   #55355# Positionierung nach Umschalten

      Rev 1.13   04 Jul 1998 19:57:22   ER
   opt: ScanNextFieldFromString mit const char* statt const String&

      Rev 1.12   23 Jun 1998 22:59:32   NN
   US-Englisch in Typ-Liste

      Rev 1.11   08 Jan 1998 16:19:38   ANK
   Flackern des Focus beseitigt

      Rev 1.10   18 Dec 1997 15:38:40   ANK
   VScroll- Verhalten geaendert

      Rev 1.9   16 Dec 1997 12:09:56   ANK
   Anfangszeile geaendert

      Rev 1.8   11 Dec 1997 17:24:22   ANK
   Erweiterungen fuer Tabellen-Control

      Rev 1.7   04 Dec 1997 17:20:58   ANK
   neue Controls

      Rev 1.6   15 Jul 1997 17:20:50   ER
   #41637# 3 Zeilen Preview im Ascii-Import Dialog

      Rev 1.5   07 Jul 1997 17:18:26   NN
   #41318# Spaltentyp/Preview auch bei Import mit Feldtrennern

      Rev 1.4   26 Mar 1997 13:05:28   NN
   ModifyHdl: Anpassung der benutzten Spalten richtig

      Rev 1.3   29 Oct 1996 14:02:40   NN
   ueberall ScResId statt ResId

      Rev 1.2   16 Sep 1996 20:05:00   NN
   kein Semikolon in Options-String

      Rev 1.1   05 Jun 1996 15:50:32   NN
   ImportAsciiDlg

      Rev 1.0   31 May 1996 19:30:02   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


