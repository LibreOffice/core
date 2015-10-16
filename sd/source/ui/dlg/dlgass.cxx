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

#include <com/sun/star/presentation/ClickAction.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/layout.hxx>
#include <vcl/idle.hxx>
#include <sfx2/doctempl.hxx>
#include <svl/lstner.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <fadedef.h>
#include <sfx2/sfxsids.hrc>
#include <svl/undo.hxx>
#include "DrawDocShell.hxx"
#include <vcl/gdimtf.hxx>
#include <tools/wintypes.hxx>
#include "docprev.hxx"
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <unotools/historyoptions.hxx>
#include <osl/file.hxx>
#include <sfx2/filedlghelper.hxx>

#include "sdpage.hxx"
#include "helpids.h"
#include "assclass.hxx"
#include "dlgass.hxx"
#include "dlgctrls.hxx"
#include "strings.hrc"
#include "dlgassim.hxx"
#include "TemplateScanner.hxx"
#include "WindowUpdater.hxx"

#include <comphelper/processfactory.hxx>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd;

using ::std::vector;

class PasswordEntry
{
public:
    uno::Sequence< beans::NamedValue > aEncryptionData;
    OUString maPath;
};

/** A simple wrapper that looks like a PushButton and is used to force the
    broadcasting of focus events primarily for accessibility tools.
    Forcing focus events is achieved by using two identical PushButtons
    which, when the focus event is requested, are exchanged and play focus
    ping-pong by moving the focus from one to the other.
*/
class NextButton
{
public:
    NextButton (vcl::Window* pParent);

    void ForceFocusEventBroadcast();
    void SetClickHdl (const Link<Button*,void>& rLink);
    bool IsEnabled();
    void Enable (bool bEnable);

private:
    VclPtr<PushButton> mpNextButton1;
    VclPtr<PushButton> mpNextButton2;
    bool mbIsFirstButtonActive;
};

class AssistentDlgImpl : public SfxListener
{
public:
    AssistentDlgImpl( vcl::Window* pWindow, const Link<ListBox&,void>& rFinishLink, bool bAutoPilot  );
    virtual ~AssistentDlgImpl();

    /// Local mutex used to serialize concurrent method calls.
    ::osl::Mutex maMutex;

    SfxObjectShellLock GetDocument();

    /** closes the current preview docshell */
    void CloseDocShell();

    /** Extract form the history list of recently used files the impress
        files and insert them into a listbox.
    */
    void    ScanDocmenu();
    /** Flag that is set to sal_True after the recently used files have been
        scanned.
    */
    bool mbRecentDocumentsReady;

    /** When the list of templates has not been scanned already this is done
        when this method is called.  That includes requesting the whole list
        of templates from UCB and extracting from that list the impress
        templates and layouts and storing them for later use in
        <member>maPresentList</member>.  Note that the first call to this
        method after installing a new Office may take some time.
    */
    void ProvideTemplates();

    /** This method transfers the template folders from the template scanner
        to the internal structures of this class.  On termination it sets
        the flag <member>mbTemplatesReady</member> to <TRUE/> to indicate
        that the templates are available.
        @param rTemplateFolders
            This is a list of template folders.  This method takes ownership
            of the supplied entries by removing them from the list and
            transferring them to an internal structure.
    */
    void TemplateScanDone (std::vector<TemplateDir*>& rTemplateFolders);

    /** Flag that is set to sal_True after the impress templates have been
        scanned.
    */
    bool mbTemplatesReady;

    /** Flag used to prevent nested or concurrent calls to the
        <member>UpdatePreview</memember> method.  A <TRUE/> value indicates
        that a preview update is currently active.
    */
    bool mbPreviewUpdating;

    VclPtr<vcl::Window> mpWindow;

    void SavePassword( SfxObjectShellLock xDoc, const OUString& rPath );
    void RestorePassword( SfxItemSet* pSet, const OUString& rPath );
    uno::Sequence < beans::NamedValue > GetPassword( const OUString& rPath );
    void DeletePasswords();

    boost::ptr_vector< PasswordEntry > maPasswordList;

    OUString maDocFile;
    OUString maLayoutFile;

    OUString GetDocFileName();
    OUString GetLayoutFileName();

    /// List of URLs of recently used impress files.
    std::vector<OUString> maOpenFilesList;

    /// List of folders containing data about impress templates.
    std::vector<TemplateDir*> maPresentList;

    /// Currently selected template folder.
    TemplateDir* mpTemplateRegion;

    /// Currently selected layout folder.
    TemplateDir* mpLayoutRegion;

    // preview
    bool mbUserDataDirty;
    Idle maPrevIdle;
    Idle maEffectPrevIdle;
    Idle maUpdatePageListIdle;

    SfxObjectShellLock xDocShell;

    ::std::unique_ptr<WindowUpdater> mpWindowUpdater;

    bool mbPreview;
    sal_uInt16 mnShowPage;
    bool mbDocPreview;

    OUString maPageListFile;

    void UpdatePreview( bool bDocPreview );
    void UpdatePageList();
    void UpdateUserData();

    static bool IsOwnFormat( const OUString& rPath );

    // dlg status
    void EndDialog( long nResult = 0 );

    void SetStartType( StartType eType );
    StartType GetStartType();

    void SelectTemplateRegion( const OUString& rRegion );
    void SelectLayoutRegion( const OUString& rRegion );

    void UpdatePage();
    void ChangePage();
    void LeavePage();

    static OUString GetUiTextForCommand (const OUString& aCommandURL);
    static Image GetUiIconForCommand (const OUString& aCommandURL);

    DECL_LINK_TYPED( SelectFileHdl, ListBox&, void );
    DECL_LINK_TYPED( SelectRegionHdl, ListBox&, void );
    DECL_LINK_TYPED( UpdatePreviewHdl, Idle *, void );
    DECL_LINK_TYPED( UpdatePageListHdl, Idle *, void );
    DECL_LINK_TYPED( StartTypeHdl, Button *, void );
    DECL_LINK_TYPED( SelectTemplateHdl, ListBox&, void);
    DECL_LINK_TYPED( NextPageHdl, Button*, void );
    DECL_LINK_TYPED( LastPageHdl, Button*, void );
    DECL_LINK_TYPED( PreviewFlagHdl, Button*, void );
    DECL_LINK_TYPED( EffectPreviewIdleHdl, Idle *, void );
    DECL_LINK_TYPED( EffectPreviewClickHdl, SdDocPreviewWin&, void );
    DECL_LINK_TYPED( SelectLayoutHdl, ListBox&, void );
    DECL_LINK_TYPED( PageSelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( PresTypeHdl, Button*, void );
    DECL_LINK_TYPED( UpdateUserDataHdl, Edit&, void );
    DECL_LINK_TYPED( SelectEffectHdl, ListBox&, void);
    DECL_LINK_TYPED( OpenButtonHdl, Button *, void );

    OUString            maCreateStr;
    OUString            maOpenStr;

    // Common
    Assistent           maAssistentFunc;
    VclPtr<CheckBox>           mpPreviewFlag;
    VclPtr<CheckBox>           mpStartWithFlag;
    VclPtr<PushButton>         mpLastPageButton;
    NextButton*                mpNextPageButton;
    VclPtr<PushButton>         mpFinishButton;
    VclPtr<SdDocPreviewWin>    mpPreview;
    VclPtr<VclContainer>       mpPage1235;

    // page 1
    VclPtr<VclContainer>       mpPage1;
    VclPtr<FixedImage>         mpPage1FB;
    VclPtr<FixedText>          mpPage1ArtFL;
    VclPtr<RadioButton>        mpPage1EmptyRB;
    VclPtr<RadioButton>        mpPage1TemplateRB;
    VclPtr<ListBox>            mpPage1RegionLB;
    VclPtr<ListBox>            mpPage1TemplateLB;
    VclPtr<RadioButton>        mpPage1OpenRB;
    VclPtr<ListBox>            mpPage1OpenLB;
    VclPtr<PushButton>         mpPage1OpenPB;

    // page 2
    VclPtr<VclContainer>       mpPage2;
    VclPtr<FixedImage>         mpPage2FB;
    VclPtr<FixedText>          mpPage2LayoutFL;
    VclPtr<ListBox>            mpPage2RegionLB;
    VclPtr<ListBox>            mpPage2LayoutLB;
    VclPtr<FixedText>          mpPage2OutTypesFL;
    VclPtr<RadioButton>        mpPage2Medium1RB;
    VclPtr<RadioButton>        mpPage2Medium2RB;
    VclPtr<RadioButton>        mpPage2Medium3RB;
    VclPtr<RadioButton>        mpPage2Medium4RB;
    VclPtr<RadioButton>        mpPage2Medium5RB;
    VclPtr<RadioButton>        mpPage2Medium6RB;

    // page 3
    VclPtr<VclContainer>       mpPage3;
    VclPtr<FixedImage>         mpPage3FB;
    VclPtr<FixedText>          mpPage3EffectFL;
    VclPtr<FixedText>          mpPage3EffectFT;
    VclPtr<FadeEffectLB>       mpPage3EffectLB;
    VclPtr<FixedText>          mpPage3SpeedFT;
    VclPtr<ListBox>            mpPage3SpeedLB;
    VclPtr<FixedText>          mpPage3PresTypeFL;
    VclPtr<RadioButton>        mpPage3PresTypeLiveRB;
    VclPtr<RadioButton>        mpPage3PresTypeKioskRB;
    VclPtr<FixedText>          mpPage3PresTimeFT;
    VclPtr<TimeField>          mpPage3PresTimeTMF;
    VclPtr<FixedText>          mpPage3BreakFT;
    VclPtr<TimeField>          mpPage3BreakTMF;
    VclPtr<CheckBox>           mpPage3LogoCB;

    // page 4
    VclPtr<VclContainer>       mpPage4;
    VclPtr<FixedImage>         mpPage4FB;
    VclPtr<FixedText>          mpPage4PersonalFL;
    VclPtr<FixedText>          mpPage4AskNameFT;
    VclPtr<Edit>               mpPage4AskNameEDT;
    VclPtr<FixedText>          mpPage4AskTopicFT;
    VclPtr<Edit>               mpPage4AskTopicEDT;
    VclPtr<FixedText>          mpPage4AskInfoFT;
    VclPtr<VclMultiLineEdit>   mpPage4AskInfoEDT;

    // page 5
    VclPtr<VclContainer>       mpPage5;
    VclPtr<FixedImage>         mpPage5FB;
    VclPtr<FixedText>          mpPage5PageListFT;
    VclPtr<SdPageListControl>  mpPage5PageListCT;
    VclPtr<CheckBox>           mpPage5SummaryCB;

};

AssistentDlgImpl::AssistentDlgImpl( vcl::Window* pWindow, const Link<ListBox&,void>& rFinishLink, bool bAutoPilot ) :
    mpTemplateRegion(NULL),
    mpLayoutRegion(NULL),
    mbUserDataDirty(false),
    xDocShell (NULL),
    mpWindowUpdater (new WindowUpdater()),
    mbPreview(true),
    mnShowPage(0),
    mbDocPreview(false),
    maAssistentFunc(5)
{
    AssistentDlg* assDlg = static_cast<AssistentDlg*>(pWindow);
    assDlg->get(mpPreviewFlag, "previewCheckbutton");
    assDlg->get(mpStartWithFlag, "startWithCheckbutton");
    assDlg->get(mpLastPageButton, "lastPageButton");
    assDlg->get(mpFinishButton, "finishButton");
    maCreateStr = mpFinishButton->GetText();
    maOpenStr = assDlg->get<PushButton>("open")->GetText();
    assDlg->get(mpPreview, "previewControl");
    assDlg->get(mpPage1235, "page1235Box");
    mpNextPageButton = new NextButton( pWindow );

    //Lock down the preferred size based on the
    //initial max-size configuration
    assDlg->get(mpPage1TemplateLB, "templatesTreeview");
    assDlg->get(mpPage1OpenLB, "openTreeview");
    long nHeight = mpPage1TemplateLB->GetTextHeight() * 7;
    mpPage1TemplateLB->set_height_request(nHeight);
    mpPage1OpenLB->set_height_request(nHeight);

    assDlg->get(mpPage1, "page1Box");
    assDlg->get(mpPage2, "page2Box");
    assDlg->get(mpPage3, "page3Box");
    assDlg->get(mpPage4, "page4Frame");
    assDlg->get(mpPage5, "page5Box");
    Size aSize(mpPage1->get_preferred_size());
    mpPage1->set_height_request(aSize.Height());
    mpPage2->set_height_request(aSize.Height());
    mpPage3->set_height_request(aSize.Height());
    mpPage4->set_height_request(aSize.Height());
    mpPage5->set_height_request(aSize.Height());
    mpPage1->set_width_request(aSize.Width());
    mpPage2->set_width_request(aSize.Width());
    mpPage3->set_width_request(aSize.Width());
    mpPage4->set_width_request(aSize.Width());
    mpPage5->set_width_request(aSize.Width());

    aSize = assDlg->get_preferred_size();
    assDlg->set_width_request(aSize.Width());
    assDlg->set_height_request(aSize.Height());

    maPageListFile = "?";
    mbRecentDocumentsReady = false;
    mbTemplatesReady = false;
    mbPreviewUpdating = false;

    mpWindow = pWindow;

    if (bAutoPilot)
        mpStartWithFlag->Hide();
    else
        maAssistentFunc.InsertControl(1, mpStartWithFlag );

    // initialize page1 and give it to the assistant functionality
    assDlg->get(mpPage1FB, "header1Image");
    assDlg->get(mpPage1ArtFL, "typeLabel");
    assDlg->get(mpPage1EmptyRB, "emptyRadiobutton");
    assDlg->get(mpPage1TemplateRB, "templateRadiobutton");
    assDlg->get(mpPage1OpenRB, "openRadiobutton");
    assDlg->get(mpPage1RegionLB, "regionCombobox");
    assDlg->get(mpPage1OpenPB, "openButton");
    maAssistentFunc.InsertControl(1, mpPage1 );
    maAssistentFunc.InsertControl(1, mpPage1235 );
    maAssistentFunc.InsertControl(1, mpPreview );
    maAssistentFunc.InsertControl(1, mpPreviewFlag );
    maAssistentFunc.InsertControl(1, mpPage1FB );
    maAssistentFunc.InsertControl(1, mpPage1ArtFL );
    maAssistentFunc.InsertControl(1, mpPage1EmptyRB);
    maAssistentFunc.InsertControl(1, mpPage1TemplateRB );
    maAssistentFunc.InsertControl(1, mpPage1OpenRB );
    maAssistentFunc.InsertControl(1, mpPage1RegionLB );
    maAssistentFunc.InsertControl(1, mpPage1TemplateLB );
    maAssistentFunc.InsertControl(1, mpPage1OpenPB );
    maAssistentFunc.InsertControl(1, mpPage1OpenLB );

    // Set text and icon of the 'Open...' button.
    {
        OUString sText (GetUiTextForCommand(".uno:Open"));
        // Remove the mnemonic and add a leading space so that icon and text
        // are not too close together.
        sText = sText.replaceAll("~", "");
        sText = " " + sText;
        mpPage1OpenPB->SetText(sText);
        // Place icon left of text and both centered in the button.
        mpPage1OpenPB->SetModeImage(
            GetUiIconForCommand(".uno:Open")
        );
        mpPage1OpenPB->EnableImageDisplay(true);
        mpPage1OpenPB->EnableTextDisplay(true);
        mpPage1OpenPB->SetImageAlign(ImageAlign::Left);
        mpPage1OpenPB->SetStyle(mpPage1OpenPB->GetStyle() | WB_CENTER);
    }

    // links&handler
    mpPage1RegionLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectRegionHdl));
    mpPage1RegionLB->SetDropDownLineCount( 6 );
    mpPage1TemplateLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectTemplateHdl));
    mpPage1TemplateLB->SetStyle(mpPage1TemplateLB->GetStyle() | WB_SORT);
    mpPage1TemplateLB->InsertEntry(SD_RESSTR(STR_ISLOADING));

    mpPage1EmptyRB->SetClickHdl(LINK(this,AssistentDlgImpl,StartTypeHdl));
    mpPage1TemplateRB->SetClickHdl(LINK(this,AssistentDlgImpl,StartTypeHdl));
    mpPage1OpenRB->SetClickHdl(LINK(this,AssistentDlgImpl,StartTypeHdl));
    mpPage1OpenLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectFileHdl));
    mpPage1OpenLB->SetDoubleClickHdl(rFinishLink);
    mpPage1OpenPB->SetClickHdl(LINK(this,AssistentDlgImpl,OpenButtonHdl));

    // page 2
    assDlg->get(mpPage2FB, "header2Image");
    assDlg->get(mpPage2LayoutFL, "layout2Label");
    assDlg->get(mpPage2RegionLB, "page2RegionCombobox");
    assDlg->get(mpPage2LayoutLB, "layoutTreeview");
    maAssistentFunc.InsertControl(2, mpPage2 );
    maAssistentFunc.InsertControl(2, mpPage1235 );
    maAssistentFunc.InsertControl(2, mpPreview );
    maAssistentFunc.InsertControl(2, mpPreviewFlag );
    maAssistentFunc.InsertControl(2, mpPage2FB );
    maAssistentFunc.InsertControl(2, mpPage2LayoutFL );
    maAssistentFunc.InsertControl(2, mpPage2RegionLB );
    maAssistentFunc.InsertControl(2, mpPage2LayoutLB );

    assDlg->get(mpPage2OutTypesFL, "outTypesLabel");
    assDlg->get(mpPage2Medium5RB, "medium5Radiobutton");
    assDlg->get(mpPage2Medium3RB, "medium3Radiobutton");
    assDlg->get(mpPage2Medium4RB, "medium4Radiobutton");
    assDlg->get(mpPage2Medium1RB, "medium1Radiobutton");
    assDlg->get(mpPage2Medium2RB, "medium2Radiobutton");
    assDlg->get(mpPage2Medium6RB, "medium6Radiobutton");
    maAssistentFunc.InsertControl(2, mpPage2OutTypesFL );
    maAssistentFunc.InsertControl(2, mpPage2Medium5RB );
    maAssistentFunc.InsertControl(2, mpPage2Medium3RB );
    maAssistentFunc.InsertControl(2, mpPage2Medium4RB );
    maAssistentFunc.InsertControl(2, mpPage2Medium1RB );
    maAssistentFunc.InsertControl(2, mpPage2Medium2RB );
    maAssistentFunc.InsertControl(2, mpPage2Medium6RB );
    mpPage2Medium5RB->Check();

    mpPage2RegionLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectRegionHdl));
    mpPage2RegionLB->SetDropDownLineCount( 6 );
    mpPage2LayoutLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectLayoutHdl));
    mpPage2LayoutLB->SetStyle(mpPage2LayoutLB->GetStyle() | WB_SORT);
    mpPage2LayoutLB->InsertEntry(SD_RESSTR(STR_ISLOADING));

    // page 3
    assDlg->get(mpPage3FB, "header3Image");
    assDlg->get(mpPage3EffectFL, "page3EffectLabel");
    assDlg->get(mpPage3EffectFT, "effectLabel");
    assDlg->get(mpPage3EffectLB, "effectCombobox");
    assDlg->get(mpPage3SpeedFT, "speedLabel");
    assDlg->get(mpPage3SpeedLB, "speedCombobox");
    assDlg->get(mpPage3PresTypeFL, "presTypeLabel");
    assDlg->get(mpPage3PresTypeLiveRB, "liveRadiobutton");
    assDlg->get(mpPage3PresTypeKioskRB, "kioskRadiobutton");
    assDlg->get(mpPage3PresTimeFT, "presTimeLabel");
    assDlg->get(mpPage3PresTimeTMF, "timeSpinbutton");
    assDlg->get(mpPage3BreakFT, "breakLabel");
    assDlg->get(mpPage3BreakTMF, "breakSpinbutton");
    assDlg->get(mpPage3LogoCB, "logoCheckbutton");

    maAssistentFunc.InsertControl(3, mpPage3 );
    maAssistentFunc.InsertControl(3, mpPage1235 );
    maAssistentFunc.InsertControl(3, mpPreview );
    maAssistentFunc.InsertControl(3, mpPreviewFlag );
    maAssistentFunc.InsertControl(3, mpPage3FB );
    maAssistentFunc.InsertControl(3, mpPage3EffectFL );
    maAssistentFunc.InsertControl(3, mpPage3EffectFT );
    maAssistentFunc.InsertControl(3, mpPage3EffectLB );
    maAssistentFunc.InsertControl(3, mpPage3SpeedFT );
    maAssistentFunc.InsertControl(3, mpPage3SpeedLB );
    maAssistentFunc.InsertControl(3, mpPage3PresTypeFL );
    maAssistentFunc.InsertControl(3, mpPage3PresTypeLiveRB );
    maAssistentFunc.InsertControl(3, mpPage3PresTypeKioskRB );
    maAssistentFunc.InsertControl(3, mpPage3PresTimeFT );
    maAssistentFunc.InsertControl(3, mpPage3PresTimeTMF );
    maAssistentFunc.InsertControl(3, mpPage3BreakFT );
    maAssistentFunc.InsertControl(3, mpPage3BreakTMF );
    maAssistentFunc.InsertControl(3, mpPage3LogoCB );

    mpPage3EffectLB->Fill();
    mpPage3EffectLB->SetSelectHdl( LINK(this,AssistentDlgImpl,SelectEffectHdl ));
    mpPage3EffectLB->SetDropDownLineCount( 12 );

    mpPage3SpeedLB->InsertEntry( SD_RESSTR(STR_SLOW) );
    mpPage3SpeedLB->InsertEntry( SD_RESSTR(STR_MEDIUM) );
    mpPage3SpeedLB->InsertEntry( SD_RESSTR(STR_FAST) );
    mpPage3SpeedLB->SetDropDownLineCount( 3 );
    mpPage3SpeedLB->SetSelectHdl( LINK(this,AssistentDlgImpl,SelectEffectHdl ));
    mpPage3SpeedLB->SelectEntryPos( 1 );

    mpPage3PresTypeLiveRB->Check();
    mpPage3PresTypeLiveRB->SetClickHdl( LINK(this,AssistentDlgImpl, PresTypeHdl ));
    mpPage3PresTypeKioskRB->SetClickHdl( LINK(this,AssistentDlgImpl, PresTypeHdl ));
    mpPage3PresTimeTMF->SetFormat( TimeFieldFormat::F_SEC );
    mpPage3PresTimeTMF->SetTime( ::tools::Time( 0, 0, 10 ) );
    mpPage3BreakTMF->SetFormat( TimeFieldFormat::F_SEC );
    mpPage3BreakTMF->SetTime( ::tools::Time( 0, 0, 10 ) );
    mpPage3LogoCB->Check();

    // set cursor in timefield
    Edit *pEditPage3PresTimeTMF = mpPage3PresTimeTMF->GetField();
    Edit *pEditPage3BreakTMF = mpPage3BreakTMF->GetField();
    Selection aSel1( pEditPage3PresTimeTMF->GetMaxTextLen(), pEditPage3PresTimeTMF->GetMaxTextLen() );
    Selection aSel2( pEditPage3BreakTMF->GetMaxTextLen(), pEditPage3BreakTMF->GetMaxTextLen() );
    pEditPage3PresTimeTMF->SetSelection( aSel1 );
    pEditPage3BreakTMF->SetSelection( aSel2 );

    // page 4
    assDlg->get(mpPage4FB, "header4Image");
    assDlg->get(mpPage4PersonalFL, "personalLabel");
    assDlg->get(mpPage4AskNameFT, "askNameLabel");
    assDlg->get(mpPage4AskNameEDT, "askNameEntry");
    assDlg->get(mpPage4AskTopicFT, "askTopicLabel");
    assDlg->get(mpPage4AskTopicEDT, "askTopicEntry");
    assDlg->get(mpPage4AskInfoFT, "askInfoLabel");
    assDlg->get(mpPage4AskInfoEDT, "askInformationTextview");
    maAssistentFunc.InsertControl(4, mpPage4 );
    maAssistentFunc.InsertControl(4, mpPage4FB );
    maAssistentFunc.InsertControl(4, mpPage4PersonalFL );
    maAssistentFunc.InsertControl(4, mpPage4AskNameFT );
    maAssistentFunc.InsertControl(4, mpPage4AskNameEDT );
    maAssistentFunc.InsertControl(4, mpPage4AskTopicFT);
    maAssistentFunc.InsertControl(4, mpPage4AskTopicEDT );
    maAssistentFunc.InsertControl(4, mpPage4AskInfoFT );
    maAssistentFunc.InsertControl(4, mpPage4AskInfoEDT );

    mpPage4AskNameEDT->SetModifyHdl(LINK(this,AssistentDlgImpl,UpdateUserDataHdl));
    mpPage4AskTopicEDT->SetModifyHdl(LINK(this,AssistentDlgImpl,UpdateUserDataHdl));
    mpPage4AskInfoEDT->SetModifyHdl(LINK(this,AssistentDlgImpl,UpdateUserDataHdl));

    // page 5
    assDlg->get(mpPage5FB, "header5Image");
    assDlg->get(mpPage5PageListFT, "pageListLabel");
    assDlg->get(mpPage5PageListCT, "TreeListBox");
    assDlg->get(mpPage5SummaryCB, "summaryCheckbutton");
    maAssistentFunc.InsertControl(5, mpPage5 );
    maAssistentFunc.InsertControl(5, mpPage1235 );
    maAssistentFunc.InsertControl(5, mpPreview );
    maAssistentFunc.InsertControl(5, mpPreviewFlag );
    maAssistentFunc.InsertControl(5, mpPage5FB );
    maAssistentFunc.InsertControl(5, mpPage5PageListFT );
    maAssistentFunc.InsertControl(5, mpPage5PageListCT );
    maAssistentFunc.InsertControl(5, mpPage5SummaryCB );

    mpPage5PageListCT->SetSelectHdl(LINK(this,AssistentDlgImpl, PageSelectHdl));

    // general
    mpLastPageButton->SetClickHdl(LINK(this,AssistentDlgImpl, LastPageHdl ));
    mpNextPageButton->SetClickHdl(LINK(this,AssistentDlgImpl, NextPageHdl ));

    mpPreviewFlag->Check( mbPreview );
    mpPreviewFlag->SetClickHdl(LINK(this, AssistentDlgImpl, PreviewFlagHdl ));
    mpPreview->SetClickHdl(LINK(this,AssistentDlgImpl, EffectPreviewClickHdl ));

    // sets the exit page
    maAssistentFunc.GotoPage(1);
    mpLastPageButton->Disable();

    maPrevIdle.SetPriority( SchedulerPriority::LOWER );
    maPrevIdle.SetIdleHdl( LINK( this, AssistentDlgImpl, UpdatePreviewHdl));

    maEffectPrevIdle.SetPriority( SchedulerPriority::MEDIUM );
    maEffectPrevIdle.SetIdleHdl( LINK( this, AssistentDlgImpl, EffectPreviewIdleHdl ));

    maUpdatePageListIdle.SetPriority( SchedulerPriority::MEDIUM );
    maUpdatePageListIdle.SetIdleHdl( LINK( this, AssistentDlgImpl, UpdatePageListHdl));

    SetStartType( ST_EMPTY );

    ChangePage();

    mpWindowUpdater->RegisterWindow (mpPreview);

    UpdatePreview( true );

    //check whether we should start with a template document initially and preselect it
    const OUString aServiceName( "com.sun.star.presentation.PresentationDocument" );
    OUString aStandardTemplate( SfxObjectFactory::GetStandardTemplate( aServiceName ) );
    if( !aStandardTemplate.isEmpty() )
    {
        ProvideTemplates();

        //find aStandardTemplate in maPresentList
        TemplateDir*   pStandardTemplateDir = 0;
        TemplateEntry* pStandardTemplateEntry = 0;

        std::vector<TemplateDir*>::iterator I;
        for (I=maPresentList.begin(); I!=maPresentList.end(); ++I)
        {
            TemplateDir* pDir = *I;
            std::vector<TemplateEntry*>::iterator   J;
            for (J=pDir->maEntries.begin(); J!=pDir->maEntries.end(); ++J)
            {
                TemplateEntry* pEntry = *J;
                if(pEntry->msPath == aStandardTemplate)
                {
                    pStandardTemplateDir = pDir;
                    pStandardTemplateEntry = pEntry;
                    break;
                }
            }
            if(pStandardTemplateDir)
                break;
        }

        //preselect template
        if( pStandardTemplateDir && pStandardTemplateEntry )
        {
            mpPage1RegionLB->SelectEntry( pStandardTemplateDir->msRegion );
            SelectTemplateRegion( pStandardTemplateDir->msRegion );
            mpPage1TemplateLB->SelectEntry( pStandardTemplateEntry->msTitle );
            SelectTemplateHdl(*mpPage1TemplateLB);
        }
    }
}

AssistentDlgImpl::~AssistentDlgImpl()
{
    CloseDocShell();

    DeletePasswords();

    //  Delete the template file infos.
    std::vector<TemplateDir*>::iterator I;
    std::vector<TemplateEntry*>::iterator   J;
    for (I=maPresentList.begin(); I!=maPresentList.end(); ++I)
    {
        for (J=(*I)->maEntries.begin(); J!=(*I)->maEntries.end(); ++J)
            delete (*J);
        delete (*I);
    }
}

void AssistentDlgImpl::CloseDocShell()
{
    if(xDocShell.Is())
    {
        uno::Reference< util::XCloseable > xCloseable( xDocShell->GetModel(), uno::UNO_QUERY );
        if( xCloseable.is() )
        {
            xCloseable->close( sal_True );
            xDocShell = NULL;
        }
        else
        {
            xDocShell->DoClose();
            xDocShell = NULL;
        }
    }
}

void AssistentDlgImpl::EndDialog( long )
{
    mpWindow = NULL;
}

void    AssistentDlgImpl::ScanDocmenu()
{
    if( mbRecentDocumentsReady )
        return;

    uno::Sequence<uno::Sequence<beans::PropertyValue> > aHistory =
        SvtHistoryOptions().GetList (ePICKLIST);

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference< container::XNameAccess > xFilterFactory( xFactory->createInstance( "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );

    uno::Reference<ucb::XSimpleFileAccess3> xFileAccess(ucb::SimpleFileAccess::create(::comphelper::getProcessComponentContext()));

    sal_uInt32 nCount = aHistory.getLength();
    for (sal_uInt32 nItem=0; nItem<nCount; ++nItem)
    {
        //  Get the current history item's properties.
        uno::Sequence<beans::PropertyValue> aPropertySet = aHistory[nItem];
        OUString   sURL;
        OUString   sFilter;
        OUString   sTitle;
        OUString   sPassword;
        sal_uInt32 nPropertyCount = aPropertySet.getLength();
        for (sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty)
            if (aPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_URL)
                aPropertySet[nProperty].Value >>= sURL;
            else if (aPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_FILTER)
                aPropertySet[nProperty].Value >>= sFilter;
            else if (aPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_TITLE)
                aPropertySet[nProperty].Value >>= sTitle;
            else if (aPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_PASSWORD)
                aPropertySet[nProperty].Value >>= sPassword;

        //  If the entry is an impress file then insert it into the
        //  history list and the list box.
        uno::Sequence< beans::PropertyValue > lProps;
        if (xFilterFactory->hasByName(sFilter))
        {
            uno::Any aFilterPropSet = xFilterFactory->getByName( sFilter );
            aFilterPropSet >>= lProps;
        }

        sal_Int32 nPropCount = lProps.getLength();
        OUString sFactoryName;
        for( sal_Int32 i=0; i<nPropCount; ++i )
        {
            if( lProps[i].Name == "DocumentService" &&
                (lProps[i].Value >>= sFactoryName) &&
                sFactoryName == "com.sun.star.presentation.PresentationDocument" )
            {
                // yes, it's an impress document
                INetURLObject aURL;

                // Do not include the file if it does not exist.
                if (xFileAccess.is() && ! xFileAccess->exists(sURL))
                    continue;

                aURL.SetSmartURL (sURL);
                // The password is set only when it is not empty.
                if (!sPassword.isEmpty())
                    aURL.SetPass (sPassword);
                maOpenFilesList.push_back (aURL.GetMainURL(INetURLObject::NO_DECODE));
                mpPage1OpenLB->InsertEntry (sTitle);
                break;
            }
        }
    }
    mbRecentDocumentsReady = true;
    try
    {
        UpdatePreview(true);
    }
    catch (uno::RuntimeException& )
    {
        // Ignore all exceptions.
    }
}

void AssistentDlgImpl::ProvideTemplates()
{
    if ( ! mbTemplatesReady)
    {
        TemplateScanner aScanner;
        aScanner.EnableEntrySorting();
        aScanner.Scan ();
        TemplateScanDone (aScanner.GetFolderList());

        try
        {
            UpdatePreview(true);
        }
        catch (uno::RuntimeException& )
        {
            // Ignore all exceptions.
        }
    }
}

void AssistentDlgImpl::TemplateScanDone (
    std::vector<TemplateDir*>& rTemplateFolder)
{
    //  This method is called from a thread.  Therefore we get the solar mutex.
    SolarMutexGuard aGuard;

    // Copy the contents of the given template folders to a local list.
    maPresentList.swap (rTemplateFolder);

    //  Fill in the list box on the first page.
    int nFirstEntry = 0;
    mpPage1RegionLB->Clear();
    std::vector<TemplateDir*>::iterator I;
    int i;
    for (i=0,I=maPresentList.begin(); I!=maPresentList.end(); ++I,++i)
    {
        TemplateDir* pDir = *I;
        if (pDir == NULL)
            continue;

        // HACK! presnt directory is always initially selected.
        // We have to look at the first entry to get a URL.
        if (!pDir->maEntries.empty() )
        {
            TemplateEntry* pEntry = pDir->maEntries.front();
            if (pEntry != NULL)
                if (pEntry->msPath.indexOf("presnt") != -1)
                    nFirstEntry = i;
        }

        mpPage1RegionLB->InsertEntry (pDir->msRegion);
    }
    mpPage1RegionLB->SelectEntryPos ((sal_uInt16)nFirstEntry);
    mpPage1RegionLB->Update();
    SelectTemplateRegion (mpPage1RegionLB->GetSelectEntry());

    //  Fill in the list box on the second page.
    nFirstEntry = 0;
    mpPage2RegionLB->Clear();
    for (i=0,I=maPresentList.begin(); I!=maPresentList.end(); ++I,++i)
    {
        TemplateDir* pDir = *I;
        if (pDir == NULL)
            continue;

        // HACK! layout directory is always initially selected.
        // We have to look at the first entry to get a URL.
        if (!pDir->maEntries.empty() )
        {
            TemplateEntry* pEntry = pDir->maEntries.front();
            if (pEntry != NULL)
                if (pEntry->msPath.indexOf("layout") != -1)
                    nFirstEntry = i;
        }

        mpPage2RegionLB->InsertEntry (pDir->msRegion);
    }
    mpPage2RegionLB->SelectEntryPos ((sal_uInt16)nFirstEntry);
    mpPage2RegionLB->Update();
    SelectLayoutRegion (mpPage2RegionLB->GetSelectEntry());

    //  Make the changes visible.
    mbTemplatesReady = true;
    if (mpWindow)
        UpdatePage();
}

// ********************************************************************
// state methods
// ********************************************************************

void AssistentDlgImpl::SetStartType( StartType eType )
{
    mpPage1EmptyRB->SetState( eType == ST_EMPTY );
    mpPage1TemplateRB->SetState( eType == ST_TEMPLATE );
    mpPage1OpenRB->SetState( eType == ST_OPEN );
    mpNextPageButton->Enable( eType != ST_OPEN );

    mpPage1RegionLB->Show(eType == ST_TEMPLATE);
    mpPage1TemplateLB->Show(eType == ST_TEMPLATE);
    mpPage1OpenLB->Show(eType == ST_OPEN);
    mpPage1OpenPB->Show(eType == ST_OPEN);

    if (eType == ST_OPEN)
        mpFinishButton->SetText(maOpenStr);
    else
        mpFinishButton->SetText(maCreateStr);
}

StartType AssistentDlgImpl::GetStartType()
{
    if( mpPage1EmptyRB->IsChecked() )
        return ST_EMPTY;
    else if( mpPage1TemplateRB->IsChecked() )
        return ST_TEMPLATE;
    else
        return ST_OPEN;
}

OUString AssistentDlgImpl::GetDocFileName()
{
    OUString aTitle;
    if(mpWindow)
    {
        aTitle = mpWindow->GetText();
        sal_Int32 nPos = aTitle.indexOf('(');
        if (nPos != -1)
            aTitle = aTitle.copy( 0, nPos-1 );
    }

    OUString aDocFile;
    if( GetStartType() == ST_TEMPLATE )
    {
        const sal_Int32 nEntry = mpPage1TemplateLB->GetSelectEntryPos();
        TemplateEntry* pEntry = NULL;
        if (mpTemplateRegion && nEntry != LISTBOX_ENTRY_NOTFOUND)
            pEntry = mpTemplateRegion->maEntries[nEntry];

        if(pEntry)
        {
            aDocFile = pEntry->msPath;

            aTitle += " (";
            aTitle += pEntry->msTitle;
            aTitle += ")";
        }
    }
    else if( GetStartType() == ST_OPEN )
    {
        const sal_Int32 nEntry = mpPage1OpenLB->GetSelectEntryPos();
        if( nEntry != LISTBOX_ENTRY_NOTFOUND )
            aDocFile = maOpenFilesList[nEntry];
    }

    if(mpWindow)
        mpWindow->SetText(aTitle);

    return aDocFile;
}

OUString AssistentDlgImpl::GetLayoutFileName()
{
    const sal_Int32 nEntry = mpPage2LayoutLB->GetSelectEntryPos();
    TemplateEntry* pEntry = NULL;
    if(nEntry != LISTBOX_ENTRY_NOTFOUND && nEntry > 0)
        pEntry = mpLayoutRegion->maEntries[nEntry-1];

    if(pEntry)
        return pEntry->msPath;

    return OUString();
}

SfxObjectShellLock AssistentDlgImpl::GetDocument()
{
    UpdatePreview(false);   // but load completely
    UpdatePageList();

    SfxObjectShell* pShell = xDocShell;
    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( pShell );
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    if(pDoc)
    {
        const sal_uInt16 nPageCount = pDoc->GetSdPageCount(PK_STANDARD);
        bool bKiosk = mpPage3PresTypeKioskRB->IsChecked();
        double fNewTime = (double)mpPage3PresTimeTMF->GetTime().GetMSFromTime() / 1000.0;
        if(bKiosk)
        {
            PresentationSettings& rSettings = pDoc->getPresentationSettings();
            rSettings.mbEndless = bKiosk;
            rSettings.mnPauseTimeout = (sal_Int32)mpPage3BreakTMF->GetTime().GetMSFromTime() / 1000;
            rSettings.mbShowPauseLogo = mpPage3LogoCB->IsChecked();
        }

        sal_uInt16 nPgAbsNum = 0;
        sal_uInt16 nPgRelNum = 0;
        while( nPgAbsNum < nPageCount )
        {
            SdPage* pPage = pDoc->GetSdPage( nPgRelNum, PK_STANDARD );
            if( mpPage5PageListCT->IsPageChecked(nPgAbsNum) )
            {
                mpPage3EffectLB->applySelected(pPage);
                const sal_Int32 nPos = mpPage3SpeedLB->GetSelectEntryPos();
                pPage->setTransitionDuration( (nPos == 0) ? 3.0 : (nPos == 1) ? 2.0 : 1.0 );
                if(bKiosk)
                {
                    pPage->SetPresChange( PRESCHANGE_AUTO );
                    pPage->SetTime(fNewTime);
                }
                nPgRelNum++;
            }
            else
            {
                // delete this page
                pDoc->DeletePage( (nPgRelNum << 1) + 2 ); // delete not page
                pDoc->DeletePage( (nPgRelNum << 1) + 1 ); // delete page
            }

            nPgAbsNum++;
        }
    }
    else
    {
        OSL_FAIL("sd::AssistentDlgImpl::GetDocument(), no template?");
    }

    SfxObjectShellLock xRet = xDocShell;
    xDocShell = NULL;

    return xRet;
}

void AssistentDlgImpl::LeavePage()
{
    int nPage = maAssistentFunc.GetCurrentPage();

    if( nPage == 4 && mbUserDataDirty )
        maPrevIdle.Start();
}

void AssistentDlgImpl::ChangePage()
{
    mpNextPageButton->Enable(!maAssistentFunc.IsLastPage());
    mpLastPageButton->Enable(!maAssistentFunc.IsFirstPage());

    UpdatePage();

    if( mpNextPageButton->IsEnabled() )
    {
      mpNextPageButton->ForceFocusEventBroadcast();
    }
    else
        mpFinishButton->GrabFocus();
}

void AssistentDlgImpl::UpdatePage()
{
    sal_uInt16 nPage = (sal_uInt16)maAssistentFunc.GetCurrentPage();

    switch(nPage)
    {
    case 1:
        {
            // Show elements on first page depending of start type
            SetStartType( GetStartType() );
            mpPage1TemplateRB->Enable(true /*mbTemplatesReady*/);
            break;
        }

    case 2:
        {
            mpPage2RegionLB->Enable(mbTemplatesReady);
            mpPage2LayoutLB->Enable(mbTemplatesReady);

            if( GetStartType() != ST_EMPTY )
            {
                mpPage2Medium5RB->Enable( true );
            }
            else
            {
                mpPage2Medium5RB->Enable( false );
                if(mpPage2Medium5RB->IsChecked())
                    mpPage2Medium1RB->Check();
            }

            break;
        }
    case 5:
        {
            if(mbDocPreview || maPageListFile != maDocFile)
                mpPage5PageListCT->Clear();

            maUpdatePageListIdle.Start();
            break;
        }

    case 3:
        {
            if(GetStartType() != ST_TEMPLATE)
                mpNextPageButton->Enable(false);

            bool bKiosk = mpPage3PresTypeKioskRB->IsChecked();
            mpPage3PresTimeFT->Enable(bKiosk);
            mpPage3BreakFT->Enable(bKiosk);
            mpPage3PresTimeTMF->Enable(bKiosk);
            mpPage3BreakTMF->Enable(bKiosk);
            mpPage3LogoCB->Enable(bKiosk);
            break;
        }
    }
}

// ********************************************************************
// UI-Handler
// ********************************************************************

IMPL_LINK_TYPED( AssistentDlgImpl, SelectRegionHdl, ListBox&, rLB, void )
{
    if( &rLB == mpPage1RegionLB )
    {
        SelectTemplateRegion( rLB.GetSelectEntry() );
        SetStartType( ST_TEMPLATE );
        mpPage2Medium5RB->Check();
    }
    else
    {
        SelectLayoutRegion( rLB.GetSelectEntry() );
    }
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, SelectEffectHdl, ListBox&, void)
{
    maEffectPrevIdle.Start();
}

IMPL_LINK_NOARG_TYPED( AssistentDlgImpl, OpenButtonHdl, Button*, void )
{
    // Clear the selection and forward the call.
    mpPage1OpenLB->SetNoSelection();
    mpPage1OpenLB->GetDoubleClickHdl().Call(*mpPage1OpenLB);
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, EffectPreviewIdleHdl, Idle *, void)
{
    if(mbPreview && xDocShell.Is() )
    {
        SfxObjectShell* pShell = xDocShell;
        DrawDocShell* pDocShell = dynamic_cast< DrawDocShell * >(pShell);
        if( pDocShell )
        {
            SdDrawDocument* pDoc = pDocShell->GetDoc();
            if( pDoc )
            {
                SdPage* pPage = pDoc->GetSdPage( mnShowPage, PK_STANDARD );
                if( pPage )
                    mpPage3EffectLB->applySelected(pPage);
            }
        }
        mpPreview->startPreview();
    }
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, EffectPreviewClickHdl, SdDocPreviewWin&, void)
{
    EffectPreviewIdleHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, PreviewFlagHdl, Button*, void)
{
    if( mpPreviewFlag->IsChecked() != mbPreview )
    {
        mbPreview = mpPreviewFlag->IsChecked();
        UpdatePreview(true);
    }
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, SelectTemplateHdl, ListBox&, void)
{
    SetStartType( ST_TEMPLATE );
    mpPage2Medium5RB->Check();
    mpPage2LayoutLB->SelectEntryPos(0);
    maPrevIdle.Start();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, SelectLayoutHdl, ListBox&, void)
{
    maPrevIdle.Start();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, SelectFileHdl, ListBox&, void)
{
    SetStartType( ST_OPEN );
    maPrevIdle.Start();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, PageSelectHdl, SvTreeListBox*, void)
{
    sal_uInt16 nPage = mpPage5PageListCT->GetSelectedPage();
    if( mnShowPage != nPage )
    {
        mnShowPage = nPage;
        UpdatePreview(false);
    }
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, UpdatePageListHdl, Idle *, void)
{
    UpdatePageList();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, UpdatePreviewHdl, Idle *, void)
{
    UpdatePreview( true );
}

IMPL_LINK_TYPED( AssistentDlgImpl, StartTypeHdl, Button *, pButton, void )
{
    StartType eType = pButton == mpPage1EmptyRB ? ST_EMPTY : pButton == mpPage1TemplateRB ? ST_TEMPLATE:ST_OPEN;

    if(eType == ST_TEMPLATE)
        ProvideTemplates();
    else if(eType == ST_OPEN)
        ScanDocmenu();

    SetStartType( eType );

    if(eType == ST_TEMPLATE)
    {
        mpPage1TemplateLB->SelectEntryPos(0);
        mpPage2Medium5RB->Check();
    }
    else if(eType == ST_OPEN)
        mpPage1OpenLB->SelectEntryPos(0);

    maPrevIdle.Start();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, NextPageHdl, Button*, void)
{
    // When changing from the first to the second page make sure that the
    // templates are present.
    if (maAssistentFunc.GetCurrentPage() == 1)
        ProvideTemplates();

    // Change to the next page.
    LeavePage();
    maAssistentFunc.NextPage();
    ChangePage();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, LastPageHdl, Button*, void)
{
    LeavePage();
    maAssistentFunc.PreviousPage();
    ChangePage();
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, PresTypeHdl, Button*, void)
{
    if (maDocFile.isEmpty())
    {
        mpNextPageButton->Enable(false);
    }

    bool bKiosk = mpPage3PresTypeKioskRB->IsChecked();
    mpPage3PresTimeFT->Enable(bKiosk);
    mpPage3BreakFT->Enable(bKiosk);
    mpPage3PresTimeTMF->Enable(bKiosk);
    mpPage3BreakTMF->Enable(bKiosk);
    mpPage3LogoCB->Enable(bKiosk);
}

IMPL_LINK_NOARG_TYPED(AssistentDlgImpl, UpdateUserDataHdl, Edit&, void)
{
    mbUserDataDirty = true;
    OUString aTopic = mpPage4AskTopicEDT->GetText();
    OUString aName  = mpPage4AskNameEDT->GetText();
    OUString aInfo  = mpPage4AskInfoEDT->GetText();

    if (aTopic.isEmpty() && aName.isEmpty() && aInfo.isEmpty())
        maDocFile.clear();
}

// ********************************************************************
// ********************************************************************

void AssistentDlgImpl::SelectTemplateRegion( const OUString& rRegion )
{
    mpPage1TemplateLB->Clear();
    std::vector<TemplateDir*>::iterator I;
    for (I=maPresentList.begin(); I!=maPresentList.end(); ++I)
    {
        TemplateDir * pDir = *I;
        mpTemplateRegion = *I;
        if (pDir->msRegion == rRegion)
        {
            std::vector<TemplateEntry*>::iterator   J;
            for (J=pDir->maEntries.begin(); J!=pDir->maEntries.end(); ++J)
                mpPage1TemplateLB->InsertEntry ((*J)->msTitle);
            mpPage1TemplateLB->Update();
            if(GetStartType() == ST_TEMPLATE)
            {
                mpPage1TemplateLB->SelectEntryPos( 0 );
                SelectTemplateHdl(*mpPage1TemplateLB);
            }
            break;
        }
    }
}

void AssistentDlgImpl::SelectLayoutRegion( const OUString& rRegion )
{
    mpPage2LayoutLB->Clear();
    mpPage2LayoutLB->InsertEntry(SD_RESSTR(STR_WIZARD_ORIGINAL));
    std::vector<TemplateDir*>::iterator I;
    for (I=maPresentList.begin(); I!=maPresentList.end(); ++I)
    {
        TemplateDir * pDir = *I;
        mpLayoutRegion = *I;

        if (pDir->msRegion == rRegion)
        {
            std::vector<TemplateEntry*>::iterator   J;
            for (J=pDir->maEntries.begin(); J!=pDir->maEntries.end(); ++J)
                mpPage2LayoutLB->InsertEntry ((*J)->msTitle);
            mpPage2LayoutLB->Update();
            break;
        }
    }
}

void AssistentDlgImpl::UpdateUserData()
{
    OUString aTopic = mpPage4AskTopicEDT->GetText();
    OUString aName  = mpPage4AskNameEDT->GetText();
    OUString aInfo  = mpPage4AskInfoEDT->GetText();

    SfxObjectShell* pShell = xDocShell;
    DrawDocShell* pDocShell = dynamic_cast< DrawDocShell *>( pShell );
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;
    SdPage* pPage = pDoc?pDoc->GetSdPage(0, PK_STANDARD):NULL;

    if (pPage && (!aTopic.isEmpty() || !aName.isEmpty() || !aInfo.isEmpty()))
    {
        if( pPage->GetAutoLayout() == AUTOLAYOUT_NONE )
            pPage->SetAutoLayout(AUTOLAYOUT_TITLE, true);

        SdrTextObj* pObj;

        if (!aTopic.isEmpty())
        {
            pObj  = dynamic_cast<SdrTextObj*>( pPage->GetPresObj( PRESOBJ_TITLE ) );
            if( pObj )
            {
                pPage->SetObjText( pObj, NULL, PRESOBJ_TITLE, aTopic );
                pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE ), true );
                pObj->SetEmptyPresObj(false);
            }

        }

        if (!aName.isEmpty() || !aInfo.isEmpty())
        {
            OUString aStrTmp( aName );
            if (!aName.isEmpty())
                aStrTmp += "\n\n";
            aStrTmp += aInfo;

            pObj = dynamic_cast<SdrTextObj*>( pPage->GetPresObj( PRESOBJ_OUTLINE ) );
            if( pObj )
            {
                pPage->SetObjText( pObj, NULL, PRESOBJ_OUTLINE, aStrTmp );
                pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE ), true );
                pObj->SetEmptyPresObj(false);
            }
            else
            {
                pObj = dynamic_cast<SdrTextObj*>( pPage->GetPresObj( PRESOBJ_TEXT ) );
                if( pObj )
                {
                    pPage->SetObjText( pObj, NULL, PRESOBJ_TEXT, aStrTmp );
                    pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT ), true );
                    pObj->SetEmptyPresObj(false);
                }
            }
        }
    }

    mbUserDataDirty = false;
}

void AssistentDlgImpl::UpdatePageList()
{
    if(mbDocPreview || !mbPreview)
        UpdatePreview(false);
    else if(maPageListFile == maDocFile)
        return;

    maPageListFile = maDocFile;

    SfxObjectShell* pShell = xDocShell;
    DrawDocShell* pDocShell = dynamic_cast< DrawDocShell *>( pShell );
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    mpPage5PageListCT->Clear();

    if(pDoc)
        mpPage5PageListCT->Fill(pDoc);
}

void AssistentDlgImpl::UpdatePreview( bool bDocPreview )
{
    // Guard against multiple concurrent execution to this method caused either
    // by calls from different threads or recursion.
    ::osl::MutexGuard aGuard (maMutex);
    if (mbPreviewUpdating)
        return;
    mbPreviewUpdating = true;

    if(!mbPreview && bDocPreview)
    {
        mpPreview->Invalidate();
        mpPreview->SetObjectShell(0);
        mbPreviewUpdating = false;
        return;
    }

    OUString aDocFile = GetDocFileName();
    OUString aLayoutFile = GetLayoutFileName();

    SfxApplication *pSfxApp = SfxGetpApp();
    sal_uLong lErr;
    bool bChangeMaster = !aLayoutFile.isEmpty();

    if (aDocFile.isEmpty())
    {
        if (!xDocShell.Is() || !maDocFile.isEmpty() ||
            (maDocFile.isEmpty() && !maLayoutFile.isEmpty() && aLayoutFile.isEmpty()))
        {
            CloseDocShell();

            DrawDocShell* pNewDocSh;
            xDocShell = pNewDocSh = new DrawDocShell(SfxObjectCreateMode::STANDARD, false);
            pNewDocSh->DoInitNew(NULL);
            SdDrawDocument* pDoc = pNewDocSh->GetDoc();
            pDoc->CreateFirstPages();
            pDoc->StopWorkStartupDelay();
            mbDocPreview = false;

            maDocFile = aDocFile;
            mbUserDataDirty = true;
        }
        else
            bChangeMaster = !aLayoutFile.isEmpty() && (maLayoutFile != aLayoutFile);
    }
    else if( aDocFile == maDocFile && ( mbDocPreview == bDocPreview || bDocPreview ) )
    {
        if( aLayoutFile != maLayoutFile )
        {
            SfxObjectShell* pShell = xDocShell;
            DrawDocShell* pDocShell = dynamic_cast< DrawDocShell *>( pShell );
            ::svl::IUndoManager* pUndoMgr = pDocShell?pDocShell->GetUndoManager():NULL;
            if(pUndoMgr)
                pUndoMgr->Undo();
            mbUserDataDirty = true;
        }
        else
            bChangeMaster = false;
    }
    else
    {
        CloseDocShell();

        vcl::Window *pParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( mpWindow );

        SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,mpWindow);

        if(IsOwnFormat(aDocFile))
        {
            SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
            pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );
            if(bDocPreview)
                pSet->Put( SfxBoolItem( SID_PREVIEW, true ) );
            RestorePassword( pSet, aDocFile );
            if( (lErr = pSfxApp->LoadTemplate( xDocShell, aDocFile, true, pSet )) != 0 )
                ErrorHandler::HandleError(lErr);
            else
                SavePassword( xDocShell, aDocFile );
        }
        else
        {
            SfxRequest aReq( SID_OPENDOC, SfxCallMode::SYNCHRON, SfxGetpApp()->GetPool() );
            aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aDocFile ));
            aReq.AppendItem( SfxStringItem( SID_REFERER, OUString() ) );
            aReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_default") ) );
            aReq.AppendItem( SfxBoolItem( SID_HIDDEN, true ) );
            aReq.AppendItem( SfxBoolItem( SID_PREVIEW, bDocPreview ) );

            const SfxViewFrameItem* pRet = dynamic_cast<const SfxViewFrameItem*>( SfxGetpApp()->ExecuteSlot( aReq )  );

            if ( pRet && pRet->GetFrame() && pRet->GetFrame()->GetObjectShell() )
                xDocShell = pRet->GetFrame()->GetObjectShell();
        }

        Application::SetDefDialogParent( pParent );

        mnShowPage = 0;
        mbDocPreview = bDocPreview;
        maDocFile = aDocFile;
        mbUserDataDirty = true;
    }

    if(bChangeMaster && (aLayoutFile != maDocFile))
    {
        // load layout template
        SfxObjectShellLock xLayoutDocShell;
        SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,mpWindow);

        vcl::Window *pParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( mpWindow );

        if(IsOwnFormat(aLayoutFile))
        {
            SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
            pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );
            pSet->Put( SfxBoolItem( SID_PREVIEW, true ) );

            RestorePassword( pSet, aLayoutFile );
            if( (lErr = pSfxApp->LoadTemplate( xLayoutDocShell, aLayoutFile, true, pSet )) != 0 )
                ErrorHandler::HandleError(lErr);
            SavePassword( xLayoutDocShell, aLayoutFile );
        }

        Application::SetDefDialogParent( pParent );

        // determine the implementation
        SfxObjectShell* pShell = xDocShell;
        DrawDocShell* pDocShell = dynamic_cast< DrawDocShell *>( pShell );
        SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

        pShell = xLayoutDocShell;
        pDocShell = dynamic_cast< DrawDocShell *>( pShell );
        SdDrawDocument* pLayoutDoc = pDocShell?pDocShell->GetDoc():NULL;

        if( pDoc && pLayoutDoc )
        {
            pDoc->SetMasterPage(0, OUString(), pLayoutDoc, true,  false );
        }
        else
        {
            OSL_FAIL("sd::AssistentDlgImpl::UpdatePreview(), no document for preview?");
        }

        mbUserDataDirty = true;
    }
    maLayoutFile = aLayoutFile;

    if(mbUserDataDirty)
        UpdateUserData();

    if ( !xDocShell.Is() || !mbPreview )
        mpPreview->SetObjectShell( 0 );
    else
    {
        mpPreview->SetObjectShell( xDocShell, mnShowPage );
    }

    mbPreviewUpdating = false;
}

void AssistentDlgImpl::SavePassword( SfxObjectShellLock xDoc, const OUString& rPath )
{
    if(xDoc.Is())
    {
        SfxMedium * pMedium = xDoc->GetMedium();
        if(pMedium && pMedium->IsStorage())
        {
          SfxItemSet * pSet = pMedium->GetItemSet();
          const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_ENCRYPTIONDATA, false);
          uno::Sequence < beans::NamedValue > aEncryptionData;
          if (pEncryptionDataItem)
              pEncryptionDataItem->GetValue() >>= aEncryptionData;
          else
              return;
          if( aEncryptionData.getLength() )
          {

            PasswordEntry* pEntry = NULL;
            for ( size_t i = 0, n = maPasswordList.size(); i < n; ++i )
            {
                if ( maPasswordList[ i ].maPath == rPath )
                {
                    pEntry = &maPasswordList[ i ];
                    break;
                }
            }

            if(pEntry == NULL)
            {
                pEntry = new PasswordEntry();
                pEntry->maPath = rPath;
                maPasswordList.push_back( pEntry );
            }

            pEntry->aEncryptionData = aEncryptionData;
          }
        }
    }
}

void AssistentDlgImpl::RestorePassword( SfxItemSet* pSet, const OUString& rPath )
{
    uno::Sequence < beans::NamedValue > aEncryptionData( GetPassword( rPath ) );

    if(aEncryptionData.getLength())
        pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
}

uno::Sequence < beans::NamedValue > AssistentDlgImpl::GetPassword( const OUString& rPath )
{
    for ( size_t i = 0, n = maPasswordList.size(); i < n; ++i )
    {
        PasswordEntry* pEntry = &maPasswordList[ i ];
        if(pEntry->maPath == rPath)
            return pEntry->aEncryptionData;
    }
    return uno::Sequence < beans::NamedValue > ();;
}

void AssistentDlgImpl::DeletePasswords()
{
    maPasswordList.clear();
}

bool AssistentDlgImpl::IsOwnFormat( const OUString& rPath )
{
    INetURLObject   aURL( rPath );
    OUString        aExt( aURL.GetFileExtension() );

    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    return !aExt.equalsIgnoreAsciiCase( "ppt" );
}

OUString AssistentDlgImpl::GetUiTextForCommand (const OUString& sCommandURL)
{
    OUString sLabel;
    Reference<container::XNameAccess> xUICommandLabels;

    try
    {
        do
        {
            if (sCommandURL.isEmpty())
                break;

            // Retrieve popup menu labels
            Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            if ( ! xContext.is())
                break;

            Reference<container::XNameAccess> const xNameAccess(
                    frame::theUICommandDescription::get(xContext) );
            Any a = xNameAccess->getByName(
                "com.sun.star.presentation.PresentationDocument");
            a >>= xUICommandLabels;
            if ( ! xUICommandLabels.is())
                break;

            OUString sString;
            Sequence<beans::PropertyValue> aProperties;
            Any aAny (xUICommandLabels->getByName(sCommandURL));
            if (aAny >>= aProperties)
            {
                sal_Int32 nCount (aProperties.getLength());
                for (sal_Int32 i=0; i<nCount; i++)
                {
                    OUString sPropertyName (aProperties[i].Name);
                    if ( sPropertyName == "Label" )
                    {
                        aProperties[i].Value >>= sString;
                        break;
                    }
                }
            }
            sLabel = sString;
        }
        while(false);
    }
    catch (com::sun::star::uno::Exception& )
    {
    }

    return sLabel;
}

Image AssistentDlgImpl::GetUiIconForCommand (const OUString& sCommandURL)
{
    Image aIcon;
    Reference<container::XNameAccess> xUICommandLabels;

    try
    {
        do
        {
            if (sCommandURL.isEmpty())
                break;

            // Retrieve popup menu labels
            Reference<uno::XComponentContext> xContext ( ::comphelper::getProcessComponentContext() );
            if ( ! xContext.is())
                break;

            Reference<ui::XModuleUIConfigurationManagerSupplier> xSupplier (
                ui::theModuleUIConfigurationManagerSupplier::get(xContext));

            Reference<com::sun::star::ui::XUIConfigurationManager> xManager (
                xSupplier->getUIConfigurationManager(
                    "com.sun.star.presentation.PresentationDocument"));
            if ( ! xManager.is())
                break;

            Reference<com::sun::star::ui::XImageManager> xImageManager (
                xManager->getImageManager(),
                UNO_QUERY_THROW);

            Sequence<OUString> aCommandList(1);
            aCommandList[0] = sCommandURL;
            Sequence<Reference<graphic::XGraphic> > xIconList (
                xImageManager->getImages(0,aCommandList));
            if ( ! xIconList.hasElements())
                break;

            aIcon = Image(Graphic(xIconList[0]).GetBitmapEx());
        }
        while(false);
    }
    catch (com::sun::star::uno::Exception& )
    {
    }

    return aIcon;
}

AssistentDlg::AssistentDlg(vcl::Window* pParent, bool bAutoPilot) :
    ModalDialog(pParent, "Assistent", "modules/simpress/ui/assistentdialog.ui")
{
    mpImpl = new AssistentDlgImpl( this, LINK(this,AssistentDlg, FinishHdl2), bAutoPilot );

    // button assignment
    mpImpl->mpFinishButton->SetClickHdl( LINK(this,AssistentDlg,FinishHdl) );
}

IMPL_LINK_NOARG_TYPED(AssistentDlg, FinishHdl, Button*, void)
{
    FinishHdl();
}

IMPL_LINK_NOARG_TYPED(AssistentDlg, FinishHdl2, ListBox&, void)
{
    FinishHdl();
}

void AssistentDlg::FinishHdl()
{
    if( GetStartType() == ST_OPEN )
    {
        //if we do not have a file here asked for one before ending the dialog
        OUString aFileToOpen = GetDocPath();
        if (aFileToOpen.isEmpty())
        {
            sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0,
                OUString("simpress") );

            if ( aFileDlg.Execute() == ERRCODE_NONE )
                aFileToOpen = aFileDlg.GetPath();
            if (aFileToOpen.isEmpty())
                return;
            else
            {
                //add the selected file to the recent-file-listbox and select the new entry
                //this is necessary for 'GetDocPath()' returning the selected file after end of dialog

                INetURLObject aURL;
                aURL.SetSmartURL(aFileToOpen);
                mpImpl->maOpenFilesList.push_back (aURL.GetMainURL(INetURLObject::NO_DECODE));
                const sal_Int32 nNewPos = mpImpl->mpPage1OpenLB->InsertEntry(aURL.getName());
                mpImpl->mpPage1OpenLB->SelectEntryPos(nNewPos);
            }
        }
    }

    // end
    mpImpl->EndDialog(RET_OK);
    EndDialog(RET_OK);
}

AssistentDlg::~AssistentDlg()
{
    disposeOnce();
}

void AssistentDlg::dispose()
{
    delete mpImpl;
    ModalDialog::dispose();
}

SfxObjectShellLock AssistentDlg::GetDocument()
{
    return mpImpl->GetDocument();
}

OutputType AssistentDlg::GetOutputMedium() const
{
    if(mpImpl->mpPage2Medium1RB->IsChecked())
        return OUTPUT_PRESENTATION;
    else if(mpImpl->mpPage2Medium2RB->IsChecked())
        return OUTPUT_SLIDE;
    else if(mpImpl->mpPage2Medium3RB->IsChecked())
        return OUTPUT_OVERHEAD;
    else if(mpImpl->mpPage2Medium4RB->IsChecked())
        return OUTPUT_PAGE;
    else if(mpImpl->mpPage2Medium6RB->IsChecked())
        return OUTPUT_WIDESCREEN;
    else
        return OUTPUT_ORIGINAL;
}

bool AssistentDlg::IsSummary() const
{
    return mpImpl->mpPage5SummaryCB->IsChecked();
}

StartType AssistentDlg::GetStartType() const
{
    return mpImpl->GetStartType();
}

OUString AssistentDlg::GetDocPath() const
{
    return mpImpl->GetDocFileName();
}

bool AssistentDlg::GetStartWithFlag() const
{
    return !mpImpl->mpStartWithFlag->IsChecked();
}

bool AssistentDlg::IsDocEmpty() const
{
    return mpImpl->GetDocFileName().isEmpty() &&
           mpImpl->GetLayoutFileName().isEmpty();
}

uno::Sequence< beans::NamedValue > AssistentDlg::GetPassword()
{
    return mpImpl->GetPassword( mpImpl->maDocFile );
}

//===== NextButton ============================================================

NextButton::NextButton (vcl::Window* pParent) :
      mbIsFirstButtonActive(true)
{
    AssistentDlg* assDlg = static_cast<AssistentDlg*>(pParent);
    assDlg->get(mpNextButton1, "nextPage1Button");
    assDlg->get(mpNextButton2, "nextPage2Button");
    // Hide the unused button.
    mpNextButton2->Hide();
}

void NextButton::ForceFocusEventBroadcast()
{
    // Hide the currently visible button and show and focus the other one.
    if (mbIsFirstButtonActive)
    {
        mbIsFirstButtonActive = false;
        mpNextButton2->Show();
        mpNextButton2->GrabFocus();
        mpNextButton1->Hide();
    }
    else
    {
        mbIsFirstButtonActive = true;
        mpNextButton1->Show();
        mpNextButton1->GrabFocus();
        mpNextButton2->Hide();
    }
}

void NextButton::SetClickHdl (const Link<Button*,void>& rLink)
{
    // Forward the setting of the click handler to the two buttons
    // regardless of which one is currently visible.
    mpNextButton1->SetClickHdl(rLink);
    mpNextButton2->SetClickHdl(rLink);
}

bool NextButton::IsEnabled()
{
    // Because the buttons are both either enabled or disabled, it is
    // sufficient to ask one to determine the state.
    return mpNextButton1->IsEnabled();
}

void NextButton::Enable (bool bEnable)
{
    // Enable or disable both buttons but do not change visibility or focus.
    mpNextButton1->Enable(bEnable);
    mpNextButton2->Enable(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
