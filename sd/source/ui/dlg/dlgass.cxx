/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgass.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:06:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

#ifndef _SFXDOCTEMPL_HXX
#include <sfx2/doctempl.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SD_FADEDEF_H
#include <fadedef.h>
#endif

#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif

#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#include <vcl/wintypes.hxx>

#ifndef _SD_DOCPREV_HXX_
#include "docprev.hxx"
#endif

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif

#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#include "sdpage.hxx"
#include "helpids.h"
#ifndef INC_ASSCLASS
#include "assclass.hxx"
#endif
#include "dlgass.hrc"
#ifndef INC_DLGASS
#include "dlgass.hxx"
#endif
#ifndef SD_DLGCTRLS_HXX
#include "dlgctrls.hxx"
#endif
#ifndef _SD_CFGID_HXX
#include "strings.hrc"
#endif
#ifndef _DATETIMEITEM_HXX
#include "dlgassim.hxx"
#endif
#ifndef _TEMPLATE_SCANNER_HXX
#include "TemplateScanner.hxx"
#endif
#ifndef SD_OUTPUT_DEVICE_UPDATER_HXX
#include "WindowUpdater.hxx"
#endif

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd;


void InterpolateFixedBitmap( FixedBitmap * pBitmap )
{
    Bitmap aBmp( pBitmap->GetBitmap() );
    Size aSize = pBitmap->GetSizePixel();
    aBmp.Scale( aSize, BMP_SCALE_INTERPOLATE );
    pBitmap->SetBitmap( aBmp );
}


// ====================================================================
// ====================================================================

UINT32 PageHelpIds[] =
{
    HID_SD_AUTOPILOT_PAGE1,
    HID_SD_AUTOPILOT_PAGE2,
    HID_SD_AUTOPILOT_PAGE3,
    HID_SD_AUTOPILOT_PAGE4,
    HID_SD_AUTOPILOT_PAGE5
};

// ====================================================================

class PasswordEntry
{
public:
    String m_aPassword;
    String m_aPath;
};

DECLARE_LIST( PasswordEntryList, PasswordEntry * );

// ====================================================================

class AssistentDlgImpl : public SfxListener
{
public:
    AssistentDlgImpl( ::Window* pWindow, const Link& rFinishLink, BOOL bAutoPilot  );
    ~AssistentDlgImpl();

    /// Local mutex used to serialize concurrent method calls.
    ::osl::Mutex m_aMutex;

    SfxObjectShellLock GetDocument();

    /** closes the current preview docshell */
    void CloseDocShell();

    /** Extract form the history list of recently used files the impress
        files and insert them into a listbox.
    */
    void    ScanDocmenu         (void);
    /** Flag that is set to TRUE after the recently used files have been
        scanned.
    */
    BOOL m_bRecentDocumentsReady;

    /** When the list of templates has not been scanned already this is done
        when this method is called.  That includes requesting the whole list
        of templates from UCB and extracting from that list the impress
        templates and layouts and storing them for later use in
        <member>m_aPresentList</member>.  Note that the first call to this
        method after installing a new Office may take some time.
    */
    void ProvideTemplates (void);

    /** This method transfers the template folders from the template scanner
        to the internal structures of this class.  On termination it sets
        the flag <member>m_bTemplatesReady</member> to <TRUE/> to indicate
        that the templates are available.
        @param rTemplateFolders
            This is a list of template folders.  This method takes ownership
            of the supplied entries by removing them from the list and
            transferring them to an internal structure.
    */
    void TemplateScanDone (std::vector<TemplateDir*>& rTemplateFolders);

    /** Flag that is set to TRUE after the impress templates have been
        scanned.
    */
    BOOL m_bTemplatesReady;

    /** Flag used to prevent nested or concurrent calls to the
        <member>UpdatePreview</memember> method.  A <TRUE/> value indicates
        that a preview update is currently active.
    */
    BOOL m_bPreviewUpdating;

    ::Window* m_pWindow;

    void SavePassword( SfxObjectShellLock xDoc, const String& rPath );
    void RestorePassword( SfxItemSet* pSet, const String& rPath );
    String GetPassword( const String rPath );
    void DeletePassords();

    PasswordEntryList m_aPasswordList;

    String m_aDocFile;
    String m_aLayoutFile;

    String GetDocFileName();
    String GetLayoutFileName();

    /// List of URLs of recently used impress files.
    std::vector<String*> m_aOpenFilesList;

    /// List of folders containing data about impress templates.
    std::vector<TemplateDir*> m_aPresentList;

    /// Currently selected template folder.
    TemplateDir* m_pTemplateRegion;

    /// Currently selected layout folder.
    TemplateDir* m_pLayoutRegion;

    // preview
    BOOL m_bUserDataDirty;
    Timer m_aPrevTimer;
    Timer m_aEffectPrevTimer;
    Timer m_aUpdatePageListTimer;
    Timer m_aStartScanTimer;

    SfxObjectShellLock xDocShell;

    ::std::auto_ptr<WindowUpdater> mpWindowUpdater;

    BOOL m_bPreview;
    USHORT m_nShowPage;
    BOOL m_bDocPreview;

    ULONG m_nTemplate;

    String m_aPageListFile;

    void UpdatePreview( BOOL bDocPreview );
    void UpdatePageList();
    void UpdateUserData();

    BOOL IsOwnFormat( const String& rPath );

    // dlg status
    void EndDialog( long nResult = 0 );

    void SetStartType( StartType eType );
    StartType GetStartType();

    void SelectTemplateRegion( const String& rRegion );
    void SelectLayoutRegion( const String& rRegion );

    void UpdatePage();
    void ChangePage();
    void LeavePage();

    String GetUiTextForCommand (const ::rtl::OUString& aCommandURL);
    Image GetUiIconForCommand (const ::rtl::OUString& aCommandURL);

    DECL_LINK( StartScanHdl, void * );
    DECL_LINK( SelectFileHdl, ListBox * );
    DECL_LINK( SelectRegionHdl, ListBox * );
    DECL_LINK( UpdatePreviewHdl, void * );
    DECL_LINK( UpdatePageListHdl, void * );
    DECL_LINK( StartTypeHdl, RadioButton * );
    DECL_LINK( SelectTemplateHdl, ListBox * );
    DECL_LINK( NextPageHdl, PushButton * );
    DECL_LINK( LastPageHdl, PushButton * );
    DECL_LINK( PreviewFlagHdl, CheckBox * );
    DECL_LINK( EffectPreviewHdl, Button * );
    DECL_LINK( SelectLayoutHdl, ListBox * );
    DECL_LINK( PageSelectHdl, Control * );
    DECL_LINK( PresTypeHdl, RadioButton * );
    DECL_LINK( UpdateUserDataHdl, Edit* );
    DECL_LINK( SelectEffectHdl, void* );
    DECL_LINK( OpenButtonHdl, Button * );

    // Common
    Assistent           m_aAssistentFunc;
    CheckBox            m_aPreviewFlag;
    CheckBox            m_aStartWithFlag;
    HelpButton          m_aHelpButton;
    CancelButton        m_aCancelButton;
    PushButton          m_aLastPageButton;
    PushButton          m_aNextPageButton;
    OKButton            m_aFinishButton;
    SdDocPreviewWin     m_aPreview;

    // Seite 1
    FixedBitmap*        m_pPage1FB;
    FixedLine*          m_pPage1ArtFL;
    RadioButton*        m_pPage1EmptyRB;
    RadioButton*        m_pPage1TemplateRB;
    ListBox*            m_pPage1RegionLB;
    ListBox*            m_pPage1TemplateLB;
    RadioButton*        m_pPage1OpenRB;
    ListBox*            m_pPage1OpenLB;
    PushButton*         m_pPage1OpenPB;

    // Seite 2
    FixedBitmap*        m_pPage2FB;
    FixedLine*          m_pPage2LayoutFL;
    ListBox*            m_pPage2RegionLB;
    ListBox*            m_pPage2LayoutLB;
    FixedLine*          m_pPage2OutTypesFL;
    RadioButton*        m_pPage2Medium1RB;
    RadioButton*        m_pPage2Medium2RB;
    RadioButton*        m_pPage2Medium3RB;
    RadioButton*        m_pPage2Medium4RB;
    RadioButton*        m_pPage2Medium5RB;

    // Seite 3
    FixedBitmap*        m_pPage3FB;
    FixedLine*          m_pPage3EffectFL;
    FixedText*          m_pPage3EffectFT;
    FadeEffectLB*       m_pPage3EffectLB;
    FixedText*          m_pPage3SpeedFT;
    ListBox*            m_pPage3SpeedLB;
    FixedLine*          m_pPage3PresTypeFL;
    RadioButton*        m_pPage3PresTypeLiveRB;
    RadioButton*        m_pPage3PresTypeKioskRB;
    FixedText*          m_pPage3PresTimeFT;
    TimeField*          m_pPage3PresTimeTMF;
    FixedText*          m_pPage3BreakFT;
    TimeField*          m_pPage3BreakTMF;
    CheckBox*           m_pPage3LogoCB;

    // Seite 4
    FixedBitmap*        m_pPage4FB;
    FixedLine*          m_pPage4PersonalFL;
    FixedText*          m_pPage4AskNameFT;
    Edit*               m_pPage4AskNameEDT;
    FixedText*          m_pPage4AskTopicFT;
    Edit*               m_pPage4AskTopicEDT;
    FixedText*          m_pPage4AskInfoFT;
    MultiLineEdit*      m_pPage4AskInfoEDT;

    // Seite 5
    FixedBitmap*        m_pPage5FB;
    FixedText*          m_pPage5PageListFT;
    SdPageListControl*  m_pPage5PageListCT;
    CheckBox*           m_pPage5SummaryCB;

};



// ====================================================================

AssistentDlgImpl::AssistentDlgImpl( ::Window* pWindow, const Link& rFinishLink, BOOL bAutoPilot ) :
    m_aFinishButton(pWindow,SdResId(BUT_FINISH)),
    m_aHelpButton(pWindow,SdResId(BUT_HELP)),
    m_aCancelButton(pWindow,SdResId(BUT_CANCEL)),
    m_aNextPageButton(pWindow,SdResId(BUT_NEXT)),
    m_aLastPageButton(pWindow,SdResId(BUT_LAST)),
    m_aPreviewFlag(pWindow,SdResId(CB_PREVIEW)),
    m_aPreview(pWindow,SdResId(CT_PREVIEW)),
    m_aStartWithFlag(pWindow,SdResId(CB_STARTWITH)),
//  m_aBitmap(pWindow,SdResId(PAGE1_BMP)),
    m_pTemplateRegion(NULL),
    m_pLayoutRegion(NULL),
    m_bPreview(TRUE),
    m_bDocPreview(FALSE),
    m_nShowPage(0),
//  m_aPageListFile('?'),
    m_bUserDataDirty(FALSE),
    m_aAssistentFunc(5),
    xDocShell (NULL),
    mpWindowUpdater (new WindowUpdater())
{
    m_aPageListFile += sal_Unicode('?'),
    m_bRecentDocumentsReady = FALSE;
    m_bTemplatesReady = FALSE;
    m_bPreviewUpdating = FALSE;

    m_pWindow = pWindow;

    if(bAutoPilot)
        m_aStartWithFlag.Hide();
    else
        m_aAssistentFunc.InsertControl(1, &m_aStartWithFlag );

    //page1 wird initialisiert und an die Assistentenfunktionalitaet
    //uebergeben
    m_aAssistentFunc.InsertControl(1, &m_aPreview );
    m_aAssistentFunc.InsertControl(1, &m_aPreviewFlag );
    m_aAssistentFunc.InsertControl(1,
        m_pPage1FB = new FixedBitmap(pWindow,SdResId(FB_PAGE1)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1ArtFL = new FixedLine(pWindow,SdResId(FL_PAGE1_ARTGROUP)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1EmptyRB=new RadioButton(pWindow,SdResId(RB_PAGE1_EMPTY)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1TemplateRB=new RadioButton(pWindow,SdResId(RB_PAGE1_TEMPLATE)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1OpenRB=new RadioButton(pWindow,SdResId(RB_PAGE1_OPEN)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1RegionLB = new ListBox(pWindow,SdResId(LB_PAGE1_REGION)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1TemplateLB=new ListBox(pWindow,SdResId(LB_PAGE1_TEMPLATES)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1OpenPB=new PushButton(pWindow,SdResId(PB_PAGE1_OPEN)));
    m_aAssistentFunc.InsertControl(1,
        m_pPage1OpenLB=new ListBox(pWindow,SdResId(LB_PAGE1_OPEN)));

    // Align the button and list box displayed for the "open existing file"
    // radio button with the text of that radio button.
    {
        RadioButton aEmptyRB (m_pWindow);
        sal_Int32 nIndent (aEmptyRB.CalcMinimumSize(0).Width());
        sal_Int32 nLeft (m_pPage1OpenRB->GetPosPixel().X() + nIndent);
        sal_Int32 nWidth (m_pPage1OpenRB->GetSizePixel().Width() - nIndent);
        m_pPage1OpenPB->SetPosSizePixel(
            Point(nLeft, m_pPage1OpenPB->GetPosPixel().Y()),
            Size(m_pPage1OpenPB->GetSizePixel()));
        m_pPage1OpenLB->SetPosSizePixel(
            Point(nLeft, m_pPage1OpenLB->GetPosPixel().Y()),
            Size(nWidth, m_pPage1OpenLB->GetSizePixel().Height()));
    }

    // Set text and icon of the 'Open...' button.
    {
        String sText (GetUiTextForCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Open"))));
        // Remove the mnemonic and add a leading space so that icon and text
        // are not too close together.
        sText.SearchAndReplaceAll(String(RTL_CONSTASCII_STRINGPARAM("~")),String());
        sText.Insert(String(RTL_CONSTASCII_STRINGPARAM(" ")),0);
        m_pPage1OpenPB->SetText(sText);
        // Place icon left of text and both centered in the button.
        m_pPage1OpenPB->SetModeImage(
            GetUiIconForCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Open"))),
            BMP_COLOR_NORMAL);
        m_pPage1OpenPB->EnableImageDisplay(TRUE);
        m_pPage1OpenPB->EnableTextDisplay(TRUE);
        m_pPage1OpenPB->SetImageAlign(IMAGEALIGN_LEFT);
        m_pPage1OpenPB->SetStyle(m_pPage1OpenPB->GetStyle() | WB_CENTER);
    }

    // links&handler
    m_pPage1RegionLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectRegionHdl));
    m_pPage1RegionLB->SetDropDownLineCount( 6 );
    m_pPage1TemplateLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectTemplateHdl));
    m_pPage1TemplateLB->InsertEntry(String(SdResId(STR_ISLOADING)));

    m_pPage1EmptyRB->SetClickHdl(LINK(this,AssistentDlgImpl,StartTypeHdl));
    m_pPage1TemplateRB->SetClickHdl(LINK(this,AssistentDlgImpl,StartTypeHdl));
    m_pPage1OpenRB->SetClickHdl(LINK(this,AssistentDlgImpl,StartTypeHdl));
    m_pPage1OpenLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectFileHdl));
    m_pPage1OpenLB->SetDoubleClickHdl(rFinishLink);
    m_pPage1OpenPB->SetClickHdl(LINK(this,AssistentDlgImpl,OpenButtonHdl));
    //  m_pPage1OpenLB->InsertEntry(String(SdResId(STR_WIZARD_POSITION)));

    // Seite 2
    m_aAssistentFunc.InsertControl(2, &m_aPreview );
    m_aAssistentFunc.InsertControl(2, &m_aPreviewFlag );
    m_aAssistentFunc.InsertControl(2,
        m_pPage2FB = new FixedBitmap(pWindow,SdResId(FB_PAGE2)));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2LayoutFL = new FixedLine( pWindow, SdResId(FL_PAGE2_LAYOUT) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2RegionLB = new ListBox(pWindow,SdResId(LB_PAGE2_REGION) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2LayoutLB = new ListBox(pWindow,SdResId(LB_PAGE2_LAYOUT) ));

    m_aAssistentFunc.InsertControl(2,
        m_pPage2OutTypesFL = new FixedLine( pWindow, SdResId(FL_PAGE2_OUTPUTTYPES) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium5RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM5) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium3RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM3) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium4RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM4) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium1RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM1) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium2RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM2) ));
    m_pPage2Medium5RB->Check();

    m_pPage2RegionLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectRegionHdl));
    m_pPage2RegionLB->SetDropDownLineCount( 6 );
    m_pPage2LayoutLB->SetSelectHdl(LINK(this,AssistentDlgImpl,SelectLayoutHdl));
    m_pPage2LayoutLB->InsertEntry(String(SdResId(STR_ISLOADING)));

    // Seite 3
    m_aAssistentFunc.InsertControl(3, &m_aPreview );
    m_aAssistentFunc.InsertControl(3, &m_aPreviewFlag );
    m_aAssistentFunc.InsertControl(3,
        m_pPage3FB = new FixedBitmap(pWindow,SdResId(FB_PAGE3)));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3EffectFL = new FixedLine( pWindow, SdResId(FL_PAGE3_EFFECT) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3EffectFT = new FixedText( pWindow, SdResId(FT_PAGE3_EFFECT) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3EffectLB = new FadeEffectLB( pWindow, SdResId(LB_PAGE3_EFFECT) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3SpeedFT = new FixedText( pWindow, SdResId(FT_PAGE3_SPEED) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3SpeedLB = new FadeEffectLB( pWindow, SdResId(LB_PAGE3_SPEED) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3PresTypeFL = new FixedLine( pWindow, SdResId(FL_PAGE3_PRESTYPE) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3PresTypeLiveRB = new RadioButton( pWindow, SdResId(RB_PAGE3_LIVE) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3PresTypeKioskRB = new RadioButton( pWindow, SdResId(RB_PAGE3_KIOSK) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3PresTimeFT = new FixedText( pWindow, SdResId( FT_PAGE3_TIME) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3PresTimeTMF = new TimeField( pWindow, SdResId( TMF_PAGE3_TIME) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3BreakFT = new FixedText( pWindow, SdResId( FT_PAGE3_BREAK) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3BreakTMF = new TimeField( pWindow, SdResId( TMF_PAGE3_BREAK) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3LogoCB = new CheckBox( pWindow, SdResId( CB_PAGE3_LOGO) ));

    m_pPage3EffectLB->Fill();
//  m_pPage3EffectLB->SelectEffect( presentation::FadeEffect_NONE );
    m_pPage3EffectLB->SetSelectHdl( LINK(this,AssistentDlgImpl,SelectEffectHdl ));
    m_pPage3EffectLB->SetDropDownLineCount( 12 );

    m_pPage3SpeedLB->InsertEntry( String( SdResId(STR_SLOW) ));
    m_pPage3SpeedLB->InsertEntry( String( SdResId(STR_MEDIUM) ));
    m_pPage3SpeedLB->InsertEntry( String( SdResId(STR_FAST) ));
    m_pPage3SpeedLB->SetDropDownLineCount( 3 );
    m_pPage3SpeedLB->SetSelectHdl( LINK(this,AssistentDlgImpl,SelectEffectHdl ));
    m_pPage3SpeedLB->SelectEntryPos( 1 );

    m_pPage3PresTypeLiveRB->Check();
    m_pPage3PresTypeLiveRB->SetClickHdl( LINK(this,AssistentDlgImpl, PresTypeHdl ));
    m_pPage3PresTypeKioskRB->SetClickHdl( LINK(this,AssistentDlgImpl, PresTypeHdl ));
    m_pPage3PresTimeTMF->SetFormat( TIMEF_SEC );
    m_pPage3PresTimeTMF->SetTime( Time( 0, 0, 10 ) );
    m_pPage3BreakTMF->SetFormat( TIMEF_SEC );
    m_pPage3BreakTMF->SetTime( Time( 0, 0, 10 ) );
    m_pPage3LogoCB->Check();

    // set cursor in timefield
    Edit *pEditPage3PresTimeTMF = m_pPage3PresTimeTMF->GetField();
    Edit *pEditPage3BreakTMF = m_pPage3BreakTMF->GetField();
    Selection aSel1( pEditPage3PresTimeTMF->GetMaxTextLen(), pEditPage3PresTimeTMF->GetMaxTextLen() );
    Selection aSel2( pEditPage3BreakTMF->GetMaxTextLen(), pEditPage3BreakTMF->GetMaxTextLen() );
    pEditPage3PresTimeTMF->SetSelection( aSel1 );
    pEditPage3BreakTMF->SetSelection( aSel2 );

    // Seite 4
    m_aAssistentFunc.InsertControl(4,
        m_pPage4FB = new FixedBitmap(pWindow,SdResId(FB_PAGE4)));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4PersonalFL = new FixedLine( pWindow, SdResId(FL_PAGE4_PERSONAL) ));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4AskNameFT   = new FixedText( pWindow, SdResId(FT_PAGE4_ASKNAME) ));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4AskNameEDT  = new Edit( pWindow, SdResId(EDT_PAGE4_ASKNAME) ));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4AskTopicFT= new FixedText( pWindow, SdResId(FT_PAGE4_ASKTOPIC) ));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4AskTopicEDT = new Edit( pWindow, SdResId(EDT_PAGE4_ASKTOPIC) ));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4AskInfoFT   = new FixedText( pWindow, SdResId(FT_PAGE4_ASKINFORMATION) ));
    m_aAssistentFunc.InsertControl(4,
        m_pPage4AskInfoEDT  = new MultiLineEdit( pWindow, SdResId(EDT_PAGE4_ASKINFORMATION) ));

    m_pPage4AskNameEDT->SetModifyHdl(LINK(this,AssistentDlgImpl,UpdateUserDataHdl));
    m_pPage4AskTopicEDT->SetModifyHdl(LINK(this,AssistentDlgImpl,UpdateUserDataHdl));
    m_pPage4AskInfoEDT->SetModifyHdl(LINK(this,AssistentDlgImpl,UpdateUserDataHdl));

    // page 5
    m_aAssistentFunc.InsertControl(5, &m_aPreview );
    m_aAssistentFunc.InsertControl(5, &m_aPreviewFlag );
    m_aAssistentFunc.InsertControl(5,
        m_pPage5FB = new FixedBitmap(pWindow,SdResId(FB_PAGE5)));
    m_aAssistentFunc.InsertControl(5,
        m_pPage5PageListFT = new FixedText( pWindow, SdResId( FT_PAGE5_PAGELIST ) ));
    m_aAssistentFunc.InsertControl(5,
        m_pPage5PageListCT = new SdPageListControl( pWindow, SdResId( CT_PAGE5_PAGELIST ) ));
    m_aAssistentFunc.InsertControl(5,
        m_pPage5SummaryCB  = new CheckBox( pWindow, SdResId( CB_PAGE5_SUMMARY ) ));

    m_pPage5PageListCT->SetSelectHdl(LINK(this,AssistentDlgImpl, PageSelectHdl));

    // generell
    InterpolateFixedBitmap( m_pPage1FB );
    InterpolateFixedBitmap( m_pPage2FB );
    InterpolateFixedBitmap( m_pPage3FB );
    InterpolateFixedBitmap( m_pPage4FB );
    InterpolateFixedBitmap( m_pPage5FB );

    m_aLastPageButton.SetClickHdl(LINK(this,AssistentDlgImpl, LastPageHdl ));
    m_aNextPageButton.SetClickHdl(LINK(this,AssistentDlgImpl, NextPageHdl ));
    m_aPreviewFlag.Check( m_bPreview );
    m_aPreviewFlag.SetClickHdl(LINK(this, AssistentDlgImpl, PreviewFlagHdl ));
    m_aPreview.SetClickHdl(LINK(this,AssistentDlgImpl, EffectPreviewHdl ));

    //setzt die Ausgangsseite
    m_aAssistentFunc.GotoPage(1);
    m_aLastPageButton.Disable();

    m_aPrevTimer.SetTimeout( 200 );
    m_aPrevTimer.SetTimeoutHdl( LINK( this, AssistentDlgImpl, UpdatePreviewHdl));

    m_aEffectPrevTimer.SetTimeout( 50 );
    m_aEffectPrevTimer.SetTimeoutHdl( LINK( this, AssistentDlgImpl, EffectPreviewHdl ));

    m_aUpdatePageListTimer.SetTimeout( 50 );
    m_aUpdatePageListTimer.SetTimeoutHdl( LINK( this, AssistentDlgImpl, UpdatePageListHdl));

    SetStartType( ST_EMPTY );

    ChangePage();

    mpWindowUpdater->RegisterWindow (&m_aPreview);

    UpdatePreview( TRUE );

    //check wether we should start with a template document initialy and preselect it
    const ::rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) );
    String aStandardTemplate( SfxObjectFactory::GetStandardTemplate( aServiceName ) );
    if( aStandardTemplate.Len() )
    {
        ProvideTemplates();

        //find aStandardTemplate in m_aPresentList
        TemplateDir*   pStandardTemplateDir = 0;
        TemplateEntry* pStandardTemplateEntry = 0;

        std::vector<TemplateDir*>::iterator I;
        for (I=m_aPresentList.begin(); I!=m_aPresentList.end(); I++)
        {
            TemplateDir* pDir = *I;
            std::vector<TemplateEntry*>::iterator   J;
            for (J=pDir->maEntries.begin(); J!=pDir->maEntries.end(); J++)
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
            m_pPage1RegionLB->SelectEntry( pStandardTemplateDir->msRegion );
            SelectTemplateRegion( pStandardTemplateDir->msRegion );
            m_pPage1TemplateLB->SelectEntry( pStandardTemplateEntry->msTitle );
            SelectTemplateHdl(m_pPage1TemplateLB);
        }
    }
}




AssistentDlgImpl::~AssistentDlgImpl()
{
    CloseDocShell();

    DeletePassords();

    //  Delete the template file infos.
    std::vector<TemplateDir*>::iterator I;
    std::vector<TemplateEntry*>::iterator   J;
    for (I=m_aPresentList.begin(); I!=m_aPresentList.end(); I++)
    {
        for (J=(*I)->maEntries.begin(); J!=(*I)->maEntries.end(); J++)
            delete (*J);
        delete (*I);
    }

    // Seite 1
    delete m_pPage1FB;
    delete m_pPage1ArtFL;
    delete m_pPage1EmptyRB;
    delete m_pPage1TemplateRB;
    delete m_pPage1TemplateLB;
    delete m_pPage1RegionLB;
    delete m_pPage1OpenRB;
    delete m_pPage1OpenLB;
    delete m_pPage1OpenPB;

    // Seite 2
    delete m_pPage2FB;
    delete m_pPage2LayoutFL;
    delete m_pPage2RegionLB;
    delete m_pPage2LayoutLB;
    delete m_pPage2OutTypesFL;
    delete m_pPage2Medium1RB;
    delete m_pPage2Medium2RB;
    delete m_pPage2Medium3RB;
    delete m_pPage2Medium4RB;
    delete m_pPage2Medium5RB;

    // Seite 3
    delete m_pPage3FB;
    delete m_pPage3EffectFL;
    delete m_pPage3EffectFT;
    delete m_pPage3EffectLB;
    delete m_pPage3SpeedFT;
    delete m_pPage3SpeedLB;
    delete m_pPage3PresTypeFL;
    delete m_pPage3PresTypeLiveRB;
    delete m_pPage3PresTypeKioskRB;
    delete m_pPage3PresTimeFT;
    delete m_pPage3PresTimeTMF;
    delete m_pPage3BreakFT;
    delete m_pPage3BreakTMF;
    delete m_pPage3LogoCB;

    // Seite 4
    delete m_pPage4FB;
    delete m_pPage4PersonalFL;
    delete m_pPage4AskNameFT;
    delete m_pPage4AskNameEDT;
    delete m_pPage4AskTopicFT;
    delete m_pPage4AskTopicEDT;
    delete m_pPage4AskInfoFT;
    delete m_pPage4AskInfoEDT;

    // Seite 5
    delete m_pPage5FB;
    delete m_pPage5PageListFT;
    delete m_pPage5PageListCT;
    delete m_pPage5SummaryCB;

    //  Delete the file history list.
    std::vector<String*>::iterator  I2;
    for (I2=m_aOpenFilesList.begin(); I2!=m_aOpenFilesList.end(); I2++)
        delete *I2;
}

void AssistentDlgImpl::CloseDocShell()
{
    if(xDocShell.Is())
    {
        //uno::Reference< lang::XComponent > xModel( xDocShell->GetModel(), uno::UNO_QUERY );
        uno::Reference< util::XCloseable > xCloseable( xDocShell->GetModel(), uno::UNO_QUERY );
        //if( xModel.is() )
        if( xCloseable.is() )
        {
            xCloseable->close( sal_True );
            xDocShell = NULL;
            //xModel->dispose();
        }
        else
        {
            xDocShell->DoClose();
            xDocShell = NULL;
        }
    }
}

void AssistentDlgImpl::EndDialog( long nResult )
{
    m_pWindow = NULL;
}




void    AssistentDlgImpl::ScanDocmenu   (void)
{
    if( m_bRecentDocumentsReady )
        return;

    uno::Sequence<uno::Sequence<beans::PropertyValue> > aHistory =
        SvtHistoryOptions().GetList (ePICKLIST);

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference< container::XNameAccess > xFilterFactory( xFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ) ) ), uno::UNO_QUERY );

    Reference< ::com::sun::star::ucb::XSimpleFileAccess > xFileAccess(
        xFactory->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
        UNO_QUERY_THROW);

    sal_uInt32 nCount = aHistory.getLength();
    for (sal_uInt32 nItem=0; nItem<nCount; ++nItem)
    {
        //  Get the current history item's properties.
        uno::Sequence<beans::PropertyValue> aPropertySet = aHistory[nItem];
        rtl::OUString   sURL;
        rtl::OUString   sFilter;
        rtl::OUString   sTitle;
        rtl::OUString   sPassword;
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

        sal_Int32 nCount = lProps.getLength();
        rtl::OUString sFactoryName;
        for( sal_Int32 i=0; i<nCount; ++i )
        {
            if( lProps[i].Name.compareToAscii( "DocumentService" ) == 0 &&
                (lProps[i].Value >>= sFactoryName) &&
                sFactoryName.compareToAscii( "com.sun.star.presentation.PresentationDocument" ) == 0 )
            {
                // yes, it's an impress document
                INetURLObject aURL;

                // Do not include the file if it does not exist.
                if (xFileAccess.is() && ! xFileAccess->exists(sURL))
                    continue;

                aURL.SetSmartURL (sURL);
                // The password is set only when it is not empty.
                if (sPassword.getLength() > 0)
                    aURL.SetPass (sPassword);
                m_aOpenFilesList.push_back (new String (aURL.GetMainURL( INetURLObject::NO_DECODE )));
                m_pPage1OpenLB->InsertEntry (sTitle);
                break;
            }
        }
    }
    m_bRecentDocumentsReady = TRUE;
    try
    {
        UpdatePreview(TRUE);
    }
    catch (uno::RuntimeException& )
    {
        // Ignore all exceptions.
    }
}



void AssistentDlgImpl::ProvideTemplates (void)
{
    if ( ! m_bTemplatesReady)
    {
        TemplateScanner aScanner;
        aScanner.Scan ();
        TemplateScanDone (aScanner.GetFolderList());

        try
        {
            UpdatePreview(TRUE);
        }
        catch (uno::RuntimeException& e)
        {
            // Ignore all exceptions.
            (void) e;
        }
    }
}

void AssistentDlgImpl::TemplateScanDone (
    std::vector<TemplateDir*>& rTemplateFolder)
{
    //  This method is called from a thread.  Therefore we get the solar mutex.
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    // Copy the contents of the given template folders to a local list.
    m_aPresentList.swap (rTemplateFolder);

    //  Fill in the list box on the first page.
    int nFirstEntry = 0;
    m_pPage1RegionLB->Clear();
    std::vector<TemplateDir*>::iterator I;
    int i;
    for (i=0,I=m_aPresentList.begin(); I!=m_aPresentList.end(); I++,i++)
    {
        TemplateDir* pDir = *I;
        if (pDir == NULL)
            continue;

        // HACK! presnt directory is always initially selected.
        // We have to look at the first entry to get a URL.
        if (pDir->maEntries.size() > 0)
        {
            TemplateEntry* pEntry = pDir->maEntries.front();
            if (pEntry != NULL)
                if (pEntry->msPath.SearchAscii("presnt") != STRING_NOTFOUND)
                    nFirstEntry = i;
        }

        m_pPage1RegionLB->InsertEntry (pDir->msRegion);
    }
    m_pPage1RegionLB->SelectEntryPos (nFirstEntry);
    m_pPage1RegionLB->Update();
    SelectTemplateRegion (m_pPage1RegionLB->GetSelectEntry());

    //  Fill in the list box on the second page.
    nFirstEntry = 0;
    m_pPage2RegionLB->Clear();
    for (i=0,I=m_aPresentList.begin(); I!=m_aPresentList.end(); I++,i++)
    {
        TemplateDir* pDir = *I;
        if (pDir == NULL)
            continue;

        // HACK! layout directory is always initially selected.
        // We have to look at the first entry to get a URL.
        if (pDir->maEntries.size() > 0)
        {
            TemplateEntry* pEntry = pDir->maEntries.front();
            if (pEntry != NULL)
                if (pEntry->msPath.SearchAscii("layout") != STRING_NOTFOUND)
                    nFirstEntry = i;
        }

        m_pPage2RegionLB->InsertEntry (pDir->msRegion);
    }
    m_pPage2RegionLB->SelectEntryPos (nFirstEntry);
    m_pPage2RegionLB->Update();
    SelectLayoutRegion (m_pPage2RegionLB->GetSelectEntry());

    //  Make the changes visible.
    m_bTemplatesReady = TRUE;
    if (m_pWindow)
        UpdatePage();
}




// ********************************************************************
// Status Methoden
// ********************************************************************

void AssistentDlgImpl::SetStartType( StartType eType )
{
    m_pPage1EmptyRB->SetState( eType == ST_EMPTY );
    m_pPage1TemplateRB->SetState( eType == ST_TEMPLATE );
    m_pPage1OpenRB->SetState( eType == ST_OPEN );
    m_aNextPageButton.Enable( eType != ST_OPEN );

    m_pPage1RegionLB->Show(eType == ST_TEMPLATE);
    m_pPage1TemplateLB->Show(eType == ST_TEMPLATE);
    m_pPage1OpenLB->Show(eType == ST_OPEN);
    m_pPage1OpenPB->Show(eType == ST_OPEN);
}

StartType AssistentDlgImpl::GetStartType()
{
    if( m_pPage1EmptyRB->IsChecked() )
        return ST_EMPTY;
    else if( m_pPage1TemplateRB->IsChecked() )
        return ST_TEMPLATE;
    else
        return ST_OPEN;
}

String AssistentDlgImpl::GetDocFileName()
{
    String aTitle;
    if(m_pWindow)
    {
        aTitle = m_pWindow->GetText();
        USHORT nPos = aTitle.Search(sal_Unicode('('));
        if(nPos != STRING_NOTFOUND)
            aTitle.Erase( nPos-1 );
    }

    String aDocFile;
    if( GetStartType() == ST_TEMPLATE )
    {
        const USHORT nEntry = m_pPage1TemplateLB->GetSelectEntryPos();
        TemplateEntry* pEntry = NULL;
        if(nEntry != (USHORT)-1)
            pEntry = m_pTemplateRegion->maEntries[nEntry];

        if(pEntry)
        {
            aDocFile = pEntry->msPath;

            aTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM(  " (" ) );
            aTitle.Append( pEntry->msTitle );
            aTitle.Append( sal_Unicode(')') );
        }
    }
    else if( GetStartType() == ST_OPEN )
    {
        const USHORT nEntry = m_pPage1OpenLB->GetSelectEntryPos();
        if(nEntry != (USHORT)-1 && nEntry >= 0)
            aDocFile = *m_aOpenFilesList[nEntry];
    }

    if(m_pWindow)
        m_pWindow->SetText(aTitle);

    return aDocFile;
}

String AssistentDlgImpl::GetLayoutFileName()
{
    String aFile;
    const USHORT nEntry = m_pPage2LayoutLB->GetSelectEntryPos();
    TemplateEntry* pEntry = NULL;
    if(nEntry != (USHORT)-1 && nEntry > 0)
        pEntry = m_pLayoutRegion->maEntries[nEntry-1];

    if(pEntry)
        aFile = pEntry->msPath;

    return aFile;
}

SfxObjectShellLock AssistentDlgImpl::GetDocument()
{
//  m_bPreview = FALSE;     // Document nicht anzeigen
    UpdatePreview(FALSE);   // aber komplett laden
    UpdatePageList();

    SfxObjectShell* pShell = xDocShell;
    ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell,pShell);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    if(pDoc)
    {
        const USHORT nPageCount = pDoc->GetSdPageCount(PK_STANDARD);
        BOOL bKiosk = m_pPage3PresTypeKioskRB->IsChecked();
        UINT32 nNewTime = (UINT32)m_pPage3PresTimeTMF->GetTime().GetMSFromTime() / 1000;
        if(bKiosk)
        {
            PresentationSettings& rSettings = pDoc->getPresentationSettings();
            rSettings.mbEndless = bKiosk;
            rSettings.mnPauseTimeout = (sal_Int32)m_pPage3BreakTMF->GetTime().GetMSFromTime() / 1000;
            rSettings.mbShowPauseLogo = m_pPage3LogoCB->IsChecked();
        }

        USHORT nPgAbsNum = 0;
        USHORT nPgRelNum = 0;
        while( nPgAbsNum < nPageCount )
        {
            SdPage* pPage = pDoc->GetSdPage( nPgRelNum, PK_STANDARD );
            if( m_pPage5PageListCT->IsPageChecked(nPgAbsNum) )
            {
                m_pPage3EffectLB->applySelected(pPage);
                const sal_uInt16 nPos = m_pPage3SpeedLB->GetSelectEntryPos();
                pPage->setTransitionDuration( (nPos == 0) ? 3.0 : (nPos == 1) ? 2.0 : 1.0 );
                if(bKiosk)
                {
                    pPage->SetPresChange( PRESCHANGE_AUTO );
                    pPage->SetTime(nNewTime);
                }
                nPgRelNum++;
            }
            else
            {
                // diese Seite loeschen
                pDoc->DeletePage( (nPgRelNum << 1) + 2 ); // Notizseite loeschen
                pDoc->DeletePage( (nPgRelNum << 1) + 1 );   // Seite loeschen
            }

            nPgAbsNum++;
        }
    }
    else
        DBG_ERROR("Keine Vorlage fuer den Autopiloten? [CL]");

    SfxObjectShellLock xRet = xDocShell;
    xDocShell = NULL;

    return xRet;
}

void AssistentDlgImpl::LeavePage()
{
    USHORT nPage = m_aAssistentFunc.GetCurrentPage();

    if( nPage == 4 && m_bUserDataDirty )
        m_aPrevTimer.Start();
}

void AssistentDlgImpl::ChangePage()
{
    m_aNextPageButton.Enable(!m_aAssistentFunc.IsLastPage());
    m_aLastPageButton.Enable(!m_aAssistentFunc.IsFirstPage());

    USHORT nPage = m_aAssistentFunc.GetCurrentPage();

    if( m_pWindow )
        m_pWindow->SetHelpId( PageHelpIds[nPage-1]);

    UpdatePage();

    if( m_aNextPageButton.IsEnabled() )
        m_aNextPageButton.GrabFocus();
    else
        m_aFinishButton.GrabFocus();
}

void AssistentDlgImpl::UpdatePage()
{
    USHORT nPage = m_aAssistentFunc.GetCurrentPage();

    switch(nPage)
    {
    case 1:
        {
            // Elemente auf der ersten Seite abhaengig vom Starttype Zeigen
            SetStartType( GetStartType() );
            m_pPage1TemplateRB->Enable(TRUE /*m_bTemplatesReady*/);
            break;
        }

    case 2:
        {
            m_pPage2RegionLB->Enable(m_bTemplatesReady);
            m_pPage2LayoutLB->Enable(m_bTemplatesReady);

            if( GetStartType() != ST_EMPTY )
            {
                m_pPage2Medium5RB->Enable( TRUE );
            }
            else
            {
                m_pPage2Medium5RB->Enable( FALSE );
                if(m_pPage2Medium5RB->IsChecked())
                    m_pPage2Medium1RB->Check();
            }

            break;
        }
    case 5:
        {
            if(m_bDocPreview || m_aPageListFile != m_aDocFile)
                m_pPage5PageListCT->Clear();

            m_aUpdatePageListTimer.Start();
            break;
        }

    case 3:
        {
            if(GetStartType() != ST_TEMPLATE)
                m_aNextPageButton.Disable();

            BOOL bKiosk = m_pPage3PresTypeKioskRB->IsChecked();
            m_pPage3PresTimeFT->Enable(bKiosk);
            m_pPage3BreakFT->Enable(bKiosk);
            m_pPage3PresTimeTMF->Enable(bKiosk);
            m_pPage3BreakTMF->Enable(bKiosk);
            m_pPage3LogoCB->Enable(bKiosk);
            break;
        }
    }
}

// ********************************************************************
// UI-Handler
// ********************************************************************

IMPL_LINK( AssistentDlgImpl, SelectRegionHdl, ListBox *, pLB )
{
    if( pLB == m_pPage1RegionLB )
    {
        SelectTemplateRegion( pLB->GetSelectEntry() );
        SetStartType( ST_TEMPLATE );
        m_pPage2Medium5RB->Check();
    }
    else
    {
        SelectLayoutRegion( pLB->GetSelectEntry() );
    }

    return 0;
}

IMPL_LINK( AssistentDlgImpl, SelectEffectHdl, void*, EMPTYARG )
{
    m_aEffectPrevTimer.Start();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, OpenButtonHdl, Button*, pButton )
{
    // Clear the selection and forward the call.
    m_pPage1OpenLB->SetNoSelection();
    return m_pPage1OpenLB->GetDoubleClickHdl().Call(pButton);
}

IMPL_LINK( AssistentDlgImpl, EffectPreviewHdl, Button *, EMPTYARG )
{
    if(m_bPreview && xDocShell.Is() )
    {
        SfxObjectShell* pShell = xDocShell;
        DrawDocShell* pDocShell = dynamic_cast< DrawDocShell * >(pShell);
        if( pDocShell )
        {
            SdDrawDocument* pDoc = pDocShell->GetDoc();
            if( pDoc )
            {
                SdPage* pPage = pDoc->GetSdPage( m_nShowPage, PK_STANDARD );
                if( pPage )
                    m_pPage3EffectLB->applySelected(pPage);
            }
        }
        m_aPreview.startPreview();
    }
    return 0;
}

IMPL_LINK( AssistentDlgImpl, PreviewFlagHdl, CheckBox *, EMPTYARG )

{
    if( m_aPreviewFlag.IsChecked() != m_bPreview )
    {
        m_bPreview = m_aPreviewFlag.IsChecked();
        UpdatePreview(TRUE);
    }
    return 0;
}

IMPL_LINK( AssistentDlgImpl, SelectTemplateHdl, ListBox *, EMPTYARG )
{
    SetStartType( ST_TEMPLATE );
    m_pPage2Medium5RB->Check();
    m_pPage2LayoutLB->SelectEntryPos(0);
    m_aPrevTimer.Start();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, SelectLayoutHdl, ListBox *, EMPTYARG )
{
    m_aPrevTimer.Start();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, SelectFileHdl, ListBox *, EMPTYARG )
{
    SetStartType( ST_OPEN );
    m_aPrevTimer.Start();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, PageSelectHdl, Control *, EMPTYARG )
{
    USHORT nPage = m_pPage5PageListCT->GetSelectedPage();
    if( m_nShowPage != nPage )
    {
        m_nShowPage = nPage;
        UpdatePreview(FALSE);
    }

    return 0;
}

IMPL_LINK( AssistentDlgImpl, UpdatePageListHdl, void *, EMPTYARG )
{
    UpdatePageList();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, UpdatePreviewHdl, void *, EMPTYARG )
{
    UpdatePreview( TRUE );
    return 0;
}

IMPL_LINK( AssistentDlgImpl, StartTypeHdl, RadioButton *, pButton )
{
    StartType eType = pButton == m_pPage1EmptyRB?ST_EMPTY:pButton == m_pPage1TemplateRB?ST_TEMPLATE:ST_OPEN;

    if(eType == ST_TEMPLATE)
        ProvideTemplates();
    else if(eType == ST_OPEN)
        ScanDocmenu();

    SetStartType( eType );

    if(eType == ST_TEMPLATE)
    {
        m_pPage1TemplateLB->SelectEntryPos(0);
        m_pPage2Medium5RB->Check();
    }
    else if(eType == ST_OPEN)
        m_pPage1OpenLB->SelectEntryPos(0);

    m_aPrevTimer.Start();
    return 0;
}


IMPL_LINK( AssistentDlgImpl, NextPageHdl, PushButton *, EMPTYARG )
{
    // When changing from the first to the second page make sure that the
    // templates are present.
    if (m_aAssistentFunc.GetCurrentPage() == 1)
        ProvideTemplates();

    // Change to the next page.
    LeavePage();
    m_aAssistentFunc.NextPage();
    ChangePage();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, LastPageHdl, PushButton *, EMPTYARG )
{
    LeavePage();
    m_aAssistentFunc.PreviousPage();
    ChangePage();
    return 0;
}

IMPL_LINK( AssistentDlgImpl, PresTypeHdl, RadioButton*, pButton )
{
    if(m_aDocFile.Len() == 0)
        m_aNextPageButton.Disable();

    BOOL bKiosk = m_pPage3PresTypeKioskRB->IsChecked();
    m_pPage3PresTimeFT->Enable(bKiosk);
    m_pPage3BreakFT->Enable(bKiosk);
    m_pPage3PresTimeTMF->Enable(bKiosk);
    m_pPage3BreakTMF->Enable(bKiosk);
    m_pPage3LogoCB->Enable(bKiosk);
    return 0;
}

IMPL_LINK( AssistentDlgImpl, UpdateUserDataHdl, Edit*, EMPTYARG )
{
    m_bUserDataDirty = TRUE;
    String aTopic = m_pPage4AskTopicEDT->GetText();
    String aName  = m_pPage4AskNameEDT->GetText();
    String aInfo  = m_pPage4AskInfoEDT->GetText();

    if(aTopic.Len() == 0 && aName.Len() == 0 && aInfo.Len() == 0)
        m_aDocFile.Erase();

    return 0;
}

// ********************************************************************
// ********************************************************************

void AssistentDlgImpl::SelectTemplateRegion( const String& rRegion )
{
    m_pPage1TemplateLB->Clear();
    std::vector<TemplateDir*>::iterator I;
    for (I=m_aPresentList.begin(); I!=m_aPresentList.end(); I++)
    {
        TemplateDir * pDir = *I;
        m_pTemplateRegion = *I;
        if (pDir->msRegion.Equals( rRegion ) )
        {
            std::vector<TemplateEntry*>::iterator   J;
            for (J=pDir->maEntries.begin(); J!=pDir->maEntries.end(); J++)
                m_pPage1TemplateLB->InsertEntry ((*J)->msTitle);
            m_pPage1TemplateLB->Update();
            if(GetStartType() == ST_TEMPLATE)
            {
                m_pPage1TemplateLB->SelectEntryPos( 0 );
                SelectTemplateHdl(NULL);
            }
            break;
        }
    }
}

void AssistentDlgImpl::SelectLayoutRegion( const String& rRegion )
{
    m_pPage2LayoutLB->Clear();
    m_pPage2LayoutLB->InsertEntry(String(SdResId(STR_WIZARD_ORIGINAL)));
    std::vector<TemplateDir*>::iterator I;
    for (I=m_aPresentList.begin(); I!=m_aPresentList.end(); I++)
    {
        TemplateDir * pDir = *I;
        m_pLayoutRegion = *I;

        if (pDir->msRegion.Equals (rRegion))
        {
            std::vector<TemplateEntry*>::iterator   J;
            for (J=pDir->maEntries.begin(); J!=pDir->maEntries.end(); J++)
                m_pPage2LayoutLB->InsertEntry ((*J)->msTitle);
            m_pPage2LayoutLB->Update();
            break;
        }
    }
}

void AssistentDlgImpl::UpdateUserData()
{
    String aTopic = m_pPage4AskTopicEDT->GetText();
    String aName  = m_pPage4AskNameEDT->GetText();
    String aInfo  = m_pPage4AskInfoEDT->GetText();

    SfxObjectShell* pShell = xDocShell;
    DrawDocShell* pDocShell = PTR_CAST(DrawDocShell,pShell);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;
    SdPage* pPage = pDoc?pDoc->GetSdPage(0, PK_STANDARD):NULL;

    if(pPage && ( aTopic.Len() != 0 || aName.Len() != 0 || aInfo.Len() != 0 ) )
    {
        if( pPage->GetAutoLayout() == AUTOLAYOUT_NONE )
            pPage->SetAutoLayout(AUTOLAYOUT_TITLE, TRUE);

        SdrTextObj* pObj;
        String aEmptyString;

        if( aTopic.Len() )
        {
            pObj  = dynamic_cast<SdrTextObj*>( pPage->GetPresObj( PRESOBJ_TITLE ) );
            if( pObj )
            {
                pPage->SetObjText( pObj, NULL, PRESOBJ_TITLE, aTopic );
                pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE ), TRUE );
                pObj->SetEmptyPresObj(FALSE);
            }

        }

        if ( aName.Len() || aInfo.Len() )
        {
            String aStrTmp( aName );
            if( aName.Len() )
                aStrTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\n\n" ) );
            aStrTmp.Append( aInfo );

            pObj = dynamic_cast<SdrTextObj*>( pPage->GetPresObj( PRESOBJ_OUTLINE ) );
            if( pObj )
            {
                pPage->SetObjText( pObj, NULL, PRESOBJ_OUTLINE, aStrTmp );
                pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE ), TRUE );
                pObj->SetEmptyPresObj(FALSE);
            }
            else
            {
                pObj = dynamic_cast<SdrTextObj*>( pPage->GetPresObj( PRESOBJ_TEXT ) );
                if( pObj )
                {
                    pPage->SetObjText( pObj, NULL, PRESOBJ_TEXT, aStrTmp );
                    pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT ), TRUE );
                    pObj->SetEmptyPresObj(FALSE);
                }
            }
        }
    }

    m_bUserDataDirty = FALSE;
}

void AssistentDlgImpl::UpdatePageList()
{
    if(m_bDocPreview || !m_bPreview)
        UpdatePreview(FALSE);
    else if(m_aPageListFile == m_aDocFile)
        return;

    m_aPageListFile = m_aDocFile;

    SfxObjectShell* pShell = xDocShell;
    DrawDocShell* pDocShell = PTR_CAST(DrawDocShell,pShell);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    m_pPage5PageListCT->Clear();

    if(pDoc)
        m_pPage5PageListCT->Fill(pDoc);
}

void AssistentDlgImpl::UpdatePreview( BOOL bDocPreview )
{
    // Guard against multiple concurrent execution to this method caused either
    // by calls from different threads or recursion.
    ::osl::MutexGuard aGuard (m_aMutex);
    if (m_bPreviewUpdating)
        return;
    m_bPreviewUpdating = TRUE;

    if(!m_bPreview && bDocPreview)
    {
        m_aPreview.Invalidate();
        m_aPreview.SetObjectShell(0);
        m_bPreviewUpdating = FALSE;
        return;
    }

    String aDocFile = GetDocFileName();
    String aLayoutFile = GetLayoutFileName();
    String aEmptyStr;

    SfxApplication *pSfxApp = SFX_APP();
    ULONG lErr;
    BOOL bChangeMaster = aLayoutFile.Len() != 0;

    if( aDocFile.Len() == 0 )
    {
        if( !xDocShell.Is() || m_aDocFile.Len() != 0 ||
            (m_aDocFile.Len() == 0 && m_aLayoutFile.Len() != 0 && aLayoutFile.Len() == 0 ))
        {
            CloseDocShell();

            DrawDocShell* pNewDocSh;
            xDocShell = pNewDocSh = new DrawDocShell(SFX_CREATE_MODE_STANDARD, FALSE);
            pNewDocSh->DoInitNew(NULL);
            SdDrawDocument* pDoc = pNewDocSh->GetDoc();
            pDoc->CreateFirstPages();
            pDoc->StopWorkStartupDelay();
            m_bDocPreview = FALSE;

            m_aDocFile = aDocFile;
            m_bUserDataDirty = TRUE;
        }
        else
            bChangeMaster = (aLayoutFile.Len() != 0) && (m_aLayoutFile != aLayoutFile);
    }
    else if( aDocFile == m_aDocFile && ( m_bDocPreview == bDocPreview || bDocPreview ) )
    {
        if( aLayoutFile != m_aLayoutFile )
        {
            SfxObjectShell* pShell = xDocShell;
            DrawDocShell* pDocShell = PTR_CAST(DrawDocShell,pShell);
            SfxUndoManager* pUndoMgr = pDocShell?pDocShell->GetUndoManager():NULL;
            if(pUndoMgr)
                pUndoMgr->Undo();
            m_bUserDataDirty = TRUE;
        }
        else
            bChangeMaster = FALSE;
    }
    else
    {
        CloseDocShell();

        ::Window *pParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( m_pWindow );

        SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,m_pWindow);

        SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
        if(IsOwnFormat(aDocFile))
        {
            pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
            if(bDocPreview)
                pSet->Put( SfxBoolItem( SID_PREVIEW, TRUE ) );
            RestorePassword( pSet, aDocFile );
            if( lErr = pSfxApp->LoadTemplate( xDocShell, aDocFile, TRUE, pSet ) )
                ErrorHandler::HandleError(lErr);
            else
                SavePassword( xDocShell, aDocFile );
        }
        else
        {
            SfxObjectShell* pShell = NULL;

            const String aTargetStr( RTL_CONSTASCII_USTRINGPARAM("_default") );

            SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool() );
            aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aDocFile ));
            aReq.AppendItem( SfxStringItem( SID_REFERER, aEmptyStr ) );
            aReq.AppendItem( SfxStringItem( SID_TARGETNAME, aTargetStr ) );
            aReq.AppendItem( SfxBoolItem( SID_VIEW, FALSE ) );
            aReq.AppendItem( SfxBoolItem( SID_PREVIEW, bDocPreview ) );

            const SfxObjectShellItem* pRet = (SfxObjectShellItem*)
                SFX_APP()->ExecuteSlot( aReq );

            if( pRet && pRet->GetObjectShell() )
                xDocShell = pRet->GetObjectShell();
        }


        Application::SetDefDialogParent( pParent );

        m_nShowPage = 0;
        m_bDocPreview = bDocPreview;
        m_aDocFile = aDocFile;
        m_bUserDataDirty = TRUE;
    }

    if(bChangeMaster && (aLayoutFile != m_aDocFile))
    {
        // Layoutvorlage laden
        SfxObjectShellLock xLayoutDocShell;
        SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,m_pWindow);
        SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );

        ::Window *pParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( m_pWindow );

        if(IsOwnFormat(aLayoutFile))
        {
            pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
            pSet->Put( SfxBoolItem( SID_PREVIEW, TRUE ) );

            RestorePassword( pSet, aLayoutFile );
            if( lErr = pSfxApp->LoadTemplate( xLayoutDocShell, aLayoutFile, TRUE, pSet ) )
                ErrorHandler::HandleError(lErr);
            SavePassword( xLayoutDocShell, aLayoutFile );
        }

        Application::SetDefDialogParent( pParent );

        // die Implementierung ermitteln
        SfxObjectShell* pShell = xDocShell;
        DrawDocShell* pDocShell = PTR_CAST(DrawDocShell,pShell);
        SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

        pShell = xLayoutDocShell;
        pDocShell = PTR_CAST(DrawDocShell,pShell);
        SdDrawDocument* pLayoutDoc = pDocShell?pDocShell->GetDoc():NULL;

        if( pDoc && pLayoutDoc )
            pDoc->SetMasterPage(0, aEmptyStr, pLayoutDoc, TRUE,  FALSE );
        else
            DBG_ERROR("Keine gueltigen Impress Documente fuer die Preview? [CL]");
        m_bUserDataDirty = TRUE;
    }
    m_aLayoutFile = aLayoutFile;

    if(m_bUserDataDirty)
        UpdateUserData();

    if ( !xDocShell.Is() || !m_bPreview )
        m_aPreview.SetObjectShell( 0 );
    else
    {
        m_aPreview.SetObjectShell( xDocShell, m_nShowPage );
    }

    m_bPreviewUpdating = FALSE;
}

void AssistentDlgImpl::SavePassword( SfxObjectShellLock xDoc, const String& rPath )
{
    if(xDoc.Is())
    {
        SfxMedium * pMedium = xDoc->GetMedium();
        if(pMedium && pMedium->IsStorage())
        {
            SfxItemSet * pSet = pMedium->GetItemSet();
          const SfxPoolItem *pItem = 0;
          if( pSet->GetItemState(SID_PASSWORD, TRUE, &pItem) == SFX_ITEM_SET )
          {
            //TODO/MBA: testing
            String aPass( ((const SfxStringItem*)pItem)->GetValue());
            if(aPass.Len() == 0)
                return;

            PasswordEntry* pEntry = m_aPasswordList.First();
            while(pEntry)
            {
                if(pEntry->m_aPath == rPath)
                    break;

                pEntry = m_aPasswordList.Next();

            }

            if(pEntry == NULL)
            {
                pEntry = new PasswordEntry();
                pEntry->m_aPath = rPath;
                m_aPasswordList.Insert( pEntry );
            }

            if(pEntry)
                pEntry->m_aPassword = aPass;
          }
        }
    }
}

void AssistentDlgImpl::RestorePassword( SfxItemSet* pSet, const String& rPath )
{
    String aPassword( GetPassword( rPath ) );

    if(aPassword.Len())
        pSet->Put( SfxStringItem( SID_PASSWORD, aPassword ) );
}

String AssistentDlgImpl::GetPassword( const String rPath )
{
    PasswordEntry* pEntry = m_aPasswordList.First();
    while(pEntry)
    {
        if(pEntry->m_aPath == rPath)
            return pEntry->m_aPassword;

        pEntry = m_aPasswordList.Next();
    }

    return String();
}

void AssistentDlgImpl::DeletePassords()
{
    PasswordEntry* pEntry = m_aPasswordList.First();
    while(pEntry)
    {
        delete pEntry;
        pEntry = m_aPasswordList.Next();
    }
}

BOOL AssistentDlgImpl::IsOwnFormat( const String& rPath )
{
    INetURLObject   aURL( rPath );
    String          aExt( aURL.GetFileExtension() );

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    return !aExt.EqualsIgnoreCaseAscii( "ppt" );
}




String AssistentDlgImpl::GetUiTextForCommand (const ::rtl::OUString& sCommandURL)
{
    String sLabel;
    Reference<container::XNameAccess> xUICommandLabels;

    try
    {
        do
        {
            if (sCommandURL.getLength() ==  0)
                break;

            // Retrieve popup menu labels
            Reference<lang::XMultiServiceFactory> xFactory (
                ::comphelper::getProcessServiceFactory ());
            if ( ! xFactory.is())
                break;

            ::rtl::OUString sModuleIdentifier (
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"));
            Reference<container::XNameAccess> xNameAccess (
                xFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.frame.UICommandDescription"))),
                UNO_QUERY);
            if ( ! xNameAccess.is())
                break;
            Any a = xNameAccess->getByName(sModuleIdentifier);
            a >>= xUICommandLabels;
            if ( ! xUICommandLabels.is())
                break;

            ::rtl::OUString sString;
            Sequence<beans::PropertyValue> aProperties;
            Any aAny (xUICommandLabels->getByName(sCommandURL));
            if (aAny >>= aProperties)
            {
                sal_Int32 nCount (aProperties.getLength());
                for (sal_Int32 i=0; i<nCount; i++)
                {
                    ::rtl::OUString sPropertyName (aProperties[i].Name);
                    if (sPropertyName.equalsAscii("Label"))
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
    catch (com::sun::star::uno::Exception& rException)
    {
        (void)rException;
    }

    return sLabel;
}




Image AssistentDlgImpl::GetUiIconForCommand (const ::rtl::OUString& sCommandURL)
{
    Image aIcon;
    Reference<container::XNameAccess> xUICommandLabels;

    try
    {
        do
        {
            if (sCommandURL.getLength() ==  0)
                break;

            // Retrieve popup menu labels
            Reference<lang::XMultiServiceFactory> xFactory (
                ::comphelper::getProcessServiceFactory ());
            if ( ! xFactory.is())
                break;

            ::rtl::OUString sModuleIdentifier (
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"));

            Reference<com::sun::star::ui::XModuleUIConfigurationManagerSupplier> xSupplier (
                xFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.ui.ModuleUIConfigurationManagerSupplier"))),
                UNO_QUERY_THROW);

            Reference<com::sun::star::ui::XUIConfigurationManager> xManager (
                xSupplier->getUIConfigurationManager(sModuleIdentifier));
            if ( ! xManager.is())
                break;

            Reference<com::sun::star::ui::XImageManager> xImageManager (
                xManager->getImageManager(),
                UNO_QUERY_THROW);

            Sequence<rtl::OUString> aCommandList(1);
            aCommandList[0] = sCommandURL;
            Sequence<Reference<graphic::XGraphic> > xIconList (
                xImageManager->getImages(0,aCommandList));
            if ( ! xIconList.hasElements())
                break;

            aIcon = Graphic(xIconList[0]).GetBitmapEx();
        }
        while(false);
    }
    catch (com::sun::star::uno::Exception& rException)
    {
        (void)rException;
    }

    return aIcon;
}


//////////////////////////////////////////////

AssistentDlg::AssistentDlg(Window* pParent, BOOL bAutoPilot) :
    ModalDialog(pParent,SdResId(DLG_ASS))
{
    Link aFinishLink = LINK(this,AssistentDlg, FinishHdl);
    m_pImpl = new AssistentDlgImpl( this, aFinishLink, bAutoPilot );

    //Buttonbelegung
    m_pImpl->m_aFinishButton.SetClickHdl(LINK(this,AssistentDlg,FinishHdl));

    FreeResource();
}

IMPL_LINK( AssistentDlg, FinishHdl, OKButton *, EMPTYARG )
{
    if( GetStartType() == ST_OPEN )
    {
        //if we do not have a file here asked for one before ending the dialog
        String aFileToOpen = GetDocPath();
        if(aFileToOpen.Len() == 0)
        {
            sfx2::FileDialogHelper aFileDlg( WB_OPEN, ::String::CreateFromAscii("simpress") );

            if ( aFileDlg.Execute() == ERRCODE_NONE )
                aFileToOpen = aFileDlg.GetPath();
            if( aFileToOpen.Len() == 0)
                return 1;
            else
            {
                //add the selected file to the recent-file-listbox and select the new entry
                //this is necessary for 'GetDocPath()' returning the selected file after end of dialog

                INetURLObject aURL;
                aURL.SetSmartURL(aFileToOpen);
                m_pImpl->m_aOpenFilesList.push_back (new String (aURL.GetMainURL( INetURLObject::NO_DECODE )));
                USHORT nNewPos = m_pImpl->m_pPage1OpenLB->InsertEntry(aURL.getName());
                m_pImpl->m_pPage1OpenLB->SelectEntryPos(nNewPos);
            }
        }
    }

    //Ende
    m_pImpl->EndDialog(RET_OK);
    EndDialog(RET_OK);
    return 0;
}

AssistentDlg::~AssistentDlg()
{
    delete m_pImpl;
}


SfxObjectShellLock AssistentDlg::GetDocument()
{
    return m_pImpl->GetDocument();
}

String AssistentDlg::GetTopic() const
{
    return m_pImpl->m_pPage4AskTopicEDT->GetText();
}

String AssistentDlg::GetUserName() const
{
    return m_pImpl->m_pPage4AskNameEDT->GetText();
}

String AssistentDlg::GetInformation() const
{
    return m_pImpl->m_pPage4AskInfoEDT->GetText();
}

OutputType AssistentDlg::GetOutputMedium() const
{
    if(m_pImpl->m_pPage2Medium1RB->IsChecked())
        return OUTPUT_PRESENTATION;
    else if(m_pImpl->m_pPage2Medium2RB->IsChecked())
        return OUTPUT_SLIDE;
    else if(m_pImpl->m_pPage2Medium3RB->IsChecked())
        return OUTPUT_OVERHEAD;
    else if(m_pImpl->m_pPage2Medium4RB->IsChecked())
        return OUTPUT_PAGE;
    else
        return OUTPUT_ORIGINAL;
}

BOOL AssistentDlg::IsSummary() const
{
    return m_pImpl->m_pPage5SummaryCB->IsChecked();
}

StartType AssistentDlg::GetStartType() const
{
    return m_pImpl->GetStartType();
}

String AssistentDlg::GetDocPath() const
{
    return m_pImpl->GetDocFileName();
}

BOOL AssistentDlg::GetStartWithFlag() const
{
    return !m_pImpl->m_aStartWithFlag.IsChecked();
}

BOOL AssistentDlg::IsDocEmpty() const
{
    return m_pImpl->GetDocFileName().Len() == 0 &&
           m_pImpl->GetLayoutFileName().Len() == 0;
}

String AssistentDlg::GetPassword()
{
    return m_pImpl->GetPassword( m_pImpl->m_aDocFile );
}
