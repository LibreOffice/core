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



#ifndef SC_TPVIEW_HXX
#define SC_TPVIEW_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/strarray.hxx>

//========================================================================
// View-Optionen:

class ScViewOptions;

//========================================================================
// TabPage Inhalte

class ScTpContentOptions : public SfxTabPage
{
    FixedLine       aLinesGB;
    CheckBox        aGridCB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;
    CheckBox        aBreakCB;
    CheckBox        aGuideLineCB;
    CheckBox        aHandleCB;
    CheckBox        aBigHandleCB;

    FixedLine       aSeparator1FL;

    FixedLine       aDisplayGB;
    CheckBox        aFormulaCB;
    CheckBox        aNilCB;
    CheckBox        aAnnotCB;
    CheckBox        aValueCB;
    CheckBox        aAnchorCB;
    CheckBox        aClipMarkCB;
    CheckBox        aRangeFindCB;

    FixedLine       aObjectGB;
    FixedText       aObjGrfFT;
    ListBox         aObjGrfLB;
    FixedText       aDiagramFT;
    ListBox         aDiagramLB;
    FixedText       aDrawFT;
    ListBox         aDrawLB;

    FixedLine       aZoomGB;
    CheckBox        aSyncZoomCB;

    FixedLine       aSeparator2FL;

    FixedLine       aWindowGB;
    CheckBox        aRowColHeaderCB;
    CheckBox        aHScrollCB;
    CheckBox        aVScrollCB;
    CheckBox        aTblRegCB;
    CheckBox        aOutlineCB;

    ScViewOptions*  pLocalOptions;

    void    InitGridOpt();
    DECL_LINK( GridHdl, CheckBox* );
    DECL_LINK( SelLbObjHdl, ListBox* );
    DECL_LINK( CBHdl, CheckBox* );

            ScTpContentOptions( Window*         pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpContentOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );


};

//========================================================================
// TabPage Layout
class ScDocument;
class ScTpLayoutOptions : public SfxTabPage
{
    FixedLine       aUnitGB;
    FixedText       aUnitFT;
    ListBox         aUnitLB;
    FixedText       aTabFT;
    MetricField     aTabMF;

    FixedLine       aSeparatorFL;
    FixedLine       aLinkGB;
    FixedText       aLinkFT;
    RadioButton     aAlwaysRB;
    RadioButton     aRequestRB;
    RadioButton     aNeverRB;

    FixedLine       aOptionsGB;
    CheckBox        aAlignCB;
    ListBox         aAlignLB;
    CheckBox        aEditModeCB;
    CheckBox        aFormatCB;
    CheckBox        aExpRefCB;
    CheckBox        aMarkHdrCB;
    CheckBox        aTextFmtCB;
    CheckBox        aReplWarnCB;

    SvxStringArray  aUnitArr;

    DECL_LINK( CBHdl, CheckBox* );

    DECL_LINK(MetricHdl, ListBox*);
    DECL_LINK( AlignHdl, CheckBox* );

    ScDocument *pDoc;

    DECL_LINK(  UpdateHdl, CheckBox* );

            ScTpLayoutOptions( Window*          pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpLayoutOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    void                SetDocument(ScDocument* pPtr){pDoc = pPtr;}

};
//========================================================================

#endif // SC_TPUSRLST_HXX

