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

#include <svx/dialogs.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/langitem.hxx>
#include <svx/langbox.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editeng.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdresid.hxx"

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
#include <vcl/ctrl.hxx>
#include <tools/color.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <vcl/decoview.hxx>

#include "undoheaderfooter.hxx"
#include "sdundogr.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace sd
{

class PresLayoutPreview : public Control
{
private:
    SdPage* mpMaster;
    HeaderFooterSettings maSettings;
    Size maPageSize;
    Rectangle maOutRect;

private:
    void Paint( OutputDevice& aOut, SdrTextObj* pObj, bool bVisible, bool bDotted = false );

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

const int nDateTimeFormatsCount = 12;
int nDateTimeFormats[nDateTimeFormatsCount] =
{
    SVXDATEFORMAT_A,
    SVXDATEFORMAT_B,
    SVXDATEFORMAT_C,
    SVXDATEFORMAT_D,
    SVXDATEFORMAT_E,
    SVXDATEFORMAT_F,

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
    static  sal_uInt16*    GetRanges();

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

IMPL_LINK( HeaderFooterDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    const sal_uInt16 nId = pTabCtrl->GetCurPageId();
    TabPage* pTabPage = pTabCtrl->GetTabPage( nId );
    pTabPage->Show();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(HeaderFooterDialog, DeactivatePageHdl)
{
    return sal_True;
}

// -----------------------------------------------------------------------

short HeaderFooterDialog::Execute()
{
    sal_uInt16 nRet = TabDialog::Execute();
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

void HeaderFooterDialog::Cancel( TabPage* )
{
    EndDialog();
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::apply( bool bToAll, bool bForceSlides )
{
    SdUndoGroup* pUndoGroup = new SdUndoGroup(mpDoc);
    OUString aComment( GetText() );
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
                SdPage* pPage = mpDoc->GetSdPage( (sal_uInt16)nPage, PK_STANDARD );
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
            SdPage* pPage = mpDoc->GetSdPage( (sal_uInt16)nPage, PK_NOTES );

            change( pUndoGroup, pPage, aNewSettings );
        }

        // and last but not least to the handout page
        change( pUndoGroup, mpDoc->GetMasterSdPage( 0, PK_HANDOUT ), aNewSettings );
    }

    // give the undo group to the undo manager
    mpViewShell->GetViewFrame()->GetObjectShell()->GetUndoManager()->AddUndoAction(pUndoGroup);
}

// -----------------------------------------------------------------------

void HeaderFooterDialog::change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings )
{
    pUndoGroup->AddAction(new SdHeaderFooterUndoAction(mpDoc, pPage, rNewSettings ));
    pPage->setHeaderFooterSettings( rNewSettings );
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

        maPBApplyToAll( this, SdResId( BT_APPLYTOALL ) ),
        maPBApply( this, SdResId( BT_APPLY ) ),
        maPBCancel( this, SdResId( BT_CANCEL ) ),
        maPBHelp( this, SdResId( BT_HELP ) ),

        maCTPreview( this, SdResId( CT_PREVIEW ),
            pActualPage ?
                (pActualPage->IsMasterPage() ? pActualPage : (SdPage*)(&(pActualPage->TRG_GetMasterPage()))) :
                (pDoc->GetMasterSdPage( 0, bHandoutMode ? PK_NOTES : PK_STANDARD )) ),
        mpCurrentPage(pActualPage),
        mpDoc(pDoc),
        mpDialog(pDialog),
        mbHandoutMode( bHandoutMode )

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

        maCBSlideNumber.SetText( SD_RESSTR( STR_PAGE_NUMBER ) );
        maFLIncludeOnPage.SetText( SD_RESSTR( STR_INCLUDE_ON_PAGE ) );
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
    meOldLanguage = MsLangId::getRealLanguage( meOldLanguage );
    maCBDateTimeLanguage.SelectLanguage( meOldLanguage );

    FillFormatList(SVXDATEFORMAT_A);

    maTBHeader.SetAccessibleRelationMemberOf(&maCBHeader);
    maRBDateTimeFixed.SetAccessibleRelationMemberOf(&maCBDateTime);
    maRBDateTimeAutomatic.SetAccessibleRelationMemberOf(&maCBDateTime);
    maTBDateTimeFixed.SetAccessibleName(maRBDateTimeFixed.GetText());
    maTBDateTimeFixed.SetAccessibleRelationMemberOf(&maCBDateTime);
    maTBDateTimeFixed.SetAccessibleRelationLabeledBy(&maRBDateTimeFixed);
    maCBDateTimeFormat.SetAccessibleRelationMemberOf(&maCBDateTime);
    maCBDateTimeFormat.SetAccessibleName(maRBDateTimeAutomatic.GetText());
    maCBDateTimeFormat.SetAccessibleRelationLabeledBy(&maRBDateTimeAutomatic);
    maCBDateTimeLanguage.SetAccessibleRelationMemberOf(&maCBDateTime);
    maTBFooter.SetAccessibleRelationMemberOf(&maCBFooter);
    maCBSlideNumber.SetAccessibleRelationMemberOf(&maFLIncludeOnPage);
    maCBFooter.SetAccessibleRelationMemberOf(&maFLIncludeOnPage);
}
// -----------------------------------------------------------------------

HeaderFooterTabPage::~HeaderFooterTabPage()
{
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(HeaderFooterTabPage, LanguageChangeHdl)
{
    FillFormatList( (int)(sal_IntPtr)maCBDateTimeFormat.GetEntryData( maCBDateTimeFormat.GetSelectEntryPos() ) );


    return 0L;
}

// -----------------------------------------------------------------------

void HeaderFooterTabPage::FillFormatList( int eFormat )
{
    LanguageType eLanguage = maCBDateTimeLanguage.GetSelectLanguage();

    maCBDateTimeFormat.Clear();

    Date aDate( Date::SYSTEM );
    Time aTime( Time::SYSTEM );

    int nFormat;
    for( nFormat = 0; nFormat < nDateTimeFormatsCount; nFormat++ )
    {
        OUString aStr( SvxDateTimeField::GetFormatted(
                aDate, aTime, nDateTimeFormats[nFormat],
                *(SD_MOD()->GetNumberFormatter()), eLanguage ) );
        sal_uInt16 nEntry = maCBDateTimeFormat.InsertEntry( aStr );
        maCBDateTimeFormat.SetEntryData( nEntry, (void*)(sal_IntPtr)nDateTimeFormats[nFormat] );
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

    sal_uInt16 nPos;
    for( nPos = 0; nPos < maCBDateTimeFormat.GetEntryCount(); nPos++ )
    {
        int nFormat = (int)(sal_IntPtr)maCBDateTimeFormat.GetEntryData( nPos );
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
        rSettings.meDateTimeFormat = (int)(sal_IntPtr)maCBDateTimeFormat.GetEntryData( maCBDateTimeFormat.GetSelectEntryPos() );

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

IMPL_LINK_NOARG(HeaderFooterTabPage, UpdateOnClickHdl)
{
    update();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(HeaderFooterTabPage, ClickApplyToAllHdl)
{
    mpDialog->ApplyToAll( this );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(HeaderFooterTabPage, ClickApplyHdl)
{
    mpDialog->Apply( this );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(HeaderFooterTabPage, ClickCancelHdl)
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
            sal_uInt16 nPageCount = mpDoc->GetMasterSdPageCount( PK_NOTES );
            sal_uInt16 nPage;
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
        sal_uInt16 nPageCount = bSet ? mpDoc->GetMasterSdPageCount( PK_NOTES ) : 1;
        sal_uInt16 nPage;
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
            sal_uInt16 nOutlMode = pOutl->GetMode();

            EditEngine* pEdit = const_cast< EditEngine* >(&pOutl->GetEditEngine());

            OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
            if( pOPO )
                pOutl->SetText( *pOPO );

            EFieldInfo aFieldInfo;
            aFieldInfo.pFieldItem = NULL;

            sal_Int32 nParaCount = pEdit->GetParagraphCount();
            sal_Int32 nPara;
            for( nPara = 0; (nPara < nParaCount) && (aFieldInfo.pFieldItem == NULL); nPara++ )
            {
                sal_uInt16 nFieldCount = pEdit->GetFieldCount( nPara );
                sal_uInt16 nField;
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

void PresLayoutPreview::Paint( OutputDevice& aOut, SdrTextObj* pObj, bool bVisible, bool bDotted /* = false*/ )
{
    // get object transformation
    basegfx::B2DHomMatrix aObjectTransform;
    basegfx::B2DPolyPolygon aObjectPolyPolygon;
    pObj->TRGetBaseGeometry(aObjectTransform, aObjectPolyPolygon);

    // build complete transformation by adding view transformation from
    // logic page coordinates to local pixel coordinates
    const double fScaleX((double)maOutRect.getWidth() / (double)maPageSize.Width());
    const double fScaleY((double)maOutRect.getHeight() / (double)maPageSize.Height());
    aObjectTransform.scale(fScaleX, fScaleY);
    aObjectTransform.translate(maOutRect.TopLeft().X(), maOutRect.TopLeft().Y());

    // create geometry using unit range and object transform
    basegfx::B2DPolyPolygon aGeometry(basegfx::tools::createUnitPolygon());
    aGeometry.transform(aObjectTransform);

    // apply line pattern if wanted
    if(bDotted)
    {
        ::std::vector<double> aPattern;
        static double fFactor(1.0);
        aPattern.push_back(3.0 * fFactor);
        aPattern.push_back(1.0 * fFactor);

        basegfx::B2DPolyPolygon aDashed;
        basegfx::tools::applyLineDashing(aGeometry, aPattern, &aDashed);
        aGeometry = aDashed;
    }

    // determine line color
    svtools::ColorConfig aColorConfig;
    svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( bVisible ? svtools::FONTCOLOR : svtools::OBJECTBOUNDARIES ) );

    // paint at OutDev
    aOut.SetLineColor(Color(aColor.nColor));
    aOut.SetFillColor();

    for(sal_uInt32 a(0); a < aGeometry.count(); a++)
    {
        aOut.DrawPolyLine(aGeometry.getB2DPolygon(a));
    }
}

// -----------------------------------------------------------------------

void PresLayoutPreview::Paint( const Rectangle& )
{
    Push();

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

    maOutRect.Left() += (maOutRect.GetWidth() - nWidth) >> 1;
    maOutRect.Right() = maOutRect.Left() + nWidth - 1;
    maOutRect.Top() += (maOutRect.GetHeight() - nHeight) >> 1;
    maOutRect.Bottom() = maOutRect.Top() + nHeight - 1;

    // draw decoration frame
    DecorationView aDecoView( this );
    maOutRect = aDecoView.DrawFrame( maOutRect, FRAME_HIGHLIGHT_IN );

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

    if( pMasterTitle )
        Paint( *this, pMasterTitle, true, true );
    if( pMasterOutline )
        Paint( *this, pMasterOutline, true, true );
    if( pHeader )
        Paint( *this, pHeader, maSettings.mbHeaderVisible );
    if( pFooter )
        Paint( *this, pFooter, maSettings.mbFooterVisible );
    if( pDate )
        Paint( *this, pDate, maSettings.mbDateTimeVisible );
    if( pNumber )
        Paint( *this, pNumber, maSettings.mbSlideNumberVisible );

    Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
