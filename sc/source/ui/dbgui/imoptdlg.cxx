/*************************************************************************
 *
 *  $RCSfile: imoptdlg.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:56:50 $
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

#include "imoptdlg.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

static const sal_Char pStrFix[] = "FIX";

//CHINA001 //========================================================================
//CHINA001 // ScDelimiterTable
//CHINA001 //========================================================================
//CHINA001
//CHINA001 class ScDelimiterTable
//CHINA001 {
//CHINA001 public:
//CHINA001 ScDelimiterTable( const String& rDelTab )
//CHINA001 :    theDelTab ( rDelTab ),
//CHINA001 cSep   ( '\t' ),
//CHINA001 nCount     ( rDelTab.GetTokenCount('\t') ),
//CHINA001 nIter      ( 0 )
//CHINA001 {}
//CHINA001
//CHINA001 USHORT   GetCode( const String& rDelimiter ) const;
//CHINA001 String   GetDelimiter( sal_Unicode nCode ) const;
//CHINA001
//CHINA001 String   FirstDel()  { nIter = 0; return theDelTab.GetToken( nIter, cSep ); }
//CHINA001 String   NextDel()   { nIter +=2; return theDelTab.GetToken( nIter, cSep ); }
//CHINA001
//CHINA001 private:
//CHINA001 const String     theDelTab;
//CHINA001 const sal_Unicode    cSep;
//CHINA001 const xub_StrLen nCount;
//CHINA001 xub_StrLen           nIter;
//CHINA001 };
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 USHORT ScDelimiterTable::GetCode( const String& rDel ) const
//CHINA001 {
//CHINA001 sal_Unicode nCode = 0;
//CHINA001 xub_StrLen i = 0;
//CHINA001
//CHINA001 if ( nCount >= 2 )
//CHINA001 {
//CHINA001 while ( i<nCount )
//CHINA001 {
//CHINA001 if ( rDel == theDelTab.GetToken( i, cSep ) )
//CHINA001 {
//CHINA001 nCode = (sal_Unicode) theDelTab.GetToken( i+1, cSep ).ToInt32();
//CHINA001 i     = nCount;
//CHINA001          }
//CHINA001          else
//CHINA001 i += 2;
//CHINA001      }
//CHINA001  }
//CHINA001
//CHINA001 return nCode;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 String ScDelimiterTable::GetDelimiter( sal_Unicode nCode ) const
//CHINA001 {
//CHINA001 String aStrDel;
//CHINA001 xub_StrLen i = 0;
//CHINA001
//CHINA001 if ( nCount >= 2 )
//CHINA001 {
//CHINA001 while ( i<nCount )
//CHINA001 {
//CHINA001 if ( nCode == (sal_Unicode) theDelTab.GetToken( i+1, cSep ).ToInt32() )
//CHINA001 {
//CHINA001 aStrDel = theDelTab.GetToken( i, cSep );
//CHINA001 i       = nCount;
//CHINA001          }
//CHINA001          else
//CHINA001 i += 2;
//CHINA001      }
//CHINA001  }
//CHINA001
//CHINA001 return aStrDel;
//CHINA001 }
//CHINA001
//CHINA001 //========================================================================
//CHINA001 // ScImportOptionsDlg
//CHINA001 //========================================================================
//CHINA001
//CHINA001 ScImportOptionsDlg::ScImportOptionsDlg(
//CHINA001 Window*                 pParent,
//CHINA001 BOOL                    bAscii,
//CHINA001 const ScImportOptions*  pOptions,
//CHINA001 const String*           pStrTitle,
//CHINA001 BOOL                    bMultiByte,
//CHINA001 BOOL                    bOnlyDbtoolsEncodings,
//CHINA001 BOOL                    bImport )
//CHINA001
//CHINA001 :    ModalDialog ( pParent, ScResId( RID_SCDLG_IMPORTOPT ) ),
//CHINA001 aBtnOk       ( this, ScResId( BTN_OK ) ),
//CHINA001 aBtnCancel   ( this, ScResId( BTN_CANCEL ) ),
//CHINA001 aBtnHelp ( this, ScResId( BTN_HELP ) ),
//CHINA001 aFtFieldSep  ( this, ScResId( FT_FIELDSEP ) ),
//CHINA001 aEdFieldSep  ( this, ScResId( ED_FIELDSEP ) ),
//CHINA001 aFtTextSep   ( this, ScResId( FT_TEXTSEP ) ),
//CHINA001 aEdTextSep   ( this, ScResId( ED_TEXTSEP ) ),
//CHINA001 aFtFont      ( this, ScResId( FT_FONT ) ),
//CHINA001 aLbFont     ( this, ScResId( bAscii ? DDLB_FONT : LB_FONT ) ),
//CHINA001 aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
//CHINA001 aCbFixed    ( this, ScResId( CB_FIXEDWIDTH ) )
//CHINA001 {
//CHINA001 // im Ctor-Initializer nicht moeglich (MSC kann das nicht):
//CHINA001 pFieldSepTab = new ScDelimiterTable( String(ScResId(SCSTR_FIELDSEP)) );
//CHINA001 pTextSepTab  = new ScDelimiterTable( String(ScResId(SCSTR_TEXTSEP)) );
//CHINA001
//CHINA001 String aStr = pFieldSepTab->FirstDel();
//CHINA001 sal_Unicode nCode;
//CHINA001
//CHINA001 while ( aStr.Len() > 0 )
//CHINA001  {
//CHINA001 aEdFieldSep.InsertEntry( aStr );
//CHINA001 aStr = pFieldSepTab->NextDel();
//CHINA001  }
//CHINA001
//CHINA001 aStr = pTextSepTab->FirstDel();
//CHINA001
//CHINA001 while ( aStr.Len() > 0 )
//CHINA001  {
//CHINA001 aEdTextSep.InsertEntry( aStr );
//CHINA001 aStr = pTextSepTab->NextDel();
//CHINA001  }
//CHINA001
//CHINA001 aEdFieldSep.SetText( aEdFieldSep.GetEntry(0) );
//CHINA001 aEdTextSep.SetText( aEdTextSep.GetEntry(0) );
//CHINA001
//CHINA001 if ( bOnlyDbtoolsEncodings )
//CHINA001  {   //!TODO: Unicode and MultiByte would need work in each filter
//CHINA001 // Think of field lengths in dBase export
//CHINA001 if ( bMultiByte )
//CHINA001 aLbFont.FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_UNICODE );
//CHINA001      else
//CHINA001 aLbFont.FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_UNICODE |
//CHINA001 RTL_TEXTENCODING_INFO_MULTIBYTE );
//CHINA001  }
//CHINA001  else if ( !bAscii )
//CHINA001  {   //!TODO: Unicode would need work in each filter
//CHINA001 if ( bMultiByte )
//CHINA001 aLbFont.FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE );
//CHINA001      else
//CHINA001 aLbFont.FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE |
//CHINA001 RTL_TEXTENCODING_INFO_MULTIBYTE );
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001 if ( pOptions )
//CHINA001      {
//CHINA001 nCode = pOptions->nFieldSepCode;
//CHINA001 aStr  = pFieldSepTab->GetDelimiter( nCode );
//CHINA001
//CHINA001 if ( !aStr.Len() )
//CHINA001 aEdFieldSep.SetText( String((sal_Unicode)nCode) );
//CHINA001          else
//CHINA001 aEdFieldSep.SetText( aStr );
//CHINA001
//CHINA001 nCode = pOptions->nTextSepCode;
//CHINA001 aStr  = pTextSepTab->GetDelimiter( nCode );
//CHINA001
//CHINA001 if ( !aStr.Len() )
//CHINA001 aEdTextSep.SetText( String((sal_Unicode)nCode) );
//CHINA001          else
//CHINA001 aEdTextSep.SetText( aStr );
//CHINA001      }
//CHINA001 // all encodings allowed, even Unicode
//CHINA001 aLbFont.FillFromTextEncodingTable( bImport );
//CHINA001  }
//CHINA001
//CHINA001 if( bAscii )
//CHINA001  {
//CHINA001 Size aWinSize( GetSizePixel() );
//CHINA001 aWinSize.Height() = aCbFixed.GetPosPixel().Y() + aCbFixed.GetSizePixel().Height();
//CHINA001 Size aDiffSize( LogicToPixel( Size( 0, 6 ), MapMode( MAP_APPFONT ) ) );
//CHINA001 aWinSize.Height() += aDiffSize.Height();
//CHINA001 SetSizePixel( aWinSize );
//CHINA001 aCbFixed.Show();
//CHINA001 aCbFixed.SetClickHdl( LINK( this, ScImportOptionsDlg, FixedWidthHdl ) );
//CHINA001 aCbFixed.Check( FALSE );
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001 aFlFieldOpt.SetText( aFtFont.GetText() );
//CHINA001 aFtFieldSep.Hide();
//CHINA001 aFtTextSep.Hide();
//CHINA001 aFtFont.Hide();
//CHINA001 aEdFieldSep.Hide();
//CHINA001 aEdTextSep.Hide();
//CHINA001 aCbFixed.Hide();
//CHINA001 aLbFont.GrabFocus();
//CHINA001 aLbFont.SetDoubleClickHdl( LINK( this, ScImportOptionsDlg, DoubleClickHdl ) );
//CHINA001  }
//CHINA001
//CHINA001 aLbFont.SelectTextEncoding( pOptions ? pOptions->eCharSet :
//CHINA001 gsl_getSystemTextEncoding() );
//CHINA001
//CHINA001 // optionaler Titel:
//CHINA001 if ( pStrTitle )
//CHINA001 SetText( *pStrTitle );
//CHINA001
//CHINA001 FreeResource();
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 __EXPORT ScImportOptionsDlg::~ScImportOptionsDlg()
//CHINA001 {
//CHINA001 delete pFieldSepTab;
//CHINA001 delete pTextSepTab;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 void ScImportOptionsDlg::GetImportOptions( ScImportOptions& rOptions ) const
//CHINA001 {
//CHINA001 rOptions.SetTextEncoding( aLbFont.GetSelectTextEncoding() );
//CHINA001
//CHINA001 if ( aCbFixed.IsVisible() )
//CHINA001  {
//CHINA001 rOptions.nFieldSepCode = GetCodeFromCombo( aEdFieldSep );
//CHINA001 rOptions.nTextSepCode  = GetCodeFromCombo( aEdTextSep );
//CHINA001 rOptions.bFixedWidth = aCbFixed.IsChecked();
//CHINA001  }
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 USHORT ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
//CHINA001 {
//CHINA001 ScDelimiterTable* pTab;
//CHINA001 String  aStr( rEd.GetText() );
//CHINA001 USHORT  nCode;
//CHINA001
//CHINA001 if ( &rEd == &aEdTextSep )
//CHINA001 pTab = pTextSepTab;
//CHINA001  else
//CHINA001 pTab = pFieldSepTab;
//CHINA001
//CHINA001 if ( !aStr.Len() )
//CHINA001  {
//CHINA001 nCode = 0;           // kein Trennzeichen
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001 nCode = pTab->GetCode( aStr );
//CHINA001
//CHINA001 if ( nCode == 0 )
//CHINA001 nCode = (USHORT)aStr.GetChar(0);
//CHINA001  }
//CHINA001
//CHINA001 return nCode;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScImportOptionsDlg, FixedWidthHdl, CheckBox*, pCheckBox )
//CHINA001 {
//CHINA001 if( pCheckBox == &aCbFixed )
//CHINA001  {
//CHINA001 BOOL bEnable = !aCbFixed.IsChecked();
//CHINA001 aFtFieldSep.Enable( bEnable );
//CHINA001 aEdFieldSep.Enable( bEnable );
//CHINA001 aFtTextSep.Enable( bEnable );
//CHINA001 aEdTextSep.Enable( bEnable );
//CHINA001  }
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScImportOptionsDlg, DoubleClickHdl, ListBox*, pLb )
//CHINA001 {
//CHINA001 if ( pLb == &aLbFont )
//CHINA001  {
//CHINA001 aBtnOk.Click();
//CHINA001  }
//CHINA001 return 0;
//CHINA001 }

//------------------------------------------------------------------------
//  Der Options-String darf kein Semikolon mehr enthalten (wegen Pickliste)
//  darum ab Version 336 Komma stattdessen


ScImportOptions::ScImportOptions( const String& rStr )
{
    bFixedWidth = FALSE;
    nFieldSepCode = 0;
    if ( rStr.GetTokenCount(',') >= 3 )
    {
        String aToken( rStr.GetToken( 0, ',' ) );
        if( aToken.EqualsIgnoreCaseAscii( pStrFix ) )
            bFixedWidth = TRUE;
        else
            nFieldSepCode = (sal_Unicode) aToken.ToInt32();
        nTextSepCode  = (sal_Unicode) rStr.GetToken(1,',').ToInt32();
        aStrFont      = rStr.GetToken(2,',');
        eCharSet      = ScGlobal::GetCharsetValue(aStrFont);
    }
}

//------------------------------------------------------------------------

String ScImportOptions::BuildString() const
{
    String  aResult;

    if( bFixedWidth )
        aResult.AppendAscii( pStrFix );
    else
        aResult += String::CreateFromInt32(nFieldSepCode);
    aResult += ',';
    aResult += String::CreateFromInt32(nTextSepCode);
    aResult += ',';
    aResult += aStrFont;

    return aResult;
}

//------------------------------------------------------------------------

void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
{
    eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
        gsl_getSystemTextEncoding() : nEnc);
    aStrFont = ScGlobal::GetCharsetString( nEnc );
}
