/*************************************************************************
 *
 *  $RCSfile: scuiasciiopt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 18:00:45 $
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
#include "scuiasciiopt.hxx"
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

//! TODO make dynamic
#ifdef WIN
const SCSIZE ASCIIDLG_MAXROWS                = 10000;
#else
const SCSIZE ASCIIDLG_MAXROWS                = MAXROWCOUNT;
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
            if ( GetScGlobalpTransliteration()->isEqual( aStr, rList.GetToken(i,'\t') ) )//CHINA001 if ( ScGlobal::pTransliteration->isEqual( aStr, rList.GetToken(i,'\t') ) )
                c = (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32();
        }
        if (!c && aStr.Len())
        {
            sal_Unicode cFirst = aStr.GetChar( 0 );
            // #i24235# first try the first character of the string directly
            if( (aStr.Len() == 1) || (cFirst < '0') || (cFirst > '9') )
                c = cFirst;
            else    // keep old behaviour for compatibility (i.e. "39" -> "'")
                c = (sal_Unicode) aStr.ToInt32();       // Ascii
        }
    }
    return c;
}


// ----------------------------------------------------------------------------

ScImportAsciiDlg::ScImportAsciiDlg( Window* pParent,String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep ) :
        ModalDialog ( pParent, ScResId( RID_SCDLG_ASCII ) ),
        pDatStream  ( pInStream ),

        pRowPosArray( NULL ),
        pRowPosArrayUnicode( NULL ),
        bVFlag      ( FALSE ),

        aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
        aFtCharSet  ( this, ScResId( FT_CHARSET ) ),
        aLbCharSet  ( this, ScResId( LB_CHARSET ) ),

        aFtRow      ( this, ScResId( FT_AT_ROW  ) ),
        aNfRow      ( this, ScResId( NF_AT_ROW  ) ),

        aFlSepOpt   ( this, ScResId( FL_SEPOPT ) ),
        aRbFixed    ( this, ScResId( RB_FIXED ) ),
        aRbSeparated( this, ScResId( RB_SEPARATED ) ),

        aCkbTab     ( this, ScResId( CKB_TAB ) ),
        aCkbSemicolon(this, ScResId( CKB_SEMICOLON ) ),
        aCkbComma   ( this, ScResId( CKB_COMMA  ) ),
        aCkbSpace   ( this, ScResId( CKB_SPACE   ) ),
        aCkbOther   ( this, ScResId( CKB_OTHER ) ),
        aEdOther    ( this, ScResId( ED_OTHER ) ),
        aCkbAsOnce  ( this, ScResId( CB_ASONCE) ),
        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aCbTextSep  ( this, ScResId( CB_TEXTSEP ) ),

        aFlWidth    ( this, ScResId( FL_WIDTH ) ),
        aFtType     ( this, ScResId( FT_TYPE ) ),
        aLbType     ( this, ScResId( LB_TYPE1 ) ),

        maTableBox  ( this, ScResId( CTR_TABLEBOX ) ),

        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),

        aCharSetUser( ScResId( SCSTR_CHARSET_USER ) ),
        aColumnUser ( ScResId( SCSTR_COLUMN_USER ) ),
        aFldSepList ( ScResId( SCSTR_FIELDSEP ) ),
        aTextSepList( ScResId( SCSTR_TEXTSEP ) )
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
    BOOL bPreselectUnicode = FALSE;
    if( pDatStream )
    {
        USHORT j;
        pRowPosArray=new ULONG[ASCIIDLG_MAXROWS+2];
        pRowPosArrayUnicode=new ULONG[ASCIIDLG_MAXROWS+2];
        memset( pRowPosArray, 0, sizeof(ULONG) * (ASCIIDLG_MAXROWS+2));
        memset( pRowPosArrayUnicode, 0, sizeof(ULONG) * (ASCIIDLG_MAXROWS+2));
        pDatStream->SetBufferSize(0x7fff);
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
    lcl_FillCombo( aCbTextSep, aTextSepList, '"' );      // Default "

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
        maTableBox.Execute( CSVCMD_SETLINECOUNT, static_cast<sal_Int32>(ASCIIDLG_MAXROWS) );

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
    rOpt.SetStartRow( (long)aNfRow.GetValue() );
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
    aEdOther.Enable( bEnable );
    aCkbAsOnce.Enable( bEnable );
    aFtTextSep.Enable( bEnable );
    aCbTextSep.Enable( bEnable );
}

void ScImportAsciiDlg::UpdateVertical( bool bSwitchToFromUnicode )
{
    if ( bSwitchToFromUnicode )
    {
        bVFlag = FALSE;
        maTableBox.Execute( CSVCMD_SETLINECOUNT, static_cast<sal_Int32>(ASCIIDLG_MAXROWS) );
    }
    ULONG nNew = 0;
    if(!bVFlag)
    {
        // dragging the scrollbar -> read entire file
        bVFlag=TRUE;
        SCSIZE nRows = 0;

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

        maTableBox.Execute( CSVCMD_SETLINECOUNT, static_cast<sal_Int32>(nRows) );
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

    if( (pCtrl == &aCkbOther) && aCkbOther.IsChecked() )
        aEdOther.GrabFocus();
    else if( pCtrl == &aEdOther )
        aCkbOther.Check( aEdOther.GetText().Len() > 0 );
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
