/*************************************************************************
 *
 *  $RCSfile: imoptdlg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:02:54 $
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

//========================================================================
// ScDelimiterTable
//========================================================================

class ScDelimiterTable
{
public:
        ScDelimiterTable( const String& rDelTab )
            :   theDelTab ( rDelTab ),
                cSep      ( '\t' ),
                nCount    ( rDelTab.GetTokenCount('\t') ),
                nIter     ( 0 )
            {}

    USHORT  GetCode( const String& rDelimiter ) const;
    String  GetDelimiter( sal_Unicode nCode ) const;

    String  FirstDel()  { nIter = 0; return theDelTab.GetToken( nIter, cSep ); }
    String  NextDel()   { nIter +=2; return theDelTab.GetToken( nIter, cSep ); }

private:
    const String        theDelTab;
    const sal_Unicode   cSep;
    const xub_StrLen    nCount;
    xub_StrLen          nIter;
};

//------------------------------------------------------------------------

USHORT ScDelimiterTable::GetCode( const String& rDel ) const
{
    sal_Unicode nCode = 0;
    xub_StrLen i = 0;

    if ( nCount >= 2 )
    {
        while ( i<nCount )
        {
            if ( rDel == theDelTab.GetToken( i, cSep ) )
            {
                nCode = (sal_Unicode) theDelTab.GetToken( i+1, cSep ).ToInt32();
                i     = nCount;
            }
            else
                i += 2;
        }
    }

    return nCode;
}

//------------------------------------------------------------------------

String ScDelimiterTable::GetDelimiter( sal_Unicode nCode ) const
{
    String aStrDel;
    xub_StrLen i = 0;

    if ( nCount >= 2 )
    {
        while ( i<nCount )
        {
            if ( nCode == (sal_Unicode) theDelTab.GetToken( i+1, cSep ).ToInt32() )
            {
                aStrDel = theDelTab.GetToken( i, cSep );
                i       = nCount;
            }
            else
                i += 2;
        }
    }

    return aStrDel;
}

//========================================================================
// ScImportOptionsDlg
//========================================================================

ScImportOptionsDlg::ScImportOptionsDlg(
        Window*                 pParent,
        BOOL                    bAscii,
        const ScImportOptions*  pOptions,
        const String*           pStrTitle,
        BOOL                    bMultiByte,
        BOOL                    bOnlyDbtoolsEncodings,
        BOOL                    bImport )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_IMPORTOPT ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),
        aFtFieldSep ( this, ScResId( FT_FIELDSEP ) ),
        aEdFieldSep ( this, ScResId( ED_FIELDSEP ) ),
        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aEdTextSep  ( this, ScResId( ED_TEXTSEP ) ),
        aFtFont     ( this, ScResId( FT_FONT ) ),
        aLbFont     ( this, ScResId( bAscii ? DDLB_FONT : LB_FONT ) ),
        aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
        aCbFixed    ( this, ScResId( CB_FIXEDWIDTH ) )
{
    // im Ctor-Initializer nicht moeglich (MSC kann das nicht):
    pFieldSepTab = new ScDelimiterTable( String(ScResId(SCSTR_FIELDSEP)) );
    pTextSepTab  = new ScDelimiterTable( String(ScResId(SCSTR_TEXTSEP)) );

    String aStr = pFieldSepTab->FirstDel();
    sal_Unicode nCode;

    while ( aStr.Len() > 0 )
    {
        aEdFieldSep.InsertEntry( aStr );
        aStr = pFieldSepTab->NextDel();
    }

    aStr = pTextSepTab->FirstDel();

    while ( aStr.Len() > 0 )
    {
        aEdTextSep.InsertEntry( aStr );
        aStr = pTextSepTab->NextDel();
    }

    aEdFieldSep.SetText( aEdFieldSep.GetEntry(0) );
    aEdTextSep.SetText( aEdTextSep.GetEntry(0) );

    if ( bOnlyDbtoolsEncodings )
    {   //!TODO: Unicode and MultiByte would need work in each filter
        // Think of field lengths in dBase export
        if ( bMultiByte )
            aLbFont.FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_UNICODE );
        else
            aLbFont.FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_UNICODE |
                RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else if ( !bAscii )
    {   //!TODO: Unicode would need work in each filter
        if ( bMultiByte )
            aLbFont.FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE );
        else
            aLbFont.FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE |
                RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else
    {
        if ( pOptions )
        {
            nCode = pOptions->nFieldSepCode;
            aStr  = pFieldSepTab->GetDelimiter( nCode );

            if ( !aStr.Len() )
                aEdFieldSep.SetText( String((sal_Unicode)nCode) );
            else
                aEdFieldSep.SetText( aStr );

            nCode = pOptions->nTextSepCode;
            aStr  = pTextSepTab->GetDelimiter( nCode );

            if ( !aStr.Len() )
                aEdTextSep.SetText( String((sal_Unicode)nCode) );
            else
                aEdTextSep.SetText( aStr );
        }
        // all encodings allowed, even Unicode
        aLbFont.FillFromTextEncodingTable( bImport );
    }

    if( bAscii )
    {
        Size aWinSize( GetSizePixel() );
        aWinSize.Height() = aCbFixed.GetPosPixel().Y() + aCbFixed.GetSizePixel().Height();
        Size aDiffSize( LogicToPixel( Size( 0, 6 ), MapMode( MAP_APPFONT ) ) );
        aWinSize.Height() += aDiffSize.Height();
        SetSizePixel( aWinSize );
        aCbFixed.Show();
        aCbFixed.SetClickHdl( LINK( this, ScImportOptionsDlg, FixedWidthHdl ) );
        aCbFixed.Check( FALSE );
    }
    else
    {
        aFlFieldOpt.SetText( aFtFont.GetText() );
        aFtFieldSep.Hide();
        aFtTextSep.Hide();
        aFtFont.Hide();
        aEdFieldSep.Hide();
        aEdTextSep.Hide();
        aCbFixed.Hide();
        aLbFont.GrabFocus();
        aLbFont.SetDoubleClickHdl( LINK( this, ScImportOptionsDlg, DoubleClickHdl ) );
    }

    aLbFont.SelectTextEncoding( pOptions ? pOptions->eCharSet :
        gsl_getSystemTextEncoding() );

    // optionaler Titel:
    if ( pStrTitle )
        SetText( *pStrTitle );

    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScImportOptionsDlg::~ScImportOptionsDlg()
{
    delete pFieldSepTab;
    delete pTextSepTab;
}

//------------------------------------------------------------------------

void ScImportOptionsDlg::GetImportOptions( ScImportOptions& rOptions ) const
{
    rOptions.SetTextEncoding( aLbFont.GetSelectTextEncoding() );

    if ( aCbFixed.IsVisible() )
    {
        rOptions.nFieldSepCode = GetCodeFromCombo( aEdFieldSep );
        rOptions.nTextSepCode  = GetCodeFromCombo( aEdTextSep );
        rOptions.bFixedWidth = aCbFixed.IsChecked();
    }
}

//------------------------------------------------------------------------

USHORT ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
{
    ScDelimiterTable* pTab;
    String  aStr( rEd.GetText() );
    USHORT  nCode;

    if ( &rEd == &aEdTextSep )
        pTab = pTextSepTab;
    else
        pTab = pFieldSepTab;

    if ( !aStr.Len() )
    {
        nCode = 0;          // kein Trennzeichen
    }
    else
    {
        nCode = pTab->GetCode( aStr );

        if ( nCode == 0 )
            nCode = (USHORT)aStr.GetChar(0);
    }

    return nCode;
}

//------------------------------------------------------------------------

IMPL_LINK( ScImportOptionsDlg, FixedWidthHdl, CheckBox*, pCheckBox )
{
    if( pCheckBox == &aCbFixed )
    {
        BOOL bEnable = !aCbFixed.IsChecked();
        aFtFieldSep.Enable( bEnable );
        aEdFieldSep.Enable( bEnable );
        aFtTextSep.Enable( bEnable );
        aEdTextSep.Enable( bEnable );
    }
    return 0;
}

 IMPL_LINK( ScImportOptionsDlg, DoubleClickHdl, ListBox*, pLb )
{
    if ( pLb == &aLbFont )
    {
        aBtnOk.Click();
    }
    return 0;
}

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
