/*************************************************************************
 *
 *  $RCSfile: headerfooterdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:59:18 $
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

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#define ITEMID_LANGUAGE EE_CHAR_LANGUAGE
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
#include <svx/svdotext.hxx>
#include <svx/editeng.hxx>

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>

#include "helpids.h"
#include "Outliner.hxx"
#include "dialogs.hrc"
#include "headerfooterdlg.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"

///////////////////////////////////////////////////////////////////////
// preview control for presentation layout
///////////////////////////////////////////////////////////////////////

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif

#include "undoheaderfooter.hxx"
#include "sdundogr.hxx"
#include "ViewShell.hxx"

namespace sd
{

class PresLayoutPreview : public Control
{
private:
    SdPage* mpMaster;
    HeaderFooterSettings maSettings;
    Size maPageSize;
    Rectangle maOutRect;
    double mdScaleX;
    double mdScaleY;

private:
    void Paint( XOutputDevice& aXOut, SdrTextObj* pObj, bool bVisible, bool bDottet = false );

public:
    PresLayoutPreview( ::Window* pParent, const ResId& rResId, SdPage* pMaster );
    ~PresLayoutPreview();

    virtual void Paint( const Rectangle& rRect );

    void update( HeaderFooterSettings& rSettings );
};

}

///////////////////////////////////////////////////////////////////////
// tab page for slide & header'n'notes
///////////////////////////////////////////////////////////////////////

namespace sd
{

const int nDateTimeFormatsCount = 10;
int nDateTimeFormats[nDateTimeFormatsCount] =
{
    SVXDATEFORMAT_A,
    SVXDATEFORMAT_F,
    SVXDATEFORMAT_D,
    SVXDATEFORMAT_C,

    SVXDATEFORMAT_A | (SVXTIMEFORMAT_24_HM << 4),
    SVXDATEFORMAT_A | (SVXTIMEFORMAT_12_HM << 4),

    (SVXTIMEFORMAT_24_HM << 4),
    (SVXTIMEFORMAT_24_HMS <<4),

    (SVXTIMEFORMAT_12_HM << 4 ),
    (SVXTIMEFORMAT_12_HMS << 4 )
};

class HeaderFooterTabPage : public TabPage
{
private:
    FixedLine   maFLIncludeOnPage;

    CheckBox    maCBHeader;
    FixedText   maFTHeader;
    Edit        maTBHeader;

    FixedLine   maFLDateTime;
    CheckBox    maCBDateTime;
    RadioButton maRBDateTimeFixed;
    RadioButton maRBDateTimeAutomatic;
    Edit        maTBDateTimeFixed;
    ListBox     maCBDateTimeFormat;
    FixedText   maFTDateTimeLanguage;
    SvxLanguageBox  maCBDateTimeLanguage;

    FixedLine   maFLFooter;
    CheckBox    maCBFooter;
    FixedText   maFTFooter;
    Edit        maTBFooter;

    FixedLine   maFLSlideNumber;
    CheckBox    maCBSlideNumber;

    FixedLine   maFLNotOnTitle;
    CheckBox    maCBNotOnTitle;

    PushButton      maPBApplyToAll;
    PushButton      maPBApply;
    CancelButton    maPBCancel;
    HelpButton      maPBHelp;

    PresLayoutPreview   maCTPreview;

    SdPage*             mpCurrentPage;
    SdDrawDocument *    mpDoc;
    HeaderFooterDialog* mpDialog;
    LanguageType        meOldLanguage;

    bool            mbHandoutMode;

    DECL_LINK( UpdateOnClickHdl, void * );

    DECL_LINK( ClickApplyToAllHdl, void * );
    DECL_LINK( ClickApplyHdl, void * );
    DECL_LINK( ClickCancelHdl, void * );

    DECL_LINK( LanguageChangeHdl, void * );

    void FillFormatList(int eFormat);
    void GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet );
    void GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet, SdPage* pPage );

public:
    HeaderFooterTabPage( HeaderFooterDialog* pDialog, ::Window* pParent, SdDrawDocument* pDoc, SdPage* pActualPage, bool bHandoutMode );
    ~HeaderFooterTabPage();

    static  SfxTabPage* Create( ::Window*, const SfxItemSet& );
    static  USHORT*    GetRanges();

    void    init( const HeaderFooterSettings& rSettings, bool bNotOnTitle, bool bHasApply );
    void    getData( HeaderFooterSettings& rSettings, bool& rNotOnTitle );
    void    update();
};

}

///////////////////////////////////////////////////////////////////////

using namespace ::sd;


HeaderFooterDialog::HeaderFooterDialog( ViewShell* pViewShell, ::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage ) :
        TabDialog ( pParent, SdResId( RID_SD_DLG_HEADERFOOTER ) ),
        maTabCtrl( this, SdResId( 1 ) ),
        mpDoc( pDoc ),
        mpCurrentPage( pCurrentPage ),
        mpViewShell( pViewShell )
{
    FreeResource();

    SdPage* pSlide;
    SdPage* pNotes;
    if( pCurrentPage->GetPageKind() == PK_STANDARD )
    {
        pSlide = pCurrentPage;
        pNotes = (SdPage*)pDoc->GetPage( pCurrentPage->GetPageNum() + 1 );
    }
    else if( pCurrentPage->GetPageKind() == PK_NOTES )
    {
        pNotes = pCurrentPage;
        pSlide = (SdPage*)pDoc->GetPage( pCurrentPage->GetPageNum() -1 );
        mpCurrentPage = pSlide;
    }
    else
    {
        // handout
        pSlide = pDoc->GetSdPage( 0, PK_STANDARD );
        pNotes = pDoc->GetSdPage( 0, PK_NOTES );
        mpCurrentPage = NULL;
    }

//  maTabCtrl.SetHelpId( HID_XML_FILTER_TABPAGE_CTRL );
    maTabCtrl.Show();

    mpSlideTabPage = new HeaderFooterTabPage( this, &maTabCtrl, pDoc, pSlide, false );
    mpSlideTabPage->SetHelpId( HID_SD_TABPAGE_HEADERFOOTER_SLIDE );
    maTabCtrl.SetTabPage( RID_SD_TABPAGE_HEADERFOOTER_SLIDE, mpSlideTabPage );

    Size aSiz = mpSlideTabPage->GetSizePixel();
    Size aCtrlSiz = maTabCtrl.GetOutputSizePixel();
    // set size on TabControl only if smaller than TabPage
    if ( aCtrlSiz.Width() < aSiz.Width() || aCtrlSiz.Height() < aSiz.Height() )
    {
        maTabCtrl.SetOutputSizePixel( aSiz );
        aCtrlSiz = aSiz;
    }

    mpNotesHandoutsTabPage = new HeaderFooterTabPage( this, &maTabCtrl, pDoc, pNotes, true );
    mpNotesHandoutsTabPage->SetHelpId( HID_SD_TABPAGE_HEADERFOOTER_NOTESHANDOUT );
    maTabCtrl.SetTabPage( RID_SD_TABPAGE_HEADERFOOTER_NOTESHANDOUT, mpNotesHandoutsTabPage );

    aSiz = mpNotesHandoutsTabPage->GetSizePixel();
    if ( aCtrlSiz.Width() < aSiz.Width() || aCtrlSiz.Height() < aSiz.Height() )
    {
        maTabCtrl.SetOutputSizePixel( aSiz );
        aCtrlSiz = aSiz;
    }

    ActivatePageHdl( &maTabCtrl );

    AdjustLayout();

    maTabCtrl.SetActivatePageHdl( LINK( this, HeaderFooterDialog, ActivatePageHdl ) );
    maTabCtrl.SetDeactivatePageHdl( LINK( this, HeaderFooterDialog, DeactivatePageHdl ) );

    maSlideSettings = pSlide->getHeaderFooterSettings();

    const HeaderFooterSettings& rTitleSettings = mpDoc->GetSdPage(0, PK_STANDARD)->getHeaderFooterSettings();
    bool bNotOnTitle = !rTitleSettings.mbFooterVisible && !rTitleSettings.mbSlideNumberVisible && !rTitleSettings.mbDateTimeVisible;

    mpSlideTabPage->init( maSlideSettings, bNotOnTitle, mpCurrentPage != NULL );

    maNotesHandoutSettings = pNotes->getHeaderFooterSettings();
    mpNotesHandoutsTabPage->init( maNotesHandoutSettings, false, false );
}

// -----------------------------------------------------------------------

HeaderFooterDialog::~HeaderFooterDialog()
{
    delete mpSlideTabPage;
    delete mpNotesHandoutsTabPage;
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::initTabPages()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    const USHORT nId = pTabCtrl->GetCurPageId();
    TabPage* pTabPage = pTabCtrl->GetTabPage( nId );
    pTabPage->Show();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterDialog, DeactivatePageHdl, TabControl *, pTabCtrl )
{
    return TRUE;
}

// -----------------------------------------------------------------------

short HeaderFooterDialog::Execute()
{
    USHORT nRet = TabDialog::Execute();
    if( nRet )
        mpViewShell->GetDocSh()->SetModified();
    return nRet;
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::ApplyToAll( TabPage* pPage )
{
    apply( true, pPage == mpSlideTabPage );
    EndDialog(1);
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::Apply( TabPage* pPage )
{
    apply( false, pPage == mpSlideTabPage );
    EndDialog(1);
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::Cancel( TabPage* pPage )
{
    EndDialog();
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::apply( bool bToAll, bool bForceSlides )
{
    SdUndoGroup* pUndoGroup = new SdUndoGroup(mpDoc);
    String aComment( GetText() );
    pUndoGroup->SetComment( aComment );

    HeaderFooterSettings aNewSettings;
    bool bNewNotOnTitle;

    // change slide settings first ...

    mpSlideTabPage->getData( aNewSettings, bNewNotOnTitle );

    // only if we pressed apply or apply all on the slide tab page or if the slide settings
    // have been changed
    if( bForceSlides || !(aNewSettings == maSlideSettings) )
    {
        // apply to all slides
        if( bToAll )
        {
            int nPageCount = mpDoc->GetSdPageCount( PK_STANDARD );
            int nPage;
            for( nPage = 0; nPage < nPageCount; nPage++ )
            {
                SdPage* pPage = mpDoc->GetSdPage( nPage, PK_STANDARD );
                change( pUndoGroup, pPage, aNewSettings );
            }
        }
        else
        {
            // apply only to the current slide
            DBG_ASSERT( mpCurrentPage && mpCurrentPage->GetPageKind() == PK_STANDARD, "no current page to apply to!" );
            if( mpCurrentPage && (mpCurrentPage->GetPageKind() == PK_STANDARD) )
            {
                change( pUndoGroup, mpCurrentPage, aNewSettings );
            }
        }
    }

    // if we don't want to have header&footer on the first slide
    if( bNewNotOnTitle )
    {
        // just hide them, plain simple UI feature
        HeaderFooterSettings aTempSettings = mpDoc->GetSdPage( 0, PK_STANDARD )->getHeaderFooterSettings();

        aTempSettings.mbFooterVisible = false;
        aTempSettings.mbSlideNumberVisible = false;
        aTempSettings.mbDateTimeVisible = false;

        change( pUndoGroup, mpDoc->GetSdPage( 0, PK_STANDARD ), aTempSettings );
    }

    // now notes settings

    mpNotesHandoutsTabPage->getData( aNewSettings, bNewNotOnTitle );

    // only if we pressed apply or apply all on the notes tab page or if the notes settings
    // have been changed
    if( !bForceSlides || !(aNewSettings == maNotesHandoutSettings) )
    {
        // first set to all notes pages
        int nPageCount = mpDoc->GetSdPageCount( PK_NOTES );
        int nPage;
        for( nPage = 0; nPage < nPageCount; nPage++ )
        {
            SdPage* pPage = mpDoc->GetSdPage( nPage, PK_NOTES );

            change( pUndoGroup, pPage, aNewSettings );
        }

        // and last but not least to the handout page
        change( pUndoGroup, mpDoc->GetMasterSdPage( 0, PK_HANDOUT ), aNewSettings );
    }

    // Undo Gruppe dem Undo Manager uebergeben
    mpViewShell->GetViewFrame()->GetObjectShell()->GetUndoManager()->AddUndoAction(pUndoGroup);
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings )
{
    pUndoGroup->AddAction(new SdHeaderFooterUndoAction(mpDoc, pPage, rNewSettings ));
    pPage->getHeaderFooterSettings() = rNewSettings;
}

///////////////////////////////////////////////////////////////////////

inline void moveY( ::Window& rWin, int deltaY )
{
    Point aPos = rWin.GetPosPixel();
    aPos.Y() += deltaY;
    rWin.SetPosPixel( aPos );
}

HeaderFooterTabPage::HeaderFooterTabPage( HeaderFooterDialog* pDialog, ::Window* pWindow, SdDrawDocument* pDoc, SdPage* pActualPage, bool bHandoutMode ) :
        TabPage( pWindow, SdResId( RID_SD_TABPAGE_HEADERFOOTER ) ),
        mbHandoutMode( bHandoutMode ),
        maPBApplyToAll( this, SdResId( BT_APPLYTOALL ) ),
        maPBApply( this, SdResId( BT_APPLY ) ),
        maPBCancel( this, SdResId( BT_CANCEL ) ),
        maPBHelp( this, SdResId( BT_HELP ) ),
        maFLIncludeOnPage( this, SdResId( FL_INCLUDE_ON_PAGE ) ),
        maCBHeader( this, SdResId( CB_HEADER ) ),
        maFTHeader( this, SdResId( FT_HEADER ) ),
        maTBHeader( this, SdResId( TB_HEADER_FIXED ) ),
        maFLDateTime( this, SdResId( FL_DATETIME ) ),
        maCBDateTime( this, SdResId( CB_DATETIME ) ),
        maRBDateTimeFixed( this, SdResId( RB_DATETIME_FIXED ) ),
        maRBDateTimeAutomatic( this, SdResId( RB_DATETIME_AUTOMATIC ) ),
        maTBDateTimeFixed( this, SdResId( TB_DATETIME_FIXED ) ),
        maCBDateTimeFormat( this, SdResId( CB_DATETIME_FORMAT ) ),
        maFTDateTimeLanguage( this, SdResId( FT_DATETIME_LANGUAGE ) ),
        maCBDateTimeLanguage( this, SdResId( CB_DATETIME_LANGUAGE ) ),
        maFLFooter( this, SdResId( FL_FOOTER ) ),
        maCBFooter( this, SdResId( CB_FOOTER ) ),
        maFTFooter( this, SdResId( FT_FOOTER ) ),
        maTBFooter( this, SdResId( TB_FOOTER_FIXED ) ),
        maFLSlideNumber( this, SdResId( FL_SLIDENUMBER ) ),
        maCBSlideNumber( this, SdResId( CB_SLIDENUMBER ) ),
        maFLNotOnTitle( this, SdResId( FL_NOTONTITLE ) ),
        maCBNotOnTitle( this, SdResId( CB_NOTONTITLE ) ),
        maCTPreview( this, SdResId( CT_PREVIEW ),
            pActualPage ?
                (pActualPage->IsMasterPage() ? pActualPage : (SdPage*)(&(pActualPage->TRG_GetMasterPage()))) :
                (pDoc->GetMasterSdPage( 0, bHandoutMode ? PK_NOTES : PK_STANDARD )) ),
        mpCurrentPage(pActualPage),
        mpDoc(pDoc),
        mpDialog(pDialog)
{
    pDoc->StopWorkStartupDelay();

    if( !mbHandoutMode )
    {
        int deltaY = maCBHeader.GetPosPixel().Y() - maCBDateTime.GetPosPixel().Y();

        moveY( maCBDateTime, deltaY );
        moveY( maRBDateTimeFixed, deltaY );
        moveY( maRBDateTimeAutomatic, deltaY );
        moveY( maTBDateTimeFixed, deltaY );
        moveY( maCBDateTimeFormat, deltaY );
        moveY( maFTDateTimeLanguage, deltaY );
        moveY( maCBDateTimeLanguage, deltaY );
        moveY( maFLFooter, deltaY );
        moveY( maCBFooter, deltaY );
        moveY( maFTFooter, deltaY );
        moveY( maTBFooter, deltaY );
        moveY( maFLSlideNumber, deltaY );
        moveY( maCBSlideNumber, deltaY );
        moveY( maFLNotOnTitle, deltaY );
        moveY( maCBNotOnTitle, deltaY );
    }
    else
    {
        int deltaY = maPBApply.GetPosPixel().Y() - maPBCancel.GetPosPixel().Y();

        moveY( maPBCancel, deltaY );
        moveY( maPBHelp, deltaY );

        String aPageNumberStr( SdResId( STR_PAGE_NUMBER ) );
        maCBSlideNumber.SetText( aPageNumberStr );

        String aIncludeOnPageStr( SdResId( STR_INCLUDE_ON_PAGE ) );
        maCBNotOnTitle.SetText( aIncludeOnPageStr );
    }

    FreeResource();

    maPBApply.Show( !mbHandoutMode );
    maCBHeader.Show( mbHandoutMode );
    maFTHeader.Show( mbHandoutMode );
    maTBHeader.Show( mbHandoutMode );
    maFLDateTime.Show( mbHandoutMode );
    maFLNotOnTitle.Show( !mbHandoutMode );
    maCBNotOnTitle.Show( !mbHandoutMode );

    maCBDateTime.SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    maRBDateTimeFixed.SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    maRBDateTimeAutomatic.SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    maCBFooter.SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    maCBHeader.SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    maCBSlideNumber.SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );

    maPBApplyToAll.SetClickHdl( LINK( this, HeaderFooterTabPage, ClickApplyToAllHdl ) );
    maPBApply.SetClickHdl( LINK( this, HeaderFooterTabPage, ClickApplyHdl ) );
    maPBCancel.SetClickHdl( LINK( this, HeaderFooterTabPage, ClickCancelHdl ) );

    maCBDateTimeLanguage.SetLanguageList( LANG_LIST_ALL|LANG_LIST_ONLY_KNOWN, false );
    maCBDateTimeLanguage.SetSelectHdl( LINK( this, HeaderFooterTabPage, LanguageChangeHdl ) );

    GetOrSetDateTimeLanguage( meOldLanguage, false );
    meOldLanguage = International::GetRealLanguage( meOldLanguage );
    maCBDateTimeLanguage.SelectLanguage( meOldLanguage );

    FillFormatList(SVXDATEFORMAT_A);

}
// -----------------------------------------------------------------------

HeaderFooterTabPage::~HeaderFooterTabPage()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterTabPage, LanguageChangeHdl, void *, EMPTYARG )
{
    FillFormatList( (int)maCBDateTimeFormat.GetEntryData( maCBDateTimeFormat.GetSelectEntryPos() ) );


    return 0L;
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::FillFormatList( int eFormat )
{
    LanguageType eLanguage = maCBDateTimeLanguage.GetSelectLanguage();

    maCBDateTimeFormat.Clear();

    Date aDate;
    Time aTime;

    int nFormat;
    for( nFormat = 0; nFormat < nDateTimeFormatsCount; nFormat++ )
    {
        String aStr( SvxDateTimeField::GetFormatted( aDate, aTime, nDateTimeFormats[nFormat], *(SD_MOD()->GetNumberFormatter()), eLanguage ) );
        USHORT nEntry = maCBDateTimeFormat.InsertEntry( aStr );
        maCBDateTimeFormat.SetEntryData( nEntry, (void*)nDateTimeFormats[nFormat] );
        if( nDateTimeFormats[nFormat] == eFormat )
        {
            maCBDateTimeFormat.SelectEntryPos( nEntry );
            maCBDateTimeFormat.SetText( aStr );
        }
    }
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::init( const HeaderFooterSettings& rSettings, bool bNotOnTitle, bool bHasApply )
{
    maCBDateTime.Check( rSettings.mbDateTimeVisible );
    maRBDateTimeFixed.Check( rSettings.mbDateTimeIsFixed );
    maRBDateTimeAutomatic.Check( !rSettings.mbDateTimeIsFixed );
    maTBDateTimeFixed.SetText( rSettings.maDateTimeText );

    maCBHeader.Check( rSettings.mbHeaderVisible );
    maTBHeader.SetText( rSettings.maHeaderText );

    maCBFooter.Check( rSettings.mbFooterVisible );
    maTBFooter.SetText( rSettings.maFooterText );

    maCBSlideNumber.Check( rSettings.mbSlideNumberVisible );

    maCBNotOnTitle.Check( bNotOnTitle );
    maPBApply.Enable( bHasApply );

    maCBDateTimeLanguage.SelectLanguage( meOldLanguage );

    USHORT nPos;
    for( nPos = 0; nPos < maCBDateTimeFormat.GetEntryCount(); nPos++ )
    {
        int nFormat = (int)maCBDateTimeFormat.GetEntryData( nPos );
        if( nFormat == rSettings.meDateTimeFormat )
        {
            maCBDateTimeFormat.SelectEntryPos( nPos );
            maCBDateTimeFormat.SetText( maCBDateTimeFormat.GetEntry(nPos) );
            break;
        }
    }

    update();
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::getData( HeaderFooterSettings& rSettings, bool& rNotOnTitle )
{
    rSettings.mbDateTimeVisible = maCBDateTime.IsChecked();
    rSettings.mbDateTimeIsFixed = maRBDateTimeFixed.IsChecked();
    rSettings.maDateTimeText = maTBDateTimeFixed.GetText();
    rSettings.mbFooterVisible = maCBFooter.IsChecked();
    rSettings.maFooterText = maTBFooter.GetText();
    rSettings.mbSlideNumberVisible = maCBSlideNumber.IsChecked();
    rSettings.mbHeaderVisible = maCBHeader.IsChecked();
    rSettings.maHeaderText = maTBHeader.GetText();

    if( maCBDateTimeFormat.GetSelectEntryCount() == 1 )
        rSettings.meDateTimeFormat = (int)maCBDateTimeFormat.GetEntryData( maCBDateTimeFormat.GetSelectEntryPos() );

    LanguageType eLanguage = maCBDateTimeLanguage.GetSelectLanguage();
    if( eLanguage != meOldLanguage )
        GetOrSetDateTimeLanguage( eLanguage, true );

    rNotOnTitle = maCBNotOnTitle.IsChecked();
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::update()
{
    maRBDateTimeFixed.Enable( maCBDateTime.IsChecked() );
    maTBDateTimeFixed.Enable( maRBDateTimeFixed.IsChecked() && maCBDateTime.IsChecked() );
    maRBDateTimeAutomatic.Enable( maCBDateTime.IsChecked() );
    maCBDateTimeFormat.Enable( maCBDateTime.IsChecked() && maRBDateTimeAutomatic.IsChecked() );
    maFTDateTimeLanguage.Enable( maCBDateTime.IsChecked() && maRBDateTimeAutomatic.IsChecked() );
    maCBDateTimeLanguage.Enable( maCBDateTime.IsChecked() && maRBDateTimeAutomatic.IsChecked() );

    maFTFooter.Enable( maCBFooter.IsChecked() );
    maTBFooter.Enable( maCBFooter.IsChecked() );

    maFTHeader.Enable( maCBHeader.IsChecked() );
    maTBHeader.Enable( maCBHeader.IsChecked() );

    HeaderFooterSettings aSettings;
    bool bNotOnTitle;
    getData( aSettings, bNotOnTitle );
    maCTPreview.update( aSettings );
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterTabPage, UpdateOnClickHdl, void *, EMPTYARG )
{
    update();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterTabPage, ClickApplyToAllHdl, void *, EMPTYARG )
{
    mpDialog->ApplyToAll( this );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterTabPage, ClickApplyHdl, void *, EMPTYARG )
{
    mpDialog->Apply( this );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( HeaderFooterTabPage, ClickCancelHdl, void *, EMPTYARG )
{
    mpDialog->Cancel( this );
    return 0;
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet )
{
    if( mbHandoutMode )
    {
        // if set, set it on all notes master pages
        if( bSet )
        {
            int nPageCount = mpDoc->GetMasterSdPageCount( PK_NOTES );
            int nPage;
            for( nPage = 0; nPage < nPageCount; nPage++ )
            {
                GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( nPage, PK_NOTES ) );
            }
        }

        // and set it, or just get it from the handout master page
        GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( 0, PK_HANDOUT ) );
    }
    else
    {
        // get the language from the first master page
        // or set it to all master pages
        int nPageCount = bSet ? mpDoc->GetMasterSdPageCount( PK_NOTES ) : 1;
        int nPage;
        for( nPage = 0; nPage < nPageCount; nPage++ )
        {
            GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( nPage, PK_STANDARD ) );
        }
    }
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet, SdPage* pPage )
{
    if( pPage )
    {
        SdrTextObj* pObj = (SdrTextObj*)pPage->GetPresObj( PRESOBJ_DATETIME );
        if( pObj )
        {
            Outliner* pOutl = mpDoc->GetInternalOutliner();
            pOutl->Init( OUTLINERMODE_TEXTOBJECT );
            USHORT nOutlMode = pOutl->GetMode();

            EditEngine* pEdit = const_cast< EditEngine* >(&pOutl->GetEditEngine());

            pOutl->SetText( *pObj->GetOutlinerParaObject() );

            EFieldInfo aFieldInfo;
            aFieldInfo.pFieldItem = NULL;

            USHORT nParaCount = pEdit->GetParagraphCount();
            USHORT nPara;
            for( nPara = 0; (nPara < nParaCount) && (aFieldInfo.pFieldItem == NULL); nPara++ )
            {
                USHORT nFieldCount = pEdit->GetFieldCount( nPara );
                USHORT nField;
                for( nField = 0; (nField < nFieldCount) && (aFieldInfo.pFieldItem == NULL); nField++ )
                {
                    aFieldInfo = pEdit->GetFieldInfo( nPara, nField );
                    if( aFieldInfo.pFieldItem )
                    {
                        const SvxFieldData* pFieldData = aFieldInfo.pFieldItem->GetField();
                        if( pFieldData && pFieldData->ISA( SvxDateTimeField ) )
                        {
                            break;
                        }
                    }
                    aFieldInfo.pFieldItem = NULL;
                }
            }


            if( aFieldInfo.pFieldItem != NULL )
            {
                if( bSet )
                {
                    SfxItemSet aSet( pEdit->GetAttribs( aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex, aFieldInfo.aPosition.nIndex+1, bSet ? GETATTRIBS_CHARATTRIBS : GETATTRIBS_ALL ) );

                    SvxLanguageItem aItem( rLanguage, EE_CHAR_LANGUAGE );
                    aSet.Put( aItem );

                    SvxLanguageItem aItemCJK( rLanguage, EE_CHAR_LANGUAGE_CJK );
                    aSet.Put( aItemCJK );

                    SvxLanguageItem aItemCTL( rLanguage, EE_CHAR_LANGUAGE_CTL );
                    aSet.Put( aItemCTL );

                    ESelection aSel( aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex, aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex+1 );
                    pEdit->QuickSetAttribs( aSet, aSel );

                    pObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
                    pOutl->UpdateFields();
                }
                else
                {
                    rLanguage =  pOutl->GetLanguage( aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex );
                }
            }

            pOutl->Clear();
            pOutl->Init( nOutlMode );
        }
    }
}

///////////////////////////////////////////////////////////////////////

PresLayoutPreview::PresLayoutPreview( ::Window* pParent, const ResId& rResId, SdPage* pMaster )
:Control( pParent, rResId ), mpMaster( pMaster ), maPageSize( pMaster->GetSize() )
{
}

// -----------------------------------------------------------------------

PresLayoutPreview::~PresLayoutPreview()
{
}

// -----------------------------------------------------------------------

void PresLayoutPreview::update( HeaderFooterSettings& rSettings )
{
    maSettings = rSettings;
    Invalidate();
}

// -----------------------------------------------------------------------

void PresLayoutPreview::Paint( XOutputDevice& aXOut, SdrTextObj* pObj, bool bVisible, bool bDottet /* = false*/ )
{
    Rectangle aRect( pObj->GetGeoRect() );

    aRect.nLeft     = long( aRect.nLeft * mdScaleX );
    aRect.nTop      = long( aRect.nTop * mdScaleY );
    aRect.nRight    = long( aRect.nRight * mdScaleX );
    aRect.nBottom   = long( aRect.nBottom * mdScaleY );

    aRect += maOutRect.TopLeft();

    svtools::ColorConfig aColorConfig;
    svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( bVisible ? svtools::FONTCOLOR : svtools::OBJECTBOUNDARIES ) );

    SfxItemSet aSet( pObj->GetModel()->GetItemPool() );
    String aEmpty;

    if( bDottet )
    {
        XDash aDash( XDASH_RECT, 1, 2, 1, 2, 2);
        aSet.Put( XLineDashItem( aEmpty, aDash ) );
    }

    aSet.Put( XLineStyleItem( bDottet ? XLINE_DASH : XLINE_SOLID ) );
    aSet.Put( XFillStyleItem( XFILL_NONE ) );
    aSet.Put( XLineColorItem(aEmpty,Color( aColor.nColor)) );
    aXOut.SetLineAttr(aSet);
    aXOut.SetFillAttr(aSet);

    const GeoStat& aGeo = pObj->GetGeoStat();

    if( aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 )
    {
        Polygon aPoly(aRect);
        if(aGeo.nShearWink!=0)
            ShearPoly(aPoly,aRect.TopLeft(),aGeo.nTan);

        if(aGeo.nDrehWink!=0)
            RotatePoly(aPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);

        aXOut.DrawPolyLine(aPoly);
    }
    else
    {
        aXOut.DrawRect(aRect);
    }
}

// -----------------------------------------------------------------------

void PresLayoutPreview::Paint( const Rectangle& rRect )
{
    Push();

    svtools::ColorConfig aColorConfig;
    svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( svtools::APPBACKGROUND ) );

    maOutRect = Rectangle( Point(0,0), GetOutputSize() );

    // calculate page size with correct aspect ratio
    int nWidth, nHeight;
    if( maPageSize.Width() > maPageSize.Height() )
    {
        nWidth = maOutRect.GetWidth();
        nHeight = long( (double)(nWidth * maPageSize.Height()) / (double)maPageSize.Width() );
    }
    else
    {
        nHeight = maOutRect.GetHeight();
        nWidth = long( (double)(nHeight * maPageSize.Width()) / (double)maPageSize.Height() );
    }

    maOutRect.nLeft += (maOutRect.GetWidth() - nWidth) >> 1;
    maOutRect.nRight = maOutRect.nLeft + nWidth - 1;
    maOutRect.nTop += (maOutRect.GetHeight() - nHeight) >> 1;
    maOutRect.nBottom = maOutRect.nTop + nHeight - 1;

    // draw decoration frame
    DecorationView aDecoView( this );
    maOutRect = aDecoView.DrawFrame( maOutRect, FRAME_HIGHLIGHT_IN );

    mdScaleX = (double)maOutRect.GetWidth() / (double)maPageSize.Width();
    mdScaleY = (double)maOutRect.GetHeight() / (double)maPageSize.Height();

    // draw page background
    SetFillColor( Color(COL_WHITE) );
    DrawRect( maOutRect );

    // paint presentation objects from masterpage
    SdrTextObj* pMasterTitle = (SdrTextObj*)mpMaster->GetPresObj( PRESOBJ_TITLE );
    SdrTextObj* pMasterOutline = (SdrTextObj*)mpMaster->GetPresObj( mpMaster->GetPageKind()==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );
    SdrTextObj* pHeader = (SdrTextObj*)mpMaster->GetPresObj( PRESOBJ_HEADER );
    SdrTextObj* pFooter = (SdrTextObj*)mpMaster->GetPresObj( PRESOBJ_FOOTER );
    SdrTextObj* pDate   = (SdrTextObj*)mpMaster->GetPresObj( PRESOBJ_DATETIME );
    SdrTextObj* pNumber = (SdrTextObj*)mpMaster->GetPresObj( PRESOBJ_SLIDENUMBER );

    XOutputDevice aXOut( this );

    if( pMasterTitle )
        Paint( aXOut, pMasterTitle, true, true );
    if( pMasterOutline )
        Paint( aXOut, pMasterOutline, true, true );
    if( pHeader )
        Paint( aXOut, pHeader, maSettings.mbHeaderVisible );
    if( pFooter )
        Paint( aXOut, pFooter, maSettings.mbFooterVisible );
    if( pDate )
        Paint( aXOut, pDate, maSettings.mbDateTimeVisible );
    if( pNumber )
        Paint( aXOut, pNumber, maSettings.mbSlideNumberVisible );

    Pop();
}
