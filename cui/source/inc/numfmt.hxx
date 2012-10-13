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
#ifndef _SVX_NUMFMT_HXX
#define _SVX_NUMFMT_HXX

//------------------------------------------------------------------------

#include <vcl/window.hxx>
#include <tools/color.hxx>
#include <tools/string.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <svx/langbox.hxx>
#include <svx/fontlb.hxx>

//------------------------------------------------------------------------

class SvxNumberFormatShell;
class SvxNumberInfoItem;
class vector;

//------------------------------------------------------------------------

class SvxNumberPreviewImpl : public Window
{
private:
    String          aPrevStr;
    Color           aPrevCol;
    sal_Int32       mnPos;
    sal_Unicode     mnChar;
    void            InitSettings( sal_Bool bForeground, sal_Bool bBackground );

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvxNumberPreviewImpl( Window* pParent, const ResId& rResId );
    ~SvxNumberPreviewImpl();

    void            NotifyChange( const String& rPrevStr, const Color* pColor = NULL );
};

// -----------------------------------------------------------------------

class SvxNumberFormatTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

public:
    ~SvxNumberFormatTabPage();

    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual int             DeactivatePage  ( SfxItemSet* pSet = NULL );

    void                    SetInfoItem( const SvxNumberInfoItem& rItem );
    void                    SetNumberFormatList( const SvxNumberInfoItem& rItem )
                                { SetInfoItem( rItem ); }

    void                    SetOkHdl( const Link& rOkHandler );
    void                    HideLanguage(sal_Bool nFlag=sal_True);
    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual void            PageCreated (SfxAllItemSet aSet);
private:
    SvxNumberFormatTabPage( Window* pParent,
                            const SfxItemSet& rCoreAttrs );
    FixedText               aFtCategory;
    ListBox                 aLbCategory;
    FixedText               aFtFormat;
    ListBox                 aLbCurrency;
    SvxFontListBox          aLbFormat;
    FixedText               aFtLanguage;
    SvxLanguageBox          aLbLanguage;
    CheckBox                aCbSourceFormat;
    SvxNumberPreviewImpl    aWndPreview;
    FixedLine               aFlOptions;
    FixedText               aFtDecimals;
    NumericField            aEdDecimals;
    CheckBox                aBtnNegRed;
    FixedText               aFtLeadZeroes;
    NumericField            aEdLeadZeroes;
    CheckBox                aBtnThousand;

    FixedText               aFtEdFormat;
    Edit                    aEdFormat;
    ImageButton             aIbAdd;
    ImageButton             aIbInfo;
    ImageButton             aIbRemove;

    FixedText               aFtComment;
    Edit                    aEdComment;
    Timer                   aResetWinTimer;


    SvxNumberInfoItem*      pNumItem;
    SvxNumberFormatShell*   pNumFmtShell;
    sal_uLong               nInitFormat;
    Link                    fnOkHdl;

    sal_Bool                bNumItemFlag; ///< for handling with DocShell
    sal_Bool                bOneAreaFlag;
    short                   nFixedCategory;

    long                    nCatHeight;

    long                    nCurFormatY;
    long                    nCurFormatHeight;
    long                    nStdFormatY;
    long                    nStdFormatHeight;
    String sAutomaticEntry;

    Window*                 pLastActivWindow;

#ifdef _SVX_NUMFMT_CXX
    void    Init_Impl();
    void    FillCurrencyBox();
    void    FillFormatListBox_Impl( std::vector<String*>& rEntries );
    void    UpdateOptions_Impl( sal_Bool bCheckCatChange );
    void    UpdateFormatListBox_Impl( sal_uInt16 bCat, sal_Bool bUpdateEdit );
    void    DeleteEntryList_Impl( std::vector<String*>& rEntries );
    void    Obstructing();
    void    EnableBySourceFormat_Impl();
    void    SetCategory( sal_uInt16 nPos );
    String  GetExpColorString( Color*& rpPreviewColor, const String& aFormatStr, short nTmpCatPos );
    void    MakePreviewText( const String& rFormat );
    void    ChangePreviewText( sal_uInt16 nPos );
    void    AddAutomaticLanguage_Impl(LanguageType eAutoLang, sal_Bool bSelect);
    // Handler
    DECL_LINK( LostFocusHdl_Impl, Edit* pEd );
    DECL_LINK( DoubleClickHdl_Impl, SvxFontListBox* pLb );
    DECL_LINK( SelFormatHdl_Impl, void * );
    DECL_LINK( ClickHdl_Impl, ImageButton* pIB );
    DECL_LINK( EditHdl_Impl, Edit* pEdFormat );
    DECL_LINK( OptHdl_Impl, void * );
    DECL_LINK(TimeHdl_Impl, void *);

#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
