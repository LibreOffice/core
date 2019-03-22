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
#ifndef INCLUDED_SVX_SRCHDLG_HXX
#define INCLUDED_SVX_SRCHDLG_HXX

#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/svmedit.hxx>
#include <svl/srchdefs.hxx>
#include <svx/svxdllapi.h>
#include <memory>
#include <vector>

class SvxSearchItem;
class SfxStyleSheetBasePool;
class SvxJSearchOptionsPage;
class SvxSearchController;
class VclAbstractDialog;
struct SearchDlg_Impl;
enum class ModifyFlags;
enum class TransliterationFlags;

struct SearchAttrItem
{
    sal_uInt16          nSlot;
    SfxPoolItem*    pItem;
};

typedef std::vector<SearchAttrItem> SrchAttrItemList;

class SVX_DLLPUBLIC SearchAttrItemList : private SrchAttrItemList
{
public:
    SearchAttrItemList() {}
    SearchAttrItemList( const SearchAttrItemList& rList );
    ~SearchAttrItemList();

    void            Put( const SfxItemSet& rSet );
    SfxItemSet&     Get( SfxItemSet& rSet );
    void            Clear();
    sal_uInt16      Count() const { return SrchAttrItemList::size(); }
    SearchAttrItem& operator[](sal_uInt16 nPos)
                        { return SrchAttrItemList::operator[]( nPos ); }
    SearchAttrItem& GetObject( sal_uInt16 nPos )
                        { return SrchAttrItemList::operator[]( nPos ); }

    // the pointer to the item is not being copied, so don't delete
    void Insert( const SearchAttrItem& rItem )
        { SrchAttrItemList::push_back( rItem ); }
    // deletes the pointer to the items
    void Remove(size_t nPos);
};

enum class SearchLabel
{
    Empty,
    End,
    Start,
    EndSheet,
    NotFound,
    StartWrapped,
    EndWrapped,
    NavElementNotFound
};

class SvxSearchDialog;
class SVX_DLLPUBLIC SvxSearchDialogWrapper : public SfxChildWindow
{
    std::shared_ptr<SvxSearchDialog> dialog;
public:
    SvxSearchDialogWrapper( vcl::Window*pParent, sal_uInt16 nId,
                            SfxBindings* pBindings, SfxChildWinInfo const * pInfo );

    virtual ~SvxSearchDialogWrapper () override;
    SvxSearchDialog *getDialog () { return dialog.get();}
    static void SetSearchLabel(const SearchLabel& rSL);
    static void SetSearchLabel(const OUString& sStr);
    static OUString GetSearchLabel();
    SFX_DECL_CHILDWINDOW_WITHID(SvxSearchDialogWrapper);
};

/**
    In this modeless dialog the attributes for a search are configured
    and a search is started from it. Several search types
    (search, search all, replace, replace all) are possible.

 */

class SVX_DLLPUBLIC SvxSearchDialog : public SfxModelessDialogController
{
friend class SvxSearchController;
friend class SvxSearchDialogWrapper;
friend class SvxJSearchOptionsDialog;

public:
    SvxSearchDialog(weld::Window* pParent, SfxChildWindow* pChildWin, SfxBindings& rBind );
    virtual ~SvxSearchDialog() override;

    virtual void    EndDialog();
    virtual void    Close();

    // Window
    virtual void    Activate() override;

    const SearchAttrItemList*   GetSearchItemList() const
                                    { return pSearchList.get(); }
    const SearchAttrItemList*   GetReplaceItemList() const
                                    { return pReplaceList.get(); }

    TransliterationFlags        GetTransliterationFlags() const;

    void SetDocWin(vcl::Window* pDocWin);
    void SetSrchFlag( bool bSuccess ) { mbSuccess = bSuccess; }
    bool GetSrchFlag() { return mbSuccess; }
    void            SetSaveToModule(bool b);

    void SetSearchLabel(const OUString& rStr) { m_xSearchLabel->set_label(rStr); }

private:
    bool            mbSuccess;
    bool            mbClosing;

    SfxBindings&    rBindings;
    bool            bWriter;
    bool            bSearch;
    bool            bFormat;
    bool            bReplaceBackwards;
    SearchOptionFlags  nOptions;
    bool            bSet;
    bool            bConstruct;
    ModifyFlags     nModifyFlag;
    OUString        aStylesStr;
    OUString        aLayoutStr;
    OUString        aLayoutWriterStr;
    OUString        aLayoutCalcStr;
    OUString        aCalcStr;

    std::vector<OUString> aSearchStrings;
    std::vector<OUString> aReplaceStrings;

    std::unique_ptr<SearchDlg_Impl>      pImpl;
    std::unique_ptr<SearchAttrItemList>  pSearchList;
    std::unique_ptr<SearchAttrItemList>  pReplaceList;
    std::unique_ptr<SvxSearchItem>       pSearchItem;

    std::unique_ptr<SvxSearchController> pSearchController;
    std::unique_ptr<SvxSearchController> pOptionsController;
    std::unique_ptr<SvxSearchController> pFamilyController;

    mutable TransliterationFlags
                            nTransliterationFlags;

    bool m_executingSubDialog = false;

    std::unique_ptr<weld::Frame> m_xSearchFrame;
    std::unique_ptr<weld::ComboBox> m_xSearchLB;
    std::unique_ptr<weld::ComboBox> m_xSearchTmplLB;
    std::unique_ptr<weld::Label> m_xSearchAttrText;
    std::unique_ptr<weld::Label> m_xSearchLabel;

    std::unique_ptr<weld::Frame> m_xReplaceFrame;
    std::unique_ptr<weld::ComboBox> m_xReplaceLB;
    std::unique_ptr<weld::ComboBox> m_xReplaceTmplLB;
    std::unique_ptr<weld::Label> m_xReplaceAttrText;

    std::unique_ptr<weld::Button> m_xSearchBtn;
    std::unique_ptr<weld::Button> m_xBackSearchBtn;
    std::unique_ptr<weld::Button> m_xSearchAllBtn;
    std::unique_ptr<weld::Button> m_xReplaceBtn;
    std::unique_ptr<weld::Button> m_xReplaceAllBtn;

    std::unique_ptr<weld::Frame> m_xComponentFrame;
    std::unique_ptr<weld::Button> m_xSearchComponent1PB;
    std::unique_ptr<weld::Button> m_xSearchComponent2PB;

    std::unique_ptr<weld::CheckButton> m_xMatchCaseCB;
    std::unique_ptr<weld::CheckButton> m_xSearchFormattedCB;
    std::unique_ptr<weld::CheckButton> m_xWordBtn;

    std::unique_ptr<weld::Button> m_xCloseBtn;
    std::unique_ptr<weld::CheckButton> m_xIncludeDiacritics;
    std::unique_ptr<weld::CheckButton> m_xIncludeKashida;
    std::unique_ptr<weld::Expander> m_xOtherOptionsExpander;
    std::unique_ptr<weld::CheckButton> m_xSelectionBtn;
    std::unique_ptr<weld::CheckButton> m_xRegExpBtn;
    std::unique_ptr<weld::CheckButton> m_xWildcardBtn;
    std::unique_ptr<weld::CheckButton> m_xSimilarityBox;
    std::unique_ptr<weld::Button> m_xSimilarityBtn;
    std::unique_ptr<weld::CheckButton> m_xLayoutBtn;
    std::unique_ptr<weld::CheckButton> m_xNotesBtn;
    std::unique_ptr<weld::CheckButton> m_xJapMatchFullHalfWidthCB;
    std::unique_ptr<weld::CheckButton> m_xJapOptionsCB;
    std::unique_ptr<weld::CheckButton> m_xReplaceBackwardsCB;
    std::unique_ptr<weld::Button> m_xJapOptionsBtn;

    std::unique_ptr<weld::Button> m_xAttributeBtn;
    std::unique_ptr<weld::Button> m_xFormatBtn;
    std::unique_ptr<weld::Button> m_xNoFormatBtn;

    std::unique_ptr<weld::Widget> m_xCalcGrid;
    std::unique_ptr<weld::Label> m_xCalcSearchInFT;
    std::unique_ptr<weld::ComboBox> m_xCalcSearchInLB;
    std::unique_ptr<weld::Label> m_xCalcSearchDirFT;
    std::unique_ptr<weld::RadioButton> m_xRowsBtn;
    std::unique_ptr<weld::RadioButton> m_xColumnsBtn;
    std::unique_ptr<weld::CheckButton> m_xAllSheetsCB;
    std::unique_ptr<weld::Label> m_xCalcStrFT;

    DECL_DLLPRIVATE_LINK( ModifyHdl_Impl, weld::ComboBox&, void );
    DECL_DLLPRIVATE_LINK( FlagHdl_Impl, weld::Button&, void );
    DECL_DLLPRIVATE_LINK( CommandHdl_Impl, weld::Button&, void );
    DECL_DLLPRIVATE_LINK(TemplateHdl_Impl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK( FocusHdl_Impl, weld::Widget&, void );
    DECL_DLLPRIVATE_LINK( LBSelectHdl_Impl, weld::ComboBox&, void );
    DECL_DLLPRIVATE_LINK(LoseFocusHdl_Impl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(FormatHdl_Impl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(NoFormatHdl_Impl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(AttributeHdl_Impl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK( TimeoutHdl_Impl, Timer*, void );
    SVX_DLLPRIVATE void ClickHdl_Impl(const weld::Widget* pCtrl);

    SVX_DLLPRIVATE void Construct_Impl();
    SVX_DLLPRIVATE void InitControls_Impl();
    SVX_DLLPRIVATE void ShowOptionalControls_Impl();
    SVX_DLLPRIVATE void Init_Impl( bool bHasItemSet );
    SVX_DLLPRIVATE void InitAttrList_Impl( const SfxItemSet* pSSet,
                                       const SfxItemSet* pRSet );
    SVX_DLLPRIVATE void Remember_Impl( const OUString &rStr, bool bSearch );
    SVX_DLLPRIVATE void PaintAttrText_Impl();
    SVX_DLLPRIVATE OUString& BuildAttrText_Impl( OUString& rStr, bool bSrchFlag ) const;

    SVX_DLLPRIVATE void TemplatesChanged_Impl( SfxStyleSheetBasePool& rPool );
    SVX_DLLPRIVATE void EnableControls_Impl( const SearchOptionFlags nFlags );
    SVX_DLLPRIVATE void EnableControl_Impl(const weld::Widget& rCtrl);
    SVX_DLLPRIVATE void SetItem_Impl( const SvxSearchItem* pItem );

    SVX_DLLPRIVATE void SetModifyFlag_Impl(const weld::Widget* pCtrl);
    SVX_DLLPRIVATE void SaveToModule_Impl();

    SVX_DLLPRIVATE void ApplyTransliterationFlags_Impl( TransliterationFlags nSettings );
    SVX_DLLPRIVATE bool IsOtherOptionsExpanded();

    SVX_DLLPRIVATE short executeSubDialog(VclAbstractDialog * dialog);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
