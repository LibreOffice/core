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
#include "helper.hxx" // for PaResId
#include "rtsetup.hrc"
#include "cmddlg.hxx"

#include "vcl/fontmanager.hxx"

#include "osl/thread.h"

#include <officecfg/Office/Common.hxx>

using namespace psp;
using namespace padmin;


void RTSDialog::insertAllPPDValues( ListBox& rBox, const PPDParser* pParser, const PPDKey* pKey )
{
    if( ! pKey || ! pParser )
        return;

    const PPDValue* pValue = NULL;
    sal_uInt16 nPos = 0;
    String aOptionText;

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

// --------------------------------------------------------------------------

/*
 * RTSDialog
 */

RTSDialog::RTSDialog( const PrinterInfo& rJobData, const String& rPrinter, bool bAllPages, Window* pParent )
    : TabDialog(pParent, "PrinterPropertiesDialog", "spa/ui/printerpropertiesdialog.ui" )
    , m_aJobData(rJobData)
    , m_aPrinter(rPrinter)
    , m_pPaperPage(NULL)
    , m_pDevicePage(NULL)
    , m_pOtherPage(NULL)
    , m_pFontSubstPage(NULL)
    , m_pCommandPage(NULL)
    , m_aInvalidString(PaResId(RID_RTS_RTSDIALOG_INVALID_TXT).toString())
{
    get(m_pOKButton, "ok");
    get(m_pCancelButton, "cancel");
    get(m_pTabControl, "notebook");

    OUString aTitle(GetText());
    SetText(aTitle.replaceAll("%s", m_aJobData.m_aPrinterName));

    if( ! bAllPages )
    {
        m_pTabControl->RemovePage(m_pTabControl->GetPageId("other"));
        m_pTabControl->RemovePage(m_pTabControl->GetPageId("font"));
        m_pTabControl->RemovePage(m_pTabControl->GetPageId("command"));
    }
    else if( m_aJobData.m_aDriverName.startsWith("CUPS:") && ! PrinterInfoManager::get().isCUPSDisabled() )
    {
        // command page makes no sense for CUPS printers
        m_pTabControl->RemovePage(m_pTabControl->GetPageId("command"));
    }

    m_pTabControl->SetActivatePageHdl( LINK( this, RTSDialog, ActivatePage ) );
    m_pOKButton->SetClickHdl( LINK( this, RTSDialog, ClickButton ) );
    m_pCancelButton->SetClickHdl( LINK( this, RTSDialog, ClickButton ) );
    ActivatePage(m_pTabControl);
}

// --------------------------------------------------------------------------

RTSDialog::~RTSDialog()
{
    delete m_pPaperPage;
    delete m_pDevicePage;
    delete m_pOtherPage;
    delete m_pFontSubstPage;
    delete m_pCommandPage;
}

// --------------------------------------------------------------------------

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
        else if (sPage == "other")
            pPage = m_pOtherPage = new RTSOtherPage( this );
        else if (sPage == "font")
            pPage = m_pFontSubstPage = new RTSFontSubstPage( this );
        else if (sPage == "command")
            pPage = m_pCommandPage = new RTSCommandPage( this );
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

// --------------------------------------------------------------------------

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
        if( m_pOtherPage )
            // write other settings
            m_pOtherPage->save();
        if( m_pCommandPage )
            // write command settings
            m_pCommandPage->save();

        EndDialog( 1 );
    }
    else if( pButton == m_pCancelButton )
        EndDialog( 0 );

    return 0;
}

// --------------------------------------------------------------------------

/*
 * RTSPaperPage
 */

RTSPaperPage::RTSPaperPage(RTSDialog* pParent)
    : TabPage(pParent->m_pTabControl, "PrinterPaperPage", "spa/ui/printerpaperpage.ui" )
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

    sal_uInt16 nPos = 0;

    // duplex
    nPos = m_pDuplexBox->InsertEntry( m_pParent->m_aInvalidString );
    m_pDuplexBox->SetEntryData( nPos, NULL );

    // paper does not have an invalid entry

    // input slots
    nPos = m_pSlotBox->InsertEntry( m_pParent->m_aInvalidString );
    m_pSlotBox->SetEntryData( nPos, NULL );

    update();
}

// --------------------------------------------------------------------------

RTSPaperPage::~RTSPaperPage()
{
}

// --------------------------------------------------------------------------

void RTSPaperPage::update()
{
    const PPDKey* pKey      = NULL;

    // orientation
    m_pOrientBox->SelectEntryPos(
        m_pParent->m_aJobData.m_eOrientation == orientation::Portrait ? 0 : 1);

    // duplex
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( String( "Duplex" ) )) )
    {
        m_pParent->insertAllPPDValues( *m_pDuplexBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_pDuplexText->Enable( sal_False );
        m_pDuplexBox->Enable( sal_False );
    }

    // paper
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( String( "PageSize" ) )) )
    {
        m_pParent->insertAllPPDValues( *m_pPaperBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_pPaperText->Enable( sal_False );
        m_pPaperBox->Enable( sal_False );
    }

    // input slots
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( OUString("InputSlot") )) )
    {
        m_pParent->insertAllPPDValues( *m_pSlotBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_pSlotText->Enable( sal_False );
        m_pSlotBox->Enable( sal_False );
    }
}

// --------------------------------------------------------------------------

IMPL_LINK( RTSPaperPage, SelectHdl, ListBox*, pBox )
{
    const PPDKey* pKey = NULL;
    if( pBox == m_pPaperBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( String( "PageSize" ) );
    }
    else if( pBox == m_pDuplexBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( String( "Duplex" ) );
    }
    else if( pBox == m_pSlotBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( String( "InputSlot" ) );
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

// --------------------------------------------------------------------------

/*
 * RTSDevicePage
 */

RTSDevicePage::RTSDevicePage( RTSDialog* pParent )
    : TabPage(pParent->m_pTabControl, "PrinterDevicePage", "spa/ui/printerdevicepage.ui" )
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

    assert(nLevelEntryData != 0 || bAutoIsPDF == m_pParent->m_aJobData.m_nPDFDevice);

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
                String aEntry( m_pParent->m_aJobData.m_pParser->translateKey( pKey->getKey() ) );
                sal_uInt16 nPos = m_pPPDKeyBox->InsertEntry( aEntry );
                m_pPPDKeyBox->SetEntryData( nPos, (void*)pKey );
            }
        }
    }
}

// --------------------------------------------------------------------------

RTSDevicePage::~RTSDevicePage()
{
}

// --------------------------------------------------------------------------

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

// ------------------------------------------------------------------

sal_uLong RTSDevicePage::getLevel()
{
    sal_uLong nLevel = (sal_uLong)m_pLevelBox->GetEntryData( m_pLevelBox->GetSelectEntryPos() );
    if (nLevel == 0)
        return 0;   //automatic
    return nLevel < 10 ? nLevel-1 : 0;
}

// ------------------------------------------------------------------

sal_uLong RTSDevicePage::getPDFDevice()
{
    sal_uLong nLevel = (sal_uLong)m_pLevelBox->GetEntryData( m_pLevelBox->GetSelectEntryPos() );
    if (nLevel > 9)
        return 2;   //explictly PDF
    else if (nLevel == 0)
        return 0;   //automatic
    return -1;      //explicitly PS
}

// ------------------------------------------------------------------

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

// ------------------------------------------------------------------

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
            String aEntry( m_pParent->m_aJobData.m_pParser->translateOption( pKey->getKey(), pValue->m_aOption ) );
            sal_uInt16 nPos = m_pPPDValueBox->InsertEntry( aEntry );
            m_pPPDValueBox->SetEntryData( nPos, (void*)pValue );
        }
    }
    pValue = m_pParent->m_aJobData.m_aContext.getValue( pKey );
    m_pPPDValueBox->SelectEntryPos( m_pPPDValueBox->GetEntryPos( (void*)pValue ) );
}

// --------------------------------------------------------------------------

/*
 * RTSOtherPage
 */

RTSOtherPage::RTSOtherPage( RTSDialog* pParent ) :
        TabPage( pParent->m_pTabControl, PaResId( RID_RTS_OTHERPAGE ) ),
        m_pParent( pParent ),
        m_aLeftTxt( this, PaResId( RID_RTS_OTHER_LEFTMARGIN_TXT ) ),
        m_aLeftLB( this, PaResId( RID_RTS_OTHER_LEFTMARGIN_BOX ) ),
        m_aTopTxt( this, PaResId( RID_RTS_OTHER_TOPMARGIN_TXT ) ),
        m_aTopLB( this, PaResId( RID_RTS_OTHER_TOPMARGIN_BOX ) ),
        m_aRightTxt( this, PaResId( RID_RTS_OTHER_RIGHTMARGIN_TXT ) ),
        m_aRightLB( this, PaResId( RID_RTS_OTHER_RIGHTMARGIN_BOX ) ),
        m_aBottomTxt( this, PaResId( RID_RTS_OTHER_BOTTOMMARGIN_TXT ) ),
        m_aBottomLB( this, PaResId( RID_RTS_OTHER_BOTTOMMARGIN_BOX ) ),
        m_aCommentTxt( this, PaResId( RID_RTS_OTHER_COMMENT_TXT ) ),
        m_aCommentEdt( this, PaResId( RID_RTS_OTHER_COMMENT_EDT ) ),
        m_aDefaultBtn( this, PaResId( RID_RTS_OTHER_DEFAULT_BTN ) )
{
    FreeResource();

    m_aTopLB.EnableEmptyFieldValue( sal_True );
    m_aBottomLB.EnableEmptyFieldValue( sal_True );
    m_aLeftLB.EnableEmptyFieldValue( sal_True );
    m_aRightLB.EnableEmptyFieldValue( sal_True );

    m_aDefaultBtn.SetClickHdl( LINK( this, RTSOtherPage, ClickBtnHdl ) );

    initValues();
}

// ------------------------------------------------------------------

RTSOtherPage::~RTSOtherPage()
{
}

// ------------------------------------------------------------------

void RTSOtherPage::initValues()
{
    int nMarginLeft = 0;
    int nMarginTop = 0;
    int nMarginRight = 0;
    int nMarginBottom = 0;

    if( m_pParent->m_aJobData.m_pParser )
    {
        m_pParent->m_aJobData.m_pParser->
            getMargins( m_pParent->m_aJobData.m_pParser->getDefaultPaperDimension(),
                        nMarginLeft,
                        nMarginRight,
                        nMarginTop,
                        nMarginBottom );
    }

    nMarginLeft     += m_pParent->m_aJobData.m_nLeftMarginAdjust;
    nMarginRight    += m_pParent->m_aJobData.m_nRightMarginAdjust;
    nMarginTop      += m_pParent->m_aJobData.m_nTopMarginAdjust;
    nMarginBottom   += m_pParent->m_aJobData.m_nBottomMarginAdjust;

    m_aLeftLB.SetValue( nMarginLeft, FUNIT_POINT );
    m_aRightLB.SetValue( nMarginRight, FUNIT_POINT );
    m_aTopLB.SetValue( nMarginTop, FUNIT_POINT );
    m_aBottomLB.SetValue( nMarginBottom, FUNIT_POINT );
    m_aCommentEdt.SetText( m_pParent->m_aJobData.m_aComment );
}

// ------------------------------------------------------------------

void RTSOtherPage::save()
{
    int nMarginLeft = 0;
    int nMarginTop = 0;
    int nMarginRight = 0;
    int nMarginBottom = 0;

    if( m_pParent->m_aJobData.m_pParser )
    {
        m_pParent->m_aJobData.m_pParser->
            getMargins( m_pParent->m_aJobData.m_pParser->getDefaultPaperDimension(),
                        nMarginLeft,
                        nMarginRight,
                        nMarginTop,
                        nMarginBottom );
    }

    m_pParent->m_aJobData.m_nLeftMarginAdjust   = m_aLeftLB.GetValue( FUNIT_POINT ) - nMarginLeft;
    m_pParent->m_aJobData.m_nRightMarginAdjust  = m_aRightLB.GetValue( FUNIT_POINT ) - nMarginRight;
    m_pParent->m_aJobData.m_nTopMarginAdjust    = m_aTopLB.GetValue( FUNIT_POINT ) - nMarginTop;
    m_pParent->m_aJobData.m_nBottomMarginAdjust = m_aBottomLB.GetValue( FUNIT_POINT ) - nMarginBottom;
    m_pParent->m_aJobData.m_aComment = m_aCommentEdt.GetText();
}

// ------------------------------------------------------------------

IMPL_LINK( RTSOtherPage, ClickBtnHdl, Button*, pButton )
{
    if( pButton == &m_aDefaultBtn )
    {
        m_pParent->m_aJobData.m_nLeftMarginAdjust =
            m_pParent->m_aJobData.m_nRightMarginAdjust =
            m_pParent->m_aJobData.m_nTopMarginAdjust =
            m_pParent->m_aJobData.m_nBottomMarginAdjust = 0;

        initValues();
    }
    return 0;
}

// ------------------------------------------------------------------

/*
 *  RTSFontSubstPage
 */

RTSFontSubstPage::RTSFontSubstPage( RTSDialog* pParent ) :
        TabPage( pParent->m_pTabControl, PaResId( RID_RTS_FONTSUBSTPAGE ) ),
        m_pParent( pParent ),
        m_aSubstitutionsText( this, PaResId( RID_RTS_FS_SUBST_TXT ) ),
        m_aSubstitutionsBox( this, PaResId( RID_RTS_FS_SUBST_BOX ) ),
        m_aFromFontText( this, PaResId( RID_RTS_FS_FROM_TXT ) ),
        m_aFromFontBox( this, PaResId( RID_RTS_FS_FROM_BOX ) ),
        m_aToFontText( this, PaResId( RID_RTS_FS_TO_TXT ) ),
        m_aToFontBox( this, PaResId( RID_RTS_FS_TO_BOX ) ),
        m_aAddButton( this, PaResId( RID_RTS_FS_ADD_BTN ) ),
        m_aRemoveButton( this, PaResId( RID_RTS_FS_REMOVE_BTN ) ),
        m_aEnableBox( this, PaResId( RID_RTS_FS_ENABLE_BTN ) )
{
    FreeResource();

    // fill to box
    PrintFontManager& rFontManager = PrintFontManager::get();
    ::std::list< FastPrintFontInfo > aFonts;
    rFontManager.getFontListWithFastInfo( aFonts, m_pParent->m_aJobData.m_pParser );
    ::std::list< FastPrintFontInfo >::const_iterator it;
    ::boost::unordered_map< OUString, int, OUStringHash > aToMap, aFromMap;
    for( it = aFonts.begin(); it != aFonts.end(); ++it )
    {
        if( it->m_eType == fonttype::Builtin )
        {
            if( aToMap.find( it->m_aFamilyName ) == aToMap.end() )
            {
                m_aToFontBox.InsertEntry( it->m_aFamilyName );
                aToMap[ it->m_aFamilyName ] = 1;
            }
        }
        else
        {
            if( aFromMap.find( it->m_aFamilyName ) == aFromMap.end() )
            {
                m_aFromFontBox.InsertEntry( it->m_aFamilyName );
                aFromMap[ it->m_aFamilyName ] = 1;
            }
        }
    }

    m_aEnableBox.Check( m_pParent->m_aJobData.m_bPerformFontSubstitution );
    m_aRemoveButton.Enable( sal_False );
    if( ! m_pParent->m_aJobData.m_bPerformFontSubstitution )
    {
        m_aSubstitutionsBox.Enable( sal_False );
        m_aSubstitutionsText.Enable( sal_False );
        m_aAddButton.Enable( sal_False );
        m_aToFontBox.Enable( sal_False );
        m_aToFontText.Enable( sal_False );
        m_aFromFontBox.Enable( sal_False );
        m_aFromFontText.Enable( sal_False );
    }

    update();

    m_aAddButton.SetClickHdl( LINK( this, RTSFontSubstPage, ClickBtnHdl ) );
    m_aRemoveButton.SetClickHdl( LINK( this, RTSFontSubstPage, ClickBtnHdl ) );
    m_aEnableBox.SetClickHdl( LINK( this, RTSFontSubstPage, ClickBtnHdl ) );
    m_aSubstitutionsBox.SetSelectHdl( LINK( this, RTSFontSubstPage, SelectHdl ) );
    m_aSubstitutionsBox.setDelPressedLink( LINK( this, RTSFontSubstPage, DelPressedHdl ) );
}

RTSFontSubstPage::~RTSFontSubstPage()
{
}

void RTSFontSubstPage::update()
{
    m_aSubstitutionsBox.Clear();
    m_aRemoveButton.Enable( sal_False );
    // fill substitutions box
    ::boost::unordered_map< OUString, OUString, OUStringHash >::const_iterator it;
    for( it = m_pParent->m_aJobData.m_aFontSubstitutes.begin();
         it != m_pParent->m_aJobData.m_aFontSubstitutes.end(); ++it )
    {
        OUString aEntry = it->first + " -> " + it->second;
        m_aSubstitutionsBox.InsertEntry( aEntry );
    }
}

IMPL_LINK( RTSFontSubstPage, DelPressedHdl, ListBox*, pBox )
{
    if( pBox == &m_aSubstitutionsBox &&
        m_aRemoveButton.IsEnabled() )
        ClickBtnHdl( &m_aRemoveButton );
    return 0;
}

IMPL_LINK( RTSFontSubstPage, SelectHdl, ListBox*, pBox )
{
    if( pBox == &m_aSubstitutionsBox )
    {
        m_aRemoveButton.Enable( m_aSubstitutionsBox.GetSelectEntryCount() && m_pParent->m_aJobData.m_bPerformFontSubstitution );
    }
    return 0;
}

IMPL_LINK( RTSFontSubstPage, ClickBtnHdl, Button*, pButton )
{
    if( pButton == &m_aAddButton )
    {
        m_pParent->m_aJobData.m_aFontSubstitutes[ m_aFromFontBox.GetText() ] = m_aToFontBox.GetSelectEntry();
        update();
    }
    else if( pButton == &m_aRemoveButton )
    {
        for( int i = 0; i < m_aSubstitutionsBox.GetSelectEntryCount(); i++ )
        {
            String aEntry( m_aSubstitutionsBox.GetSelectEntry( i ) );
            sal_uInt16 nPos = aEntry.SearchAscii( " -> " );
            aEntry.Erase( nPos );
            m_pParent->m_aJobData.m_aFontSubstitutes.erase( aEntry );
        }
        update();
    }
    else if( pButton == &m_aEnableBox )
    {
        m_pParent->m_aJobData.m_bPerformFontSubstitution = m_aEnableBox.IsChecked() ? true : false;
        m_aSubstitutionsBox.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aSubstitutionsText.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aAddButton.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aRemoveButton.Enable( m_aSubstitutionsBox.GetSelectEntryCount() && m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aToFontBox.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aToFontText.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aFromFontBox.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
        m_aFromFontText.Enable( m_pParent->m_aJobData.m_bPerformFontSubstitution );
    }
    return 0;
}


class RTSPWDialog : public ModalDialog
{
    FixedText       m_aText;
    FixedText       m_aUserText;
    Edit            m_aUserEdit;
    FixedText       m_aPassText;
    Edit            m_aPassEdit;

    OKButton        m_aOKButton;
    CancelButton    m_aCancelButton;
public:
    RTSPWDialog( const OString& rServer, const OString& rUserName, Window* pParent );
    ~RTSPWDialog();

    OString getUserName() const;
    OString getPassword() const;
};

RTSPWDialog::RTSPWDialog( const OString& rServer, const OString& rUserName, Window* pParent )
        :
        ModalDialog( pParent, PaResId( RID_RTS_PWDIALOG ) ),
        m_aText( this, PaResId( RID_RTS_PWDIALOG_TXT ) ),
        m_aUserText( this, PaResId( RID_RTS_PWDIALOG_USER_TXT ) ),
        m_aUserEdit( this, PaResId( RID_RTS_PWDIALOG_USER_EDT ) ),
        m_aPassText( this, PaResId( RID_RTS_PWDIALOG_PASS_TXT ) ),
        m_aPassEdit( this, PaResId( RID_RTS_PWDIALOG_PASS_EDT ) ),
        m_aOKButton( this, PaResId( RID_RTS_PWDIALOG_OK_BTN ) ),
        m_aCancelButton( this, PaResId( RID_RTS_PWDIALOG_CANCEL_BTN ) )
{
    FreeResource();
    String aText( m_aText.GetText() );
    aText.SearchAndReplace( String( "%s" ), OStringToOUString( rServer, osl_getThreadTextEncoding() ) );
    m_aText.SetText( aText );
    m_aUserEdit.SetText( OStringToOUString( rUserName, osl_getThreadTextEncoding() ) );
}

RTSPWDialog::~RTSPWDialog()
{
}

OString RTSPWDialog::getUserName() const
{
    return OUStringToOString( m_aUserEdit.GetText(), osl_getThreadTextEncoding() );
}

OString RTSPWDialog::getPassword() const
{
    return OUStringToOString( m_aPassEdit.GetText(), osl_getThreadTextEncoding() );
}

extern "C" {

    int SPA_DLLPUBLIC Sal_SetupPrinterDriver( ::psp::PrinterInfo& rJobData )
    {
        int nRet = 0;
        RTSDialog aDialog( rJobData, rJobData.m_aPrinterName, false );

        if( aDialog.Execute() )
        {
            rJobData = aDialog.getSetup();
            nRet = 1;
        }

        return nRet;
    }

    int SPA_DLLPUBLIC Sal_queryFaxNumber( String& rNumber )
    {
        String aTmpString( PaResId( RID_TXT_QUERYFAXNUMBER ) );
        QueryString aQuery( NULL, aTmpString, rNumber );
        return aQuery.Execute();
    }

    bool SPA_DLLPUBLIC Sal_authenticateQuery( const OString& rServer, OString& rUserName, OString& rPassword )
    {
        bool bRet = false;

        RTSPWDialog aDialog( rServer, rUserName, NULL );
        if( aDialog.Execute() )
        {
            rUserName = aDialog.getUserName();
            rPassword = aDialog.getPassword();
            bRet = true;
        }
        return bRet;
    }

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
