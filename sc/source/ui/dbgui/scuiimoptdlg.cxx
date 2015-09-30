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
#include "sc.hrc"
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>

// ScDelimiterTable

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
    const sal_Int32    nCount;
    sal_Int32          nIter;
};

sal_uInt16 ScDelimiterTable::GetCode( const OUString& rDel ) const
{
    sal_Unicode nCode = 0;

    if ( nCount >= 2 )
    {
        sal_Int32 i = 0;
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

OUString ScDelimiterTable::GetDelimiter( sal_Unicode nCode ) const
{
    OUString aStrDel;

    if ( nCount >= 2 )
    {
        sal_Int32 i = 0;
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

// ScImportOptionsDlg

ScImportOptionsDlg::ScImportOptionsDlg(
        vcl::Window*                 pParent,
        bool                    bAscii,
        const ScImportOptions*  pOptions,
        const OUString*         pStrTitle,
        bool                    bMultiByte,
        bool                    bOnlyDbtoolsEncodings,
        bool                    bImport )
    :   ModalDialog ( pParent, "ImOptDialog",
            "modules/scalc/ui/imoptdialog.ui" )
{
    get(m_pFieldFrame, "fieldframe");
    get(m_pFtCharset, "charsetft");
    if (bAscii)
        get(m_pLbCharset, "charsetdropdown");
    else
    {
        get(m_pLbCharset, "charsetlist");
        m_pLbCharset->set_height_request(6 * m_pLbCharset->GetTextHeight());
    }
    m_pLbCharset->SetStyle(m_pLbCharset->GetStyle() | WB_SORT);
    m_pLbCharset->Show();
    get(m_pFtFieldSep, "fieldft");
    get(m_pEdFieldSep, "field");
    get(m_pFtTextSep, "textft");
    get(m_pEdTextSep, "text");
    get(m_pCbShown, "asshown");
    get(m_pCbFormulas, "formulas");
    get(m_pCbQuoteAll, "quoteall");
    get(m_pCbFixed, "fixedwidth");
    get(m_pBtnOk, "ok");

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
        m_pEdFieldSep->InsertEntry( aStr );
        aStr = pFieldSepTab->NextDel();
    }

    aStr = pTextSepTab->FirstDel();

    while ( !aStr.isEmpty() )
    {
        m_pEdTextSep->InsertEntry( aStr );
        aStr = pTextSepTab->NextDel();
    }

    m_pEdFieldSep->SetText( m_pEdFieldSep->GetEntry(0) );
    m_pEdTextSep->SetText( m_pEdTextSep->GetEntry(0) );

    if ( bOnlyDbtoolsEncodings )
    {
        // Even dBase export allows multibyte now
        if ( bMultiByte )
            m_pLbCharset->FillFromDbTextEncodingMap( bImport );
        else
            m_pLbCharset->FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else if ( !bAscii )
    {   //!TODO: Unicode would need work in each filter
        if ( bMultiByte )
            m_pLbCharset->FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE );
        else
            m_pLbCharset->FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE |
                RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else
    {
        if ( pOptions )
        {
            nCode = pOptions->nFieldSepCode;
            aStr  = pFieldSepTab->GetDelimiter( nCode );

            if ( aStr.isEmpty() )
                m_pEdFieldSep->SetText( OUString((sal_Unicode)nCode) );
            else
                m_pEdFieldSep->SetText( aStr );

            nCode = pOptions->nTextSepCode;
            aStr  = pTextSepTab->GetDelimiter( nCode );

            if ( aStr.isEmpty() )
                m_pEdTextSep->SetText( OUString((sal_Unicode)nCode) );
            else
                m_pEdTextSep->SetText( aStr );
        }
        // all encodings allowed, even Unicode
        m_pLbCharset->FillFromTextEncodingTable( bImport );
    }

    if( bAscii )
    {
        m_pCbFixed->Show();
        m_pCbFixed->SetClickHdl( LINK( this, ScImportOptionsDlg, FixedWidthHdl ) );
        m_pCbFixed->Check( false );
        m_pCbShown->Show();
        m_pCbShown->Check();
        m_pCbQuoteAll->Show();
        m_pCbQuoteAll->Check( false );
        m_pCbFormulas->Show();
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );
        bool bFormulas = pViewSh &&
                pViewSh->GetViewData().GetOptions().GetOption( VOPT_FORMULAS);
        m_pCbFormulas->Check( bFormulas );
    }
    else
    {
        m_pFieldFrame->set_label(m_pFtCharset->GetText());
        m_pFtFieldSep->Hide();
        m_pFtTextSep->Hide();
        m_pFtCharset->Hide();
        m_pEdFieldSep->Hide();
        m_pEdTextSep->Hide();
        m_pCbFixed->Hide();
        m_pCbShown->Hide();
        m_pCbQuoteAll->Hide();
        m_pCbFormulas->Hide();
        m_pLbCharset->GrabFocus();
        m_pLbCharset->SetDoubleClickHdl( LINK( this, ScImportOptionsDlg, DoubleClickHdl ) );
    }

    m_pLbCharset->SelectTextEncoding( pOptions ? pOptions->eCharSet :
        osl_getThreadTextEncoding() );

    // optionaler Titel:
    if ( pStrTitle )
        SetText( *pStrTitle );
}

ScImportOptionsDlg::~ScImportOptionsDlg()
{
    disposeOnce();
}

void ScImportOptionsDlg::dispose()
{
    delete pFieldSepTab;
    delete pTextSepTab;
    m_pFieldFrame.clear();
    m_pFtCharset.clear();
    m_pLbCharset.clear();
    m_pFtFieldSep.clear();
    m_pEdFieldSep.clear();
    m_pFtTextSep.clear();
    m_pEdTextSep.clear();
    m_pCbShown.clear();
    m_pCbFormulas.clear();
    m_pCbQuoteAll.clear();
    m_pCbFixed.clear();
    m_pBtnOk.clear();
    ModalDialog::dispose();
}

void ScImportOptionsDlg::GetImportOptions( ScImportOptions& rOptions ) const
{
    rOptions.SetTextEncoding( m_pLbCharset->GetSelectTextEncoding() );

    if ( m_pCbFixed->IsVisible() )
    {
        rOptions.nFieldSepCode = GetCodeFromCombo( *m_pEdFieldSep );
        rOptions.nTextSepCode  = GetCodeFromCombo( *m_pEdTextSep );
        rOptions.bFixedWidth = m_pCbFixed->IsChecked();
        rOptions.bSaveAsShown = m_pCbShown->IsChecked();
        rOptions.bQuoteAllText = m_pCbQuoteAll->IsChecked();
        rOptions.bSaveFormulas = m_pCbFormulas->IsChecked();
    }
}

sal_uInt16 ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
{
    ScDelimiterTable* pTab;
    OUString  aStr( rEd.GetText() );
    sal_uInt16  nCode;

    if ( &rEd == m_pEdTextSep )
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

IMPL_LINK_TYPED( ScImportOptionsDlg, FixedWidthHdl, Button*, pCheckBox, void )
{
    if (pCheckBox == m_pCbFixed)
    {
        bool bEnable = !m_pCbFixed->IsChecked();
        m_pFtFieldSep->Enable( bEnable );
        m_pEdFieldSep->Enable( bEnable );
        m_pFtTextSep->Enable( bEnable );
        m_pEdTextSep->Enable( bEnable );
        m_pCbShown->Enable( bEnable );
        m_pCbQuoteAll->Enable( bEnable );
    }
}

IMPL_LINK_TYPED( ScImportOptionsDlg, DoubleClickHdl, ListBox&, rLb, void )
{
    if (&rLb == m_pLbCharset)
    {
        m_pBtnOk->Click();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
