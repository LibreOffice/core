/*************************************************************************
 *
 *  $RCSfile: autocdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 16:09:49 $
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

#ifdef PRECOMPILED
#include "ofapch.hxx"
#endif

#pragma hdrstop

#ifndef PRECOMPILED
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_KEYCODES_HXX //autogen
#include <vcl/keycodes.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_COLLATOROPTIONS_HPP_
#include <com/sun/star/i18n/CollatorOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svtools/eitem.hxx>

#define _OFA_AUTOCDLG_CXX
#include "autocdlg.hxx"
#include "autocdlg.hrc"
#include "helpid.hrc"
#include "acorrcfg.hxx"
#include "svxacorr.hxx"
#include "charmap.hxx"
#include "unolingu.hxx"
#include "dialmgr.hxx"
#include "cuicharmap.hxx" // add CHINA001
static LanguageType eLastDialogLanguage = LANGUAGE_SYSTEM;

using namespace ::com::sun::star::util;
using namespace ::rtl;

static ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory >& GetProcessFact()
{
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();
    return xMSF;
}

/*-----------------14.10.96 15.47-------------------

--------------------------------------------------*/

OfaAutoCorrDlg::OfaAutoCorrDlg(Window* pParent, const SfxItemSet* pSet ) :
    SfxTabDialog(pParent, ResId( RID_OFA_AUTOCORR_DLG, DIALOG_MGR() ), pSet),
    aLanguageFT(    this, ResId(FT_LANG           )),
    aLanguageLB(    this, ResId(LB_LANG           ))
{
    BOOL bShowSWOptions = FALSE;
    if ( pSet )
    {
        SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_AUTO_CORRECT_DLG, FALSE );
        if ( pItem && pItem->GetValue() )
            bShowSWOptions = TRUE;
    }

    aLanguageFT.SetZOrder(0, WINDOW_ZORDER_FIRST);
    aLanguageLB.SetZOrder(&aLanguageFT, WINDOW_ZORDER_BEHIND);
    aLanguageLB.SetHelpId(HID_AUTOCORR_LANGUAGE);
    FreeResource();

    AddTabPage(RID_OFAPAGE_AUTOCORR_OPTIONS, OfaAutocorrOptionsPage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOFMT_APPLY, OfaSwAutoFmtOptionsPage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOCOMPLETE_OPTIONS,
                                        OfaAutoCompleteTabPage::Create, 0);

    if (!bShowSWOptions)
    {
        RemoveTabPage(RID_OFAPAGE_AUTOFMT_APPLY);
        RemoveTabPage(RID_OFAPAGE_AUTOCOMPLETE_OPTIONS);
    }
    else
        RemoveTabPage(RID_OFAPAGE_AUTOCORR_OPTIONS);

    AddTabPage(RID_OFAPAGE_AUTOCORR_REPLACE, OfaAutocorrReplacePage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOCORR_EXCEPT,  OfaAutocorrExceptPage::Create, 0);
    AddTabPage(RID_OFAPAGE_AUTOCORR_QUOTE,   OfaQuoteTabPage::Create, 0);

    // initialize languages
    //! LANGUAGE_NONE is displayed as '[All]' and the LanguageType
    //! will be set to LANGUAGE_DONTKNOW
    aLanguageLB.SetLanguageList( LANG_LIST_WESTERN, TRUE, TRUE );
    aLanguageLB.SelectLanguage( LANGUAGE_NONE );
    USHORT nPos = aLanguageLB.GetSelectEntryPos();
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
    aLanguageFT.SetPosSizePixel( 0, 0, aMinSize.Width() + 20, 0, WINDOW_POSSIZE_WIDTH );
}
/*-----------------16.10.96 14.06-------------------

--------------------------------------------------*/

BOOL lcl_FindEntry( ListBox& rLB, const String& rEntry,
                    CollatorWrapper& rCmpClass )
{
    USHORT nCount = rLB.GetEntryCount();
    USHORT nSelPos = rLB.GetSelectEntryPos();
    USHORT i;
    for(i = 0; i < nCount; i++)
    {
        if( 0 == rCmpClass.compareString(rEntry, rLB.GetEntry(i) ))
        {
            rLB.SelectEntryPos(i, TRUE);
            return TRUE;
        }
    }
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        rLB.SelectEntryPos(nSelPos, FALSE);
    return FALSE;
}

/* -----------------23.11.98 10:46-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(OfaAutoCorrDlg, SelectLanguageHdl, ListBox*, pBox)
{
    USHORT nPos = pBox->GetSelectEntryPos();
    void* pVoid = pBox->GetEntryData(nPos);
    LanguageType eNewLang = (LanguageType)(long)pVoid;
    //alte Einstellungen speichern und neu fuellen
    if(eNewLang != eLastDialogLanguage)
    {
        USHORT  nPageId = GetCurPageId();
        if(RID_OFAPAGE_AUTOCORR_REPLACE == nPageId)
            ((OfaAutocorrReplacePage*)GetTabPage( nPageId ))->SetLanguage(eNewLang);
        else if(RID_OFAPAGE_AUTOCORR_EXCEPT == nPageId)
            ((OfaAutocorrExceptPage*)GetTabPage( nPageId ))->SetLanguage(eNewLang);
    }
    return 0;
}
/*-----------------14.10.96 15.57-------------------

--------------------------------------------------*/

OfaAutocorrOptionsPage::OfaAutocorrOptionsPage( Window* pParent,
                                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, ResId( RID_OFAPAGE_AUTOCORR_OPTIONS, DIALOG_MGR() ), rSet),
    aCheckLB            (this, ResId(CLB_SETTINGS   )),

    sInput              (ResId(ST_USE_REPLACE       )),
    sFirst              (ResId(ST_ORDINAL           )),
    sDoubleCaps         (ResId(ST_CPTL_STT_WORD     )),
    sStartCap           (ResId(ST_CPTL_STT_SENT     )),
    sURL                (ResId(ST_DETECT_URL        )),
    sBoldUnderline      (ResId(ST_BOLD_UNDER        )),
    sHalf               (ResId(ST_FRACTION          )),
    sDash               (ResId(ST_DASH              )),
    sNoDblSpaces        (ResId(STR_NO_DBL_SPACES    ))
{
    FreeResource();

    aCheckLB.SetHelpId(HID_OFAPAGE_AUTOCORR_CLB);
}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


OfaAutocorrOptionsPage::~OfaAutocorrOptionsPage()
{
}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


SfxTabPage* OfaAutocorrOptionsPage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrOptionsPage(pParent, rSet);
}
/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


BOOL OfaAutocorrOptionsPage::FillItemSet( SfxItemSet& rSet )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    long nFlags = pAutoCorrect->GetFlags();

    USHORT nPos = 0;
    pAutoCorrect->SetAutoCorrFlag(Autocorrect,          aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CptlSttWrd,           aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CptlSttSntnc,         aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgWeightUnderl,      aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(SetINetAttr,          aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgOrdinalNumber,     aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgFractionSymbol,    aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgToEnEmDash,        aCheckLB.IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(IngnoreDoubleSpace,   aCheckLB.IsChecked(nPos++));

    BOOL bReturn = nFlags != pAutoCorrect->GetFlags();
    if(bReturn )
    {
        SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
        pCfg->SetModified();
        pCfg->Commit();
    }
    return bReturn;
}

/* -----------------23.11.98 16:15-------------------
 *
 * --------------------------------------------------*/
void    OfaAutocorrOptionsPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(FALSE);
}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


void OfaAutocorrOptionsPage::Reset( const SfxItemSet& rSet )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    const long nFlags = pAutoCorrect->GetFlags();

    aCheckLB.SetUpdateMode(FALSE);
    aCheckLB.Clear();

    aCheckLB.InsertEntry(sInput);
    aCheckLB.InsertEntry(sDoubleCaps);
    aCheckLB.InsertEntry(sStartCap);
    aCheckLB.InsertEntry(sBoldUnderline);
    aCheckLB.InsertEntry(sURL);
    aCheckLB.InsertEntry(sFirst);
    aCheckLB.InsertEntry(sHalf);
    aCheckLB.InsertEntry(sDash);
    aCheckLB.InsertEntry(sNoDblSpaces);

    USHORT nPos = 0;
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & Autocorrect) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & CptlSttWrd) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & CptlSttSntnc) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgWeightUnderl) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & SetINetAttr) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgOrdinalNumber) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgFractionSymbol) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & ChgToEnEmDash) );
    aCheckLB.CheckEntryPos( nPos++, 0 != (nFlags & IngnoreDoubleSpace) );

    aCheckLB.SetUpdateMode(TRUE);
}

/*********************************************************************/
/*                                                                   */
/*  Hilfs-struct fuer dUserDaten der Checklistbox                    */
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
/*  Dialog fuer Prozenteinstellung                                   */
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
            ModalDialog(pParent, ResId(RID_OFADLG_PRCNT_SET, DIALOG_MGR())),
                aOKPB(this,         ResId(BT_OK)),
                aCancelPB(this,     ResId(BT_CANCEL)),
                aPrcntFL(this,      ResId(FL_PRCNT)),
                aPrcntMF(this,  ResId(ED_RIGHT_MARGIN))
            {
                FreeResource();
            }
    MetricField&    GetPrcntFld(){return aPrcntMF;}
};


/*********************************************************************/
/*                                                                   */
/*  veraenderter LBoxString                                          */
/*                                                                   */
/*********************************************************************/

class OfaImpBrwString : public SvLBoxString
{
public:

    OfaImpBrwString( SvLBoxEntry* pEntry, USHORT nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr){}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
                                            SvLBoxEntry* pEntry);
};

/*********************************************************************/
/*                                                                   */
/*********************************************************************/


void __EXPORT OfaImpBrwString::Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
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

        BOOL bFett = TRUE;
        USHORT nPos = 0;
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
/*  TabPage Autoformat anwenden                                      */
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
    REPLACE_1ST,
    REPLACE_HALF,
    REPLACE_DASHES,
    DEL_SPACES_AT_STT_END,
    DEL_SPACES_BETWEEN_LINES,
    IGNORE_DBLSPACE,
    APPLY_NUMBERING,
    INSERT_BORDER,
    CREATE_TABLE,
    REPLACE_STYLES,
    DEL_EMPTY_NODE,
    REPLACE_USER_COLL,
    REPLACE_BULLETS,
    REPLACE_QUOTATION,
    MERGE_SINGLE_LINE_PARA
};

OfaSwAutoFmtOptionsPage::OfaSwAutoFmtOptionsPage( Window* pParent,
                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, ResId(RID_OFAPAGE_AUTOFMT_APPLY, DIALOG_MGR()), rSet),
    aCheckLB            (this,  ResId(CLB_SETTINGS)),
    aEditPB             (this,  ResId(PB_EDIT)),
    aHeader1Expl        (this,  ResId(FT_HEADER1_EXPLANATION)),
    aHeader2Expl        (this,  ResId(FT_HEADER2_EXPLANATION)),
    sHeader1            (ResId( STR_HEADER1     )),
    sHeader2            (ResId( STR_HEADER2     )),
    sDeleteEmptyPara    (ResId( ST_DEL_EMPTY_PARA)),
    sUseReplaceTbl      (ResId( ST_USE_REPLACE  )),
    sCptlSttWord        (ResId( ST_CPTL_STT_WORD)),
    sCptlSttSent        (ResId( ST_CPTL_STT_SENT)),
    sTypo               (ResId( ST_TYPO         )),
    sUserStyle          (ResId( ST_USER_STYLE   )),
    sBullet             (ResId( ST_BULLET       )),
    sBoldUnder          (ResId( ST_BOLD_UNDER   )),
    sNoDblSpaces        (ResId( STR_NO_DBL_SPACES)),
    sFraction           (ResId( ST_FRACTION     )),
    sDetectURL          (ResId( ST_DETECT_URL   )),
    sDash               (ResId( ST_DASH         )),
    sOrdinal            (ResId( ST_ORDINAL      )),
    sRightMargin        (ResId( ST_RIGHT_MARGIN )),
    sNum                (ResId( STR_NUM         )),
    sBorder             (ResId( STR_BORDER      )),
    sTable              (ResId( STR_TABLE       )),
    sReplaceTemplates   (ResId( STR_REPLACE_TEMPLATES)),
    sDelSpaceAtSttEnd   (ResId( STR_DEL_SPACES_AT_STT_END)),
    sDelSpaceBetweenLines(ResId(STR_DEL_SPACES_BETWEEN_LINES)),
    aChkunBmp           (ResId( CHKBUT_UNCHECKED)),
    aChkchBmp           (ResId( CHKBUT_CHECKED  )),
    aChkchhiBmp         (ResId( CHKBUT_HICHECKED)),
    aChkunhiBmp         (ResId( CHKBUT_HIUNCHECKED)),
    aChktriBmp          (ResId( CHKBUT_TRISTATE )),
    aChktrihiBmp        (ResId( CHKBUT_HITRISTATE)),

    nPercent        ( 50 ),
    pCheckButtonData( NULL )

{
    FreeResource();

    //typ. Anfuehrungszeichen einsetzen
    SvtSysLocale aSysLcl;
    const LocaleDataWrapper& rLcl = aSysLcl.GetLocaleData();
    sTypo.SearchAndReplace( String::CreateFromAscii("%1"),
                                        rLcl.getDoubleQuotationMarkStart());
    sTypo.SearchAndReplace( String::CreateFromAscii("%2"),
                                        rLcl.getDoubleQuotationMarkEnd());

    aCheckLB.SetHelpId(HID_OFAPAGE_AUTOFORMAT_CLB);
    aCheckLB.SetWindowBits(WB_HSCROLL| WB_VSCROLL);

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

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

SvLBoxEntry* OfaSwAutoFmtOptionsPage::CreateEntry(String& rTxt, USHORT nCol)
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if ( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData( &aCheckLB );
        aCheckLB.SetCheckButtonData( pCheckButtonData );
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!

    String sEmpty;
    if (nCol == CBCOL_SECOND)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    pEntry->AddItem( new OfaImpBrwString( pEntry, 0, rTxt ) );

    return pEntry;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/


__EXPORT OfaSwAutoFmtOptionsPage::~OfaSwAutoFmtOptionsPage()
{
    delete (ImpUserData*) aCheckLB.GetUserData( REPLACE_BULLETS );
    delete (ImpUserData*) aCheckLB.GetUserData( APPLY_NUMBERING );
    delete (ImpUserData*) aCheckLB.GetUserData( MERGE_SINGLE_LINE_PARA );
    delete pCheckButtonData;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

SfxTabPage* __EXPORT OfaSwAutoFmtOptionsPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new OfaSwAutoFmtOptionsPage(pParent, rAttrSet);
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

BOOL OfaSwAutoFmtOptionsPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    long nFlags = pAutoCorrect->GetFlags();

    BOOL bCheck = aCheckLB.IsChecked(USE_REPLACE_TABLE, CBCOL_FIRST);
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

    pAutoCorrect->SetAutoCorrFlag(IngnoreDoubleSpace,
                        aCheckLB.IsChecked(IGNORE_DBLSPACE, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(DETECT_URL, CBCOL_FIRST);
    bModified |= pOpt->bSetINetAttr != bCheck;
    pOpt->bSetINetAttr = bCheck;
    pAutoCorrect->SetAutoCorrFlag(SetINetAttr,
                        aCheckLB.IsChecked(DETECT_URL, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(REPLACE_1ST, CBCOL_FIRST);
    bModified |= pOpt->bChgOrdinalNumber != bCheck;
    pOpt->bChgOrdinalNumber = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ChgOrdinalNumber,
                        aCheckLB.IsChecked(REPLACE_1ST, CBCOL_SECOND));

    bCheck = aCheckLB.IsChecked(DEL_EMPTY_NODE, CBCOL_FIRST);
    bModified |= pOpt->bDelEmptyNode != bCheck;
    pOpt->bDelEmptyNode = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_QUOTATION, CBCOL_FIRST);
    bModified |= pOpt->bReplaceQuote != bCheck;
    pOpt->bReplaceQuote = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_USER_COLL, CBCOL_FIRST);
    bModified |= pOpt->bChgUserColl != bCheck;
    pOpt->bChgUserColl = bCheck;

    bCheck = aCheckLB.IsChecked(REPLACE_BULLETS, CBCOL_FIRST);
    bModified |= pOpt->bChgEnumNum != bCheck;
    pOpt->bChgEnumNum = bCheck;
    bModified |= aBulletFont != pOpt->aBulletFont;
    pOpt->aBulletFont = aBulletFont;
    bModified |= String(pOpt->cBullet) != sBulletChar;
    pOpt->cBullet = sBulletChar.GetChar(0);

    bModified |= aByInputBulletFont != pOpt->aByInputBulletFont;
    bModified |= String(pOpt->cByInputBullet) != sByInputBulletChar;
    pOpt->aByInputBulletFont = aByInputBulletFont;
    pOpt->cByInputBullet = sByInputBulletChar.GetChar(0);

    bCheck = aCheckLB.IsChecked(MERGE_SINGLE_LINE_PARA, CBCOL_FIRST);
    bModified |= pOpt->bRightMargin != bCheck;
    pOpt->bRightMargin = bCheck;
    bModified |= nPercent != pOpt->nRightMargin;
    pOpt->nRightMargin = (BYTE)nPercent;

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

    bCheck = aCheckLB.IsChecked(REPLACE_HALF, CBCOL_FIRST);
    bModified |= pOpt->bChgFracionSymbol != bCheck;
    pOpt->bChgFracionSymbol = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ChgFractionSymbol,
                        aCheckLB.IsChecked(REPLACE_HALF, CBCOL_SECOND));

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
        SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
        pCfg->SetModified();
        pCfg->Commit();
    }

    return TRUE;
}

/* -----------------23.11.98 16:15-------------------
 *
 * --------------------------------------------------*/
void    OfaSwAutoFmtOptionsPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(FALSE);
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/


void OfaSwAutoFmtOptionsPage::Reset( const SfxItemSet& rSet )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const long nFlags = pAutoCorrect->GetFlags();

    aCheckLB.SetUpdateMode(FALSE);
    aCheckLB.Clear();

    // Die folgenden Eintraege muessen in der selben Reihenfolge, wie im
    // OfaAutoFmtOptions-enum eingefuegt werden!
    aCheckLB.GetModel()->Insert(CreateEntry(sUseReplaceTbl,     CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sCptlSttWord,       CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sCptlSttSent,       CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sBoldUnder,         CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDetectURL,         CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sOrdinal,           CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sFraction,          CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDash,              CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDelSpaceAtSttEnd,  CBCOL_BOTH  ));
    aCheckLB.GetModel()->Insert(CreateEntry(sDelSpaceBetweenLines, CBCOL_BOTH  ));

    aCheckLB.GetModel()->Insert(CreateEntry(sNoDblSpaces,       CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sNum,               CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sBorder,            CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sTable,             CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sReplaceTemplates,  CBCOL_SECOND));
    aCheckLB.GetModel()->Insert(CreateEntry(sDeleteEmptyPara,   CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sUserStyle,         CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sBullet,            CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sTypo,              CBCOL_FIRST ));
    aCheckLB.GetModel()->Insert(CreateEntry(sRightMargin,       CBCOL_FIRST ));

    aCheckLB.CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_FIRST,    pOpt->bAutoCorrect );
    aCheckLB.CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_SECOND,   0 != (nFlags & Autocorrect));
    aCheckLB.CheckEntryPos( CORR_UPPER,         CBCOL_FIRST,    pOpt->bCptlSttWrd );
    aCheckLB.CheckEntryPos( CORR_UPPER,         CBCOL_SECOND,   0 != (nFlags & CptlSttWrd) );
    aCheckLB.CheckEntryPos( BEGIN_UPPER,        CBCOL_FIRST,    pOpt->bCptlSttSntnc );
    aCheckLB.CheckEntryPos( BEGIN_UPPER,        CBCOL_SECOND,   0 != (nFlags & CptlSttSntnc) );
    aCheckLB.CheckEntryPos( BOLD_UNDERLINE,     CBCOL_FIRST,    pOpt->bChgWeightUnderl );
    aCheckLB.CheckEntryPos( BOLD_UNDERLINE,     CBCOL_SECOND,   0 != (nFlags & ChgWeightUnderl) );
    aCheckLB.CheckEntryPos( IGNORE_DBLSPACE,    CBCOL_SECOND,   0 != (nFlags & IngnoreDoubleSpace) );
    aCheckLB.CheckEntryPos( DETECT_URL,         CBCOL_FIRST,    pOpt->bSetINetAttr );
    aCheckLB.CheckEntryPos( DETECT_URL,         CBCOL_SECOND,   0 != (nFlags & SetINetAttr) );
    aCheckLB.CheckEntryPos( REPLACE_1ST,        CBCOL_FIRST,    pOpt->bChgOrdinalNumber );
    aCheckLB.CheckEntryPos( REPLACE_1ST,        CBCOL_SECOND,   0 != (nFlags & ChgOrdinalNumber) );
    aCheckLB.CheckEntryPos( REPLACE_HALF,       CBCOL_FIRST,    pOpt->bChgFracionSymbol );
    aCheckLB.CheckEntryPos( REPLACE_HALF,       CBCOL_SECOND,   0 != (nFlags & ChgFractionSymbol) );
    aCheckLB.CheckEntryPos( REPLACE_DASHES,     CBCOL_FIRST,    pOpt->bChgToEnEmDash );
    aCheckLB.CheckEntryPos( REPLACE_DASHES,     CBCOL_SECOND,   0 != (nFlags & ChgToEnEmDash) );
    aCheckLB.CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_FIRST,    pOpt->bAFmtDelSpacesAtSttEnd );
    aCheckLB.CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_SECOND,   pOpt->bAFmtByInpDelSpacesAtSttEnd );
    aCheckLB.CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_FIRST,    pOpt->bAFmtDelSpacesBetweenLines );
    aCheckLB.CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_SECOND,   pOpt->bAFmtByInpDelSpacesBetweenLines );
    aCheckLB.CheckEntryPos( DEL_EMPTY_NODE,     CBCOL_FIRST,    pOpt->bDelEmptyNode );
    aCheckLB.CheckEntryPos( REPLACE_QUOTATION,  CBCOL_FIRST,    pOpt->bReplaceQuote );
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

    aCheckLB.SetUpdateMode(TRUE);
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(OfaSwAutoFmtOptionsPage, SelectHdl, OfaACorrCheckListBox*, pBox)
{
    aEditPB.Enable(0 != pBox->FirstSelected()->GetUserData());
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(OfaSwAutoFmtOptionsPage, EditHdl, PushButton*, EMPTYARG)
{
    ULONG nSelEntryPos = aCheckLB.GetSelectEntryPos();
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
            *pUserData->pString = pMapDlg->GetChar();
        }
        delete pMapDlg;
    }
    else if( MERGE_SINGLE_LINE_PARA == nSelEntryPos )
    {
        // Dialog fuer Prozenteinstellung
        OfaAutoFmtPrcntSet aDlg(this);
        aDlg.GetPrcntFld().SetValue(nPercent);
        if(RET_OK == aDlg.Execute())
        {
            nPercent = (USHORT)aDlg.GetPrcntFld().GetValue();
            sMargin = ' ';
            sMargin += String::CreateFromInt32( nPercent );
            sMargin += '%';
        }
    }
    aCheckLB.Invalidate();
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

void OfaACorrCheckListBox::SetTabs()
{
    SvxSimpleTable::SetTabs();
    USHORT nAdjust = SV_LBOXTAB_ADJUST_RIGHT|SV_LBOXTAB_ADJUST_LEFT|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_ADJUST_NUMERIC|SV_LBOXTAB_FORCE;

    if( aTabs.Count() > 1 )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(1);
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
    if( aTabs.Count() > 2 )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(2);
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

void OfaACorrCheckListBox::CheckEntryPos(ULONG nPos, USHORT nCol, BOOL bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

BOOL OfaACorrCheckListBox::IsChecked(ULONG nPos, USHORT nCol)
{
    return GetCheckButtonState( GetEntry(nPos), nCol ) == SV_BUTTON_CHECKED;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

void OfaACorrCheckListBox::SetCheckButtonState( SvLBoxEntry* pEntry, USHORT nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found")
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

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

SvButtonState OfaACorrCheckListBox::GetCheckButtonState( SvLBoxEntry* pEntry, USHORT nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found")

    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
    {
        USHORT nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaACorrCheckListBox::HBarClick()
{
    // Sortierung durch diese Ueberladung abgeklemmt
}
/* -----------------------------22.05.2002 11:06------------------------------

 ---------------------------------------------------------------------------*/
void    OfaACorrCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        ULONG nSelPos = GetSelectEntryPos();
        USHORT nCheck = IsChecked(nSelPos, 1) ? 1 : 0;
        if(IsChecked(nSelPos, 0))
            nCheck += 2;
        nCheck--;
        nCheck &= 3;
        CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
        CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
    }
    else
        SvxSimpleTable::KeyInput(rKEvt);
}
/* -----------------19.11.98 15:57-------------------
 *
 * --------------------------------------------------*/
struct DoubleString
{
    String  sShort;
    String  sLong;
    void*   pUserData; // CheckBox -> form. Text Bool -> Selektionstext
};
typedef DoubleString* DoubleStringPtr;
SV_DECL_PTRARR_DEL(DoubleStringArray, DoubleStringPtr, 4, 4);
SV_IMPL_PTRARR(DoubleStringArray, DoubleStringPtr);
/* -----------------19.11.98 16:07-------------------
 *
 * --------------------------------------------------*/
void lcl_ClearTable(DoubleStringTable& rTable)
{
    DoubleStringArrayPtr pArray = rTable.Last();
    while(pArray)
    {
        pArray->DeleteAndDestroy(0, pArray->Count());
        delete pArray;
        pArray = rTable.Prev();
    }
    rTable.Clear();
}

/*-----------------14.10.96 15.57-------------------

--------------------------------------------------*/

OfaAutocorrReplacePage::OfaAutocorrReplacePage( Window* pParent,
                                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, ResId( RID_OFAPAGE_AUTOCORR_REPLACE, DIALOG_MGR() ), rSet),
    aShortFT  (     this, ResId(FT_SHORT          )),
    aShortED  (     this, ResId(ED_SHORT          )),
    aReplaceFT(     this, ResId(FT_REPLACE        )),
    aReplaceED(     this, ResId(ED_REPLACE        )),
    aReplaceTLB(    this, ResId(TLB_REPLACE       )),
    aTextOnlyCB(    this, ResId(CB_TEXT_ONLY      )),
    aNewReplacePB(  this, ResId(PB_NEW_REPLACE    )),
    aDeleteReplacePB(this,ResId(PB_DELETE_REPLACE )),
    sModify(ResId(STR_MODIFY)),
    sNew(aNewReplacePB.GetText()),
    eLang(eLastDialogLanguage),
    bHasSelectionText(FALSE),
    pFormatText(0),
    bFirstSelect(TRUE),
    bReplaceEditChanged(FALSE),
    bSWriter(TRUE),
    pAutoCorrect(SvxAutoCorrCfg::Get()->GetAutoCorrect())
{
    FreeResource();
    SfxModule *pMod = *(SfxModule**)GetAppData(SHL_WRITER);
    bSWriter = pMod == SFX_APP()->GetActiveModule();

    ::com::sun::star::lang::Locale aLcl( SvxCreateLocale(eLastDialogLanguage ));
    pCompareClass = new CollatorWrapper( GetProcessFact() );
    pCompareCaseClass = new CollatorWrapper( GetProcessFact() );
    pCompareClass->loadDefaultCollator( aLcl, ::com::sun::star::i18n::
                            CollatorOptions::CollatorOptions_IGNORE_CASE );
    pCompareCaseClass->loadDefaultCollator( aLcl, 0 );
    pCharClass = new CharClass( aLcl );

    static long nTabs[] = { 2 /* Tab-Count */, 1, 61 };
    aReplaceTLB.SetTabs( &nTabs[0], MAP_APPFONT );

    aReplaceTLB.SetWindowBits(WB_HSCROLL|WB_CLIPCHILDREN);
    aReplaceTLB.SetSelectHdl(LINK(this, OfaAutocorrReplacePage, SelectHdl));
    aNewReplacePB.SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl));
    aDeleteReplacePB.SetClickHdl(LINK(this, OfaAutocorrReplacePage, NewDelHdl));
    aShortED.SetModifyHdl(LINK(this, OfaAutocorrReplacePage, ModifyHdl));
    aReplaceED.SetModifyHdl(LINK(this, OfaAutocorrReplacePage, ModifyHdl));
    aShortED.SetActionHdl(LINK(this, OfaAutocorrReplacePage, NewDelHdl));
    aReplaceED.SetActionHdl(LINK(this, OfaAutocorrReplacePage, NewDelHdl));

    aReplaceED.SetSpaces(TRUE);
    aShortED.SetSpaces(TRUE);
    aShortED.SetMaxTextLen(30);
}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


OfaAutocorrReplacePage::~OfaAutocorrReplacePage()
{
    delete pFormatText;
    lcl_ClearTable(aDoubleStringTable);
    delete pCompareClass;
    delete pCompareCaseClass;
    delete pCharClass;
}
/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

SfxTabPage* OfaAutocorrReplacePage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrReplacePage(pParent, rSet);
}
/* -----------------20.11.98 13:26-------------------
 *
 * --------------------------------------------------*/
void    OfaAutocorrReplacePage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(TRUE);
}
/* -----------------20.11.98 13:26-------------------
 *
 * --------------------------------------------------*/
int OfaAutocorrReplacePage::DeactivatePage( SfxItemSet* pSet )
{
    return LEAVE_PAGE;
}
/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

BOOL OfaAutocorrReplacePage::FillItemSet( SfxItemSet& rSet )
{
    DoubleStringArrayPtr pArray = aDoubleStringTable.Last();
    while(pArray)
    {
        LanguageType eCurLang = (LanguageType)aDoubleStringTable.GetCurKey();
        if(eCurLang != eLang) // die aktuelle Sprache wird weiter hinten behandelt
        {
            SvxAutocorrWordList* pWordList = pAutoCorrect->LoadAutocorrWordList(eCurLang);
            USHORT nWordListCount = pWordList->Count();
            USHORT nEntryCount = pArray->Count();
            USHORT nPos = nEntryCount;
            USHORT nLastPos = nPos;
            // 1. Durchlauf: Eintraege loeschen oder veraendern:

            USHORT i;
            for( i = nWordListCount; i; i-- )
            {
                SvxAutocorrWordPtr pWordPtr = pWordList->GetObject(i - 1);
                String sEntry(pWordPtr->GetShort());
                // formatierter Text steht nur im Writer
                BOOL bFound = !bSWriter && !pWordPtr->IsTextOnly();
                while(!bFound && nPos)
                {
                    DoubleString* pDouble = pArray->GetObject( nPos - 1);

                    if( 0 == pCompareClass->compareString(
                                                    sEntry, pDouble->sShort ))
                    {
                        nLastPos = nPos - 1;
                        bFound = TRUE;
                        if( !(pWordPtr->IsTextOnly() == (0 == pDouble->pUserData)
                            && 0 == pCompareCaseClass->compareString(
                                pWordPtr->GetLong(), pDouble->sLong ) ) )
                        {
                            pAutoCorrect->PutText(sEntry, pDouble->sLong, eCurLang);
                        }
                        pArray->DeleteAndDestroy(i - 1, 1);
                        break;
                    }
                    nPos--;
                }
                nPos = nLastPos;
                if(!bFound)
                {
                    pAutoCorrect->DeleteText(sEntry, eCurLang);
                }
            }
            nEntryCount = pArray->Count();
            for(i = 0; i < nEntryCount; i++ )
            {
                //jetzt sollte es nur noch neue Eintraege geben
                DoubleString* pDouble = pArray->GetObject( i );
                if(pDouble->pUserData == &bHasSelectionText)
                    pAutoCorrect->PutText( pDouble->sShort,
                                *SfxObjectShell::Current(), eCurLang );
                else
                {
                    pAutoCorrect->PutText( pDouble->sShort, pDouble->sLong,
                                                            eCurLang);
                }
            }
        }
        pArray->DeleteAndDestroy(0, pArray->Count());
        delete pArray;
        pArray = aDoubleStringTable.Prev();
    }
    aDoubleStringTable.Clear();
    // jetzt noch die aktuelle Selektion
    SvxAutocorrWordList* pWordList = pAutoCorrect->LoadAutocorrWordList(eLang);
    USHORT nWordListCount = pWordList->Count();
    USHORT nListBoxCount = (USHORT)aReplaceTLB.GetEntryCount();

    aReplaceTLB.SetUpdateMode(FALSE);
    USHORT nListBoxPos = nListBoxCount;
    USHORT nLastListBoxPos = nListBoxPos;
    // 1. Durchlauf: Eintraege loeschen oder veraendern:

    USHORT i;
    for( i = nWordListCount; i; i-- )
    {
        SvxAutocorrWordPtr pWordPtr = pWordList->GetObject(i- 1);
        String sEntry(pWordPtr->GetShort());
        // formatierter Text steht nur im Writer
        BOOL bFound = !bSWriter && !pWordPtr->IsTextOnly();
        while(!bFound && nListBoxPos)
        {
            SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( nListBoxPos - 1);
            if( 0 == pCompareClass->compareString( sEntry,
                        aReplaceTLB.GetEntryText(pEntry, 0)))
            {
                nLastListBoxPos = nListBoxPos - 1;
                bFound = TRUE;
                String sLong = aReplaceTLB.GetEntryText(pEntry, 1);
                if( !(pWordPtr->IsTextOnly() == (0 == pEntry->GetUserData())
                    && 0 == pCompareCaseClass->compareString(
                        pWordPtr->GetLong(), sLong )))
                {
                    pAutoCorrect->PutText(sEntry, sLong, eLang);
                }
                aReplaceTLB.GetModel()->Remove(pEntry);
                break;

            }
            nListBoxPos --;
        }
        nListBoxPos = nLastListBoxPos;
        if(!bFound)
        {
            pAutoCorrect->DeleteText(sEntry, eLang);
        }

    }
    nListBoxCount = (USHORT)aReplaceTLB.GetEntryCount();
    for(i = 0; i < nListBoxCount; i++ )
    {
        //jetzt sollte es nur noch neue Eintraege geben
        SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( i );
        String sShort = aReplaceTLB.GetEntryText(pEntry, 0);
        if(pEntry->GetUserData() == &bHasSelectionText)
            pAutoCorrect->PutText(sShort, *SfxObjectShell::Current(), eLang);
        else
        {
            String sLong = aReplaceTLB.GetEntryText(pEntry, 1);
            pAutoCorrect->PutText(sShort, sLong, eLang);
        }
    }

    return FALSE;
}

/* -----------------19.11.98 13:16-------------------
 *
 * --------------------------------------------------*/
void OfaAutocorrReplacePage::RefillReplaceBox(BOOL bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
        lcl_ClearTable(aDoubleStringTable);
    else
    {
        DoubleStringArray* pArray = 0;
        if(aDoubleStringTable.IsKeyValid(eOldLanguage))
        {
            pArray = aDoubleStringTable.Seek(ULONG(eOldLanguage));
            pArray->DeleteAndDestroy(0, pArray->Count());
        }
        else
        {
            pArray = new DoubleStringArray;
            aDoubleStringTable.Insert(ULONG(eOldLanguage), pArray);
        }

        USHORT nListBoxCount = (USHORT)aReplaceTLB.GetEntryCount();
        USHORT i;
        for(i = 0; i < nListBoxCount; i++)
        {
            DoubleString* pDouble = new DoubleString();
            SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( i );
            pDouble->sShort = aReplaceTLB.GetEntryText(pEntry, 0);
            pDouble->sLong = aReplaceTLB.GetEntryText(pEntry, 1);
            pDouble->pUserData = pEntry->GetUserData();
            pArray->Insert(pDouble, i);
        }
    }

    aReplaceTLB.Clear();
    if(!bSWriter)
    {
        if(pFormatText)
            pFormatText->DeleteAndDestroy(0, pFormatText->Count());
        else
            pFormatText = new SvStringsISortDtor();
    }

    if(aDoubleStringTable.IsKeyValid(eLang))
    {
        DoubleStringArray* pArray = aDoubleStringTable.Seek(ULONG(eNewLanguage));
        for(USHORT i = 0; i < pArray->Count(); i++)
        {
            DoubleString* pDouble = pArray->GetObject(i);
            BOOL bTextOnly = 0 == pDouble->pUserData;
            // formatierter Text wird nur im Writer angeboten
            if(bSWriter || bTextOnly)
            {
                String sEntry(pDouble->sShort);
                sEntry += '\t';
                sEntry += pDouble->sLong;
                SvLBoxEntry* pEntry = aReplaceTLB.InsertEntry(sEntry);
                aTextOnlyCB.Check(bTextOnly);
                if(!bTextOnly)
                    pEntry->SetUserData(pDouble->pUserData); // Das heisst: mit Formatinfo oder sogar mit Selektionstext
            }
            else
            {
                pFormatText->Insert(new String(pDouble->sShort));
            }
        }
    }
    else
    {
        SvxAutocorrWordList* pWordList = pAutoCorrect->LoadAutocorrWordList(eLang);

        for(USHORT i = 0; i < pWordList->Count(); i++)
        {
            SvxAutocorrWordPtr pWordPtr = pWordList->GetObject(i);
            BOOL bTextOnly = pWordPtr->IsTextOnly();
            // formatierter Text wird nur im Writer angeboten
            if(bSWriter || bTextOnly)
            {
                String sEntry(pWordPtr->GetShort());
                sEntry += '\t';
                sEntry += pWordPtr->GetLong();
                SvLBoxEntry* pEntry = aReplaceTLB.InsertEntry(sEntry);
                aTextOnlyCB.Check(pWordPtr->IsTextOnly());
                if(!bTextOnly)
                    pEntry->SetUserData(&aTextOnlyCB); // Das heisst: mit Formatinfo
            }
            else
            {
                pFormatText->Insert(new String(pWordPtr->GetShort()));
            }
        }
        aNewReplacePB.Enable(FALSE);
        aDeleteReplacePB.Enable(FALSE);

    }

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if( pViewShell && pViewShell->HasSelection( TRUE ) )
    {
        bHasSelectionText = TRUE;
        const String sSelection( pViewShell->GetSelectionText() );
        aReplaceED.SetText( sSelection );
        aTextOnlyCB.Check( !bSWriter );
        aTextOnlyCB.Enable( bSWriter && sSelection.Len() );
    }
    else
        aTextOnlyCB.Enable( FALSE );
}
/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

void OfaAutocorrReplacePage::Reset( const SfxItemSet& rSet )
{
    RefillReplaceBox(TRUE, eLang, eLang);
    aShortED.GrabFocus();
}

/* -----------------23.11.98 10:33-------------------
 *
 * --------------------------------------------------*/
void OfaAutocorrReplacePage::SetLanguage(LanguageType eSet)
{
    //save old settings an refill
    if(eSet != eLang)
    {
        RefillReplaceBox(FALSE, eLang, eSet);
        eLastDialogLanguage = eSet;
        delete pCompareClass;
        delete pCompareCaseClass;
        delete pCharClass;

        ::com::sun::star::lang::Locale aLcl( SvxCreateLocale(eLastDialogLanguage ));
        pCompareClass = new CollatorWrapper( GetProcessFact() );
        pCompareCaseClass = new CollatorWrapper( GetProcessFact() );
        pCompareClass->loadDefaultCollator( aLcl, ::com::sun::star::i18n::
                                CollatorOptions::CollatorOptions_IGNORE_CASE );
        pCompareCaseClass->loadDefaultCollator( aLcl, 0 );
        pCharClass = new CharClass( aLcl );
        ModifyHdl(&aShortED);
    }
}
/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


IMPL_LINK(OfaAutocorrReplacePage, SelectHdl, SvTabListBox*, pBox)
{
    if(!bFirstSelect || !bHasSelectionText)
    {
        SvLBoxEntry* pEntry = pBox->FirstSelected();
        String sTmpShort(pBox->GetEntryText(pEntry, 0));
        // wird der Text ueber den ModifyHdl gesetzt, dann steht der Cursor sonst immer am Wortanfang,
        // obwohl man gerade hier editiert
        BOOL bSameContent = 0 == pCompareClass->compareString(
                sTmpShort, aShortED.GetText() );
        Selection aSel = aShortED.GetSelection();
        if(aShortED.GetText() != sTmpShort)
        {
            aShortED.SetText(sTmpShort);
            //war es nur eine andere Schreibweise, dann muss die Selektion auch wieder gesetzt werden
            if(bSameContent)
                aShortED.SetSelection(aSel);
        }
        aReplaceED.SetText(pBox->GetEntryText(pEntry, 1));
        // mit UserData gibt es eine Formatinfo
        aTextOnlyCB.Check(0 == pEntry->GetUserData());
    }
    else
        bFirstSelect = FALSE;

    aNewReplacePB.Enable(FALSE);
    aDeleteReplacePB.Enable();
    return 0;
};

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/


IMPL_LINK(OfaAutocorrReplacePage, NewDelHdl, PushButton*, pBtn)
{
    SvLBoxEntry* pEntry = aReplaceTLB.FirstSelected();
    if(pBtn == &aDeleteReplacePB)
    {
        DBG_ASSERT(pEntry, "keine Eintrag selektiert")
        if(pEntry)
        {
            aReplaceTLB.GetModel()->Remove(pEntry);
            ModifyHdl(&aShortED);
            return 0;
        }
    }
    if(pBtn == &aNewReplacePB || aNewReplacePB.IsEnabled())
    {
        SvLBoxEntry* pEntry = aReplaceTLB.FirstSelected();
        String sEntry(aShortED.GetText());
        if(sEntry.Len() && ( aReplaceED.GetText().Len() ||
                ( bHasSelectionText && bSWriter ) ))
        {
            aReplaceTLB.SetUpdateMode(FALSE);
            USHORT nPos = USHRT_MAX;
            sEntry += '\t';
            sEntry += aReplaceED.GetText();
            if(pEntry)
            {
                nPos = (USHORT)aReplaceTLB.GetModel()->GetAbsPos(pEntry);
                aReplaceTLB.GetModel()->Remove(pEntry);
            }
            else
            {
                USHORT j;
                for( j = 0; j < aReplaceTLB.GetEntryCount(); j++ )
                {
                    SvLBoxEntry* pEntry = aReplaceTLB.GetEntry(j);
                    if( 0 >=  pCompareClass->compareString(sEntry,
                                    aReplaceTLB.GetEntryText(pEntry, 0) ) )
                        break;
                }
                nPos = j;
            }
            SvLBoxEntry* pInsEntry =
                aReplaceTLB.InsertEntry(sEntry,
                                nPos == USHRT_MAX ? LIST_APPEND : (ULONG)nPos);
            if( !bReplaceEditChanged && !aTextOnlyCB.IsChecked())
                pInsEntry->SetUserData(&bHasSelectionText); // neuer formatierter Text

            aReplaceTLB.MakeVisible( pInsEntry );
            aReplaceTLB.SetUpdateMode(TRUE);
            // falls der Request aus dem ReplaceEdit kam, dann Focus in das ShortEdit setzen
            if(aReplaceED.HasFocus())
                aShortED.GrabFocus();

        }
    }
    else
    {
        // das kann nur ein Enter in einem der beiden Edit-Felder sein und das
        // bedeutet EndDialog() - muss im KeyInput ausgewertet werden
        return 0;
    }
    ModifyHdl(&aShortED);
    return 1;
}

/*-----------------17.10.96 07.49-------------------

--------------------------------------------------*/
IMPL_LINK(OfaAutocorrReplacePage, ModifyHdl, Edit*, pEdt)
{
    SvLBoxEntry* pFirstSel = aReplaceTLB.FirstSelected();
    BOOL bShort = pEdt == &aShortED;
    const String rEntry = pEdt->GetText();
    const String rRepString = aReplaceED.GetText();
    String aWordStr( pCharClass->lower( rEntry ));

    if(bShort)
    {
        if(rEntry.Len())
        {
            BOOL bFound = FALSE;
            BOOL bTmpSelEntry=FALSE;

            for(USHORT i = 0; i < aReplaceTLB.GetEntryCount(); i++)
            {
                SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( i );
                String aTestStr=aReplaceTLB.GetEntryText(pEntry, 0);
                if( 0 == pCompareClass->compareString(rEntry, aTestStr ))
                {
                    if(rRepString.Len())
                        bFirstSelect = TRUE;
                    aReplaceTLB.SetCurEntry(pEntry);
                    pFirstSel = pEntry;
                    aNewReplacePB.SetText(sModify);
                    bFound= TRUE;
                    break;
                }
                else
                {
                    pCharClass->toLower( aTestStr );
                    if(aTestStr.Search(aWordStr)==0 && !bTmpSelEntry)
                    {
                        aReplaceTLB.MakeVisible(pEntry);
                        bTmpSelEntry=TRUE;
                    }
                }
            }
            if(!bFound)
            {
                aReplaceTLB.SelectAll(FALSE);
                pFirstSel = 0;
                aNewReplacePB.SetText(sNew);
                if(bReplaceEditChanged)
                    aTextOnlyCB.Enable(FALSE);
            }
            aDeleteReplacePB.Enable(bFound);
        }
        else if(aReplaceTLB.GetEntryCount()>0)
        {
            SvLBoxEntry*  pEntry = aReplaceTLB.GetEntry( 0 );
            aReplaceTLB.MakeVisible(pEntry);
        }

    }
    else if(!bShort)
    {
        bReplaceEditChanged = TRUE;
        if(pFirstSel)
        {
            aNewReplacePB.SetText(sModify);
        }
    }

    const String& rShortTxt = aShortED.GetText();
    BOOL bEnableNew = rShortTxt.Len() &&
                        ( rRepString.Len() ||
                                ( bHasSelectionText && bSWriter )) &&
                        ( !pFirstSel || rRepString !=
                                aReplaceTLB.GetEntryText( pFirstSel, 1 ) );
    if(bEnableNew && pFormatText)
    {
        for(USHORT i = 0; i < pFormatText->Count(); i++)
            if(*pFormatText->GetObject(i) == rShortTxt)
            {
                bEnableNew = FALSE;
                break;
            }
    }
    aNewReplacePB.Enable(bEnableNew);

    return 0;
}

/* -----------------20.11.98 13:48-------------------
 *
 * --------------------------------------------------*/

struct StringsArrays
{

    SvStringsDtor aAbbrevStrings;
    SvStringsDtor aDoubleCapsStrings;

    StringsArrays() :
        aAbbrevStrings(5,5), aDoubleCapsStrings(5,5) {}
};
typedef StringsArrays* StringsArraysPtr;
/* -----------------19.11.98 16:07-------------------
 *
 * --------------------------------------------------*/
BOOL lcl_FindInArray(SvStringsDtor& rStrings, const String& rString)
{
    for(USHORT i = 0; i < rStrings.Count(); i++)
        if(rString == *rStrings.GetObject(i))
            return TRUE;
    return FALSE;
}

void lcl_ClearTable(StringsTable& rTable)
{
    StringsArraysPtr pArrays = rTable.Last();
    while(pArrays)
    {
        delete pArrays;
        pArrays = rTable.Prev();
    }
    rTable.Clear();
}

/*-----------------14.10.96 15.57-------------------

--------------------------------------------------*/

OfaAutocorrExceptPage::OfaAutocorrExceptPage( Window* pParent,
                                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, ResId( RID_OFAPAGE_AUTOCORR_EXCEPT, DIALOG_MGR() ), rSet),
    aAbbrevFL       (this, ResId(FL_ABBREV       )),
    aAbbrevED       (this, ResId(ED_ABBREV       )),
    aAbbrevLB       (this, ResId(LB_ABBREV       )),
    aNewAbbrevPB    (this, ResId(PB_NEWABBREV    )),
    aDelAbbrevPB    (this, ResId(PB_DELABBREV    )),
    aAutoAbbrevCB   (this, ResId(CB_AUTOABBREV   )),
    aDoubleCapsFL   (this, ResId(FL_DOUBLECAPS   )),
    aDoubleCapsED   (this, ResId(ED_DOUBLE_CAPS  )),
    aDoubleCapsLB   (this, ResId(LB_DOUBLE_CAPS  )),
    aNewDoublePB    (this, ResId(PB_NEWDOUBLECAPS)),
    aDelDoublePB    (this, ResId(PB_DELDOUBLECAPS)),
    aAutoCapsCB     (this, ResId(CB_AUTOCAPS     )),
    eLang(eLastDialogLanguage),
    pAutoCorrect    (SvxAutoCorrCfg::Get()->GetAutoCorrect())
{
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

}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

OfaAutocorrExceptPage::~OfaAutocorrExceptPage()
{
    lcl_ClearTable(aStringsTable);
    delete pCompareClass;
}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

SfxTabPage* OfaAutocorrExceptPage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrExceptPage(pParent, rSet);
}
/* -----------------20.11.98 13:26-------------------
 *
 * --------------------------------------------------*/
void    OfaAutocorrExceptPage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(TRUE);
}
/* -----------------20.11.98 13:26-------------------
 *
 * --------------------------------------------------*/
int     OfaAutocorrExceptPage::DeactivatePage( SfxItemSet* pSet )
{
    return LEAVE_PAGE;
}
/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

BOOL OfaAutocorrExceptPage::FillItemSet( SfxItemSet& rSet )
{
    StringsArraysPtr pArrays = aStringsTable.Last();
    while(pArrays)
    {
        LanguageType eCurLang = (LanguageType)aStringsTable.GetCurKey();
        if(eCurLang != eLang) // die aktuelle Sprache wird weiter hinten behandelt
        {
            SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eCurLang);

            if(pWrdList)
            {
                USHORT nCount = pWrdList->Count();
                USHORT i;
                for( i = nCount; i; )
                {
                    String* pString = pWrdList->GetObject( --i );
                    //Eintrag finden u. gfs entfernen
                    if( !lcl_FindInArray(pArrays->aDoubleCapsStrings, *pString))
                      pWrdList->DeleteAndDestroy( i );
                }
                nCount = pArrays->aDoubleCapsStrings.Count();
                for( i = 0; i < nCount; ++i )
                {
                    String* pEntry = new String( *pArrays->aDoubleCapsStrings.GetObject( i ) );
                    if( !pWrdList->Insert( pEntry ))
                        delete pEntry;
                }
                pAutoCorrect->SaveWrdSttExceptList(eCurLang);
            }

            SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eCurLang);

            if(pCplList)
            {
                USHORT nCount = pCplList->Count();
                USHORT i;
                for( i = nCount; i; )
                {
                    String* pString = pCplList->GetObject( --i );
                    if( !lcl_FindInArray(pArrays->aAbbrevStrings, *pString))
                        pCplList->DeleteAndDestroy( i );
                }
                nCount = pArrays->aAbbrevStrings.Count();
                for( i = 0; i < nCount; ++i )
                {
                    String* pEntry = new String( *pArrays->aAbbrevStrings.GetObject(i) );
                    if( !pCplList->Insert( pEntry ))
                        delete pEntry;
                }
                pAutoCorrect->SaveCplSttExceptList(eCurLang);
            }
        }
        pArrays = aStringsTable.Prev();
    }
    aStringsTable.Clear();

    SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eLang);

    if(pWrdList)
    {
        USHORT nCount = pWrdList->Count();
        USHORT i;
        for( i = nCount; i; )
        {
            String* pString = pWrdList->GetObject( --i );
            if( USHRT_MAX == aDoubleCapsLB.GetEntryPos(*pString) )
                pWrdList->DeleteAndDestroy( i );
        }
        nCount = aDoubleCapsLB.GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            String* pEntry = new String( aDoubleCapsLB.GetEntry( i ) );
            if( !pWrdList->Insert( pEntry ))
                delete pEntry;
        }
        pAutoCorrect->SaveWrdSttExceptList(eLang);
    }

    SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eLang);

    if(pCplList)
    {
        USHORT nCount = pCplList->Count();
        USHORT i;
        for( i = nCount; i; )
        {
            String* pString = pCplList->GetObject( --i );
            if( USHRT_MAX == aAbbrevLB.GetEntryPos(*pString) )
                pCplList->DeleteAndDestroy( i );
        }
        nCount = aAbbrevLB.GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            String* pEntry = new String( aAbbrevLB.GetEntry( i ) );
            if( !pCplList->Insert( pEntry ))
                delete pEntry;
        }
        pAutoCorrect->SaveCplSttExceptList(eLang);
    }
    if(aAutoAbbrevCB.IsChecked() != aAutoAbbrevCB.GetSavedValue())
        pAutoCorrect->SetAutoCorrFlag( SaveWordCplSttLst, aAutoAbbrevCB.IsChecked());
    if(aAutoCapsCB.IsChecked() != aAutoCapsCB.GetSavedValue())
        pAutoCorrect->SetAutoCorrFlag( SaveWordWrdSttLst, aAutoCapsCB.IsChecked());
    return FALSE;
}

/* -----------------23.11.98 10:33-------------------
 *
 * --------------------------------------------------*/
void OfaAutocorrExceptPage::SetLanguage(LanguageType eSet)
{
    if(eLang != eSet)
    {
        //alte Einstellungen speichern und neu fuellen
        RefillReplaceBoxes(FALSE, eLang, eSet);
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
/* -----------------20.11.98 14:06-------------------
 *
 * --------------------------------------------------*/
void OfaAutocorrExceptPage::RefillReplaceBoxes(BOOL bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
        lcl_ClearTable(aStringsTable);
    else
    {
        StringsArraysPtr pArrays = 0;
        if(aStringsTable.IsKeyValid(eOldLanguage))
        {
            pArrays = aStringsTable.Seek(ULONG(eOldLanguage));
            pArrays->aAbbrevStrings.DeleteAndDestroy(
                                    0, pArrays->aAbbrevStrings.Count());
            pArrays->aDoubleCapsStrings.DeleteAndDestroy(
                                    0, pArrays->aDoubleCapsStrings.Count());
        }
        else
        {
            pArrays = new StringsArrays;
            aStringsTable.Insert(ULONG(eOldLanguage), pArrays);
        }

        USHORT i;
        for(i = 0; i < aAbbrevLB.GetEntryCount(); i++)
        {
            pArrays->aAbbrevStrings.Insert(
                new String(aAbbrevLB.GetEntry(i)), i);

        }
        for(i = 0; i < aDoubleCapsLB.GetEntryCount(); i++)
        {
            pArrays->aDoubleCapsStrings.Insert(
                new String(aDoubleCapsLB.GetEntry(i)), i);
        }
    }
    aDoubleCapsLB.Clear();
    aAbbrevLB.Clear();
    String sTemp;
    aAbbrevED.SetText(sTemp);
    aDoubleCapsED.SetText(sTemp);

    if(aStringsTable.IsKeyValid(eLang))
    {
        StringsArraysPtr pArrays = aStringsTable.Seek(ULONG(eLang));
        USHORT i;
        for(i = 0; i < pArrays->aAbbrevStrings.Count(); i++ )
        {
            aAbbrevLB.InsertEntry(*pArrays->aAbbrevStrings.GetObject(i));
        }
        for( i = 0; i < pArrays->aDoubleCapsStrings.Count(); i++ )
        {
            aDoubleCapsLB.InsertEntry(*pArrays->aDoubleCapsStrings.GetObject(i));
        }
    }
    else
    {
        const SvStringsISortDtor* pCplList = pAutoCorrect->GetCplSttExceptList(eLang);
        const SvStringsISortDtor* pWrdList = pAutoCorrect->GetWrdSttExceptList(eLang);
        USHORT i;
        for( i = 0; i < pCplList->Count(); i++ )
        {
            aAbbrevLB.InsertEntry(*pCplList->GetObject(i));
        }
        for( i = 0; i < pWrdList->Count(); i++ )
        {
            aDoubleCapsLB.InsertEntry(*pWrdList->GetObject(i));
        }
    }
}

/*-----------------14.10.96 15.58-------------------

--------------------------------------------------*/

void OfaAutocorrExceptPage::Reset( const SfxItemSet& rSet )
{
    RefillReplaceBoxes(TRUE, eLang, eLang);
    aAutoAbbrevCB.  Check(  pAutoCorrect->IsAutoCorrFlag( SaveWordCplSttLst ));
    aAutoCapsCB.    Check(  pAutoCorrect->IsAutoCorrFlag( SaveWordWrdSttLst ));
    aAutoAbbrevCB.SaveValue();
    aAutoCapsCB.SaveValue();
}

/*-----------------16.10.96 12.27-------------------

--------------------------------------------------*/


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

/*-----------------16.10.96 12.57-------------------

--------------------------------------------------*/

IMPL_LINK(OfaAutocorrExceptPage, SelectHdl, ListBox*, pBox)
{
    if(pBox == &aAbbrevLB)
    {
        aAbbrevED.SetText(pBox->GetSelectEntry());
        aNewAbbrevPB.Enable(FALSE);
        aDelAbbrevPB.Enable();
    }
    else
    {
        aDoubleCapsED.SetText(pBox->GetSelectEntry());
        aNewDoublePB.Enable(FALSE);
        aDelDoublePB.Enable();
    }
    return 0;
}

/*-----------------16.10.96 13.02-------------------

--------------------------------------------------*/

IMPL_LINK(OfaAutocorrExceptPage, ModifyHdl, Edit*, pEdt)
{
//  BOOL bSame = pEdt->GetText() == ->GetSelectEntry();
    const String& sEntry = pEdt->GetText();
    BOOL bEntryLen = 0!= sEntry.Len();
    if(pEdt == &aAbbrevED)
    {
        BOOL bSame = lcl_FindEntry(aAbbrevLB, sEntry, *pCompareClass);
        if(bSame && sEntry != aAbbrevLB.GetSelectEntry())
            pEdt->SetText(aAbbrevLB.GetSelectEntry());
        aNewAbbrevPB.Enable(!bSame && bEntryLen);
        aDelAbbrevPB.Enable(bSame && bEntryLen);
    }
    else
    {
        BOOL bSame = lcl_FindEntry(aDoubleCapsLB, sEntry, *pCompareClass);
        if(bSame && sEntry != aDoubleCapsLB.GetSelectEntry())
            pEdt->SetText(aDoubleCapsLB.GetSelectEntry());
        aNewDoublePB.Enable(!bSame && bEntryLen);
        aDelDoublePB.Enable(bSame && bEntryLen);
    }
    return 0;
}

/*-----------------16.10.96 15.03-------------------

--------------------------------------------------*/

void AutoCorrEdit::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode aKeyCode = rKEvt.GetKeyCode();
    const USHORT nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN )
    {
        //wird bei Enter nichts getan, dann doch die Basisklasse rufen
        // um den Dialog zu schliessen
        if(!nModifier && !aActionLink.Call(this))
                 Edit::KeyInput(rKEvt);
    }
    else if(bSpaces || aKeyCode.GetCode() != KEY_SPACE)
        Edit::KeyInput(rKEvt);
}

/*-----------------03.07.97 13:17-------------------

--------------------------------------------------*/

OfaQuoteTabPage::OfaQuoteTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, ResId( RID_OFAPAGE_AUTOCORR_QUOTE, DIALOG_MGR() ), rSet),
    aSingleFL           (this, ResId(FL_SINGLE       )),
    aDoubleFL           (this, ResId(FL_DOUBLE       )),
    aSingleTypoCB       (this, ResId(CB_SGL_TYPO     )),
    aSglStartExFT       (this, ResId(FT_SGSTEX       )),
    aSglEndExFT         (this, ResId(FT_SGENEX       )),
    aDblStartExFT       (this, ResId(FT_DBSTEX       )),
    aDblEndExFT         (this, ResId(FT_DBECEX       )),
    aTypoCB             (this, ResId(CB_TYPO         )),
    aStartQuoteFT       (this, ResId(FT_STARTQUOTE   )),
    aStartQuotePB       (this, ResId(PB_STARTQUOTE   )),
    aEndQuoteFT         (this, ResId(FT_ENDQUOTE     )),
    aEndQuotePB         (this, ResId(PB_ENDQUOTE     )),
    aDblStandardPB      (this, ResId(PB_DBL_STD      )),
    aSglStartQuoteFT    (this, ResId(FT_SGL_STARTQUOTE )),
    aSglStartQuotePB    (this, ResId(PB_SGL_STARTQUOTE )),
    aSglEndQuoteFT      (this, ResId(FT_SGL_ENDQUOTE   )),
    aSglEndQuotePB      (this, ResId(PB_SGL_ENDQUOTE   )),
    aSglStandardPB      (this, ResId(PB_SGL_STD      )),
    sStartQuoteDlg  (ResId(STR_CHANGE_START)),
    sEndQuoteDlg    (ResId(STR_CHANGE_END)),
    sStandard(ResId(ST_STANDARD))
{
    FreeResource();

    aStartQuotePB.SetClickHdl(LINK(this,    OfaQuoteTabPage, QuoteHdl));
    aEndQuotePB.SetClickHdl(LINK(this,      OfaQuoteTabPage, QuoteHdl));
    aSglStartQuotePB.SetClickHdl(LINK(this, OfaQuoteTabPage, QuoteHdl));
    aSglEndQuotePB.SetClickHdl(LINK(this,   OfaQuoteTabPage, QuoteHdl));
    aDblStandardPB.SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));
    aSglStandardPB.SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));

}
/*-----------------03.07.97 13:17-------------------

--------------------------------------------------*/
OfaQuoteTabPage::~OfaQuoteTabPage()
{
}
/*-----------------03.07.97 13:17-------------------

--------------------------------------------------*/
SfxTabPage* OfaQuoteTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new OfaQuoteTabPage(pParent, rAttrSet);
}
/*-----------------03.07.97 13:18-------------------

--------------------------------------------------*/
BOOL OfaQuoteTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();

    long nFlags = pAutoCorrect->GetFlags();
    pAutoCorrect->SetAutoCorrFlag(ChgQuotes,        aTypoCB.IsChecked());
    pAutoCorrect->SetAutoCorrFlag(ChgSglQuotes,     aSingleTypoCB.IsChecked());
    BOOL bReturn = nFlags != pAutoCorrect->GetFlags();
    if(cStartQuote != pAutoCorrect->GetStartDoubleQuote())
    {
        bReturn = TRUE;
        pAutoCorrect->SetStartDoubleQuote(cStartQuote);
    }
    if(cEndQuote != pAutoCorrect->GetEndDoubleQuote())
    {
        bReturn = TRUE;
        pAutoCorrect->SetEndDoubleQuote(cEndQuote);
    }
    if(cSglStartQuote != pAutoCorrect->GetStartSingleQuote())
    {
        bReturn = TRUE;
        pAutoCorrect->SetStartSingleQuote(cSglStartQuote);
    }
    if(cSglEndQuote != pAutoCorrect->GetEndSingleQuote())
    {
        bReturn = TRUE;
        pAutoCorrect->SetEndSingleQuote(cSglEndQuote);
    }

    if(bReturn )
    {
        SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
        pCfg->SetModified();
        pCfg->Commit();
    }
    return bReturn;
}
/* -----------------23.11.98 16:15-------------------
 *
 * --------------------------------------------------*/
void OfaQuoteTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(FALSE);
}
/*-----------------03.07.97 13:18-------------------

--------------------------------------------------*/
void OfaQuoteTabPage::Reset( const SfxItemSet& rSet )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    const long nFlags = pAutoCorrect->GetFlags();

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


/*-----------------15.10.96 16.42-------------------

--------------------------------------------------*/
#define SGL_START       0
#define DBL_START       1
#define SGL_END         2
#define DBL_END         3


IMPL_LINK( OfaQuoteTabPage, QuoteHdl, PushButton*, pBtn )
{
    BOOL bStart = pBtn == &aStartQuotePB;
    USHORT nMode = SGL_START;
    if(pBtn == &aSglEndQuotePB)
        nMode = SGL_END;
    else if(pBtn == &aStartQuotePB)
        nMode = DBL_START;
    else if(pBtn == &aEndQuotePB)
        nMode = DBL_END;
    // Zeichenauswahl-Dialog starten
    SvxCharacterMap* pMap = new SvxCharacterMap( this, TRUE );
    pMap->SetCharFont( OutputDevice::GetDefaultFont(DEFAULTFONT_LATIN_TEXT,
                        LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE, 0 ));
    pMap->SetText(nMode < SGL_END ? sStartQuoteDlg  :  sEndQuoteDlg );
    sal_Unicode cDlg;
    //The two lines below are added by BerryJia for Bug95846 Time:2002-8-13 15:50
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    LanguageType eLang = Application::GetSettings().GetLanguage();
    switch( nMode )
    {
        case SGL_START:
            cDlg = cSglStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'',TRUE,eLang);   //add by BerryJia for Bug95846 Time:2002-8-13 15:50
        break;
        case SGL_END:
            cDlg = cSglEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'',FALSE,eLang);  //add by BerryJia for Bug95846 Time:2002-8-13 15:50
        break;
        case DBL_START:
            cDlg = cStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"',TRUE,eLang);   //add by BerryJia for Bug95846 Time:2002-8-13 15:50
        break;
        case DBL_END:
            cDlg = cEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"',FALSE,eLang);  //add by BerryJia for Bug95846 Time:2002-8-13 15:50
        break;
    }
    pMap->SetChar(  cDlg );
    pMap->DisableFontSelection();
    if(pMap->Execute() == RET_OK)
    {
        sal_Unicode cNewChar = pMap->GetChar();
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

/*-----------------27.06.97 09.55-------------------

--------------------------------------------------*/
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

/*-----------------15.10.96 16.25-------------------

--------------------------------------------------*/


String OfaQuoteTabPage::ChangeStringExt_Impl( sal_Unicode cChar )
{
    if( cChar )
    {
        String sExt = cChar;
        sExt += String::CreateFromAscii(" ( ");

        //convert value to hex display
        String sHex = String::CreateFromAscii("0x0000");
        for(USHORT i = 0; i < 4; i++)
        {
            BYTE cValue = cChar & 0x0f;
            cChar >>= 4;
            sal_Unicode cResult = cValue > 9 ? ('A' + cValue - 10) : ('0' + cValue);
            sHex.SetChar(sHex.Len() - i - 1, cResult);
        }
        sExt += sHex;
        sExt += String::CreateFromAscii(" )");
        return sExt;
    }
    return sStandard;
}



OfaAutoCompleteTabPage::OfaAutoCompleteTabPage( Window* pParent,
                                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, ResId( RID_OFAPAGE_AUTOCOMPLETE_OPTIONS, DIALOG_MGR() ), rSet),
    aCBActiv        (this, ResId(CB_ACTIV)),
    aCBCollect      (this, ResId(CB_COLLECT)),
    aCBKeepList     (this, ResId(CB_KEEP_LIST)),
    aCBAppendSpace  (this, ResId(CB_APPEND_SPACE)),
    aCBAsTip        (this, ResId(CB_AS_TIP)),
    aFTMinWordlen   (this, ResId(FT_MIN_WORDLEN)),
    aFTMaxEntries   (this, ResId(FT_MAX_ENTRIES)),
    aNFMinWordlen   (this, ResId(NF_MIN_WORDLEN)),
    aNFMaxEntries   (this, ResId(NF_MAX_ENTRIES)),
    aFTExpandKey    (this, ResId(FT_EXPAND_KEY)),
    aDCBExpandKey   (this, ResId(DCB_EXPAND_KEY)),
    aLBEntries      (*this, ResId(LB_ENTRIES)),
    aPBEntries      (this, ResId(PB_ENTRIES)),
    pAutoCmpltList( 0 ),
    nAutoCmpltListCnt( 0 )
{
    FreeResource();

    // the defined KEYs
    static const USHORT aKeyCodes[] = {
        KEY_END,
        KEY_RETURN,
        KEY_SPACE,
        KEY_RIGHT,
        0
    };

    for( const USHORT* pKeys = aKeyCodes; *pKeys; ++pKeys )
    {
        KeyCode aKCode( *pKeys );
        USHORT nPos = aDCBExpandKey.InsertEntry( aKCode.GetName() );
        aDCBExpandKey.SetEntryData( nPos, (void*)(ULONG)*pKeys );
        if( KEY_RETURN == *pKeys )      // default to RETURN
            aDCBExpandKey.SelectEntryPos( nPos );
    }

    aPBEntries.SetClickHdl(LINK(this, OfaAutoCompleteTabPage, DeleteHdl));
    aCBActiv.SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    aCBCollect.SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
}

OfaAutoCompleteTabPage::~OfaAutoCompleteTabPage()
{
}

SfxTabPage* OfaAutoCompleteTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet)
{
    return new OfaAutoCompleteTabPage( pParent, rSet );
}

BOOL OfaAutoCompleteTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE, bCheck;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    USHORT nVal;

    bCheck = aCBActiv.IsChecked();
    bModified |= pOpt->bAutoCompleteWords != bCheck;
    pOpt->bAutoCompleteWords = bCheck;
    bCheck = aCBCollect.IsChecked();
    bModified |= pOpt->bAutoCmpltCollectWords != bCheck;
    pOpt->bAutoCmpltCollectWords = bCheck;
    bCheck = aCBKeepList.IsChecked();
    bModified |= pOpt->bAutoCmpltKeepList != bCheck;
    pOpt->bAutoCmpltKeepList = bCheck;
    bCheck = aCBAppendSpace.IsChecked();
    bModified |= pOpt->bAutoCmpltAppendBlanc != bCheck;
    pOpt->bAutoCmpltAppendBlanc = bCheck;
    bCheck = aCBAsTip.IsChecked();
    bModified |= pOpt->bAutoCmpltShowAsTip != bCheck;
    pOpt->bAutoCmpltShowAsTip = bCheck;

    nVal = (USHORT)aNFMinWordlen.GetValue();
    bModified |= nVal != pOpt->nAutoCmpltWordLen;
    pOpt->nAutoCmpltWordLen = nVal;

    nVal = (USHORT)aNFMaxEntries.GetValue();
    bModified |= nVal != pOpt->nAutoCmpltListLen;
    pOpt->nAutoCmpltListLen = nVal;

    nVal = aDCBExpandKey.GetSelectEntryPos();
    if( nVal < aDCBExpandKey.GetEntryCount() )
    {
        ULONG nKey = (ULONG)aDCBExpandKey.GetEntryData( nVal );
        bModified |= nKey != pOpt->nAutoCmpltExpandKey;
        pOpt->nAutoCmpltExpandKey = (USHORT)nKey;
   }

    if( pAutoCmpltList && nAutoCmpltListCnt != aLBEntries.GetEntryCount() )
    {
        bModified = TRUE;
        pOpt->pAutoCmpltList = pAutoCmpltList;
    }
    if( bModified )
    {
        SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
        pCfg->SetModified();
        pCfg->Commit();
    }
    return TRUE;
}

void OfaAutoCompleteTabPage::Reset( const SfxItemSet& rSet )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

    aCBActiv.Check( 0 != pOpt->bAutoCompleteWords );
    aCBCollect.Check( 0 != pOpt->bAutoCmpltCollectWords );
    aCBKeepList.Check( 0 != pOpt->bAutoCmpltKeepList);
    aCBAppendSpace.Check( 0 != pOpt->bAutoCmpltAppendBlanc );
    aCBAsTip.Check( 0 != pOpt->bAutoCmpltShowAsTip );

    aNFMinWordlen.SetValue( pOpt->nAutoCmpltWordLen );
    aNFMaxEntries.SetValue( pOpt->nAutoCmpltListLen );

    // select the specific KeyCode:
    {
        ULONG nKey = pOpt->nAutoCmpltExpandKey;
        for( USHORT n = 0, nCnt = aDCBExpandKey.GetEntryCount(); n < nCnt; ++n )
            if( nKey == (ULONG)aDCBExpandKey.GetEntryData( n ))
            {
                aDCBExpandKey.SelectEntryPos( n );
                break;
            }
    }

    if( pOpt->pAutoCmpltList && pOpt->pAutoCmpltList->Count() )
    {
        pAutoCmpltList = (SvStringsISortDtor*)pOpt->pAutoCmpltList;
        pOpt->pAutoCmpltList = 0;
        nAutoCmpltListCnt = pAutoCmpltList->Count();
        for( USHORT n = 0; n < nAutoCmpltListCnt; ++n )
        {
            const StringPtr pStr = pAutoCmpltList->GetObject( n );
            USHORT nPos = aLBEntries.InsertEntry( *pStr );
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
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage( FALSE );
}

IMPL_LINK( OfaAutoCompleteTabPage, DeleteHdl, PushButton*, pBtn )
{
    USHORT nSelCnt = pAutoCmpltList ? aLBEntries.GetSelectEntryCount() : 0;
    while( nSelCnt )
    {
        USHORT nPos = aLBEntries.GetSelectEntryPos( --nSelCnt );
        const StringPtr pStr = (StringPtr)aLBEntries.GetEntryData( nPos );
        aLBEntries.RemoveEntry( nPos );
        nPos = pAutoCmpltList->GetPos( pStr );
        if( USHRT_MAX != nPos )
            pAutoCmpltList->Remove( nPos );
    }
    return 0;
}

IMPL_LINK( OfaAutoCompleteTabPage, CheckHdl, CheckBox*, pBox )
{
    BOOL bEnable = pBox->IsChecked();
    if( pBox == &aCBActiv )
    {
        aCBAppendSpace.Enable( bEnable );
        aCBAppendSpace.Enable( bEnable );
        aCBAsTip.Enable( bEnable );
        aDCBExpandKey.Enable( bEnable );
    }
    else if(&aCBCollect == pBox)
        aCBKeepList.Enable( bEnable );
    return 0;
}

void OfaAutoCompleteTabPage::CopyToClipboard() const
{
    USHORT nSelCnt = aLBEntries.GetSelectEntryCount();
    if( pAutoCmpltList && nSelCnt )
    {
        TransferDataContainer* pCntnr = new TransferDataContainer;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::datatransfer::XTransferable > xRef( pCntnr );

        ByteString sData;
        const sal_Char* pLineEnd =
#if defined(MAC)
                "\015";
#elif defined(UNX)
                "\012";
#else
                "\015\012";
#endif

        rtl_TextEncoding nEncode = gsl_getSystemTextEncoding();

        for( USHORT n = 0; n < nSelCnt; ++n )
        {
            sData += ByteString( aLBEntries.GetSelectEntry( n ), nEncode );
            sData += pLineEnd;
        }
        pCntnr->CopyByteString( SOT_FORMAT_STRING, sData );
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


