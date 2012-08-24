/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vcl/msgbox.hxx>
#include <vcl/field.hxx>
#include <vcl/keycodes.hxx>
#include <sot/exchange.hxx>
#include <svtools/transfer.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/module.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>
#include <svl/languageoptions.hxx>
#include <svx/SmartTagMgr.hxx>
#include <com/sun/star/smarttags/XSmartTagRecognizer.hpp>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include <rtl/strbuf.hxx>

#define _OFA_AUTOCDLG_CXX
#include "autocdlg.hxx"
#include "autocdlg.hrc"
#include "helpid.hrc"
#include <editeng/acorrcfg.hxx>
#include <editeng/svxacorr.hxx>
#include "cuicharmap.hxx"
#include <editeng/unolingu.hxx>
#include <dialmgr.hxx>
#include <svx/svxids.hrc> // SID_OPEN_SMARTTAGSOPTIONS

static LanguageType eLastDialogLanguage = LANGUAGE_SYSTEM;

using namespace ::com::sun::star::util;
using namespace ::com::sun::star;
using namespace ::rtl;

static ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory >& GetProcessFact()
{
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();
    return xMSF;
}

OfaAutoCorrDlg::OfaAutoCorrDlg(Window* pParent, const SfxItemSet* _pSet ) :
    SfxTabDialog( pParent,  CUI_RES( RID_OFA_AUTOCORR_DLG ), _pSet ),
    aLanguageFT ( this,     CUI_RES( FT_LANG ) ),
    aLanguageLB ( this,     CUI_RES( LB_LANG ) )
{
    sal_Bool bShowSWOptions = sal_False;
    sal_Bool bOpenSmartTagOptions = sal_False;

    if ( _pSet )
    {
        SFX_ITEMSET_ARG( _pSet, pItem, SfxBoolItem, SID_AUTO_CORRECT_DLG, sal_False );
        if ( pItem && pItem->GetValue() )
            bShowSWOptions = sal_True;

        SFX_ITEMSET_ARG( _pSet, pItem2, SfxBoolItem, SID_OPEN_SMARTTAGOPTIONS, sal_False );
        if ( pItem2 && pItem2->GetValue() )
            bOpenSmartTagOptions = sal_True;
    }

    aLanguageFT.SetZOrder(0, WINDOW_ZORDER_FIRST);
    aLanguageLB.SetZOrder(&aLanguageFT, WINDOW_ZORDER_BEHIND);
    aLanguageLB.SetHelpId(HID_AUTOCORR_LANGUAGE);
    FreeResource();

    AddTabPage(RID_OFAPAGE_AUTOCORR_OPTIONS, OfaAutocorrOptionsPage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOFMT_APPLY, OfaSwAutoFmtOptionsPage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOCOMPLETE_OPTIONS, OfaAutoCompleteTabPage::Create, 0);
    AddTabPage(RID_OFAPAGE_SMARTTAG_OPTIONS, OfaSmartTagOptionsTabPage::Create, 0);

    if (!bShowSWOptions)
    {
        RemoveTabPage(RID_OFAPAGE_AUTOFMT_APPLY);
        RemoveTabPage(RID_OFAPAGE_AUTOCOMPLETE_OPTIONS);
        RemoveTabPage(RID_OFAPAGE_SMARTTAG_OPTIONS);
    }
    else
    {
        // remove smart tag tab page if no extensions are installed
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
        if ( !pOpt || !pOpt->pSmartTagMgr || 0 == pOpt->pSmartTagMgr->NumberOfRecognizers() )
            RemoveTabPage(RID_OFAPAGE_SMARTTAG_OPTIONS);

        RemoveTabPage(RID_OFAPAGE_AUTOCORR_OPTIONS);
    }

    AddTabPage(RID_OFAPAGE_AUTOCORR_REPLACE, OfaAutocorrReplacePage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOCORR_EXCEPT,  OfaAutocorrExceptPage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOCORR_QUOTE,   OfaQuoteTabPage::Create, 0);

    // initialize languages
    //! LANGUAGE_NONE is displayed as '[All]' and the LanguageType
    //! will be set to LANGUAGE_DONTKNOW
    sal_Int16 nLangList = LANG_LIST_WESTERN;

    if( SvtLanguageOptions().IsCTLFontEnabled() )
        nLangList |= LANG_LIST_CTL;
    aLanguageLB.SetLanguageList( nLangList, sal_True, sal_True );
    aLanguageLB.SelectLanguage( LANGUAGE_NONE );
    sal_uInt16 nPos = aLanguageLB.GetSelectEntryPos();
    DBG_ASSERT( LISTBOX_ENTRY_NOTFOUND != nPos, "listbox entry missing" );
    aLanguageLB.SetEntryData( nPos, (void*)(long) LANGUAGE_DONTKNOW );

    // Initialisierung funktionier fuer static nicht unter Linux - deswegen hier
    if( LANGUAGE_SYSTEM == eLastDialogLanguage )
        eLastDialogLanguage = Application::GetSettings().GetLanguage();

    LanguageType nSelectLang = LANGUAGE_DONTKNOW;
    nPos = aLanguageLB.GetEntryPos( (void*)(long) eLastDialogLanguage );
    if (LISTBOX_ENTRY_NOTFOUND != nPos)
        nSelectLang = eLastDialogLanguage;
    aLanguageLB.SelectLanguage( nSelectLang );

    aLanguageLB.SetSelectHdl(LINK(this, OfaAutoCorrDlg, SelectLanguageHdl));

    Size aMinSize(aLanguageFT.CalcMinimumSize());
    //reserve some extra space for CJK accelerators that are possible inserted
    //later (like '(A)')
    aLanguageFT.setPosSizePixel( 0, 0, aMinSize.Width() + 20, 0, WINDOW_POSSIZE_WIDTH );

    if ( bOpenSmartTagOptions )
        SetCurPageId( RID_OFAPAGE_SMARTTAG_OPTIONS );
}

sal_Bool lcl_FindEntry( ListBox& rLB, const String& rEntry,
                    CollatorWrapper& rCmpClass )
{
    sal_uInt16 nCount = rLB.GetEntryCount();
    sal_uInt16 nSelPos = rLB.GetSelectEntryPos();
    sal_uInt16 i;
    for(i = 0; i < nCount; i++)
    {
        if( 0 == rCmpClass.compareString(rEntry, rLB.GetEntry(i) ))
        {
            rLB.SelectEntryPos(i, sal_True);
            return sal_True;
        }
    }
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        rLB.SelectEntryPos(nSelPos, sal_False);
    return sal_False;
}

IMPL_LINK(OfaAutoCorrDlg, SelectLanguageHdl, ListBox*, pBox)
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    void* pVoid = pBox->GetEntryData(nPos);
    LanguageType eNewLang = (LanguageType)(long)pVoid;
    // save old settings and fill anew
    if(eNewLang != eLastDialogLanguage)
    {
        sal_uInt16  nPageId = GetCurPageId();
        if(RID_OFAPAGE_AUTOCORR_REPLACE == nPageId)
            ((OfaAutocorrReplacePage*)GetTabPage( nPageId ))->SetLanguage(eNewLang);
        else if(RID_OFAPAGE_AUTOCORR_EXCEPT == nPageId)
            ((OfaAutocorrExceptPage*)GetTabPage( nPageId ))->SetLanguage(eNewLang);
    }
    return 0;
}

OfaAutocorrOptionsPage::OfaAutocorrOptionsPage( Window* pParent,
                                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_AUTOCORR_OPTIONS ), rSet),
    aCheckLB            (this, CUI_RES(CLB_SETTINGS )),

    sInput              (CUI_RES(ST_USE_REPLACE     )),
    sDoubleCaps         (CUI_RES(ST_CPTL_STT_WORD     )),
    sStartCap           (CUI_RES(ST_CPTL_STT_SENT     )),
    sBoldUnderline      (CUI_RES(ST_BOLD_UNDER        )),
    sURL                (CUI_RES(ST_DETECT_URL        )),
    sNoDblSpaces        (CUI_RES(STR_NO_DBL_SPACES    )),
    sDash               (CUI_RES(ST_DASH              )),
    sAccidentalCaps     (CUI_RES(ST_CORRECT_ACCIDENTAL_CAPS_LOCK))
{
    FreeResource();

    aCheckLB.SetHelpId(HID_OFAPAGE_AUTOCORR_CLB);
}

OfaAutocorrOptionsPage::~OfaAutocorrOptionsPage()
{
}

SfxTabPage* OfaAutocorrOptionsPage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrOptionsPage(pParent, rSet);
}

sal_Bool OfaAutocorrOptionsPage::FillItemSet( SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    long nFlags = pAutoCorrect->GetFlags();

    sal_uInt16 nPos = 0;
    pAutoCorrect->SetAutoCorrFlag(Autocorrect,          aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CptlSttWrd,           aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CptlSttSntnc,         aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgWeightUnderl,      aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(SetINetAttr,          aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgToEnEmDash,        aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(IgnoreDoubleSpace,    aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CorrectCapsLock,      aCheckLB.IsChecked(nPos++));

    sal_Bool bReturn = nFlags != pAutoCorrect->GetFlags();
    if(bReturn )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return bReturn;
}

void    OfaAutocorrOptionsPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_False);
}

void OfaAutocorrOptionsPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const long nFlags = pAutoCorrect->GetFlags();

    aCheckLB.SetUpdateMode(sal_False);
    aCheckLB.Clear();

    aCheckLB.InsertEntry(sInput);
    aCheckLB.InsertEntry(sDoubleCaps);
    aCheckLB.InsertEntry(sStartCap);
    aCheckLB.InsertEntry(sBoldUnderline);
    aCheckLB.InsertEntry(sURL);
    aCheckLB.InsertEntry(sDash);
    aCheckLB.InsertEntry(sNoDblSpaces);
    aCheckLB.InsertEntry(sAccidentalCaps);

    sal_uInt16 nPos = 0;
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & Autocorrect) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & CptlSttWrd) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & CptlSttSntnc) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgWeightUnderl) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & SetINetAttr) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgToEnEmDash) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & IgnoreDoubleSpace) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & CorrectCapsLock) );

    aCheckLB.SetUpdateMode(sal_True);
}

/*********************************************************************/
/*                                                                   */
/*  helping struct for dUserData of the Checklistbox                 */
/*                                                                   */
/*********************************************************************/

struct ImpUserData
{
    String  *pString;
    Font*   pFont;

    ImpUserData(String* pText, Font* pFnt)
        { pString = pText; pFont = pFnt;}
};


/*********************************************************************/
/*                                                                   */
/*  dialog for per cent settings                                     */
/*                                                                   */
/*********************************************************************/

class OfaAutoFmtPrcntSet : public ModalDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    FixedLine       aPrcntFL;
    MetricField     aPrcntMF;

    public:
        OfaAutoFmtPrcntSet(Window* pParent) :
            ModalDialog(pParent, CUI_RES(RID_OFADLG_PRCNT_SET)),
                aOKPB(this,         CUI_RES(BT_OK)),
                aCancelPB(this,     CUI_RES(BT_CANCEL)),
                aPrcntFL(this,      CUI_RES(FL_PRCNT)),
                aPrcntMF(this,  CUI_RES(ED_RIGHT_MARGIN))
            {
                FreeResource();
            }
    MetricField&    GetPrcntFld(){return aPrcntMF;}
};


/*********************************************************************/
/*                                                                   */
/*  changed LBoxString                                               */
/*                                                                   */
/*********************************************************************/

class OfaImpBrwString : public SvLBoxString
{
public:

    OfaImpBrwString( SvLBoxEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr){}

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags,
                                            SvLBoxEntry* pEntry);
};

void OfaImpBrwString::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 /*nFlags*/,
    SvLBoxEntry* pEntry )
{
    rDev.DrawText( rPos, GetText() );
    if(pEntry->GetUserData())
    {
        ImpUserData* pUserData = (ImpUserData* )pEntry->GetUserData();
        Point aNewPos(rPos);
        aNewPos.X() += rDev.GetTextWidth(GetText());
        Font aOldFont( rDev.GetFont());
        Font aFont( aOldFont );
        if(pUserData->pFont)
        {
            aFont = *pUserData->pFont;
            aFont.SetColor(aOldFont.GetColor());
            aFont.SetSize(aOldFont.GetSize());
        }
        aFont.SetWeight( WEIGHT_BOLD );

        sal_Bool bFett = sal_True;
        sal_uInt16 nPos = 0;
        do {
            String sTxt( pUserData->pString->GetToken( 0, 1, nPos ));

            if( bFett )
                rDev.SetFont( aFont );

            rDev.DrawText( aNewPos, sTxt );

            if( STRING_NOTFOUND != nPos )
                aNewPos.X() += rDev.GetTextWidth( sTxt );

            if( bFett )
                rDev.SetFont( aOldFont );

            bFett = !bFett;
        } while( STRING_NOTFOUND != nPos );
    }
}

/*********************************************************************/
/*                                                                   */
/*  use TabPage autoformat                                           */
/*                                                                   */
/*********************************************************************/

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

enum OfaAutoFmtOptions
{
    USE_REPLACE_TABLE,
    CORR_UPPER,
    BEGIN_UPPER,
    BOLD_UNDERLINE,
    DETECT_URL,
    REPLACE_DASHES,
    DEL_SPACES_AT_STT_END,
    DEL_SPACES_BETWEEN_LINES,
    IGNORE_DBLSPACE,
    CORRECT_CAPS_LOCK,
    APPLY_NUMBERING,
    INSERT_BORDER,
    CREATE_TABLE,
    REPLACE_STYLES,
    DEL_EMPTY_NODE,
    REPLACE_USER_COLL,
    REPLACE_BULLETS,
    MERGE_SINGLE_LINE_PARA
};

OfaSwAutoFmtOptionsPage::OfaSwAutoFmtOptionsPage( Window* pParent,
                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, CUI_RES(RID_OFAPAGE_AUTOFMT_APPLY), rSet),
    m_aCheckLBContainer(this, CUI_RES(CLB_SETTINGS)),
    aCheckLB(m_aCheckLBContainer),
    aEditPB             (this,  CUI_RES(PB_EDIT)),
    aHeader1Expl        (this,  CUI_RES(FT_HEADER1_EXPLANATION)),
    aHeader2Expl        (this,  CUI_RES(FT_HEADER2_EXPLANATION)),
    sHeader1            (CUI_RES( STR_HEADER1       )),
    sHeader2            (CUI_RES( STR_HEADER2       )),
    sDeleteEmptyPara    (CUI_RES( ST_DEL_EMPTY_PARA)),
    sUseReplaceTbl      (CUI_RES(   ST_USE_REPLACE  )),
    sCptlSttWord        (CUI_RES(   ST_CPTL_STT_WORD)),
    sCptlSttSent        (CUI_RES(   ST_CPTL_STT_SENT)),
    sUserStyle          (CUI_RES(   ST_USER_STYLE   )),
    sBullet             (CUI_RES(   ST_BULLET       )),
    sBoldUnder          (CUI_RES(   ST_BOLD_UNDER   )),
    sNoDblSpaces        (CUI_RES(   STR_NO_DBL_SPACES)),
    sCorrectCapsLock    (CUI_RES(   ST_CORRECT_ACCIDENTAL_CAPS_LOCK)),
    sDetectURL          (CUI_RES(   ST_DETECT_URL   )),
    sDash               (CUI_RES(   ST_DASH         )),
    sRightMargin        (CUI_RES(   ST_RIGHT_MARGIN )),
    sNum                (CUI_RES(   STR_NUM         )),
    sBorder             (CUI_RES(   STR_BORDER      )),
    sTable              (CUI_RES(   STR_TABLE       )),
    sReplaceTemplates   (CUI_RES(   STR_REPLACE_TEMPLATES)),
    sDelSpaceAtSttEnd   (CUI_RES(   STR_DEL_SPACES_AT_STT_END)),
    sDelSpaceBetweenLines(CUI_RES(STR_DEL_SPACES_BETWEEN_LINES)),

    nPercent        ( 50 ),
    pCheckButtonData( NULL )

{
    FreeResource();

    // set typ. inverted commas
    SvtSysLocale aSysLcl;

    aCheckLB.SetHelpId(HID_OFAPAGE_AUTOFORMAT_CLB);
    aCheckLB.SetStyle(aCheckLB.GetStyle()|WB_HSCROLL| WB_VSCROLL);

    aCheckLB.SetSelectHdl(LINK(this, OfaSwAutoFmtOptionsPage, SelectHdl));
    aCheckLB.SetDoubleClickHdl(LINK(this, OfaSwAutoFmtOptionsPage, EditHdl));

    static long aStaticTabs[]=
    {
        3, 0, 20, 40
    };

    aCheckLB.SvxSimpleTable::SetTabs(aStaticTabs);
    String sHeader( sHeader1 );
    sHeader += '\t';
    sHeader += sHeader2;
    sHeader += '\t';
    aCheckLB.InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                        HIB_CENTER | HIB_VCENTER | HIB_FIXEDPOS | HIB_FIXED);

    aEditPB.SetClickHdl(LINK(this, OfaSwAutoFmtOptionsPage, EditHdl));
}

SvLBoxEntry* OfaSwAutoFmtOptionsPage::CreateEntry(String& rTxt, sal_uInt16 nCol)
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if ( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData( &aCheckLB );
        aCheckLB.SetCheckButtonData( pCheckButtonData );
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));

    String sEmpty;
    if (nCol == CBCOL_SECOND)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );
    pEntry->AddItem( new OfaImpBrwString( pEntry, 0, rTxt ) );

    return pEntry;
}

OfaSwAutoFmtOptionsPage::~OfaSwAutoFmtOptionsPage()
{
    delete (ImpUserData*) aCheckLB.GetUserData( REPLACE_BULLETS );
    delete (ImpUserData*) aCheckLB.GetUserData( APPLY_NUMBERING );
    delete (ImpUserData*) aCheckLB.GetUserData( MERGE_SINGLE_LINE_PARA );
    delete pCheckButtonData;
}

SfxTabPage* OfaSwAutoFmtOptionsPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new OfaSwAutoFmtOptionsPage(pParent, rAttrSet);
}

sal_Bool OfaSwAutoFmtOptionsPage::FillItemSet( SfxItemSet&  )
{
    sal_Bool bModified = sal_False;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    long nFlags = pAutoCorrect->GetFlags();

    sal_Bool bCheck = aCheckLB.IsChecked(USE_REPLACE_TABLE, CBCOL_FIRST);
    bModified |= pOpt->bAutoCorrect != bCheck;
    pOpt->bAutoCorrect = bCheck;
    pAutoCorrect->SetAutoCorrFlag(Autocorrect,
                        aCheckLB.IsChecked(USE_REPLACE_TABLE, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(CORR_UPPER, CBCOL_FIRST);
    bModified |= pOpt->bCptlSttWrd != bCheck;
    pOpt->bCptlSttWrd = bCheck;
    pAutoCorrect->SetAutoCorrFlag(CptlSttWrd,
                        aCheckLB.IsChecked(CORR_UPPER, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(BEGIN_UPPER, CBCOL_FIRST);
    bModified |= pOpt->bCptlSttSntnc != bCheck;
    pOpt->bCptlSttSntnc = bCheck;
    pAutoCorrect->SetAutoCorrFlag(CptlSttSntnc,
                        aCheckLB.IsChecked(BEGIN_UPPER, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(BOLD_UNDERLINE, CBCOL_FIRST);
    bModified |= pOpt->bChgWeightUnderl != bCheck;
    pOpt->bChgWeightUnderl = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ChgWeightUnderl,
                        aCheckLB.IsChecked(BOLD_UNDERLINE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(IgnoreDoubleSpace,
                        aCheckLB.IsChecked(IGNORE_DBLSPACE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(CorrectCapsLock,
                        aCheckLB.IsChecked(CORRECT_CAPS_LOCK, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(DETECT_URL, CBCOL_FIRST);
    bModified |= pOpt->bSetINetAttr != bCheck;
    pOpt->bSetINetAttr = bCheck;
    pAutoCorrect->SetAutoCorrFlag(SetINetAttr,
                        aCheckLB.IsChecked(DETECT_URL, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(DEL_EMPTY_NODE, CBCOL_FIRST);
    bModified |= pOpt->bDelEmptyNode != bCheck;
    pOpt->bDelEmptyNode = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_USER_COLL, CBCOL_FIRST);
    bModified |= pOpt->bChgUserColl != bCheck;
    pOpt->bChgUserColl = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_BULLETS, CBCOL_FIRST);
    bModified |= pOpt->bChgEnumNum != bCheck;
    pOpt->bChgEnumNum = bCheck;
    bModified |= aBulletFont != pOpt->aBulletFont;
    pOpt->aBulletFont = aBulletFont;
    bModified |= !comphelper::string::equals(sBulletChar, pOpt->cBullet);
    pOpt->cBullet = sBulletChar.GetChar(0);

    bModified |= aByInputBulletFont != pOpt->aByInputBulletFont;
    bModified |= !comphelper::string::equals(sByInputBulletChar, pOpt->cByInputBullet);
    pOpt->aByInputBulletFont = aByInputBulletFont;
    pOpt->cByInputBullet = sByInputBulletChar.GetChar(0);

    bCheck = aCheckLB.IsChecked(MERGE_SINGLE_LINE_PARA, CBCOL_FIRST);
    bModified |= pOpt->bRightMargin != bCheck;
    pOpt->bRightMargin = bCheck;
    bModified |= nPercent != pOpt->nRightMargin;
    pOpt->nRightMargin = (sal_uInt8)nPercent;

    bCheck = aCheckLB.IsChecked(APPLY_NUMBERING, CBCOL_SECOND);
    bModified |= pOpt->bSetNumRule != bCheck;
    pOpt->bSetNumRule = bCheck;

    bCheck = aCheckLB.IsChecked(INSERT_BORDER, CBCOL_SECOND);
    bModified |= pOpt->bSetBorder != bCheck;
    pOpt->bSetBorder = bCheck;

    bCheck = aCheckLB.IsChecked(CREATE_TABLE, CBCOL_SECOND);
    bModified |= pOpt->bCreateTable != bCheck;
    pOpt->bCreateTable = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_STYLES, CBCOL_SECOND);
    bModified |= pOpt->bReplaceStyles != bCheck;
    pOpt->bReplaceStyles = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_DASHES, CBCOL_FIRST);
    bModified |= pOpt->bChgToEnEmDash != bCheck;
    pOpt->bChgToEnEmDash = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ChgToEnEmDash,
                        aCheckLB.IsChecked(REPLACE_DASHES, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(DEL_SPACES_AT_STT_END, CBCOL_FIRST);
    bModified |= pOpt->bAFmtDelSpacesAtSttEnd != bCheck;
    pOpt->bAFmtDelSpacesAtSttEnd = bCheck;
    bCheck = aCheckLB.IsChecked(DEL_SPACES_AT_STT_END, CBCOL_SECOND);
    bModified |= pOpt->bAFmtByInpDelSpacesAtSttEnd != bCheck;
    pOpt->bAFmtByInpDelSpacesAtSttEnd = bCheck;

    bCheck = aCheckLB.IsChecked(DEL_SPACES_BETWEEN_LINES, CBCOL_FIRST);
    bModified |= pOpt->bAFmtDelSpacesBetweenLines != bCheck;
    pOpt->bAFmtDelSpacesBetweenLines = bCheck;
    bCheck = aCheckLB.IsChecked(DEL_SPACES_BETWEEN_LINES, CBCOL_SECOND);
    bModified |= pOpt->bAFmtByInpDelSpacesBetweenLines != bCheck;
    pOpt->bAFmtByInpDelSpacesBetweenLines = bCheck;

    if(bModified || nFlags != pAutoCorrect->GetFlags())
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }

    return sal_True;
}

void    OfaSwAutoFmtOptionsPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_False);
}

void OfaSwAutoFmtOptionsPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const long nFlags = pAutoCorrect->GetFlags();

    aCheckLB.SetUpdateMode(sal_False);
    aCheckLB.Clear();

    // The following entries have to be inserted in the same order
    // as in the OfaAutoFmtOptions-enum!
    aCheckLB.GetModel()->Insert(CreateEntry(sUseReplaceTbl,     CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sCptlSttWord,       CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sCptlSttSent,       CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sBoldUnder,         CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDetectURL,         CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDash,              CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDelSpaceAtSttEnd,  CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDelSpaceBetweenLines, CBCOL_BOTH  ));

    aCheckLB.GetModel()->Insert(CreateEntry(sNoDblSpaces,       CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sCorrectCapsLock,   CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sNum,               CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sBorder,            CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sTable,             CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sReplaceTemplates,  CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sDeleteEmptyPara,   CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sUserStyle,         CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sBullet,            CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sRightMargin,       CBCOL_FIRST ));

    aCheckLB.CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_FIRST,    pOpt->bAutoCorrect );
    aCheckLB.CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_SECOND,   0 != (nFlags & Autocorrect));
    aCheckLB.CheckEntryPos( CORR_UPPER,         CBCOL_FIRST,    pOpt->bCptlSttWrd );
    aCheckLB.CheckEntryPos( CORR_UPPER,         CBCOL_SECOND,   0 != (nFlags & CptlSttWrd) );
    aCheckLB.CheckEntryPos( BEGIN_UPPER,        CBCOL_FIRST,    pOpt->bCptlSttSntnc );
    aCheckLB.CheckEntryPos( BEGIN_UPPER,        CBCOL_SECOND,   0 != (nFlags & CptlSttSntnc) );
    aCheckLB.CheckEntryPos( BOLD_UNDERLINE,     CBCOL_FIRST,    pOpt->bChgWeightUnderl );
    aCheckLB.CheckEntryPos( BOLD_UNDERLINE,     CBCOL_SECOND,   0 != (nFlags & ChgWeightUnderl) );
    aCheckLB.CheckEntryPos( IGNORE_DBLSPACE,    CBCOL_SECOND,   0 != (nFlags & IgnoreDoubleSpace) );
    aCheckLB.CheckEntryPos( CORRECT_CAPS_LOCK,  CBCOL_SECOND,   0 != (nFlags & CorrectCapsLock) );
    aCheckLB.CheckEntryPos( DETECT_URL,         CBCOL_FIRST,    pOpt->bSetINetAttr );
    aCheckLB.CheckEntryPos( DETECT_URL,         CBCOL_SECOND,   0 != (nFlags & SetINetAttr) );
    aCheckLB.CheckEntryPos( REPLACE_DASHES,     CBCOL_FIRST,    pOpt->bChgToEnEmDash );
    aCheckLB.CheckEntryPos( REPLACE_DASHES,     CBCOL_SECOND,   0 != (nFlags & ChgToEnEmDash) );
    aCheckLB.CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_FIRST,    pOpt->bAFmtDelSpacesAtSttEnd );
    aCheckLB.CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_SECOND,   pOpt->bAFmtByInpDelSpacesAtSttEnd );
    aCheckLB.CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_FIRST,    pOpt->bAFmtDelSpacesBetweenLines );
    aCheckLB.CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_SECOND,   pOpt->bAFmtByInpDelSpacesBetweenLines );
    aCheckLB.CheckEntryPos( DEL_EMPTY_NODE,     CBCOL_FIRST,    pOpt->bDelEmptyNode );
    aCheckLB.CheckEntryPos( REPLACE_USER_COLL,  CBCOL_FIRST,    pOpt->bChgUserColl );
    aCheckLB.CheckEntryPos( REPLACE_BULLETS,    CBCOL_FIRST,    pOpt->bChgEnumNum );

    aBulletFont = pOpt->aBulletFont;
    sBulletChar = pOpt->cBullet;
    ImpUserData* pUserData = new ImpUserData(&sBulletChar, &aBulletFont);
    aCheckLB.SetUserData(  REPLACE_BULLETS, pUserData );

    nPercent = pOpt->nRightMargin;
    sMargin = ' ';
    sMargin += String::CreateFromInt32( nPercent );
    sMargin += '%';
    pUserData = new ImpUserData(&sMargin, 0);
    aCheckLB.SetUserData( MERGE_SINGLE_LINE_PARA, pUserData );

    aCheckLB.CheckEntryPos( APPLY_NUMBERING,    CBCOL_SECOND,   pOpt->bSetNumRule );

    aByInputBulletFont = pOpt->aByInputBulletFont;
    sByInputBulletChar = pOpt->cByInputBullet;
    ImpUserData* pUserData2 = new ImpUserData(&sByInputBulletChar, &aByInputBulletFont);
    aCheckLB.SetUserData( APPLY_NUMBERING , pUserData2 );

    aCheckLB.CheckEntryPos( MERGE_SINGLE_LINE_PARA, CBCOL_FIRST, pOpt->bRightMargin );
    aCheckLB.CheckEntryPos( INSERT_BORDER,      CBCOL_SECOND,   pOpt->bSetBorder );
    aCheckLB.CheckEntryPos( CREATE_TABLE,       CBCOL_SECOND,   pOpt->bCreateTable );
    aCheckLB.CheckEntryPos( REPLACE_STYLES,     CBCOL_SECOND,   pOpt->bReplaceStyles );

    aCheckLB.SetUpdateMode(sal_True);
}

IMPL_LINK(OfaSwAutoFmtOptionsPage, SelectHdl, OfaACorrCheckListBox*, pBox)
{
    aEditPB.Enable(0 != pBox->FirstSelected()->GetUserData());
    return 0;
}

IMPL_LINK_NOARG(OfaSwAutoFmtOptionsPage, EditHdl)
{
    sal_uLong nSelEntryPos = aCheckLB.GetSelectEntryPos();
    if( nSelEntryPos == REPLACE_BULLETS ||
        nSelEntryPos == APPLY_NUMBERING)
    {
        SvxCharacterMap *pMapDlg = new SvxCharacterMap(this);
        ImpUserData* pUserData = (ImpUserData*)aCheckLB.FirstSelected()->GetUserData();
        pMapDlg->SetCharFont(*pUserData->pFont);
        pMapDlg->SetChar( pUserData->pString->GetChar(0) );
        if(RET_OK == pMapDlg->Execute())
        {
            Font aFont(pMapDlg->GetCharFont());
            *pUserData->pFont = aFont;
            sal_UCS4 aChar = pMapDlg->GetChar();
            // using the UCS4 constructor
            rtl::OUString aOUStr( &aChar, 1 );
            *pUserData->pString = aOUStr;
        }
        delete pMapDlg;
    }
    else if( MERGE_SINGLE_LINE_PARA == nSelEntryPos )
    {
        // dialog for per cent settings
        OfaAutoFmtPrcntSet aDlg(this);
        aDlg.GetPrcntFld().SetValue(nPercent);
        if(RET_OK == aDlg.Execute())
        {
            nPercent = (sal_uInt16)aDlg.GetPrcntFld().GetValue();
            sMargin = ' ';
            sMargin += String::CreateFromInt32( nPercent );
            sMargin += '%';
        }
    }
    aCheckLB.Invalidate();
    return 0;
}

void OfaACorrCheckListBox::SetTabs()
{
    SvxSimpleTable::SetTabs();
    sal_uInt16 nAdjust = SV_LBOXTAB_ADJUST_RIGHT|SV_LBOXTAB_ADJUST_LEFT|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_ADJUST_NUMERIC|SV_LBOXTAB_FORCE;

    if( aTabs.size() > 1 )
    {
        SvLBoxTab* pTab = aTabs[1];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
    if( aTabs.size() > 2 )
    {
        SvLBoxTab* pTab = aTabs[2];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
}

void OfaACorrCheckListBox::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

sal_Bool OfaACorrCheckListBox::IsChecked(sal_uLong nPos, sal_uInt16 nCol)
{
    return GetCheckButtonState( GetEntry(nPos), nCol ) == SV_BUTTON_CHECKED;
}

void OfaACorrCheckListBox::SetCheckButtonState( SvLBoxEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
    {
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                pItem->SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                pItem->SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                pItem->SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState OfaACorrCheckListBox::GetCheckButtonState( SvLBoxEntry* pEntry, sal_uInt16 nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found");

    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
    {
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaACorrCheckListBox::HBarClick()
{
    // sorting is stopped by this overload
}

void    OfaACorrCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        sal_uLong nSelPos = GetSelectEntryPos();
        sal_uInt16 nCol = GetCurrentTabPos() - 1;
        if ( nCol < 2 )
        {
            CheckEntryPos( nSelPos, nCol, !IsChecked( nSelPos, nCol ) );
            CallImplEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)GetEntry( nSelPos ) );
        }
        else
        {
            sal_uInt16 nCheck = IsChecked(nSelPos, 1) ? 1 : 0;
            if(IsChecked(nSelPos, 0))
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvxSimpleTable::KeyInput(rKEvt);
}

OfaAutocorrReplacePage::OfaAutocorrReplacePage( Window* pParent,
                                                const SfxItemSet& rSet ) :
    SfxTabPage          ( pParent, CUI_RES( RID_OFAPAGE_AUTOCORR_REPLACE ), rSet),
    aTextOnlyCB         ( this, CUI_RES(CB_TEXT_ONLY )),
    aShortFT            ( this, CUI_RES(FT_SHORT )),
    aShortED            ( this, CUI_RES(ED_SHORT )),
    aReplaceFT          ( this, CUI_RES(FT_REPLACE )),
    aReplaceED          ( this, CUI_RES(ED_REPLACE )),
    aReplaceTLB         ( this, CUI_RES(TLB_REPLACE )),
    aNewReplacePB       ( this, CUI_RES(PB_NEW_REPLACE )),
    aDeleteReplacePB    ( this,CUI_RES(PB_DELETE_REPLACE )),
    sModify             ( CUI_RES(STR_MODIFY) ),
    sNew                ( aNewReplacePB.GetText() ),
    eLang               ( eLastDialogLanguage ),
    bHasSelectionText   ( sal_False ),
    bFirstSelect        ( sal_True ),
    bReplaceEditChanged ( sal_False ),
    bSWriter            ( sal_True )
{
    FreeResource();
    SfxModule *pMod = *(SfxModule**)GetAppData(SHL_WRITER);
    bSWriter = pMod == SfxModule::GetActiveModule();

    ::com::sun::star::lang::Locale aLocale( SvxCreateLocale(eLastDialogLanguage ));
    pCompareClass = new CollatorWrapper( GetProcessFact() );
    pCompareClass->loadDefaultCollator( aLocale, 0 );
    pCharClass = new CharClass( aLocale );

    static long nTabs[] = { 2 /* Tab-Count */, 1, 61 };
    aReplaceTLB.SetTabs( &nTabs[0], MAP_APPFONT );

    aReplaceTLB.SetStyle( aReplaceTLB.GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN );
    aReplaceTLB.SetSelectHdl( LINK(this, OfaAutocorrReplacePage, SelectHdl) );
    aNewReplacePB.SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );
    aDeleteReplacePB.SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );
    aShortED.SetModifyHdl( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    aReplaceED.SetModifyHdl( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    aShortED.SetActionHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );
    aReplaceED.SetActionHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );

    aReplaceED.SetSpaces( sal_True );
    aShortED.SetSpaces( sal_True );
    aShortED.SetMaxTextLen( 30 );
}

OfaAutocorrReplacePage::~OfaAutocorrReplacePage()
{
    aDoubleStringTable.clear();
    aChangesTable.clear();

    delete pCompareClass;
    delete pCharClass;
}

SfxTabPage* OfaAutocorrReplacePage::Create( Window* pParent, const SfxItemSet& rSet)
{
    return new OfaAutocorrReplacePage(pParent, rSet);
}

void OfaAutocorrReplacePage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_True);
}

int OfaAutocorrReplacePage::DeactivatePage( SfxItemSet*  )
{
    return LEAVE_PAGE;
}

sal_Bool OfaAutocorrReplacePage::FillItemSet( SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    for (StringChangeTable::reverse_iterator it = aChangesTable.rbegin(); it != aChangesTable.rend(); it++)
    {
        LanguageType eCurrentLang = it->first;
        StringChangeList& rStringChangeList = it->second;
        std::vector<SvxAutocorrWord> aDeleteWords;
        std::vector<SvxAutocorrWord> aNewWords;

        for (sal_uInt32 i = 0; i < rStringChangeList.aDeletedEntries.size(); i++)
        {
            DoubleString& deleteEntry = rStringChangeList.aDeletedEntries[i];
            SvxAutocorrWord aDeleteWord( deleteEntry.sShort, deleteEntry.sLong );
            aDeleteWords.push_back( aDeleteWord );
        }

        for (sal_uInt32 i = 0; i < rStringChangeList.aNewEntries.size(); i++)
        {
            DoubleString& newEntry = rStringChangeList.aNewEntries[i];
            SvxAutocorrWord aNewWord( newEntry.sShort, newEntry.sLong );
            aNewWords.push_back( aNewWord );
        }
        pAutoCorrect->MakeCombinedChanges( aNewWords, aDeleteWords, eCurrentLang );
    }
    aChangesTable.clear();
    return sal_False;
}

void OfaAutocorrReplacePage::RefillReplaceBox(sal_Bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
    {
        aDoubleStringTable.clear();
        aChangesTable.clear();
    }
    else
    {
        DoubleStringArray* pArray;
        if(aDoubleStringTable.find(eOldLanguage) != aDoubleStringTable.end())
        {
            pArray = &aDoubleStringTable[eOldLanguage];
            pArray->clear();
        }
        else
        {
            pArray = &aDoubleStringTable[eOldLanguage]; // create new array
        }

        sal_uInt32 nListBoxCount = (sal_uInt32) aReplaceTLB.GetEntryCount();
        sal_uInt32 i;
        for(i = 0; i < nListBoxCount; i++)
        {
            pArray->push_back(DoubleString());
            DoubleString& rDouble = (*pArray)[pArray->size() - 1];
            SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( i );
            rDouble.sShort = aReplaceTLB.GetEntryText(pEntry, 0);
            rDouble.sLong = aReplaceTLB.GetEntryText(pEntry, 1);
            rDouble.pUserData = pEntry->GetUserData();
        }
    }

    aReplaceTLB.Clear();
    if( !bSWriter )
        aFormatText.clear();

    if( aDoubleStringTable.find(eLang) != aDoubleStringTable.end() )
    {
        DoubleStringArray& rArray = aDoubleStringTable[eNewLanguage];
        for( sal_uInt32 i = 0; i < rArray.size(); i++ )
        {
            DoubleString& rDouble = rArray[i];
            sal_Bool bTextOnly = 0 == rDouble.pUserData;
            // formatted text is only in Writer
            if(bSWriter || bTextOnly)
            {
                String sEntry(rDouble.sShort);
                sEntry += '\t';
                sEntry += rDouble.sLong;
                SvLBoxEntry* pEntry = aReplaceTLB.InsertEntry(sEntry);
                aTextOnlyCB.Check(bTextOnly);
                if(!bTextOnly)
                    pEntry->SetUserData(rDouble.pUserData); // that means: with format info or even with selection text
            }
            else
            {
                aFormatText.insert(rDouble.sShort);
            }
        }
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxAutocorrWordList* pWordList = pAutoCorrect->LoadAutocorrWordList(eLang);
        aReplaceTLB.SetUpdateMode(sal_False);
        for( SvxAutocorrWordList::iterator it = pWordList->begin(); it != pWordList->end(); ++it )
        {
            SvxAutocorrWord* pWordPtr = *it;
            sal_Bool bTextOnly = pWordPtr->IsTextOnly();
            // formatted text is only in Writer
            if(bSWriter || bTextOnly)
            {
                String sEntry(pWordPtr->GetShort());
                sEntry += '\t';
                sEntry += pWordPtr->GetLong();
                SvLBoxEntry* pEntry = aReplaceTLB.InsertEntry(sEntry);
                aTextOnlyCB.Check(pWordPtr->IsTextOnly());
                if(!bTextOnly)
                    pEntry->SetUserData(&aTextOnlyCB); // that means: with format info
            }
            else
            {
                aFormatText.insert(pWordPtr->GetShort());
            }
        }
        aNewReplacePB.Enable(sal_False);
        aDeleteReplacePB.Enable(sal_False);
        aReplaceTLB.SetUpdateMode(sal_True);
    }

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if( pViewShell && pViewShell->HasSelection( sal_True ) )
    {
        bHasSelectionText = sal_True;
        const String sSelection( pViewShell->GetSelectionText() );
        aReplaceED.SetText( sSelection );
        aTextOnlyCB.Check( !bSWriter );
        aTextOnlyCB.Enable( bSWriter && sSelection.Len() );
    }
    else
    {
        aTextOnlyCB.Enable( sal_False );
    }
}

void OfaAutocorrReplacePage::Reset( const SfxItemSet& )
{
    RefillReplaceBox(sal_True, eLang, eLang);
    aShortED.GrabFocus();
}

void OfaAutocorrReplacePage::SetLanguage(LanguageType eSet)
{
    //save old settings an refill
    if(eSet != eLang)
    {
        RefillReplaceBox(sal_False, eLang, eSet);
        eLastDialogLanguage = eSet;
        delete pCompareClass;
        delete pCharClass;

        ::com::sun::star::lang::Locale aLocale( SvxCreateLocale(eLastDialogLanguage ));
        pCompareClass = new CollatorWrapper( GetProcessFact() );
        pCompareClass->loadDefaultCollator( aLocale, 0 );
        pCharClass = new CharClass( aLocale );
        ModifyHdl(&aShortED);
    }
}

IMPL_LINK(OfaAutocorrReplacePage, SelectHdl, SvTabListBox*, pBox)
{
    if(!bFirstSelect || !bHasSelectionText)
    {
        SvLBoxEntry* pEntry = pBox->FirstSelected();
        String sTmpShort(pBox->GetEntryText(pEntry, 0));
        // if the text is set via ModifyHdl, the cursor is always at the beginning
        // of a word, although you're editing here
        sal_Bool bSameContent = 0 == pCompareClass->compareString( sTmpShort, aShortED.GetText() );
        Selection aSel = aShortED.GetSelection();
        if(aShortED.GetText() != sTmpShort)
        {
            aShortED.SetText(sTmpShort);
            // if it was only a different notation, the selection has to be set again
            if(bSameContent)
            {
                aShortED.SetSelection(aSel);
            }
        }
        aReplaceED.SetText( pBox->GetEntryText(pEntry, 1) );
        // with UserData there is a Formatinfo
        aTextOnlyCB.Check( pEntry->GetUserData() == 0);
    }
    else
    {
        bFirstSelect = sal_False;
    }

    aNewReplacePB.Enable(sal_False);
    aDeleteReplacePB.Enable();
    return 0;
};

void OfaAutocorrReplacePage::NewEntry(String sShort, String sLong)
{
    DoubleStringArray& rNewArray = aChangesTable[eLang].aNewEntries;
    for (sal_uInt32 i = 0; i < rNewArray.size(); i++)
    {
        if (rNewArray[i].sShort == sShort)
        {
            rNewArray.erase(rNewArray.begin() + i);
            break;
        }
    }

    DoubleStringArray& rDeletedArray = aChangesTable[eLang].aDeletedEntries;
    for (sal_uInt32 i = 0; i < rDeletedArray.size(); i++)
    {
        if (rDeletedArray[i].sShort == sShort)
        {
            rDeletedArray.erase(rDeletedArray.begin() + i);
            break;
        }
    }

    DoubleString aNewString = DoubleString();
    aNewString.sShort = sShort;
    aNewString.sLong = sLong;
    rNewArray.push_back(aNewString);
}

void OfaAutocorrReplacePage::DeleteEntry(String sShort, String sLong)
{
    DoubleStringArray& rNewArray = aChangesTable[eLang].aNewEntries;
    for (sal_uInt32 i = 0; i < rNewArray.size(); i++)
    {
        if (rNewArray[i].sShort == sShort)
        {
            rNewArray.erase(rNewArray.begin() + i);
            break;
        }
    }

    DoubleStringArray& rDeletedArray = aChangesTable[eLang].aDeletedEntries;
    for (sal_uInt32 i = 0; i < rDeletedArray.size(); i++)
    {
        if (rDeletedArray[i].sShort == sShort)
        {
            rDeletedArray.erase(rDeletedArray.begin() + i);
            break;
        }
    }

    DoubleString aDeletedString = DoubleString();
    aDeletedString.sShort = sShort;
    aDeletedString.sLong = sLong;
    rDeletedArray.push_back(aDeletedString);
}

IMPL_LINK(OfaAutocorrReplacePage, NewDelHdl, PushButton*, pBtn)
{
    SvLBoxEntry* pEntry = aReplaceTLB.FirstSelected();
    if( pBtn == &aDeleteReplacePB )
    {
        DBG_ASSERT( pEntry, "no entry selected" );
        if( pEntry )
        {
            DeleteEntry(aReplaceTLB.GetEntryText(pEntry, 0), aReplaceTLB.GetEntryText(pEntry, 1));
            aReplaceTLB.GetModel()->Remove(pEntry);
            ModifyHdl(&aShortED);
            return 0;
        }
    }
    if(pBtn == &aNewReplacePB || aNewReplacePB.IsEnabled())
    {
        SvLBoxEntry* _pNewEntry = aReplaceTLB.FirstSelected();
        String sEntry(aShortED.GetText());
        if(sEntry.Len() && ( aReplaceED.GetText().Len() ||
                ( bHasSelectionText && bSWriter ) ))
        {
            NewEntry(aShortED.GetText(), aReplaceED.GetText());
            aReplaceTLB.SetUpdateMode(sal_False);
            sal_uInt32 nPos = UINT_MAX;
            sEntry += '\t';
            sEntry += aReplaceED.GetText();
            if(_pNewEntry)
            {
                nPos = (sal_uInt32) aReplaceTLB.GetModel()->GetAbsPos(_pNewEntry);
                aReplaceTLB.GetModel()->Remove(_pNewEntry);
            }
            else
            {
                sal_uInt32 j;
                for( j = 0; j < aReplaceTLB.GetEntryCount(); j++ )
                {
                    SvLBoxEntry* pReplaceEntry = aReplaceTLB.GetEntry(j);
                    if( 0 >=  pCompareClass->compareString(sEntry, aReplaceTLB.GetEntryText(pReplaceEntry, 0) ) )
                        break;
                }
                nPos = j;
            }
            SvLBoxEntry* pInsEntry = aReplaceTLB.InsertEntry(
                                        sEntry, static_cast< SvLBoxEntry * >(NULL), false,
                                        nPos == UINT_MAX ? LIST_APPEND : nPos);
            if( !bReplaceEditChanged && !aTextOnlyCB.IsChecked())
            {
                pInsEntry->SetUserData(&bHasSelectionText); // new formatted text
            }

            aReplaceTLB.MakeVisible( pInsEntry );
            aReplaceTLB.SetUpdateMode( sal_True );
            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if(aReplaceED.HasFocus())
            {
                aShortED.GrabFocus();
            }
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return 0;
    }
    ModifyHdl( &aShortED );
    return 1;
}

IMPL_LINK(OfaAutocorrReplacePage, ModifyHdl, Edit*, pEdt)
{
    SvLBoxEntry* pFirstSel = aReplaceTLB.FirstSelected();
    sal_Bool bShort = pEdt == &aShortED;
    const String rEntry = pEdt->GetText();
    const String rRepString = aReplaceED.GetText();
    String aWordStr( pCharClass->lowercase( rEntry ));

    if(bShort)
    {
        if(rEntry.Len())
        {
            sal_Bool bFound = sal_False;
            sal_Bool bTmpSelEntry=sal_False;

            for(sal_uInt32 i = 0; i < aReplaceTLB.GetEntryCount(); i++)
            {
                SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( i );
                String aTestStr=aReplaceTLB.GetEntryText(pEntry, 0);
                if( pCompareClass->compareString(rEntry, aTestStr ) == 0 )
                {
                    if( rRepString.Len() )
                    {
                        bFirstSelect = sal_True;
                    }
                    aReplaceTLB.SetCurEntry(pEntry);
                    pFirstSel = pEntry;
                    aNewReplacePB.SetText(sModify);
                    bFound = sal_True;
                    break;
                }
                else
                {
                    aTestStr = pCharClass->lowercase( aTestStr );
                    if( aTestStr.Search(aWordStr) == 0 && !bTmpSelEntry )
                    {
                        aReplaceTLB.MakeVisible( pEntry );
                        bTmpSelEntry = sal_True;
                    }
                }
            }
            if( !bFound )
            {
                aReplaceTLB.SelectAll( sal_False );
                pFirstSel = 0;
                aNewReplacePB.SetText( sNew );
                if( bReplaceEditChanged )
                    aTextOnlyCB.Enable(sal_False);
            }
            aDeleteReplacePB.Enable( bFound );
        }
        else if( aReplaceTLB.GetEntryCount() > 0 )
        {
            SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( 0 );
            aReplaceTLB.MakeVisible( pEntry );
        }

    }
    else if( !bShort )
    {
        bReplaceEditChanged = sal_True;
        if( pFirstSel )
        {
            aNewReplacePB.SetText( sModify );
        }
    }

    const String& rShortTxt = aShortED.GetText();
    sal_Bool bEnableNew = rShortTxt.Len() &&
                        ( rRepString.Len() ||
                                ( bHasSelectionText && bSWriter )) &&
                        ( !pFirstSel || rRepString !=
                                aReplaceTLB.GetEntryText( pFirstSel, 1 ) );
    if( bEnableNew )
    {
        for(std::set<rtl::OUString>::iterator i = aFormatText.begin(); i != aFormatText.end(); ++i)
        {
            if((*i).equals(rShortTxt))
            {
                bEnableNew = sal_False;
                break;
            }
        }
    }
    aNewReplacePB.Enable( bEnableNew );

    return 0;
}

sal_Bool lcl_FindInArray(std::vector<rtl::OUString>& rStrings, const String& rString)
{
    for(std::vector<rtl::OUString>::iterator i = rStrings.begin(); i != rStrings.end(); ++i)
    {
        if((*i).equals(rString))
        {
            return sal_True;
        }
    }
    return sal_False;
}

OfaAutocorrExceptPage::OfaAutocorrExceptPage( Window* pParent,
                                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_AUTOCORR_EXCEPT ), rSet),
    aAbbrevFL       (this, CUI_RES(FL_ABBREV         )),
    aAbbrevED       (this, CUI_RES(ED_ABBREV         )),
    aAbbrevLB       (this, CUI_RES(LB_ABBREV         )),
    aNewAbbrevPB    (this, CUI_RES(PB_NEWABBREV  )),
    aDelAbbrevPB    (this, CUI_RES(PB_DELABBREV  )),
    aAutoAbbrevCB   (this, CUI_RES(CB_AUTOABBREV     )),
    aDoubleCapsFL   (this, CUI_RES(FL_DOUBLECAPS     )),
    aDoubleCapsED   (this, CUI_RES(ED_DOUBLE_CAPS    )),
    aDoubleCapsLB   (this, CUI_RES(LB_DOUBLE_CAPS    )),
    aNewDoublePB    (this, CUI_RES(PB_NEWDOUBLECAPS)),
    aDelDoublePB    (this, CUI_RES(PB_DELDOUBLECAPS)),
    aAutoCapsCB     (this, CUI_RES(CB_AUTOCAPS   )),
    eLang(eLastDialogLanguage)
{
    aNewAbbrevPB.SetAccessibleName(String(CUI_RES(STR_PB_NEWABBREV) ) );
    aDelAbbrevPB.SetAccessibleName(String(CUI_RES(STR_PB_DELABBREV) ) );
    aNewDoublePB.SetAccessibleName(String(CUI_RES(STR_PB_NEWDOUBLECAPS) ) );
    aDelDoublePB.SetAccessibleName(String(CUI_RES(STR_PB_DELDOUBLECAPS) ) );

    FreeResource();

    ::com::sun::star::lang::Locale aLcl( SvxCreateLocale(eLastDialogLanguage ));
    pCompareClass = new CollatorWrapper( GetProcessFact() );
    pCompareClass->loadDefaultCollator( aLcl, ::com::sun::star::i18n::
                            CollatorOptions::CollatorOptions_IGNORE_CASE );

    aNewAbbrevPB.SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    aDelAbbrevPB.SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    aNewDoublePB.SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    aDelDoublePB.SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));

    aAbbrevLB.SetSelectHdl(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    aDoubleCapsLB.SetSelectHdl(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    aAbbrevED.SetModifyHdl(LINK(this, OfaAutocorrExceptPage, ModifyHdl));
    aDoubleCapsED.SetModifyHdl(LINK(this, OfaAutocorrExceptPage, ModifyHdl));

    aAbbrevED.SetActionHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    aDoubleCapsED.SetActionHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));

    aNewAbbrevPB.SetAccessibleRelationMemberOf(&aAbbrevFL);
    aDelAbbrevPB.SetAccessibleRelationMemberOf(&aAbbrevFL);
    aNewDoublePB.SetAccessibleRelationMemberOf(&aDoubleCapsFL);
    aDelDoublePB.SetAccessibleRelationMemberOf(&aDoubleCapsFL);
}

OfaAutocorrExceptPage::~OfaAutocorrExceptPage()
{
    aStringsTable.clear();
    delete pCompareClass;
}

SfxTabPage* OfaAutocorrExceptPage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrExceptPage(pParent, rSet);
}

void    OfaAutocorrExceptPage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_True);
}

int     OfaAutocorrExceptPage::DeactivatePage( SfxItemSet* )
{
    return LEAVE_PAGE;
}

sal_Bool OfaAutocorrExceptPage::FillItemSet( SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    for(StringsTable::reverse_iterator it1 = aStringsTable.rbegin(); it1 != aStringsTable.rend(); ++it1)
    {
        LanguageType eCurLang = it1->first;
        StringsArrays& rArrays = it1->second;
        if(eCurLang != eLang) // current language is treated later
        {
            SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eCurLang);

            if(pWrdList)
            {
                sal_uInt16 nCount = pWrdList->size();
                sal_uInt16 i;
                for( i = nCount; i; )
                {
                    String* pString = (*pWrdList)[ --i ];

                    if( !lcl_FindInArray(rArrays.aDoubleCapsStrings, *pString))
                    {
                      delete (*pWrdList)[ i ];
                      pWrdList->erase(i);
                    }
                }

                for(std::vector<rtl::OUString>::iterator it = rArrays.aDoubleCapsStrings.begin(); it != rArrays.aDoubleCapsStrings.end(); ++i)
                {
                    String* s = new String(*it);
                    if(!pWrdList->insert(s).second)
                        delete s;
                }
                pAutoCorrect->SaveWrdSttExceptList(eCurLang);
            }

            SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eCurLang);

            if(pCplList)
            {
                sal_uInt16 nCount = pCplList->size();
                sal_uInt16 i;
                for( i = nCount; i; )
                {
                    String* pString = (*pCplList)[ --i ];
                    if( !lcl_FindInArray(rArrays.aAbbrevStrings, *pString))
                    {
                        delete (*pCplList)[ i ];
                        pCplList->erase(i);
                    }
                }

                for(std::vector<rtl::OUString>::iterator it = rArrays.aAbbrevStrings.begin(); it != rArrays.aAbbrevStrings.end(); ++it)
                {
                    String* s = new String(*it);
                    if(!pCplList->insert(s).second)
                        delete s;
                }

                pAutoCorrect->SaveCplSttExceptList(eCurLang);
            }
        }
    }
    aStringsTable.clear();

    SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eLang);

    if(pWrdList)
    {
        sal_uInt16 nCount = pWrdList->size();
        sal_uInt16 i;
        for( i = nCount; i; )
        {
            String* pString = (*pWrdList)[ --i ];
            if( USHRT_MAX == aDoubleCapsLB.GetEntryPos(*pString) )
            {
                delete (*pWrdList)[ i ];
                pWrdList->erase(i);
            }
        }
        nCount = aDoubleCapsLB.GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            String* pEntry = new String( aDoubleCapsLB.GetEntry( i ) );
            if( !pWrdList->insert( pEntry ).second)
                delete pEntry;
        }
        pAutoCorrect->SaveWrdSttExceptList(eLang);
    }

    SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eLang);

    if(pCplList)
    {
        sal_uInt16 nCount = pCplList->size();
        sal_uInt16 i;
        for( i = nCount; i; )
        {
            String* pString = (*pCplList)[ --i ];
            if( USHRT_MAX == aAbbrevLB.GetEntryPos(*pString) )
            {
                delete (*pCplList)[ i ];
                pCplList->erase(i);
            }
        }
        nCount = aAbbrevLB.GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            String* pEntry = new String( aAbbrevLB.GetEntry( i ) );
            if( !pCplList->insert( pEntry ).second)
                delete pEntry;
        }
        pAutoCorrect->SaveCplSttExceptList(eLang);
    }
    if(aAutoAbbrevCB.IsChecked() != aAutoAbbrevCB.GetSavedValue())
        pAutoCorrect->SetAutoCorrFlag( SaveWordCplSttLst, aAutoAbbrevCB.IsChecked());
    if(aAutoCapsCB.IsChecked() != aAutoCapsCB.GetSavedValue())
        pAutoCorrect->SetAutoCorrFlag( SaveWordWrdSttLst, aAutoCapsCB.IsChecked());
    return sal_False;
}

void OfaAutocorrExceptPage::SetLanguage(LanguageType eSet)
{
    if(eLang != eSet)
    {
        // save old settings and fill anew
        RefillReplaceBoxes(sal_False, eLang, eSet);
        eLastDialogLanguage = eSet;
        delete pCompareClass;
        pCompareClass = new CollatorWrapper( GetProcessFact() );
        pCompareClass->loadDefaultCollator( SvxCreateLocale( eLastDialogLanguage ),
                        ::com::sun::star::i18n::
                            CollatorOptions::CollatorOptions_IGNORE_CASE );
        ModifyHdl(&aAbbrevED);
        ModifyHdl(&aDoubleCapsED);
    }
}

void OfaAutocorrExceptPage::RefillReplaceBoxes(sal_Bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
    {
        aStringsTable.clear();
    }
    else
    {
        StringsArrays* pArrays;
        if(aStringsTable.find(eOldLanguage) != aStringsTable.end())
        {
            pArrays = &aStringsTable[eOldLanguage];
            pArrays->aAbbrevStrings.clear();
            pArrays->aDoubleCapsStrings.clear();
        }
        else
        {
            pArrays = &aStringsTable[eOldLanguage]; // create new array
        }

        sal_uInt16 i;
        for(i = 0; i < aAbbrevLB.GetEntryCount(); i++)
            pArrays->aAbbrevStrings.push_back(rtl::OUString(aAbbrevLB.GetEntry(i)));

        for(i = 0; i < aDoubleCapsLB.GetEntryCount(); i++)
            pArrays->aDoubleCapsStrings.push_back(rtl::OUString(aDoubleCapsLB.GetEntry(i)));
    }
    aDoubleCapsLB.Clear();
    aAbbrevLB.Clear();
    String sTemp;
    aAbbrevED.SetText(sTemp);
    aDoubleCapsED.SetText(sTemp);

    if(aStringsTable.find(eLang) != aStringsTable.end())
    {
        StringsArrays& rArrays = aStringsTable[eLang];
        for(std::vector<rtl::OUString>::iterator i = rArrays.aAbbrevStrings.begin(); i != rArrays.aAbbrevStrings.end(); ++i)
            aAbbrevLB.InsertEntry(*i);

        for(std::vector<rtl::OUString>::iterator i = rArrays.aDoubleCapsStrings.begin(); i != rArrays.aDoubleCapsStrings.end(); ++i)
            aDoubleCapsLB.InsertEntry(*i);
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        const SvStringsISortDtor* pCplList = pAutoCorrect->GetCplSttExceptList(eLang);
        const SvStringsISortDtor* pWrdList = pAutoCorrect->GetWrdSttExceptList(eLang);
        sal_uInt16 i;
        for( i = 0; i < pCplList->size(); i++ )
        {
            aAbbrevLB.InsertEntry(*(*pCplList)[i]);
        }
        for( i = 0; i < pWrdList->size(); i++ )
        {
            aDoubleCapsLB.InsertEntry(*(*pWrdList)[i]);
        }
    }
}

void OfaAutocorrExceptPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    RefillReplaceBoxes(sal_True, eLang, eLang);
    aAutoAbbrevCB.  Check(  pAutoCorrect->IsAutoCorrFlag( SaveWordCplSttLst ));
    aAutoCapsCB.    Check(  pAutoCorrect->IsAutoCorrFlag( SaveWordWrdSttLst ));
    aAutoAbbrevCB.SaveValue();
    aAutoCapsCB.SaveValue();
}

IMPL_LINK(OfaAutocorrExceptPage, NewDelHdl, PushButton*, pBtn)
{
    if((pBtn == &aNewAbbrevPB || pBtn == (PushButton*)&aAbbrevED )
        && aAbbrevED.GetText().Len())
    {
        aAbbrevLB.InsertEntry(aAbbrevED.GetText());
        ModifyHdl(&aAbbrevED);
    }
    else if(pBtn == &aDelAbbrevPB)
    {
        aAbbrevLB.RemoveEntry(aAbbrevED.GetText());
        ModifyHdl(&aAbbrevED);
    }
    else if((pBtn == &aNewDoublePB || pBtn == (PushButton*)&aDoubleCapsED )
            && aDoubleCapsED.GetText().Len())
    {
        aDoubleCapsLB.InsertEntry(aDoubleCapsED.GetText());
        ModifyHdl(&aDoubleCapsED);
    }
    else if(pBtn == &aDelDoublePB)
    {
        aDoubleCapsLB.RemoveEntry(aDoubleCapsED.GetText());
        ModifyHdl(&aDoubleCapsED);
    }
    return 0;
}

IMPL_LINK(OfaAutocorrExceptPage, SelectHdl, ListBox*, pBox)
{
    if(pBox == &aAbbrevLB)
    {
        aAbbrevED.SetText(pBox->GetSelectEntry());
        aNewAbbrevPB.Enable(sal_False);
        aDelAbbrevPB.Enable();
    }
    else
    {
        aDoubleCapsED.SetText(pBox->GetSelectEntry());
        aNewDoublePB.Enable(sal_False);
        aDelDoublePB.Enable();
    }
    return 0;
}

IMPL_LINK(OfaAutocorrExceptPage, ModifyHdl, Edit*, pEdt)
{
//  sal_Bool bSame = pEdt->GetText() == ->GetSelectEntry();
    const String& sEntry = pEdt->GetText();
    sal_Bool bEntryLen = 0!= sEntry.Len();
    if(pEdt == &aAbbrevED)
    {
        sal_Bool bSame = lcl_FindEntry(aAbbrevLB, sEntry, *pCompareClass);
        if(bSame && sEntry != aAbbrevLB.GetSelectEntry())
            pEdt->SetText(aAbbrevLB.GetSelectEntry());
        aNewAbbrevPB.Enable(!bSame && bEntryLen);
        aDelAbbrevPB.Enable(bSame && bEntryLen);
    }
    else
    {
        sal_Bool bSame = lcl_FindEntry(aDoubleCapsLB, sEntry, *pCompareClass);
        if(bSame && sEntry != aDoubleCapsLB.GetSelectEntry())
            pEdt->SetText(aDoubleCapsLB.GetSelectEntry());
        aNewDoublePB.Enable(!bSame && bEntryLen);
        aDelDoublePB.Enable(bSame && bEntryLen);
    }
    return 0;
}

void AutoCorrEdit::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode aKeyCode = rKEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN )
    {
        // if there's nothing done on enter, call the
        // base class after all to close the dialog
        if(!nModifier && !aActionLink.Call(this))
                 Edit::KeyInput(rKEvt);
    }
    else if(bSpaces || aKeyCode.GetCode() != KEY_SPACE)
        Edit::KeyInput(rKEvt);
}

enum OfaQuoteOptions
{
    ADD_NONBRK_SPACE,
    REPLACE_1ST
};

SvLBoxEntry* OfaQuoteTabPage::CreateEntry(String& rTxt, sal_uInt16 nCol)
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if ( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData( &aSwCheckLB );
        aSwCheckLB.SetCheckButtonData( pCheckButtonData );
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));

    String sEmpty;
    if (nCol == CBCOL_SECOND)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );

    pEntry->AddItem( new OfaImpBrwString( pEntry, 0, rTxt ) );

    return pEntry;
}

OfaQuoteTabPage::OfaQuoteTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_AUTOCORR_QUOTE ), rSet),
    aCheckLB            (this, CUI_RES(CLB_SETTINGS     )),
    m_aSwCheckLBContainer(this, CUI_RES(CLB_SETTINGS)),
    aSwCheckLB(m_aSwCheckLBContainer),
    sHeader1            (CUI_RES( STR_HEADER1           )),
    sHeader2            (CUI_RES( STR_HEADER2           )),
    sNonBrkSpace        (CUI_RES( ST_NON_BREAK_SPACE    )),
    sOrdinal            (CUI_RES( ST_ORDINAL            )),
    pCheckButtonData    ( NULL ),

    aSingleFL           (this, CUI_RES(FL_SINGLE         )),
    aSingleTypoCB       (this, CUI_RES(CB_SGL_TYPO     )),
    aSglStartQuoteFT    (this, CUI_RES(FT_SGL_STARTQUOTE )),
    aSglStartQuotePB    (this, CUI_RES(PB_SGL_STARTQUOTE )),
    aSglStartExFT       (this, CUI_RES(FT_SGSTEX       )),
    aSglEndQuoteFT      (this, CUI_RES(FT_SGL_ENDQUOTE   )),
    aSglEndQuotePB      (this, CUI_RES(PB_SGL_ENDQUOTE   )),
    aSglEndExFT         (this, CUI_RES(FT_SGENEX       )),
    aSglStandardPB      (this, CUI_RES(PB_SGL_STD      )),

    aDoubleFL           (this, CUI_RES(FL_DOUBLE       )),
    aTypoCB             (this, CUI_RES(CB_TYPO         )),
    aStartQuoteFT       (this, CUI_RES(FT_STARTQUOTE   )),
    aStartQuotePB       (this, CUI_RES(PB_STARTQUOTE   )),
    aDblStartExFT       (this, CUI_RES(FT_DBSTEX       )),
    aEndQuoteFT         (this, CUI_RES(FT_ENDQUOTE     )),
    aEndQuotePB         (this, CUI_RES(PB_ENDQUOTE     )),
    aDblEndExFT         (this, CUI_RES(FT_DBECEX       )),
    aDblStandardPB      (this, CUI_RES(PB_DBL_STD      )),

    sStartQuoteDlg  (CUI_RES(STR_CHANGE_START)),
    sEndQuoteDlg    (CUI_RES(STR_CHANGE_END)),

    sStandard(CUI_RES(ST_STANDARD))
{
    aSglStandardPB.SetAccessibleName(String(CUI_RES(STR_PB_SGL_STD) ) );
    aDblStandardPB.SetAccessibleName(String(CUI_RES(STR_PB_DBL_STD) ) );
    aStartQuotePB.SetAccessibleName( String(CUI_RES(STR_PB_DBL_START) ));
    aEndQuotePB.SetAccessibleName(String(CUI_RES(STR_PB_DBL_END) ));
    aSglStartQuotePB.SetAccessibleName(String(CUI_RES(STR_PB_SGL_START) ));
    aSglEndQuotePB.SetAccessibleName(String(CUI_RES(STR_PB_SGL_END) ) );

    FreeResource();

    sal_Bool bShowSWOptions = sal_False;

    aCheckLB.SetHelpId( HID_OFAPAGE_QUOTE_CLB );
    aSwCheckLB.SetHelpId( HID_OFAPAGE_QUOTE_SW_CLB );

    SFX_ITEMSET_ARG( &rSet, pItem, SfxBoolItem, SID_AUTO_CORRECT_DLG, sal_False );
    if ( pItem && pItem->GetValue() )
        bShowSWOptions = sal_True;

    if ( bShowSWOptions )
    {
        static long aStaticTabs[]=
        {
            3, 0, 20, 40
        };

        aSwCheckLB.SetStyle(aSwCheckLB.GetStyle() | WB_HSCROLL| WB_VSCROLL);

        aSwCheckLB.SvxSimpleTable::SetTabs(aStaticTabs);
        String sHeader( sHeader1 );
        sHeader += '\t';
        sHeader += sHeader2;
        sHeader += '\t';
        aSwCheckLB.InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                        HIB_CENTER | HIB_VCENTER | HIB_FIXEDPOS | HIB_FIXED);
        aCheckLB.Hide( sal_True );
    }
    else
    {
        aSwCheckLB.HideTable( );
    }

    aStartQuotePB.SetClickHdl(LINK(this,    OfaQuoteTabPage, QuoteHdl));
    aEndQuotePB.SetClickHdl(LINK(this,      OfaQuoteTabPage, QuoteHdl));
    aSglStartQuotePB.SetClickHdl(LINK(this, OfaQuoteTabPage, QuoteHdl));
    aSglEndQuotePB.SetClickHdl(LINK(this,   OfaQuoteTabPage, QuoteHdl));
    aDblStandardPB.SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));
    aSglStandardPB.SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));

    aSglStartQuotePB.SetAccessibleRelationLabeledBy( &aSglStartQuoteFT );
    aSglEndQuotePB.SetAccessibleRelationLabeledBy( &aSglEndQuoteFT );
    aSglStartQuotePB.SetAccessibleRelationMemberOf( &aSingleFL );
    aSglEndQuotePB.SetAccessibleRelationMemberOf( &aSingleFL );
    aStartQuotePB.SetAccessibleRelationLabeledBy( &aStartQuoteFT );
    aEndQuotePB.SetAccessibleRelationLabeledBy( &aEndQuoteFT );
    aStartQuotePB.SetAccessibleRelationMemberOf( &aDoubleFL );
    aEndQuotePB.SetAccessibleRelationMemberOf( &aDoubleFL );
    aSglStandardPB.SetAccessibleRelationMemberOf(&aSingleFL);
    aDblStandardPB.SetAccessibleRelationMemberOf(&aDoubleFL);
}

OfaQuoteTabPage::~OfaQuoteTabPage()
{
    delete( pCheckButtonData );
}

SfxTabPage* OfaQuoteTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new OfaQuoteTabPage(pParent, rAttrSet);
}

sal_Bool OfaQuoteTabPage::FillItemSet( SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    long nFlags = pAutoCorrect->GetFlags();

    if ( aCheckLB.IsVisible( ) )
    {
        sal_uInt16 nPos = 0;
        pAutoCorrect->SetAutoCorrFlag(AddNonBrkSpace,       aCheckLB.IsChecked(nPos++));
        pAutoCorrect->SetAutoCorrFlag(ChgOrdinalNumber,     aCheckLB.IsChecked(nPos++));
    }

    sal_Bool bModified = sal_False;
    if ( aSwCheckLB.IsVisible( ) )
    {
        SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

        sal_Bool bCheck = aSwCheckLB.IsChecked(ADD_NONBRK_SPACE, CBCOL_FIRST);
        bModified |= pOpt->bAddNonBrkSpace != bCheck;
        pOpt->bAddNonBrkSpace = bCheck;
        pAutoCorrect->SetAutoCorrFlag(AddNonBrkSpace,
                            aSwCheckLB.IsChecked(ADD_NONBRK_SPACE, CBCOL_SECOND));

        bCheck = aSwCheckLB.IsChecked(REPLACE_1ST, CBCOL_FIRST);
        bModified |= pOpt->bChgOrdinalNumber != bCheck;
        pOpt->bChgOrdinalNumber = bCheck;
        pAutoCorrect->SetAutoCorrFlag(ChgOrdinalNumber,
                        aSwCheckLB.IsChecked(REPLACE_1ST, CBCOL_SECOND));
    }

    pAutoCorrect->SetAutoCorrFlag(ChgQuotes,        aTypoCB.IsChecked());
    pAutoCorrect->SetAutoCorrFlag(ChgSglQuotes,     aSingleTypoCB.IsChecked());
    sal_Bool bReturn = nFlags != pAutoCorrect->GetFlags();
    if(cStartQuote != pAutoCorrect->GetStartDoubleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cStartQuote); //TODO
        pAutoCorrect->SetStartDoubleQuote(cUCS2);
    }
    if(cEndQuote != pAutoCorrect->GetEndDoubleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cEndQuote); //TODO
        pAutoCorrect->SetEndDoubleQuote(cUCS2);
    }
    if(cSglStartQuote != pAutoCorrect->GetStartSingleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cSglStartQuote); //TODO
        pAutoCorrect->SetStartSingleQuote(cUCS2);
    }
    if(cSglEndQuote != pAutoCorrect->GetEndSingleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cSglEndQuote); //TODO
        pAutoCorrect->SetEndSingleQuote(cUCS2);
    }

    if( bModified || bReturn )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return bReturn;
}

void OfaQuoteTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_False);
}

void OfaQuoteTabPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const long nFlags = pAutoCorrect->GetFlags();

    // Initialize the Sw options
    if ( aSwCheckLB.IsVisible( ) )
    {
        SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

        aSwCheckLB.SetUpdateMode( sal_False );
        aSwCheckLB.Clear();

        aSwCheckLB.GetModel()->Insert(CreateEntry(sNonBrkSpace,       CBCOL_BOTH ));
        aSwCheckLB.GetModel()->Insert(CreateEntry(sOrdinal,           CBCOL_BOTH ));

        aSwCheckLB.CheckEntryPos( ADD_NONBRK_SPACE, CBCOL_FIRST,    pOpt->bAddNonBrkSpace );
        aSwCheckLB.CheckEntryPos( ADD_NONBRK_SPACE, CBCOL_SECOND,   0 != (nFlags & AddNonBrkSpace) );
        aSwCheckLB.CheckEntryPos( REPLACE_1ST,      CBCOL_FIRST,    pOpt->bChgOrdinalNumber );
        aSwCheckLB.CheckEntryPos( REPLACE_1ST,      CBCOL_SECOND,   0 != (nFlags & ChgOrdinalNumber) );

        aSwCheckLB.SetUpdateMode( sal_True );
    }

    // Initialize the non Sw options
    if ( aCheckLB.IsVisible( ) )
    {
        aCheckLB.SetUpdateMode( sal_False );
        aCheckLB.Clear( );

        aCheckLB.InsertEntry( sNonBrkSpace );
        aCheckLB.InsertEntry( sOrdinal );

        sal_uInt16 nPos = 0;
        aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & AddNonBrkSpace) );
        aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgOrdinalNumber) );

        aCheckLB.SetUpdateMode( sal_True );
    }

    // Initialize the quote stuffs
    aTypoCB             .Check(0 != (nFlags & ChgQuotes));
    aSingleTypoCB       .Check(0 != (nFlags & ChgSglQuotes));
    aTypoCB             .SaveValue();
    aSingleTypoCB       .SaveValue();

    cStartQuote = pAutoCorrect->GetStartDoubleQuote();
    cEndQuote = pAutoCorrect->GetEndDoubleQuote();
    cSglStartQuote = pAutoCorrect->GetStartSingleQuote();
    cSglEndQuote = pAutoCorrect->GetEndSingleQuote();

    aSglStartExFT .SetText(ChangeStringExt_Impl(cSglStartQuote));
    aSglEndExFT   .SetText(ChangeStringExt_Impl(cSglEndQuote));
    aDblStartExFT .SetText(ChangeStringExt_Impl(cStartQuote));
    aDblEndExFT   .SetText(ChangeStringExt_Impl(cEndQuote));
}

#define SGL_START       0
#define DBL_START       1
#define SGL_END         2
#define DBL_END         3


IMPL_LINK( OfaQuoteTabPage, QuoteHdl, PushButton*, pBtn )
{
    sal_uInt16 nMode = SGL_START;
    if(pBtn == &aSglEndQuotePB)
        nMode = SGL_END;
    else if(pBtn == &aStartQuotePB)
        nMode = DBL_START;
    else if(pBtn == &aEndQuotePB)
        nMode = DBL_END;
    // start character selection dialog
    SvxCharacterMap* pMap = new SvxCharacterMap( this, sal_True );
    pMap->SetCharFont( OutputDevice::GetDefaultFont(DEFAULTFONT_LATIN_TEXT,
                        LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE, 0 ));
    pMap->SetText(nMode < SGL_END ? sStartQuoteDlg  :  sEndQuoteDlg );
    sal_UCS4 cDlg;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    LanguageType eLang = Application::GetSettings().GetLanguage();
    switch( nMode )
    {
        case SGL_START:
            cDlg = cSglStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'', sal_True, eLang);
        break;
        case SGL_END:
            cDlg = cSglEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'', sal_False, eLang);
        break;
        case DBL_START:
            cDlg = cStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"', sal_True, eLang);
        break;
        case DBL_END:
            cDlg = cEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"', sal_False, eLang);
        break;
        default:
            OSL_FAIL("svx::OfaQuoteTabPage::QuoteHdl(), how to initialize cDlg?" );
            cDlg = 0;
            break;

    }
    pMap->SetChar(  cDlg );
    pMap->DisableFontSelection();
    if(pMap->Execute() == RET_OK)
    {
        sal_UCS4 cNewChar = pMap->GetChar();
        switch( nMode )
        {
            case SGL_START:
                cSglStartQuote = cNewChar;
                aSglStartExFT.SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case SGL_END:
                cSglEndQuote = cNewChar;
                aSglEndExFT.SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_START:
                cStartQuote = cNewChar;
                aDblStartExFT.SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_END:
                cEndQuote = cNewChar;
                aDblEndExFT.SetText(ChangeStringExt_Impl(cNewChar));
            break;
        }
    }
    delete pMap;

    return 0;
}

IMPL_LINK( OfaQuoteTabPage, StdQuoteHdl, PushButton*, pBtn )
{
    if(pBtn == &aDblStandardPB)
    {
        cStartQuote = 0;
        aDblStartExFT.SetText(ChangeStringExt_Impl(0));
        cEndQuote = 0;
        aDblEndExFT.SetText(ChangeStringExt_Impl(0));

    }
    else
    {
        cSglStartQuote = 0;
        aSglStartExFT.SetText(ChangeStringExt_Impl(0));
        cSglEndQuote = 0;
        aSglEndExFT.SetText(ChangeStringExt_Impl(0));
    }
    return 0;
}

// --------------------------------------------------

String OfaQuoteTabPage::ChangeStringExt_Impl( sal_UCS4 cChar )
{
    if( !cChar )
        return sStandard;

    // convert codepoint value to unicode-hex string
    sal_UCS4 aStrCodes[32] = { 0, ' ', '(', 'U', '+', '0' };
    aStrCodes[0] = cChar;
    int nFullLen = 5;
    int nHexLen = 4;
    while( (cChar >> (4*nHexLen)) != 0 )
        ++nHexLen;
    for( int i = nHexLen; --i >= 0;)
    {
        sal_UCS4 cHexDigit = ((cChar >> (4*i)) & 0x0f) + '0';
        if( cHexDigit > '9' )
            cHexDigit += 'A' - ('9' + 1);
        aStrCodes[ nFullLen++ ] = cHexDigit;
    }
    aStrCodes[ nFullLen++ ] = ')';
    // using the new UCS4 constructor
    rtl::OUString aOUStr( aStrCodes, nFullLen );
    return aOUStr;
}

OfaAutoCompleteTabPage::OfaAutoCompleteTabPage( Window* pParent,
                                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_AUTOCOMPLETE_OPTIONS ), rSet),
    aCBActiv        (this, CUI_RES(CB_ACTIV)),
    aCBAppendSpace  (this, CUI_RES(CB_APPEND_SPACE)),
    aCBAsTip        (this, CUI_RES(CB_AS_TIP)),
    aCBCollect      (this, CUI_RES(CB_COLLECT)),
    aCBRemoveList   (this, CUI_RES(CB_REMOVE_LIST)),
    aFTExpandKey    (this, CUI_RES(FT_EXPAND_KEY)),
    aDCBExpandKey   (this, CUI_RES(DCB_EXPAND_KEY)),
    aFTMinWordlen   (this, CUI_RES(FT_MIN_WORDLEN)),
    aNFMinWordlen   (this, CUI_RES(NF_MIN_WORDLEN)),
    aFTMaxEntries   (this, CUI_RES(FT_MAX_ENTRIES)),
    aNFMaxEntries   (this, CUI_RES(NF_MAX_ENTRIES)),
    aLBEntries      (*this, CUI_RES(LB_ENTRIES)),
    aPBEntries      (this, CUI_RES(PB_ENTRIES)),
    m_pAutoCompleteList( 0 ),
    nAutoCmpltListCnt( 0 )
{
    FreeResource();

    // the defined KEYs
    static const sal_uInt16 aKeyCodes[] = {
        KEY_END,
        KEY_RETURN,
        KEY_SPACE,
        KEY_RIGHT,
        KEY_TAB,
        0
    };

    for( const sal_uInt16* pKeys = aKeyCodes; *pKeys; ++pKeys )
    {
        KeyCode aKCode( *pKeys );
        sal_uInt16 nPos = aDCBExpandKey.InsertEntry( aKCode.GetName() );
        aDCBExpandKey.SetEntryData( nPos, (void*)(sal_uLong)*pKeys );
        if( KEY_RETURN == *pKeys )      // default to RETURN
            aDCBExpandKey.SelectEntryPos( nPos );
    }

    aPBEntries.SetClickHdl(LINK(this, OfaAutoCompleteTabPage, DeleteHdl));
    aCBActiv.SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    aCBCollect.SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    aLBEntries.SetAccessibleRelationLabeledBy(&aLBEntries);
}

OfaAutoCompleteTabPage::~OfaAutoCompleteTabPage()
{
}

SfxTabPage* OfaAutoCompleteTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet)
{
    return new OfaAutoCompleteTabPage( pParent, rSet );
}

sal_Bool OfaAutoCompleteTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bModified = sal_False, bCheck;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    sal_uInt16 nVal;

    bCheck = aCBActiv.IsChecked();
    bModified |= pOpt->bAutoCompleteWords != bCheck;
    pOpt->bAutoCompleteWords = bCheck;
    bCheck = aCBCollect.IsChecked();
    bModified |= pOpt->bAutoCmpltCollectWords != bCheck;
    pOpt->bAutoCmpltCollectWords = bCheck;
    bCheck = !aCBRemoveList.IsChecked(); // inverted value!
    bModified |= pOpt->bAutoCmpltKeepList != bCheck;
    pOpt->bAutoCmpltKeepList = bCheck;
    bCheck = aCBAppendSpace.IsChecked();
    bModified |= pOpt->bAutoCmpltAppendBlanc != bCheck;
    pOpt->bAutoCmpltAppendBlanc = bCheck;
    bCheck = aCBAsTip.IsChecked();
    bModified |= pOpt->bAutoCmpltShowAsTip != bCheck;
    pOpt->bAutoCmpltShowAsTip = bCheck;

    nVal = (sal_uInt16)aNFMinWordlen.GetValue();
    bModified |= nVal != pOpt->nAutoCmpltWordLen;
    pOpt->nAutoCmpltWordLen = nVal;

    nVal = (sal_uInt16)aNFMaxEntries.GetValue();
    bModified |= nVal != pOpt->nAutoCmpltListLen;
    pOpt->nAutoCmpltListLen = nVal;

    nVal = aDCBExpandKey.GetSelectEntryPos();
    if( nVal < aDCBExpandKey.GetEntryCount() )
    {
        sal_uLong nKey = (sal_uLong)aDCBExpandKey.GetEntryData( nVal );
        bModified |= nKey != pOpt->nAutoCmpltExpandKey;
        pOpt->nAutoCmpltExpandKey = (sal_uInt16)nKey;
   }

    if (m_pAutoCompleteList && nAutoCmpltListCnt != aLBEntries.GetEntryCount())
    {
        bModified = sal_True;
        pOpt->m_pAutoCompleteList = m_pAutoCompleteList;
    }
    if( bModified )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return sal_True;
}

void OfaAutoCompleteTabPage::Reset( const SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

    aCBActiv.Check( 0 != pOpt->bAutoCompleteWords );
    aCBCollect.Check( 0 != pOpt->bAutoCmpltCollectWords );
    aCBRemoveList.Check( !pOpt->bAutoCmpltKeepList ); //inverted value!
    aCBAppendSpace.Check( 0 != pOpt->bAutoCmpltAppendBlanc );
    aCBAsTip.Check( 0 != pOpt->bAutoCmpltShowAsTip );

    aNFMinWordlen.SetValue( pOpt->nAutoCmpltWordLen );
    aNFMaxEntries.SetValue( pOpt->nAutoCmpltListLen );

    // select the specific KeyCode:
    {
        sal_uLong nKey = pOpt->nAutoCmpltExpandKey;
        for( sal_uInt16 n = 0, nCnt = aDCBExpandKey.GetEntryCount(); n < nCnt; ++n )
            if( nKey == (sal_uLong)aDCBExpandKey.GetEntryData( n ))
            {
                aDCBExpandKey.SelectEntryPos( n );
                break;
            }
    }

    if (pOpt->m_pAutoCompleteList && pOpt->m_pAutoCompleteList->size())
    {
        m_pAutoCompleteList = const_cast<editeng::SortedAutoCompleteStrings*>(
                pOpt->m_pAutoCompleteList);
        pOpt->m_pAutoCompleteList = 0;
        nAutoCmpltListCnt = m_pAutoCompleteList->size();
        for (size_t n = 0; n < nAutoCmpltListCnt; ++n)
        {
            const String* pStr =
                &(*m_pAutoCompleteList)[n]->GetAutoCompleteString();
            sal_uInt16 nPos = aLBEntries.InsertEntry( *pStr );
            aLBEntries.SetEntryData( nPos, (void*)pStr );
        }
    }
    else
    {
        aLBEntries.Disable();
        aPBEntries.Disable();
    }

    CheckHdl( &aCBActiv );
    CheckHdl( &aCBCollect );
}

void OfaAutoCompleteTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage( sal_False );
}

IMPL_LINK_NOARG(OfaAutoCompleteTabPage, DeleteHdl)
{
    sal_uInt16 nSelCnt =
        (m_pAutoCompleteList) ? aLBEntries.GetSelectEntryCount() : 0;
    while( nSelCnt )
    {
        sal_uInt16 nPos = aLBEntries.GetSelectEntryPos( --nSelCnt );
        String* pStr = static_cast<String*>(aLBEntries.GetEntryData(nPos));
        aLBEntries.RemoveEntry( nPos );
        editeng::IAutoCompleteString hack(*pStr); // UGLY
        m_pAutoCompleteList->erase(&hack);
    }
    return 0;
}

IMPL_LINK( OfaAutoCompleteTabPage, CheckHdl, CheckBox*, pBox )
{
    sal_Bool bEnable = pBox->IsChecked();
    if( pBox == &aCBActiv )
    {
        aCBAppendSpace.Enable( bEnable );
        aCBAppendSpace.Enable( bEnable );
        aCBAsTip.Enable( bEnable );
        aDCBExpandKey.Enable( bEnable );
    }
    else if(&aCBCollect == pBox)
        aCBRemoveList.Enable( bEnable );
    return 0;
}

void OfaAutoCompleteTabPage::CopyToClipboard() const
{
    sal_uInt16 nSelCnt = aLBEntries.GetSelectEntryCount();
    if (m_pAutoCompleteList && nSelCnt)
    {
        TransferDataContainer* pCntnr = new TransferDataContainer;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::datatransfer::XTransferable > xRef( pCntnr );

        rtl::OStringBuffer sData;
        const sal_Char aLineEnd[] =
#if defined(WNT)
                "\015\012";
#else
                "\012";
#endif

        rtl_TextEncoding nEncode = osl_getThreadTextEncoding();

        for( sal_uInt16 n = 0; n < nSelCnt; ++n )
        {
            sData.append(rtl::OUStringToOString(aLBEntries.GetSelectEntry(n),
                nEncode));
            sData.append(RTL_CONSTASCII_STRINGPARAM(aLineEnd));
        }
        pCntnr->CopyByteString( SOT_FORMAT_STRING, sData.makeStringAndClear() );
        pCntnr->CopyToClipboard( (Window*)this );
    }
}

long OfaAutoCompleteTabPage::AutoCompleteMultiListBox::PreNotify(
            NotifyEvent& rNEvt )
{
    long nHandled = MultiListBox::PreNotify( rNEvt );

    if( !nHandled && EVENT_KEYUP == rNEvt.GetType() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
        {
        case KEY_DELETE:
            rPage.DeleteHdl( 0 );
            nHandled = 1;
            break;

        default:
            if( KEYFUNC_COPY == rKeyCode.GetFunction() )
            {
                rPage.CopyToClipboard();
                nHandled = 1;
            }
            break;
        }
    }
    return nHandled;
}

// class OfaSmartTagOptionsTabPage ---------------------------------------------

OfaSmartTagOptionsTabPage::OfaSmartTagOptionsTabPage( Window* pParent,
                                                      const SfxItemSet& rSet )
    : SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_SMARTTAG_OPTIONS ), rSet),
    m_aMainCB( this, CUI_RES(CB_SMARTTAGS) ),
    m_aSmartTagTypesLB( this, CUI_RES(LB_SMARTTAGS) ),
    m_aPropertiesPB( this, CUI_RES(PB_SMARTTAGS) ),
    m_aTitleFT( this, CUI_RES(FT_SMARTTAGS) )
{
    FreeResource();

    // some options for the list box:
    m_aSmartTagTypesLB.SetStyle( m_aSmartTagTypesLB.GetStyle() | WB_HSCROLL | WB_HIDESELECTION );
    m_aSmartTagTypesLB.SetHighlightRange();

    // set the handlers:
    m_aMainCB.SetToggleHdl(LINK(this, OfaSmartTagOptionsTabPage, CheckHdl));
    m_aPropertiesPB.SetClickHdl(LINK(this, OfaSmartTagOptionsTabPage, ClickHdl));
    m_aSmartTagTypesLB.SetSelectHdl(LINK(this, OfaSmartTagOptionsTabPage, SelectHdl));
}

OfaSmartTagOptionsTabPage::~OfaSmartTagOptionsTabPage()
{

}

SfxTabPage* OfaSmartTagOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet)
{
    return new OfaSmartTagOptionsTabPage( pParent, rSet );
}

/** This struct is used to associate list box entries with smart tag data
*/
struct ImplSmartTagLBUserData
{
    rtl::OUString maSmartTagType;
    uno::Reference< smarttags::XSmartTagRecognizer > mxRec;
    sal_Int32 mnSmartTagIdx;

    ImplSmartTagLBUserData( const rtl::OUString& rSmartTagType,
                            uno::Reference< smarttags::XSmartTagRecognizer > xRec,
                            sal_Int32 nSmartTagIdx ) :
        maSmartTagType( rSmartTagType ),
        mxRec( xRec ),
        mnSmartTagIdx( nSmartTagIdx ) {}
};

/** Clears m_aSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::ClearListBox()
{
    const sal_uLong nCount = m_aSmartTagTypesLB.GetEntryCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const SvLBoxEntry* pEntry = m_aSmartTagTypesLB.GetEntry(i);
        const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
        delete pUserData;
    }

    m_aSmartTagTypesLB.Clear();
}

/** Inserts items into m_aSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::FillListBox( const SmartTagMgr& rSmartTagMgr )
{
    // first we have to clear the list box:
    ClearListBox();

    // fill list box:
    const sal_uInt32 nNumberOfRecognizers = rSmartTagMgr.NumberOfRecognizers();
    const lang::Locale aLocale( SvxCreateLocale( eLastDialogLanguage ) );

    for ( sal_uInt32 i = 0; i < nNumberOfRecognizers; ++i )
    {
        uno::Reference< smarttags::XSmartTagRecognizer > xRec = rSmartTagMgr.GetRecognizer(i);

        const rtl::OUString aName = xRec->getName( aLocale );
        const rtl::OUString aDesc = xRec->getDescription( aLocale );
        const sal_Int32 nNumberOfSupportedSmartTags = xRec->getSmartTagCount();

        for ( sal_Int32 j = 0; j < nNumberOfSupportedSmartTags; ++j )
        {
            const rtl::OUString aSmartTagType = xRec->getSmartTagName(j);
            rtl::OUString aSmartTagCaption = rSmartTagMgr.GetSmartTagCaption( aSmartTagType, aLocale );

            if ( aSmartTagCaption.isEmpty() )
                aSmartTagCaption = aSmartTagType;

            const rtl::OUString aLBEntry = aSmartTagCaption +
                                           OUString(RTL_CONSTASCII_USTRINGPARAM(" (")) +
                                           aName +
                                           OUString(RTL_CONSTASCII_USTRINGPARAM(")"));

            SvLBoxEntry* pEntry = m_aSmartTagTypesLB.SvTreeListBox::InsertEntry( aLBEntry );
            if ( pEntry )
            {
                const bool bCheck = rSmartTagMgr.IsSmartTagTypeEnabled( aSmartTagType );
                m_aSmartTagTypesLB.SetCheckButtonState( pEntry, bCheck ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
                pEntry->SetUserData(static_cast<void*>(new ImplSmartTagLBUserData( aSmartTagType, xRec, j ) ) );
            }
        }
    }
}

/** Handler for the push button
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, ClickHdl)
{
    const sal_uInt16 nPos = m_aSmartTagTypesLB.GetSelectEntryPos();
    const SvLBoxEntry* pEntry = m_aSmartTagTypesLB.GetEntry(nPos);
    const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

     const lang::Locale aLocale( SvxCreateLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        xRec->displayPropertyPage( nSmartTagIdx, aLocale );

    return 0;
}

/** Handler for the check box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, CheckHdl)
{
    const sal_Bool bEnable = m_aMainCB.IsChecked();
    m_aSmartTagTypesLB.Enable( bEnable );
    m_aSmartTagTypesLB.Invalidate();
    m_aPropertiesPB.Enable( false );

    // if the controls are currently enabled, we still have to check
    // if the properties button should be disabled because the currently
    // seleted smart tag type does not have a properties dialog.
    // We do this by calling SelectHdl:
    if ( bEnable )
        SelectHdl( &m_aSmartTagTypesLB );

    return 0;
}

/** Handler for the list box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, SelectHdl)
{
    if ( m_aSmartTagTypesLB.GetEntryCount() < 1 )
        return 0;

    const sal_uInt16 nPos = m_aSmartTagTypesLB.GetSelectEntryPos();
    const SvLBoxEntry* pEntry = m_aSmartTagTypesLB.GetEntry(nPos);
    const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

    const lang::Locale aLocale( SvxCreateLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        m_aPropertiesPB.Enable( sal_True );
    else
        m_aPropertiesPB.Enable( sal_False );

    return 0;
}

/** Propagates the current settings to the smart tag manager.
*/
sal_Bool OfaSmartTagOptionsTabPage::FillItemSet( SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    SmartTagMgr* pSmartTagMgr = pOpt->pSmartTagMgr;

    // robust!
    if ( !pSmartTagMgr )
        return sal_False;

    sal_Bool bModifiedSmartTagTypes = sal_False;
    std::vector< rtl::OUString > aDisabledSmartTagTypes;

    const sal_uLong nCount = m_aSmartTagTypesLB.GetEntryCount();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const SvLBoxEntry* pEntry = m_aSmartTagTypesLB.GetEntry(i);
        const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
        const sal_Bool bChecked = m_aSmartTagTypesLB.IsChecked(i);
        const sal_Bool bIsCurrentlyEnabled = pSmartTagMgr->IsSmartTagTypeEnabled( pUserData->maSmartTagType );

        bModifiedSmartTagTypes = bModifiedSmartTagTypes || ( !bChecked != !bIsCurrentlyEnabled );

        if ( !bChecked )
            aDisabledSmartTagTypes.push_back( pUserData->maSmartTagType );

        delete pUserData;
    }

    const sal_Bool bModifiedRecognize = ( !m_aMainCB.IsChecked() != !pSmartTagMgr->IsLabelTextWithSmartTags() );
    if ( bModifiedSmartTagTypes || bModifiedRecognize )
    {
        bool bLabelTextWithSmartTags = m_aMainCB.IsChecked() ? true : false;
        pSmartTagMgr->WriteConfiguration( bModifiedRecognize     ? &bLabelTextWithSmartTags : 0,
                                          bModifiedSmartTagTypes ? &aDisabledSmartTagTypes : 0 );
    }

    return sal_True;
}

/** Sets the controls based on the current settings at SmartTagMgr.
*/
void OfaSmartTagOptionsTabPage::Reset( const SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const SmartTagMgr* pSmartTagMgr = pOpt->pSmartTagMgr;

    // robust, should not happen!
    if ( !pSmartTagMgr )
        return;

    FillListBox( *pSmartTagMgr );
    m_aSmartTagTypesLB.SelectEntryPos( 0 );
    m_aMainCB.Check( pSmartTagMgr->IsLabelTextWithSmartTags() );
    CheckHdl( &m_aMainCB );
}

void OfaSmartTagOptionsTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage( sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
