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

    const PPDValue* pValue = NULL;
    sal_Int32 nPos = 0;
    OUString aOptionText;

    for( int i = 0; i < pKey->countValues(); i++ )
    {
        pValue = pKey->getValue( i );
        aOptionText = pParser->translateOption( pKey->getKey(), pValue->m_aOption) ;

        if( m_aJobData.m_aContext.checkConstraints( pKey, pValue ) )
        {
            if( rBox.GetEntryPos( (void*)pValue ) == LISTBOX_ENTRY_NOTFOUND )
            {
                nPos = rBox.InsertEntry( aOptionText, LISTBOX_APPEND );
                    rBox.SetEntryData( nPos, (void*)pValue );
            }
        }
        else
        {
            if( ( nPos = rBox.GetEntryPos( (void*)pValue ) ) != LISTBOX_ENTRY_NOTFOUND )
                rBox.RemoveEntry( nPos );
        }
    }
    pValue = m_aJobData.m_aContext.getValue( pKey );
    if( pValue )
    {
        if( ( nPos = rBox.GetEntryPos( (void*)pValue ) ) != LISTBOX_ENTRY_NOTFOUND )
            rBox.SelectEntryPos( nPos );
    }
    else
        rBox.SelectEntry( m_aInvalidString );
}



/*
 * RTSDialog
 */

RTSDialog::RTSDialog(const PrinterInfo& rJobData, const OUString& rPrinter, Window* pParent)
    : TabDialog(pParent, "PrinterPropertiesDialog", "vcl/ui/printerpropertiesdialog.ui")
    , m_aJobData(rJobData)
    , m_aPrinter(rPrinter)
    , m_pPaperPage(NULL)
    , m_pDevicePage(NULL)
    , m_aInvalidString(VclResId(SV_PRINT_INVALID_TXT))
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
    delete m_pPaperPage;
    delete m_pDevicePage;
}

IMPL_LINK( RTSDialog, ActivatePage, TabControl*, pTabCtrl )
{
    if( pTabCtrl != m_pTabControl )
        return 0;

    sal_uInt16 nId = m_pTabControl->GetCurPageId();
    OString sPage = m_pTabControl->GetPageName(nId);
    if ( ! m_pTabControl->GetTabPage( nId ) )
    {
        TabPage *pPage = NULL;
        if (sPage == "paper")
            pPage = m_pPaperPage = new RTSPaperPage( this );
        else if (sPage == "device")
            pPage = m_pDevicePage = new RTSDevicePage( this );
        if( pPage )
            m_pTabControl->SetTabPage( nId, pPage );
    }
    else
    {
        if (sPage == "paper")
            m_pPaperPage->update();
        else if (sPage == "device")
            m_pDevicePage->update();
    }

    return 0;
}



IMPL_LINK( RTSDialog, ClickButton, Button*, pButton )
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
        EndDialog( 0 );

    return 0;
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
    m_pDuplexBox->SetEntryData( nPos, NULL );

    // paper does not have an invalid entry

    // input slots
    nPos = m_pSlotBox->InsertEntry( m_pParent->m_aInvalidString );
    m_pSlotBox->SetEntryData( nPos, NULL );

    update();
}



RTSPaperPage::~RTSPaperPage()
{
}



void RTSPaperPage::update()
{
    const PPDKey* pKey      = NULL;

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
}



IMPL_LINK( RTSPaperPage, SelectHdl, ListBox*, pBox )
{
    const PPDKey* pKey = NULL;
    if( pBox == m_pPaperBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "PageSize" ) );
    }
    else if( pBox == m_pDuplexBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "Duplex" ) );
    }
    else if( pBox == m_pSlotBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString( "InputSlot" ) );
    }
    else if( pBox == m_pOrientBox )
    {
        m_pParent->m_aJobData.m_eOrientation = m_pOrientBox->GetSelectEntryPos() == 0 ? orientation::Portrait : orientation::Landscape;
    }
    if( pKey )
    {
        PPDValue* pValue =
            (PPDValue*)pBox->GetEntryData( pBox->GetSelectEntryPos() );
        m_pParent->m_aJobData.m_aContext.setValue( pKey, pValue );
        update();
    }
    return 0;
}



/*
 * RTSDevicePage
 */

RTSDevicePage::RTSDevicePage( RTSDialog* pParent )
    : TabPage(pParent->m_pTabControl, "PrinterDevicePage", "vcl/ui/printerdevicepage.ui" )
    , m_pParent( pParent )
{
    get(m_pPPDKeyBox, "options");
    get(m_pPPDValueBox, "values");

    m_pPPDKeyBox->SetDropDownLineCount(12);
    m_pPPDValueBox->SetDropDownLineCount(12);

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
    if( m_pParent->m_aJobData.m_nPDFDevice == 2 ) //explicitly PDF
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

    for( sal_uInt16 i = 0; i < m_pLevelBox->GetEntryCount(); i++ )
    {
        if( (sal_uLong)m_pLevelBox->GetEntryData( i ) == nLevelEntryData )
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
            if( pKey->isUIKey()                                 &&
                ! pKey->getKey().equalsAscii( "PageSize" )      &&
                ! pKey->getKey().equalsAscii( "InputSlot" )     &&
                ! pKey->getKey().equalsAscii( "PageRegion" )    &&
                ! pKey->getKey().equalsAscii( "Duplex" )
                )
            {
                OUString aEntry( m_pParent->m_aJobData.m_pParser->translateKey( pKey->getKey() ) );
                sal_uInt16 nPos = m_pPPDKeyBox->InsertEntry( aEntry );
                m_pPPDKeyBox->SetEntryData( nPos, (void*)pKey );
            }
        }
    }
}



RTSDevicePage::~RTSDevicePage()
{
}



void RTSDevicePage::update()
{
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
    sal_uLong nLevel = (sal_uLong)m_pLevelBox->GetEntryData( m_pLevelBox->GetSelectEntryPos() );
    if (nLevel == 0)
        return 0;   //automatic
    return nLevel < 10 ? nLevel-1 : 0;
}



sal_uLong RTSDevicePage::getPDFDevice()
{
    sal_uLong nLevel = (sal_uLong)m_pLevelBox->GetEntryData( m_pLevelBox->GetSelectEntryPos() );
    if (nLevel > 9)
        return 2;   //explictly PDF
    else if (nLevel == 0)
        return 0;   //automatic
    return -1;      //explicitly PS
}



IMPL_LINK( RTSDevicePage, SelectHdl, ListBox*, pBox )
{
    if( pBox == m_pPPDKeyBox )
    {
        const PPDKey* pKey = (PPDKey*)m_pPPDKeyBox->GetEntryData( m_pPPDKeyBox->GetSelectEntryPos() );
        FillValueBox( pKey );
    }
    else if( pBox == m_pPPDValueBox )
    {
        const PPDKey* pKey = (PPDKey*)m_pPPDKeyBox->GetEntryData( m_pPPDKeyBox->GetSelectEntryPos() );
        const PPDValue* pValue = (PPDValue*)m_pPPDValueBox->GetEntryData( m_pPPDValueBox->GetSelectEntryPos() );
        if( pKey && pValue )
        {
            m_pParent->m_aJobData.m_aContext.setValue( pKey, pValue );
            FillValueBox( pKey );
        }
    }
    return 0;
}



void RTSDevicePage::FillValueBox( const PPDKey* pKey )
{
    m_pPPDValueBox->Clear();

    if( ! pKey )
        return;

    const PPDValue* pValue = NULL;
    for( int i = 0; i < pKey->countValues(); i++ )
    {
        pValue = pKey->getValue( i );
        if( m_pParent->m_aJobData.m_aContext.checkConstraints( pKey, pValue ) &&
            m_pParent->m_aJobData.m_pParser )
        {
            OUString aEntry( m_pParent->m_aJobData.m_pParser->translateOption( pKey->getKey(), pValue->m_aOption ) );
            sal_uInt16 nPos = m_pPPDValueBox->InsertEntry( aEntry );
            m_pPPDValueBox->SetEntryData( nPos, (void*)pValue );
        }
    }
    pValue = m_pParent->m_aJobData.m_aContext.getValue( pKey );
    m_pPPDValueBox->SelectEntryPos( m_pPPDValueBox->GetEntryPos( (void*)pValue ) );
}

int SetupPrinterDriver(::psp::PrinterInfo& rJobData)
{
    int nRet = 0;
    RTSDialog aDialog( rJobData, rJobData.m_aPrinterName, NULL );

    if( aDialog.Execute() )
    {
        rJobData = aDialog.getSetup();
        nRet = 1;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
