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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



#include "scuiimoptdlg.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"
#include <rtl/tencinfo.h>
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

    sal_uInt16  GetCode( const String& rDelimiter ) const;
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

sal_uInt16 ScDelimiterTable::GetCode( const String& rDel ) const
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
        sal_Bool                    bAscii,
        const ScImportOptions*  pOptions,
        const String*           pStrTitle,
        sal_Bool                    bMultiByte,
        sal_Bool                    bOnlyDbtoolsEncodings,
        sal_Bool                    bImport )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_IMPORTOPT ) ),
        aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
        aFtFont     ( this, ScResId( FT_FONT ) ),
        aLbFont     ( this, ScResId( bAscii ? DDLB_FONT : LB_FONT ) ),
        aFtFieldSep ( this, ScResId( FT_FIELDSEP ) ),
        aEdFieldSep ( this, ScResId( ED_FIELDSEP ) ),
        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aEdTextSep  ( this, ScResId( ED_TEXTSEP ) ),
        aCbQuoteAll ( this, ScResId( CB_QUOTEALL ) ),
        aCbShown    ( this, ScResId( CB_SAVESHOWN ) ),
        aCbFixed    ( this, ScResId( CB_FIXEDWIDTH ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) )
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
    {
        // Even dBase export allows multibyte now
        if ( bMultiByte )
            aLbFont.FillFromDbTextEncodingMap( bImport );
        else
            aLbFont.FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_MULTIBYTE );
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
        aCbFixed.Check( sal_False );
        aCbShown.Show();
        aCbShown.Check( sal_True );
        aCbQuoteAll.Show();
        aCbQuoteAll.Check( sal_False );
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
        aCbShown.Hide();
        aCbQuoteAll.Hide();
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
        rOptions.bSaveAsShown = aCbShown.IsChecked();
        rOptions.bQuoteAllText = aCbQuoteAll.IsChecked();
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
{
    ScDelimiterTable* pTab;
    String  aStr( rEd.GetText() );
    sal_uInt16  nCode;

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
            nCode = (sal_uInt16)aStr.GetChar(0);
    }

    return nCode;
}

//------------------------------------------------------------------------

IMPL_LINK( ScImportOptionsDlg, FixedWidthHdl, CheckBox*, pCheckBox )
{
    if( pCheckBox == &aCbFixed )
    {
        sal_Bool bEnable = !aCbFixed.IsChecked();
        aFtFieldSep.Enable( bEnable );
        aEdFieldSep.Enable( bEnable );
        aFtTextSep.Enable( bEnable );
        aEdTextSep.Enable( bEnable );
        aCbShown.Enable( bEnable );
        aCbQuoteAll.Enable( bEnable );
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
