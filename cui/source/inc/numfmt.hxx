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
#ifndef INCLUDED_CUI_SOURCE_INC_NUMFMT_HXX
#define INCLUDED_CUI_SOURCE_INC_NUMFMT_HXX


#include <rtl/ustring.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <svx/fontlb.hxx>
#include <tools/color.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/window.hxx>


class SvxNumberFormatShell;
class SvxNumberInfoItem;
class vector;


class SvxNumberPreview : public vcl::Window
{
private:
    OUString        aPrevStr;
    Color           aPrevCol;
    sal_Int32       mnPos;
    sal_Unicode     mnChar;
    void            InitSettings( bool bForeground, bool bBackground );

protected:
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SvxNumberPreview(vcl::Window* pParent, WinBits nStyle = WB_BORDER);

    void            NotifyChange( const OUString& rPrevStr, const Color* pColor = nullptr );
};


class SvxNumberFormatTabPage : public SfxTabPage
{
    friend class VclPtr<SvxNumberFormatTabPage>;
    using SfxTabPage::DeactivatePage;
    static const sal_uInt16 pRanges[];

public:
    virtual ~SvxNumberFormatTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );
    // Returns area information.
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual DeactivateRC    DeactivatePage  ( SfxItemSet* pSet ) override;

    void                    SetInfoItem( const SvxNumberInfoItem& rItem );
    void                    SetNumberFormatList( const SvxNumberInfoItem& rItem )
                                { SetInfoItem( rItem ); }

    void                    SetOkHdl( const Link<SfxPoolItem*,void>& rOkHandler );
    void                    HideLanguage(bool bFlag=true);
    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
private:
    SvxNumberFormatTabPage( vcl::Window* pParent,
                            const SfxItemSet& rCoreAttrs );
    VclPtr<FixedText>              m_pFtCategory;
    VclPtr<ListBox>                m_pLbCategory;
    VclPtr<FixedText>              m_pFtFormat;
    VclPtr<ListBox>                m_pLbCurrency;
    VclPtr<SvxFontListBox>         m_pLbFormat;
    VclPtr<FixedText>              m_pFtLanguage;
    VclPtr<SvxLanguageBox>         m_pLbLanguage;
    VclPtr<CheckBox>               m_pCbSourceFormat;
    VclPtr<SvxNumberPreview>       m_pWndPreview;
    VclPtr<FixedText>              m_pFtOptions;
    VclPtr<FixedText>              m_pFtDecimals;
    VclPtr<NumericField>           m_pEdDecimals;
    VclPtr<FixedText>              m_pFtDenominator;
    VclPtr<NumericField>           m_pEdDenominator;
    VclPtr<CheckBox>               m_pBtnNegRed;
    VclPtr<FixedText>              m_pFtLeadZeroes;
    VclPtr<NumericField>           m_pEdLeadZeroes;
    VclPtr<CheckBox>               m_pBtnThousand;
    VclPtr<CheckBox>               m_pBtnEngineering;

    VclPtr<VclContainer>           m_pFormatCodeFrame;
    VclPtr<Edit>                   m_pEdFormat;
    VclPtr<PushButton>             m_pIbAdd;
    VclPtr<PushButton>             m_pIbInfo;
    VclPtr<PushButton>             m_pIbRemove;

    VclPtr<FixedText>              m_pFtComment;
    VclPtr<Edit>                   m_pEdComment;
    Timer                   aResetWinTimer;


    SvxNumberInfoItem*      pNumItem;
    SvxNumberFormatShell*   pNumFmtShell;
    sal_uLong               nInitFormat;
    Link<SfxPoolItem*,void> fnOkHdl;

    bool                bNumItemFlag; ///< for handling with DocShell
    bool                bOneAreaFlag;
    short                   nFixedCategory;

    OUString sAutomaticEntry;

    VclPtr<vcl::Window>            pLastActivWindow;

    void    Init_Impl();
    void    FillCurrencyBox();
    void    FillFormatListBox_Impl( std::vector<OUString>& rEntries );
    void    UpdateOptions_Impl( bool bCheckCatChange );
    void    UpdateFormatListBox_Impl( bool bCat, bool bUpdateEdit );
    void    UpdateThousandEngineeringCheckBox();
    void    UpdateDecimalsDenominatorEditBox();
    void    Obstructing();
    void    EnableBySourceFormat_Impl();
    void    SetCategory( sal_uInt16 nPos );
    OUString  GetExpColorString( Color*& rpPreviewColor, const OUString& aFormatStr, short nTmpCatPos );
    void    MakePreviewText( const OUString& rFormat );
    void    ChangePreviewText( sal_uInt16 nPos );
    void    AddAutomaticLanguage_Impl(LanguageType eAutoLang, bool bSelect);
    bool    Click_Impl(PushButton* pIB);
    // Handler
    DECL_LINK_TYPED( LostFocusHdl_Impl, Control&, void );
    DECL_LINK_TYPED( DoubleClickHdl_Impl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( SelFormatListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( SelFormatTreeListBoxHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( SelFormatClickHdl_Impl, Button*, void );
    void SelFormatHdl_Impl(void*);
    DECL_LINK_TYPED( ClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED( EditModifyHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( OptEditHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( OptClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED( TimeHdl_Impl, Timer *, void);
    void EditHdl_Impl(Edit*);
    void OptHdl_Impl(void*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
