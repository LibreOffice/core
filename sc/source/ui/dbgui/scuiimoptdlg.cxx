/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#undef SC_DLLIMPLEMENTATION



#include "scuiimoptdlg.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>

//========================================================================
// ScDelimiterTable
//========================================================================

class ScDelimiterTable
{
public:
        ScDelimiterTable( const OUString& rDelTab )
            :   theDelTab ( rDelTab ),
                cSep      ( '\t' ),
                nCount    ( comphelper::string::getTokenCount(rDelTab, '\t') ),
                nIter     ( 0 )
            {}

    sal_uInt16  GetCode( const OUString& rDelimiter ) const;
    OUString  GetDelimiter( sal_Unicode nCode ) const;

    OUString  FirstDel()  { nIter = 0; return theDelTab.getToken( nIter, cSep ); }
    OUString  NextDel()   { nIter +=2; return theDelTab.getToken( nIter, cSep ); }

private:
    const OUString      theDelTab;
    const sal_Unicode   cSep;
    const xub_StrLen    nCount;
    xub_StrLen          nIter;
};

//------------------------------------------------------------------------

sal_uInt16 ScDelimiterTable::GetCode( const OUString& rDel ) const
{
    sal_Unicode nCode = 0;

    if ( nCount >= 2 )
    {
        xub_StrLen i = 0;
        while ( i<nCount )
        {
            if ( rDel == theDelTab.getToken( i, cSep ) )
            {
                nCode = (sal_Unicode) theDelTab.getToken( i+1, cSep ).toInt32();
                i     = nCount;
            }
            else
                i += 2;
        }
    }

    return nCode;
}

//------------------------------------------------------------------------

OUString ScDelimiterTable::GetDelimiter( sal_Unicode nCode ) const
{
    OUString aStrDel;

    if ( nCount >= 2 )
    {
        xub_StrLen i = 0;
        while ( i<nCount )
        {
            if ( nCode == (sal_Unicode) theDelTab.getToken( i+1, cSep ).toInt32() )
            {
                aStrDel = theDelTab.getToken( i, cSep );
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
        const OUString*         pStrTitle,
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
        aCbShown    ( this, ScResId( CB_SAVESHOWN ) ),
        aCbFormulas ( this, ScResId( CB_FORMULAS ) ),
        aCbQuoteAll ( this, ScResId( CB_QUOTEALL ) ),
        aCbFixed    ( this, ScResId( CB_FIXEDWIDTH ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) )
{
    OUString sFieldSep(SC_RESSTR(SCSTR_FIELDSEP));
    sFieldSep = sFieldSep.replaceFirst( "%TAB",   SC_RESSTR(SCSTR_FIELDSEP_TAB) );
    sFieldSep = sFieldSep.replaceFirst( "%SPACE", SC_RESSTR(SCSTR_FIELDSEP_SPACE) );

    // im Ctor-Initializer nicht moeglich (MSC kann das nicht):
    pFieldSepTab = new ScDelimiterTable( sFieldSep );
    pTextSepTab  = new ScDelimiterTable( OUString(ScResId(SCSTR_TEXTSEP)) );

    OUString aStr = pFieldSepTab->FirstDel();
    sal_Unicode nCode;

    while ( !aStr.isEmpty() )
    {
        aEdFieldSep.InsertEntry( aStr );
        aStr = pFieldSepTab->NextDel();
    }

    aStr = pTextSepTab->FirstDel();

    while ( !aStr.isEmpty() )
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

            if ( aStr.isEmpty() )
                aEdFieldSep.SetText( OUString((sal_Unicode)nCode) );
            else
                aEdFieldSep.SetText( aStr );

            nCode = pOptions->nTextSepCode;
            aStr  = pTextSepTab->GetDelimiter( nCode );

            if ( aStr.isEmpty() )
                aEdTextSep.SetText( OUString((sal_Unicode)nCode) );
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
        aCbFixed.Check( false );
        aCbShown.Show();
        aCbShown.Check( sal_True );
        aCbQuoteAll.Show();
        aCbQuoteAll.Check( false );
        aCbFormulas.Show();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current());
        bool bFormulas = (pViewSh ?
                pViewSh->GetViewData()->GetOptions().GetOption( VOPT_FORMULAS) :
                false);
        aCbFormulas.Check( bFormulas );
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
        aCbFormulas.Hide();
        aLbFont.GrabFocus();
        aLbFont.SetDoubleClickHdl( LINK( this, ScImportOptionsDlg, DoubleClickHdl ) );
    }

    aLbFont.SelectTextEncoding( pOptions ? pOptions->eCharSet :
        osl_getThreadTextEncoding() );

    // optionaler Titel:
    if ( pStrTitle )
        SetText( *pStrTitle );

    FreeResource();
}

//------------------------------------------------------------------------

ScImportOptionsDlg::~ScImportOptionsDlg()
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
        rOptions.bSaveFormulas = aCbFormulas.IsChecked();
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
{
    ScDelimiterTable* pTab;
    OUString  aStr( rEd.GetText() );
    sal_uInt16  nCode;

    if ( &rEd == &aEdTextSep )
        pTab = pTextSepTab;
    else
        pTab = pFieldSepTab;

    if ( aStr.isEmpty() )
    {
        nCode = 0;          // kein Trennzeichen
    }
    else
    {
        nCode = pTab->GetCode( aStr );

        if ( nCode == 0 )
            nCode = (sal_uInt16)aStr[0];
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
