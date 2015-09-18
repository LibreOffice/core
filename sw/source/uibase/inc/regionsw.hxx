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

#include <memory>
#include <map>

class SwWrtShell;

namespace sfx2
{
    class DocumentInserter;
    class FileDialogHelper;
}

// dialog "edit regions"
class SectRepr;
typedef std::map<size_t, std::unique_ptr<SectRepr>> SectReprs_t;

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
    SectReprs_t             m_SectReprs;
    const SwSection*        pCurrSect;
    sfx2::DocumentInserter* m_pDocInserter;
    VclPtr<vcl::Window>     m_pOldDefDlgParent;

    bool            bDontCheckPasswd :1;
    bool            bWeb            :1;

    Image  BuildBitmap(bool bProtect, bool bHidden);

    void    RecurseList( const SwSectionFormat* pFormat, SvTreeListEntry* pEntry);
    size_t  FindArrPos(const SwSectionFormat* pFormat);

    DECL_LINK_TYPED( GetFirstEntryHdl, SvTreeListBox *, void );
    DECL_LINK_TYPED( DeselectHdl, SvTreeListBox *, void );

    DECL_LINK_TYPED(OkHdl, Button*, void);
    DECL_LINK(NameEditHdl, void *);
    DECL_LINK( ConditionEditHdl, Edit * );

    DECL_LINK_TYPED( ChangePasswdHdl, Button *, void );
    DECL_LINK_TYPED( ChangeProtectHdl, Button *, void );
    DECL_LINK_TYPED( ChangeHideHdl, Button *, void );
    // #114856# edit in readonly sections
    DECL_LINK_TYPED( ChangeEditInReadonlyHdl, Button *, void );
    DECL_LINK_TYPED(ChangeDismissHdl, Button*, void);
    DECL_LINK_TYPED( UseFileHdl, Button*, void );
    DECL_LINK_TYPED(FileSearchHdl, Button*, void);
    DECL_LINK_TYPED(OptionsHdl, Button*, void);
    DECL_LINK( FileNameHdl, Edit* );
    DECL_LINK_TYPED( DDEHdl, Button*, void );
    DECL_LINK_TYPED( DlgClosedHdl, sfx2::FileDialogHelper*, void );
    DECL_LINK_TYPED( SubRegionEventHdl, VclWindowEvent&, void );

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

    DECL_LINK_TYPED( ChangeHideHdl, Button *, void );
    DECL_LINK_TYPED( ChangeProtectHdl, Button *, void );
    DECL_LINK_TYPED( ChangePasswdHdl, Button *, void );
    DECL_LINK(NameEditHdl, void *);
    DECL_LINK_TYPED( UseFileHdl, Button*, void );
    DECL_LINK_TYPED(FileSearchHdl, Button*, void );
    DECL_LINK_TYPED( DDEHdl, Button*, void );
    DECL_LINK_TYPED( DlgClosedHdl, sfx2::FileDialogHelper*, void );

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

class SwSectionFootnoteEndTabPage : public SfxTabPage
{
    VclPtr<CheckBox>        pFootnoteNtAtTextEndCB;

    VclPtr<CheckBox>        pFootnoteNtNumCB;
    VclPtr<FixedText>       pFootnoteOffsetLbl;
    VclPtr<NumericField>    pFootnoteOffsetField;

    VclPtr<CheckBox>        pFootnoteNtNumFormatCB;
    VclPtr<FixedText>       pFootnotePrefixFT;
    VclPtr<Edit>            pFootnotePrefixED;
    VclPtr<SwNumberingTypeListBox> pFootnoteNumViewBox;
    VclPtr<FixedText>       pFootnoteSuffixFT;
    VclPtr<Edit>            pFootnoteSuffixED;

    VclPtr<CheckBox>        pEndNtAtTextEndCB;

    VclPtr<CheckBox>        pEndNtNumCB;
    VclPtr<FixedText>       pEndOffsetLbl;
    VclPtr<NumericField>    pEndOffsetField;

    VclPtr<CheckBox>        pEndNtNumFormatCB;
    VclPtr<FixedText>       pEndPrefixFT;
    VclPtr<Edit>            pEndPrefixED;
    VclPtr<SwNumberingTypeListBox> pEndNumViewBox;
    VclPtr<FixedText>       pEndSuffixFT;
    VclPtr<Edit>            pEndSuffixED;

    DECL_LINK_TYPED( FootEndHdl, Button*, void );
    void ResetState( bool bFootnote, const SwFormatFootnoteEndAtTextEnd& );

public:
    SwSectionFootnoteEndTabPage( vcl::Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionFootnoteEndTabPage();
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
