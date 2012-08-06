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
