/*************************************************************************
 *
 *  $RCSfile: dlgass.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-30 15:45:02 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _SV_LSTBOX_HXX  // class ListBox
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX //class ComboBox
#include <vcl/combobox.hxx>
#endif

#ifndef _SFXDOCTEMPL_HXX // class SfxDocumentTemplate
#include <sfx2/doctempl.hxx>
#endif
#ifndef _SFXLSTNER_HXX // class SfxListener
#include <svtools/lstner.hxx>
#endif

/* no chaos, no fun
#ifndef _CNTAPI_HXX // class CntAnchor
#include <chaos/cntapi.hxx>
#endif

#ifndef _CNTRULES_HXX // Cnt Rules
#define ITEMID_RULESET WID_RULES
#include <chaos/cntrules.hxx>
#endif

#ifndef _SVTOOLS_CTYPEITM_HXX // CntContentTypeItem
#include <svtools/ctypeitm.hxx>
#endif

#ifndef _CNTSYS_HXX //!!!TEMP
#include <chaos/cntsys.hxx>
#endif

#ifndef _CNTCCITM_HXX // class CntCmpCommandItem
#include <chaos/cntccitm.hxx>
#endif

#ifndef _CSTRITEM_HXX // class CntStringItem
#include <chaos/cstritem.hxx>
#endif
*/

#ifndef _SFXDOCINF_HXX // SfxDocumentInfo
#include <sfx2/docinf.hxx>
#endif

#ifndef _SFX_OBJSH_HXX // SfxObjectShell
#include <sfx2/objsh.hxx>
#endif

#ifndef _EHDL_HXX // SfxErrorContext
#include <svtools/ehdl.hxx>
#endif

#ifndef _SFXECODE_HXX //ERRCTX_SFX_LOADTEMPLATE
#include <svtools/sfxecode.hxx>
#endif

#ifndef _URLOBJ_HXX // INetURLObject
#include <tools/urlobj.hxx>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SD_FADEDEF_H // enum FadeSpeed
#include <fadedef.h>
#endif

#ifndef _SFXSIDS_HRC //autogen
#include <sfx2/sfxsids.hrc>
#endif

#ifndef _UNDO_HXX // class SfxUndoManager
#include <svtools/undo.hxx>
#endif

#ifndef _SD_DOCSHELL_HXX
#include "docshell.hxx"
#endif

#ifndef _SV_GDIMTF_HXX //autogen
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SD_DOCPREV_HXX_
#include "docprev.hxx"
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "sdpage.hxx"
#include "helpids.h"
#include "assclass.hxx"
#include "dlgass.hrc"
#include "dlgass.hxx"
#include "dlgctrls.hxx"
#include "strings.hrc"
#include "dlgassim.hxx"

using namespace ::com::sun::star;

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

class TemplateEntry
{
public:
    TemplateEntry( const String& rTitle, const String& rPath ) { m_aTitle = rTitle; m_aPath = rPath; }

    String m_aTitle;
    String m_aPath;
};

DECLARE_LIST( TemplateEntryList, TemplateEntry * );


class TemplateDir
{
public:
    TemplateDir( const String& rRegion, const String& rUrl ) { m_aRegion = rRegion; m_aUrl = rUrl; }

    String m_aRegion;
    String m_aUrl;
    TemplateEntryList m_aEntrys;
};

DECLARE_LIST( TemplateDirList, TemplateDir * );
DECLARE_LIST( StringPtrList, String * );

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
    AssistentDlgImpl( Window* pWindow, const Link& rFinishLink, BOOL bAutoPilot  );
    ~AssistentDlgImpl();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );


    SfxObjectShellLock GetDocument();


    void TemplateScanDone();
//  void AddTemplateDir( CntAnchorRef xRoot );
//  void ScanTemplateDir( CntAnchorRef xDir );
    void ScanTemplates();
    void DocmenuScanDone();
    void ScanDocmenu();

    Window* m_pWindow;

    void SavePassword( SfxObjectShellLock xDoc, const String& rPath );
    void RestorePassword( SfxItemSet* pSet, const String& rPath );
    String GetPassword( const String rPath );
    void DeletePassords();

    // chaos
    String m_aDocFile;
    String m_aLayoutFile;

    String GetDocFileName();
    String GetLayoutFileName();

    StringPtrList m_aOpenFilesList;
    TemplateDir* m_pTemplateRegion;
    TemplateDir* m_pLayoutRegion;
//  CntAnchorRef m_xRoot;
//  CntAnchor* m_pTemplateDir;

    TemplateDirList m_aPresentList;
    PasswordEntryList m_aPasswordList;

    BOOL m_bTemplatesReady;
//  BOOL m_bOpenReady;

    // preview
    BOOL m_bUserDataDirty;
    Timer m_aPrevTimer;
    Timer m_aEffectPrevTimer;
    Timer m_aUpdatePageListTimer;
    Timer m_aStartScanTimer;

    SfxObjectShellLock xDocShell;

    BOOL m_bPreview;
    USHORT m_nShowPage;
    BOOL m_bDocPreview;

//  CntAnchorRef m_xDocmenu;
    ULONG m_nTemplate;

    String m_aPageListFile;

    void UpdatePreview( BOOL bDocPreview );
    void UpdatePageList();
    void UpdateUserData();

    // templates
    TemplateCache   m_aCache;

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

    // Common
    Assistent           m_aAssistentFunc;
    OKButton            m_aFinishButton;
    CancelButton        m_aCancelButton;
    HelpButton          m_aHelpButton;
    PushButton          m_aLastPageButton;
    PushButton          m_aNextPageButton;
    CheckBox            m_aPreviewFlag;
    SdDocPreviewWin     m_aPreview;
    CheckBox            m_aStartWithFlag;

    // Seite 1
    FixedBitmap*        m_pPage1FB;
    GroupBox*           m_pPage1ArtGRB;
    RadioButton*        m_pPage1EmptyRB;
    RadioButton*        m_pPage1TemplateRB;
    ListBox*            m_pPage1RegionLB;
    ListBox*            m_pPage1TemplateLB;
    RadioButton*        m_pPage1OpenRB;
    ListBox*            m_pPage1OpenLB;

    // Seite 2
    FixedBitmap*        m_pPage2FB;
    GroupBox*           m_pPage2LayoutGRP;
    ListBox*            m_pPage2RegionLB;
    ListBox*            m_pPage2LayoutLB;
    GroupBox*           m_pPage2OutTypesGRP;
    RadioButton*        m_pPage2Medium1RB;
    RadioButton*        m_pPage2Medium2RB;
    RadioButton*        m_pPage2Medium3RB;
    RadioButton*        m_pPage2Medium4RB;
    RadioButton*        m_pPage2Medium5RB;

    // Seite 3
    FixedBitmap*        m_pPage3FB;
    GroupBox*           m_pPage3EffectGRP;
    FixedText*          m_pPage3EffectFT;
    FadeEffectLB*       m_pPage3EffectLB;
    FixedText*          m_pPage3SpeedFT;
    ListBox*            m_pPage3SpeedLB;
    GroupBox*           m_pPage3PresTypeGRP;
    RadioButton*        m_pPage3PresTypeLiveRB;
    RadioButton*        m_pPage3PresTypeKioskRB;
    FixedText*          m_pPage3PresTimeFT;
    TimeField*          m_pPage3PresTimeTMF;
    FixedText*          m_pPage3BreakFT;
    TimeField*          m_pPage3BreakTMF;
    CheckBox*           m_pPage3LogoCB;

    // Seite 4
    FixedBitmap*        m_pPage4FB;
    GroupBox*           m_pPage4PersonalGRP;
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

AssistentDlgImpl::AssistentDlgImpl( Window* pWindow, const Link& rFinishLink, BOOL bAutoPilot ) :
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
    m_aAssistentFunc(5)
{
    m_aPageListFile += sal_Unicode('?'),
    m_bTemplatesReady = FALSE;
//  m_bOpenReady      = FALSE;

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
        m_pPage1ArtGRB = new GroupBox(pWindow,SdResId(RB_PAGE1_ARTGROUP)));
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
        m_pPage1OpenLB=new ListBox(pWindow,SdResId(LB_PAGE1_OPEN)));

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
    m_pPage1OpenLB->InsertEntry(String(SdResId(STR_WIZARD_POSITION)));

    // Seite 2
    m_aAssistentFunc.InsertControl(2, &m_aPreview );
    m_aAssistentFunc.InsertControl(2, &m_aPreviewFlag );
    m_aAssistentFunc.InsertControl(2,
        m_pPage2FB = new FixedBitmap(pWindow,SdResId(FB_PAGE2)));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2LayoutGRP = new GroupBox( pWindow, SdResId(GRP_PAGE2_LAYOUT) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2RegionLB = new ListBox(pWindow,SdResId(LB_PAGE2_REGION) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2LayoutLB = new ListBox(pWindow,SdResId(LB_PAGE2_LAYOUT) ));

    m_aAssistentFunc.InsertControl(2,
        m_pPage2OutTypesGRP = new GroupBox( pWindow, SdResId(GRP_PAGE2_OUTPUTTYPES) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium5RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM5) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium1RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM1) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium2RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM2) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium3RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM3) ));
    m_aAssistentFunc.InsertControl(2,
        m_pPage2Medium4RB = new RadioButton( pWindow, SdResId(RB_PAGE2_MEDIUM4) ));
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
        m_pPage3EffectGRP = new GroupBox( pWindow, SdResId(GRP_PAGE3_EFFECT) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3EffectFT = new FixedText( pWindow, SdResId(FT_PAGE3_EFFECT) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3EffectLB = new FadeEffectLB( pWindow, SdResId(LB_PAGE3_EFFECT) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3SpeedFT = new FixedText( pWindow, SdResId(FT_PAGE3_SPEED) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3SpeedLB = new FadeEffectLB( pWindow, SdResId(LB_PAGE3_SPEED) ));
    m_aAssistentFunc.InsertControl(3,
        m_pPage3PresTypeGRP = new GroupBox( pWindow, SdResId(GRP_PAGE3_PRESTYPE) ));
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
    m_pPage3EffectLB->SelectEffect( presentation::FadeEffect_NONE );
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
        m_pPage4PersonalGRP = new GroupBox( pWindow, SdResId(GRP_PAGE4_PERSONAL) ));
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
    if( !SfxApplication::IsPlugin() )
    {
        InterpolateFixedBitmap( m_pPage1FB );
        InterpolateFixedBitmap( m_pPage2FB );
        InterpolateFixedBitmap( m_pPage3FB );
        InterpolateFixedBitmap( m_pPage4FB );
        InterpolateFixedBitmap( m_pPage5FB );
    }

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

    m_aStartScanTimer.SetTimeout(5);
    m_aStartScanTimer.SetTimeoutHdl( LINK( this, AssistentDlgImpl, StartScanHdl ));
    m_aStartScanTimer.Start();

    ChangePage();
}

AssistentDlgImpl::~AssistentDlgImpl()
{
    DeletePassords();
/*
    if(m_pTemplateDir)
        EndListening(*m_pTemplateDir);
*/
    TemplateDir* pDir = m_aPresentList.First();
    while(pDir)
    {
        TemplateEntry* pEntry = pDir->m_aEntrys.First();
        while(pEntry)
        {
            delete pEntry;
            pEntry = pDir->m_aEntrys.Next();
        }

        pDir = m_aPresentList.Next();
    }

/*
    if(m_xRoot)
    {
        m_xRoot->Collapse();
        EndListening(*(CntAnchor*)m_xRoot);
        m_xRoot = NULL;
    }
*/
/*
    if(m_xDocmenu)
    {
        EndListening( *(CntAnchor*)m_xDocmenu );
        m_xDocmenu = NULL;
    }
*/

    // Seite 1
    delete m_pPage1FB;
    delete m_pPage1ArtGRB;
    delete m_pPage1EmptyRB;
    delete m_pPage1TemplateRB;
    delete m_pPage1TemplateLB;
    delete m_pPage1RegionLB;
    delete m_pPage1OpenRB;
    delete m_pPage1OpenLB;

    // Seite 2
    delete m_pPage2FB;
    delete m_pPage2LayoutGRP;
    delete m_pPage2RegionLB;
    delete m_pPage2LayoutLB;
    delete m_pPage2OutTypesGRP;
    delete m_pPage2Medium1RB;
    delete m_pPage2Medium2RB;
    delete m_pPage2Medium3RB;
    delete m_pPage2Medium4RB;
    delete m_pPage2Medium5RB;

    // Seite 3
    delete m_pPage3FB;
    delete m_pPage3EffectGRP;
    delete m_pPage3EffectFT;
    delete m_pPage3EffectLB;
    delete m_pPage3SpeedFT;
    delete m_pPage3SpeedLB;
    delete m_pPage3PresTypeGRP;
    delete m_pPage3PresTypeLiveRB;
    delete m_pPage3PresTypeKioskRB;
    delete m_pPage3PresTimeFT;
    delete m_pPage3PresTimeTMF;
    delete m_pPage3BreakFT;
    delete m_pPage3BreakTMF;
    delete m_pPage3LogoCB;

    // Seite 4
    delete m_pPage4FB;
    delete m_pPage4PersonalGRP;
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

    for( ULONG nIdx = 0; nIdx < m_aOpenFilesList.Count(); nIdx++ )
        delete m_aOpenFilesList.GetObject(nIdx);

    if( m_bTemplatesReady && m_aCache.ClearInvalidEntrys() )
        m_aCache.Save();
}

void AssistentDlgImpl::EndDialog( long nResult )
{
    m_pWindow = NULL;
}

// ********************************************************************
// I/O Chaos
// ********************************************************************

void AssistentDlgImpl::ScanTemplates()
{
/*
    m_pTemplateDir = NULL;
    m_nTemplate = 0;
    m_xRoot = new CntAnchor( NULL, ".component:Template/" );

    StartListening( *m_xRoot );
    m_xRoot->MakeVisible( TRUE );
    m_xRoot->MarkAsRoot();
    m_xRoot->DontThread();

    CntOpenModeItem aItem( WID_OPEN, CNT_OPEN_FOLDERS );
    m_xRoot->Put( aItem );
*/
}

/*
void AssistentDlgImpl::ScanTemplateDir( CntAnchorRef xDir )
{
    StartListening( *xDir );
    xDir->MakeVisible( TRUE );
    xDir->MarkAsRoot();
    xDir->DontThread();

    xDir->Put(CntOpenModeItem( WID_OPEN, CNT_OPEN_ALL ));
}
*/

/*
void AssistentDlgImpl::AddTemplateDir( CntAnchorRef xRoot )
{
    const ULONG nCount = xRoot->SubAnchorCount();

    if( nCount == 0 )
        return;

    TemplateDir* pPresntDir = NULL;
    TemplateDir* pLayoutDir = NULL;

    INetURLObject aRootUrl( xRoot->GetViewURL() );

    CntContentTypeItem aImpressType1( 0, CONTENT_TYPE_APP_STARIMPRESS );
    CntContentTypeItem aImpressType2( 0, CONTENT_TYPE_APP_VND_IMPRESS );

    for( ULONG i = 0; i < nCount; i++ )
    {
        CntAnchor* pSubAnchor = xRoot->GetSubAnchor( i );

        DateTime aTime;

        if((pSubAnchor->GetItemState(WID_DATE_MODIFIED) & SFX_ITEM_SET) == SFX_ITEM_SET)
            aTime = ((SfxDateTimeItem&)pSubAnchor->Get(WID_DATE_MODIFIED)).GetDateTime();

        TemplateCacheInfo* pEntry = m_aCache.GetFileInfo( pSubAnchor->GetViewURL() );
        if(pEntry && pEntry->GetDateTime() >= aTime)
        {
            pEntry->SetValid( TRUE );

            if(!pEntry->IsImpress())
                continue;
        }
        else
        {
            if(pEntry == NULL)
                pEntry = m_aCache.AddFileInfo( pSubAnchor->GetViewURL() );

            pEntry->SetDateTime( aTime );
            pEntry->SetValid( TRUE );
            pEntry->SetModified(TRUE);

            pSubAnchor->Put(SfxVoidItem(WID_GETDATA));

            const CntContentTypeItem& rContentType = (const CntContentTypeItem&)pSubAnchor->Get(WID_CONTENT_TYPE);

            if( rContentType != aImpressType1 && rContentType != aImpressType2 )
            {
                pEntry->SetImpress(FALSE);
                continue;
            }
            else
            {
                pEntry->SetImpress(TRUE);
            }
        }

        String aTitle;
        const SfxPoolItem* pItem;
        if ( pSubAnchor->GetItemState( WID_TITLE, TRUE, &pItem ) == SFX_ITEM_SET )
            aTitle = ( (CntStringItem*)pItem )->GetValue();
        else
        {
            INetURLObject aUrl( pSubAnchor->GetViewURL() );
            aTitle = aUrl.GetName();
        }

        if( pPresntDir == NULL )
        {
            String aRootTitle;
            const SfxPoolItem* pItem;
            if ( xRoot->GetItemState( WID_TITLE, TRUE, &pItem ) == SFX_ITEM_SET )
                aRootTitle = ( (CntStringItem*)pItem )->GetValue();
            else
                aRootTitle = aRootUrl.GetName();

            pPresntDir = new TemplateDir( aRootTitle, aRootUrl.GetLastName() );
        }

        pPresntDir->m_aEntrys.Insert( new TemplateEntry( aTitle, pSubAnchor->GetViewURL() ), CONTAINER_APPEND);
    }

    if(pPresntDir != NULL)
        m_aPresentList.Insert( pPresntDir, CONTAINER_APPEND );

}
*/

void AssistentDlgImpl::TemplateScanDone()
{
/*
    if(m_pTemplateDir)
    {
        EndListening(*m_pTemplateDir);
        m_pTemplateDir = NULL;
    }

    EndListening( *m_xRoot );
    m_xRoot = NULL;

    int nFirstEntry = 0;
    m_pPage1RegionLB->Clear();
    const ULONG nCount = m_aPresentList.Count();
    for( ULONG i = 0; i < nCount; i++ )
    {
        TemplateDir* pDir = m_aPresentList.GetObject(i);
        if(pDir->m_aUrl == "presnt") //HACK! presnt Verzeichnis immer als erste Zeigen
            nFirstEntry = i;

        m_pPage1RegionLB->InsertEntry( pDir->m_aRegion );
    }
    m_pPage1RegionLB->SelectEntryPos(nFirstEntry);
    m_pPage1RegionLB->Update();
    SelectTemplateRegion( m_pPage1RegionLB->GetSelectEntry() );

    nFirstEntry = 0;
    m_pPage2RegionLB->Clear();
    for( i = 0; i < nCount; i++ )
    {
        TemplateDir* pDir = m_aPresentList.GetObject(i);
        if(pDir->m_aUrl == "layout")
            nFirstEntry = i;

        m_pPage2RegionLB->InsertEntry( pDir->m_aRegion );
    }
    m_pPage2RegionLB->SelectEntryPos(nFirstEntry);
    m_pPage2RegionLB->Update();
    SelectLayoutRegion( m_pPage2RegionLB->GetSelectEntry() );

    m_bTemplatesReady = nCount != 0;

    if( m_pWindow )
        UpdatePage();
*/
}

void AssistentDlgImpl::ScanDocmenu()
{
/*
    m_xDocmenu = new CntAnchor(NULL, ".component:Docmenu/");
    m_xDocmenu->MakeVisible( TRUE );
    m_xDocmenu->MarkAsRoot();
    m_xDocmenu->DontThread();

    StartListening( *m_xDocmenu );
    m_xDocmenu->Put(CntOpenModeItem( WID_OPEN, CNT_OPEN_ALL ));
*/
}

void AssistentDlgImpl::DocmenuScanDone()
{
/*
    EndListening( *m_xDocmenu );

    const ULONG nCount = m_xDocmenu->SubAnchorCount();
    CntContentTypeItem aImpressType1( 0, CONTENT_TYPE_APP_STARIMPRESS );
    CntContentTypeItem aImpressType2( 0, CONTENT_TYPE_APP_VND_IMPRESS );

    for( ULONG i = 0; i < nCount; i++ )
    {
        CntAnchor* pSubAnchor = m_xDocmenu->GetSubAnchor( i );

        String aCommand;
        const CntCmpCommandItem& rCmdItem = (const CntCmpCommandItem&)pSubAnchor->Get(WID_COMPONENT_COMMAND);
        String aCmpCommand( OUStringToString( rCmdItem.GetCommand(), CHARSET_SYSTEM) );
//#endif
        INetURLObject aUrl( aCmpCommand );

        if( INET_PROT_FILE != aUrl.GetProtocol())
            continue;

        String aExt = aUrl.GetExtension().Lower();

        BOOL bImpress = FALSE;

        if(aExt.Compare( "sdd" ) == COMPARE_EQUAL)
        {
            bImpress = TRUE;
        }
        else if( aExt.Compare( "ppt" ) == COMPARE_EQUAL )
        {
            bImpress = TRUE;
        }
        else if( aExt.Compare( "vor" ) == COMPARE_EQUAL )
        {
            CntAnchorRef xAnchor = new CntAnchor( 0, aUrl.GetMainURL() );
            xAnchor->Put(SfxVoidItem(WID_GETDATA));
            const CntContentTypeItem& rContentType = (const CntContentTypeItem&)xAnchor->Get(WID_CONTENT_TYPE);

            bImpress = rContentType == aImpressType1 || rContentType == aImpressType2;
        }

        if( bImpress)
        {
            m_aOpenFilesList.Insert( new String( aUrl.GetMainURL() ), CONTAINER_APPEND);
            m_pPage1OpenLB->InsertEntry( aUrl.GetName() );
//          m_bOpenReady = TRUE;
        }

    }

    m_xDocmenu = NULL;

    UpdatePage();
*/
}

void AssistentDlgImpl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
/*
    NAMESPACE_VOS( OGuard ) aGuard( Application::GetSolarMutex() );

    if( rHint.ISA(CntStatusHint) )
    {
        const CntStatusHint& rStatusHint = (const CntStatusHint&)rHint;
        USHORT nWhich = rStatusHint.GetRequest() ?
            rStatusHint.GetRequest()->Which() : 0;

        switch( rStatusHint.GetStatus() )
        {
            case CNT_STATUS_DONE:
            {
                if( nWhich != WID_OPEN )
                    break;

                CntAnchor* pBC = PTR_CAST( CntAnchor, &rBC );

                if(pBC == m_xDocmenu )
                {
                    DocmenuScanDone();
                }
                else
                {
                    if( pBC == m_xRoot )
                    {
                        // das Template Root Verzeichnis wurde komplett eingelesen
                        // jetzt beginnen diese Verzeichnisse einzulesen
                        m_nTemplate = (ULONG)0;
                    }
                    else if( pBC == m_pTemplateDir )
                    {
                        // aktuell eingelessenes Template Verzeichnis speichern
                        AddTemplateDir( m_pTemplateDir );
                        EndListening( *m_pTemplateDir );
                        m_pTemplateDir = NULL;
                        m_nTemplate += 1;
                    }
                    else
                        DBG_ERROR("CNT_STATUS_DONE from unknown CntAnchor!");

                    if( m_nTemplate < m_xRoot->SubAnchorCount() )
                    {
                        // nächstes Template Verzeichniss einlesen
                        m_pTemplateDir = m_xRoot->GetSubAnchor( m_nTemplate );
                        ScanTemplateDir( m_pTemplateDir );
                    }
                    else
                    {
                        // alle Template Verzeichnisse fertig eingelesen
                        TemplateScanDone();
                    }

                }
                break;
            }
            case CNT_STATUS_ERROR:
                // Fehlermeldung ausgeben
                break;
        }
    }
*/
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
            pEntry = m_pTemplateRegion->m_aEntrys.GetObject(nEntry);

        if(pEntry)
        {
            aDocFile = pEntry->m_aPath;

            aTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM(  " (" ) );
            aTitle.Append( pEntry->m_aTitle );
            aTitle.Append( sal_Unicode(')') );
        }
    }
    else if( GetStartType() == ST_OPEN )
    {
        const USHORT nEntry = m_pPage1OpenLB->GetSelectEntryPos();
        if(nEntry != (USHORT)-1 && nEntry > 0)
            aDocFile = *m_aOpenFilesList.GetObject(nEntry-1);
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
        pEntry = m_pLayoutRegion->m_aEntrys.GetObject(nEntry-1);

    if(pEntry)
        aFile = pEntry->m_aPath;

    return aFile;
}

SfxObjectShellLock AssistentDlgImpl::GetDocument()
{
//  m_bPreview = FALSE;     // Document nicht anzeigen
    UpdatePreview(FALSE);   // aber komplett laden
    UpdatePageList();

    SfxObjectShell* pShell = xDocShell;
    SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pShell);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    if(pDoc)
    {
        const USHORT nPageCount = pDoc->GetSdPageCount(PK_STANDARD);
        BOOL bKiosk = m_pPage3PresTypeKioskRB->IsChecked();
        UINT32 nNewTime = (UINT32)m_pPage3PresTimeTMF->GetTime().GetMSFromTime() / 1000;
        if(bKiosk)
        {
            pDoc->SetPresEndless(bKiosk);
            pDoc->SetPresPause( (UINT32)m_pPage3BreakTMF->GetTime().GetMSFromTime() / 1000 );
            pDoc->SetPresShowLogo( m_pPage3LogoCB->IsChecked() );
        }

        USHORT nPgAbsNum = 0;
        USHORT nPgRelNum = 0;
        while( nPgAbsNum < nPageCount )
        {
            SdPage* pPage = pDoc->GetSdPage( nPgRelNum, PK_STANDARD );
            if( m_pPage5PageListCT->IsPageChecked(nPgAbsNum) )
            {
                pPage->SetFadeEffect( m_pPage3EffectLB->GetSelectedEffect() );
                pPage->SetFadeSpeed( (FadeSpeed)m_pPage3SpeedLB->GetSelectEntryPos() );
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

    return xDocShell;
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
            m_pPage1TemplateRB->Enable(m_bTemplatesReady);
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

IMPL_LINK( AssistentDlgImpl, StartScanHdl, void *, EMPTYARG )
{
    m_aCache.Load();

    ScanDocmenu();
    ScanTemplates();

    UpdatePreview(TRUE);

    return 0;
}

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

IMPL_LINK( AssistentDlgImpl, EffectPreviewHdl, Button *, EMPTYARG )
{
    if(m_bPreview && xDocShell.Is() )
        m_aPreview.ShowEffect( m_pPage3EffectLB->GetSelectedEffect(), (FadeSpeed)m_pPage3SpeedLB->GetSelectEntryPos() );
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
    ULONG i = m_aPresentList.Count();
    while( i > 0 )
    {
        i--;
        TemplateDir* pDir = m_aPresentList.GetObject(i);
        m_pTemplateRegion = pDir;
        if( pDir->m_aRegion.Equals( rRegion ) )
        {
            for( i = 0; i < pDir->m_aEntrys.Count(); i++ )
            {
                TemplateEntry* pEntry = pDir->m_aEntrys.GetObject(i);
                m_pPage1TemplateLB->InsertEntry( pEntry->m_aTitle );
            }
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
    ULONG i = m_aPresentList.Count();
    while( i > 0 )
    {
        i--;
        TemplateDir* pDir = m_aPresentList.GetObject(i);
        m_pLayoutRegion = pDir;

        if( pDir->m_aRegion.Equals( rRegion ) )
        {
            for( i = 0; i < pDir->m_aEntrys.Count(); i++ )
            {
                TemplateEntry* pEntry = pDir->m_aEntrys.GetObject(i);
                m_pPage2LayoutLB->InsertEntry( pEntry->m_aTitle );
            }
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
    SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pShell);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;
    SdPage* pPage = pDoc?pDoc->GetSdPage(0, PK_STANDARD):NULL;

    if(pPage && ( aTopic.Len() != 0 || aName.Len() != 0 || aInfo.Len() != 0 ) )
    {
        if( pPage->GetAutoLayout() == AUTOLAYOUT_NONE )
            pPage->SetAutoLayout(AUTOLAYOUT_TITLE, TRUE);

        SdrObject* pObj;
        SdrObjKind eSdrObjKind;
        String aEmptyString;
        USHORT nIndex;
        List* pPresObjList;

        pPresObjList=pPage->GetPresObjList();
        for(nIndex=0;nIndex<pPresObjList->Count();nIndex++)
        {
            pObj=(SdrObject*)pPresObjList->GetObject(nIndex);
            if (pObj && pObj->GetObjInventor() == SdrInventor)
            {
                eSdrObjKind=(SdrObjKind)pObj->GetObjIdentifier();
                if(eSdrObjKind==OBJ_TITLETEXT)
                {
                    if ( aTopic.Len() )
                    {
                        pPage->SetObjText( (SdrTextObj*)pObj, NULL, PRESOBJ_TITLE, aTopic );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE ), TRUE );
                        pObj->SetEmptyPresObj(FALSE);
                    }
                }

                if ( aName.Len() || aInfo.Len() )
                {
                    String aStrTmp( aName );
                    aStrTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\n\n" ) );
                    aStrTmp.Append( aInfo );

                    if (eSdrObjKind == OBJ_OUTLINETEXT)
                    {
                        pPage->SetObjText( (SdrTextObj*) pObj, NULL, PRESOBJ_OUTLINE, aStrTmp );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE ), TRUE );
                        pObj->SetEmptyPresObj(FALSE);
                    }
                    else if (eSdrObjKind == OBJ_TEXT)
                    {
                        pPage->SetObjText( (SdrTextObj*) pObj, NULL, PRESOBJ_TEXT, aStrTmp );
                        pObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT ), TRUE );
                        pObj->SetEmptyPresObj(FALSE);
                    }
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
    SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pShell);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    m_pPage5PageListCT->Clear();

    if(pDoc)
        m_pPage5PageListCT->Fill(pDoc);
}

void AssistentDlgImpl::UpdatePreview( BOOL bDocPreview )
{
    if(!m_bPreview && bDocPreview)
    {
        m_aPreview.Invalidate();
        m_aPreview.SetObjectShell(0);
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
            if(xDocShell.Is())
                xDocShell->Clear();

            SdDrawDocShell* pNewDocSh;
            xDocShell = pNewDocSh = new SdDrawDocShell(SFX_CREATE_MODE_STANDARD, FALSE);
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
            SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pShell);
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
        if(xDocShell.Is())
            xDocShell.Clear();

        Window *pParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( m_pWindow );

        SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,m_pWindow);

        SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
        if(IsOwnFormat(aDocFile))
        {
            pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
            if(bDocPreview)
                pSet->Put( SfxBoolItem( SID_PREVIEW, TRUE ) );
            RestorePassword( pSet, aDocFile );
            if( lErr = pSfxApp->LoadTemplate( xDocShell, aDocFile, aEmptyStr, TRUE, pSet ) )
                ErrorHandler::HandleError(lErr);
            else
                SavePassword( xDocShell, aDocFile );
        }
        else
        {
            SfxStringItem aFile( SID_FILE_NAME, aDocFile );
            SfxStringItem aReferer( SID_REFERER, aEmptyStr );
            SfxBoolItem aView( SID_VIEW, FALSE );
            SfxBoolItem aPreview( SID_PREVIEW, bDocPreview );

            SfxObjectShell* pShell = NULL;

            const SfxObjectShellItem* pRet = (SfxObjectShellItem*)
                                      SfxViewFrame::Current()->GetDispatcher()->Execute(
                                        SID_OPENDOC, SFX_CALLMODE_SYNCHRON, &aFile, &aReferer, &aView, &aPreview, 0L );

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

        Window *pParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( m_pWindow );

        if(IsOwnFormat(aLayoutFile))
        {
            pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
            pSet->Put( SfxBoolItem( SID_PREVIEW, TRUE ) );

            RestorePassword( pSet, aLayoutFile );
            if( lErr = pSfxApp->LoadTemplate( xLayoutDocShell, aLayoutFile, aEmptyStr, TRUE, pSet ) )
                ErrorHandler::HandleError(lErr);
            SavePassword( xDocShell, aLayoutFile );
        }
        else
        {
            SfxStringItem aFile( SID_FILE_NAME, aDocFile );
            SfxStringItem aReferer( SID_REFERER, aEmptyStr );
            SfxBoolItem aView( SID_VIEW, FALSE );
            SfxBoolItem aPreview( SID_PREVIEW, bDocPreview );

            SfxObjectShell* pShell = NULL;

            const SfxObjectShellItem* pRet = (SfxObjectShellItem*)
                                      SfxViewFrame::Current()->GetDispatcher()->Execute(
                                        SID_OPENDOC, SFX_CALLMODE_SYNCHRON, &aFile, &aReferer, &aView, &aPreview, 0L );

            if( pRet && pRet->GetObjectShell() )
                xDocShell = pRet->GetObjectShell();
        }

        Application::SetDefDialogParent( pParent );

        // die Implementierung ermitteln
        SfxObjectShell* pShell = xDocShell;
        SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pShell);
        SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

        pShell = xLayoutDocShell;
        pDocShell = PTR_CAST(SdDrawDocShell,pShell);
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
        m_aPreview.SetObjectShell( xDocShell, m_nShowPage );
}

void AssistentDlgImpl::SavePassword( SfxObjectShellLock xDoc, const String& rPath )
{
    if(xDoc.Is())
    {
        SfxMedium * pMedium = xDoc->GetMedium();
        if(pMedium && pMedium->IsStorage())
        {
            SfxItemSet * pSet = pMedium->GetItemSet();
//          const SfxPoolItem *pItem = 0;
//          if( pSet->GetItemState(SID_PASSWORD, TRUE, &pItem) == SFX_ITEM_SET )
//          {
//              String aPass( ((const SfxStringItem*)pItem)->GetValue());
            SvStorage* pStorage = pMedium->GetStorage();
            if(pStorage)
            {

                String aPass( pStorage->GetKey(), RTL_TEXTENCODING_ASCII_US );

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

