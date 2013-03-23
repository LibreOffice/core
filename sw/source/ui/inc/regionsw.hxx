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
#pragma once
#if 1

#include <hintids.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
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
    FixedLine       aNameFL;
    Edit            aCurName;
    SvTreeListBox   aTree;

    FixedLine       aLinkFL;
    TriStateBox     aFileCB;
    CheckBox        aDDECB;
    FixedText       aFileNameFT;
    FixedText       aDDECommandFT;
    Edit            aFileNameED;
    PushButton      aFilePB;
    FixedText       aSubRegionFT;
    ComboBox        aSubRegionED;
    bool            bSubRegionsFilled;

    FixedLine       aProtectFL;
    TriStateBox     aProtectCB;
    CheckBox        aPasswdCB;
    PushButton      aPasswdPB;

    FixedLine       aHideFL;
    TriStateBox     aHideCB;
    FixedText       aConditionFT;
    ConditionEdit   aConditionED;

    // #114856# edit in readonly sections
    FixedLine       aPropertiesFL;
    TriStateBox     aEditInReadonlyCB;

    OKButton        aOK;
    CancelButton    aCancel;
    PushButton      aOptionsPB;
    PushButton      aDismiss;
    HelpButton      aHelp;
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
    FixedLine       aNameFL;
    ComboBox        aCurName;

    FixedLine       aLinkFL;
    CheckBox        aFileCB;
    CheckBox        aDDECB;
    FixedText       aDDECommandFT;
    FixedText       aFileNameFT;
    Edit            aFileNameED;
    PushButton      aFilePB;
    FixedText       aSubRegionFT;
    ComboBox        aSubRegionED;

    FixedLine       aProtectFL;
    CheckBox        aProtectCB;
    CheckBox        aPasswdCB;
    PushButton      aPasswdPB;

    FixedLine       aHideFL;
    CheckBox        aHideCB;
    FixedText       aConditionFT;
    ConditionEdit   aConditionED;

    // #114856# edit in readonly sections
    FixedLine       aPropertiesFL;
    CheckBox        aEditInReadonlyCB;

    String          m_sFileName;
    String          m_sFilterName;
    String          m_sFilePasswd;

//  SwFmtCol*       pCols;
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
    FixedLine       aFtnFL;
    CheckBox        aFtnNtAtTextEndCB;

    CheckBox        aFtnNtNumCB;
    FixedText       aFtnOffsetLbl;
    NumericField    aFtnOffsetFld;

    CheckBox        aFtnNtNumFmtCB;
    FixedText       aFtnPrefixFT;
    Edit            aFtnPrefixED;
    SwNumberingTypeListBox   aFtnNumViewBox;
    FixedText       aFtnSuffixFT;
    Edit            aFtnSuffixED;

    FixedLine       aEndFL;
    CheckBox        aEndNtAtTextEndCB;

    CheckBox        aEndNtNumCB;
    FixedText       aEndOffsetLbl;
    NumericField    aEndOffsetFld;

    CheckBox        aEndNtNumFmtCB;
    FixedText       aEndPrefixFT;
    Edit            aEndPrefixED;
    SwNumberingTypeListBox   aEndNumViewBox;
    FixedText       aEndSuffixFT;
    Edit            aEndSuffixED;


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
    FixedLine           aIndentFL;
    FixedText           aBeforeFT;
    MetricField         aBeforeMF;
    FixedText           aAfterFT;
    MetricField         aAfterMF;

    SvxParaPrevWindow   aPreviewWin;

    DECL_LINK(IndentModifyHdl, void *);
public:
    SwSectionIndentTabPage( Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionIndentTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void    SetWrtShell(SwWrtShell& rSh);
};

class SwInsertSectionTabDialog : public SfxTabDialog
{
    SwWrtShell&     rWrtSh;
    ::std::auto_ptr<SwSectionData> m_pSectionData;

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
protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
public:
    SwSectionPropertyTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwSectionPropertyTabDialog();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
