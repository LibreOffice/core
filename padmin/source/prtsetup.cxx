/*************************************************************************
 *
 *  $RCSfile: prtsetup.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2001-06-15 15:30:08 $
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

#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PAD_PRTSETUP_HXX_
#include <prtsetup.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx> // for PaResId
#endif
#ifndef _PAD_RTSETUP_HRC_
#include <rtsetup.hrc>
#endif
#ifndef _PAD_COMMANDDLG_HXX_
#include <cmddlg.hxx>
#endif

#define LSCAPE_STRING String( RTL_CONSTASCII_USTRINGPARAM( "Landscape" ) )
#define PORTRAIT_STRING String( RTL_CONSTASCII_USTRINGPARAM( "Portrait" ) )

using namespace rtl;
using namespace psp;
using namespace padmin;

void RTSDialog::insertAllPPDValues( ListBox& rBox, const PPDKey* pKey )
{
    if( ! pKey )
        return;

    const PPDValue* pValue = NULL;
    USHORT nPos = 0;
    String aOptionText;

    for( int i = 0; i < pKey->countValues(); i++ )
    {
        pValue = pKey->getValue( i );
        aOptionText = pValue->m_aOptionTranslation.Len() ? pValue->m_aOptionTranslation : pValue->m_aOption;

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
        m_aTabControl( this, PaResId( RID_RTS_RTSDIALOG_TABCONTROL ) ),

        m_aJobData( rJobData ),
        m_aPrinter( rPrinter ),
        m_aOKButton( this ),
        m_aCancelButton( this ),

        m_aInvalidString( PaResId( RID_RTS_RTSDIALOG_INVALID_TXT ) ),
        m_aFromDriverString( PaResId( RID_RTS_RTSDIALOG_FROMDRIVER_TXT ) ),

        m_pPaperPage( NULL ),
        m_pDevicePage( NULL ),
        m_pOtherPage( NULL ),
        m_pFontSubstPage( NULL ),
        m_pCommandPage( NULL )
{
    FreeResource();
    rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();

    String aTitle( GetText() );
    aTitle.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), m_aJobData.m_aPrinterName );
    SetText( aTitle );

    if( ! bAllPages )
    {
        m_aTabControl.RemovePage( RID_RTS_OTHERPAGE );
        m_aTabControl.RemovePage( RID_RTS_FONTSUBSTPAGE );
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

String RTSDialog::getPaperSize()
{
    String aRet;
    const PPDKey* pKey = m_aJobData.m_pParser->getKey( String::CreateFromAscii( "PageSize" ) );
    if( pKey )
    {
        const PPDValue* pValue = m_aJobData.m_aContext.getValue( pKey );
        aRet = pValue->m_aOption;
    }
    return aRet;
}

// --------------------------------------------------------------------------

IMPL_LINK( RTSDialog, ActivatePage, TabControl*, pTabCtrl )
{
    if( pTabCtrl != &m_aTabControl )
        return 0;

    USHORT nId = m_aTabControl.GetCurPageId();

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
            // scale
            m_aJobData.m_nScale = m_pPaperPage->getScale();
            // orientation
            m_aJobData.m_eOrientation = m_pPaperPage->getOrientation().Equals( LSCAPE_STRING ) ? orientation::Landscape : orientation::Portrait;
        }
        if( m_pDevicePage )
        {
            m_aJobData.m_nColorDepth    = m_pDevicePage->getDepth();
            m_aJobData.m_nColorDevice   = m_pDevicePage->getColorDevice();
            m_aJobData.m_nPSLevel       = m_pDevicePage->getLevel();
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
        m_aSlotBox( this, PaResId( RID_RTS_PAPER_SLOT_BOX ) ),
        m_aScaleText( this, PaResId( RID_RTS_PAPER_SCALE_TXT ) ),
        m_aScaleBox( this, PaResId( RID_RTS_PAPER_SCALE_BOX ) )
{
    m_aPaperBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aOrientBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aDuplexBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aSlotBox.SetSelectHdl( LINK( this, RTSPaperPage, SelectHdl ) );
    m_aScaleBox.SetMin( 1, FUNIT_PERCENT );
    m_aScaleBox.SetMax( 10000, FUNIT_PERCENT );
    m_aScaleBox.SetUnit( FUNIT_PERCENT );

    FreeResource();

    USHORT nPos = 0;

    m_aOrientBox.InsertEntry( PORTRAIT_STRING );
    m_aOrientBox.InsertEntry( LSCAPE_STRING );
    // duplex
    nPos = m_aDuplexBox.InsertEntry( m_pParent->m_aInvalidString );
    m_aDuplexBox.SetEntryData( nPos, NULL );

    // paper does not have an invalid entry

    // input slots
    nPos = m_aSlotBox.InsertEntry( m_pParent->m_aInvalidString );
    m_aSlotBox.SetEntryData( nPos, NULL );

    // scale
    m_aScaleBox.SetValue( m_pParent->m_aJobData.m_nScale );

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
    if( pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) ) )
        m_pParent->insertAllPPDValues( m_aDuplexBox, pKey );
    else
        m_aDuplexBox.Enable( FALSE );

    // paper
    if( pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) ) )
        m_pParent->insertAllPPDValues( m_aPaperBox, pKey );
    else
        m_aPaperBox.Enable( FALSE );

    // input slots
    if( pKey = m_pParent->m_aJobData.m_pParser->getKey( String::CreateFromAscii( "InputSlot" ) ) )
        m_pParent->insertAllPPDValues( m_aSlotBox, pKey );
    else
        m_aSlotBox.Enable( FALSE );
}

// --------------------------------------------------------------------------

IMPL_LINK( RTSPaperPage, SelectHdl, ListBox*, pBox )
{
    const PPDKey* pKey = NULL;
    if( pBox == &m_aPaperBox )
        pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
    else if( pBox == &m_aDuplexBox )
        pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
    else if( pBox == &m_aSlotBox )
        pKey = m_pParent->m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
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
        m_aLevelText( this, PaResId( RID_RTS_DEVICE_LEVEL_TXT ) ),
        m_aLevelBox( this, PaResId( RID_RTS_DEVICE_LEVEL_BOX ) ),
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

    m_aLevelBox.InsertEntry( m_pParent->m_aFromDriverString );
    m_aLevelBox.InsertEntry( String( RTL_CONSTASCII_USTRINGPARAM( "1" ) ) );
    m_aLevelBox.InsertEntry( String( RTL_CONSTASCII_USTRINGPARAM( "2" ) ) );
    if( m_pParent->m_aJobData.m_nPSLevel == 0 )
        m_aLevelBox.SelectEntry( m_pParent->m_aFromDriverString );
    else
        m_aLevelBox.SelectEntry( String::CreateFromInt32( m_pParent->m_aJobData.m_nPSLevel ) );

    m_aDepthBox.SelectEntry( String::CreateFromInt32( m_pParent->m_aJobData.m_nColorDepth ).AppendAscii( " Bit" ) );

    // fill ppd boxes
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
            USHORT nPos =
                m_aPPDKeyBox.InsertEntry( pKey->getUITranslation().Len() ? pKey->getUITranslation() : pKey->getKey() );
            m_aPPDKeyBox.SetEntryData( nPos, (void*)pKey );
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
        if( m_pParent->m_aJobData.m_aContext.checkConstraints( pKey, pValue ) )
        {
            USHORT nPos =
                m_aPPDValueBox.InsertEntry( pValue->m_aOptionTranslation.Len() ? pValue->m_aOptionTranslation : pValue->m_aOption );
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
        m_aDefaultBtn( this, PaResId( RID_RTS_OTHER_DEFAULT_BTN ) ),
        m_aLeftLB( this, PaResId( RID_RTS_OTHER_LEFTMARGIN_BOX ) ),
        m_aRightLB( this, PaResId( RID_RTS_OTHER_RIGHTMARGIN_BOX ) ),
        m_aTopLB( this, PaResId( RID_RTS_OTHER_TOPMARGIN_BOX ) ),
        m_aBottomLB( this, PaResId( RID_RTS_OTHER_BOTTOMMARGIN_BOX ) ),
        m_aLeftTxt( this, PaResId( RID_RTS_OTHER_LEFTMARGIN_TXT ) ),
        m_aRightTxt( this, PaResId( RID_RTS_OTHER_RIGHTMARGIN_TXT ) ),
        m_aTopTxt( this, PaResId( RID_RTS_OTHER_TOPMARGIN_TXT ) ),
        m_aBottomTxt( this, PaResId( RID_RTS_OTHER_BOTTOMMARGIN_TXT ) ),
        m_aCommentTxt( this, PaResId( RID_RTS_OTHER_COMMENT_TXT ) ),
        m_aCommentEdt( this, PaResId( RID_RTS_OTHER_COMMENT_EDT ) )
{
    FreeResource();

    m_aTopLB.EnableEmptyFieldValue( TRUE );
    m_aBottomLB.EnableEmptyFieldValue( TRUE );
    m_aLeftLB.EnableEmptyFieldValue( TRUE );
    m_aRightLB.EnableEmptyFieldValue( TRUE );

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
    m_nMarginLeft   = m_pParent->m_aJobData.m_nLeftMarginAdjust;
    m_nMarginRight  = m_pParent->m_aJobData.m_nRightMarginAdjust;
    m_nMarginTop    = m_pParent->m_aJobData.m_nTopMarginAdjust;
    m_nMarginBottom = m_pParent->m_aJobData.m_nBottomMarginAdjust;

    m_aLeftLB.SetValue( m_nMarginLeft );
    m_aRightLB.SetValue( m_nMarginRight );
    m_aTopLB.SetValue( m_nMarginTop );
    m_aBottomLB.SetValue( m_nMarginBottom );
    m_aCommentEdt.SetText( m_pParent->m_aJobData.m_aComment );
}

// ------------------------------------------------------------------

void RTSOtherPage::save()
{
    m_nMarginLeft   = m_pParent->m_aJobData.m_nLeftMarginAdjust     = m_aLeftLB.GetValue();
    m_nMarginRight  = m_pParent->m_aJobData.m_nRightMarginAdjust    = m_aRightLB.GetValue();
    m_nMarginTop    = m_pParent->m_aJobData.m_nTopMarginAdjust      = m_aTopLB.GetValue();
    m_nMarginBottom = m_pParent->m_aJobData.m_nBottomMarginAdjust   = m_aBottomLB.GetValue();
    m_pParent->m_aJobData.m_aComment = m_aCommentEdt.GetText();
}

// ------------------------------------------------------------------

IMPL_LINK( RTSOtherPage, ClickBtnHdl, Button*, pButton )
{
    if( pButton == &m_aDefaultBtn )
    {
        m_nMarginBottom = m_nMarginTop = m_nMarginRight = m_nMarginLeft = 0;
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
    rFontManager.getFontListWithFastInfo( aFonts, m_pParent->m_aJobData.m_pParser );
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
    m_aRemoveButton.Enable( FALSE );
    if( ! m_pParent->m_aJobData.m_bPerformFontSubstitution )
    {
        m_aSubstitutionsBox.Enable( FALSE );
        m_aSubstitutionsText.Enable( FALSE );
        m_aAddButton.Enable( FALSE );
        m_aToFontBox.Enable( FALSE );
        m_aToFontText.Enable( FALSE );
        m_aFromFontBox.Enable( FALSE );
        m_aFromFontText.Enable( FALSE );
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
    m_aRemoveButton.Enable( FALSE );
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
            USHORT nPos = aEntry.SearchAscii( " -> " );
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



extern "C" {

    int Sal_SetupPrinterDriver( ::psp::PrinterInfo& rJobData )
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

    int Sal_queryFaxNumber( String& rNumber )
    {
        QueryString aQuery( NULL, String( PaResId( RID_TXT_QUERYFAXNUMBER ) ), rNumber );
        return aQuery.Execute();
    }

} // extern "C"
