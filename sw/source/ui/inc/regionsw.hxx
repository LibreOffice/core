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
#ifndef _REGIONSW_HXX
#define _REGIONSW_HXX

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

/*************************************************************************
    dialog "edit regions"
*************************************************************************/
class SectRepr;
typedef SectRepr* SectReprPtr;
typedef boost::ptr_set<SectRepr> SectReprArr;

class SwEditRegionDlg : public SfxModalDialog
{
    Edit*           m_pCurName;
    SvTreeListBox*  m_pTree;

    TriStateBox*    m_pFileCB;
    CheckBox*       m_pDDECB;
    VclContainer*   m_pDDEFrame;
    FixedText*      m_pFileNameFT;
    FixedText*      m_pDDECommandFT;
    Edit*           m_pFileNameED;
    PushButton*     m_pFilePB;
    FixedText*      m_pSubRegionFT;
    ComboBox*       m_pSubRegionED;
    bool            m_bSubRegionsFilled;

    TriStateBox*    m_pProtectCB;
    CheckBox*       m_pPasswdCB;
    PushButton*     m_pPasswdPB;

    TriStateBox*    m_pHideCB;
    FixedText*      m_pConditionFT;
    ConditionEdit*  m_pConditionED;

    // #114856# edit in readonly sections
    TriStateBox*    m_pEditInReadonlyCB;

    OKButton*       m_pOK;
    PushButton*     m_pOptionsPB;
    PushButton*     m_pDismiss;
    ImageList       aImageIL;

    SwWrtShell&             rSh;
    SectReprArr             aSectReprArr;
    const SwSection*        pCurrSect;
    sfx2::DocumentInserter* m_pDocInserter;
    Window*                 m_pOldDefDlgParent;

    sal_Bool            bDontCheckPasswd :1;
    sal_Bool            bWeb            :1;


    Image  BuildBitmap(sal_Bool bProtect,sal_Bool bHidden);

    void    RecurseList( const SwSectionFmt* pFmt, SvTreeListEntry* pEntry);
    sal_uInt16  FindArrPos(const SwSectionFmt* pFmt);

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

    sal_Bool CheckPasswd(CheckBox* pBox = 0);

public:
    SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh );
    virtual ~SwEditRegionDlg();

    void    SelectSection(const String& rSectionName);

};

/*************************************************************************
    dialog "insert region"
*************************************************************************/
class SwInsertSectionTabPage : public SfxTabPage
{
    ComboBox*       m_pCurName;

    CheckBox*       m_pFileCB;
    CheckBox*       m_pDDECB;
    FixedText*      m_pDDECommandFT;
    FixedText*      m_pFileNameFT;
    Edit*           m_pFileNameED;
    PushButton*     m_pFilePB;
    FixedText*      m_pSubRegionFT;
    ComboBox*       m_pSubRegionED;

    CheckBox*       m_pProtectCB;
    CheckBox*       m_pPasswdCB;
    PushButton*     m_pPasswdPB;

    CheckBox*       m_pHideCB;
    FixedText*      m_pConditionFT;
    ConditionEdit*  m_pConditionED;

    // #114856# edit in readonly sections
    CheckBox*       m_pEditInReadonlyCB;

    String          m_sFileName;
    String          m_sFilterName;
    String          m_sFilePasswd;

    ::com::sun::star::uno::Sequence <sal_Int8 > m_aNewPasswd;
    SwWrtShell*             m_pWrtSh;
    sfx2::DocumentInserter* m_pDocInserter;
    Window*                 m_pOldDefDlgParent;

    DECL_LINK( ChangeHideHdl, CheckBox * );
    // #114856# edit in readonly sections
    DECL_LINK(ChangeEditInReadonlyHdl, void *);
    DECL_LINK( ChangeProtectHdl, CheckBox * );
    DECL_LINK( ChangePasswdHdl, Button * );
    DECL_LINK(NameEditHdl, void *);
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK(FileSearchHdl, void *);
    DECL_LINK( DDEHdl, CheckBox* );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );

public:
    SwInsertSectionTabPage(Window *pParent, const SfxItemSet &rAttrSet);
    virtual ~SwInsertSectionTabPage();

    void    SetWrtShell(SwWrtShell& rSh);

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};

class SwSectionFtnEndTabPage : public SfxTabPage
{
    CheckBox*        pFtnNtAtTextEndCB;

    CheckBox*        pFtnNtNumCB;
    FixedText*       pFtnOffsetLbl;
    NumericField*    pFtnOffsetFld;

    CheckBox*        pFtnNtNumFmtCB;
    FixedText*       pFtnPrefixFT;
    Edit*            pFtnPrefixED;
    SwNumberingTypeListBox*   pFtnNumViewBox;
    FixedText*       pFtnSuffixFT;
    Edit*            pFtnSuffixED;

    CheckBox*        pEndNtAtTextEndCB;

    CheckBox*        pEndNtNumCB;
    FixedText*       pEndOffsetLbl;
    NumericField*    pEndOffsetFld;

    CheckBox*        pEndNtNumFmtCB;
    FixedText*       pEndPrefixFT;
    Edit*            pEndPrefixED;
    SwNumberingTypeListBox*   pEndNumViewBox;
    FixedText*       pEndSuffixFT;
    Edit*            pEndSuffixED;


    DECL_LINK( FootEndHdl, CheckBox * );
    void ResetState( sal_Bool bFtn, const SwFmtFtnEndAtTxtEnd& );

public:
    SwSectionFtnEndTabPage( Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionFtnEndTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};

class SwSectionIndentTabPage : public SfxTabPage
{
    MetricField*       m_pBeforeMF;
    MetricField*       m_pAfterMF;
    SvxParaPrevWindow* m_pPreviewWin;

    DECL_LINK(IndentModifyHdl, void *);
public:
    SwSectionIndentTabPage( Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionIndentTabPage();

    virtual sal_Bool    FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void    SetWrtShell(SwWrtShell& rSh);
};

class SwInsertSectionTabDialog : public SfxTabDialog
{
    SwWrtShell&     rWrtSh;
    ::std::auto_ptr<SwSectionData> m_pSectionData;

    sal_uInt16 m_nSectionPageId;
    sal_uInt16 m_nColumnPageId;
    sal_uInt16 m_nBackPageId;
    sal_uInt16 m_nNotePageId;
    sal_uInt16 m_nIndentPage;

protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual short   Ok();
public:
    SwInsertSectionTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
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
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
public:
    SwSectionPropertyTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwSectionPropertyTabDialog();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
