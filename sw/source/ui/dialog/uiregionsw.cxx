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

#include <hintids.hxx>
#include <regionsw.hxx>
#include <svl/urihelper.hxx>
#include <svl/PasswordHelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <editeng/sizeitem.hxx>
#include <svtools/htmlcfg.hxx>

#include <comphelper/storagehelper.hxx>
#include <uitool.hxx>
#include <IMark.hxx>
#include <section.hxx>
#include <docary.hxx>
#include <doc.hxx>                      // for the SwSectionFmt-Array
#include <basesh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>                   // for Undo-Ids
#include <column.hxx>
#include <fmtfsize.hxx>
#include <shellio.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <regionsw.hrc>
#include <comcore.hrc>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <svx/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>

using namespace ::com::sun::star;

static void   lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox );

static void lcl_FillList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames, const SwSectionFmt* pNewFmt )
{
    const SwSectionFmt* pFmt;
    if( !pNewFmt )
    {
        sal_uInt16 nCount = rSh.GetSectionFmtCount();
        for(sal_uInt16 i=0;i<nCount;i++)
        {
            SectionType eTmpType;
            if( !(pFmt = &rSh.GetSectionFmt(i))->GetParent() &&
                    pFmt->IsInNodesArr() &&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
            {
                    String* pString =
                        new String(pFmt->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(*pString);
                    rSubRegions.InsertEntry(*pString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFmt );
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        sal_uInt16 nCnt = pNewFmt->GetChildSections(aTmpArr,SORTSECT_POS);
        if( nCnt )
        {
            SectionType eTmpType;
            for( sal_uInt16 n = 0; n < nCnt; ++n )
                if( (pFmt = aTmpArr[n]->GetFmt())->IsInNodesArr()&&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    String* pString =
                        new String(pFmt->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(*pString);
                    rSubRegions.InsertEntry(*pString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFmt );
                }
        }
    }
}

static void lcl_FillSubRegionList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames )
{
    lcl_FillList( rSh, rSubRegions, pAvailNames, 0 );
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
        ppMark != pMarkAccess->getMarksEnd();
        ++ppMark)
    {
        const ::sw::mark::IMark* pBkmk = ppMark->get();
        if( pBkmk->IsExpanded() )
            rSubRegions.InsertEntry( pBkmk->GetName() );
    }
}

/*----------------------------------------------------------------------------
 Description: user data class for region information
----------------------------------------------------------------------------*/

class SectRepr
{
private:
    SwSectionData           m_SectionData;
    SwFmtCol                m_Col;
    SvxBrushItem            m_Brush;
    SwFmtFtnAtTxtEnd        m_FtnNtAtEnd;
    SwFmtEndAtTxtEnd        m_EndNtAtEnd;
    SwFmtNoBalancedColumns  m_Balance;
    SvxFrameDirectionItem   m_FrmDirItem;
    SvxLRSpaceItem          m_LRSpaceItem;
    sal_uInt16                  m_nArrPos;
    // shows, if maybe textcontent is in the region
    bool                    m_bContent  : 1;
    // for multiselection, mark at first, then work with TreeListBox!
    bool                    m_bSelected : 1;
    uno::Sequence<sal_Int8> m_TempPasswd;

public:
    SectRepr(sal_uInt16 nPos, SwSection& rSect);
    bool    operator==(const SectRepr& rSectRef) const
            { return m_nArrPos == rSectRef.GetArrPos(); }

    bool    operator< (const SectRepr& rSectRef) const
            { return m_nArrPos <  rSectRef.GetArrPos(); }

    SwSectionData &     GetSectionData()        { return m_SectionData; }
    SwSectionData const&GetSectionData() const  { return m_SectionData; }
    SwFmtCol&               GetCol()            { return m_Col; }
    SvxBrushItem&           GetBackground()     { return m_Brush; }
    SwFmtFtnAtTxtEnd&       GetFtnNtAtEnd()     { return m_FtnNtAtEnd; }
    SwFmtEndAtTxtEnd&       GetEndNtAtEnd()     { return m_EndNtAtEnd; }
    SwFmtNoBalancedColumns& GetBalance()        { return m_Balance; }
    SvxFrameDirectionItem&  GetFrmDir()         { return m_FrmDirItem; }
    SvxLRSpaceItem&         GetLRSpace()        { return m_LRSpaceItem; }

    sal_uInt16              GetArrPos() const { return m_nArrPos; }
    String              GetFile() const;
    String              GetSubRegion() const;
    void                SetFile(String const& rFile);
    void                SetFilter(String const& rFilter);
    void                SetSubRegion(String const& rSubRegion);

    bool                IsContent() { return m_bContent; }
    void                SetContent(bool const bValue) { m_bContent = bValue; }

    void                SetSelected() { m_bSelected = true; }
    bool                IsSelected() const { return m_bSelected; }

    uno::Sequence<sal_Int8> & GetTempPasswd() { return m_TempPasswd; }
    void SetTempPasswd(const uno::Sequence<sal_Int8> & rPasswd)
        { m_TempPasswd = rPasswd; }
};


SectRepr::SectRepr( sal_uInt16 nPos, SwSection& rSect )
    : m_SectionData( rSect )
    , m_Brush( RES_BACKGROUND )
    , m_FrmDirItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR )
    , m_LRSpaceItem( RES_LR_SPACE )
    , m_nArrPos(nPos)
    , m_bContent(m_SectionData.GetLinkFileName().Len() == 0)
    , m_bSelected(false)
{
    SwSectionFmt *pFmt = rSect.GetFmt();
    if( pFmt )
    {
        m_Col = pFmt->GetCol();
        m_Brush = pFmt->GetBackground();
        m_FtnNtAtEnd = pFmt->GetFtnAtTxtEnd();
        m_EndNtAtEnd = pFmt->GetEndAtTxtEnd();
        m_Balance.SetValue(pFmt->GetBalancedColumns().GetValue());
        m_FrmDirItem = pFmt->GetFrmDir();
        m_LRSpaceItem = pFmt->GetLRSpace();
    }
}

void SectRepr::SetFile( const String& rFile )
{
    String sNewFile( INetURLObject::decode( rFile, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sSub( sOldFileName.GetToken( 2, sfx2::cTokenSeperator ) );

    if( rFile.Len() || sSub.Len() )
    {
        sNewFile += sfx2::cTokenSeperator;
        if( rFile.Len() ) // Filter only with FileName
            sNewFile += sOldFileName.GetToken( 1, sfx2::cTokenSeperator );

        sNewFile += sfx2::cTokenSeperator;
        sNewFile += sSub;
    }

    m_SectionData.SetLinkFileName( sNewFile );

    if( rFile.Len() || sSub.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}


void SectRepr::SetFilter( const String& rFilter )
{
    String sNewFile;
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sFile( sOldFileName.GetToken( 0, sfx2::cTokenSeperator ) );
    String sSub( sOldFileName.GetToken( 2, sfx2::cTokenSeperator ) );

    if( sFile.Len() )
        (((( sNewFile = sFile ) += sfx2::cTokenSeperator ) += rFilter )
                                += sfx2::cTokenSeperator ) += sSub;
    else if( sSub.Len() )
        (( sNewFile = sfx2::cTokenSeperator ) += sfx2::cTokenSeperator ) += sSub;

    m_SectionData.SetLinkFileName( sNewFile );

    if( sNewFile.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
}

void SectRepr::SetSubRegion(const String& rSubRegion)
{
    String sNewFile;
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sFilter( sOldFileName.GetToken( 1, sfx2::cTokenSeperator ) );
    sOldFileName = sOldFileName.GetToken( 0, sfx2::cTokenSeperator );

    if( rSubRegion.Len() || sOldFileName.Len() )
        (((( sNewFile = sOldFileName ) += sfx2::cTokenSeperator ) += sFilter )
                                       += sfx2::cTokenSeperator ) += rSubRegion;

    m_SectionData.SetLinkFileName( sNewFile );

    if( rSubRegion.Len() || sOldFileName.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}


String SectRepr::GetFile() const
{
    String sLinkFile( m_SectionData.GetLinkFileName() );
    if( sLinkFile.Len() )
    {
        if (DDE_LINK_SECTION == m_SectionData.GetType())
        {
            sal_uInt16 n = sLinkFile.SearchAndReplace( sfx2::cTokenSeperator, ' ' );
            sLinkFile.SearchAndReplace( sfx2::cTokenSeperator, ' ',  n );
        }
        else
            sLinkFile = INetURLObject::decode( sLinkFile.GetToken( 0,
                                               sfx2::cTokenSeperator ),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 );
    }
    return sLinkFile;
}


String SectRepr::GetSubRegion() const
{
    String sLinkFile( m_SectionData.GetLinkFileName() );
    if( sLinkFile.Len() )
        sLinkFile = sLinkFile.GetToken( 2, sfx2::cTokenSeperator );
    return sLinkFile;
}

/*----------------------------------------------------------------------------
 Description: dialog edit regions
----------------------------------------------------------------------------*/
SwEditRegionDlg::SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh )
    : SfxModalDialog( pParent, SW_RES(MD_EDIT_REGION) ),
    aNameFL             ( this, SW_RES( FL_NAME ) ),
    aCurName            ( this, SW_RES( ED_RANAME ) ),
    aTree               ( this, SW_RES( TLB_SECTION )),
    aLinkFL             ( this, SW_RES( FL_LINK ) ),
    aFileCB             ( this, SW_RES( CB_FILE ) ),
    aDDECB              ( this, SW_RES( CB_DDE ) ) ,
    aFileNameFT         ( this, SW_RES( FT_FILE ) ) ,
    aDDECommandFT       ( this, SW_RES( FT_DDE ) ) ,
    aFileNameED         ( this, SW_RES( ED_FILE ) ),
    aFilePB             ( this, SW_RES( PB_FILE ) ),
    aSubRegionFT        ( this, SW_RES( FT_SUBREG ) ) ,
    aSubRegionED        ( this, SW_RES( LB_SUBREG ) ) ,
    bSubRegionsFilled( false ),

    aProtectFL          ( this, SW_RES( FL_PROTECT ) ),
    aProtectCB          ( this, SW_RES( CB_PROTECT ) ),
    aPasswdCB           ( this, SW_RES( CB_PASSWD ) ),
    aPasswdPB           ( this, SW_RES( PB_PASSWD ) ),

    aHideFL             ( this, SW_RES( FL_HIDE ) ),
    aHideCB             ( this, SW_RES( CB_HIDE ) ),
    aConditionFT        ( this, SW_RES( FT_CONDITION ) ),
    aConditionED        ( this, SW_RES( ED_CONDITION ) ),

    // edit in readonly sections
    aPropertiesFL       ( this, SW_RES( FL_PROPERTIES ) ),
    aEditInReadonlyCB   ( this, SW_RES( CB_EDIT_IN_READONLY ) ),

    aOK                 ( this, SW_RES( PB_OK ) ),
    aCancel             ( this, SW_RES( PB_CANCEL ) ),
    aOptionsPB          ( this, SW_RES( PB_OPTIONS ) ),
    aDismiss            ( this, SW_RES( CB_DISMISS ) ),
    aHelp               ( this, SW_RES( PB_HELP ) ),

    aImageIL            (       SW_RES(IL_BITMAPS)),

    rSh( rWrtSh ),
    m_pDocInserter        ( NULL ),
    m_pOldDefDlgParent    ( NULL ),
    bDontCheckPasswd    ( sal_True)
{
    FreeResource();

    bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );

    aTree.SetSelectHdl      ( LINK( this, SwEditRegionDlg, GetFirstEntryHdl));
    aTree.SetDeselectHdl    ( LINK( this, SwEditRegionDlg, DeselectHdl));
    aCurName.SetModifyHdl   ( LINK( this, SwEditRegionDlg, NameEditHdl));
    aConditionED.SetModifyHdl( LINK( this, SwEditRegionDlg, ConditionEditHdl));
    aOK.SetClickHdl         ( LINK( this, SwEditRegionDlg, OkHdl));
    aPasswdCB.SetClickHdl   ( LINK( this, SwEditRegionDlg, ChangePasswdHdl));
    aPasswdPB.SetClickHdl   ( LINK( this, SwEditRegionDlg, ChangePasswdHdl));
    aHideCB.SetClickHdl     ( LINK( this, SwEditRegionDlg, ChangeHideHdl));
    // edit in readonly sections
    aEditInReadonlyCB.SetClickHdl ( LINK( this, SwEditRegionDlg, ChangeEditInReadonlyHdl));

    aOptionsPB.Show();
    aOptionsPB.SetClickHdl  ( LINK( this, SwEditRegionDlg, OptionsHdl));
    aProtectCB.SetClickHdl  ( LINK( this, SwEditRegionDlg, ChangeProtectHdl));
    aDismiss.SetClickHdl    ( LINK( this, SwEditRegionDlg, ChangeDismissHdl));
    aFileCB.SetClickHdl     ( LINK( this, SwEditRegionDlg, UseFileHdl ));
    aFilePB.SetClickHdl     ( LINK( this, SwEditRegionDlg, FileSearchHdl ));
    aFileNameED.SetModifyHdl( LINK( this, SwEditRegionDlg, FileNameHdl ));
    aSubRegionED.SetModifyHdl( LINK( this, SwEditRegionDlg, FileNameHdl ));
    aSubRegionED.AddEventListener( LINK( this, SwEditRegionDlg, SubRegionEventHdl ));
    aSubRegionED.EnableAutocomplete( sal_True, sal_True );

    aTree.SetHelpId(HID_REGION_TREE);
    aTree.SetSelectionMode( MULTIPLE_SELECTION );
    aTree.SetStyle(aTree.GetStyle()|WB_HASBUTTONSATROOT|WB_CLIPCHILDREN|WB_HSCROLL);
    aTree.SetSpaceBetweenEntries(0);

    if(bWeb)
    {
        aConditionFT         .Hide();
        aConditionED    .Hide();
        aPasswdCB       .Hide();
        aHideCB         .Hide();

        aDDECB              .Hide();
        aDDECommandFT       .Hide();
    }

    aDDECB.SetClickHdl      ( LINK( this, SwEditRegionDlg, DDEHdl ));

    pCurrSect = rSh.GetCurrSection();
    RecurseList( 0, 0 );
    // if the cursor is not in a region
    // the first one will always be selected
    if( !aTree.FirstSelected() && aTree.First() )
        aTree.Select( aTree.First() );
    aTree.Show();
    bDontCheckPasswd = sal_False;

    aPasswdPB.SetAccessibleRelationMemberOf(&aProtectFL);
    aPasswdPB.SetAccessibleRelationLabeledBy(&aPasswdCB);
    aSubRegionED.SetAccessibleName(aSubRegionFT.GetText());
}

sal_Bool SwEditRegionDlg::CheckPasswd(CheckBox* pBox)
{
    if(bDontCheckPasswd)
        return sal_True;
    sal_Bool bRet = sal_True;
    SvTreeListEntry* pEntry = aTree.FirstSelected();
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if (!pRepr->GetTempPasswd().getLength()
            && pRepr->GetSectionData().GetPassword().getLength())
        {
            SfxPasswordDialog aPasswdDlg(this);
            bRet = sal_False;
            if (aPasswdDlg.Execute())
            {
                String sNewPasswd( aPasswdDlg.GetPassword() );
                ::com::sun::star::uno::Sequence <sal_Int8 > aNewPasswd;
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if (SvPasswordHelper::CompareHashPassword(
                        pRepr->GetSectionData().GetPassword(), sNewPasswd))
                {
                    pRepr->SetTempPasswd(aNewPasswd);
                    bRet = sal_True;
                }
                else
                {
                    InfoBox(this, SW_RES(REG_WRONG_PASSWORD)).Execute();
                }
            }
        }
        pEntry = aTree.NextSelected(pEntry);
    }
    if(!bRet && pBox)
    {
        //reset old button state
        if(pBox->IsTriStateEnabled())
            pBox->SetState(pBox->IsChecked() ? STATE_NOCHECK : STATE_DONTKNOW);
        else
            pBox->Check(!pBox->IsChecked());
    }

    return bRet;
}

/*---------------------------------------------------------------------
    Description: recursively look for child-sections
---------------------------------------------------------------------*/
void SwEditRegionDlg::RecurseList( const SwSectionFmt* pFmt, SvTreeListEntry* pEntry )
{
    SwSection* pSect = 0;
    SvTreeListEntry* pSelEntry = 0;

    if (!pFmt)
    {
        sal_uInt16 nCount=rSh.GetSectionFmtCount();
        for ( sal_uInt16 n=0; n < nCount; n++ )
        {
            SectionType eTmpType;
            if( !( pFmt = &rSh.GetSectionFmt(n))->GetParent() &&
                pFmt->IsInNodesArr() &&
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
            {
                SectRepr* pSectRepr = new SectRepr( n,
                                            *(pSect=pFmt->GetSection()) );
                Image aImg = BuildBitmap( pSect->IsProtect(),pSect->IsHidden());
                pEntry = aTree.InsertEntry(pSect->GetSectionName(), aImg, aImg);
                pEntry->SetUserData(pSectRepr);
                RecurseList( pFmt, pEntry );
                if (pEntry->HasChildren())
                    aTree.Expand(pEntry);
                if (pCurrSect==pSect)
                    aTree.Select(pEntry);
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        SvTreeListEntry* pNEntry;
        sal_uInt16 nCnt = pFmt->GetChildSections(aTmpArr,SORTSECT_POS);
        if( nCnt )
        {
            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                SectionType eTmpType;
                pFmt = aTmpArr[n]->GetFmt();
                if( pFmt->IsInNodesArr() &&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    pSect=aTmpArr[n];
                    SectRepr* pSectRepr=new SectRepr(
                                    FindArrPos( pSect->GetFmt() ), *pSect );
                    Image aImage = BuildBitmap( pSect->IsProtect(),
                                            pSect->IsHidden());
                    pNEntry = aTree.InsertEntry(
                        pSect->GetSectionName(), aImage, aImage, pEntry);
                    pNEntry->SetUserData(pSectRepr);
                    RecurseList( aTmpArr[n]->GetFmt(), pNEntry );
                    if( pNEntry->HasChildren())
                        aTree.Expand(pNEntry);
                    if (pCurrSect==pSect)
                        pSelEntry = pNEntry;
                }
            }
        }
    }
    if(0 != pSelEntry)
    {
        aTree.MakeVisible(pSelEntry);
        aTree.Select(pSelEntry);
    }
}

sal_uInt16 SwEditRegionDlg::FindArrPos(const SwSectionFmt* pFmt )
{
    sal_uInt16 nCount=rSh.GetSectionFmtCount();
    for (sal_uInt16 i=0;i<nCount;i++)
        if (pFmt==&rSh.GetSectionFmt(i))
            return i;

    OSL_FAIL("SectionFormat not on the list" );
    return USHRT_MAX;
}

SwEditRegionDlg::~SwEditRegionDlg( )
{
    SvTreeListEntry* pEntry = aTree.First();
    while( pEntry )
    {
        delete (SectRepr*)pEntry->GetUserData();
        pEntry = aTree.Next( pEntry );
    }

    delete m_pDocInserter;
}

void    SwEditRegionDlg::SelectSection(const String& rSectionName)
{
    SvTreeListEntry* pEntry = aTree.First();
    while(pEntry)
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if (pRepr->GetSectionData().GetSectionName() == rSectionName)
            break;
        pEntry = aTree.Next(pEntry);
    }
    if(pEntry)
    {
        aTree.SelectAll( sal_False);
        aTree.Select(pEntry);
        aTree.MakeVisible(pEntry);
    }
}

/*---------------------------------------------------------------------
    Description:    selected entry in TreeListBox is showed in
                    Edit window
                    in case of multiselection some controls are disabled
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, GetFirstEntryHdl, SvTreeListBox *, pBox )
{
    bDontCheckPasswd = sal_True;
    SvTreeListEntry* pEntry=pBox->FirstSelected();
    aHideCB     .Enable(sal_True);
    // edit in readonly sections
    aEditInReadonlyCB.Enable(sal_True);

    aProtectCB  .Enable(sal_True);
    aFileCB     .Enable(sal_True);
    ::com::sun::star::uno::Sequence <sal_Int8> aCurPasswd;
    if( 1 < pBox->GetSelectionCount() )
    {
        aHideCB.EnableTriState( sal_True );
        aProtectCB.EnableTriState( sal_True );
        // edit in readonly sections
        aEditInReadonlyCB.EnableTriState ( sal_True );

        aFileCB.EnableTriState( sal_True );

        bool bHiddenValid       = true;
        bool bProtectValid      = true;
        bool bConditionValid    = true;
        // edit in readonly sections
        bool bEditInReadonlyValid = true;
        bool bEditInReadonly    = true;

        bool bHidden            = true;
        bool bProtect           = true;
        String sCondition;
        sal_Bool bFirst             = sal_True;
        sal_Bool bFileValid         = sal_True;
        sal_Bool bFile              = sal_True;
        sal_Bool bPasswdValid       = sal_True;

        while( pEntry )
        {
            SectRepr* pRepr=(SectRepr*) pEntry->GetUserData();
            SwSectionData const& rData( pRepr->GetSectionData() );
            if(bFirst)
            {
                sCondition      = rData.GetCondition();
                bHidden         = rData.IsHidden();
                bProtect        = rData.IsProtectFlag();
                // edit in readonly sections
                bEditInReadonly = rData.IsEditInReadonlyFlag();

                bFile           = (rData.GetType() != CONTENT_SECTION);
                aCurPasswd      = rData.GetPassword();
            }
            else
            {
                String sTemp(rData.GetCondition());
                if(sCondition != sTemp)
                    bConditionValid = sal_False;
                bHiddenValid      = (bHidden == rData.IsHidden());
                bProtectValid     = (bProtect == rData.IsProtectFlag());
                // edit in readonly sections
                bEditInReadonlyValid =
                    (bEditInReadonly == rData.IsEditInReadonlyFlag());

                bFileValid        = (bFile ==
                    (rData.GetType() != CONTENT_SECTION));
                bPasswdValid      = (aCurPasswd == rData.GetPassword());
            }
            pEntry = pBox->NextSelected(pEntry);
            bFirst = sal_False;
        }

        aHideCB.SetState( !bHiddenValid ? STATE_DONTKNOW :
                    bHidden ? STATE_CHECK : STATE_NOCHECK);
        aProtectCB.SetState( !bProtectValid ? STATE_DONTKNOW :
                    bProtect ? STATE_CHECK : STATE_NOCHECK);
        // edit in readonly sections
        aEditInReadonlyCB.SetState( !bEditInReadonlyValid ? STATE_DONTKNOW :
                    bEditInReadonly ? STATE_CHECK : STATE_NOCHECK);

        aFileCB.SetState(!bFileValid ? STATE_DONTKNOW :
                    bFile ? STATE_CHECK : STATE_NOCHECK);

        if(bConditionValid)
            aConditionED.SetText(sCondition);
        else
        {
            aConditionFT.Enable(sal_False);
            aConditionED.Enable(sal_False);
        }

        aFilePB.Enable(sal_False);
        aFileNameFT .Enable(sal_False);
        aFileNameED .Enable(sal_False);
        aSubRegionFT.Enable(sal_False);
        aSubRegionED.Enable(sal_False);
        aCurName    .Enable(sal_False);
        aOptionsPB  .Enable(sal_False);
        aDDECB              .Enable(sal_False);
        aDDECommandFT       .Enable(sal_False);
        sal_Bool bPasswdEnabled = aProtectCB.GetState() == STATE_CHECK;
        aPasswdCB.Enable(bPasswdEnabled);
        aPasswdPB.Enable(bPasswdEnabled);
        if(!bPasswdValid)
        {
            pEntry = pBox->FirstSelected();
            pBox->SelectAll( sal_False );
            pBox->Select( pEntry );
            GetFirstEntryHdl(pBox);
            return 0;
        }
        else
            aPasswdCB.Check(aCurPasswd.getLength() > 0);
    }
    else if (pEntry )
    {
        aCurName    .Enable(sal_True);
        aOptionsPB  .Enable(sal_True);
        SectRepr* pRepr=(SectRepr*) pEntry->GetUserData();
        SwSectionData const& rData( pRepr->GetSectionData() );
        aConditionED.SetText(rData.GetCondition());
        aHideCB.Enable();
        aHideCB.SetState((rData.IsHidden()) ? STATE_CHECK : STATE_NOCHECK);
        sal_Bool bHide = STATE_CHECK == aHideCB.GetState();
        aConditionED.Enable(bHide);
        aConditionFT.Enable(bHide);
        aPasswdCB.Check(rData.GetPassword().getLength() > 0);

        aOK.Enable();
        aPasswdCB.Enable();
        aCurName.SetText(pBox->GetEntryText(pEntry));
        aCurName.Enable();
        aDismiss.Enable();
        String aFile = pRepr->GetFile();
        String sSub = pRepr->GetSubRegion();
        bSubRegionsFilled = false;
        aSubRegionED.Clear();
        if(aFile.Len()||sSub.Len())
        {
            aFileCB.Check(sal_True);
            aFileNameED.SetText(aFile);
            aSubRegionED.SetText(sSub);
            aDDECB.Check(rData.GetType() == DDE_LINK_SECTION);
        }
        else
        {
            aFileCB.Check(sal_False);
            aFileNameED.SetText(aFile);
            aDDECB.Enable(sal_False);
            aDDECB.Check(sal_False);
        }
        UseFileHdl(&aFileCB);
        DDEHdl( &aDDECB );
        aProtectCB.SetState((rData.IsProtectFlag())
                ? STATE_CHECK : STATE_NOCHECK);
        aProtectCB.Enable();

        // edit in readonly sections
        aEditInReadonlyCB.SetState((rData.IsEditInReadonlyFlag())
                ? STATE_CHECK : STATE_NOCHECK);
        aEditInReadonlyCB.Enable();

        sal_Bool bPasswdEnabled = aProtectCB.IsChecked();
        aPasswdCB.Enable(bPasswdEnabled);
        aPasswdPB.Enable(bPasswdEnabled);
    }
    bDontCheckPasswd = sal_False;
    return 0;
}

IMPL_LINK( SwEditRegionDlg, DeselectHdl, SvTreeListBox *, pBox )
{
    if( !pBox->GetSelectionCount() )
    {
        aHideCB     .Enable(sal_False);
        aProtectCB  .Enable(sal_False);
        // edit in readonly sections
        aEditInReadonlyCB.Enable(sal_False);

        aPasswdCB   .Enable(sal_False);
        aPasswdCB   .Enable(sal_False);
        aConditionFT     .Enable(sal_False);
        aConditionED.Enable(sal_False);
        aFileCB     .Enable(sal_False);
        aFilePB     .Enable(sal_False);
        aFileNameFT  .Enable(sal_False);
        aFileNameED  .Enable(sal_False);
        aSubRegionFT .Enable(sal_False);
        aSubRegionED .Enable(sal_False);
        aCurName     .Enable(sal_False);
        aDDECB              .Enable(sal_False);
        aDDECommandFT       .Enable(sal_False);

        UseFileHdl(&aFileCB);
        DDEHdl( &aDDECB );
    }
    return 0;
}

/*---------------------------------------------------------------------
    Description:    in OkHdl the modified settings are being applied
                    and reversed regions are deleted
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, OkHdl)
{
    // temp. Array because during changing of a region the position
    // inside of the "Core-Arrays" can be shifted:
    //  - at linked regions, when they have more SubRegions or get
    //    new ones.
    // StartUndo must certainly also happen not before the formats
    // are copied (ClearRedo!)

    const SwSectionFmts& rDocFmts = rSh.GetDoc()->GetSections();
    SwSectionFmts aOrigArray(rDocFmts);

    rSh.StartAllAction();
    rSh.StartUndo();
    rSh.ResetSelect( 0,sal_False );
    SvTreeListEntry* pEntry = aTree.First();

    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        SwSectionFmt* pFmt = aOrigArray[ pRepr->GetArrPos() ];
        if (!pRepr->GetSectionData().IsProtectFlag())
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        sal_uInt16 nNewPos = rDocFmts.GetPos( pFmt );
        if( USHRT_MAX != nNewPos )
        {
            SfxItemSet* pSet = pFmt->GetAttrSet().Clone( sal_False );
            if( pFmt->GetCol() != pRepr->GetCol() )
                pSet->Put( pRepr->GetCol() );

            if( pFmt->GetBackground(sal_False) != pRepr->GetBackground() )
                pSet->Put( pRepr->GetBackground() );

            if( pFmt->GetFtnAtTxtEnd(sal_False) != pRepr->GetFtnNtAtEnd() )
                pSet->Put( pRepr->GetFtnNtAtEnd() );

            if( pFmt->GetEndAtTxtEnd(sal_False) != pRepr->GetEndNtAtEnd() )
                pSet->Put( pRepr->GetEndNtAtEnd() );

            if( pFmt->GetBalancedColumns() != pRepr->GetBalance() )
                pSet->Put( pRepr->GetBalance() );

            if( pFmt->GetFrmDir() != pRepr->GetFrmDir() )
                pSet->Put( pRepr->GetFrmDir() );

            if( pFmt->GetLRSpace() != pRepr->GetLRSpace())
                pSet->Put( pRepr->GetLRSpace());

            rSh.UpdateSection( nNewPos, pRepr->GetSectionData(),
                            pSet->Count() ? pSet : 0 );
            delete pSet;
        }
        pEntry = aTree.Next( pEntry );
    }

    for (SectReprArr::reverse_iterator aI = aSectReprArr.rbegin(), aEnd = aSectReprArr.rend(); aI != aEnd; ++aI)
    {
        SwSectionFmt* pFmt = aOrigArray[ aI->GetArrPos() ];
        sal_uInt16 nNewPos = rDocFmts.GetPos( pFmt );
        if( USHRT_MAX != nNewPos )
            rSh.DelSectionFmt( nNewPos );
    }

    aOrigArray.clear();

    // EndDialog must be called ahead of EndAction's end,
    // otherwise ScrollError can occur.
    EndDialog(RET_OK);

    rSh.EndUndo();
    rSh.EndAllAction();

    return 0;
}

/*---------------------------------------------------------------------
 Description: Toggle protect
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, ChangeProtectHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState( sal_False );
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    sal_Bool bCheck = STATE_CHECK == pBox->GetState();
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetProtectFlag(bCheck);
        Image aImage = BuildBitmap( bCheck,
                                    STATE_CHECK == aHideCB.GetState());
        aTree.SetExpandedEntryBmp(  pEntry, aImage );
        aTree.SetCollapsedEntryBmp( pEntry, aImage );
        pEntry = aTree.NextSelected(pEntry);
    }
    aPasswdCB.Enable(bCheck);
    aPasswdPB.Enable(bCheck);
    return 0;
}

/*---------------------------------------------------------------------
 Description: Toggle hide
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, ChangeHideHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState( sal_False );
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetHidden(STATE_CHECK == pBox->GetState());

        Image aImage = BuildBitmap(STATE_CHECK == aProtectCB.GetState(),
                                    STATE_CHECK == pBox->GetState());
        aTree.SetExpandedEntryBmp(  pEntry, aImage );
        aTree.SetCollapsedEntryBmp( pEntry, aImage );

        pEntry = aTree.NextSelected(pEntry);
    }

    sal_Bool bHide = STATE_CHECK == pBox->GetState();
    aConditionED.Enable(bHide);
    aConditionFT.Enable(bHide);
    return 0;
}

/*---------------------------------------------------------------------
 Description: Toggle edit in readonly
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, ChangeEditInReadonlyHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState( sal_False );
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetEditInReadonlyFlag(
                STATE_CHECK == pBox->GetState());
        pEntry = aTree.NextSelected(pEntry);
    }

    return 0;
}

/*---------------------------------------------------------------------
 Description: clear selected region
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, ChangeDismissHdl)
{
    if(!CheckPasswd())
        return 0;
    SvTreeListEntry* pEntry = aTree.FirstSelected();
    SvTreeListEntry* pChild;
    SvTreeListEntry* pParent;
    // at first mark all selected
    while(pEntry)
    {
        const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        pSectRepr->SetSelected();
        pEntry = aTree.NextSelected(pEntry);
    }
    pEntry = aTree.FirstSelected();
    // then delete
    while(pEntry)
    {
        const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SvTreeListEntry* pRemove = 0;
        sal_Bool bRestart = sal_False;
        if(pSectRepr->IsSelected())
        {
            aSectReprArr.insert( pSectRepr );
            while( (pChild = aTree.FirstChild(pEntry) )!= 0 )
            {
                // because of the repositioning we have to start at the beginning again
                bRestart = sal_True;
                pParent=aTree.GetParent(pEntry);
                aTree.GetModel()->Move(pChild, pParent, aTree.GetModel()->GetRelPos(pEntry));
            }
            pRemove = pEntry;
        }
        if(bRestart)
            pEntry = aTree.First();
        else
            pEntry = aTree.Next(pEntry);
        if(pRemove)
            aTree.GetModel()->Remove( pRemove );
    }

    if ( aTree.FirstSelected() == 0 )
    {
        aConditionFT.   Enable(sal_False);
        aConditionED.   Enable(sal_False);
        aDismiss.       Enable(sal_False);
        aCurName.       Enable(sal_False);
        aProtectCB.     Enable(sal_False);
        aPasswdCB.      Enable(sal_False);
        aHideCB.        Enable(sal_False);
        // edit in readonly sections
        aEditInReadonlyCB.Enable(sal_False);
        aEditInReadonlyCB.SetState(STATE_NOCHECK);
        aProtectCB.     SetState(STATE_NOCHECK);
        aPasswdCB.      Check(sal_False);
        aHideCB.        SetState(STATE_NOCHECK);
        aFileCB.        Check(sal_False);
        // otherwise the focus would be on HelpButton
        aOK.GrabFocus();
        UseFileHdl(&aFileCB);
    }
    return 0;
}

/*---------------------------------------------------------------------
 Description: link CheckBox to file?
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, UseFileHdl, CheckBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    SvTreeListEntry* pEntry = aTree.FirstSelected();
    pBox->EnableTriState(sal_False);
    sal_Bool bMulti = 1 < aTree.GetSelectionCount();
    sal_Bool bFile = pBox->IsChecked();
    if(pEntry)
    {
        while(pEntry)
        {
            const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
            sal_Bool bContent = pSectRepr->IsContent();
            if( pBox->IsChecked() && bContent && rSh.HasSelection() )
            {
                if( RET_NO == QueryBox( this, SW_RES(QB_CONNECT) ).Execute() )
                    pBox->Check( sal_False );
            }
            if( bFile )
                pSectRepr->SetContent(sal_False);
            else
            {
                pSectRepr->SetFile(aEmptyStr);
                pSectRepr->SetSubRegion(aEmptyStr);
                pSectRepr->GetSectionData().SetLinkFilePassword(aEmptyStr);
            }

            pEntry = aTree.NextSelected(pEntry);
        }
        aFileNameFT.Enable(bFile && ! bMulti);
        aFileNameED.Enable(bFile && ! bMulti);
        aFilePB.Enable(bFile && ! bMulti);
        aSubRegionED.Enable(bFile && ! bMulti);
        aSubRegionFT.Enable(bFile && ! bMulti);
        aDDECommandFT.Enable(bFile && ! bMulti);
        aDDECB.Enable(bFile && ! bMulti);
        if( bFile )
        {
            aProtectCB.SetState(STATE_CHECK);
            aFileNameED.GrabFocus();

        }
        else
        {
            aDDECB.Check(sal_False);
            DDEHdl(&aDDECB);
            aSubRegionED.SetText(aEmptyStr);
        }
    }
    else
    {
        pBox->Check(sal_False);
        pBox->Enable(sal_False);
        aFilePB.Enable(sal_False);
        aFileNameED.Enable(sal_False);
        aFileNameFT.Enable(sal_False);
        aSubRegionED.Enable(sal_False);
        aSubRegionFT.Enable(sal_False);
        aDDECB.Check(sal_False);
        aDDECB.Enable(sal_False);
        aDDECommandFT.Enable(sal_False);
    }
    return 0;
}

/*---------------------------------------------------------------------
    Description: call dialog paste file
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, FileSearchHdl)
{
    if(!CheckPasswd(0))
        return 0;

    m_pOldDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    delete m_pDocInserter;
    m_pDocInserter =
        new ::sfx2::DocumentInserter( rtl::OUString("swriter") );
    m_pDocInserter->StartExecuteModal( LINK( this, SwEditRegionDlg, DlgClosedHdl ) );
    return 0;
}

IMPL_LINK_NOARG(SwEditRegionDlg, OptionsHdl)
{
    if(!CheckPasswd())
        return 0;
    SvTreeListEntry* pEntry = aTree.FirstSelected();

    if(pEntry)
    {
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SfxItemSet aSet(rSh.GetView().GetPool(),
                            RES_COL, RES_COL,
                            RES_COLUMNBALANCE, RES_FRAMEDIR,
                            RES_BACKGROUND, RES_BACKGROUND,
                            RES_FRM_SIZE, RES_FRM_SIZE,
                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                            RES_LR_SPACE, RES_LR_SPACE,
                            RES_FTN_AT_TXTEND, RES_END_AT_TXTEND,
                            0);

        aSet.Put( pSectRepr->GetCol() );
        aSet.Put( pSectRepr->GetBackground() );
        aSet.Put( pSectRepr->GetFtnNtAtEnd() );
        aSet.Put( pSectRepr->GetEndNtAtEnd() );
        aSet.Put( pSectRepr->GetBalance() );
        aSet.Put( pSectRepr->GetFrmDir() );
        aSet.Put( pSectRepr->GetLRSpace() );

        const SwSectionFmts& rDocFmts = rSh.GetDoc()->GetSections();
        SwSectionFmts aOrigArray(rDocFmts);

        SwSectionFmt* pFmt = aOrigArray[pSectRepr->GetArrPos()];
        long nWidth = rSh.GetSectionWidth(*pFmt);
        aOrigArray.clear();
        if (!nWidth)
            nWidth = USHRT_MAX;

        aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));

        SwSectionPropertyTabDialog aTabDlg(this, aSet, rSh);
        if(RET_OK == aTabDlg.Execute())
        {
            const SfxItemSet* pOutSet = aTabDlg.GetOutputItemSet();
            if( pOutSet && pOutSet->Count() )
            {
                const SfxPoolItem *pColItem, *pBrushItem,
                                  *pFtnItem, *pEndItem, *pBalanceItem,
                                  *pFrmDirItem, *pLRSpaceItem;
                SfxItemState eColState = pOutSet->GetItemState(
                                        RES_COL, sal_False, &pColItem );
                SfxItemState eBrushState = pOutSet->GetItemState(
                                        RES_BACKGROUND, sal_False, &pBrushItem );
                SfxItemState eFtnState = pOutSet->GetItemState(
                                        RES_FTN_AT_TXTEND, sal_False, &pFtnItem );
                SfxItemState eEndState = pOutSet->GetItemState(
                                        RES_END_AT_TXTEND, sal_False, &pEndItem );
                SfxItemState eBalanceState = pOutSet->GetItemState(
                                        RES_COLUMNBALANCE, sal_False, &pBalanceItem );
                SfxItemState eFrmDirState = pOutSet->GetItemState(
                                        RES_FRAMEDIR, sal_False, &pFrmDirItem );
                SfxItemState eLRState = pOutSet->GetItemState(
                                        RES_LR_SPACE, sal_False, &pLRSpaceItem);

                if( SFX_ITEM_SET == eColState ||
                    SFX_ITEM_SET == eBrushState ||
                    SFX_ITEM_SET == eFtnState ||
                    SFX_ITEM_SET == eEndState ||
                    SFX_ITEM_SET == eBalanceState||
                    SFX_ITEM_SET == eFrmDirState||
                    SFX_ITEM_SET == eLRState)
                {
                    SvTreeListEntry* pSelEntry = aTree.FirstSelected();
                    while( pSelEntry )
                    {
                        SectReprPtr pRepr = (SectReprPtr)pSelEntry->GetUserData();
                        if( SFX_ITEM_SET == eColState )
                            pRepr->GetCol() = *(SwFmtCol*)pColItem;
                        if( SFX_ITEM_SET == eBrushState )
                            pRepr->GetBackground() = *(SvxBrushItem*)pBrushItem;
                        if( SFX_ITEM_SET == eFtnState )
                            pRepr->GetFtnNtAtEnd() = *(SwFmtFtnAtTxtEnd*)pFtnItem;
                        if( SFX_ITEM_SET == eEndState )
                            pRepr->GetEndNtAtEnd() = *(SwFmtEndAtTxtEnd*)pEndItem;
                        if( SFX_ITEM_SET == eBalanceState )
                            pRepr->GetBalance().SetValue(((SwFmtNoBalancedColumns*)pBalanceItem)->GetValue());
                        if( SFX_ITEM_SET == eFrmDirState )
                            pRepr->GetFrmDir().SetValue(((SvxFrameDirectionItem*)pFrmDirItem)->GetValue());
                        if( SFX_ITEM_SET == eLRState )
                            pRepr->GetLRSpace() = *(SvxLRSpaceItem*)pLRSpaceItem;

                        pSelEntry = aTree.NextSelected(pSelEntry);
                    }
                }
            }
        }
    }

    return 0;
}

/*---------------------------------------------------------------------
    Description:    Applying of the filename or the
                    linked region
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, FileNameHdl, Edit *, pEdit )
{
    Selection aSelect = pEdit->GetSelection();
    if(!CheckPasswd())
        return 0;
    pEdit->SetSelection(aSelect);
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
    if(pEdit == &aFileNameED)
    {
        bSubRegionsFilled = false;
        aSubRegionED.Clear();
        if( aDDECB.IsChecked() )
        {
            String sLink( pEdit->GetText() );
            sal_uInt16 nPos = 0;
            while( STRING_NOTFOUND != (nPos = sLink.SearchAscii( "  ", nPos )) )
                sLink.Erase( nPos--, 1 );

            nPos = sLink.SearchAndReplace( ' ', sfx2::cTokenSeperator );
            sLink.SearchAndReplace( ' ', sfx2::cTokenSeperator, nPos );

            pSectRepr->GetSectionData().SetLinkFileName( sLink );
            pSectRepr->GetSectionData().SetType( DDE_LINK_SECTION );
        }
        else
        {
            String sTmp(pEdit->GetText());
            if(sTmp.Len())
            {
                SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                sTmp = URIHelper::SmartRel2Abs(
                    aAbs, sTmp, URIHelper::GetMaybeFileHdl() );
            }
            pSectRepr->SetFile( sTmp );
            pSectRepr->GetSectionData().SetLinkFilePassword( aEmptyStr );
        }
    }
    else
    {
        pSectRepr->SetSubRegion( pEdit->GetText() );
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, DDEHdl, CheckBox*, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    if(pEntry)
    {
        sal_Bool bFile = aFileCB.IsChecked();
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SwSectionData & rData( pSectRepr->GetSectionData() );
        sal_Bool bDDE = pBox->IsChecked();
        if(bDDE)
        {
            aFileNameFT.Hide();
            aDDECommandFT.Enable();
            aDDECommandFT.Show();
            aSubRegionFT.Hide();
            aSubRegionED.Hide();
            if (FILE_LINK_SECTION == rData.GetType())
            {
                pSectRepr->SetFile(aEmptyStr);
                aFileNameED.SetText(aEmptyStr);
                rData.SetLinkFilePassword( aEmptyStr );
            }
            rData.SetType(DDE_LINK_SECTION);
            aFileNameED.SetAccessibleName(aDDECommandFT.GetText());
        }
        else
        {
            aDDECommandFT.Hide();
            aFileNameFT.Enable(bFile);
            aFileNameFT.Show();
            aSubRegionED.Show();
            aSubRegionFT.Show();
            aSubRegionED.Enable(bFile);
            aSubRegionFT.Enable(bFile);
            aSubRegionED.Enable(bFile);
            if (DDE_LINK_SECTION == rData.GetType())
            {
                rData.SetType(FILE_LINK_SECTION);
                pSectRepr->SetFile(aEmptyStr);
                rData.SetLinkFilePassword( aEmptyStr );
                aFileNameED.SetText(aEmptyStr);
            }
            aFileNameED.SetAccessibleName(aFileNameFT.GetText());
        }
        aFilePB.Enable(bFile && !bDDE);
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, ChangePasswdHdl, Button *, pBox )
{
    sal_Bool bChange = pBox == &aPasswdPB;
    if(!CheckPasswd(0))
    {
        if(!bChange)
            aPasswdCB.Check(!aPasswdCB.IsChecked());
        return 0;
    }
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    sal_Bool bSet = bChange ? bChange : aPasswdCB.IsChecked();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if(bSet)
        {
            if(!pRepr->GetTempPasswd().getLength() || bChange)
            {
                SfxPasswordDialog aPasswdDlg(this);
                aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
                if(RET_OK == aPasswdDlg.Execute())
                {
                    String sNewPasswd( aPasswdDlg.GetPassword() );
                    if( aPasswdDlg.GetConfirm() == sNewPasswd )
                    {
                        SvPasswordHelper::GetHashPassword( pRepr->GetTempPasswd(), sNewPasswd );
                    }
                    else
                    {
                        InfoBox(pBox, SW_RES(REG_WRONG_PASSWD_REPEAT)).Execute();
                        ChangePasswdHdl(pBox);
                        break;
                    }
                }
                else
                {
                    if(!bChange)
                        aPasswdCB.Check(sal_False);
                    break;
                }
            }
            pRepr->GetSectionData().SetPassword(pRepr->GetTempPasswd());
        }
        else
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        pEntry = aTree.NextSelected(pEntry);
    }
    return 0;
}

/*---------------------------------------------------------------------
    Description:    the current region name is being added to the
                    TreeListBox immediately during editing, with empty
                    string no Ok()
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, NameEditHdl)
{
    if(!CheckPasswd(0))
        return 0;
    SvTreeListEntry* pEntry=aTree.FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    if (pEntry)
    {
        String  aName = aCurName.GetText();
        aTree.SetEntryText(pEntry,aName);
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetSectionName(aName);

        aOK.Enable(aName.Len() != 0);
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, ConditionEditHdl, Edit *, pEdit )
{
    Selection aSelect = pEdit->GetSelection();
    if(!CheckPasswd(0))
        return 0;
    pEdit->SetSelection(aSelect);
    SvTreeListEntry* pEntry = aTree.FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        pRepr->GetSectionData().SetCondition(pEdit->GetText());
        pEntry = aTree.NextSelected(pEntry);
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg )
{
    String sFileName, sFilterName, sPassword;
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        SfxMedium* pMedium = m_pDocInserter->CreateMedium();
        if ( pMedium )
        {
            sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                sPassword = ( (SfxStringItem*)pItem )->GetValue();
            ::lcl_ReadSections( *pMedium, aSubRegionED );
            delete pMedium;
        }
    }

    SvTreeListEntry* pEntry = aTree.FirstSelected();
    OSL_ENSURE( pEntry, "no entry found" );
    if ( pEntry )
    {
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        pSectRepr->SetFile( sFileName );
        pSectRepr->SetFilter( sFilterName );
        pSectRepr->GetSectionData().SetLinkFilePassword(sPassword);
        aFileNameED.SetText( pSectRepr->GetFile() );
    }

    Application::SetDefDialogParent( m_pOldDefDlgParent );
    return 0;
}

IMPL_LINK( SwEditRegionDlg, SubRegionEventHdl, VclWindowEvent *, pEvent )
{
    if( !bSubRegionsFilled && pEvent && pEvent->GetId() == VCLEVENT_DROPDOWN_PRE_OPEN )
    {
        //if necessary fill the names bookmarks/sections/tables now

        rtl::OUString sFileName = aFileNameED.GetText();
        if(!sFileName.isEmpty())
        {
            SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();
            sFileName = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );

            //load file and set the shell
            SfxMedium aMedium( sFileName, STREAM_STD_READ );
            sFileName = aMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            ::lcl_ReadSections( aMedium, aSubRegionED );
        }
        else
            lcl_FillSubRegionList( rSh, aSubRegionED, 0 );
        bSubRegionsFilled = true;
    }
    return 0;
}

Image SwEditRegionDlg::BuildBitmap( sal_Bool bProtect, sal_Bool bHidden )
{
    ImageList& rImgLst = aImageIL;
    return rImgLst.GetImage((!bHidden+(bProtect<<1)) + 1);
}

/*--------------------------------------------------------------------
    Description:    helper function - read region names from medium
 --------------------------------------------------------------------*/
static void lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox )
{
    rBox.Clear();
    uno::Reference < embed::XStorage > xStg;
    if( rMedium.IsStorage() && (xStg = rMedium.GetStorage()).is() )
    {
        std::vector<String*> aArr;
        sal_uInt32 nFormat = SotStorage::GetFormatID( xStg );
        if ( nFormat == SOT_FORMATSTR_ID_STARWRITER_60 || nFormat == SOT_FORMATSTR_ID_STARWRITERGLOB_60 ||
            nFormat == SOT_FORMATSTR_ID_STARWRITER_8 || nFormat == SOT_FORMATSTR_ID_STARWRITERGLOB_8)
            SwGetReaderXML()->GetSectionList( rMedium, aArr );

        for(std::vector<String*>::const_iterator it(aArr.begin()); it != aArr.end(); ++it) {
            rBox.InsertEntry( **it );
            delete *it;
        }
    }
}

SwInsertSectionTabDialog::SwInsertSectionTabDialog(
            Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) :
    SfxTabDialog( pParent, SW_RES(DLG_INSERT_SECTION), &rSet ),
    rWrtSh(rSh)
    , m_pSectionData(0)
{
    String sInsert(SW_RES(ST_INSERT));
    GetOKButton().SetText(sInsert);
    FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AddTabPage(TP_INSERT_SECTION, SwInsertSectionTabPage::Create, 0);
    AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0);
    AddTabPage(TP_SECTION_FTNENDNOTES, SwSectionFtnEndTabPage::Create, 0);
    AddTabPage(TP_SECTION_INDENTS, SwSectionIndentTabPage::Create, 0);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();

    sal_Bool bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );
    if(bWeb)
    {
        RemoveTabPage(TP_SECTION_FTNENDNOTES);
        RemoveTabPage(TP_SECTION_INDENTS);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage(TP_COLUMN);
    }
    SetCurPageId(TP_INSERT_SECTION);
}

SwInsertSectionTabDialog::~SwInsertSectionTabDialog()
{
}

void SwInsertSectionTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if(TP_INSERT_SECTION == nId)
        ((SwInsertSectionTabPage&)rPage).SetWrtShell(rWrtSh);
    else if( TP_BACKGROUND == nId  )
    {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
            rPage.PageCreated(aSet);
    }
    else if( TP_COLUMN == nId )
    {
        const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)GetInputSetImpl()->Get(RES_FRM_SIZE);
        ((SwColumnPage&)rPage).SetPageWidth(rSize.GetWidth());
        ((SwColumnPage&)rPage).ShowBalance(sal_True);
        ((SwColumnPage&)rPage).SetInSection(sal_True);
    }
    else if(TP_SECTION_INDENTS == nId)
        ((SwSectionIndentTabPage&)rPage).SetWrtShell(rWrtSh);
}

void SwInsertSectionTabDialog::SetSectionData(SwSectionData const& rSect)
{
    m_pSectionData.reset( new SwSectionData(rSect) );
}

short   SwInsertSectionTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    OSL_ENSURE(m_pSectionData.get(),
            "SwInsertSectionTabDialog: no SectionData?");
    const SfxItemSet* pOutputItemSet = GetOutputItemSet();
    rWrtSh.InsertSection(*m_pSectionData, pOutputItemSet);
    SfxViewFrame* pViewFrm = rWrtSh.GetView().GetViewFrame();
    uno::Reference< frame::XDispatchRecorder > xRecorder =
            pViewFrm->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        SfxRequest aRequest( pViewFrm, FN_INSERT_REGION);
        const SfxPoolItem* pCol;
        if(SFX_ITEM_SET == pOutputItemSet->GetItemState(RES_COL, sal_False, &pCol))
        {
            aRequest.AppendItem(SfxUInt16Item(SID_ATTR_COLUMNS,
                ((const SwFmtCol*)pCol)->GetColumns().size()));
        }
        aRequest.AppendItem(SfxStringItem( FN_PARAM_REGION_NAME,
                    m_pSectionData->GetSectionName()));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_REGION_CONDITION,
                    m_pSectionData->GetCondition()));
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_HIDDEN,
                    m_pSectionData->IsHidden()));
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_PROTECT,
                    m_pSectionData->IsProtectFlag()));
        // edit in readonly sections
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_EDIT_IN_READONLY,
                    m_pSectionData->IsEditInReadonlyFlag()));

        String sLinkFileName( m_pSectionData->GetLinkFileName() );
        aRequest.AppendItem(SfxStringItem( FN_PARAM_1, sLinkFileName.GetToken( 0, sfx2::cTokenSeperator )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_2, sLinkFileName.GetToken( 1, sfx2::cTokenSeperator )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_3, sLinkFileName.GetToken( 2, sfx2::cTokenSeperator )));
        aRequest.Done();
    }
    return nRet;
}

SwInsertSectionTabPage::SwInsertSectionTabPage(
                            Window *pParent, const SfxItemSet &rAttrSet) :
    SfxTabPage( pParent, SW_RES(TP_INSERT_SECTION), rAttrSet ),
    aNameFL       ( this, SW_RES( FL_NAME ) ),
    aCurName            ( this, SW_RES( ED_RNAME ) ),
    aLinkFL             ( this, SW_RES( FL_LINK ) ),
    aFileCB             ( this, SW_RES( CB_FILE ) ),
    aDDECB              ( this, SW_RES( CB_DDE ) ) ,
    aDDECommandFT       ( this, SW_RES( FT_DDE ) ) ,
    aFileNameFT         ( this, SW_RES( FT_FILE ) ) ,
    aFileNameED         ( this, SW_RES( ED_FILE ) ),
    aFilePB             ( this, SW_RES( PB_FILE ) ),
    aSubRegionFT        ( this, SW_RES( FT_SUBREG ) ) ,
    aSubRegionED        ( this, SW_RES( LB_SUBREG ) ) ,

    aProtectFL          ( this, SW_RES( FL_PROTECT ) ),
    aProtectCB          ( this, SW_RES( CB_PROTECT ) ),
    aPasswdCB           ( this, SW_RES( CB_PASSWD ) ),
    aPasswdPB           ( this, SW_RES( PB_PASSWD ) ),

    aHideFL             ( this, SW_RES( FL_HIDE ) ),
    aHideCB             ( this, SW_RES( CB_HIDE ) ),
    aConditionFT             ( this, SW_RES( FT_CONDITION ) ),
    aConditionED        ( this, SW_RES( ED_CONDITION ) ),
    // edit in readonly sections
    aPropertiesFL       ( this, SW_RES( FL_PROPERTIES ) ),
    aEditInReadonlyCB   ( this, SW_RES( CB_EDIT_IN_READONLY ) ),

    m_pWrtSh(0),
    m_pDocInserter(NULL),
    m_pOldDefDlgParent(NULL)
{
    FreeResource();

    aProtectCB.SetClickHdl  ( LINK( this, SwInsertSectionTabPage, ChangeProtectHdl));
    aPasswdCB.SetClickHdl   ( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    aPasswdPB.SetClickHdl   ( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    aHideCB.SetClickHdl     ( LINK( this, SwInsertSectionTabPage, ChangeHideHdl));
    // edit in readonly sections
    aEditInReadonlyCB.SetClickHdl       ( LINK( this, SwInsertSectionTabPage, ChangeEditInReadonlyHdl));
    aFileCB.SetClickHdl     ( LINK( this, SwInsertSectionTabPage, UseFileHdl ));
    aFilePB.SetClickHdl     ( LINK( this, SwInsertSectionTabPage, FileSearchHdl ));
    aCurName.SetModifyHdl   ( LINK( this, SwInsertSectionTabPage, NameEditHdl));
    aDDECB.SetClickHdl      ( LINK( this, SwInsertSectionTabPage, DDEHdl ));
    ChangeProtectHdl(&aProtectCB);
    aPasswdPB.SetAccessibleRelationMemberOf(&aProtectFL);
    aSubRegionED.EnableAutocomplete( sal_True, sal_True );
}

SwInsertSectionTabPage::~SwInsertSectionTabPage()
{
    delete m_pDocInserter;
}

void    SwInsertSectionTabPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pWrtSh = &rSh;

    sal_Bool bWeb = 0 != PTR_CAST(SwWebDocShell, m_pWrtSh->GetView().GetDocShell());
    if(bWeb)
    {
        aHideCB         .Hide();
        aConditionED    .Hide();
        aConditionFT    .Hide();
        aDDECB           .Hide();
        aDDECommandFT    .Hide();
    }

    lcl_FillSubRegionList( *m_pWrtSh, aSubRegionED, &aCurName );

    SwSectionData *const pSectionData =
        static_cast<SwInsertSectionTabDialog*>(GetTabDialog())
            ->GetSectionData();
    if (pSectionData) // something set?
    {
        aCurName.SetText(
            rSh.GetUniqueSectionName(& pSectionData->GetSectionName()));
        aProtectCB.Check( 0 != pSectionData->IsProtectFlag() );
        m_sFileName = pSectionData->GetLinkFileName();
        m_sFilePasswd = pSectionData->GetLinkFilePassword();
        aFileCB.Check( 0 != m_sFileName.Len() );
        aFileNameED.SetText( m_sFileName );
        UseFileHdl( &aFileCB );
    }
    else
    {
        aCurName.SetText( rSh.GetUniqueSectionName() );
    }
}

sal_Bool SwInsertSectionTabPage::FillItemSet( SfxItemSet& )
{
    SwSectionData aSection(CONTENT_SECTION, aCurName.GetText());
    aSection.SetCondition(aConditionED.GetText());
    sal_Bool bProtected = aProtectCB.IsChecked();
    aSection.SetProtectFlag(bProtected);
    aSection.SetHidden(aHideCB.IsChecked());
    // edit in readonly sections
    aSection.SetEditInReadonlyFlag(aEditInReadonlyCB.IsChecked());

    if(bProtected)
    {
        aSection.SetPassword(m_aNewPasswd);
    }
    String sFileName = aFileNameED.GetText();
    String sSubRegion = aSubRegionED.GetText();
    sal_Bool bDDe = aDDECB.IsChecked();
    if(aFileCB.IsChecked() && (sFileName.Len() || sSubRegion.Len() || bDDe))
    {
        String aLinkFile;
        if( bDDe )
        {
            aLinkFile = sFileName;

            sal_uInt16 nPos = 0;
            while( STRING_NOTFOUND != (nPos = aLinkFile.SearchAscii( "  ", nPos )) )
                aLinkFile.Erase( nPos--, 1 );

            nPos = aLinkFile.SearchAndReplace( ' ', sfx2::cTokenSeperator );
            aLinkFile.SearchAndReplace( ' ', sfx2::cTokenSeperator, nPos );
        }
        else
        {
            if(sFileName.Len())
            {
                SfxMedium* pMedium = m_pWrtSh->GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                aLinkFile = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );
                aSection.SetLinkFilePassword( m_sFilePasswd );
            }

            aLinkFile += sfx2::cTokenSeperator;
            aLinkFile += m_sFilterName;
            aLinkFile += sfx2::cTokenSeperator;
            aLinkFile += sSubRegion;
        }

        aSection.SetLinkFileName(aLinkFile);
        if(aLinkFile.Len())
        {
            aSection.SetType( aDDECB.IsChecked() ?
                                    DDE_LINK_SECTION :
                                        FILE_LINK_SECTION);
        }
    }
    ((SwInsertSectionTabDialog*)GetTabDialog())->SetSectionData(aSection);
    return sal_True;
}

void SwInsertSectionTabPage::Reset( const SfxItemSet& )
{
}

SfxTabPage* SwInsertSectionTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwInsertSectionTabPage(pParent, rAttrSet);
}

IMPL_LINK( SwInsertSectionTabPage, ChangeHideHdl, CheckBox *, pBox )
{
    sal_Bool bHide = pBox->IsChecked();
    aConditionED.Enable(bHide);
    aConditionFT.Enable(bHide);
    return 0;
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, ChangeEditInReadonlyHdl)
{
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, ChangeProtectHdl, CheckBox *, pBox )
{
    sal_Bool bCheck = pBox->IsChecked();
    aPasswdCB.Enable(bCheck);
    aPasswdPB.Enable(bCheck);
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, ChangePasswdHdl, Button *, pButton )
{
    sal_Bool bChange = pButton == &aPasswdPB;
    sal_Bool bSet = bChange ? bChange : aPasswdCB.IsChecked();
    if(bSet)
    {
        if(!m_aNewPasswd.getLength() || bChange)
        {
            SfxPasswordDialog aPasswdDlg(this);
            aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
            if(RET_OK == aPasswdDlg.Execute())
            {
                String sNewPasswd( aPasswdDlg.GetPassword() );
                if( aPasswdDlg.GetConfirm() == sNewPasswd )
                {
                    SvPasswordHelper::GetHashPassword( m_aNewPasswd, sNewPasswd );
                }
                else
                {
                    InfoBox(pButton, SW_RES(REG_WRONG_PASSWD_REPEAT)).Execute();
                }
            }
            else if(!bChange)
                aPasswdCB.Check(sal_False);
        }
    }
    else
        m_aNewPasswd.realloc(0);
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwInsertSectionTabPage, NameEditHdl)
{
    String  aName=aCurName.GetText();
    GetTabDialog()->GetOKButton().Enable(aName.Len() && aCurName.GetEntryPos( aName ) == USHRT_MAX);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsertSectionTabPage, NameEditHdl)

IMPL_LINK( SwInsertSectionTabPage, UseFileHdl, CheckBox *, pBox )
{
    if( pBox->IsChecked() )
    {
        if( m_pWrtSh->HasSelection() &&
            RET_NO == QueryBox( this, SW_RES(QB_CONNECT) ).Execute() )
            pBox->Check( sal_False );
    }

    sal_Bool bFile = pBox->IsChecked();
    aFileNameFT.Enable(bFile);
    aFileNameED.Enable(bFile);
    aFilePB.Enable(bFile);
    aSubRegionFT.Enable(bFile);
    aSubRegionED.Enable(bFile);
    aDDECommandFT.Enable(bFile);
    aDDECB.Enable(bFile);
    if( bFile )
    {
        aFileNameED.GrabFocus();
        aProtectCB.Check( sal_True );
    }
    else
    {
        aDDECB.Check(sal_False);
        DDEHdl(&aDDECB);
    }
    return 0;
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, FileSearchHdl)
{
    m_pOldDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter(
            rtl::OUString("swriter") );
    m_pDocInserter->StartExecuteModal( LINK( this, SwInsertSectionTabPage, DlgClosedHdl ) );
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, DDEHdl, CheckBox*, pBox )
{
    sal_Bool bDDE = pBox->IsChecked();
    sal_Bool bFile = aFileCB.IsChecked();
    aFilePB.Enable(!bDDE && bFile);
    if(bDDE)
    {
        aFileNameFT.Hide();
        aDDECommandFT.Enable(bDDE);
        aDDECommandFT.Show();
        aSubRegionFT.Hide();
        aSubRegionED.Hide();
        aFileNameED.SetAccessibleName(aDDECommandFT.GetText());
    }
    else
    {
        aDDECommandFT.Hide();
        aFileNameFT.Enable(bFile);
        aFileNameFT.Show();
        aSubRegionFT.Show();
        aSubRegionED.Show();
        aSubRegionED.Enable(bFile);
        aFileNameED.SetAccessibleName(aFileNameFT.GetText());
    }
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg )
{
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        SfxMedium* pMedium = m_pDocInserter->CreateMedium();
        if ( pMedium )
        {
            m_sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            m_sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                m_sFilePasswd = ( (SfxStringItem*)pItem )->GetValue();
            aFileNameED.SetText( INetURLObject::decode(
                m_sFileName, INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8 ) );
            ::lcl_ReadSections( *pMedium, aSubRegionED );
            delete pMedium;
        }
    }
    else
        m_sFilterName = m_sFilePasswd = aEmptyStr;

    Application::SetDefDialogParent( m_pOldDefDlgParent );
    return 0;
}

// --------------------------------------------------------------

// numbering format conversion:
// ListBox  - format            - enum-value
// 0        - A, B, C, ...      - 0
// 1        - a, b, c, ...      - 1
// 2        - I, II, III, ...   - 2
// 3        - i, ii, iii, ...   - 3
// 4        - 1, 2, 3, ...      - 4
// 5        - A, .., AA, ..,    - 9
// 6        - a, .., aa, ..,    - 10

inline sal_uInt16 GetNumPos( sal_uInt16 n )
{
    return SVX_NUM_ARABIC < n ? n - 4 : n;
}

inline SvxExtNumType GetNumType( sal_uInt16 n )
{
    return (SvxExtNumType)(4 < n ? n + 4 : n );
}

SwSectionFtnEndTabPage::SwSectionFtnEndTabPage( Window *pParent,
                                                const SfxItemSet &rAttrSet)
    : SfxTabPage( pParent, SW_RES( TP_SECTION_FTNENDNOTES ), rAttrSet ),
    aFtnFL              ( this, SW_RES( FL_FTN ) ),
    aFtnNtAtTextEndCB   ( this, SW_RES( CB_FTN_AT_TXTEND ) ),

    aFtnNtNumCB         ( this, SW_RES( CB_FTN_NUM ) ),
    aFtnOffsetLbl       ( this, SW_RES( FT_FTN_OFFSET   )),
    aFtnOffsetFld       ( this, SW_RES( FLD_FTN_OFFSET   )),

    aFtnNtNumFmtCB      ( this, SW_RES( CB_FTN_NUM_FMT ) ),
    aFtnPrefixFT        ( this, SW_RES( FT_FTN_PREFIX   )),
    aFtnPrefixED        ( this, SW_RES( ED_FTN_PREFIX    )),
    aFtnNumViewBox      ( this, SW_RES( LB_FTN_NUMVIEW  ), INSERT_NUM_EXTENDED_TYPES),
    aFtnSuffixFT        ( this, SW_RES( FT_FTN_SUFFIX    )),
    aFtnSuffixED        ( this, SW_RES( ED_FTN_SUFFIX    )),

    aEndFL              ( this, SW_RES( FL_END ) ),
    aEndNtAtTextEndCB   ( this, SW_RES( CB_END_AT_TXTEND )),

    aEndNtNumCB         ( this, SW_RES( CB_END_NUM )),
    aEndOffsetLbl       ( this, SW_RES( FT_END_OFFSET   )),
    aEndOffsetFld       ( this, SW_RES( FLD_END_OFFSET   )),

    aEndNtNumFmtCB      ( this, SW_RES( CB_END_NUM_FMT ) ),
    aEndPrefixFT        ( this, SW_RES( FT_END_PREFIX   )),
    aEndPrefixED        ( this, SW_RES( ED_END_PREFIX    )),
    aEndNumViewBox      ( this, SW_RES( LB_END_NUMVIEW  ), INSERT_NUM_EXTENDED_TYPES),
    aEndSuffixFT        ( this, SW_RES( FT_END_SUFFIX    )),
    aEndSuffixED        ( this, SW_RES( ED_END_SUFFIX    ))
{
    FreeResource();

    Link aLk( LINK( this, SwSectionFtnEndTabPage, FootEndHdl));
    aFtnNtAtTextEndCB.SetClickHdl( aLk );
    aFtnNtNumCB.SetClickHdl( aLk );
    aEndNtAtTextEndCB.SetClickHdl( aLk );
    aEndNtNumCB.SetClickHdl( aLk );
    aFtnNtNumFmtCB.SetClickHdl( aLk );
    aEndNtNumFmtCB.SetClickHdl( aLk );
}

SwSectionFtnEndTabPage::~SwSectionFtnEndTabPage()
{
}

sal_Bool SwSectionFtnEndTabPage::FillItemSet( SfxItemSet& rSet )
{
    SwFmtFtnAtTxtEnd aFtn( aFtnNtAtTextEndCB.IsChecked()
                            ? ( aFtnNtNumCB.IsChecked()
                                ? ( aFtnNtNumFmtCB.IsChecked()
                                    ? FTNEND_ATTXTEND_OWNNUMANDFMT
                                    : FTNEND_ATTXTEND_OWNNUMSEQ )
                                : FTNEND_ATTXTEND )
                            : FTNEND_ATPGORDOCEND );

    switch( aFtn.GetValue() )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aFtn.SetNumType( aFtnNumViewBox.GetSelectedNumberingType() );
        aFtn.SetPrefix( aFtnPrefixED.GetText() );
        aFtn.SetSuffix( aFtnSuffixED.GetText() );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        aFtn.SetOffset( static_cast< sal_uInt16 >( aFtnOffsetFld.GetValue()-1 ) );
        // no break;
    }

    SwFmtEndAtTxtEnd aEnd( aEndNtAtTextEndCB.IsChecked()
                            ? ( aEndNtNumCB.IsChecked()
                                ? ( aEndNtNumFmtCB.IsChecked()
                                    ? FTNEND_ATTXTEND_OWNNUMANDFMT
                                    : FTNEND_ATTXTEND_OWNNUMSEQ )
                                : FTNEND_ATTXTEND )
                            : FTNEND_ATPGORDOCEND );

    switch( aEnd.GetValue() )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aEnd.SetNumType( aEndNumViewBox.GetSelectedNumberingType() );
        aEnd.SetPrefix( aEndPrefixED.GetText() );
        aEnd.SetSuffix( aEndSuffixED.GetText() );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        aEnd.SetOffset( static_cast< sal_uInt16 >( aEndOffsetFld.GetValue()-1 ) );
        // no break;
    }

    rSet.Put( aFtn );
    rSet.Put( aEnd );

    return sal_True;
}

void SwSectionFtnEndTabPage::ResetState( sal_Bool bFtn,
                                    const SwFmtFtnEndAtTxtEnd& rAttr )
{
    CheckBox *pNtAtTextEndCB, *pNtNumCB, *pNtNumFmtCB;
    FixedText*pPrefixFT, *pSuffixFT;
    Edit *pPrefixED, *pSuffixED;
    SwNumberingTypeListBox *pNumViewBox;
    FixedText* pOffsetTxt;
    NumericField *pOffsetFld;

    if( bFtn )
    {
        pNtAtTextEndCB = &aFtnNtAtTextEndCB;
        pNtNumCB = &aFtnNtNumCB;
        pNtNumFmtCB = &aFtnNtNumFmtCB;
        pPrefixFT = &aFtnPrefixFT;
        pPrefixED = &aFtnPrefixED;
        pSuffixFT = &aFtnSuffixFT;
        pSuffixED = &aFtnSuffixED;
        pNumViewBox = &aFtnNumViewBox;
        pOffsetTxt = &aFtnOffsetLbl;
        pOffsetFld = &aFtnOffsetFld;
    }
    else
    {
        pNtAtTextEndCB = &aEndNtAtTextEndCB;
        pNtNumCB = &aEndNtNumCB;
        pNtNumFmtCB = &aEndNtNumFmtCB;
        pPrefixFT = &aEndPrefixFT;
        pPrefixED = &aEndPrefixED;
        pSuffixFT = &aEndSuffixFT;
        pSuffixED = &aEndSuffixED;
        pNumViewBox = &aEndNumViewBox;
        pOffsetTxt = &aEndOffsetLbl;
        pOffsetFld = &aEndOffsetFld;
    }

    sal_uInt16 eState = rAttr.GetValue();
    switch( eState )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        pNtNumFmtCB->SetState( STATE_CHECK );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNtNumCB->SetState( STATE_CHECK );
        // no break;

    case FTNEND_ATTXTEND:
        pNtAtTextEndCB->SetState( STATE_CHECK );
        // no break;
    }

    pNumViewBox->SelectNumberingType( rAttr.GetNumType() );
    pOffsetFld->SetValue( rAttr.GetOffset() + 1 );
    pPrefixED->SetText( rAttr.GetPrefix() );
    pSuffixED->SetText( rAttr.GetSuffix() );

    switch( eState )
    {
    case FTNEND_ATPGORDOCEND:
        pNtNumCB->Enable( sal_False );
        // no break;

    case FTNEND_ATTXTEND:
        pNtNumFmtCB->Enable( sal_False );
        pOffsetFld->Enable( sal_False );
        pOffsetTxt->Enable( sal_False );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNumViewBox->Enable( sal_False );
        pPrefixFT->Enable( sal_False );
        pPrefixED->Enable( sal_False );
        pSuffixFT->Enable( sal_False );
        pSuffixED->Enable( sal_False );
        // no break;
    }
}

void SwSectionFtnEndTabPage::Reset( const SfxItemSet& rSet )
{
    ResetState( sal_True, (const SwFmtFtnAtTxtEnd&)rSet.Get(
                                    RES_FTN_AT_TXTEND, sal_False ));
    ResetState( sal_False, (const SwFmtEndAtTxtEnd&)rSet.Get(
                                    RES_END_AT_TXTEND, sal_False ));
}

SfxTabPage* SwSectionFtnEndTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwSectionFtnEndTabPage(pParent, rAttrSet);
}

IMPL_LINK( SwSectionFtnEndTabPage, FootEndHdl, CheckBox *, pBox )
{
    sal_Bool bFoot = &aFtnNtAtTextEndCB == pBox || &aFtnNtNumCB == pBox ||
                    &aFtnNtNumFmtCB == pBox ;

    CheckBox *pNumBox, *pNumFmtBox, *pEndBox;
    SwNumberingTypeListBox* pNumViewBox;
    FixedText* pOffsetTxt;
    NumericField *pOffsetFld;
    FixedText*pPrefixFT, *pSuffixFT;
    Edit *pPrefixED, *pSuffixED;

    if( bFoot )
    {
        pEndBox = &aFtnNtAtTextEndCB;
        pNumBox = &aFtnNtNumCB;
        pNumFmtBox = &aFtnNtNumFmtCB;
        pNumViewBox = &aFtnNumViewBox;
        pOffsetTxt = &aFtnOffsetLbl;
        pOffsetFld = &aFtnOffsetFld;
        pPrefixFT = &aFtnPrefixFT;
        pSuffixFT = &aFtnSuffixFT;
        pPrefixED = &aFtnPrefixED;
        pSuffixED = &aFtnSuffixED;
    }
    else
    {
        pEndBox = &aEndNtAtTextEndCB;
        pNumBox = &aEndNtNumCB;
        pNumFmtBox = &aEndNtNumFmtCB;
        pNumViewBox = &aEndNumViewBox;
        pOffsetTxt = &aEndOffsetLbl;
        pOffsetFld = &aEndOffsetFld;
        pPrefixFT = &aEndPrefixFT;
        pSuffixFT = &aEndSuffixFT;
        pPrefixED = &aEndPrefixED;
        pSuffixED = &aEndSuffixED;
    }

    sal_Bool bEnableAtEnd = STATE_CHECK == pEndBox->GetState();
    sal_Bool bEnableNum = bEnableAtEnd && STATE_CHECK == pNumBox->GetState();
    sal_Bool bEnableNumFmt = bEnableNum && STATE_CHECK == pNumFmtBox->GetState();

    pNumBox->Enable( bEnableAtEnd );
    pOffsetTxt->Enable( bEnableNum );
    pOffsetFld->Enable( bEnableNum );
    pNumFmtBox->Enable( bEnableNum );
    pNumViewBox->Enable( bEnableNumFmt );
    pPrefixED->Enable( bEnableNumFmt );
    pSuffixED->Enable( bEnableNumFmt );
    pPrefixFT->Enable( bEnableNumFmt );
    pSuffixFT->Enable( bEnableNumFmt );

    return 0;
}

SwSectionPropertyTabDialog::SwSectionPropertyTabDialog(
    Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) :
    SfxTabDialog(pParent, SW_RES(DLG_SECTION_PROPERTIES), &rSet),
    rWrtSh(rSh)
{
    FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage(TP_SECTION_FTNENDNOTES, SwSectionFtnEndTabPage::Create, 0);
    AddTabPage(TP_SECTION_INDENTS, SwSectionIndentTabPage::Create, 0);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();
    sal_Bool bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );
    if(bWeb)
    {
        RemoveTabPage(TP_SECTION_FTNENDNOTES);
        RemoveTabPage(TP_SECTION_INDENTS);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage(TP_COLUMN);
    }
}

SwSectionPropertyTabDialog::~SwSectionPropertyTabDialog()
{
}

void SwSectionPropertyTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if( TP_BACKGROUND == nId  )
    {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
            rPage.PageCreated(aSet);
    }
    else if( TP_COLUMN == nId )
    {
        ((SwColumnPage&)rPage).ShowBalance(sal_True);
        ((SwColumnPage&)rPage).SetInSection(sal_True);
    }
    else if(TP_SECTION_INDENTS == nId)
        ((SwSectionIndentTabPage&)rPage).SetWrtShell(rWrtSh);
}

SwSectionIndentTabPage::SwSectionIndentTabPage( Window *pParent, const SfxItemSet &rAttrSet ) :
    SfxTabPage(pParent, SW_RES(TP_SECTION_INDENTS), rAttrSet),
    aIndentFL(this,     SW_RES(FL_INDENT     )),
    aBeforeFT(this,     SW_RES(FT_BEFORE     )),
    aBeforeMF(this,     SW_RES(MF_BEFORE     )),
    aAfterFT(this,      SW_RES(FT_AFTER      )),
    aAfterMF(this,      SW_RES(MF_AFTER      )),
    aPreviewWin(this,   SW_RES(WIN_PREVIEW   ))
{
    FreeResource();
    Link aLk = LINK(this, SwSectionIndentTabPage, IndentModifyHdl);
    aBeforeMF.SetModifyHdl(aLk);
    aAfterMF.SetModifyHdl(aLk);
    aPreviewWin.SetAccessibleName(aIndentFL.GetText());
}

SwSectionIndentTabPage::~SwSectionIndentTabPage()
{
}

sal_Bool SwSectionIndentTabPage::FillItemSet( SfxItemSet& rSet)
{
    if(aBeforeMF.IsValueModified() ||
            aAfterMF.IsValueModified())
    {
        SvxLRSpaceItem aLRSpace(
                static_cast< long >(aBeforeMF.Denormalize(aBeforeMF.GetValue(FUNIT_TWIP))) ,
                static_cast< long >(aAfterMF.Denormalize(aAfterMF.GetValue(FUNIT_TWIP))), 0, 0, RES_LR_SPACE);
        rSet.Put(aLRSpace);
    }
    return sal_True;
}

void SwSectionIndentTabPage::Reset( const SfxItemSet& rSet)
{
    //this page doesn't show up in HTML mode
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(aBeforeMF, aMetric);
    SetMetric(aAfterMF , aMetric);

    SfxItemState eItemState = rSet.GetItemState( RES_LR_SPACE );
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxLRSpaceItem& rSpace =
            (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE );

        aBeforeMF.SetValue( aBeforeMF.Normalize(rSpace.GetLeft()), FUNIT_TWIP );
        aAfterMF.SetValue( aAfterMF.Normalize(rSpace.GetRight()), FUNIT_TWIP );
    }
    else
    {
        aBeforeMF.SetEmptyFieldValue();
        aAfterMF.SetEmptyFieldValue();
    }
    aBeforeMF.SaveValue();
    aAfterMF.SaveValue();
    IndentModifyHdl(0);
}

SfxTabPage*  SwSectionIndentTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet)
{
    return new SwSectionIndentTabPage(pParent, rAttrSet);
}

void SwSectionIndentTabPage::SetWrtShell(SwWrtShell& rSh)
{
    //set sensible values at the preview
    aPreviewWin.SetAdjust(SVX_ADJUST_BLOCK);
    aPreviewWin.SetLastLine(SVX_ADJUST_BLOCK);
    const SwRect& rPageRect = rSh.GetAnyCurRect( RECT_PAGE, 0 );
    Size aPageSize(rPageRect.Width(), rPageRect.Height());
    aPreviewWin.SetSize(aPageSize);
}

IMPL_LINK_NOARG(SwSectionIndentTabPage, IndentModifyHdl)
{
    aPreviewWin.SetLeftMargin( static_cast< long >(aBeforeMF.Denormalize(aBeforeMF.GetValue(FUNIT_TWIP))) );
    aPreviewWin.SetRightMargin( static_cast< long >(aAfterMF.Denormalize(aAfterMF.GetValue(FUNIT_TWIP))) );
    aPreviewWin.Draw(sal_True);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
