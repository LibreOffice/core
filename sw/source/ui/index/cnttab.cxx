/*************************************************************************
 *
 *  $RCSfile: cnttab.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-31 21:24:21 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <rsc/rscsfx.hxx>

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_SIMPTABL_HXX //autogen wg. SvxSimpleTable
#include <svx/simptabl.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEXESSUPPLIER_HPP_
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTIONSSUPPLIER_HPP_
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XPARAGRAPHCURSOR_HPP_
#include <com/sun/star/text/XParagraphCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#ifndef _COLUMN_HXX //autogen
#include <column.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _OUTLINE_HXX
#include <outline.hxx>
#endif
#ifndef _CNTTAB_HXX
#include <cnttab.hxx>
#endif
#ifndef _FORMEDT_HXX
#include <formedt.hxx>
#endif
#ifndef _TOXMGR_HXX
#include <toxmgr.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _UIITEMS_HXX //autogen
#include <uiitems.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _FLDBAS_HXX //autogen wg. SwFieldType
#include <fldbas.hxx>
#endif
#ifndef _EXPFLD_HXX //autogen wg. SwSetExpFieldType
#include <expfld.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODOPT_HXX
#include <modcfg.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _INDEX_HRC
#include <index.hrc>
#endif
#ifndef _CNTTAB_HRC
#include <cnttab.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif



using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

#define C2S(cChar) UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))
#define C2U(cChar) OUString::createFromAscii(cChar)

#ifndef _SVX_DBBROWSE_HXX
#include <svx/dbbrowse.hxx>
#endif

static const sal_Unicode aDeliStart = '['; //fuer die form
static const sal_Unicode aDeliEnd    = ']'; //fuer die form

#define POS_GRF     0
#define POS_OLE     1
#define POS_TABLE   2
#define POS_FRAME   3

#define IDX_FILE_EXTENSION String::CreateFromAscii( \
                            RTL_CONSTASCII_STRINGPARAM( "*.sdi" ))
#define INDEX_SECTION_NAME String::CreateFromAscii( \
                            RTL_CONSTASCII_STRINGPARAM( "IndexSection_" ))

#define TOX_PAGE_SELECT 1
#define TOX_PAGE_ENTRY  2
#define TOX_PAGE_STYLES 3

//-----------------------------------------------------------------------------
struct SwIndexSections_Impl
{
    uno::Reference< text::XTextSection >    xContainerSection;
    uno::Reference< text::XDocumentIndex >    xDocumentIndex;
};

/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
String lcl_CreateAutoMarkFileDlg( Window* pParent, const String& rURL,
                                const String& rFileString, sal_Bool bOpen )
{
    String sRet;
    SfxFileDialog* pFileDlg = new SfxFileDialog( pParent,
                                    bOpen ? WB_OPEN | WB_3DLOOK
                                           : WB_SAVEAS | WB_3DLOOK );
    pFileDlg->DisableSaveLastDirectory();
    pFileDlg->SetHelpId(HID_FILEDLG_SRCVIEW);
    {
        String sCurFltr( IDX_FILE_EXTENSION );
        pFileDlg->AddFilter( rFileString, sCurFltr );
        pFileDlg->SetCurFilter( sCurFltr );
    }

    String& rLastSaveDir = (String&)SFX_APP()->GetLastSaveDirectory();
    String sSaveDir = rLastSaveDir;

    if( rURL.Len() )
        pFileDlg->SetPath( rURL );
    else
    {
        SvtPathOptions aPathOpt;
        pFileDlg->SetPath( aPathOpt.GetUserConfigPath() );
    }

    if( RET_OK == pFileDlg->Execute())
        sRet = pFileDlg->GetPath();

    delete pFileDlg;
    rLastSaveDir = sSaveDir;

    return sRet;
}
/* -----------------------------19.01.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
struct AutoMarkEntry
{
    String sSearch;
    String sAlternative;
    String sPrimKey;
    String sSecKey;
    String sComment;
    sal_Bool    bCase;
    sal_Bool    bWord;

    AutoMarkEntry() :
        bCase(sal_False),
        bWord(sal_False){}
};
typedef AutoMarkEntry* AutoMarkEntryPtr;
SV_DECL_PTRARR_DEL(AutoMarkEntryArr, AutoMarkEntryPtr, 0, 4);
SV_IMPL_PTRARR(AutoMarkEntryArr, AutoMarkEntryPtr);

class SwEntryBrowseBox : public DbBrowseBox
{
    Edit            aCellEdit;
    DbCheckBoxCtrl  aCellCheckBox;

    String  sSearch;
    String  sAlternative;
    String  sPrimKey;
    String  sSecKey;
    String  sComment;
    String  sCaseSensitive;
    String  sWordOnly;
    String  sYes;
    String  sNo;

    AutoMarkEntryArr    aEntryArr;

    DbCellControllerRef xController;
    DbCellControllerRef xCheckController;

    long    nCurrentRow;
    sal_Bool    bModified;

    const String& GetCellText( long nRow, long nColumn ) const;
    void                        SetModified() {bModified = sal_True;}

protected:
    virtual sal_Bool                SeekRow( long nRow );
//  virtual void                PaintField( OutputDevice& rDev, const awt::Rectangle& rRect,
//                                          sal_uInt16 nColumnId ) const;
    virtual void                PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const;
    virtual void                InitController(DbCellControllerRef& rController, long nRow, sal_uInt16 nCol);
    virtual DbCellController*   GetController(long nRow, sal_uInt16 nCol);
    virtual sal_Bool                SaveModified();

public:
                    SwEntryBrowseBox(Window* pParent, const ResId& rId,
                               BrowserMode nMode = 0 );
    void    ReadEntries(SvStream& rInStr);
    void    WriteEntries(SvStream& rOutStr);

    sal_Bool                        IsModified()const;
};

class SwAutoMarkDlg_Impl : public ModalDialog
{
    OKButton            aOKPB;
    CancelButton        aCancelPB;
    HelpButton          aHelpPB;

    SwEntryBrowseBox    aEntriesBB;
    GroupBox            aEntriesGB;

    String              sAutoMarkURL;
    const String        sAutoMarkType;

    sal_Bool                bCreateMode;

    DECL_LINK(OkHdl, OKButton*);
public:
    SwAutoMarkDlg_Impl(Window* pParent, const String& rAutoMarkURL,
                        const String& rAutoMarkType, sal_Bool bCreate);
    ~SwAutoMarkDlg_Impl();

};
/* -----------------04.11.99 11:02-------------------

 --------------------------------------------------*/
sal_uInt16 CurTOXType::GetFlatIndex() const
{
    sal_uInt16 nRet = eType;
    if(eType == TOX_USER && nIndex)
    {
        nRet = TOX_AUTHORITIES + nIndex;
    }
    return nRet;
}
/* -----------------29.11.99 09:04-------------------

 --------------------------------------------------*/
#if 0
IdxExampleResource::IdxExampleResource(const ResId& rResId) :
    Resource(rResId),
    aTextArray(ResId(ARR_TEXT))
{
    FreeResource();
}
#endif
/*************************************************************************

*************************************************************************/
#define EDIT_MINWIDTH 15

/* -----------------14.06.99 12:12-------------------

 --------------------------------------------------*/
SwMultiTOXTabDialog::SwMultiTOXTabDialog(Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell &rShell,
                    const SwTOXBase* pCurTOX,
                    sal_uInt16 nToxType, sal_Bool bGlobal) :
        SfxTabDialog(   pParent, SW_RES(DLG_MULTI_TOX), &rSet),
        aExampleContainerWIN(this, ResId(WIN_EXAMPLE)),
        aExampleWIN( &aExampleContainerWIN, 0 ),
        aShowExampleCB( this, ResId(CB_SHOWEXAMPLE)),
        sUserDefinedIndex(ResId(ST_USERDEFINEDINDEX)),
        pMgr( new SwTOXMgr( &rShell ) ),
        rSh(rShell),
        nInitialTOXType(nToxType),
        bEditTOX(sal_False),
        pExampleFrame(0),
        bExampleCreated(sal_False),
        bGlobalFlag(bGlobal)
{
    FreeResource();
    aExampleWIN.SetPosSizePixel(aExampleContainerWIN.GetPosPixel(),
                                aExampleContainerWIN.GetSizePixel());
    eCurrentTOXType.eType = TOX_CONTENT;
    eCurrentTOXType.nIndex = 0;

    sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    nTypeCount = nUserTypeCount + 6;
    pFormArr = new SwForm*[nTypeCount];
    pDescArr = new SwTOXDescription*[nTypeCount];
    pxIndexSectionsArr = new SwIndexSections_Impl*[nTypeCount];
    //the standard user index is on position TOX_USER
    //all user user indexes follow after position TOX_AUTHORITIES
    if(pCurTOX)
    {
        bEditTOX = sal_True;
    }
    for(int i = nTypeCount - 1; i > -1; i--)
    {
        pFormArr[i] = 0;
        pDescArr[i] = 0;
        pxIndexSectionsArr[i] = new SwIndexSections_Impl;
        if(pCurTOX)
        {
            eCurrentTOXType.eType = pCurTOX->GetType();
            sal_uInt16 nArrayIndex = eCurrentTOXType.eType;
            if(eCurrentTOXType.eType == TOX_USER)
            {
                //which user type is it?
                for(sal_uInt16 nUser = 0; nUser < nUserTypeCount; nUser++)
                {
                    const SwTOXType* pTemp = rSh.GetTOXType(TOX_USER, nUser);
                    if(pCurTOX->GetTOXType() == pTemp)
                    {
                        eCurrentTOXType.nIndex = nUser;
                        nArrayIndex = nUser > 0 ? TOX_AUTHORITIES + nUser : TOX_USER;
                        break;
                    }
                }
            }
            pFormArr[nArrayIndex] = new SwForm(pCurTOX->GetTOXForm());
            pDescArr[nArrayIndex] = CreateTOXDescFromTOXBase(pCurTOX);
            if(TOX_AUTHORITIES == eCurrentTOXType.eType)
            {
                const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                                rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
                if(pFType)
                {
                    String sBrackets(pFType->GetPrefix());
                    sBrackets += pFType->GetSuffix();
                    pDescArr[nArrayIndex]->SetAuthBrackets(sBrackets);
                    pDescArr[nArrayIndex]->SetAuthSequence(pFType->IsSequence());
                }
                else
                {
                    pDescArr[nArrayIndex]->SetAuthBrackets(C2S("[]"));
                }
            }
        }
    }

    AddTabPage(TP_TOX_SELECT, SwTOXSelectTabPage::Create, 0);
    AddTabPage(TP_TOX_STYLES, SwTOXStylesTabPage::Create, 0);
    AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,  0);
    AddTabPage(TP_TOX_ENTRY, SwTOXEntryTabPage::Create,     0);
    if(!pCurTOX)
        SetCurPageId(TP_TOX_SELECT);

    aShowExampleCB.SetClickHdl(LINK(this, SwMultiTOXTabDialog, ShowPreviewHdl));

    aShowExampleCB.Check( SW_MOD()->GetModuleConfig()->IsShowIndexPreview());
    SetViewAlign( WINDOWALIGN_LEFT );
    // SetViewWindow does not work if the dialog is visible!
    if(!aShowExampleCB.IsChecked())
    {
        SetViewWindow( &aExampleContainerWIN );
    }
    Point aOldPos = GetPosPixel();
    ShowPreviewHdl(0);
     Point aNewPos = GetPosPixel();
    //72040: initial position may be left of the view - that has to be corrected
    if(aNewPos.X() < 0)
        SetPosPixel(aOldPos);
}
/*-- 14.06.99 13:11:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMultiTOXTabDialog::~SwMultiTOXTabDialog()
{
    SW_MOD()->GetModuleConfig()->SetShowIndexPreview(aShowExampleCB.IsChecked());

    for(sal_uInt16 i = 0; i < nTypeCount; i++)
    {
        delete pFormArr[i];
        delete pDescArr[i];
        delete pxIndexSectionsArr[i];
    }
    delete pxIndexSectionsArr;

    delete pFormArr;
    delete pDescArr;
    delete pMgr;
    delete pExampleFrame;
}
/*-- 14.06.99 13:11:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwMultiTOXTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if( TP_BACKGROUND == nId  )

        ((SvxBackgroundTabPage&)rPage).ShowSelector();
    else if(TP_COLUMN == nId )
    {
        const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)GetInputSetImpl()->Get(RES_FRM_SIZE);

        ((SwColumnPage&)rPage).SetPageWidth(rSize.GetWidth());
    }
    else if(TP_TOX_ENTRY == nId)
        ((SwTOXEntryTabPage&)rPage).SetWrtShell(rSh);
    if(TP_TOX_SELECT == nId)
    {
        ((SwTOXSelectTabPage&)rPage).SetWrtShell(rSh);
        if(USHRT_MAX != nInitialTOXType)
            ((SwTOXSelectTabPage&)rPage).SelectType((TOXTypes)nInitialTOXType);
    }
}
/*-- 14.06.99 13:11:40---------------------------------------------------

  -----------------------------------------------------------------------*/
short   SwMultiTOXTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    SwTOXDescription& rDesc = GetTOXDescription(eCurrentTOXType);
    SwTOXBase aNewDef(*rSh.GetDefaultTOXBase( eCurrentTOXType.eType, sal_True ));

    sal_uInt16 nIndex = eCurrentTOXType.eType;
    if(eCurrentTOXType.eType == TOX_USER && eCurrentTOXType.nIndex)
    {
        nIndex = TOX_AUTHORITIES + eCurrentTOXType.nIndex;
    }

    if(pFormArr[nIndex])
    {
        rDesc.SetForm(*pFormArr[nIndex]);
        aNewDef.SetTOXForm(*pFormArr[nIndex]);
    }
    rDesc.ApplyTo(aNewDef);
    if(!bGlobalFlag)
        pMgr->UpdateOrInsertTOX(
                rDesc, 0, GetOutputItemSet());

    if(!eCurrentTOXType.nIndex)
        rSh.SetDefaultTOXBase(aNewDef);

    return nRet;
}
/* -----------------16.06.99 11:59-------------------

 --------------------------------------------------*/
SwForm* SwMultiTOXTabDialog::GetForm(CurTOXType eType)
{
    sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!pFormArr[nIndex])
        pFormArr[nIndex] = new SwForm(eType.eType);
    return pFormArr[nIndex];
}
/* -----------------09.09.99 11:29-------------------

 --------------------------------------------------*/
SwTOXDescription&   SwMultiTOXTabDialog::GetTOXDescription(CurTOXType eType)
{
    sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!pDescArr[nIndex])
    {
        const SwTOXBase* pDef = rSh.GetDefaultTOXBase( eType.eType );
        if(pDef)
            pDescArr[nIndex] = CreateTOXDescFromTOXBase(pDef);
        else
        {
            pDescArr[nIndex] = new SwTOXDescription(eType.eType);
            if(eType.eType == TOX_USER)
                pDescArr[nIndex]->SetTitle(sUserDefinedIndex);
            else
                pDescArr[nIndex]->SetTitle(
                    rSh.GetTOXType(eType.eType, 0)->GetTypeName());
        }
        if(TOX_AUTHORITIES == eType.eType)
        {
            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                            rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                String sBrackets(pFType->GetPrefix());
                sBrackets += pFType->GetSuffix();
                pDescArr[nIndex]->SetAuthBrackets(sBrackets);
                pDescArr[nIndex]->SetAuthSequence(pFType->IsSequence());
            }
            else
            {
                pDescArr[nIndex]->SetAuthBrackets(C2S("[]"));
            }
        }
    }
    return *pDescArr[nIndex];
}
/* -----------------09.09.99 11:36-------------------

 --------------------------------------------------*/
SwTOXDescription* SwMultiTOXTabDialog::CreateTOXDescFromTOXBase(
            const SwTOXBase*pCurTOX)
{
    SwTOXDescription * pDesc = new SwTOXDescription(pCurTOX->GetType());
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pDesc->SetStyleNames(pCurTOX->GetStyleNames(i), i);
    pDesc->SetAutoMarkURL(rSh.GetTOIAutoMarkURL());
    pDesc->SetTitle(pCurTOX->GetTitle());


    pDesc->SetContentOptions(pCurTOX->GetCreateType());
    if(pDesc->GetTOXType() == TOX_INDEX)
        pDesc->SetIndexOptions(pCurTOX->GetOptions());
    pDesc->SetMainEntryCharStyle(pCurTOX->GetMainEntryCharStyle());
    if(pDesc->GetTOXType() != TOX_INDEX)
        pDesc->SetLevel(pCurTOX->GetLevel());
    pDesc->SetCreateFromObjectNames(pCurTOX->IsFromObjectNames());
    pDesc->SetSequenceName(pCurTOX->GetSequenceName());
    pDesc->SetCaptionDisplay(pCurTOX->GetCaptionDisplay());
    pDesc->SetFromChapter(pCurTOX->IsFromChapter());
    pDesc->SetReadonly(pCurTOX->IsProtected());
    pDesc->SetOLEOptions(pCurTOX->GetOLEOptions());
    pDesc->SetLevelFromChapter(pCurTOX->IsLevelFromChapter());
    return pDesc;
}

/* -----------------------------29.12.99 09:53--------------------------------

 ---------------------------------------------------------------------------*/

IMPL_LINK( SwMultiTOXTabDialog, ShowPreviewHdl, CheckBox *, pBox )
{
    if(aShowExampleCB.IsChecked())
    {
        if(!pExampleFrame && !bExampleCreated)
        {
            bExampleCreated = sal_True;
            const sal_Unicode  cSearch = SFX_SEARCHPATH_DELIMITER;
            const String  sAccess = INET_PATH_TOKEN;
            SvtPathOptions aPathOpt;

            String sPath( aPathOpt.GetTemplatePath() );
            sal_uInt16 nTokenCount = sPath.GetTokenCount(cSearch);
            sal_uInt16 nToken = 0;
            sal_Bool bExist = sal_False;
            String sFile = sAccess;
            sFile.AppendAscii( RTL_CONSTASCII_STRINGPARAM("internal"));
            sFile += sAccess;
            sFile.AppendAscii( RTL_CONSTASCII_STRINGPARAM("idxexample.sdw"));
            String sURL;
            do
            {
                sURL = sPath.GetToken(nToken, cSearch);
                sURL = URIHelper::SmartRelToAbs(sURL);
                sURL += sFile;
                nToken++;
                try
                {
                    ::ucb::Content aTestContent(
                        sURL,
                        uno::Reference< XCommandEnvironment >());

                    uno::Any aAny = aTestContent.getPropertyValue( OUString::createFromAscii("IsDocument") );
                    bExist = aAny.hasValue() ? *(sal_Bool*)aAny.getValue() : FALSE;
                }
                catch(...)
                {
                    bExist = FALSE;
                }

            }while(!bExist && nToken < nTokenCount);

            if(!bExist)
            {
                String sInfo(SW_RES(STR_FILE_NOT_FOUND));
                sInfo.SearchAndReplaceAscii("%1", sFile);
                sInfo.SearchAndReplaceAscii("%2", sPath);
                InfoBox aInfo(GetParent(), sInfo);
                aInfo.Execute();
            }
            else
            {
                Link aLink(LINK(this, SwMultiTOXTabDialog, CreateExample_Hdl));
                pExampleFrame = new SwOneExampleFrame(
                            aExampleWIN, EX_SHOW_ONLINE_LAYOUT, &aLink, &sURL);

                if(!pExampleFrame->IsServiceAvailable())
                {
                    pExampleFrame->CreateErrorMessage(0);
                }
            }
            aShowExampleCB.Show(pExampleFrame && pExampleFrame->IsServiceAvailable());
        }
    }
    sal_Bool bSetViewWindow = aShowExampleCB.IsChecked()
        && pExampleFrame && pExampleFrame->IsServiceAvailable();
    aExampleContainerWIN.Show(bSetViewWindow);

    SetViewWindow( bSetViewWindow ? &aExampleContainerWIN  : 0);
     Point aPos = GetPosPixel();
     Size aSize = GetSizePixel();
    if(pBox)
        AdjustLayout();
    long nDiffWidth = GetSizePixel().Width() - aSize.Width();
    aPos.X() -= nDiffWidth;
    SetPosPixel(aPos);

    return 0;
}
//-----------------------------------------------------------------------------
IMPL_LINK( SwMultiTOXTabDialog, CreateExample_Hdl, void*, EMPTYARG )
{
    try
    {
         uno::Reference< frame::XModel > & xModel = pExampleFrame->GetModel();
        uno::Reference< lang::XUnoTunnel > xDocTunnel(xModel, uno::UNO_QUERY);
        SwXTextDocument* pDoc = (SwXTextDocument*)xDocTunnel->getSomething(SwXTextDocument::getUnoTunnelId());

        if( pDoc )
            pDoc->GetDocShell()->LoadStyles( *rSh.GetView().GetDocShell() );

         uno::Reference< lang::XMultiServiceFactory >  xFact(
                                             xModel, uno::UNO_QUERY);
         uno::Reference< text::XTextCursor > & xCrsr =
                                             pExampleFrame->GetTextCursor();

         uno::Reference< text::XTextSectionsSupplier >  xSectionSupplier(
                                                 xModel, uno::UNO_QUERY);
         uno::Reference< container::XNameAccess >  xSections =
                                        xSectionSupplier->getTextSections();

        String sSectionName( INDEX_SECTION_NAME );
        for(int i = 0; i < 7; ++i )
        {
            String sTmp( sSectionName ); sTmp += String::CreateFromInt32(i);
            uno::Any aSection = xSections->getByName( sTmp );
            pxIndexSectionsArr[i]->xContainerSection = * (uno::Reference<
                                text::XTextSection > *)aSection.getValue();
         }
         uno::Reference< text::XDocumentIndexesSupplier >  xIdxSupp(xModel, uno::UNO_QUERY);
         uno::Reference< container::XIndexAccess >  xIdxs = xIdxSupp->getDocumentIndexes();
        int n = xIdxs->getCount();
        while(n)
        {
            n--;
            uno::Any aIdx = xIdxs->getByIndex(n);
             uno::Reference< text::XDocumentIndex >  xIdx = * (uno::Reference< text::XDocumentIndex > *)aIdx.getValue();
            xIdx->dispose();
        }
        CreateOrUpdateExample(eCurrentTOXType.eType);
    }
    catch(...)
    {
        DBG_ERROR("::CreateExample() - exception caught")
    }
    return 0;
}
/* -----------------04.11.99 11:28-------------------

 --------------------------------------------------*/
void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
    uno::Reference< XPropertySet > & xProps,
    const char* rPropName,
    const String& rValue)
{
    OUString uPropName(C2U(rPropName));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue <<= OUString(rValue);
        xProps->setPropertyValue(uPropName, aValue);
    }
}
void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
    uno::Reference< beans::XPropertySet > & xProps,
    const char* rPropName, sal_Int16 nValue)
{
    OUString uPropName(C2U(rPropName));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue <<= nValue;
        xProps->setPropertyValue(uPropName, aValue);
    }
}
/* --------------------------------------------------

 --------------------------------------------------*/
void lcl_SetBOOLProp( uno::Reference< beans::XPropertySetInfo > & xInfo,
    uno::Reference< beans::XPropertySet > & xProps,
    const char* rPropName, sal_Bool bValue)
{
    OUString uPropName(C2U(rPropName));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue.setValue(&bValue, ::getCppuBooleanType());
        xProps->setPropertyValue(uPropName, aValue);
    }
}
/* --------------------------------------------------

 --------------------------------------------------*/
void SwMultiTOXTabDialog::CreateOrUpdateExample(
    sal_uInt16 nTOXIndex, sal_uInt16 nPage, sal_uInt16 nCurrentLevel)
{
    if(!pExampleFrame || !pExampleFrame->IsInitialized())
        return;

    const char* __FAR_DATA IndexServiceNames[] =
    {
        "com.sun.star.text.DocumentIndex",
        "com.sun.star.text.UserIndex",
        "com.sun.star.text.ContentIndex",
        "com.sun.star.text.IllustrationsIndex",
        "com.sun.star.text.ObjectIndex",
        "com.sun.star.text.TableIndex",
        "com.sun.star.text.Bibliography"
    };

    try
    {
        DBG_ASSERT(pxIndexSectionsArr[nTOXIndex] &&
                        pxIndexSectionsArr[nTOXIndex]->xContainerSection.is(),
                            "Section not created")
         uno::Reference< frame::XModel > & xModel = pExampleFrame->GetModel();
        sal_Bool bInitialCreate = sal_True;
        if(!pxIndexSectionsArr[nTOXIndex]->xDocumentIndex.is())
        {
            bInitialCreate = sal_True;
         uno::Reference< text::XTextRange >  xAnchor = pxIndexSectionsArr[nTOXIndex]->xContainerSection->getAnchor();
            xAnchor = xAnchor->getStart();
         uno::Reference< text::XTextCursor >  xCrsr = xAnchor->getText()->createTextCursorByRange(xAnchor);

         uno::Reference< lang::XMultiServiceFactory >  xFact(xModel, uno::UNO_QUERY);

            String sIndexTypeName;
            sIndexTypeName.AssignAscii( IndexServiceNames[
                    nTOXIndex <= TOX_AUTHORITIES ? nTOXIndex : TOX_USER] );
            pxIndexSectionsArr[nTOXIndex]->xDocumentIndex = uno::Reference< text::XDocumentIndex > (xFact->createInstance(
                                                    sIndexTypeName), uno::UNO_QUERY);
         uno::Reference< text::XTextContent >  xContent(pxIndexSectionsArr[nTOXIndex]->xDocumentIndex, uno::UNO_QUERY);
         uno::Reference< text::XTextRange >  xRg(xCrsr, uno::UNO_QUERY);
            xCrsr->getText()->insertTextContent(xRg, xContent, sal_False);
        }
        OUString uIsVisible(C2U(UNO_NAME_IS_VISIBLE));
        for(sal_uInt16 i = 0 ; i <= TOX_AUTHORITIES; i++)
        {
         uno::Reference< beans::XPropertySet >  xSectPr(pxIndexSectionsArr[i]->xContainerSection, uno::UNO_QUERY);
            uno::Any aVal;

            if(xSectPr.is())
            {
                BOOL bTemp = i == nTOXIndex;
                aVal.setValue(&bTemp, ::getBooleanCppuType());
                xSectPr->setPropertyValue(uIsVisible, aVal);
            }
        }
        // set properties
     uno::Reference< beans::XPropertySet >  xIdxProps(pxIndexSectionsArr[nTOXIndex]->xDocumentIndex, uno::UNO_QUERY);
     uno::Reference< beans::XPropertySetInfo >  xInfo = xIdxProps->getPropertySetInfo();
        SwTOXDescription& rDesc = GetTOXDescription(eCurrentTOXType);
        sal_uInt16 nIdxOptions = rDesc.GetIndexOptions();
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_SELECT)
        {
            //title
            if(rDesc.GetTitle())
                lcl_SetProp(xInfo, xIdxProps, UNO_NAME_TITLE, *rDesc.GetTitle());

            //stylenames
            sal_uInt16  nContentOptions = rDesc.GetContentOptions();
            if(xInfo->hasPropertyByName(C2U(UNO_NAME_LEVEL_PARAGRAPH_STYLES)))
            {
                sal_Bool bOn = 0!=(nContentOptions&TOX_TEMPLATE    );
                uno::Any aStyleNames(xIdxProps->getPropertyValue(C2U(UNO_NAME_LEVEL_PARAGRAPH_STYLES)));
                 uno::Reference< container::XIndexReplace >  xAcc =
                    *(uno::Reference< container::XIndexReplace > *)aStyleNames.getValue();

                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    String sLevel;
                    if(bOn)
                        sLevel = rDesc.GetStyleNames(i);
                    sal_uInt16 nStyles = sLevel.GetTokenCount(TOX_STYLE_DELIMITER);
                    uno::Sequence<OUString> aStyles(nStyles);
                    OUString* pArr = aStyles.getArray();
                    for(sal_uInt16 nStyle = 0; nStyle < nStyles; nStyle++)
                        pArr[nStyle] = sLevel.GetToken(nStyle, TOX_STYLE_DELIMITER);
                    uno::Any aAny(&aStyles, ::getCppuType((uno::Sequence<OUString>*)0));
                    xAcc->replaceByIndex(i, aAny);
                }
            }
            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LEVEL, (sal_Int16)rDesc.GetLevel());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_MARKS,           0!=(nContentOptions&TOX_MARK        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_OUTLINE,         0!=(nContentOptions&TOX_OUTLINELEVEL));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS,0!=(nContentOptions&TOX_OLE            ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_TABLES ,         0!=(nContentOptions&TOX_TABLE          ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS, 0!=(nContentOptions&TOX_GRAPHIC        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_TEXT_FRAMES,     0!=(nContentOptions&TOX_FRAME          ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_LABELS,          0!=(nContentOptions&TOX_SEQUENCE       ));

            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_CHAPTER, rDesc.IsFromChapter());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_PROTECTED, rDesc.IsReadonly());

            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_COMBINED_ENTRIES,        0 != (nIdxOptions&TOI_SAME_ENTRY        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_P_P,                     0 != (nIdxOptions&TOI_FF                   ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_CASE_SENSITIVE,           0 != (nIdxOptions&TOI_CASE_SENSITIVE     ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_KEY_AS_ENTRY,            0 != (nIdxOptions&TOI_KEY_AS_ENTRY     ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_ALPHABETICAL_SEPARATORS, 0 != (nIdxOptions&TOI_ALPHA_DELIMITTER));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_DASH,                    0 != (nIdxOptions&TOI_DASH             ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_UPPER_CASE,              0 != (nIdxOptions&TOI_INITIAL_CAPS     ));

            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LABEL_CATEGORY,          rDesc.GetSequenceName());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_LABELS,  !rDesc.IsCreateFromObjectNames());

            sal_Int16 nSet = text::ChapterFormat::NAME_NUMBER;
            switch (rDesc.GetCaptionDisplay())
            {
                case CAPTION_COMPLETE:  nSet = text::ChapterFormat::NAME_NUMBER;break;
                case CAPTION_NUMBER  :  nSet = text::ChapterFormat::NUMBER; break;
                case CAPTION_TEXT    :  nSet = text::ChapterFormat::NAME;      break;
            }
            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LABEL_DISPLAY_TYPE, nSet);

            sal_uInt16  nOLEOptions = rDesc.GetOLEOptions();
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_MATH,   0 != (TOO_MATH  &nOLEOptions           ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_CHART,  0 != (TOO_CHART &nOLEOptions       ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_CALC,   0 != (TOO_CALC  &nOLEOptions           ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_DRAW,   0 != (TOO_DRAW_IMPRESS&nOLEOptions));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS, 0 != (TOO_OTHER|nOLEOptions       ));

            //lcl_SetBOOLProp(xInfo, xIdxProps, , rDesc.IsLevelFromChapter());
        }
        const SwForm* pForm = GetForm(eCurrentTOXType);
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_ENTRY)
        {
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_COMMA_SEPARATED, pForm->IsCommaSeparated());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_ALPHABETICAL_SEPARATORS, 0 != (nIdxOptions&TOI_ALPHA_DELIMITTER));
            if(nCurrentLevel < pForm->GetFormMax() &&
                xInfo->hasPropertyByName(C2U(UNO_NAME_LEVEL_FORMAT)))
            {
                String sTokenType;
                 uno::Sequence< beans::PropertyValues> aSequPropVals(10);
                long nTokenIndex = 0;
                long nParamCount = 2;
                sal_Bool bTabRightAligned = sal_False;
                SwFormTokenEnumerator aTokenEnum(pForm->GetPattern(nCurrentLevel));
                while(aTokenEnum.HasNextToken())
                {
                    if( aSequPropVals.getLength() <= nTokenIndex)
                        aSequPropVals.realloc(nTokenIndex + 10);

                    SwFormToken aToken = aTokenEnum.GetNextToken();
                    switch(aToken.eTokenType)
                    {
                        case TOKEN_ENTRY_NO     :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenEntryNumber"));
                            // fuer Inhaltsverzeichnis Numerierung
                        break;
                        case TOKEN_ENTRY_TEXT   :
                        case TOKEN_ENTRY        :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenEntryText"));
                        break;
                        case TOKEN_TAB_STOP     :
                            nParamCount += 3;
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenTabStop"));
                        break;
                        case TOKEN_TEXT         :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenText"));
                            nParamCount += 1;
                        break;
                        case TOKEN_PAGE_NUMS    :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenPageNumber"));
                        break;
                        case TOKEN_CHAPTER_INFO :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenChapterInfo"));
                        {
                            aToken.nChapterFormat;
                        }
                        break;
                        case TOKEN_LINK_START   :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenHyperlinkStart"));
                        break;
                        case TOKEN_LINK_END     :
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenHyperlinkEnd"));
                        break;
                        case TOKEN_AUTHORITY :
                        {
                            sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                "TokenBibliographyDataField"));
                            ToxAuthorityField eField = (ToxAuthorityField)aToken.nAuthorityField;
                        }
                        break;
                    }
                     beans::PropertyValues aPropVals(nParamCount);
                     beans::PropertyValue* pPropValArr = aPropVals.getArray();
                    pPropValArr[0].Name = C2U("TokenType");
                    pPropValArr[0].Value <<= OUString(sTokenType);
                    pPropValArr[1].Name = C2U("CharacterStyleName");
                    pPropValArr[1].Value <<= OUString(aToken.sCharStyleName);
                    if(TOKEN_TAB_STOP == aToken.eTokenType)
                    {
                        pPropValArr[2].Name = C2U("TabStopRightAligned");
                        BOOL bTemp = SVX_TAB_ADJUST_END == aToken.eTabAlign;
                        pPropValArr[2].Value.setValue(&bTemp, ::getBooleanCppuType());
                        pPropValArr[3].Name = C2U("TabStopFillCharacter");
                        pPropValArr[3].Value <<= OUString(aToken.cTabFillChar);
                        pPropValArr[4].Name = C2U("TabStopPosition");
                        SwTwips nTempPos = aToken.nTabStopPosition >= 0 ?
                                                        aToken.nTabStopPosition : 0;
                        nTempPos = TWIP_TO_MM100(nTempPos);
                        pPropValArr[4].Value <<= (sal_Int32)nTempPos;
                    }
                    else if(TOKEN_TEXT == aToken.eTokenType)
                    {
                        pPropValArr[2].Name = C2U("Text");
                        pPropValArr[2].Value <<= OUString(aToken.sText);
                    }
                 beans::PropertyValues* pValues = aSequPropVals.getArray();
                    pValues[nTokenIndex] = aPropVals;
                    nTokenIndex++;
                }
                aSequPropVals.realloc(nTokenIndex);

                uno::Any aFormatAccess = xIdxProps->getPropertyValue(C2U(UNO_NAME_LEVEL_FORMAT));
                DBG_ASSERT(aFormatAccess.getValueType() == ::getCppuType((uno::Reference<container::XIndexReplace>*)0),
                    "wrong property type")


                 uno::Reference< container::XIndexReplace >  xFormatAccess =
                    *(uno::Reference< container::XIndexReplace > *)aFormatAccess.getValue();
                 uno::Any aLevelProp(&aSequPropVals, ::getCppuType((uno::Sequence<beans::PropertyValues>*)0));
                xFormatAccess->replaceByIndex(nCurrentLevel, aLevelProp);
            }
        }
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_STYLES)
        {
            lcl_SetProp(xInfo, xIdxProps, "ParaStyleHeading", pForm->GetTemplate(0));
            sal_uInt16 nOffset = 0;
            sal_uInt16 nEndLevel = 2;
            switch(eCurrentTOXType.eType)
            {
                case  TOX_INDEX:
                {
                    nOffset = 1;
                    nEndLevel = 4;
                    lcl_SetProp(xInfo, xIdxProps, "ParaStyleSeparator", pForm->GetTemplate(1));
                }
                break;
                case TOX_CONTENT :
                    nEndLevel = 11;
                break;

            }
            for(sal_uInt16 i = 1; i < nEndLevel; i++)
            {
                String sPropName(C2S("ParaStyleLevel"));
                sPropName += String::CreateFromInt32( i );
                lcl_SetProp(xInfo,
                    xIdxProps,
                    ByteString(sPropName, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                    pForm->GetTemplate(i + nOffset));
            }
        }
/*
    const String&   GetAutoMarkURL() const { return sAutoMarkURL;}
    const String&   GetMainEntryCharStyle() const {return sMainEntryCharStyle;}

    String          GetAuthBrackets() const {return sAuthBrackets;}
    sal_Bool            IsAuthSequence() const {return bIsAuthSequence;}
    sal_Bool            IsSortByDocument()const {return bSortByDocument ;}

    SwTOXSortKey GetSortKey1() const {return eSortKey1;}
    SwTOXSortKey GetSortKey2() const {return eSortKey2;}
    SwTOXSortKey GetSortKey3() const {return eSortKey3;}
*/
        //
        pxIndexSectionsArr[nTOXIndex]->xDocumentIndex->update();

//#ifdef DEBUG
//      uno::Reference< frame::XStorable >  xStor(xModel, uno::UNO_QUERY);
//      String sURL("file:///e|/temp/sw/idxexample.sdw");
//   uno::Sequence< beans::PropertyValue > aArgs(0);
//      xStor->storeToURL(S2U(sURL), aArgs);
//#endif

    }
    catch(...)
    {
        DBG_ERROR("::CreateExample() - exception caught")
    }
}
/* -----------------05.07.99 09:49-------------------

 --------------------------------------------------*/
sal_Bool SwMultiTOXTabDialog::IsNoNum(SwWrtShell& rSh, const String& rName)
{
    SwTxtFmtColl* pColl = rSh.GetParaStyle(rName);
    if(pColl && pColl->GetOutlineLevel() == NO_NUMBERING)
        return sal_True;

    sal_uInt16 nId = rSh.GetPoolId(rName, GET_POOLID_TXTCOLL);
    if(nId != USHRT_MAX &&
       rSh.GetTxtCollFromPool(nId)->GetOutlineLevel() == NO_NUMBERING)
        return sal_True;

    return sal_False;
}
/* -----------------14.07.99 16:01-------------------

 --------------------------------------------------*/
class SwIndexTreeLB : public SvTreeListBox
{
    const HeaderBar* pHeaderBar;
public:
    SwIndexTreeLB(Window* pWin, const ResId& rResId) :
        SvTreeListBox(pWin, rResId), pHeaderBar(0){}

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
    void            SetHeaderBar(const HeaderBar* pHB) {pHeaderBar = pHB;}
};
/* -----------------14.07.99 16:03-------------------

 --------------------------------------------------*/
long  SwIndexTreeLB::GetTabPos( SvLBoxEntry* pEntry, SvLBoxTab* pTab)
{
    long nData = (long)pEntry->GetUserData();
    if(nData != USHRT_MAX)
    {
        sal_uInt16  nPos = pHeaderBar->GetItemRect( 101 + nData ).TopLeft().X();
        nData = nPos;
    }
    else
        nData = 0;
    nData += pTab->GetPos();
    return nData;
}
/* -----------------25.08.99 11:14-------------------

 --------------------------------------------------*/
void    SwIndexTreeLB::KeyInput( const KeyEvent& rKEvt )
{
    SvLBoxEntry* pEntry = FirstSelected();
    KeyCode aCode = rKEvt.GetKeyCode();
    sal_Bool bChanged = sal_False;
    if(pEntry)
    {
        long nLevel = (long)pEntry->GetUserData();
        if(aCode.GetCode() == KEY_ADD )
        {
            if(nLevel < MAXLEVEL - 1)
                nLevel++;
            else if(nLevel == USHRT_MAX)
                nLevel = 0;
            bChanged = sal_True;
        }
        else if(aCode.GetCode() == KEY_SUBTRACT)
        {
            if(!nLevel)
                nLevel = USHRT_MAX;
            else if(nLevel != USHRT_MAX)
                nLevel--;
            bChanged = sal_True;
        }
        if(bChanged)
        {
            pEntry->SetUserData((void*)nLevel);
            Invalidate();
        }
    }
    if(!bChanged)
        SvTreeListBox::KeyInput(rKEvt);
}

/* -----------------16.07.99 10:01-------------------

 --------------------------------------------------*/
class SwHeaderTree : public Control
{
    HeaderBar       aStylesHB;
    SwIndexTreeLB   aStylesTLB;

public:
    SwHeaderTree(Window* pParent, const ResId rResId);

    HeaderBar&      GetHeaderBar() {return aStylesHB;}
    SwIndexTreeLB&  GetTreeListBox() { return aStylesTLB;}

    virtual void    GetFocus();
};
/* -----------------16.07.99 10:11-------------------

 --------------------------------------------------*/
SwHeaderTree::SwHeaderTree(Window* pParent, const ResId rResId) :
        Control(pParent, rResId),
        aStylesHB(  this, ResId(HB_STYLES)),
        aStylesTLB( this, ResId(TLB_STYLES  ))
{
    FreeResource();
    aStylesHB.SetStyle(aStylesHB.GetStyle()|WB_BUTTONSTYLE|WB_TABSTOP|WB_BORDER);
     Size aHBSize(aStylesHB.GetSizePixel());
    aHBSize.Height() = aStylesHB.CalcWindowSizePixel().Height();
    aStylesHB.SetSizePixel(aHBSize);
    aStylesTLB.SetPosPixel(Point(0, aHBSize.Height()));
     Size aTLBSize(aStylesHB.GetSizePixel());
    aTLBSize.Height() = GetOutputSizePixel().Height() - aHBSize.Height();
    aStylesTLB.SetSizePixel(aTLBSize);
    aStylesTLB.SetHeaderBar(&aStylesHB);
}
/* -----------------25.08.99 10:38-------------------

 --------------------------------------------------*/
void    SwHeaderTree::GetFocus()
{
    Control::GetFocus();
    aStylesTLB.GrabFocus();
}
/* -----------------13.07.99 15:29-------------------

 --------------------------------------------------*/
class SwAddStylesDlg_Impl : public SfxModalDialog
{
    OKButton        aOk;
    CancelButton    aCancel;
    HelpButton      aHelp;

    GroupBox        aStylesGB;
    SwHeaderTree    aHeaderTree;
    PushButton      aLeftPB;
    PushButton      aRightPB;

    String          sHBFirst;
    String*         pStyleArr;

    DECL_LINK(OkHdl, OKButton*);
    DECL_LINK(LeftRightHdl, PushButton*);
    DECL_LINK(HeaderDragHdl, HeaderBar*);

public:
    SwAddStylesDlg_Impl(Window* pParent, SwWrtShell& rWrtSh, String rStringArr[]);
    ~SwAddStylesDlg_Impl();
};
/* -----------------13.07.99 15:39-------------------

 --------------------------------------------------*/
SwAddStylesDlg_Impl::SwAddStylesDlg_Impl(Window* pParent,
            SwWrtShell& rWrtSh, String rStringArr[]) :
    SfxModalDialog(pParent, SW_RES(DLG_ADD_IDX_STYLES)),
    pStyleArr(rStringArr),
    aHeaderTree(this, ResId(TR_HEADER   )),
    aStylesGB(  this, ResId(GB_STYLES   )),
    aOk(        this, ResId(PB_OK       )),
    aCancel(    this, ResId(PB_CANCEL   )),
    aHelp(      this, ResId(PB_HELP     )),
    aLeftPB(    this, ResId(PB_LEFT     )),
    aRightPB(   this, ResId(PB_RIGHT    )),
    sHBFirst(   ResId(ST_HB_FIRST))
{
    FreeResource();

    aOk.SetClickHdl(LINK(this, SwAddStylesDlg_Impl, OkHdl));
    aLeftPB.SetClickHdl(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));
    aRightPB.SetClickHdl(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));

    HeaderBar& rHB = aHeaderTree.GetHeaderBar();
    rHB.SetEndDragHdl(LINK(this, SwAddStylesDlg_Impl, HeaderDragHdl));

    long nWidth = rHB.GetSizePixel().Width();
    nWidth /= 14;
    nWidth--;
    rHB.InsertItem( 100, sHBFirst,  4 * nWidth );
    for(sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        rHB.InsertItem( 100 + i, String::CreateFromInt32(i), nWidth );
    rHB.Show();

    SwIndexTreeLB& rTLB = aHeaderTree.GetTreeListBox();
    rTLB.SetWindowBits(WB_CLIPCHILDREN|WB_SORT);
    //aStylesTLB.SetSelectHdl(LINK(this, SwAddStylesDlg_Impl, SelectHdl));
    rTLB.GetModel()->SetSortMode(SortAscending);
    for(i = 0; i < MAXLEVEL; ++i)
    {
        String sStyles(rStringArr[i]);
        for(sal_uInt16 nToken = 0; nToken < sStyles.GetTokenCount(TOX_STYLE_DELIMITER); nToken++)
        {
            String sTmp(sStyles.GetToken(nToken, TOX_STYLE_DELIMITER));
            SvLBoxEntry* pEntry = rTLB.InsertEntry(sTmp);
            pEntry->SetUserData((void*)i);
        }
    }
    // now the other styles
    //
    const SwTxtFmtColl *pColl   = 0;
    const sal_uInt16 nSz = rWrtSh.GetTxtFmtCollCount();

    for ( sal_uInt16 j = 0;j < nSz; ++j )
    {
        pColl = &rWrtSh.GetTxtFmtColl(j);
        if(pColl->IsDefault())
            continue;

        const String& rName = pColl->GetName();

        if(rName.Len() > 0)
        {
            SvLBoxEntry* pEntry = rTLB.First();
            sal_Bool bFound = sal_False;
            while(pEntry && !bFound)
            {
                if(rTLB.GetEntryText(pEntry) == rName)
                    bFound = sal_True;
                pEntry = rTLB.Next(pEntry);
            }
            if(!bFound)
            {
                rTLB.InsertEntry(rName)->SetUserData((void*)USHRT_MAX);
            }
        }
    }
    rTLB.GetModel()->Resort();
}

/* -----------------13.07.99 15:39-------------------

 --------------------------------------------------*/
SwAddStylesDlg_Impl::~SwAddStylesDlg_Impl()
{
}
/* -----------------13.07.99 15:39-------------------

 --------------------------------------------------*/
IMPL_LINK(SwAddStylesDlg_Impl, OkHdl, OKButton*, pBtn)
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pStyleArr[i].Erase();

    SwIndexTreeLB& rTLB = aHeaderTree.GetTreeListBox();
    SvLBoxEntry* pEntry = rTLB.First();
    while(pEntry)
    {
        long nLevel = (long)pEntry->GetUserData();
        if(nLevel != USHRT_MAX)
        {
            String sName(rTLB.GetEntryText(pEntry));
            if(pStyleArr[nLevel].Len())
                pStyleArr[nLevel] += TOX_STYLE_DELIMITER;
            pStyleArr[nLevel] += sName;
        }
        pEntry = rTLB.Next(pEntry);
    }

    //TODO write back style names
    EndDialog(RET_OK);
    return 0;
}
/* -----------------16.07.99 09:27-------------------

 --------------------------------------------------*/
IMPL_LINK(SwAddStylesDlg_Impl, HeaderDragHdl, HeaderBar*, pHB)
{
    aHeaderTree.GetTreeListBox().Invalidate();
    //pHB->SetSizePixel(pHB->CalcWindowSizePixel());
    return 0;
}
/* -----------------13.07.99 15:39-------------------

 --------------------------------------------------*/
IMPL_LINK(SwAddStylesDlg_Impl, LeftRightHdl, PushButton*, pBtn)
{
    sal_Bool bLeft = pBtn == &aLeftPB;
    SvLBoxEntry* pEntry = aHeaderTree.GetTreeListBox().FirstSelected();
    if(pEntry)
    {
        long nLevel = (long)pEntry->GetUserData();
        if(bLeft)
        {
            if(!nLevel)
                nLevel = USHRT_MAX;
            else if(nLevel != USHRT_MAX)
                nLevel--;
        }
        else
        {
            if(nLevel < MAXLEVEL - 1)
                nLevel++;
            else if(nLevel == USHRT_MAX)
                nLevel = 0;
        }
        pEntry->SetUserData((void*)nLevel);
        aHeaderTree.GetTreeListBox().Invalidate();
    }
    return 0;
}

/*-- 14.06.99 13:11:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTOXSelectTabPage::SwTOXSelectTabPage(Window* pParent, const SfxItemSet& rAttrSet) :
    SfxTabPage(pParent, SW_RES(TP_TOX_SELECT), rAttrSet),
    aTypeFT(            this, ResId(FT_TYPE             )),
    aTypeLB(            this, ResId(LB_TYPE             )),
    aReadOnlyCB(        this, ResId(CB_READONLY         )),
    aTitleFT(           this, ResId(FT_TITLE            )),
    aTitleED(           this, ResId(ED_TITLE            )),
    aTypeTitleGB(       this, ResId(GB_TYPETITLE        )),
    aAreaLB(            this, ResId(LB_AREA             )),
    aLevelFT(           this, ResId(FT_LEVEL            )),
    aLevelNF(           this, ResId(NF_LEVEL            )),
    aAreaGB(            this, ResId(GB_AREA             )),
    aFromHeadingsCB(    this, ResId(CB_FROMHEADINGS     )),
    aChapterDlgPB(      this, ResId(PB_CHAPTERDLG       )),
    aAddStylesCB(       this, ResId(CB_ADDSTYLES        )),
    aAddStylesPB(       this, ResId(PB_ADDSTYLES        )),
    aFromTablesCB(      this, ResId(CB_FROMTABLES       )),
    aFromFramesCB(      this, ResId(CB_FROMFRAMES       )),
    aFromGraphicsCB(    this, ResId(CB_FROMGRAPHICS     )),
    aFromOLECB(         this, ResId(CB_FROMOLE          )),
    aLevelFromChapterCB(this, ResId(CB_LEVELFROMCHAPTER )),
    aFromCaptionsRB(    this, ResId(RB_FROMCAPTIONS     )),
    aFromObjectNamesRB( this, ResId(RB_FROMOBJECTNAMES  )),
    aCaptionSequenceFT( this, ResId(FT_CAPTIONSEQUENCE  )),
    aCaptionSequenceLB( this, ResId(LB_CAPTIONSEQUENCE  )),
    aDisplayTypeFT(     this, ResId(FT_DISPLAYTYPE      )),
    aDisplayTypeLB(     this, ResId(LB_DISPLAYTYPE      )),
    aTOXMarksCB(        this, ResId(CB_TOXMARKS         )),
    aCreateFromGB(      this, ResId(GB_CREATEFROM       )),
    aCollectSameCB(     this, ResId(CB_COLLECTSAME      )),
    aUseFFCB(           this, ResId(CB_USEFF            )),
    aUseDashCB(         this, ResId(CB_USE_DASH         )),
    aCaseSensitiveCB(   this, ResId(CB_CASESENSITIVE    )),
    aInitialCapsCB(     this, ResId(CB_INITIALCAPS      )),
    aKeyAsEntryCB(      this, ResId(CB_KEYASENTRY       )),
    aFromFileCB(        this, ResId(CB_FROMFILE         )),
    aAutoMarkPB(        this, ResId(MB_AUTOMARK         )),
//  aFilePB(            this, ResId(PB_FILE             )),
//  aCreateAutoMarkPB(  this, ResId(PB_CREATE_AUTOMARK  )),
//  aEditAutoMarkPB(    this, ResId(PB_EDIT_AUTOMARK    )),
    aIdxOptionsGB(      this, ResId(GB_IDXOPTIONS       )),
    aFromNames(         ResId(RES_SRCTYPES              )),
    aFromObjCLB(        this, ResId(CLB_FROMOBJ         )),
    aFromObjGB(         this, ResId(GB_FROMOBJ          )),
    aSequenceCB(        this, ResId(CB_SEQUENCE         )),
    aBracketFT(         this, ResId(FT_BRACKET          )),
    aBracketLB(         this, ResId(LB_BRACKET          )),
    aAuthorityFormatGB( this, ResId(GB_AUTHORITY        )),
    sAddStyleUser(ResId(ST_USER_ADDSTYLE)),
    sAutoMarkType(ResId(ST_AUTOMARK_TYPE)),
    bFirstCall(sal_True)
{
    FreeResource();
    sAddStyleContent = aAddStylesCB.GetText();

    aCBLeftPos1 = aFromHeadingsCB.GetPosPixel();
    aCBLeftPos2 = aAddStylesCB.GetPosPixel();
    aCBLeftPos3 = aTOXMarksCB.GetPosPixel();

    ResStringArray& rNames = aFromNames.GetNames();
    for(sal_uInt16 i = 0; i < rNames.Count(); i++)
    {
        aFromObjCLB.InsertEntry(rNames.GetString(i));
        aFromObjCLB.SetEntryData( i, (void*)rNames.GetValue(i) );
    }
    aFromObjCLB.SetHelpId(HID_OLE_CHECKLB);

    SetExchangeSupport();
    aTypeLB.SetSelectHdl(LINK(this, SwTOXSelectTabPage, TOXTypeHdl));

    aAddStylesPB.SetClickHdl(LINK(this, SwTOXSelectTabPage, AddStylesHdl));
    aChapterDlgPB.SetClickHdl(LINK(this, SwTOXSelectTabPage, ChapterHdl));

    PopupMenu*  pMenu = aAutoMarkPB.GetPopupMenu();
    pMenu->SetActivateHdl(LINK(this, SwTOXSelectTabPage, MenuEnableHdl));
    pMenu->SetSelectHdl(LINK(this, SwTOXSelectTabPage, MenuExecuteHdl));

//  aFilePB.SetClickHdl(LINK(this, SwTOXSelectTabPage, AutoMarkHdl));
//  aCreateAutoMarkPB.SetClickHdl(LINK(this, SwTOXSelectTabPage, CreateEditAutoMarkHdl));
//  aEditAutoMarkPB.SetClickHdl(LINK(this, SwTOXSelectTabPage, CreateEditAutoMarkHdl));

    Link aLk =  LINK(this, SwTOXSelectTabPage, CheckBoxHdl);
    aAddStylesCB    .SetClickHdl(aLk);
    aFromHeadingsCB .SetClickHdl(aLk);
    aTOXMarksCB     .SetClickHdl(aLk);
    aFromFileCB     .SetClickHdl(aLk);
    aCollectSameCB  .SetClickHdl(aLk);
    aUseFFCB        .SetClickHdl(aLk);
    aUseDashCB      .SetClickHdl(aLk);
    aInitialCapsCB  .SetClickHdl(aLk);
    aKeyAsEntryCB   .SetClickHdl(aLk);

    Link aModifyLk = LINK(this, SwTOXSelectTabPage, ModifyHdl);
    aTitleED.SetModifyHdl(aModifyLk);
    aLevelNF.SetModifyHdl(aModifyLk);

    aLk =  LINK(this, SwTOXSelectTabPage, RadioButtonHdl);
    aFromCaptionsRB.SetClickHdl(aLk);
    aFromObjectNamesRB.SetClickHdl(aLk);
    RadioButtonHdl(&aFromCaptionsRB);

    aTypeLB.SelectEntryPos(0);
    aTitleED.SaveValue();
}
/*-- 14.06.99 13:11:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTOXSelectTabPage::~SwTOXSelectTabPage()
{
}
/* -----------------21.10.99 17:03-------------------

 --------------------------------------------------*/
void SwTOXSelectTabPage::SetWrtShell(SwWrtShell& rSh)
{
    sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    if(nUserTypeCount > 1)
    {
        //insert all new user indexes names after the standard user index
        sal_uInt16 nPos = aTypeLB.GetEntryPos((void*)(sal_uInt32)TO_USER);
        nPos++;
        for(sal_uInt16 nUser = 1; nUser < nUserTypeCount; nUser++)
        {
            nPos = aTypeLB.InsertEntry(rSh.GetTOXType(TOX_USER, nUser)->GetTypeName(), nPos);
            sal_uInt32 nEntryData = nUser << 8;
            nEntryData |= TO_USER;
            aTypeLB.SetEntryData(nPos, (void*)nEntryData);
        }
    }
}
/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
sal_Bool SwTOXSelectTabPage::FillItemSet( SfxItemSet& )
{
    return sal_True;
}
/* -----------------25.08.99 14:31-------------------

 --------------------------------------------------*/
long lcl_TOXTypesToUserData(CurTOXType eType)
{
    sal_uInt16 nRet;
    switch(eType.eType)
    {
        case TOX_INDEX       : nRet = TO_INDEX;     break;
        case TOX_USER        :
        {
            nRet = eType.nIndex << 8;
            nRet |= TO_USER;
        }
        break;
        case TOX_CONTENT     : nRet = TO_CONTENT;   break;
        case TOX_ILLUSTRATIONS:nRet = TO_ILLUSTRATION; break;
        case TOX_OBJECTS     : nRet = TO_OBJECT;    break;
        case TOX_TABLES      : nRet = TO_TABLE;     break;
        case TOX_AUTHORITIES : nRet = TO_AUTHORITIES; break;
    }
    return nRet;
}
//-----------------------------------------------------------------
void SwTOXSelectTabPage::SelectType(TOXTypes eSet)
{
    CurTOXType eCurType = {eSet, 0};

    long nData = lcl_TOXTypesToUserData(eCurType);
    aTypeLB.SelectEntryPos(aTypeLB.GetEntryPos((void*)nData));
    aTypeFT.Enable(sal_False);
    aTypeLB.Enable(sal_False);
    TOXTypeHdl(&aTypeLB);
}

/*-- 14.06.99 13:10:45---------------------------------------------------

  -----------------------------------------------------------------------*/
//-----------------------------------------------------------------
CurTOXType lcl_UserData2TOXTypes(sal_uInt16 nData)
{
    CurTOXType eRet;
    eRet.nIndex = 0;
    switch(nData&0xff)
    {
        case TO_INDEX       : eRet.eType = TOX_INDEX;       break;
        case TO_USER        :
        {
            eRet.eType = TOX_USER;
            eRet.nIndex  = (nData&0xff00) >> 8;
        }
        break;
        case TO_CONTENT     : eRet.eType = TOX_CONTENT;     break;
        case TO_ILLUSTRATION: eRet.eType = TOX_ILLUSTRATIONS; break;
        case TO_OBJECT      : eRet.eType = TOX_OBJECTS;     break;
        case TO_TABLE       : eRet.eType = TOX_TABLES;      break;
        case TO_AUTHORITIES : eRet.eType = TOX_AUTHORITIES; break;
        default: DBG_ERROR("what a type?");
    }
    return eRet;
}
/* -----------------02.09.99 08:16-------------------

 --------------------------------------------------*/
void    SwTOXSelectTabPage::ApplyTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    aReadOnlyCB.Check(rDesc.IsReadonly());
    if(aTitleED.GetText() == aTitleED.GetSavedValue())
    {
        if(rDesc.GetTitle())
            aTitleED.SetText(*rDesc.GetTitle());
        else
            aTitleED.SetText(aEmptyStr);
        aTitleED.SaveValue();
    }

    aAreaLB.SelectEntryPos(rDesc.IsFromChapter() ? 1 : 0);

    if(aCurType.eType != TOX_INDEX)
        aLevelNF.SetValue(rDesc.GetLevel());   //content, user

    sal_uInt16 nCreateType = rDesc.GetContentOptions();

    //user + content
    sal_Bool bHasStyleNames = sal_False;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        if(rDesc.GetStyleNames(i).Len())
        {
            bHasStyleNames = sal_True;
            break;
        }
    aAddStylesCB.Check(bHasStyleNames && (nCreateType & TOX_TEMPLATE));

    aFromOLECB.Check(nCreateType&TOX_OLE);
    aFromTablesCB.Check(nCreateType&TOX_TABLE);
    aFromGraphicsCB.Check(nCreateType&TOX_GRAPHIC);
    aFromFramesCB.Check(nCreateType&TOX_FRAME);

    aLevelFromChapterCB.Check(rDesc.IsLevelFromChapter());

    //all but illustration and table
    aTOXMarksCB.Check(nCreateType&TOX_MARK );

    //content
    if(TOX_CONTENT == aCurType.eType)
    {
        aFromHeadingsCB.Check(nCreateType&TOX_OUTLINELEVEL);
        aChapterDlgPB.Enable(aFromHeadingsCB.IsChecked());
        aAddStylesCB.SetText(sAddStyleContent);
        aAddStylesPB.Enable(aAddStylesCB.IsChecked());
    }
    //index only
    else if(TOX_INDEX == aCurType.eType)
    {
        sal_uInt16 nIndexOptions = rDesc.GetIndexOptions();
        aCollectSameCB.     Check(nIndexOptions&TOI_SAME_ENTRY);
        aUseFFCB.           Check(nIndexOptions&TOI_FF);
        aUseDashCB.         Check(nIndexOptions&TOI_DASH);
        if(aUseFFCB.IsChecked())
            aUseDashCB.Enable(sal_False);
        else if(aUseDashCB.IsChecked())
            aUseFFCB.Enable(sal_False);

        aCaseSensitiveCB.   Check(nIndexOptions&TOI_CASE_SENSITIVE);
        aInitialCapsCB.     Check(nIndexOptions&TOI_INITIAL_CAPS);
        aKeyAsEntryCB.      Check(nIndexOptions&TOI_KEY_AS_ENTRY);
    }
    else if(TOX_ILLUSTRATIONS == aCurType.eType ||
        TOX_TABLES == aCurType.eType)
    {
        aFromObjectNamesRB.Check(rDesc.IsCreateFromObjectNames());
        aFromCaptionsRB.Check(!rDesc.IsCreateFromObjectNames());
        aCaptionSequenceLB.SelectEntry(rDesc.GetSequenceName());
        aDisplayTypeLB.SelectEntryPos(rDesc.GetCaptionDisplay());
        RadioButtonHdl(&aFromCaptionsRB);

    }
    else if(TOX_OBJECTS == aCurType.eType)
    {
        long nOLEData = rDesc.GetOLEOptions();
        for(sal_uInt16 i = 0; i < aFromObjCLB.GetEntryCount(); i++)
        {
            long nData = (long)aFromObjCLB.GetEntryData(i);
            aFromObjCLB.CheckEntryPos(i, 0 != (nData & nOLEData));
        }
    }
    else if(TOX_AUTHORITIES == aCurType.eType)
    {
        aBracketLB.SelectEntry(rDesc.GetAuthBrackets());
        aSequenceCB.Check(rDesc.IsAuthSequence());
    }
    aAutoMarkPB.Enable(aFromFileCB.IsChecked());
//  aCreateAutoMarkPB.Enable(aFromFileCB.IsChecked());
//  aEditAutoMarkPB.Enable(aFromFileCB.IsChecked() && sAutoMarkURL.Len());

    for(i = 0; i < MAXLEVEL; i++)
        aStyleArr[i] = rDesc.GetStyleNames(i);
}
/* -----------------09.09.99 11:57-------------------

 --------------------------------------------------*/
void SwTOXSelectTabPage::FillTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    rDesc.SetTitle(aTitleED.GetText());
    rDesc.SetFromChapter(1 == aAreaLB.GetSelectEntryPos());
    sal_uInt16 nContentOptions = 0;
    if(aTOXMarksCB.IsVisible() && aTOXMarksCB.IsChecked())
        nContentOptions |= TOX_MARK;

    sal_uInt16 nIndexOptions = rDesc.GetIndexOptions()&TOI_ALPHA_DELIMITTER;
    switch(rDesc.GetTOXType())
    {
        case TOX_CONTENT:
            if(aFromHeadingsCB.IsChecked())
                nContentOptions |= TOX_OUTLINELEVEL;
        break;
        case TOX_USER:
        {
            rDesc.SetTOUName(aTypeLB.GetSelectEntry());

            if(aFromOLECB.IsChecked())
                nContentOptions |= TOX_OLE;
            if(aFromTablesCB.IsChecked())
                nContentOptions |= TOX_TABLE;
            if(aFromFramesCB.IsChecked())
                nContentOptions |= TOX_FRAME;
            if(aFromGraphicsCB.IsChecked())
                nContentOptions |= TOX_GRAPHIC;
        }
        break;
        case  TOX_INDEX:
        {
            nContentOptions = TOX_MARK;

            if(aCollectSameCB.IsChecked())
                nIndexOptions |= TOI_SAME_ENTRY;
            if(aUseFFCB.IsChecked())
                nIndexOptions |= TOI_FF;
            if(aUseDashCB.IsChecked())
                nIndexOptions |= TOI_DASH;
            if(aCaseSensitiveCB.IsChecked())
                nIndexOptions |= TOI_CASE_SENSITIVE;
            if(aInitialCapsCB.IsChecked())
                nIndexOptions |= TOI_INITIAL_CAPS;
            if(aKeyAsEntryCB.IsChecked())
                nIndexOptions |= TOI_KEY_AS_ENTRY;
            if(aFromFileCB.IsChecked())
                rDesc.SetAutoMarkURL(sAutoMarkURL);
            else
                rDesc.SetAutoMarkURL(aEmptyStr);
        }
        break;
        case TOX_ILLUSTRATIONS:
        case TOX_TABLES :
            rDesc.SetCreateFromObjectNames(aFromObjectNamesRB.IsChecked());
            rDesc.SetSequenceName(aCaptionSequenceLB.GetSelectEntry());
            rDesc.SetCaptionDisplay((SwCaptionDisplay)aDisplayTypeLB.GetSelectEntryPos());
        break;
        case TOX_OBJECTS:
        {
            long nOLEData = 0;
            for(sal_uInt16 i = 0; i < aFromObjCLB.GetEntryCount(); i++)
            {
                if(aFromObjCLB.IsChecked(i))
                {
                    long nData = (long)aFromObjCLB.GetEntryData(i);
                    nOLEData |= nData;
                }
            }
            rDesc.SetOLEOptions(nOLEData);
        }
        break;
        case TOX_AUTHORITIES:
        {
            rDesc.SetAuthBrackets(aBracketLB.GetSelectEntry());
            rDesc.SetAuthSequence(aSequenceCB.IsChecked());
        }
        break;
    }

    rDesc.SetLevelFromChapter(  aLevelFromChapterCB.IsVisible() &&
                                aLevelFromChapterCB.IsChecked());
    if(aTOXMarksCB.IsChecked() && aTOXMarksCB.IsVisible())
        nContentOptions |= TOX_MARK;
    if(aFromHeadingsCB.IsChecked() && aFromHeadingsCB.IsVisible())
        nContentOptions |= TOX_OUTLINELEVEL;
    if(aAddStylesCB.IsChecked() && aAddStylesCB.IsVisible())
        nContentOptions |= TOX_TEMPLATE;

    rDesc.SetContentOptions(nContentOptions);
    rDesc.SetIndexOptions(nIndexOptions);
    rDesc.SetLevel(aLevelNF.GetValue());

    rDesc.SetReadonly(aReadOnlyCB.IsChecked());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        rDesc.SetStyleNames(aStyleArr[i], i);
}
/* -----------------05.07.99 15:09-------------------

 --------------------------------------------------*/
void SwTOXSelectTabPage::Reset( const SfxItemSet& )
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    SwWrtShell& rSh = pTOXDlg->GetWrtShell();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    long nData = lcl_TOXTypesToUserData(aCurType);
    aTypeLB.SelectEntryPos(aTypeLB.GetEntryPos((void*)nData));

    sAutoMarkURL = INetURLObject::decode( rSh.GetTOIAutoMarkURL(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 );
    aFromFileCB.Check(sAutoMarkURL.Len());

    aCaptionSequenceLB.Clear();
    sal_uInt16 i, nCount = rSh.GetFldTypeCount(RES_SETEXPFLD);
    for (i = 0; i < nCount; i++)
    {
        SwFieldType *pType = rSh.GetFldType( i, RES_SETEXPFLD );
        if( pType->Which() == RES_SETEXPFLD &&
            ((SwSetExpFieldType *) pType)->GetType() & GSE_SEQ )
            aCaptionSequenceLB.InsertEntry(pType->GetName());
    }

    if(pTOXDlg->IsTOXEditMode())
    {
        aTypeFT.Enable(sal_False);
        aTypeLB.Enable(sal_False);
    }
    TOXTypeHdl(&aTypeLB);
    CheckBoxHdl(&aAddStylesCB);
}
/*-- 14.06.99 13:10:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTOXSelectTabPage::ActivatePage( const SfxItemSet& )
{
    //nothing to do
}
/*-- 14.06.99 13:11:00---------------------------------------------------

  -----------------------------------------------------------------------*/
int SwTOXSelectTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if(pSet)
        pSet->Put(SfxUInt16Item(FN_PARAM_TOX_TYPE,
            (sal_uInt16)(long)aTypeLB.GetEntryData( aTypeLB.GetSelectEntryPos() )));
    sal_uInt16 nUserData = (sal_uInt16)(long)aTypeLB.GetEntryData( aTypeLB.GetSelectEntryPos() );
    FillTOXDescription();
    return LEAVE_PAGE;
}

/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
SfxTabPage* SwTOXSelectTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet)
{
    return new SwTOXSelectTabPage(pParent, rAttrSet);
}
/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, TOXTypeHdl,   ListBox*, pBox)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    if(!bFirstCall)
    {
        // save current values into the proper TOXDescription
        FillTOXDescription();
    }
    bFirstCall = sal_False;
    const long nType = (long)pBox->GetEntryData( pBox->GetSelectEntryPos() );
    CurTOXType eCurType = lcl_UserData2TOXTypes(nType);
    pTOXDlg->SetCurrentTOXType(eCurType);

    aAreaLB.Show(nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT));
    aLevelFT.Show(nType & (TO_CONTENT));
    aLevelNF.Show(nType & (TO_CONTENT));
    aLevelFromChapterCB.Show(nType & (TO_USER));
    aAreaGB.Show(nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT));

    aFromHeadingsCB.Show(nType & (TO_CONTENT));
    aChapterDlgPB.Show(nType & (TO_CONTENT));
    aAddStylesCB.Show(nType & (TO_CONTENT|TO_USER));
    aAddStylesPB.Show(nType & (TO_CONTENT|TO_USER));

    aFromTablesCB.Show(nType & (TO_USER));
    aFromFramesCB.Show(nType & (TO_USER));
    aFromGraphicsCB.Show(nType & (TO_USER));
    aFromOLECB.Show(nType & (TO_USER));

    aFromCaptionsRB.Show(nType & (TO_ILLUSTRATION|TO_TABLE));
    aFromObjectNamesRB.Show(nType & (TO_ILLUSTRATION|TO_TABLE));

    aTOXMarksCB.Show(nType & (TO_CONTENT|TO_USER));

    aCreateFromGB.Show(nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_TABLE));
    aCaptionSequenceFT.Show(nType & (TO_ILLUSTRATION|TO_TABLE));
    aCaptionSequenceLB.Show(nType & (TO_ILLUSTRATION|TO_TABLE));
    aDisplayTypeFT.Show(nType & (TO_ILLUSTRATION|TO_TABLE));
    aDisplayTypeLB.Show(nType & (TO_ILLUSTRATION|TO_TABLE));

    aSequenceCB.Show(nType & TO_AUTHORITIES);
    aBracketFT.Show(nType & TO_AUTHORITIES);
    aBracketLB.Show(nType & TO_AUTHORITIES);
    aAuthorityFormatGB.Show(nType & TO_AUTHORITIES);

    String sStr;
    if(nType & TO_CONTENT)
    {
         Point aPos(aAddStylesPB.GetPosPixel());
        aPos.X() = aChapterDlgPB.GetPosPixel().X();
        aAddStylesPB.SetPosPixel(aPos);
    }
    else if( nType & TO_ILLUSTRATION )
        aCaptionSequenceLB.SelectEntry( GetDocPoolNm( RES_POOLCOLL_LABEL_ABB, sStr ));
    else if( nType & TO_TABLE )
        aCaptionSequenceLB.SelectEntry( GetDocPoolNm( RES_POOLCOLL_LABEL_TABLE, sStr ));
    else if( nType & TO_USER )
    {
        aAddStylesCB.SetText(sAddStyleUser);
        // move left!
         Point aPos(aAddStylesPB.GetPosPixel());
        aPos.X() = aChapterDlgPB.GetPosPixel().X();
        aPos.X() -= 2 * aAddStylesPB.GetSizePixel().Width();
        aAddStylesPB.SetPosPixel(aPos);
    }

    aCollectSameCB.Show(nType & TO_INDEX);
    aUseFFCB.Show(nType & TO_INDEX);
    aUseDashCB.Show(nType & TO_INDEX);
    aCaseSensitiveCB.Show(nType & TO_INDEX);
    aInitialCapsCB.Show(nType & TO_INDEX);
    aKeyAsEntryCB.Show(nType & TO_INDEX);
    aFromFileCB.Show(nType & TO_INDEX);
    aAutoMarkPB.Show(nType & TO_INDEX);
//  aCreateAutoMarkPB.Show(nType &TO_INDEX);
//  aEditAutoMarkPB.Show(nType & TO_INDEX);

    aIdxOptionsGB.Show(nType & TO_INDEX);

    //object index
    aFromObjCLB.Show(nType & TO_OBJECT);
    aFromObjGB.Show(nType & TO_OBJECT);

    //move controls
    aAddStylesCB.SetPosPixel(nType & TO_USER ? aCBLeftPos1 : aCBLeftPos2);
 Point aPBPos(aAddStylesPB.GetPosPixel());
    aPBPos.Y() = nType & TO_USER ? aCBLeftPos1.Y() : aCBLeftPos2.Y();
    aAddStylesPB.SetPosPixel(aPBPos);
    aTOXMarksCB.SetPosPixel(nType & TO_USER ? aCBLeftPos2 : aCBLeftPos3);
    //set control values from the proper TOXDescription
    {
        ApplyTOXDescription();
    }
    ModifyHdl(0);
    return 0;
}
/* -----------------30.11.99 12:48-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, ModifyHdl, void*, EMPTYARG)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    if(pTOXDlg)
    {
        FillTOXDescription();
        pTOXDlg->CreateOrUpdateExample(pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_SELECT);
    }
    return 0;
}
/* -----------------05.07.99 10:13-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, CheckBoxHdl,  CheckBox*, pBox )
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    if(TOX_CONTENT == aCurType.eType)
    {
        //at least one of the three CheckBoxes must be checked
        if(!aAddStylesCB.IsChecked() && !aFromHeadingsCB.IsChecked() && !aTOXMarksCB.IsChecked())
        {
            //TODO: InfoBox?
            pBox->Check(sal_True);
        }
        aAddStylesPB.Enable(aAddStylesCB.IsChecked());
        aChapterDlgPB.Enable(aFromHeadingsCB.IsChecked());
    }
    if(TOX_USER == aCurType.eType)
    {
        aAddStylesPB.Enable(aAddStylesCB.IsChecked());
    }
    else if(TOX_INDEX == aCurType.eType)
    {
        aAutoMarkPB.Enable(aFromFileCB.IsChecked());
//      aCreateAutoMarkPB.Enable(aFromFileCB.IsChecked());
//      aEditAutoMarkPB.Enable(aFromFileCB.IsChecked() && sAutoMarkURL.Len());

        aUseFFCB.Enable(aCollectSameCB.IsChecked() && !aUseDashCB.IsChecked());
        aUseDashCB.Enable(aCollectSameCB.IsChecked() && !aUseFFCB.IsChecked());
        aCaseSensitiveCB.Enable(aCollectSameCB.IsChecked());
    }
    ModifyHdl(0);
    return 0;
};
/* -----------------14.07.99 14:21-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, RadioButtonHdl, RadioButton*, pButton )
{
    sal_Bool bEnable = aFromCaptionsRB.IsChecked();
    aCaptionSequenceFT.Enable(bEnable);
    aCaptionSequenceLB.Enable(bEnable);
    aDisplayTypeFT.Enable(bEnable);
    aDisplayTypeLB.Enable(bEnable);
    ModifyHdl(0);
    return 0;
}
/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, TOXAreaHdl,   ListBox*, pBox)
{
    DBG_WARNING("not implemented")
    switch((long)pBox->GetEntryData( pBox->GetSelectEntryPos() ))
    {
        case AREA_DOCUMENT  : break;
        case AREA_CHAPTER   : break;
    }
    return 0;
}
/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, ChapterHdl,   PushButton*, pButton)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    SwWrtShell& rSh = pTOXDlg->GetWrtShell();

    SfxItemSet aTmp(rSh.GetView().GetPool(), FN_PARAM_1, FN_PARAM_1);
    SwOutlineTabDialog* pDlg = new SwOutlineTabDialog(pButton, &aTmp, rSh);
    if(RET_OK == pDlg->Execute())
    {
        CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
        SwForm* pForm = ((SwMultiTOXTabDialog*)GetTabDialog())->GetForm(aCurType);
        // jetzt muss ueberprueft werden, ob dem sdbcx::Index Ueberschriftenvorlagen
        // zugewiesen wurden
        String sStr;
        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {
            sal_Bool bNum = !SwMultiTOXTabDialog::IsNoNum(rSh, pForm->GetTemplate( i + 1 ));
            if(bNum)
            {
                //es gibt getrennte Resourcebereiche fuer die Inhaltsverzeichnisse
                if(i < 5)
                    GetDocPoolNm( RES_POOLCOLL_TOX_CNTNT1 + i, sStr );
                else
                    GetDocPoolNm( RES_POOLCOLL_TOX_CNTNT6 + i - 5, sStr );
                pForm->SetTemplate( i + 1, sStr );
            }
        }

    }
    delete pDlg;
    return 0;
}
/* -----------------14.06.99 13:10-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXSelectTabPage, AddStylesHdl, PushButton*, pButton)
{
    SwAddStylesDlg_Impl* pDlg = new SwAddStylesDlg_Impl(pButton,
        ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell(),
        aStyleArr);
    pDlg->Execute();
    delete pDlg;
    ModifyHdl(0);
    return 0;
}
/* -----------------------------19.01.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/

IMPL_LINK(SwTOXSelectTabPage, MenuEnableHdl, Menu*, pMenu)
{
    pMenu->EnableItem(MN_AUTOMARK_EDIT, sAutoMarkURL.Len());
    return 0;
}

IMPL_LINK(SwTOXSelectTabPage, MenuExecuteHdl, Menu*, pMenu)
{
    const String sSaveAutoMarkURL = sAutoMarkURL;
    switch(pMenu->GetCurItemId())
    {
        case  MN_AUTOMARK_OPEN:
            sAutoMarkURL = lcl_CreateAutoMarkFileDlg( &aAutoMarkPB,
                                    sAutoMarkURL, sAutoMarkType, sal_True);
        break;
        case  MN_AUTOMARK_NEW :
            sAutoMarkURL = lcl_CreateAutoMarkFileDlg( &aAutoMarkPB,
                                    sAutoMarkURL, sAutoMarkType, sal_False);
            if( !sAutoMarkURL.Len() )
                break;
        //no break
        case  MN_AUTOMARK_EDIT:
        {
            sal_Bool bNew = pMenu->GetCurItemId()== MN_AUTOMARK_NEW;
            SwAutoMarkDlg_Impl* pAutoMarkDlg = new SwAutoMarkDlg_Impl(
                    &aAutoMarkPB, sAutoMarkURL, sAutoMarkType, bNew );

            if( RET_OK != pAutoMarkDlg->Execute() && bNew )
                sAutoMarkURL = sSaveAutoMarkURL;
            delete pAutoMarkDlg;

            sAutoMarkURL = URIHelper::SmartRelToAbs( sAutoMarkURL );
        }
        break;
    }
    return 0;
}

/* -----------------16.06.99 10:46-------------------

 --------------------------------------------------*/
class SwTOXEdit : public Edit
{
    SwFormToken aFormToken;
    Link        aPrevNextControlLink;
    Link        aGetFocusLink;
       sal_Bool     bNextControl;
    SwTokenWindow* m_pParent;
public:
    SwTOXEdit( Window* pParent, SwTokenWindow* pTokenWin,
                const SwFormToken& aToken)
        : Edit( pParent, WB_BORDER|WB_TABSTOP|WB_CENTER),
        bNextControl(sal_False),
        aFormToken(aToken),
        m_pParent( pTokenWin )
    {
        SetHelpId( HID_TOX_ENTRY_EDIT );
    }

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    GetFocus();
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    sal_Bool    IsNextControl() const {return bNextControl;}
    void SetPrevNextLink( const Link& rLink )   {aPrevNextControlLink = rLink;}
    void SetGetFocusLink(const Link& rLink)     {aGetFocusLink = rLink;}

    const SwFormToken&  GetFormToken()
        {
            aFormToken.sText = GetText();
            return aFormToken;
        }

    void    SetCharStyleName(const String& rSet, sal_uInt16 nPoolId)
        {
            aFormToken.sCharStyleName = rSet;
            aFormToken.nPoolId = nPoolId;
        }

    void    AdjustSize();
};

//---------------------------------------------------
void    SwTOXEdit::RequestHelp( const HelpEvent& rHEvt )
{
    if(!m_pParent->CreateQuickHelp(this, aFormToken, rHEvt))
        Edit::RequestHelp(rHEvt);
}
//---------------------------------------------------
void    SwTOXEdit::KeyInput( const KeyEvent& rKEvt )
{
    const Selection& rSel = GetSelection();
    sal_uInt16 nTextLen = GetText().Len();
    if(rSel.A() == rSel.B() &&
        !rSel.A() || rSel.A() == nTextLen )
    {
        sal_Bool bCall = sal_False;
        KeyCode aCode = rKEvt.GetKeyCode();
        if(aCode.GetCode() == KEY_RIGHT && rSel.A() == nTextLen)
        {
            bNextControl = sal_True;
            bCall = sal_True;
        }
        else if(aCode.GetCode() == KEY_LEFT && !rSel.A() )
        {
            bNextControl = sal_False;
            bCall = sal_True;
        }

        if(bCall && aPrevNextControlLink.IsSet())
            aPrevNextControlLink.Call(this);

    }
    Edit::KeyInput(rKEvt);
}
/* -----------------16.07.99 12:41-------------------

 --------------------------------------------------*/
void SwTOXEdit::AdjustSize()
{
     Size aSize(GetSizePixel());
     Size aTextSize(GetTextWidth(GetText()), GetTextHeight());
    aTextSize = LogicToPixel(aTextSize);
    aSize.Width() = aTextSize.Width() + EDIT_MINWIDTH;
    SetSizePixel(aSize);
}

//---------------------------------------------------
void    SwTOXEdit::GetFocus()
{
    if(aGetFocusLink.IsSet())
        aGetFocusLink.Call(this);
    Edit::GetFocus();
}

//---------------------------------------------------
//---------------------------------------------------
class SwTOXButton : public PushButton
{
    SwFormToken aFormToken;
    Link        aPrevNextControlLink;
    Link        aGetFocusLink;
    sal_Bool        bNextControl;
    SwTokenWindow* m_pParent;
public:
    SwTOXButton( Window* pParent, SwTokenWindow* pTokenWin,
                const SwFormToken& rToken)
        : PushButton(pParent, WB_BORDER|WB_TABSTOP),
        bNextControl(sal_False),
        aFormToken(rToken),
        m_pParent(pTokenWin)
    {
        SetHelpId(HID_TOX_ENTRY_BUTTON);
    }

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    GetFocus();
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    sal_Bool IsNextControl() const          {return bNextControl;}
    void SetPrevNextLink(const Link& rLink) {aPrevNextControlLink = rLink;}
    void SetGetFocusLink(const Link& rLink) {aGetFocusLink = rLink;}
    const SwFormToken& GetFormToken() const {return aFormToken;}

    void SetCharStyleName(const String& rSet, sal_uInt16 nPoolId)
        {
            aFormToken.sCharStyleName = rSet;
            aFormToken.nPoolId = nPoolId;
        }

    void SetTabPosition(SwTwips nSet)
        { aFormToken.nTabStopPosition = nSet; }

    void SetFillChar( sal_Unicode cSet )
        { aFormToken.cTabFillChar = cSet; }

    void SetTabAlign(SvxTabAdjust eAlign)
         {  aFormToken.eTabAlign = eAlign;}

    void SetChapterInfo(sal_uInt16 nSet) { aFormToken.nChapterFormat = nSet;}
    sal_uInt16 GetChapterInfo() const{ return aFormToken.nChapterFormat;}

    void SetLinkEnd()
        {
            DBG_ASSERT(TOKEN_LINK_START == aFormToken.eTokenType,
                                    "call SetLinkEnd for link start only!")
            aFormToken.eTokenType = TOKEN_LINK_END;
            aFormToken.sText.AssignAscii(SwForm::aFormLinkEnd);
            SetText(aFormToken.sText);
        }
    void SetLinkStart()
        {
            DBG_ASSERT(TOKEN_LINK_END == aFormToken.eTokenType,
                                    "call SetLinkStart for link start only!")
            aFormToken.eTokenType = TOKEN_LINK_START;
            aFormToken.sText.AssignAscii(SwForm::aFormLinkStt);
            SetText(aFormToken.sText);
        }
};

//---------------------------------------------------
void    SwTOXButton::KeyInput( const KeyEvent& rKEvt )
{
    sal_Bool bCall = sal_False;
    KeyCode aCode = rKEvt.GetKeyCode();
    if(aCode.GetCode() == KEY_RIGHT)
    {
        bNextControl = sal_True;
        bCall = sal_True;
    }
    else if(aCode.GetCode() == KEY_LEFT  )
    {
        bNextControl = sal_False;
        bCall = sal_True;
    }
    else if(aCode.GetCode() == KEY_DELETE)
    {
        m_pParent->RemoveControl(this, sal_True);
        //this is invalid here
        return;
    }
    if(bCall && aPrevNextControlLink.IsSet())
            aPrevNextControlLink.Call(this);
    else
        PushButton::KeyInput(rKEvt);
}
//---------------------------------------------------
void    SwTOXButton::GetFocus()
{
    if(aGetFocusLink.IsSet())
        aGetFocusLink.Call(this);
    PushButton::GetFocus();
}
//---------------------------------------------------
void    SwTOXButton::RequestHelp( const HelpEvent& rHEvt )
{
    if(!m_pParent->CreateQuickHelp(this, aFormToken, rHEvt))
        Button::RequestHelp(rHEvt);
}
/* -----------------------------23.12.99 14:28--------------------------------

 ---------------------------------------------------------------------------*/
SwIdxTreeListBox::SwIdxTreeListBox(SwTOXEntryTabPage* pPar, const ResId& rResId) :
        SvTreeListBox(pPar, rResId),
        pParent(pPar)
{
}
/* -----------------------------23.12.99 14:19--------------------------------

 ---------------------------------------------------------------------------*/
void    SwIdxTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
     Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            sal_uInt16 nLevel = GetModel()->GetAbsPos(pEntry);
            String sEntry = pParent->GetLevelHelp(++nLevel);
            if('*' == sEntry)
                sEntry = GetEntryText(pEntry);
            if(sEntry.Len())
            {
                SvLBoxTab* pTab;
                SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
                if( pItem && SV_ITEM_ID_LBOXSTRING == pItem->IsA())
                {
                    aPos = GetEntryPos( pEntry );

                    aPos.X() = GetTabPos( pEntry, pTab );
                 Size aSize( pItem->GetSize( this, pEntry ) );

                    if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                        aSize.Width() = GetSizePixel().Width() - aPos.X();

                    aPos = OutputToScreenPixel(aPos);
                     Rectangle aItemRect( aPos, aSize );
                    Help::ShowQuickHelp( this, aItemRect, sEntry,
                            QUICKHELP_LEFT|QUICKHELP_VCENTER );
                }
            }
        }
    }
    else
        SvTreeListBox::RequestHelp(rHEvt);
}
//---------------------------------------------------
SwTOXEntryTabPage::SwTOXEntryTabPage(Window* pParent, const SfxItemSet& rAttrSet) :
        SfxTabPage(pParent, SW_RES(TP_TOX_ENTRY), rAttrSet),
    aLevelLB(this,              ResId(LB_LEVEL              )),
    aLevelFT(this,              ResId(FT_LEVEL              )),
    aTokenWIN(this,             ResId(WIN_TOKEN             )),
    aAllLevelsPB(this,          ResId(PB_ALL_LEVELS         )),
    aEntryNoPB(this,            ResId(PB_ENTRYNO            )),
    aEntryPB(this,              ResId(PB_ENTRY              )),
    aChapterInfoPB(this,        ResId(PB_CHAPTERINFO        )),
    aPageNoPB(this,             ResId(PB_PAGENO             )),
    aTabPB(this,                ResId(PB_TAB                )),
    aHyperLinkPB(this,          ResId(PB_HYPERLINK          )),
    aAuthFieldsLB(this,         ResId(LB_AUTHFIELD          )),
    aAuthInsertPB(this,         ResId(PB_AUTHINSERT         )),
    aAuthRemovePB(this,         ResId(PB_AUTHREMOVE         )),
    aCharStyleFT(this,          ResId(FT_CHARSTYLE          )),
    aCharStyleLB(this,          ResId(LB_CHARSTYLE          )),
    aEditStylePB(this,          ResId(PB_EDITSTYLE          )),
    aFillCharFT(this,           ResId(FT_FILLCHAR           )),
    aFillCharCB(this,           ResId(CB_FILLCHAR           )),
    aChapterEntryFT(this,       ResId(FT_CHAPTERENTRY       )),
    aChapterEntryLB(this,       ResId(LB_CHAPTERENTRY       )),
    aTabPosFT(this,             ResId(FT_TABPOS             )),
    aTabPosMF(this,             ResId(MF_TABPOS             )),
    aAutoRightCB(this,          ResId(CB_AUTORIGHT          )),
    aEntryGB(this,              ResId(GB_ENTRY              )),
    aRelToStyleCB(this,         ResId(CB_RELTOSTYLE         )),
    aMainEntryStyleFT(this,     ResId(FT_MAIN_ENTRY_STYLE)),
    aMainEntryStyleLB(this,     ResId(LB_MAIN_ENTRY_STYLE)),
    aAlphaDelimCB(this,         ResId(CB_ALPHADELIM         )),
    aCommaSeparatedCB(this,     ResId(CB_COMMASEPARATED     )),
    aFormatGB(this,             ResId(GB_FORMAT             )),

    aSortDocPosRB(this,         ResId(RB_DOCPOS             )),
    aSortContentRB(this,        ResId(RB_SORTCONTENT        )),
    aSortingGB(this,            ResId(GB_SORTING            )),

    aFirstKeyFT(this,           ResId(FT_FIRSTKEY           )),
    aFirstKeyLB(this,           ResId(LB_FIRSTKEY           )),
    aSecondKeyFT(this,          ResId(FT_SECONDKEY          )),
    aSecondKeyLB(this,          ResId(LB_SECONDKEY          )),
    aThirdKeyFT(this,           ResId(FT_THIRDDKEY          )),
    aThirdKeyLB(this,           ResId(LB_THIRDKEY           )),
    aFirstSortUpRB(this,        ResId(RB_SORTUP1            )),
    aFirstSortDownRB(this,      ResId(RB_SORTDOWN1          )),
    aSecondSortUpRB(this,       ResId(RB_SORTUP2            )),
    aSecondSortDownRB(this,     ResId(RB_SORTDOWN2          )),
    aThirdSortUpRB(this,        ResId(RB_SORTUP3            )),
    aThirdSortDownRB(this,      ResId(RB_SORTDOWN3          )),
    aSortKeyGB(this,            ResId(GB_SORTKEY            )),
    sNoCharStyle(               ResId(STR_NO_CHAR_STYLE)),
    sNoCharSortKey(             ResId(STR_NOSORTKEY     )),
    sDelimStr(                  ResId(STR_DELIM)),
    sAuthTypeStr(               ResId(ST_AUTHTYPE)),
    pCurrentForm(0),
    bInLevelHdl(sal_False)
{
    FreeResource();

    sLevelStr = aLevelFT.GetText();

    aLevelLB.SetWindowBits( WB_HSCROLL );
    aLevelLB.SetSpaceBetweenEntries(0);
    aLevelLB.SetSelectionMode( SINGLE_SELECTION );
    aLevelLB.SetHighlightRange();   // select full width
    aLevelLB.SetHelpId(HID_INSERT_INDEX_ENTRY_LEVEL_LB);
    aLevelLB.Show();

    aLastTOXType.eType = (TOXTypes)USHRT_MAX;
    aLastTOXType.nIndex = 0;
    //aLevelGBSize = aLevelGB.GetSizePixel();
    aLevelGBSize = aLevelFT.GetSizePixel();

    SetExchangeSupport();
    aEntryNoPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    aEntryPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    aChapterInfoPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    aPageNoPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    aTabPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    aHyperLinkPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    aEditStylePB.SetClickHdl(LINK(this, SwTOXEntryTabPage, EditStyleHdl));
    aLevelLB.SetSelectHdl(LINK(this, SwTOXEntryTabPage, LevelHdl));
    aTokenWIN.SetButtonSelectedHdl(LINK(this, SwTOXEntryTabPage, TokenSelectedHdl));
    aTokenWIN.SetModifyHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    aCharStyleLB.SetSelectHdl(LINK(this, SwTOXEntryTabPage, StyleSelectHdl));
    aCharStyleLB.InsertEntry(sNoCharStyle);
    aChapterEntryLB.SetSelectHdl(LINK(this, SwTOXEntryTabPage, ChapterInfoHdl));
    aTabPosMF.SetModifyHdl(LINK(this, SwTOXEntryTabPage, TabPosHdl));
    aFillCharCB.SetModifyHdl(LINK(this, SwTOXEntryTabPage, FillCharHdl));
    aAutoRightCB.SetClickHdl(LINK(this, SwTOXEntryTabPage, AutoRightHdl));
    aAuthInsertPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, RemoveInsertAuthHdl));
    aAuthRemovePB.SetClickHdl(LINK(this, SwTOXEntryTabPage, RemoveInsertAuthHdl));
    aSortDocPosRB.SetClickHdl(LINK(this, SwTOXEntryTabPage, SortKeyHdl));
    aSortContentRB.SetClickHdl(LINK(this, SwTOXEntryTabPage, SortKeyHdl));
    aAllLevelsPB.SetClickHdl(LINK(this, SwTOXEntryTabPage, AllLevelsHdl));

    aAlphaDelimCB.SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    aCommaSeparatedCB.SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    aRelToStyleCB.SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));

    aSortDocPosRB.Check();

    aFillCharCB.SetMaxTextLen(1);
    aFillCharCB.InsertEntry(' ');
    aFillCharCB.InsertEntry('.');
    aFillCharCB.InsertEntry('-');
    aFillCharCB.InsertEntry('_');

    aButtonPositions[0] = aEntryNoPB.GetPosPixel();
    aButtonPositions[1] = aEntryPB.GetPosPixel();
    aButtonPositions[2] = aChapterInfoPB.GetPosPixel();
    aButtonPositions[3] = aPageNoPB.GetPosPixel();
    aButtonPositions[4] = aTabPB.GetPosPixel();

    aRelToStylePos = aRelToStyleCB.GetPosPixel();
    aRelToStyleIdxPos = aRelToStylePos;
    aRelToStyleIdxPos.Y() +=
        2 * (aAlphaDelimCB.GetPosPixel().Y() - aRelToStyleIdxPos.Y());
    aEditStylePB.Enable(sal_False);

    //fill the types in
    sal_uInt32 i;
    for( i = 0; i < AUTH_FIELD_END; i++)
    {
        String sTmp(SW_RES(STR_AUTH_FIELD_START + i));
        sTmp.EraseAllChars('~');
        sal_uInt16 nPos = aAuthFieldsLB.InsertEntry(sTmp);
        aAuthFieldsLB.SetEntryData(nPos, (void*)(i));
    }
    sal_uInt16 nPos = aFirstKeyLB.InsertEntry(sNoCharSortKey);
    aFirstKeyLB.SetEntryData(nPos, (void*)(USHRT_MAX));
    nPos = aSecondKeyLB.InsertEntry(sNoCharSortKey);
    aSecondKeyLB.SetEntryData(nPos, (void*)(USHRT_MAX));
    nPos = aThirdKeyLB.InsertEntry(sNoCharSortKey);
    aThirdKeyLB.SetEntryData(nPos, (void*)(USHRT_MAX));

    for( i = 0; i < AUTH_FIELD_END; i++)
    {
        String sTmp(aAuthFieldsLB.GetEntry(i));
        void* pEntryData = aAuthFieldsLB.GetEntryData(i);
        nPos = aFirstKeyLB.InsertEntry(sTmp);
        aFirstKeyLB.SetEntryData(nPos, pEntryData);
        nPos = aSecondKeyLB.InsertEntry(sTmp);
        aSecondKeyLB.SetEntryData(nPos, pEntryData);
        nPos = aThirdKeyLB.InsertEntry(sTmp);
        aThirdKeyLB.SetEntryData(nPos, pEntryData);
    }
    aFirstKeyLB.SelectEntryPos(0);
    aSecondKeyLB.SelectEntryPos(0);
    aThirdKeyLB.SelectEntryPos(0);

}
/* -----------------30.11.99 13:37-------------------
    pVoid is used as signal to change all levels of the example
 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, ModifyHdl, void*, pVoid)
{
    UpdateDescriptor();
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();

    if(pTOXDlg)
    {
        sal_uInt16 nCurLevel = aLevelLB.GetModel()->GetAbsPos(aLevelLB.FirstSelected()) + 1;
        if(aLastTOXType.eType == TOX_CONTENT && pVoid)
            nCurLevel = USHRT_MAX;
        pTOXDlg->CreateOrUpdateExample(
            pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_ENTRY, nCurLevel);
    }
    return 0;
}

/*-- 16.06.99 10:47:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTOXEntryTabPage::~SwTOXEntryTabPage()
{
}
/*-- 16.06.99 10:47:33---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SwTOXEntryTabPage::FillItemSet( SfxItemSet& )
{
    // nothing to do
    return sal_True;
}
/*-- 16.06.99 10:47:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTOXEntryTabPage::Reset( const SfxItemSet& )
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    pCurrentForm = pTOXDlg->GetForm(aCurType);
    if(TOX_INDEX == aCurType.eType)
    {
        SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
        String sMainEntryCharStyle = rDesc.GetMainEntryCharStyle();
        if(sMainEntryCharStyle.Len())
        {
            if( LISTBOX_ENTRY_NOTFOUND ==
                    aMainEntryStyleLB.GetEntryPos(sMainEntryCharStyle))
                aMainEntryStyleLB.InsertEntry(
                        sMainEntryCharStyle);
            aMainEntryStyleLB.SelectEntry(sMainEntryCharStyle);
        }
        else
            aMainEntryStyleLB.SelectEntry(sNoCharStyle);
        aRelToStyleCB.Check(pCurrentForm->IsRelTabPos());
        aAlphaDelimCB.Check(rDesc.GetIndexOptions()&TOI_ALPHA_DELIMITTER);
    }
    aCommaSeparatedCB.Check(pCurrentForm->IsCommaSeparated());
}
/*-- 16.06.99 10:47:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_ChgWidth(Window& rWin, long nDiff)
{
 Size aTempSz(rWin.GetSizePixel());
    aTempSz.Width() += nDiff;
    rWin.SetSizePixel(aTempSz);
}
/* ----------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_ChgXPos(Window& rWin, long nDiff)
{
    Point aTempPos(rWin.GetPosPixel());
    aTempPos.X() += nDiff;
    rWin.SetPosPixel(aTempPos);
}
/* ----------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTOXEntryTabPage::ActivatePage( const SfxItemSet& rSet)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();

    pCurrentForm = pTOXDlg->GetForm(aCurType);
    if( !( aLastTOXType == aCurType ))
    {
        BOOL bToxIsAuthorities = TOX_AUTHORITIES == aCurType.eType;
        BOOL bToxIsIndex =       TOX_INDEX == aCurType.eType;
        BOOL bToxIsContent =     TOX_CONTENT == aCurType.eType;

        aLevelLB.Clear();
        for(sal_uInt16 i = 1; i < pCurrentForm->GetFormMax(); i++)
        {
            if(bToxIsAuthorities)
                aLevelLB.InsertEntry( SwAuthorityFieldType::GetAuthTypeName(
                                            (ToxAuthorityType) (i - 1)) );
            else if( bToxIsIndex )
            {
                if(i == 1)
                    aLevelLB.InsertEntry( sDelimStr );
                else
                    aLevelLB.InsertEntry( String::CreateFromInt32(i - 1) );
            }
            else
                aLevelLB.InsertEntry(String::CreateFromInt32(i));
        }
        if(bToxIsAuthorities)
        {
            //
            SwWrtShell& rSh = pTOXDlg->GetWrtShell();
            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                    rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                if(pFType->IsSortByDocument())
                    aSortDocPosRB.Check();
                else
                {
                    aSortContentRB.Check();
                    sal_uInt16 nKeyCount = pFType->GetSortKeyCount();
                    if(0 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(0);
                        aFirstKeyLB.SelectEntryPos(
                            aFirstKeyLB.GetEntryPos((void*)(sal_uInt32)pKey->eField));
                        aFirstSortUpRB.Check(pKey->bSortAscending);
                        aFirstSortDownRB.Check(!pKey->bSortAscending);
                    }
                    if(1 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(1);
                        aSecondKeyLB.SelectEntryPos(
                            aSecondKeyLB.GetEntryPos((void*)(sal_uInt32)pKey->eField));
                        aSecondSortUpRB.Check(pKey->bSortAscending);
                        aSecondSortDownRB.Check(!pKey->bSortAscending);
                    }
                    if(2 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(2);
                        aThirdKeyLB.SelectEntryPos(
                            aThirdKeyLB.GetEntryPos((void*)(sal_uInt32)pKey->eField));
                        aThirdSortUpRB.Check(pKey->bSortAscending);
                        aThirdSortDownRB.Check(!pKey->bSortAscending);
                    }
                }
            }
            SortKeyHdl(aSortDocPosRB.IsChecked() ? &aSortDocPosRB : &aSortContentRB);
            aLevelFT.SetText(sAuthTypeStr);
        }
        else
            aLevelFT.SetText(sLevelStr);

        long nDiff = 0;
        if( bToxIsAuthorities ? aLevelFT.GetSizePixel() == aLevelGBSize
                              : aLevelFT.GetSizePixel() != aLevelGBSize )
        {
            nDiff = aLevelGBSize.Width();
            if( !bToxIsAuthorities )
                nDiff *= -1;
        }

        if(nDiff)
        {
//          lcl_ChgWidth(aLevelGB, nDiff);
            lcl_ChgWidth(aLevelFT, nDiff);
            lcl_ChgWidth(aLevelLB, nDiff);
            lcl_ChgXPos(aCharStyleFT, nDiff);
            lcl_ChgXPos(aCharStyleLB, nDiff);
            lcl_ChgWidth(aCharStyleLB, -nDiff);
    //          lcl_ChgXPos(aEditStylePB, -nDiff);
            lcl_ChgXPos(aFillCharFT,  nDiff);
            lcl_ChgXPos(aFillCharCB,  nDiff);
            lcl_ChgXPos(aTabPosFT,   nDiff);
            lcl_ChgXPos(aTabPosMF,   nDiff);
            lcl_ChgXPos(aAutoRightCB, nDiff);
            lcl_ChgXPos(aAuthFieldsLB,   nDiff);
            lcl_ChgXPos(aAuthInsertPB,   nDiff);
            lcl_ChgXPos(aAuthRemovePB,   nDiff);
            lcl_ChgXPos(aTokenWIN,   nDiff);
            lcl_ChgWidth(aTokenWIN, -nDiff);
            lcl_ChgXPos(aSortDocPosRB,   nDiff);
            lcl_ChgXPos(aSortContentRB,      nDiff);
            lcl_ChgXPos(aFormatGB,  nDiff);
            lcl_ChgWidth(aFormatGB,     -nDiff);
            lcl_ChgXPos(aSortingGB, nDiff);
            lcl_ChgWidth(aSortingGB,    -nDiff);
            lcl_ChgXPos(aEntryGB,   nDiff);
            lcl_ChgWidth(aEntryGB,  -nDiff);
        }
        Link aLink = aLevelLB.GetSelectHdl();
        aLevelLB.SetSelectHdl(Link());
        aLevelLB.Select( aLevelLB.GetEntry( bToxIsIndex ? 1 : 0 ) );
        aLevelLB.SetSelectHdl(aLink);

        // sort token buttons
        aEntryNoPB.SetPosPixel(aButtonPositions[0]);
        aEntryPB.SetPosPixel(aButtonPositions[ bToxIsContent ? 1 : 0]);
        aChapterInfoPB.SetPosPixel(aButtonPositions[2]);
        aPageNoPB.SetPosPixel(aButtonPositions[3]);
        sal_uInt16 nBtPos = 1;
        if( bToxIsContent )
            nBtPos = 2;
        else if( bToxIsAuthorities )
            nBtPos = 4;
        aTabPB.SetPosPixel(aButtonPositions[nBtPos]);
        aHyperLinkPB.SetPosPixel(aButtonPositions[4]);

        //show or hide controls
        aAllLevelsPB.Show(      bToxIsContent );
        aEntryNoPB.Show(        bToxIsContent );
        aHyperLinkPB.Show(      bToxIsContent );
        aRelToStyleCB.Show(    !bToxIsAuthorities );
        aChapterInfoPB.Show(    bToxIsIndex );
        aEntryPB.Show(         !bToxIsAuthorities );
        aPageNoPB.Show(        !bToxIsAuthorities );
        aAuthFieldsLB.Show(     bToxIsAuthorities );
        aAuthInsertPB.Show(     bToxIsAuthorities );
        aAuthRemovePB.Show(     bToxIsAuthorities );
        aFormatGB.Show(        !bToxIsAuthorities );
        aSortDocPosRB.Show(     bToxIsAuthorities );
        aSortContentRB.Show(    bToxIsAuthorities );
        aSortingGB.Show(        bToxIsAuthorities );
        aFirstKeyFT.Show(       bToxIsAuthorities );
        aFirstKeyLB.Show(       bToxIsAuthorities );
        aSecondKeyFT.Show(      bToxIsAuthorities );
        aSecondKeyLB.Show(      bToxIsAuthorities );
        aThirdKeyFT.Show(       bToxIsAuthorities );
        aThirdKeyLB.Show(       bToxIsAuthorities );
        aSortKeyGB.Show(        bToxIsAuthorities );
        aFirstSortUpRB.Show(    bToxIsAuthorities );
        aFirstSortDownRB.Show(  bToxIsAuthorities );
        aSecondSortUpRB.Show(   bToxIsAuthorities );
        aSecondSortDownRB.Show( bToxIsAuthorities );
        aThirdSortUpRB.Show(    bToxIsAuthorities );
        aThirdSortDownRB.Show(  bToxIsAuthorities );

        aRelToStyleCB.SetPosPixel( bToxIsIndex ? aRelToStyleIdxPos
                                               : aRelToStylePos );

//      aRecalcTabCB.Show(  aCurType.eType == TOX_CONTENT);

        aMainEntryStyleFT.Show( bToxIsIndex );
        aMainEntryStyleLB.Show( bToxIsIndex );
        aAlphaDelimCB.Show(     bToxIsIndex );
        aCommaSeparatedCB.Show( bToxIsIndex );

        //Resizing
        if( TOX_CONTENT == aLastTOXType.eType|| bToxIsContent )
        {
         Point aTokenWinPos(aTokenWIN.GetPosPixel());
         Size aTokenWinSz(aTokenWIN.GetSizePixel());

            long nButtonWidth = aAllLevelsPB.GetSizePixel().Width();
            long nButtonXPos = aAllLevelsPB.GetPosPixel().X();
            long nTokenWinSize;
            if( bToxIsContent )
                nTokenWinSize = nButtonXPos - ( nButtonWidth / 13);
            else
                nTokenWinSize = nButtonXPos + nButtonWidth;
            nTokenWinSize -= aTokenWinPos.X();
            aTokenWinSz.Width() = nTokenWinSize;
            aTokenWIN.SetSizePixel(aTokenWinSz);
        }
    }
    aLastTOXType = aCurType;

    //invalidate PatternWindow
    aTokenWIN.Invalidate();
    LevelHdl(&aLevelLB);
}
/* -----------------30.11.99 15:04-------------------

 --------------------------------------------------*/
void SwTOXEntryTabPage::UpdateDescriptor()
{
    WriteBackLevel();
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aLastTOXType);
    if(TOX_INDEX == aLastTOXType.eType)
    {
        String sTemp(aMainEntryStyleLB.GetSelectEntry());
        rDesc.SetMainEntryCharStyle(sNoCharStyle == sTemp ? aEmptyStr : sTemp);
        sal_uInt16 nIdxOptions = rDesc.GetIndexOptions() & ~TOI_ALPHA_DELIMITTER;
        if(aAlphaDelimCB.IsChecked())
            nIdxOptions |= TOI_ALPHA_DELIMITTER;
        rDesc.SetIndexOptions(nIdxOptions);
    }
    else if(TOX_AUTHORITIES == aLastTOXType.eType)
    {
        rDesc.SetSortByDocument(aSortDocPosRB.IsChecked());
        SwTOXSortKey aKey1, aKey2, aKey3;
        aKey1.eField = (ToxAuthorityField)(sal_uInt32)aFirstKeyLB.GetEntryData(
                                    aFirstKeyLB.GetSelectEntryPos());
        aKey1.bSortAscending = aFirstSortUpRB.IsChecked();
        aKey2.eField = (ToxAuthorityField)(sal_uInt32)aSecondKeyLB.GetEntryData(
                                    aSecondKeyLB.GetSelectEntryPos());
        aKey2.bSortAscending = aSecondSortUpRB.IsChecked();
        aKey3.eField = (ToxAuthorityField)(sal_uInt32)aThirdKeyLB.GetEntryData(
                                aThirdKeyLB.GetSelectEntryPos());
        aKey3.bSortAscending = aThirdSortUpRB.IsChecked();


        rDesc.SetSortKeys(aKey1, aKey2, aKey3);
    }
    SwForm* pCurrentForm = pTOXDlg->GetForm(aLastTOXType);
    if(aRelToStyleCB.IsVisible())
    {
        pCurrentForm->SetRelTabPos(aRelToStyleCB.IsChecked());
    }
    if(aCommaSeparatedCB.IsVisible())
        pCurrentForm->SetCommaSeparated(aCommaSeparatedCB.IsChecked());
}
/*-- 16.06.99 10:47:34---------------------------------------------------

  -----------------------------------------------------------------------*/
int SwTOXEntryTabPage::DeactivatePage( SfxItemSet* pSet)
{
    UpdateDescriptor();
    return LEAVE_PAGE;
}
/*-- 16.06.99 10:47:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SwTOXEntryTabPage::Create( Window* pParent,     const SfxItemSet& rAttrSet)
{
    return new SwTOXEntryTabPage(pParent, rAttrSet);
}
/*-- 16.06.99 10:47:35---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, EditStyleHdl, PushButton*, pBtn)
{
    if( LISTBOX_ENTRY_NOTFOUND != aCharStyleLB.GetSelectEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, aCharStyleLB.GetSelectEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, SFX_STYLE_FAMILY_CHAR);
        // TODO: WrtShell?
//      SwPtrItem aShell(FN_PARAM_WRTSHELL, pWrtShell);
        Window* pDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );
        ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell().
        GetView().GetViewFrame()->GetDispatcher()->Execute(
        SID_STYLE_EDIT, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_MODAL,
            &aStyle, &aFamily/*, &aShell*/, 0L);
        Application::SetDefDialogParent( pDefDlgParent );
    }
    return 0;
}
/* -----------------04.10.99 11:34-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, RemoveInsertAuthHdl, PushButton*, pButton)
{
    sal_Bool bInsert = pButton == &aAuthInsertPB;
    if(bInsert)
    {
        sal_uInt16 nSelPos = aAuthFieldsLB.GetSelectEntryPos();
        String sToInsert(aAuthFieldsLB.GetSelectEntry());
        SwFormToken aInsert(TOKEN_AUTHORITY);
        aInsert.nAuthorityField = (sal_uInt16)(sal_uInt32)aAuthFieldsLB.GetEntryData(nSelPos);
        aTokenWIN.InsertAtSelection(String::CreateFromAscii(
                                            SwForm::aFormAuth), aInsert);
        aAuthFieldsLB.RemoveEntry(sToInsert);
        aAuthFieldsLB.SelectEntryPos( nSelPos ? nSelPos - 1 : 0);
    }
    else
    {
        Control* pCtrl = aTokenWIN.GetActiveControl();
        DBG_ASSERT(WINDOW_EDIT != pCtrl->GetType(), "Remove should be disabled")
        if( WINDOW_EDIT != pCtrl->GetType() )
        {
            //fill it into the ListBox
            const SwFormToken& rToken = ((SwTOXButton*)pCtrl)->GetFormToken();
            PreTokenButtonRemoved(rToken);
            aTokenWIN.RemoveControl((SwTOXButton*)pCtrl);
        }
    }
    ModifyHdl(0);
    return 0;
}
/* -----------------------------17.01.00 13:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwTOXEntryTabPage::PreTokenButtonRemoved(const SwFormToken& rToken)
{
    //fill it into the ListBox
    sal_uInt32 nData = rToken.nAuthorityField;
    String sTemp(SW_RES(STR_AUTH_FIELD_START + nData));
    sTemp.EraseAllChars('~');
    sal_uInt16 nPos = aAuthFieldsLB.InsertEntry(sTemp);
    aAuthFieldsLB.SetEntryData(nPos, (void*)(nData));
}
/*-- 16.06.99 10:47:35---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, InsertTokenHdl, PushButton*, pBtn)
{
    String sText;
    FormTokenType eTokenType;
    if(pBtn == &aEntryNoPB)
    {
        sText.AssignAscii(SwForm::aFormEntryNum);
        eTokenType = TOKEN_ENTRY_NO;
    }
    else if(pBtn == &aEntryPB)
    {
        if( TOX_CONTENT == pCurrentForm->GetTOXType() )
        {
            sText.AssignAscii( SwForm::aFormEntryTxt );
            eTokenType = TOKEN_ENTRY_TEXT;
        }
        else
        {
            sText.AssignAscii( SwForm::aFormEntry);
            eTokenType = TOKEN_ENTRY;
        }
    }
    else if(pBtn == &aChapterInfoPB)
    {
        sText.AssignAscii( SwForm::aFormChapterMark);
        eTokenType = TOKEN_CHAPTER_INFO;
    }
    else if(pBtn == &aPageNoPB)
    {
        sText.AssignAscii(SwForm::aFormPageNums);
        eTokenType = TOKEN_PAGE_NUMS;
    }
    else if(pBtn == &aHyperLinkPB)
    {
        sText.AssignAscii(SwForm::aFormLinkStt);
        eTokenType = TOKEN_LINK_START;
    }
    else if(pBtn == &aTabPB)
    {
        sText.AssignAscii(SwForm::aFormTab);
        eTokenType = TOKEN_TAB_STOP;
    }
    SwFormToken aInsert(eTokenType);
    aInsert.nTabStopPosition = 0;
    aTokenWIN.InsertAtSelection(sText, aInsert);
    ModifyHdl(0);
    return 0;
}
/* -----------------------------14.01.00 11:53--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, AllLevelsHdl, PushButton*, pButton)
{
    //get current level
    //write it into all levels
    if(aTokenWIN.IsValid())
    {
        String sNewToken = aTokenWIN.GetPattern();
        for(sal_uInt16 i = 1; i < pCurrentForm->GetFormMax(); i++)
            pCurrentForm->SetPattern(i, sNewToken);
        //
        ModifyHdl(this);
    }
    return 0;
}

/* -----------------02.12.99 12:40-------------------

 --------------------------------------------------*/
void SwTOXEntryTabPage::WriteBackLevel()
{
    if(aTokenWIN.IsValid())
    {
        String sNewToken = aTokenWIN.GetPattern();
        sal_uInt16 nLastLevel = aTokenWIN.GetLastLevel();
        if(nLastLevel != USHRT_MAX)
            pCurrentForm->SetPattern(nLastLevel + 1, sNewToken );
    }
}
/*-- 16.06.99 10:47:35---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, LevelHdl, SvTreeListBox*, pBox)
{
    if(bInLevelHdl)
        return 0;
    bInLevelHdl = sal_True;
    WriteBackLevel();

    sal_uInt16 nLevel = pBox->GetModel()->GetAbsPos(pBox->FirstSelected());
    aTokenWIN.SetForm(*pCurrentForm, nLevel);
    if(TOX_AUTHORITIES == pCurrentForm->GetTOXType())
    {
        //fill the types in
        aAuthFieldsLB.Clear();
        for( sal_uInt32 i = 0; i < AUTH_FIELD_END; i++)
        {
            String sTmp(SW_RES(STR_AUTH_FIELD_START + i));
            sTmp.EraseAllChars('~');
            sal_uInt16 nPos = aAuthFieldsLB.InsertEntry(sTmp);
            aAuthFieldsLB.SetEntryData(nPos, (void*)(i));
        }
        String sLevelPattern(pCurrentForm->GetPattern(nLevel + 1));
        SwFormTokenEnumerator aEnumerator(sLevelPattern);
        while(aEnumerator.HasNextToken())
        {
            SwFormToken aToken = aEnumerator.GetNextToken();
            if(TOKEN_AUTHORITY == aToken.eTokenType)
            {
                sal_uInt32 nSearch = aToken.nAuthorityField;
                sal_uInt16  nLstBoxPos = aAuthFieldsLB.GetEntryPos( (void*) nSearch );
                DBG_ASSERT(LISTBOX_ENTRY_NOTFOUND != nLstBoxPos, "Entry not found?")
                aAuthFieldsLB.RemoveEntry(nLstBoxPos);
            }
        }
        aAuthFieldsLB.SelectEntryPos(0);
    }
    bInLevelHdl = sal_False;
    pBox->GrabFocus();
    return 0;
}
/* -----------------20.10.99 13:16-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, SortKeyHdl, RadioButton*, pButton)
{
    sal_Bool bEnable = &aSortContentRB == pButton;
    aFirstKeyFT.Enable(bEnable);
    aFirstKeyLB.Enable(bEnable);
    aSecondKeyFT.Enable(bEnable);
    aSecondKeyLB.Enable(bEnable);
    aThirdKeyFT.Enable(bEnable);
    aThirdKeyLB.Enable(bEnable);
    aSortKeyGB.Enable(bEnable);
    aFirstSortUpRB.Enable(bEnable);
    aFirstSortDownRB.Enable(bEnable);
    aSecondSortUpRB.Enable(bEnable);
    aSecondSortDownRB.Enable(bEnable);
    aThirdSortUpRB.Enable(bEnable);
    aThirdSortDownRB.Enable(bEnable);
    return 0;
}
/* -----------------01.07.99 12:21-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, TokenSelectedHdl, SwFormToken*, pToken)
{
    if(pToken->sCharStyleName.Len())
        aCharStyleLB.SelectEntry(pToken->sCharStyleName);
    else
        aCharStyleLB.SelectEntry(sNoCharStyle);
    //StyleSelectHdl(&aCharStyleLB);

    sal_Bool bTabStop = pToken->eTokenType == TOKEN_TAB_STOP;

    aCharStyleLB.Enable(!bTabStop);
    String sEntry = aCharStyleLB.GetSelectEntry();
    aEditStylePB.Enable(!bTabStop && sEntry != sNoCharStyle);

    if(pToken->eTokenType == TOKEN_CHAPTER_INFO)
    {
        if(pToken->nChapterFormat < 3)
            aChapterEntryLB.SelectEntryPos(pToken->nChapterFormat);
        else
            aChapterEntryLB.SetNoSelection();
    }

    aFillCharFT.Show(bTabStop);
    aFillCharCB.Show(bTabStop);
    aTabPosFT.Show(bTabStop);
    aTabPosMF.Show(bTabStop);
    aAutoRightCB.Show(bTabStop);
    aAutoRightCB.Enable(bTabStop);
    if(bTabStop)
    {
        aTabPosMF.SetValue(aTabPosMF.Normalize(pToken->nTabStopPosition), FUNIT_TWIP);
        aAutoRightCB.Check(SVX_TAB_ADJUST_END == pToken->eTabAlign);
        aFillCharCB.SetText(pToken->cTabFillChar);
        aTabPosFT.Enable(!aAutoRightCB.IsChecked());
        aTabPosMF.Enable(!aAutoRightCB.IsChecked());
    }
    else
    {
        aTabPosMF.Enable(sal_False);
    }

    aChapterEntryFT.Show(pToken->eTokenType == TOKEN_CHAPTER_INFO);
    aChapterEntryLB.Show(pToken->eTokenType == TOKEN_CHAPTER_INFO);

    //now enable the visible buttons
    //- inserting the same type of control is not allowed
    //- some types of controls can only appear once (EntryText EntryNumber)

    if(aEntryNoPB.IsVisible())
    {
        aEntryNoPB.Enable(TOKEN_ENTRY_NO != pToken->eTokenType );
    }
    if(aEntryPB.IsVisible())
    {
        aEntryPB.Enable(TOKEN_ENTRY_TEXT != pToken->eTokenType &&
                                !aTokenWIN.Contains(TOKEN_ENTRY_TEXT)
                                && !aTokenWIN.Contains(TOKEN_ENTRY));
    }

    if(aChapterInfoPB.IsVisible())
    {
        aChapterInfoPB.Enable(TOKEN_CHAPTER_INFO != pToken->eTokenType);
    }
    if(aPageNoPB.IsVisible())
    {
        aPageNoPB.Enable(TOKEN_PAGE_NUMS != pToken->eTokenType &&
                                !aTokenWIN.Contains(TOKEN_PAGE_NUMS));
    }
    if(aTabPB.IsVisible())
    {
        aTabPB.Enable(!bTabStop);
    }
    if(aHyperLinkPB.IsVisible())
    {
        aHyperLinkPB.Enable(TOKEN_LINK_START != pToken->eTokenType &&
                            TOKEN_LINK_END != pToken->eTokenType);
    }
    //table of authorities
    if(aAuthInsertPB.IsVisible())
    {
        sal_Bool bText = TOKEN_TEXT == pToken->eTokenType;
        aAuthInsertPB.Enable(bText && aAuthFieldsLB.GetSelectEntry().Len());
        aAuthRemovePB.Enable(!bText);
    }

    return 0;
}
/* -----------------01.07.99 12:36-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, StyleSelectHdl, ListBox*, pBox)
{
    String sEntry = pBox->GetSelectEntry();
    sal_uInt16 nId = (sal_uInt16)(long)pBox->GetEntryData(pBox->GetSelectEntryPos());
    aEditStylePB.Enable(sEntry != sNoCharStyle);
    if(sEntry == sNoCharStyle)
        sEntry.Erase();
    Control* pCtrl = aTokenWIN.GetActiveControl();
    DBG_ASSERT(pCtrl, "no active control?")
    if(pCtrl)
    {
        if(WINDOW_EDIT == pCtrl->GetType())
            ((SwTOXEdit*)pCtrl)->SetCharStyleName(sEntry, nId);
        else
            ((SwTOXButton*)pCtrl)->SetCharStyleName(sEntry, nId);

    }
    ModifyHdl(0);
    return 0;
}
/* -----------------------------11.01.00 12:54--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, ChapterInfoHdl, ListBox*, pBox)
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = aTokenWIN.GetActiveControl();
        DBG_ASSERT(pCtrl, "no active control?")
        if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
            ((SwTOXButton*)pCtrl)->SetChapterInfo(nPos);

        ModifyHdl(0);
    }
    return 0;
}
/* -----------------19.08.99 15:37-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, TabPosHdl, MetricField*, pField)
{
    Control* pCtrl = aTokenWIN.GetActiveControl();
    DBG_ASSERT(pCtrl && WINDOW_EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == ((SwTOXButton*)pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?")
    if( pCtrl && WINDOW_EDIT != pCtrl->GetType() )
    {
        ((SwTOXButton*)pCtrl)->SetTabPosition(
                pField->Denormalize( pField->GetValue( FUNIT_TWIP )));
    }
    ModifyHdl(0);
    return 0;
}
/* -----------------09.09.99 15:37-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, FillCharHdl, ComboBox*, pBox)
{
    Control* pCtrl = aTokenWIN.GetActiveControl();
    DBG_ASSERT(pCtrl && WINDOW_EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == ((SwTOXButton*)pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?")
    if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
    {
        sal_Unicode cSet;
        if( pBox->GetText().Len() )
            cSet = pBox->GetText().GetChar(0);
        else
            cSet = ' ';
        ((SwTOXButton*)pCtrl)->SetFillChar( cSet );
    }
    ModifyHdl(0);
    return 0;
}

/*-- 16.06.99 10:47:36---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, AutoRightHdl, CheckBox*, pBox)
{
    //the most right style::TabStop is usually right aligned
    Control* pCurCtrl = aTokenWIN.GetActiveControl();
    DBG_ASSERT(WINDOW_EDIT != pCurCtrl->GetType() &&
            ((SwTOXButton*)pCurCtrl)->GetFormToken().eTokenType == TOKEN_TAB_STOP,
            "no style::TabStop selected!")

    const SwFormToken& rToken = ((SwTOXButton*)pCurCtrl)->GetFormToken();
    sal_Bool bChecked = pBox->IsChecked();
    if(rToken.eTokenType == TOKEN_TAB_STOP)
        ((SwTOXButton*)pCurCtrl)->SetTabAlign(
            bChecked ? SVX_TAB_ADJUST_END : SVX_TAB_ADJUST_LEFT);
    aTabPosFT.Enable(!bChecked);
    aTabPosMF.Enable(!bChecked);
    ModifyHdl(0);
    return 0;
}
/* -----------------16.06.99 11:00-------------------

 --------------------------------------------------*/
void SwTOXEntryTabPage::SetWrtShell(SwWrtShell& rSh)
{
    SwDocShell* pDocSh = rSh.GetView().GetDocShell();
    ::FillCharStyleListBox(aCharStyleLB, pDocSh, TRUE);
    for(sal_uInt16 i = 0; i < aCharStyleLB.GetEntryCount(); i++)
    {
        aMainEntryStyleLB.InsertEntry( aCharStyleLB.GetEntry(i) );
        aMainEntryStyleLB.SetEntryData(i, aCharStyleLB.GetEntryData(i));
    }
    String sTmp; GetDocPoolNm( RES_POOLCHR_IDX_MAIN_ENTRY, sTmp );
    aMainEntryStyleLB.SelectEntry(sTmp);
}
/* -----------------------------23.12.99 14:23--------------------------------

 ---------------------------------------------------------------------------*/
String  SwTOXEntryTabPage::GetLevelHelp(sal_uInt16 nLevel) const
{
    String sRet;
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    if( TOX_INDEX == aCurType.eType )
        GetDocPoolNm( 1 == nLevel ? RES_POOLCOLL_TOX_IDXBREAK
                                  : RES_POOLCOLL_TOX_IDX1 + nLevel-2, sRet );

    else if( TOX_AUTHORITIES == aCurType.eType )
    {
        //wildcard -> show entry text
        sRet = '*';
    }
    return sRet;
}
/* -----------------16.06.99 15:18-------------------

 --------------------------------------------------*/

SwTokenWindow::SwTokenWindow(SwTOXEntryTabPage* pParent, const ResId& rResId) :
        Window( pParent, rResId ),
        aLeftScrollWin(this, ResId(WIN_LEFT_SCROLL   )),
        aCtrlParentWin(this, ResId(WIN_CTRL_PARENT   )),
        aRightScrollWin(this, ResId(WIN_RIGHT_SCROLL )),
        pForm(0),
        m_pParent(pParent),
        nLevel(0),
        bValid(sal_False),
        pActiveCtrl(0),
        sCharStyle(ResId(STR_CHARSTYLE))
{
    SetHelpId(HID_TOKEN_WINDOW);
    for(sal_uInt16 i = 0; i < TOKEN_END; i++)
    {
        sal_uInt16 nTextId = STR_BUTTON_TEXT_START + i;
        if( STR_TOKEN_ENTRY_TEXT == nTextId )
            nTextId = STR_TOKEN_ENTRY;
        aButtonTexts[i] = String(ResId(nTextId));

        sal_uInt16 nHelpId = STR_BUTTON_HELP_TEXT_START + i;
        if(STR_TOKEN_HELP_ENTRY_TEXT == nHelpId)
            nHelpId = STR_TOKEN_HELP_ENTRY;
        aButtonHelpTexts[i] = String(ResId(nHelpId));
    }

    FreeResource();

    Link aLink(LINK(this, SwTokenWindow, ScrollHdl));
    aLeftScrollWin.SetClickHdl(aLink);
    aRightScrollWin.SetClickHdl(aLink);
}
/* -----------------01.07.99 12:17-------------------

 --------------------------------------------------*/
SwTokenWindow::~SwTokenWindow()
{
//  for(sal_uInt16 i = GetItemCount(); i ; i--)
//      RemoveItem(i - 1);

    for( sal_uInt16 i = aControlList.Count(); i; )
    {
        Control* pControl = aControlList.Remove( --i );
        delete pControl;
    }
}
/* -----------------16.06.99 13:56-------------------

 --------------------------------------------------*/
void    SwTokenWindow::SetForm(SwForm& rForm, sal_uInt16 nL)
{
    SetActiveControl(0);
    bValid = sal_True;
    if(pForm)
    {
        //apply current level settings to the form
        for( sal_uInt16 i = aControlList.Count(); i; )
        {
            Control* pControl = aControlList.Remove( --i );
            delete pControl;
        }
    }
    nLevel = nL;
    pForm = &rForm;
    //now the display
    if(nLevel < MAXLEVEL || rForm.GetTOXType() == TOX_AUTHORITIES)
    {
        sal_uInt16 nControlId = 1;
         Size aToolBoxSize = GetSizePixel();

        String sLevelPattern(pForm->GetPattern(nLevel + 1));
        SwFormTokenEnumerator aEnumerator(sLevelPattern);
        sal_Bool bLastWasText = sal_False; //assure alternating text - code - text

        Control* pSetActiveControl = 0;
        while(aEnumerator.HasNextToken())
        {
            SwFormToken aToken(aEnumerator.GetNextToken());
            if(TOKEN_TEXT == aToken.eTokenType)
            {
                DBG_ASSERT(!bLastWasText, "text following text is invalid")
                Control* pCtrl = InsertItem(aToken.sText, aToken);
                bLastWasText = sal_True;
                if(!GetActiveControl())
                    SetActiveControl(pCtrl);
            }
            else
            {
                if( !bLastWasText )
                {
                    bLastWasText = sal_True;
                    SwFormToken aTemp(TOKEN_TEXT);
                    Control* pCtrl = InsertItem(aEmptyStr, aTemp);
                    if(!pSetActiveControl)
                        pSetActiveControl = pCtrl;
                }
                const sal_Char* pTmp = 0;
                switch( aToken.eTokenType )
                {
                case TOKEN_ENTRY_NO:    pTmp = SwForm::aFormEntryNum; break;
                case TOKEN_ENTRY_TEXT:  pTmp = SwForm::aFormEntryTxt; break;
                case TOKEN_ENTRY:       pTmp = SwForm::aFormEntry; break;
                case TOKEN_TAB_STOP:    pTmp = SwForm::aFormTab; break;
                case TOKEN_PAGE_NUMS:   pTmp = SwForm::aFormPageNums; break;
                case TOKEN_CHAPTER_INFO:pTmp = SwForm::aFormChapterMark; break;
                case TOKEN_LINK_START:  pTmp = SwForm::aFormLinkStt; break;
                case TOKEN_LINK_END:    pTmp = SwForm::aFormLinkEnd; break;
                case TOKEN_AUTHORITY:   pTmp = SwForm::aFormAuth; break;
                }

                InsertItem( pTmp ? String::CreateFromAscii(pTmp)
                                 : aEmptyStr, aToken );
                bLastWasText = sal_False;
            }
        }
        SetActiveControl(pSetActiveControl);

        if(!bLastWasText)
        {
            bLastWasText = sal_True;
            SwFormToken aTemp(TOKEN_TEXT);
            InsertItem(aEmptyStr, aTemp);
        }
    }
    AdjustScrolling();
}
/* -----------------19.08.99 13:00-------------------

 --------------------------------------------------*/
void SwTokenWindow::SetActiveControl(Control* pSet)
{
    if( pSet != pActiveCtrl )
    {
        if( pActiveCtrl && WINDOW_EDIT == pActiveCtrl->GetType() )
            ((SwTOXEdit*)pActiveCtrl)->SetControlBackground( COL_WHITE );

        pActiveCtrl = pSet;
        if( pActiveCtrl )
        {
            pActiveCtrl->GrabFocus();
            //it must be a SwTOXEdit
            const SwFormToken* pFToken;
            if( WINDOW_EDIT == pActiveCtrl->GetType() )
            {
                ((SwTOXEdit*)pActiveCtrl)->SetControlBackground(COL_LIGHTGRAY);
                pFToken = &((SwTOXEdit*)pActiveCtrl)->GetFormToken();
            }
            else
                pFToken = &((SwTOXButton*)pActiveCtrl)->GetFormToken();

            SwFormToken aTemp( *pFToken );
            aButtonSelectedHdl.Call( &aTemp );
        }
    }
}

/* -----------------17.06.99 09:53-------------------

 --------------------------------------------------*/
Control*    SwTokenWindow::InsertItem(const String& rText, const SwFormToken& rToken)
{
    Control* pRet = 0;
    Control* pLast = aControlList.Last();
    Size aControlSize(GetOutputSizePixel());
     Point aControlPos;
    if( pLast )
    {
        aControlSize = pLast->GetSizePixel();
        aControlPos = pLast->GetPosPixel();
        aControlPos.X() += aControlSize.Width();
    }
    if(TOKEN_TEXT == rToken.eTokenType)
    {
        SwTOXEdit* pEdit = new SwTOXEdit(&aCtrlParentWin, this, rToken);
        pEdit->SetPosPixel(aControlPos);
        aControlList.Insert(pEdit, aControlList.Count());
        pEdit->SetText(rText);
         Size aEditSize(aControlSize);
        aEditSize.Width() = pEdit->GetTextWidth(rText) + EDIT_MINWIDTH;
        pEdit->SetSizePixel(aEditSize);
        pEdit->SetModifyHdl(LINK(this, SwTokenWindow, EditResize ));
        pEdit->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemHdl));
        pEdit->SetGetFocusLink(LINK(this, SwTokenWindow, TbxFocusHdl));
        pEdit->Show();
        pRet = pEdit;
    }
    else
    {
        SwTOXButton* pButton = new SwTOXButton(&aCtrlParentWin, this, rToken);
        pButton->SetPosPixel(aControlPos);
        aControlList.Insert(pButton, aControlList.Count());
         Size aEditSize(aControlSize);
        aEditSize.Width() = pButton->GetTextWidth(rText) + 5;
        pButton->SetControlBackground(Color(COL_WHITE));
        pButton->SetSizePixel(aEditSize);
        pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
        pButton->SetGetFocusLink(LINK(this, SwTokenWindow, TbxFocusBtnHdl));
        if(TOKEN_AUTHORITY != rToken.eTokenType)
            pButton->SetText(aButtonTexts[rToken.eTokenType]);
        else
        {
            //use the first two chars as symbol
            String sTmp(SwAuthorityFieldType::GetAuthFieldName(
                        (ToxAuthorityField)rToken.nAuthorityField));
            pButton->SetText(sTmp.Copy(0, 2));
        }
        pButton->Show();
        pRet = pButton;
    }
    return pRet;
}
/* -----------------16.07.99 11:50-------------------

 --------------------------------------------------*/
void    SwTokenWindow::InsertAtSelection(
            const String& rText,
            const SwFormToken& rToken)
{
    DBG_ASSERT(pActiveCtrl, "no active control!")
    if(!pActiveCtrl)
        return;
    SwFormToken aToInsertToken(rToken);
    if(TOKEN_LINK_START == aToInsertToken.eTokenType)
    {
        //determine if start or end of hyperlink is appropriate
        //eventually change a following link start into a link end
        // groups of LS LE should be ignored
        // <insert>
        //LS <insert>
        //LE <insert>
        //<insert> LS
        //<insert> LE
        //<insert>
        sal_Bool bPreStartLinkFound = sal_False; //
        sal_Bool bPreEndLinkFound = sal_False;

        const Control* pControl = aControlList.First();
        const Control* pExchange = 0;
        while( pControl && pActiveCtrl != pControl )
        {
            if( WINDOW_EDIT != pControl->GetType())
            {
                const SwFormToken& rNewToken =
                                ((SwTOXButton*)pControl)->GetFormToken();
                if( TOKEN_LINK_START == rNewToken.eTokenType )
                {
                    bPreStartLinkFound = sal_True;
                    pExchange = 0;
                }
                else if(TOKEN_LINK_END == rNewToken.eTokenType)
                {
                    if( bPreStartLinkFound )
                        bPreStartLinkFound = sal_False;
                    else
                    {
                        bPreEndLinkFound = sal_False;
                        pExchange = pControl;
                    }
                }
            }
            pControl = aControlList.Next();
        }

        sal_Bool bPostLinkEndFound = sal_False;
        sal_Bool bPostLinkStartFound = sal_False;
        if(!bPreStartLinkFound && !bPreEndLinkFound)
            while(pControl)
            {
                if( pControl != pActiveCtrl &&
                    WINDOW_EDIT != pControl->GetType())
                {
                    const SwFormToken& rNewToken =
                                    ((SwTOXButton*)pControl)->GetFormToken();
                    if( TOKEN_LINK_START == rNewToken.eTokenType )
                    {
                        if(bPostLinkStartFound)
                            break;
                        bPostLinkStartFound = sal_True;
                        pExchange = pControl;
                    }
                    else if(TOKEN_LINK_END == rNewToken.eTokenType )
                    {
                        if(bPostLinkStartFound)
                        {
                            bPostLinkStartFound = sal_False;
                            pExchange = 0;
                        }
                        else
                        {
                            bPostLinkEndFound = sal_True;
                        }
                        break;
                    }
                }
                pControl = aControlList.Next();
            }

        if(bPreStartLinkFound)
        {
            aToInsertToken.eTokenType = TOKEN_LINK_END;
            aToInsertToken.sText =  aButtonTexts[TOKEN_LINK_END];
        }

        if(bPostLinkStartFound)
        {
            DBG_ASSERT(pExchange, "no control to exchange?")
            if(pExchange)
            {
                ((SwTOXButton*)pExchange)->SetLinkEnd();
                ((SwTOXButton*)pExchange)->SetText(aButtonTexts[TOKEN_LINK_END]);
            }
        }

        if(bPreEndLinkFound)
        {
            DBG_ASSERT(pExchange, "no control to exchange?")
            if(pExchange)
            {
                ((SwTOXButton*)pExchange)->SetLinkStart();
                ((SwTOXButton*)pExchange)->SetText(aButtonTexts[TOKEN_LINK_START]);
            }
        }
    }

    //if the active control is text then insert a new button at the selection
    //else replace the button
    sal_uInt32 nActivePos = aControlList.GetPos(pActiveCtrl);
    sal_uInt32 nInsertPos = nActivePos;

 Size aControlSize(GetOutputSizePixel());
    if( WINDOW_EDIT == pActiveCtrl->GetType())
    {
        nInsertPos++;
         Selection aSel = ((SwTOXEdit*)pActiveCtrl)->GetSelection();
        aSel.Justify();
        String sEditText = ((SwTOXEdit*)pActiveCtrl)->GetText();
        String sLeft = sEditText.Copy(0, aSel.A());
        String sRight = sEditText.Copy(aSel.B(), sEditText.Len() - aSel.B());

        ((SwTOXEdit*)pActiveCtrl)->SetText(sLeft);
        ((SwTOXEdit*)pActiveCtrl)->AdjustSize();

        SwFormToken aTmpToken(TOKEN_TEXT);
        SwTOXEdit* pEdit = new SwTOXEdit(&aCtrlParentWin, this, aTmpToken);
        aControlList.Insert(pEdit, nActivePos + 1);
        pEdit->SetText(sRight);
        pEdit->SetSizePixel(aControlSize);
        pEdit->AdjustSize();
        pEdit->SetModifyHdl(LINK(this, SwTokenWindow, EditResize ));
        pEdit->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemHdl));
        pEdit->SetGetFocusLink(LINK(this, SwTokenWindow, TbxFocusHdl));
        pEdit->Show();
    }
    else
    {
        aControlList.Remove(pActiveCtrl);
        pActiveCtrl->Hide();
        delete pActiveCtrl;
    }
    //now the new button
    SwTOXButton* pButton = new SwTOXButton(&aCtrlParentWin, this, aToInsertToken);
    aControlList.Insert(pButton, nInsertPos);
    pButton->SetControlBackground(Color(COL_WHITE));
    pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
    pButton->SetGetFocusLink(LINK(this, SwTokenWindow, TbxFocusBtnHdl));
    if(TOKEN_AUTHORITY != aToInsertToken.eTokenType)
        pButton->SetText(aButtonTexts[aToInsertToken.eTokenType]);
    else
    {
        //use the first two chars as symbol
        String sTmp(SwAuthorityFieldType::GetAuthFieldName(
                    (ToxAuthorityField)aToInsertToken.nAuthorityField));
        pButton->SetText(sTmp.Copy(0, 2));
    }

 Size aEditSize(GetOutputSizePixel());
    aEditSize.Width() = pButton->GetTextWidth(rText) + 5;
    pButton->SetSizePixel(aEditSize);
    pButton->Check(sal_True);
    pButton->Show();
    SetActiveControl(pButton);

    AdjustPositions();
}
/* -----------------19.08.99 12:42-------------------

 --------------------------------------------------*/
void SwTokenWindow::RemoveControl(SwTOXButton* pDel, sal_Bool bInternalCall )
{
    if(bInternalCall && TOX_AUTHORITIES == pForm->GetTOXType())
        m_pParent->PreTokenButtonRemoved(pDel->GetFormToken());

    sal_uInt32 nActivePos = aControlList.GetPos(pDel);
    DBG_ASSERT(nActivePos != 0xffffffff, "Control does not exist!");
    // the two neighbours of the box must be merged
    // the properties of the right one will be lost
    DBG_ASSERT(nActivePos && nActivePos < aControlList.Count() - 1,
        "Button at first or last position?");
    aControlList.Seek(nActivePos - 1);
    Control* pLeftEdit = aControlList.GetCurObject();
    aControlList.Seek(nActivePos + 1);
    Control* pRightEdit = aControlList.GetCurObject();
    String sTemp(((SwTOXEdit*)pLeftEdit)->GetText());
    sTemp += ((SwTOXEdit*)pRightEdit)->GetText();
    ((SwTOXEdit*)pLeftEdit)->SetText(sTemp);
    ((SwTOXEdit*)pLeftEdit)->AdjustSize();

    aControlList.Remove(pRightEdit);
    delete pRightEdit;

    aControlList.Remove(pDel);
    pActiveCtrl->Hide();
    delete pActiveCtrl;
    SetActiveControl(pLeftEdit);
    AdjustPositions();
    if(aModifyHdl.IsSet())
        aModifyHdl.Call(0);
}

/* -----------------16.07.99 12:39-------------------

 --------------------------------------------------*/
void SwTokenWindow::AdjustPositions()
{
    if(aControlList.Count() > 1)
    {
        Control* pCtrl = aControlList.First();
     Point aNextPos = pCtrl->GetPosPixel();
        aNextPos.X() += pCtrl->GetSizePixel().Width();
        while(0 != (pCtrl = aControlList.Next()))
        {
            pCtrl->SetPosPixel(aNextPos);
            aNextPos.X() += pCtrl->GetSizePixel().Width();
        }
        AdjustScrolling();
    }
};
/* -----------------------------16.08.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
void SwTokenWindow::MoveControls(long nOffset)
{
    // move the complete list
    Control* pCtrl = aControlList.First();
    do
    {
        Point aPos = pCtrl->GetPosPixel();
        aPos.X() += nOffset;
        pCtrl->SetPosPixel(aPos);
    }while(0 != (pCtrl = aControlList.Next()));
}
/* -----------------------------14.01.00 13:03--------------------------------

 ---------------------------------------------------------------------------*/
void SwTokenWindow::AdjustScrolling()
{
    if(aControlList.Count() > 1)
    {
        //validate scroll buttons
        Control* pLastCtrl = aControlList.Last();
        Control* pFirstCtrl = aControlList.First();
        long nSpace = aCtrlParentWin.GetSizePixel().Width();
        long nWidth = pLastCtrl->GetPosPixel().X() - pFirstCtrl->GetPosPixel().X()
                                                    + pLastCtrl->GetSizePixel().Width();
        sal_Bool bEnable = nWidth > nSpace;
        //the active control must be visible
        if(bEnable && pActiveCtrl)
        {
             Point aActivePos(pActiveCtrl->GetPosPixel());
            long nMove = 0;
            if(aActivePos.X() < 0)
                nMove = -aActivePos.X();
            else if((aActivePos.X() + pActiveCtrl->GetSizePixel().Width())  > nSpace)
                nMove = -(aActivePos.X() + pActiveCtrl->GetSizePixel().Width() - nSpace);
            if(nMove)
                MoveControls(nMove);
            aLeftScrollWin.Enable(aControlList.First()->GetPosPixel().X() < 0);
            Control* pCtrl = aControlList.Last();
            aRightScrollWin.Enable((pCtrl->GetPosPixel().X() + pCtrl->GetSizePixel().Width()) > nSpace);
        }
        else
        {
            if(pFirstCtrl)
            {
                //if the control fits into the space then the first control must be at postion 0
                long nFirstPos = pFirstCtrl->GetPosPixel().X();
                if(nFirstPos != 0)
                    MoveControls(-nFirstPos);
            }
            aRightScrollWin.Enable(sal_False);
            aLeftScrollWin.Enable(sal_False);
        }
    }
}
/* -----------------------------14.01.00 13:57--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTokenWindow, ScrollHdl, ImageButton*, pBtn )
{
    if(aControlList.Count())
    {
        const long nSpace = aCtrlParentWin.GetSizePixel().Width();
#ifdef DEBUG
    //find all start/end positions and print it
    String sMessage(String::CreateFromAscii("Space: "));
    sMessage += String::CreateFromInt32(nSpace);
    sMessage += String::CreateFromAscii(" | ");
    Control* pDebugCtrl = aControlList.First();
    do
    {
        long nDebugXPos = pDebugCtrl->GetPosPixel().X();
        long nDebugWidth = pDebugCtrl->GetSizePixel().Width();
        sMessage += String::CreateFromInt32( nDebugXPos );
        sMessage += String::CreateFromAscii(" ");
        sMessage += String::CreateFromInt32(nDebugXPos + nDebugWidth);
        sMessage += String::CreateFromAscii(" | ");

    }while(0 != (pDebugCtrl = aControlList.Next()));

#endif

        long nMove = 0;
        if(pBtn == &aLeftScrollWin)
        {
            //find the first completely visible control (left edge visible)
            for(sal_uInt16 i = 0; i < aControlList.Count(); i++ )
            {
                Control* pCtrl = aControlList.GetObject(i);
                long nXPos = pCtrl->GetPosPixel().X();
                if(nXPos >= 0)
                {
                    if(!i)
                        //move the current control to the left edge
                        nMove = -nXPos;
                    else
                        //move the left neighbor to the start position
                        nMove = -aControlList.GetObject(i - 1)->GetPosPixel().X();
                    break;
                }
            }
        }
        else
        {
            //find the first completely visible control (left edge visible)
            for(sal_uInt16 i = aControlList.Count(); i; i-- )
            {
                Control* pCtrl = aControlList.GetObject(i - 1);
                long nCtrlWidth = pCtrl->GetSizePixel().Width();
                long nXPos = pCtrl->GetPosPixel().X() + nCtrlWidth;
                if(nXPos <= nSpace)
                {
                    if( i < aControlList.Count())
                    {
                        //move the right neighbor  to the right edge right aligned
                        Control* pRight = aControlList.GetObject(i);
                        nMove = nSpace - pRight->GetPosPixel().X() - pRight->GetSizePixel().Width();
                    }
                    break;
                }
            }

            //move it left until it's completely visible
        }
        if(nMove)
        {
            // move the complete list
            Control* pCtrl = aControlList.First();
            do
            {
             Point aPos = pCtrl->GetPosPixel();
                aPos.X() += nMove;
                pCtrl->SetPosPixel(aPos);
            }while(0 != (pCtrl = aControlList.Next()));
            aLeftScrollWin.Enable(aControlList.First()->GetPosPixel().X() < 0);
            pCtrl = aControlList.Last();
            aRightScrollWin.Enable((pCtrl->GetPosPixel().X() + pCtrl->GetSizePixel().Width()) > nSpace);

#ifdef DEBUG
            sMessage.AppendAscii("Move: ");
            sMessage += String::CreateFromInt32(nMove);
            GetParent()->GetParent()->GetParent()->SetText(sMessage);
#endif
        }
    }
    return 0;
}
/* -----------------17.06.99 11:59-------------------

 --------------------------------------------------*/
String  SwTokenWindow::GetPattern() const
{
    String sRet;
    const Control* pControl = ((SwTokenWindow*)this)->aControlList.First();
    while(pControl)
    {
        const SwFormToken& rNewToken = WINDOW_EDIT == pControl->GetType()
                    ? ((SwTOXEdit*)pControl)->GetFormToken()
                    : ((SwTOXButton*)pControl)->GetFormToken();

        //TODO: prevent input of TOX_STYLE_DELIMITER in KeyInput
        sRet += rNewToken.GetString();

        pControl = ((SwTokenWindow*)this)->aControlList.Next();
    }
    return sRet;
}
/* -----------------19.08.99 11:27-------------------
    Description: Check if a control of the specified
                    TokenType is already contained in the list
 --------------------------------------------------*/
sal_Bool SwTokenWindow::Contains(FormTokenType eSearchFor) const
{
    sal_Bool bRet = sal_False;
    const Control* pControl = ((SwTokenWindow*)this)->aControlList.First();
    while(pControl)
    {
        const SwFormToken& rNewToken = WINDOW_EDIT == pControl->GetType()
                    ? ((SwTOXEdit*)pControl)->GetFormToken()
                    : ((SwTOXButton*)pControl)->GetFormToken();

        if(eSearchFor == rNewToken.eTokenType)
        {
            bRet = sal_True;
            break;
        }
        pControl = ((SwTokenWindow*)this)->aControlList.Next();
    }
    return bRet;
}
//---------------------------------------------------
BOOL SwTokenWindow::CreateQuickHelp(Control* pCtrl,
            const SwFormToken& rToken,
            const HelpEvent& rHEvt)
{
    BOOL bRet = FALSE;
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        sal_Bool bBalloon = Help::IsBalloonHelpEnabled();
        String sEntry;
        if(bBalloon || rToken.eTokenType != TOKEN_AUTHORITY)
            sEntry = (aButtonHelpTexts[rToken.eTokenType]);
        if(rToken.eTokenType == TOKEN_AUTHORITY )
        {
             sEntry += SwAuthorityFieldType::GetAuthFieldName(
                                (ToxAuthorityField) rToken.nAuthorityField);
        }

     Point aPos = OutputToScreenPixel(pCtrl->GetPosPixel());
     Rectangle aItemRect( aPos, pCtrl->GetSizePixel() );
        if(rToken.eTokenType == TOKEN_TAB_STOP )
        {
//          sEntry += '\n';
//          sEntry += rToken.nTabStopPosition;
        }
        else
        {
            if(rToken.sCharStyleName.Len())
            {
                if(bBalloon)
                    sEntry += '\n';
                else
                    sEntry += ' ';
                sEntry += sCharStyle;
                  sEntry += rToken.sCharStyleName;
            }
        }
        if(bBalloon)
        {
            Help::ShowBalloon( this, aPos, aItemRect, sEntry );
        }
        else
            Help::ShowQuickHelp( this, aItemRect, sEntry,
                QUICKHELP_LEFT|QUICKHELP_VCENTER );
        bRet = TRUE;
    }
    return bRet;
}
/* -----------------------------14.01.00 12:22--------------------------------

 ---------------------------------------------------------------------------*/
void SwTokenWindow::Resize()
{
 Size aCompleteSize(GetOutputSizePixel());

 Point aRightPos(aRightScrollWin.GetPosPixel());
 Size aRightSize(aRightScrollWin.GetSizePixel());

 Size aMiddleSize(aCtrlParentWin.GetSizePixel());

    long nMove = aCompleteSize.Width() - aRightSize.Width() - aRightPos.X();

    aRightPos.X() += nMove;
    aRightScrollWin.SetPosPixel(aRightPos);
    aMiddleSize.Width() += nMove;
    aCtrlParentWin.SetSizePixel(aMiddleSize);
}

/* -----------------16.06.99 15:23-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTokenWindow, EditResize, Edit*, pEdit)
{
    ((SwTOXEdit*)pEdit)->AdjustSize();
    AdjustPositions();
    if(aModifyHdl.IsSet())
        aModifyHdl.Call(0);
    return 0;
}
/* -----------------16.06.99 15:56-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTokenWindow, NextItemHdl, SwTOXEdit*,  pEdit)
{
    sal_uInt16 nPos = (sal_uInt16)aControlList.GetPos(pEdit);
    if(nPos && !pEdit->IsNextControl() ||
        nPos < aControlList.Count() - 1 && pEdit->IsNextControl())
    {
        aControlList.Seek(nPos);
        Control* pNextPrev = pEdit->IsNextControl() ? aControlList.Next() : aControlList.Prev();
        nPos += pEdit->IsNextControl() ? 1 : -1;
        pNextPrev->GrabFocus();
        ((SwTOXButton*)pNextPrev)->Check();
        AdjustScrolling();
    }
    return 0;
}
/* -----------------17.06.99 08:58-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTokenWindow, TbxFocusHdl, SwTOXEdit*, pEdit)
{
    for(sal_uInt16 i = 0; i < aControlList.Count(); i++)
    {
        Control* pControl = aControlList.First();
        while(pControl)
        {
            if(WINDOW_EDIT != pControl->GetType() )
                ((SwTOXButton*)pControl)->Check(sal_False);
            pControl = aControlList.Next();
        }
    }
    SetActiveControl(pEdit);
    return 0;
}
/* -----------------17.06.99 10:05-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTokenWindow, NextItemBtnHdl, SwTOXButton*, pBtn )
{
    sal_uInt16 nPos = (sal_uInt16)aControlList.GetPos(pBtn);
    if(nPos && !pBtn->IsNextControl() ||
        nPos < aControlList.Count() - 1 && pBtn->IsNextControl())
    {
        aControlList.Seek(nPos);
        sal_Bool bNext = pBtn->IsNextControl();
        Control* pNextPrev = bNext ? aControlList.Next() : aControlList.Prev();
        pNextPrev->GrabFocus();
         Selection aSel(0, 0);
        if(!bNext)
        {
            sal_uInt16 nLen = ((SwTOXEdit*)pNextPrev)->GetText().Len();
            aSel.A() = nLen;
            aSel.B() = nLen;
        }
        ((SwTOXEdit*)pNextPrev)->SetSelection(aSel);
        pBtn->Check(sal_False);
        AdjustScrolling();
    }
    return 0;
}

/* -----------------17.06.99 10:04-------------------

 --------------------------------------------------*/
IMPL_LINK(SwTokenWindow, TbxFocusBtnHdl, SwTOXButton*, pBtn )
{
    for(sal_uInt16 i = 0; i < aControlList.Count(); i++)
    {
        Control* pControl = aControlList.First();
        while(pControl)
        {
            if(WINDOW_EDIT != pControl->GetType() )
                ((SwTOXButton*)pControl)->Check(pBtn == pControl);
            pControl = aControlList.Next();
        }
    }
    SetActiveControl(pBtn);
    return 0;
}

/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
SwTOXStylesTabPage::SwTOXStylesTabPage(Window* pParent, const SfxItemSet& rAttrSet ) :
    SfxTabPage(pParent, SW_RES(TP_TOX_STYLES), rAttrSet),
    aLevelFT2(this,     SW_RES(FT_LEVEL  )),
    aLevelLB(this,      SW_RES(LB_LEVEL  )),
    aTemplateFT(this,   SW_RES(FT_TEMPLATE)),
    aParaLayLB(this,    SW_RES(LB_PARALAY )),
    aStdBT(this,        SW_RES(BT_STD    )),
    aAssignBT(this,     SW_RES(BT_ASSIGN  )),
    aEditStyleBT(this,  SW_RES(BT_EDIT_STYLE    )),
    aFormatGB(this,     SW_RES(GB_FORMAT  )),
    pCurrentForm(0)
{
    FreeResource();
    SetExchangeSupport( sal_True );

    aEditStyleBT.SetClickHdl   (LINK(   this, SwTOXStylesTabPage, EditStyleHdl));
    aAssignBT.SetClickHdl      (LINK(   this, SwTOXStylesTabPage, AssignHdl));
    aStdBT.SetClickHdl         (LINK(   this, SwTOXStylesTabPage, StdHdl));
    aParaLayLB.SetSelectHdl    (LINK(   this, SwTOXStylesTabPage, EnableSelectHdl));
    aLevelLB.SetSelectHdl      (LINK(   this, SwTOXStylesTabPage, EnableSelectHdl));
    aParaLayLB.SetDoubleClickHdl(LINK(  this, SwTOXStylesTabPage, DoubleClickHdl));
}
/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
SwTOXStylesTabPage::~SwTOXStylesTabPage()
{
    delete pCurrentForm;
}
/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
sal_Bool    SwTOXStylesTabPage::FillItemSet( SfxItemSet& )
{
    return sal_True;
}
/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
void    SwTOXStylesTabPage::Reset( const SfxItemSet& rSet )
{
    ActivatePage(rSet);
}
/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
void    SwTOXStylesTabPage::ActivatePage( const SfxItemSet& )
{
    pCurrentForm = new SwForm(GetForm());
    aParaLayLB.Clear();
    aLevelLB.Clear();

    // kein Hyperlink fuer Benutzerverzeichnisse

    sal_uInt16 i, nSize = pCurrentForm->GetFormMax();

    // form-Pattern anzeigen ohne Titel

    //1. TemplateEntry  anzeigen
    String aStr( SW_RES( STR_TITLE ));
    if( pCurrentForm->GetTemplate( 0 ).Len() )
    {
        aStr += ' ';
        aStr += aDeliStart;
        aStr += pCurrentForm->GetTemplate( 0 );
        aStr += aDeliEnd;
    }
    aLevelLB.InsertEntry(aStr);

    for( i=1; i < nSize; ++i )
    {
        if( TOX_INDEX == pCurrentForm->GetTOXType() &&
            FORM_ALPHA_DELIMITTER == i )
            aStr = SW_RESSTR(STR_ALPHA);
        else
        {
            aStr  = SW_RESSTR(STR_LEVEL);
            aStr += String::CreateFromInt32(
                    TOX_INDEX == pCurrentForm->GetTOXType() ? i - 1 : i );
        }
        String aCpy( aStr );
        aStr += ' ';
        aStr += aDeliStart;
        aStr += pCurrentForm->GetPattern(i);
        aStr += aDeliEnd;

        if( pCurrentForm->GetTemplate( i ).Len() )
        {
            aCpy += ' ';
            aCpy += aDeliStart;
            aCpy += pCurrentForm->GetTemplate( i );
            aCpy += aDeliEnd;
        }
        aLevelLB.InsertEntry( aCpy );
    }

    // Templates initialisieren
    const SwTxtFmtColl *pColl;
    SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();
    const sal_uInt16 nSz = rSh.GetTxtFmtCollCount();

    for( i = 0; i < nSz; ++i )
        if( !(pColl = &rSh.GetTxtFmtColl( i ))->IsDefault() )
            aParaLayLB.InsertEntry( pColl->GetName() );

    // Pool-Collections abfragen und fuer das Verzeichnis setzen
    for( i = 0; i < pCurrentForm->GetFormMax(); ++i )
    {
        aStr = pCurrentForm->GetTemplate( i );
        if( aStr.Len() &&
            LISTBOX_ENTRY_NOTFOUND == aParaLayLB.GetEntryPos( aStr ))
            aParaLayLB.InsertEntry( aStr );
    }

    EnableSelectHdl(&aParaLayLB);
}
/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
int     SwTOXStylesTabPage::DeactivatePage( SfxItemSet* pSet  )
{
    GetForm() = *pCurrentForm;
    return LEAVE_PAGE;
}

/* -----------------25.03.99 15:17-------------------
 *
 * --------------------------------------------------*/
SfxTabPage* SwTOXStylesTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTOXStylesTabPage(pParent, rAttrSet);
}
/* -----------------26.03.99 12:47-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SwTOXStylesTabPage, EditStyleHdl, Button *, pBtn )
{
    if( LISTBOX_ENTRY_NOTFOUND != aParaLayLB.GetSelectEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, aParaLayLB.GetSelectEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA);
//      SwPtrItem aShell(FN_PARAM_WRTSHELL, pWrtShell);
        Window* pDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );
        SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(
        SID_STYLE_EDIT, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_MODAL,
            &aStyle, &aFamily/*, &aShell*/, 0L);
        Application::SetDefDialogParent( pDefDlgParent );
    }
    return 0;
}
/*--------------------------------------------------------------------
     Beschreibung: Vorlagen zuweisen
 --------------------------------------------------------------------*/
IMPL_LINK( SwTOXStylesTabPage, AssignHdl, Button *, EMPTYARG )
{
    sal_uInt16 nLevPos   = aLevelLB.GetSelectEntryPos();
    sal_uInt16 nTemplPos = aParaLayLB.GetSelectEntryPos();
    if(nLevPos   != LISTBOX_ENTRY_NOTFOUND &&
       nTemplPos != LISTBOX_ENTRY_NOTFOUND)
    {
        String aStr(aLevelLB.GetEntry(nLevPos));
        sal_uInt16 nDelPos = aStr.Search(aDeliStart);
        if(nDelPos != STRING_NOTFOUND)
            aStr.Erase(nDelPos-1);
        aStr += ' ';
        aStr += aDeliStart;
        aStr += aParaLayLB.GetSelectEntry();

        pCurrentForm->SetTemplate(nLevPos, aParaLayLB.GetSelectEntry());

        aStr += aDeliEnd;

        aLevelLB.RemoveEntry(nLevPos);
        aLevelLB.InsertEntry(aStr, nLevPos);
        aLevelLB.SelectEntry(aStr);
        ModifyHdl(0);
    }
    return 0;
}
/* -----------------26.03.99 09:10-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SwTOXStylesTabPage, StdHdl, Button *, EMPTYARG )
{
    sal_uInt16 nPos = aLevelLB.GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
    {   String aStr(aLevelLB.GetEntry(nPos));
        sal_uInt16 nDelPos = aStr.Search(aDeliStart);
        if(nDelPos != STRING_NOTFOUND)
            aStr.Erase(nDelPos-1);
        aLevelLB.RemoveEntry(nPos);
        aLevelLB.InsertEntry(aStr, nPos);
        aLevelLB.SelectEntry(aStr);
        pCurrentForm->SetTemplate(nPos, aEmptyStr);
        ModifyHdl(0);
    }
    return 0;
}
/* -----------------26.03.99 09:11-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK_INLINE_START( SwTOXStylesTabPage, DoubleClickHdl, Button *, EMPTYARG )
{
    String aTmpName( aParaLayLB.GetSelectEntry() );
    SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();

    if(aParaLayLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
       (aLevelLB.GetSelectEntryPos() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)))
        AssignHdl(&aAssignBT);
    return 0;
}
IMPL_LINK_INLINE_END( SwTOXStylesTabPage, DoubleClickHdl, Button *, EMPTYARG )
/*--------------------------------------------------------------------
     Beschreibung: nur wenn selektiert enable
 --------------------------------------------------------------------*/
IMPL_LINK( SwTOXStylesTabPage, EnableSelectHdl, ListBox *, EMPTYARG )
{
    //UpdatePattern();
    aStdBT.Enable(aLevelLB.GetSelectEntryPos()  != LISTBOX_ENTRY_NOTFOUND);

    SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();
    String aTmpName(aParaLayLB.GetSelectEntry());
    aAssignBT.Enable(aParaLayLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
                     LISTBOX_ENTRY_NOTFOUND != aLevelLB.GetSelectEntryPos() &&
       (aLevelLB.GetSelectEntryPos() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)));
    aEditStyleBT.Enable(aParaLayLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
    return 0;
}
/* -----------------------------18.01.00 16:54--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTOXStylesTabPage, ModifyHdl, void*, EMPTYARG)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    if(pTOXDlg)
    {
        GetForm() = *pCurrentForm;
        pTOXDlg->CreateOrUpdateExample(pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_STYLES);
    }
    return 0;
}
/******************************************************************************

******************************************************************************/
#define ITEM_SEARCH         1
#define ITEM_ALTERNATIVE    2
#define ITEM_PRIM_KEY       3
#define ITEM_SEC_KEY        4
#define ITEM_COMMENT        5
#define ITEM_CASE           6
#define ITEM_WORDONLY       7


SwEntryBrowseBox::SwEntryBrowseBox(Window* pParent, const ResId& rId,
                                                       BrowserMode nMode ) :
            DbBrowseBox( pParent, rId, nMode,
                           BROWSER_KEEPSELECTION |
                           BROWSER_COLUMNSELECTION |
                           BROWSER_MULTISELECTION |
                           BROWSER_TRACKING_TIPS |
                           BROWSER_HLINESFULL |
                           BROWSER_VLINESFULL |
                           BROWSER_AUTO_VSCROLL|
                           BROWSER_HIDECURSOR   ),
            sSearch(        ResId(ST_SEARCH         )),
            sAlternative(   ResId(ST_ALTERNATIVE    )),
            sPrimKey(       ResId(ST_PRIMKEY        )),
            sSecKey(        ResId(ST_SECKEY         )),
            sComment(       ResId(ST_COMMENT        )),
            sCaseSensitive( ResId(ST_CASESENSITIVE  )),
            sWordOnly(      ResId(ST_WORDONLY       )),
            sYes(           ResId(ST_TRUE           )),
            sNo(            ResId(ST_FALSE          )),
            aCellEdit(&GetDataWindow(), 0),
            aCellCheckBox(&GetDataWindow()),
            bModified(sal_False)
{
    FreeResource();
    aCellCheckBox.GetBox().EnableTriState(sal_False);
    xController = new DbEditCellController(&aCellEdit);
    xCheckController = new DbCheckBoxCellController(&aCellCheckBox);

    //////////////////////////////////////////////////////////////////////
    // HACK: BrowseBox invalidiert nicht ihre Childs, wie es eigentlich sein sollte.
    // Deshalb wird WB_CLIPCHILDREN zurueckgesetzt, wodurch das Invalidieren
    // der Childs erzwungen wird.
    WinBits aStyle = GetStyle();
    if( aStyle & WB_CLIPCHILDREN )
    {
        aStyle &= ~WB_CLIPCHILDREN;
        SetStyle( aStyle );
    }
    const String* aTitles[7] =
    {
        &sSearch,
        &sAlternative,
        &sPrimKey,
        &sSecKey,
        &sComment,
        &sCaseSensitive,
        &sWordOnly
    };

    long nWidth = GetSizePixel().Width();
    nWidth /=7;
    --nWidth;
    for(sal_uInt16 i = 1; i < 8; i++)
        InsertDataColumn( i, *aTitles[i - 1], nWidth,
                          HIB_STDSTYLE, HEADERBAR_APPEND );

}
/* -----------------------------19.01.00 11:29--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwEntryBrowseBox::SeekRow( long nRow )
{
    nCurrentRow = nRow;
    return TRUE;
}
/* -----------------------------19.01.00 15:32--------------------------------

 ---------------------------------------------------------------------------*/
const String& SwEntryBrowseBox::GetCellText(long nRow, long nColumn) const
{
    const String* pRet = &aEmptyStr;
    if(aEntryArr.Count() > nRow)
    {
        AutoMarkEntry* pEntry = aEntryArr[nRow];
        switch(nColumn)
        {
            case  ITEM_SEARCH       :pRet = &pEntry->sSearch; break;
            case  ITEM_ALTERNATIVE  :pRet = &pEntry->sAlternative; break;
            case  ITEM_PRIM_KEY     :pRet = &pEntry->sPrimKey   ; break;
            case  ITEM_SEC_KEY      :pRet = &pEntry->sSecKey    ; break;
            case  ITEM_COMMENT      :pRet = &pEntry->sComment   ; break;
            case  ITEM_CASE         :pRet = pEntry->bCase ? &sYes : &sNo; break;
            case  ITEM_WORDONLY     :pRet = pEntry->bWord ? &sYes : &sNo; break;
        }
    }
    return *pRet;
}

/* -----------------------------19.01.00 11:29--------------------------------

 ---------------------------------------------------------------------------*/
void    SwEntryBrowseBox::PaintCell(OutputDevice& rDev,
                                const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    const String& rPaint = GetCellText( nCurrentRow, nColumnId );
    sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
    rDev.DrawText( rRect, rPaint, nStyle );
}
/* -----------------------------19.01.00 14:51--------------------------------

 ---------------------------------------------------------------------------*/
DbCellController* SwEntryBrowseBox::GetController(long nRow, sal_uInt16 nCol)
{
    return nCol < ITEM_CASE ? xController : xCheckController;
}
/* -----------------------------19.01.00 15:36--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwEntryBrowseBox::SaveModified()
{
    SetModified();
    long nRow = GetCurRow();
    sal_uInt16 nCol = GetCurColumnId();

    String sNew;
    sal_Bool bVal = sal_False;
    DbCellController* pController = 0;
    if(nCol < ITEM_CASE)
    {
        pController = xController;
        sNew = ((DbEditCellController*)pController)->GetEditWindow().GetText();
    }
    else
    {
        pController = xCheckController;
        bVal = ((DbCheckBoxCellController*)pController)->GetCheckBox().IsChecked();
    }
    AutoMarkEntry* pEntry = nRow >= aEntryArr.Count() ? new AutoMarkEntry
                                                      : aEntryArr[nRow];
    switch(nCol)
    {
        case  ITEM_SEARCH       : pEntry->sSearch = sNew; break;
        case  ITEM_ALTERNATIVE  : pEntry->sAlternative = sNew; break;
        case  ITEM_PRIM_KEY     : pEntry->sPrimKey   = sNew; break;
        case  ITEM_SEC_KEY      : pEntry->sSecKey    = sNew; break;
        case  ITEM_COMMENT      : pEntry->sComment   = sNew; break;
        case  ITEM_CASE         : pEntry->bCase = bVal; break;
        case  ITEM_WORDONLY     : pEntry->bWord = bVal; break;
    }
    if(nRow >= aEntryArr.Count())
    {
        aEntryArr.Insert( pEntry, aEntryArr.Count() );
        RowInserted(nRow, 1, sal_True, sal_True);
        if(nCol < ITEM_WORDONLY)
        {
            pController->ClearModified();
            GoToRow( nRow );
        }
    }
    return sal_True;
}
/* -----------------------------19.01.00 14:32--------------------------------

 ---------------------------------------------------------------------------*/
void    SwEntryBrowseBox::InitController(
                DbCellControllerRef& rController, long nRow, sal_uInt16 nCol)
{
    const String& rTxt = GetCellText( nRow, nCol );
    if(nCol < ITEM_CASE)
    {
        rController = xController;
        DbCellController* pController = xController;
        ((DbEditCellController*)pController)->GetEditWindow().SetText( rTxt );
    }
    else
    {
        rController = xCheckController;
        DbCellController* pController = xCheckController;
        ((DbCheckBoxCellController*)pController)->GetCheckBox().Check(
                                                            rTxt == sYes );
     }
}
/* -----------------------------19.01.00 12:19--------------------------------

 ---------------------------------------------------------------------------*/
void    SwEntryBrowseBox::ReadEntries(SvStream& rInStr)
{
    AutoMarkEntry* pToInsert = 0;
    const String sZero('0');
    rtl_TextEncoding  eTEnc = gsl_getSystemTextEncoding();
    while( !rInStr.GetError() && !rInStr.IsEof() )
    {
        String sLine;
        rInStr.ReadByteStringLine( sLine, eTEnc );

        // # -> comment
        // ; -> delimiter between entries ->
        // Format: TextToSearchFor;AlternativeString;PrimaryKey;SecondaryKey
        // Leading and trailing blanks are ignored
        if( sLine.Len() )
        {
            //comments are contained in separate lines but are put into the struct of the following data
            //line (if available)
            if( '#' != sLine.GetChar(0) )
            {
                if( !pToInsert )
                    pToInsert = new AutoMarkEntry;

                USHORT nSttPos = 0;
                pToInsert->sSearch      = sLine.GetToken(0, ';', nSttPos );
                pToInsert->sAlternative = sLine.GetToken(0, ';', nSttPos );
                pToInsert->sPrimKey     = sLine.GetToken(0, ';', nSttPos );
                pToInsert->sSecKey      = sLine.GetToken(0, ';', nSttPos );

                String sStr = sLine.GetToken(0, ';', nSttPos );
                pToInsert->bCase = sStr.Len() && sStr != sZero;

                sStr = sLine.GetToken(0, ';', nSttPos );
                pToInsert->bWord = sStr.Len() && sStr != sZero;

                aEntryArr.Insert( pToInsert, aEntryArr.Count() );
                pToInsert = 0;
            }
            else
            {
                if(pToInsert)
                    aEntryArr.Insert(pToInsert, aEntryArr.Count());
                pToInsert = new AutoMarkEntry;
                pToInsert->sComment = sLine;
                pToInsert->sComment.Erase(0, 1);
            }
        }
    }
    if( pToInsert )
        aEntryArr.Insert(pToInsert, aEntryArr.Count());
    RowInserted(0, aEntryArr.Count() + 1, sal_True);
}
/* -----------------------------19.01.00 12:19--------------------------------

 ---------------------------------------------------------------------------*/
void    SwEntryBrowseBox::WriteEntries(SvStream& rOutStr)
{
    //check if the current controller is modified
    sal_uInt16 nCol = GetCurColumnId();
    DbCellController* pController;
    if(nCol < ITEM_CASE)
        pController = xController;
    else
        pController = xCheckController;
    if(pController ->IsModified())
        GoToColumnId(nCol < ITEM_CASE ? ++nCol : --nCol );

    rtl_TextEncoding  eTEnc = gsl_getSystemTextEncoding();
    for(sal_uInt16 i = 0; i < aEntryArr.Count();i++)
    {
        AutoMarkEntry* pEntry = aEntryArr[i];
        if(pEntry->sComment.Len())
        {
            String sWrite('#');
            sWrite += pEntry->sComment;
            rOutStr.WriteByteStringLine( sWrite, eTEnc );
        }

        String sWrite( pEntry->sSearch );
        sWrite += ';';
        sWrite += pEntry->sAlternative;
        sWrite += ';';
        sWrite += pEntry->sPrimKey;
        sWrite += ';';
        sWrite += pEntry->sSecKey;
        sWrite += ';';
        sWrite += pEntry->bCase ? '1' : '0';
        sWrite += ';';
        sWrite += pEntry->bWord ? '1' : '0';

        if( sWrite.Len() > 5 )
            rOutStr.WriteByteStringLine( sWrite, eTEnc );
    }
}
/* -----------------------------21.01.00 11:49--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwEntryBrowseBox::IsModified()const
{
    if(bModified)
        return sal_True;


    //check if the current controller is modified
    sal_uInt16 nCol = GetCurColumnId();
    DbCellController* pController;
    if(nCol < ITEM_CASE)
        pController = xController;
    else
        pController = xCheckController;
    return pController ->IsModified();
}
/* -----------------------------19.01.00 11:29--------------------------------

 ---------------------------------------------------------------------------*/
SwAutoMarkDlg_Impl::SwAutoMarkDlg_Impl(Window* pParent, const String& rAutoMarkURL,
        const String& rAutoMarkType, sal_Bool bCreate) :
    ModalDialog(pParent, SW_RES(DLG_CREATE_AUTOMARK)),
    aOKPB(      this, ResId(PB_OK       )),
    aCancelPB(  this, ResId(PB_CANCEL   )),
    aHelpPB(    this, ResId(PB_HELP     )),
    aEntriesBB( this, ResId(BB_ENTRIES  )),
    aEntriesGB( this, ResId(GB_ENTRIES  )),
    sAutoMarkURL(rAutoMarkURL),
    sAutoMarkType(rAutoMarkType),
    bCreateMode(bCreate)
{
    FreeResource();
    aOKPB.SetClickHdl(LINK(this, SwAutoMarkDlg_Impl, OkHdl));

    INetURLObject aURLObj( sAutoMarkURL );
    sAutoMarkURL = aURLObj.PathToFileName();
    String sTitle = GetText();
    sTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM(": "));
    sTitle += sAutoMarkURL;
    SetText(sTitle);
    sal_Bool bError = sal_False;
    if( bCreateMode )
        aEntriesBB.RowInserted(0, 1, sal_True);
    else
    {
        SfxMedium aMed( sAutoMarkURL, STREAM_STD_READ, FALSE );
        if( !aMed.GetInStream()->GetError() )
            aEntriesBB.ReadEntries( *aMed.GetInStream() );
        else
            bError = sal_True;
    }

    if(bError)
        EndDialog(RET_CANCEL);
}
/* -----------------------------19.01.00 11:12--------------------------------

 ---------------------------------------------------------------------------*/
SwAutoMarkDlg_Impl::~SwAutoMarkDlg_Impl()
{
}
/* -----------------------------19.01.00 16:43--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwAutoMarkDlg_Impl, OkHdl, OKButton*, pButton)
{
    sal_Bool bError = sal_False;
    if(aEntriesBB.IsModified() || bCreateMode)
    {
        SfxMedium aMed( sAutoMarkURL,
                        bCreateMode ? STREAM_WRITE
                                    : STREAM_WRITE| STREAM_TRUNC,
                        FALSE );
        SvStream* pStrm = aMed.GetOutStream();
        pStrm->SetStreamCharSet( RTL_TEXTENCODING_MS_1253 );
        if( !pStrm->GetError() )
        {
            aEntriesBB.WriteEntries( *pStrm );
            aMed.Commit();
        }
        else
            bError = sal_True;
    }
    if( !bError )
        EndDialog(RET_OK);
    return 0;
}

