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

#include "prtsetup.hxx"
#include "helper.hxx" // for PaResId
#include "rtsetup.hrc"
#include "cmddlg.hxx"

#include "vcl/fontmanager.hxx"

#include "osl/thread.h"

#define LSCAPE_STRING String( RTL_CONSTASCII_USTRINGPARAM( "Landscape" ) )
#define PORTRAIT_STRING String( RTL_CONSTASCII_USTRINGPARAM( "Portrait" ) )

using namespace rtl;
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

RTSDialog::RTSDialog( const PrinterInfo& rJobData, const String& rPrinter, bool bAllPages, Window* pParent ) :
        TabDialog(  pParent, PaResId( RID_RTS_RTSDIALOG ) ),
        m_aJobData( rJobData ),
        m_aPrinter( rPrinter ),
        m_aTabControl( this, PaResId( RID_RTS_RTSDIALOG_TABCONTROL ) ),
        m_aOKButton( this ),
        m_aCancelButton( this ),
        m_pPaperPage( NULL ),
        m_pDevicePage( NULL ),
        m_pOtherPage( NULL ),
        m_pFontSubstPage( NULL ),
        m_pCommandPage( NULL ),
        m_aInvalidString( PaResId( RID_RTS_RTSDIALOG_INVALID_TXT ) ),
        m_aFromDriverString( PaResId( RID_RTS_RTSDIALOG_FROMDRIVER_TXT ) )
{
    FreeResource();

    String aTitle( GetText() );
    aTitle.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), m_aJobData.m_aPrinterName );
    SetText( aTitle );

    if( ! bAllPages )
    {
        m_aTabControl.RemovePage( RID_RTS_OTHERPAGE );
        m_aTabControl.RemovePage( RID_RTS_FONTSUBSTPAGE );
        m_aTabControl.RemovePage( RID_RTS_COMMANDPAGE );
    }
    else if( m_aJobData.m_aDriverName.compareToAscii( "CUPS:", 5 ) == 0 && ! PrinterInfoManager::get().isCUPSDisabled() )
    {
        // command page makes no sense for CUPS printers
        m_aTabControl.RemovePage( RID_RTS_COMMANDPAGE );
    }

    m_aTabControl.SetActivatePageHdl( LINK( this, RTSDialog, ActivatePage ) );
    m_aOKButton.SetClickHdl( LINK( this, RTSDialog, ClickButton ) );
    m_aCancelButton.SetClickHdl( LINK( this, RTSDialog, ClickButton ) );
    ActivatePage( &m_aTabControl );

    m_aOKButton.Show();
    m_aCancelButton.Show();
}

// --------------------------------------------------------------------------

RTSDialog::~RTSDialog()
{
    if( m_pPaperPage )
        delete m_pPaperPage;
    if( m_pDevicePage )
        delete m_pDevicePage;
    if( m_pOtherPage )
        delete m_pOtherPage;
    if( m_pFontSubstPage )
        delete m_pFontSubstPage;
    if( m_pCommandPage )
        delete m_pCommandPage;
}

// --------------------------------------------------------------------------

IMPL_LINK( RTSDialog, ActivatePage, TabControl*, pTabCtrl )
{
    if( pTabCtrl != &m_aTabControl )
        return 0;

    sal_uInt16 nId = m_aTabControl.GetCurPageId();

    if ( ! m_aTabControl.GetTabPage( nId ) )
    {
        TabPage *pPage = NULL;
        if( nId == RID_RTS_PAPERPAGE )
            pPage = m_pPaperPage = new RTSPaperPage( this );
        else if( nId == RID_RTS_DEVICEPAGE )
            pPage = m_pDevicePage = new RTSDevicePage( this );
        else if( nId == RID_RTS_OTHERPAGE )
            pPage = m_pOtherPage = new RTSOtherPage( this );
        else if( nId == RID_RTS_FONTSUBSTPAGE )
            pPage = m_pFontSubstPage = new RTSFontSubstPage( this );
        else if( nId == RID_RTS_COMMANDPAGE )
            pPage = m_pCommandPage = new RTSCommandPage( this );
        if( pPage )
            m_aTabControl.SetTabPage( nId, pPage );
    }
    else
    {
        switch( nId )
        {
            case RID_RTS_PAPERPAGE:     m_pPaperPage->update();break;
            case RID_RTS_DEVICEPAGE:    m_pDevicePage->update();break;
            default: break;
        }
    }

    return 0;
}

// --------------------------------------------------------------------------

IMPL_LINK( RTSDialog, ClickButton, Button*, pButton )
{
    if( pButton == &m_aOKButton )
    {
        // refresh the changed values
        if( m_pPaperPage )
        {
            // orientation
            m_aJobData.m_eOrientation = m_pPaperPage->getOrientation().Equals( LSCAPE_STRING ) ? orientation::Landscape : orientation::Portrait;
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
    else if( pButton == &m_aCancelButton )
        EndDialog( 0 );

    return 0;
}

// --------------------------------------------------------------------------

/*
 * RTSPaperPage
 */

RTSPaperPage::RTSPaperPage( RTSDialog* pParent ) :
        TabPage( & pParent->m_aTabControl, PaResId( RID_RTS_PAPERPAGE ) ),

        m_pParent( pParent ),

        m_aPaperText( this, PaResId( RID_RTS_PAPER_PAPER_TXT ) ),
        m_aPaperBox( this, PaResId( RID_RTS_PAPER_PAPER_BOX ) ),
        m_aOrientText( this, PaResId( RID_RTS_PAPER_ORIENTATION_TXT ) ),
        m_aOrientBox( this, PaResId( RID_RTS_PAPER_ORIENTATION_BOX ) ),
        m_aDuplexText( this, PaResId( RID_RTS_PAPER_DUPLEX_TXT ) ),
        m_aDuplexBox( this, PaResId( RID_RTS_PAPER_DUPLEX_BOX ) ),
        m_aSlotText( this, PaResId( RID_RTS_PAPER_SLOT_TXT ) ),
        m_aSlotBox( this, PaResId( RID_RTS_PAPER_SLOT_BOX ) )
{
    m_aPaperBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aOrientBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aDuplexBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aSlotBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );

    FreeResource();

    sal_uInt16 nPos = 0;

    m_aOrientBox.InsertEntry( PORTRAIT_STRING );
    m_aOrientBox.InsertEntry( LSCAPE_STRING );
    // duplex
    nPos = m_aDuplexBox.InsertEntry( m_pParent->m_aInvalidString );
    m_aDuplexBox.SetEntryData( nPos, NULL );

    // paper does not have an invalid entry

    // input slots
    nPos = m_aSlotBox.InsertEntry( m_pParent->m_aInvalidString );
    m_aSlotBox.SetEntryData( nPos, NULL );

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
    m_aOrientBox.SelectEntry(
        m_pParent->m_aJobData.m_eOrientation == orientation::Landscape
        ? LSCAPE_STRING : PORTRAIT_STRING );

    // duplex
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) )) )
    {
        m_pParent->insertAllPPDValues( m_aDuplexBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_aDuplexText.Enable( sal_False );
        m_aDuplexBox.Enable( sal_False );
    }

    // paper
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) )) )
    {
        m_pParent->insertAllPPDValues( m_aPaperBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_aPaperText.Enable( sal_False );
        m_aPaperBox.Enable( sal_False );
    }

    // input slots
    if( m_pParent->m_aJobData.m_pParser &&
        (pKey = m_pParent->m_aJobData.m_pParser->getKey( String::CreateFromAscii( "InputSlot" ) )) )
    {
        m_pParent->insertAllPPDValues( m_aSlotBox, m_pParent->m_aJobData.m_pParser, pKey );
    }
    else
    {
        m_aSlotText.Enable( sal_False );
        m_aSlotBox.Enable( sal_False );
    }
}

// --------------------------------------------------------------------------

IMPL_LINK( RTSPaperPage, SelectHdl, ListBox*, pBox )
{
    const PPDKey* pKey = NULL;
    if( pBox == &m_aPaperBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
    }
    else if( pBox == &m_aDuplexBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
    }
    else if( pBox == &m_aSlotBox )
    {
        if( m_pParent->m_aJobData.m_pParser )
            pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
    }
    else if( pBox == &m_aOrientBox )
    {
        m_pParent->m_aJobData.m_eOrientation = m_aOrientBox.GetSelectEntry().Equals( LSCAPE_STRING ) ? orientation::Landscape : orientation::Portrait;
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

RTSDevicePage::RTSDevicePage( RTSDialog* pParent ) :
        TabPage( & pParent->m_aTabControl, PaResId( RID_RTS_DEVICEPAGE ) ),

        m_pParent( pParent ),

        m_aSpaceColor( PaResId( RID_RTS_DEVICE_COLOR_TXT ) ),
        m_aSpaceGray( PaResId( RID_RTS_DEVICE_GRAY_TXT ) ),
        m_aPPDKeyText( this, PaResId( RID_RTS_DEVICE_PPDKEY_TXT ) ),
        m_aPPDKeyBox( this, PaResId( RID_RTS_DEVICE_PPDKEY_BOX ) ),
        m_aPPDValueText( this, PaResId( RID_RTS_DEVICE_PPDVALUE_TXT ) ),
        m_aPPDValueBox( this, PaResId( RID_RTS_DEVICE_PPDVALUE_BOX ) ),
        m_aLevelText( this, PaResId( RID_RTS_DEVICE_PRINTLANG_TXT ) ),
        m_aLevelBox( this, PaResId( RID_RTS_DEVICE_PRINTLANG_BOX ) ),
        m_aSpaceText( this, PaResId( RID_RTS_DEVICE_SPACE_TXT ) ),
        m_aSpaceBox( this, PaResId( RID_RTS_DEVICE_SPACE_BOX ) ),
        m_aDepthText( this, PaResId( RID_RTS_DEVICE_DEPTH_TXT ) ),
        m_aDepthBox( this, PaResId( RID_RTS_DEVICE_DEPTH_BOX ) )
{
    FreeResource();

    m_aPPDKeyBox.SetSelectHdl( LINK( this, RTSDevicePage, SelectHdl ) );
    m_aPPDValueBox.SetSelectHdl( LINK( this, RTSDevicePage, SelectHdl ) );

    m_aSpaceBox.InsertEntry( m_pParent->m_aFromDriverString );
    m_aSpaceBox.InsertEntry( m_aSpaceColor );
    m_aSpaceBox.InsertEntry( m_aSpaceGray );
    switch( m_pParent->m_aJobData.m_nColorDevice )
    {
        case -1: m_aSpaceBox.SelectEntry( m_aSpaceGray );break;
        case  0: m_aSpaceBox.SelectEntry( m_pParent->m_aFromDriverString );break;
        case  1: m_aSpaceBox.SelectEntry( m_aSpaceColor );break;
    }

    sal_uLong nLevelEntryData = 0;
    if( m_pParent->m_aJobData.m_nPDFDevice > 0 )
        nLevelEntryData = 10;
    else
        nLevelEntryData = m_pParent->m_aJobData.m_nPSLevel+1;
    for( sal_uInt16 i = 0; i < m_aLevelBox.GetEntryCount(); i++ )
    {
        if( (sal_uLong)m_aLevelBox.GetEntryData( i ) == nLevelEntryData )
        {
            m_aLevelBox.SelectEntryPos( i );
            break;
        }
    }

    m_aDepthBox.SelectEntry( String::CreateFromInt32( m_pParent->m_aJobData.m_nColorDepth ).AppendAscii( " Bit" ) );

    // fill ppd boxes
    if( m_pParent->m_aJobData.m_pParser )
    {
        for( int i = 0; i < m_pParent->m_aJobData.m_pParser->getKeys(); i++ )
        {
            const PPDKey* pKey = m_pParent->m_aJobData.m_pParser->getKey( i );
            if( pKey->isUIKey()                                 &&
                ! pKey->getKey().EqualsAscii( "PageSize" )      &&
                ! pKey->getKey().EqualsAscii( "InputSlot" )     &&
                ! pKey->getKey().EqualsAscii( "PageRegion" )    &&
                ! pKey->getKey().EqualsAscii( "Duplex" )
                )
            {
                String aEntry( m_pParent->m_aJobData.m_pParser->translateKey( pKey->getKey() ) );
                sal_uInt16 nPos = m_aPPDKeyBox.InsertEntry( aEntry );
                m_aPPDKeyBox.SetEntryData( nPos, (void*)pKey );
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

// ------------------------------------------------------------------

sal_uLong RTSDevicePage::getLevel()
{
    sal_uLong nLevel = (sal_uLong)m_aLevelBox.GetEntryData( m_aLevelBox.GetSelectEntryPos() );
    return nLevel < 10 ? nLevel-1 : 0;
}

// ------------------------------------------------------------------

sal_uLong RTSDevicePage::getPDFDevice()
{
    sal_uLong nLevel = (sal_uLong)m_aLevelBox.GetEntryData( m_aLevelBox.GetSelectEntryPos() );
    return nLevel > 9 ? 1 : 0;
}

// ------------------------------------------------------------------

IMPL_LINK( RTSDevicePage, SelectHdl, ListBox*, pBox )
{
    if( pBox == &m_aPPDKeyBox )
    {
        const PPDKey* pKey = (PPDKey*)m_aPPDKeyBox.GetEntryData( m_aPPDKeyBox.GetSelectEntryPos() );
        FillValueBox( pKey );
    }
    else if( pBox == &m_aPPDValueBox )
    {
        const PPDKey* pKey = (PPDKey*)m_aPPDKeyBox.GetEntryData( m_aPPDKeyBox.GetSelectEntryPos() );
        const PPDValue* pValue = (PPDValue*)m_aPPDValueBox.GetEntryData( m_aPPDValueBox.GetSelectEntryPos() );
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
    m_aPPDValueBox.Clear();

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
            sal_uInt16 nPos = m_aPPDValueBox.InsertEntry( aEntry );
            m_aPPDValueBox.SetEntryData( nPos, (void*)pValue );
        }
    }
    pValue = m_pParent->m_aJobData.m_aContext.getValue( pKey );
    m_aPPDValueBox.SelectEntryPos( m_aPPDValueBox.GetEntryPos( (void*)pValue ) );
}

// --------------------------------------------------------------------------

/*
 * RTSOtherPage
 */

RTSOtherPage::RTSOtherPage( RTSDialog* pParent ) :
        TabPage( &pParent->m_aTabControl, PaResId( RID_RTS_OTHERPAGE ) ),
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
        TabPage( &pParent->m_aTabControl, PaResId( RID_RTS_FONTSUBSTPAGE ) ),
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
    rFontManager.getFontListWithFastInfo( aFonts, m_pParent->m_aJobData.m_pParser, false );
    ::std::list< FastPrintFontInfo >::const_iterator it;
    ::std::hash_map< OUString, int, OUStringHash > aToMap, aFromMap;
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
    ::std::hash_map< OUString, OUString, OUStringHash >::const_iterator it;
    for( it = m_pParent->m_aJobData.m_aFontSubstitutes.begin();
         it != m_pParent->m_aJobData.m_aFontSubstitutes.end(); ++it )
    {
        String aEntry( it->first );
        aEntry.AppendAscii( " -> " );
        aEntry.Append( String( it->second ) );
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
    aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), OStringToOUString( rServer, osl_getThreadTextEncoding() ) );
    m_aText.SetText( aText );
    m_aUserEdit.SetText( OStringToOUString( rUserName, osl_getThreadTextEncoding() ) );
}

RTSPWDialog::~RTSPWDialog()
{
}

OString RTSPWDialog::getUserName() const
{
    return rtl::OUStringToOString( m_aUserEdit.GetText(), osl_getThreadTextEncoding() );
}

OString RTSPWDialog::getPassword() const
{
    return rtl::OUStringToOString( m_aPassEdit.GetText(), osl_getThreadTextEncoding() );
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
