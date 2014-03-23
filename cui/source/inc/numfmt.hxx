/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

//CHINA001 #define SVX_NUMVAL_STANDARD      -1234.12345678901234
//CHINA001 #define SVX_NUMVAL_CURRENCY      -1234
//CHINA001 #define SVX_NUMVAL_PERCENT       -0.1295
//CHINA001 #define SVX_NUMVAL_TIME      36525.5678935185
//CHINA001 #define SVX_NUMVAL_DATE      36525.5678935185
//CHINA001 #define SVX_NUMVAL_BOOLEAN       1

//------------------------------------------------------------------------

class SvxNumberFormatShell;
class SvxNumberInfoItem;

//------------------------------------------------------------------------

class SvxNumberPreviewImpl : public Window
{
private:
    String          aPrevStr;
    Color           aPrevCol;

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

#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>

class SvxNumberFormatTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

public:
    ~SvxNumberFormatTabPage();

    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );
    static sal_uInt16*          GetRanges();

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual int             DeactivatePage  ( SfxItemSet* pSet = NULL );

    void                    SetInfoItem( const SvxNumberInfoItem& rItem );
    void                    SetNumberFormatList( const SvxNumberInfoItem& rItem )
                                { SetInfoItem( rItem ); }

    void                    SetOkHdl( const Link& rOkHandler );
    void                    HideLanguage(sal_Bool nFlag=sal_True);
    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual void            PageCreated (SfxAllItemSet aSet); //add CHINA001
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
    sal_uLong                   nInitFormat;
    Link                    fnOkHdl;

    sal_Bool                    bNumItemFlag; //Fuer Handling mit DocShell
    sal_Bool                    bOneAreaFlag;
    short                   nFixedCategory;

    long                    nCatHeight;

    long                    nCurFormatY;
    long                    nCurFormatHeight;
    long                    nStdFormatY;
    long                    nStdFormatHeight;
    LocalizedString sAutomaticEntry;

    Window*                 pLastActivWindow;

#ifdef _SVX_NUMFMT_CXX
    void    Init_Impl();
    void    FillCurrencyBox();
    void    FillFormatListBox_Impl( SvxDelStrgs& rEntries );
    void    UpdateOptions_Impl( sal_Bool bCheckCatChange );
    void    UpdateFormatListBox_Impl( sal_uInt16 bCat, sal_Bool bUpdateEdit );
    void    DeleteEntryList_Impl( SvxDelStrgs& rEntries );
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
    DECL_LINK( TimeHdl_Impl, Timer * );

#endif
};

#include <layout/layout-post.hxx>

#endif

