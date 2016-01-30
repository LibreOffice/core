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

#include "prtsetup.hxx"
#include "svdata.hxx"
#include "svids.hrc"

#include "osl/thread.h"

#include <officecfg/Office/Common.hxx>

using namespace psp;

void RTSDialog::insertAllPPDValues( ListBox& rBox, const PPDParser* pParser, const PPDKey* pKey )
{
    if( ! pKey || ! pParser )
        return;

    const PPDValue* pValue = nullptr;
    sal_Int32 nPos = 0;
    OUString aOptionText;

    for( int i = 0; i < pKey->countValues(); i++ )
    {
        pValue = pKey->getValue( i );
        if (pValue->m_bCustomOption)
            continue;
        aOptionText = pParser->translateOption( pKey->getKey(), pValue->m_aOption) ;

        if( m_aJobData.m_aContext.checkConstraints( pKey, pValue ) )
        {
            if( rBox.GetEntryPos( static_cast<void const *>(pValue) ) == LISTBOX_ENTRY_NOTFOUND )
            {
                nPos = rBox.InsertEntry( aOptionText );
                    rBox.SetEntryData( nPos, const_cast<PPDValue *>(pValue) );
            }
        }
        else
        {
            if( ( nPos = rBox.GetEntryPos( static_cast<void const *>(pValue) ) ) != LISTBOX_ENTRY_NOTFOUND )
                rBox.RemoveEntry( nPos );
        }
    }
    pValue = m_aJobData.m_aContext.getValue( pKey );
    if (pValue && !pValue->m_bCustomOption)
    {
        if( ( nPos = rBox.GetEntryPos( static_cast<void const *>(pValue) ) ) != LISTBOX_ENTRY_NOTFOUND )
            rBox.SelectEntryPos( nPos );
    }
    else
        rBox.SelectEntry( m_aInvalidString );
}

/*
 * RTSDialog
 */

RTSDialog::RTSDialog(const PrinterInfo& rJobData, vcl::Window* pParent)
    : TabDialog(pParent, "PrinterPropertiesDialog", "vcl/ui/printerpropertiesdialog.ui")
    , m_aJobData(rJobData)
    , m_pPaperPage(nullptr)
    , m_pDevicePage(nullptr)
    , m_aInvalidString(VclResId(SV_PRINT_INVALID_TXT))
    , mbDataModified(false)
{
    get(m_pOKButton, "ok");
    get(m_pCancelButton, "cancel");
    get(m_pTabControl, "notebook");

    OUString aTitle(GetText());
    SetText(aTitle.replaceAll("%s", m_aJobData.m_aPrinterName));

    m_pTabControl->SetActivatePageHdl( LINK( this, RTSDialog, ActivatePage ) );
    m_pOKButton->SetClickHdl( LINK( this, RTSDialog, ClickButton ) );
    m_pCancelButton->SetClickHdl( LINK( this, RTSDialog, ClickButton ) );
    ActivatePage(m_pTabControl);
}

RTSDialog::~RTSDialog()
{
    disposeOnce();
}

void RTSDialog::dispose()
{
    m_pTabControl.clear();
    m_pOKButton.clear();
    m_pCancelButton.clear();
    m_pPaperPage.disposeAndClear();
    m_pDevicePage.disposeAndClear();
    TabDialog::dispose();
}

IMPL_LINK_TYPED( RTSDialog, ActivatePage, TabControl*, pTabCtrl, void )
{
    if( pTabCtrl != m_pTabControl )
        return;

    sal_uInt16 nId = m_pTabControl->GetCurPageId();
    OString sPage = m_pTabControl->GetPageName(nId);
    if ( ! m_pTabControl->GetTabPage( nId ) )
    {
        TabPage *pPage = nullptr;
        if (sPage == "paper")
            pPage = m_pPaperPage = VclPtr<RTSPaperPage>::Create( this );
        else if (sPage == "device")
            pPage = m_pDevicePage = VclPtr<RTSDevicePage>::Create( this );
        if( pPage )
            m_pTabControl->SetTabPage( nId, pPage );
    }
    else
    {
        if (sPage == "paper")
            m_pPaperPage->update();
    }
}

IMPL_LINK_TYPED( RTSDialog, ClickButton, Button*, pButton, void )
{
    if( pButton == m_pOKButton )
    {
        // refresh the changed values
        if( m_pPaperPage )
        {
            // orientation
            m_aJobData.m_eOrientation = m_pPaperPage->getOrientation() == 0 ?
                orientation::Portrait : orientation::Landscape;
        }
        if( m_pDevicePage )
        {
            m_aJobData.m_nColorDepth    = m_pDevicePage->getDepth();
            m_aJobData.m_nColorDevice   = m_pDevicePage->getColorDevice();
            m_aJobData.m_nPSLevel       = m_pDevicePage->getLevel();
            m_aJobData.m_nPDFDevice     = m_pDevicePage->getPDFDevice();
        }
        EndDialog( 1 );
    }
    else if( pButton == m_pCancelButton )
        EndDialog();
}

/*
 * RTSPaperPage
 */

RTSPaperPage::RTSPaperPage(RTSDialog* pParent)
    : TabPage(pParent->m_pTabControl, "PrinterPaperPage", "vcl/ui/printerpaperpage.ui")
    , m_pParent( pParent )
{
    get(m_pPaperText, "paperft");
    get(m_pPaperBox, "paperlb");
    get(m_pOrientBox, "orientlb");
    get(m_pDuplexText, "duplexft");
    get(m_pDuplexBox, "duplexlb");
    get(m_pSlotText, "slotft");
    get(m_pSlotBox, "slotlb");

    m_pPaperBox->SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_pOrientBox->SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_pDuplexBox->SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_pSlotBox->SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );

    sal_Int32 nPos = 0;

    // duplex
    nPos = m_pDuplexBox->InsertEntry( m_pParent->m_aInvalidString );
    m_pDuplexBox->SetEntryData( nPos, nullptr );

    // paper does not have an invalid entry

    // input slots
    nPos = m_pSlotBox->InsertEntry( m_pParent->m_aInvalidString );
    m_pSlotBox->SetEntryData( nPos, nullptr );

    update();
}

RTSPaperPage::~RTSPaperPage()
{
    disposeOnce();
}

void RTSPaperPage::dispose()
{
    m_pParent.clear();
    m_pPaperText.clear();
    m_pPaperBox.clear();
    m_pOrientBox.clear();
    m_pDuplexText.clear();
    m_pDuplexBox.clear();
    m_pSlotText.clear();
    m_pSlotBox.clear();
    TabPage::dispose();
}

void RTSPaperPage::update()
{
    const PPDKey* pKey      = nullptr;

    // orientation
    m_pOrientBox->SelectEntryPos(
        m_pParent->m_aJobData.m_eOrientation == orientation::Portrait ? 0 : 1);

    // duplex
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "Duplex" ) )) )
    {
        m_pParent->insertAllPPDValues( *m_pDuplexBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_pDuplexText->Enable( false );
        m_pDuplexBox->Enable( false );
    }

    // paper
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "PageSize" ) )) )
    {
        m_pParent->insertAllPPDValues( *m_pPaperBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_pPaperText->Enable( false );
        m_pPaperBox->Enable( false );
    }

    // input slots
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString("InputSlot") )) )
    {
        m_pParent->insertAllPPDValues( *m_pSlotBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_pSlotText->Enable( false );
        m_pSlotBox->Enable( false );
    }

    // disable those, unless user wants to use papersize from printer prefs
    // as they have no influence on what's going to be printed anyway
    if (!m_pParent->m_aJobData.m_bPapersizeFromSetup)
    {
        m_pPaperBox->Enable( false );
        m_pOrientBox->Enable( false );
    }
}

IMPL_LINK_TYPED( RTSPaperPage, SelectHdl, ListBox&, rBox, void )
{
    const PPDKey* pKey = nullptr;
    if( &rBox == m_pPaperBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "PageSize" ) );
    }
    else if( &rBox == m_pDuplexBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "Duplex" ) );
    }
    else if( &rBox == m_pSlotBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "InputSlot" ) );
    }
    else if( &rBox == m_pOrientBox )
    {
        m_pParent->m_aJobData.m_eOrientation = m_pOrientBox->GetSelectEntryPos() == 0 ? orientation::Portrait : orientation::Landscape;
    }
    if( pKey )
    {
        PPDValue* pValue = static_cast<PPDValue*>(rBox.GetSelectEntryData());
        m_pParent->m_aJobData.m_aContext.setValue( pKey, pValue );
        update();
    }

    m_pParent->SetDataModified( true );
}

/*
 * RTSDevicePage
 */

RTSDevicePage::RTSDevicePage( RTSDialog* pParent )
    : TabPage(pParent->m_pTabControl, "PrinterDevicePage", "vcl/ui/printerdevicepage.ui")
    , m_pParent(pParent)
    , m_pCustomValue(nullptr)
{
    get(m_pPPDKeyBox, "options");
    get(m_pPPDValueBox, "values");

    m_pPPDKeyBox->SetDropDownLineCount(12);
    m_pPPDValueBox->SetDropDownLineCount(12);

    get(m_pCustomEdit, "custom");
    m_pCustomEdit->SetModifyHdl(LINK(this, RTSDevicePage, ModifyHdl));

    get(m_pLevelBox, "level");
    get(m_pSpaceBox, "colorspace");
    get(m_pDepthBox, "colordepth");

    m_pPPDKeyBox->SetSelectHdl( LINK( this, RTSDevicePage, SelectHdl ) );
    m_pPPDValueBox->SetSelectHdl( LINK( this, RTSDevicePage, SelectHdl ) );

    switch( m_pParent->m_aJobData.m_nColorDevice )
    {
        case  0: m_pSpaceBox->SelectEntryPos(0);break;
        case  1: m_pSpaceBox->SelectEntryPos(1);break;
        case -1: m_pSpaceBox->SelectEntryPos(2);break;
    }

    sal_uLong nLevelEntryData = 0; //automatic
    if( m_pParent->m_aJobData.m_nPDFDevice == 2 ) //explicit PDF
        nLevelEntryData = 10;
    else if (m_pParent->m_aJobData.m_nPSLevel > 0) //explicit PS Level
        nLevelEntryData = m_pParent->m_aJobData.m_nPSLevel+1;
    else if (m_pParent->m_aJobData.m_nPDFDevice == 1) //automatically PDF
        nLevelEntryData = 0;
    else if (m_pParent->m_aJobData.m_nPDFDevice == -1) //explicitly PS from driver
        nLevelEntryData = 1;

    bool bAutoIsPDF = officecfg::Office::Common::Print::Option::Printer::PDFAsStandardPrintJobFormat::get();

    assert(nLevelEntryData != 0
            || "Generic Printer" == m_pParent->m_aJobData.m_aPrinterName
            || int(bAutoIsPDF) == m_pParent->m_aJobData.m_nPDFDevice);

    OUString sStr = m_pLevelBox->GetEntry(0);
    m_pLevelBox->InsertEntry(sStr.replaceAll("%s", bAutoIsPDF ? m_pLevelBox->GetEntry(5) : m_pLevelBox->GetEntry(1)), 0);
    m_pLevelBox->SetEntryData(0, m_pLevelBox->GetEntryData(1));
    m_pLevelBox->RemoveEntry(1);

    for( sal_Int32 i = 0; i < m_pLevelBox->GetEntryCount(); i++ )
    {
        if( reinterpret_cast<sal_uLong>(m_pLevelBox->GetEntryData( i )) == nLevelEntryData )
        {
            m_pLevelBox->SelectEntryPos( i );
            break;
        }
    }

    if (m_pParent->m_aJobData.m_nColorDepth == 8)
        m_pDepthBox->SelectEntryPos(0);
    else if (m_pParent->m_aJobData.m_nColorDepth == 24)
        m_pDepthBox->SelectEntryPos(1);

    // fill ppd boxes
    if( m_pParent->m_aJobData.m_pParser )
    {
        for( int i = 0; i < m_pParent->m_aJobData.m_pParser->getKeys(); i++ )
        {
            const PPDKey* pKey = m_pParent->m_aJobData.m_pParser->getKey( i );

            // skip options already shown somewhere else
            // also skip options from the "InstallableOptions" PPD group
            // Options in that group define hardware features that are not
            // job-specific and should better be handled in the system-wide
            // printer configuration. Keyword is defined in PPD specification
            // (version 4.3), section 5.4.
            if( pKey->isUIKey()                   &&
                pKey->getKey() != "PageSize"      &&
                pKey->getKey() != "InputSlot"     &&
                pKey->getKey() != "PageRegion"    &&
                pKey->getKey() != "Duplex"        &&
                pKey->getGroup() != "InstallableOptions")
            {
                OUString aEntry( m_pParent->m_aJobData.m_pParser->translateKey( pKey->getKey() ) );
                sal_uInt16 nPos = m_pPPDKeyBox->InsertEntry( aEntry );
                m_pPPDKeyBox->SetEntryData( nPos, const_cast<PPDKey *>(pKey) );
            }
        }
    }
}

RTSDevicePage::~RTSDevicePage()
{
    disposeOnce();
}

void RTSDevicePage::dispose()
{
    m_pParent.clear();
    m_pPPDKeyBox.clear();
    m_pPPDValueBox.clear();
    m_pCustomEdit.clear();
    m_pLevelBox.clear();
    m_pSpaceBox.clear();
    m_pDepthBox.clear();
    TabPage::dispose();
}

sal_uLong RTSDevicePage::getDepth()
{
    sal_uInt16 nSelectPos = m_pDepthBox->GetSelectEntryPos();
    if (nSelectPos == 0)
        return 8;
    else
        return 24;
}

sal_uLong RTSDevicePage::getColorDevice()
{
    sal_uInt16 nSelectPos = m_pSpaceBox->GetSelectEntryPos();
    switch (nSelectPos)
    {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return -1;
    }
    return 0;
}

sal_uLong RTSDevicePage::getLevel()
{
    sal_uLong nLevel = reinterpret_cast<sal_uLong>(m_pLevelBox->GetSelectEntryData());
    if (nLevel == 0)
        return 0;   //automatic
    return nLevel < 10 ? nLevel-1 : 0;
}

sal_uLong RTSDevicePage::getPDFDevice()
{
    sal_uLong nLevel = reinterpret_cast<sal_uLong>(m_pLevelBox->GetSelectEntryData());
    if (nLevel > 9)
        return 2;   //explicitly PDF
    else if (nLevel == 0)
        return 0;   //automatic
    return -1;      //explicitly PS
}

IMPL_LINK_TYPED(RTSDevicePage, ModifyHdl, Edit&, rEdit, void)
{
    if (m_pCustomValue)
    {
        m_pCustomValue->m_aCustomOption = rEdit.GetText();
    }
}

IMPL_LINK_TYPED( RTSDevicePage, SelectHdl, ListBox&, rBox, void )
{
    if( &rBox == m_pPPDKeyBox )
    {
        const PPDKey* pKey = static_cast<PPDKey*>(m_pPPDKeyBox->GetSelectEntryData());
        FillValueBox( pKey );
    }
    else if( &rBox == m_pPPDValueBox )
    {
        const PPDKey* pKey = static_cast<PPDKey*>(m_pPPDKeyBox->GetSelectEntryData());
        const PPDValue* pValue = static_cast<PPDValue*>(m_pPPDValueBox->GetSelectEntryData());
        if (pKey && pValue)
        {
            m_pParent->m_aJobData.m_aContext.setValue( pKey, pValue );
            FillValueBox( pKey );
        }
    }
    m_pParent->SetDataModified( true );
}

void RTSDevicePage::FillValueBox( const PPDKey* pKey )
{
    m_pPPDValueBox->Clear();
    m_pCustomEdit->Hide();

    if( ! pKey )
        return;

    const PPDValue* pValue = nullptr;
    for( int i = 0; i < pKey->countValues(); i++ )
    {
        pValue = pKey->getValue( i );
        if( m_pParent->m_aJobData.m_aContext.checkConstraints( pKey, pValue ) &&
            m_pParent->m_aJobData.m_pParser )
        {
            OUString aEntry;
            if (pValue->m_bCustomOption)
                aEntry = VclResId(SV_PRINT_CUSTOM_TXT);
            else
                aEntry = m_pParent->m_aJobData.m_pParser->translateOption( pKey->getKey(), pValue->m_aOption);
            sal_uInt16 nPos = m_pPPDValueBox->InsertEntry( aEntry );
            m_pPPDValueBox->SetEntryData( nPos, const_cast<PPDValue *>(pValue) );
        }
    }
    pValue = m_pParent->m_aJobData.m_aContext.getValue( pKey );
    m_pPPDValueBox->SelectEntryPos( m_pPPDValueBox->GetEntryPos( static_cast<void const *>(pValue) ) );
    if (pValue->m_bCustomOption)
    {
        m_pCustomValue = pValue;
        m_pParent->m_aJobData.m_aContext.setValue(pKey, pValue);
        m_pCustomEdit->SetText(m_pCustomValue->m_aCustomOption);
        m_pCustomEdit->Show();
    }
}

int SetupPrinterDriver(::psp::PrinterInfo& rJobData)
{
    int nRet = 0;
    ScopedVclPtrInstance< RTSDialog > aDialog(  rJobData, nullptr  );

    // return 0 if cancel was pressed or if the data
    // weren't modified, 1 otherwise
    if( aDialog->Execute() )
    {
        rJobData = aDialog->getSetup();
        nRet = aDialog->GetDataModified() ? 1 : 0;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
