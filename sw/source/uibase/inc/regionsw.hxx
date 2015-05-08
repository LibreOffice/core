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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_REGIONSW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_REGIONSW_HXX

#include <hintids.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <svtools/treelistbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <editeng/brushitem.hxx>

#include <condedit.hxx>
#include <section.hxx>
#include <fmtclds.hxx>
#include <fmtftntx.hxx>
#include <fmtclbl.hxx>
#include <numberingtypelistbox.hxx>
#include <editeng/frmdiritem.hxx>
#include <vcl/image.hxx>
#include <svx/paraprev.hxx>
#include <editeng/lrspitem.hxx>

#include <boost/ptr_container/ptr_set.hpp>

class SwWrtShell;

namespace sfx2
{
    class DocumentInserter;
    class FileDialogHelper;
}

// dialog "edit regions"
class SectRepr;
typedef SectRepr* SectReprPtr;
typedef boost::ptr_set<SectRepr> SectReprArr;

class SwEditRegionDlg : public SfxModalDialog
{
    VclPtr<Edit>           m_pCurName;
    VclPtr<SvTreeListBox>  m_pTree;

    VclPtr<TriStateBox>    m_pFileCB;
    VclPtr<CheckBox>       m_pDDECB;
    VclPtr<VclContainer>   m_pDDEFrame;
    VclPtr<FixedText>      m_pFileNameFT;
    VclPtr<FixedText>      m_pDDECommandFT;
    VclPtr<Edit>           m_pFileNameED;
    VclPtr<PushButton>     m_pFilePB;
    VclPtr<FixedText>      m_pSubRegionFT;
    VclPtr<ComboBox>       m_pSubRegionED;
    bool            m_bSubRegionsFilled;

    VclPtr<TriStateBox>    m_pProtectCB;
    VclPtr<CheckBox>       m_pPasswdCB;
    VclPtr<PushButton>     m_pPasswdPB;

    VclPtr<TriStateBox>    m_pHideCB;
    VclPtr<FixedText>      m_pConditionFT;
    VclPtr<ConditionEdit>  m_pConditionED;

    // #114856# edit in readonly sections
    VclPtr<TriStateBox>    m_pEditInReadonlyCB;

    VclPtr<OKButton>       m_pOK;
    VclPtr<PushButton>     m_pOptionsPB;
    VclPtr<PushButton>     m_pDismiss;
    ImageList       aImageIL;

    SwWrtShell&             rSh;
    SectReprArr             aSectReprArr;
    const SwSection*        pCurrSect;
    sfx2::DocumentInserter* m_pDocInserter;
    VclPtr<vcl::Window>     m_pOldDefDlgParent;

    bool            bDontCheckPasswd :1;
    bool            bWeb            :1;

    Image  BuildBitmap(bool bProtect, bool bHidden);

    void    RecurseList( const SwSectionFmt* pFmt, SvTreeListEntry* pEntry);
    size_t  FindArrPos(const SwSectionFmt* pFmt);

    DECL_LINK( GetFirstEntryHdl, SvTreeListBox * );
    DECL_LINK( DeselectHdl, SvTreeListBox * );

    DECL_LINK(OkHdl, void *);
    DECL_LINK(NameEditHdl, void *);
    DECL_LINK( ConditionEditHdl, Edit * );

    DECL_LINK( ChangePasswdHdl, Button * );
    DECL_LINK( ChangeProtectHdl, TriStateBox * );
    DECL_LINK( ChangeHideHdl, TriStateBox * );
    // #114856# edit in readonly sections
    DECL_LINK( ChangeEditInReadonlyHdl, TriStateBox * );
    DECL_LINK(ChangeDismissHdl, void *);
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK(FileSearchHdl, void *);
    DECL_LINK(OptionsHdl, void *);
    DECL_LINK( FileNameHdl, Edit* );
    DECL_LINK( DDEHdl, CheckBox* );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );
    DECL_LINK( SubRegionEventHdl, VclWindowEvent * );

    bool CheckPasswd(CheckBox* pBox = 0);

public:
    SwEditRegionDlg( vcl::Window* pParent, SwWrtShell& rWrtSh );
    virtual ~SwEditRegionDlg();
    virtual void dispose() SAL_OVERRIDE;

    void    SelectSection(const OUString& rSectionName);

};

// dialog "insert region"
class SwInsertSectionTabPage : public SfxTabPage
{
    VclPtr<ComboBox>       m_pCurName;

    VclPtr<CheckBox>       m_pFileCB;
    VclPtr<CheckBox>       m_pDDECB;
    VclPtr<FixedText>      m_pDDECommandFT;
    VclPtr<FixedText>      m_pFileNameFT;
    VclPtr<Edit>           m_pFileNameED;
    VclPtr<PushButton>     m_pFilePB;
    VclPtr<FixedText>      m_pSubRegionFT;
    VclPtr<ComboBox>       m_pSubRegionED;

    VclPtr<CheckBox>       m_pProtectCB;
    VclPtr<CheckBox>       m_pPasswdCB;
    VclPtr<PushButton>     m_pPasswdPB;

    VclPtr<CheckBox>       m_pHideCB;
    VclPtr<FixedText>      m_pConditionFT;
    VclPtr<ConditionEdit>  m_pConditionED;

    // #114856# edit in readonly sections
    VclPtr<CheckBox>       m_pEditInReadonlyCB;

    OUString        m_sFileName;
    OUString        m_sFilterName;
    OUString        m_sFilePasswd;

    ::com::sun::star::uno::Sequence <sal_Int8 > m_aNewPasswd;
    SwWrtShell*             m_pWrtSh;
    sfx2::DocumentInserter* m_pDocInserter;
    VclPtr<vcl::Window>     m_pOldDefDlgParent;

    DECL_LINK( ChangeHideHdl, CheckBox * );
    // #114856# edit in readonly sections
    DECL_STATIC_LINK(SwInsertSectionTabPage, ChangeEditInReadonlyHdl, void *);
    DECL_LINK( ChangeProtectHdl, CheckBox * );
    DECL_LINK( ChangePasswdHdl, Button * );
    DECL_LINK(NameEditHdl, void *);
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK(FileSearchHdl, void *);
    DECL_LINK( DDEHdl, CheckBox* );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );

public:
    SwInsertSectionTabPage(vcl::Window *pParent, const SfxItemSet &rAttrSet);
    virtual ~SwInsertSectionTabPage();
    virtual void dispose() SAL_OVERRIDE;

    void    SetWrtShell(SwWrtShell& rSh);

    virtual bool        FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* ) SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);
};

class SwSectionFtnEndTabPage : public SfxTabPage
{
    VclPtr<CheckBox>        pFtnNtAtTextEndCB;

    VclPtr<CheckBox>        pFtnNtNumCB;
    VclPtr<FixedText>       pFtnOffsetLbl;
    VclPtr<NumericField>    pFtnOffsetFld;

    VclPtr<CheckBox>        pFtnNtNumFmtCB;
    VclPtr<FixedText>       pFtnPrefixFT;
    VclPtr<Edit>            pFtnPrefixED;
    VclPtr<SwNumberingTypeListBox> pFtnNumViewBox;
    VclPtr<FixedText>       pFtnSuffixFT;
    VclPtr<Edit>            pFtnSuffixED;

    VclPtr<CheckBox>        pEndNtAtTextEndCB;

    VclPtr<CheckBox>        pEndNtNumCB;
    VclPtr<FixedText>       pEndOffsetLbl;
    VclPtr<NumericField>    pEndOffsetFld;

    VclPtr<CheckBox>        pEndNtNumFmtCB;
    VclPtr<FixedText>       pEndPrefixFT;
    VclPtr<Edit>            pEndPrefixED;
    VclPtr<SwNumberingTypeListBox> pEndNumViewBox;
    VclPtr<FixedText>       pEndSuffixFT;
    VclPtr<Edit>            pEndSuffixED;

    DECL_LINK( FootEndHdl, CheckBox * );
    void ResetState( bool bFtn, const SwFmtFtnEndAtTxtEnd& );

public:
    SwSectionFtnEndTabPage( vcl::Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionFtnEndTabPage();
    virtual void dispose() SAL_OVERRIDE;

    virtual bool        FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* ) SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);
};

class SwSectionIndentTabPage : public SfxTabPage
{
    VclPtr<MetricField>       m_pBeforeMF;
    VclPtr<MetricField>       m_pAfterMF;
    VclPtr<SvxParaPrevWindow> m_pPreviewWin;

    DECL_LINK(IndentModifyHdl, void *);
public:
    SwSectionIndentTabPage( vcl::Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionIndentTabPage();
    virtual void dispose() SAL_OVERRIDE;

    virtual bool        FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* ) SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    void    SetWrtShell(SwWrtShell& rSh);
};

class SwInsertSectionTabDialog : public SfxTabDialog
{
    SwWrtShell&     rWrtSh;
    ::std::unique_ptr<SwSectionData> m_pSectionData;

    sal_uInt16 m_nSectionPageId;
    sal_uInt16 m_nColumnPageId;
    sal_uInt16 m_nBackPageId;
    sal_uInt16 m_nNotePageId;
    sal_uInt16 m_nIndentPage;

protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;
    virtual short   Ok() SAL_OVERRIDE;
public:
    SwInsertSectionTabDialog(vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwInsertSectionTabDialog();

    void        SetSectionData(SwSectionData const& rSect);
    SwSectionData * GetSectionData() { return m_pSectionData.get(); }
};

class SwSectionPropertyTabDialog : public SfxTabDialog
{
    SwWrtShell& rWrtSh;

    sal_uInt16 m_nColumnPageId;
    sal_uInt16 m_nBackPageId;
    sal_uInt16 m_nNotePageId;
    sal_uInt16 m_nIndentPage;

protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;
public:
    SwSectionPropertyTabDialog(vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwSectionPropertyTabDialog();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
