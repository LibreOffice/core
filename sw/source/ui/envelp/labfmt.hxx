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


#ifndef _LABFMT_HXX
#define _LABFMT_HXX

#include "swuilabimp.hxx"
#include "labimg.hxx"
#include <vcl/msgbox.hxx>
class SwLabFmtPage;

// class SwLabPreview -------------------------------------------------------

class SwLabPreview : public Window
{
    long lOutWPix;
    long lOutHPix;
    long lOutWPix23;
    long lOutHPix23;

    Color aGrayColor;

    String aHDistStr;
    String aVDistStr;
    String aWidthStr;
    String aHeightStr;
    String aLeftStr;
    String aUpperStr;
    String aColsStr;
    String aRowsStr;

    long lHDistWidth;
    long lVDistWidth;
    long lHeightWidth;
    long lLeftWidth;
    long lUpperWidth;
    long lColsWidth;

    long lXWidth;
    long lXHeight;

    SwLabItem aItem;

    void Paint(const Rectangle&);

    void DrawArrow(const Point& rP1, const Point& rP2, sal_Bool bArrow);

    using Window::GetParent;
    SwLabFmtPage* GetParent() {return (SwLabFmtPage*) Window::GetParent();}

    using Window::Update;

public:

     SwLabPreview(const SwLabFmtPage* pParent, const ResId& rResID);
    ~SwLabPreview();

    void Update(const SwLabItem& rItem);
};

// class SwLabFmtPage -------------------------------------------------------

class SwLabFmtPage : public SfxTabPage
{
    FixedInfo       aMakeFI;
    FixedInfo       aTypeFI;
    SwLabPreview aPreview;
    FixedText    aHDistText;
    MetricField  aHDistField;
    FixedText    aVDistText;
    MetricField  aVDistField;
    FixedText    aWidthText;
    MetricField  aWidthField;
    FixedText    aHeightText;
    MetricField  aHeightField;
    FixedText    aLeftText;
    MetricField  aLeftField;
    FixedText    aUpperText;
    MetricField  aUpperField;
    FixedText    aColsText;
    NumericField aColsField;
    FixedText    aRowsText;
    NumericField aRowsField;
    FixedText    aPaperWidthText;
    MetricField  aPaperWidthField;
    FixedText    aPaperHeightText;
    MetricField  aPaperHeightField;
    PushButton   aSavePB;

    Timer aPreviewTimer;
    sal_Bool  bModified;

    SwLabItem    aItem;

     SwLabFmtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabFmtPage();

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( PreviewHdl, Timer * );
    DECL_LINK( LoseFocusHdl, Control * );
    DECL_LINK( SaveHdl, PushButton* );

    void ChangeMinMax();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    using Window::GetParent;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwLabItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);

    SwLabDlg* GetParent() {return (SwLabDlg*) SfxTabPage::GetParent()->GetParent();}
};
/* -----------------------------23.01.01 10:26--------------------------------

 ---------------------------------------------------------------------------*/
class SwSaveLabelDlg : public ModalDialog
{
    FixedLine       aOptionsFL;
    FixedText       aMakeFT;
    ComboBox        aMakeCB;
    FixedText       aTypeFT;
    Edit            aTypeED;

    OKButton        aOKPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;

    QueryBox        aQueryMB;

    sal_Bool        bSuccess;
    SwLabFmtPage*   pLabPage;
    SwLabRec&       rLabRec;

    DECL_LINK(OkHdl, OKButton*);
    DECL_LINK(ModifyHdl, Edit*);

public:
    SwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec);

    void    SetLabel(const rtl::OUString& rMake, const rtl::OUString& rType)
        {
            aMakeCB.SetText(String(rMake));
            aTypeED.SetText(String(rType));
        }
    sal_Bool GetLabel(SwLabItem& rItem);
};
#endif

