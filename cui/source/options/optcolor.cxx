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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include <svtools/headbar.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/scrbar.hxx>
#include <svx/xtable.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <boost/shared_ptr.hpp>
#include <svx/svxdlg.hxx>
#include <helpid.hrc>
#include <dialmgr.hxx>
#include "optcolor.hxx"
#include <cuires.hrc>
#include "optcolor.hrc"
#include <svx/dlgutil.hxx>

using namespace ::com::sun::star;
using namespace ::svtools;

#define GROUP_COUNT     7
#define GROUP_UNKNOWN   -1
#define GROUP_GENERAL   0
#define GROUP_WRITER    1
#define GROUP_HTML      2
#define GROUP_CALC      3
#define GROUP_DRAW      4
#define GROUP_BASIC     5
#define GROUP_SQL       6

const char* aColorLBHids[] =
{
     HID_COLORPAGE_DOCCOLOR_LB,
     HID_COLORPAGE_DOCBOUNDARIES_LB,
     HID_COLORPAGE_APPBACKGROUND_LB,
     HID_COLORPAGE_OBJECTBOUNDARIES_LB,
     HID_COLORPAGE_TABLEBOUNDARIES_LB,
     HID_COLORPAGE_FONTCOLOR_LB,
     HID_COLORPAGE_LINKS_LB,
     HID_COLORPAGE_LINKSVISITED_LB,
     HID_COLORPAGE_ANCHOR_LB,
     HID_COLORPAGE_SPELL_LB,
     HID_COLORPAGE_WRITERTEXTGRID_LB,
     HID_COLORPAGE_WRITERFIELDSHADINGS_LB,
     HID_COLORPAGE_WRITERIDXSHADINGS_LB,
     HID_COLORPAGE_WRITERDIRECTCURSOR_LB,
     HID_COLORPAGE_WRITERNOTESINDICATOR_LB,
     HID_COLORPAGE_WRITERSCRIPTINDICATOR_LB,
     HID_COLORPAGE_WRITERSECTIONBOUNDARIES_LB,
     HID_COLORPAGE_WRITERPAGEBREAKS_LB,
     HID_COLORPAGE_HTMLSGML_LB,
     HID_COLORPAGE_HTMLCOMMENT_LB,
     HID_COLORPAGE_HTMLKEYWORD_LB,
     HID_COLORPAGE_HTMLUNKNOWN_LB,
     HID_COLORPAGE_CALCGRID_LB,
     HID_COLORPAGE_CALCPAGEBREAK_LB,
     HID_COLORPAGE_CALCPAGEBREAKMANUAL_LB,
     HID_COLORPAGE_CALCPAGEBREAKAUTOMATIC_LB,
     HID_COLORPAGE_CALCDETECTIVE_LB,
     HID_COLORPAGE_CALCDETECTIVEERROR_LB,
     HID_COLORPAGE_CALCREFERENCE_LB,
     HID_COLORPAGE_CALCNOTESBACKGROUND_LB,
     HID_COLORPAGE_DRAWGRID_LB,
     HID_COLORPAGE_DRAWDRAWING_LB,
     HID_COLORPAGE_DRAWFILL_LB,
     HID_COLORPAGE_BASICIDENTIFIER_LB,
     HID_COLORPAGE_BASICCOMMENT_LB,
     HID_COLORPAGE_BASICNUMBER_LB,
     HID_COLORPAGE_BASICSTRING_LB,
     HID_COLORPAGE_BASICOPERATOR_LB,
     HID_COLORPAGE_BASICKEYWORD_LB,
     HID_COLORPAGE_BASICERROR_LB
};

const char* aColorCBHids[] =
{
     HID_COLORPAGE_DOCCOLOR_CB,
     HID_COLORPAGE_DOCBOUNDARIES_CB,
     HID_COLORPAGE_APPBACKGROUND_CB,
     HID_COLORPAGE_OBJECTBOUNDARIES_CB,
     HID_COLORPAGE_TABLEBOUNDARIES_CB,
     HID_COLORPAGE_FONTCOLOR_CB,
     HID_COLORPAGE_LINKS_CB,
     HID_COLORPAGE_LINKSVISITED_CB,
     HID_COLORPAGE_ANCHOR_CB,
     HID_COLORPAGE_SPELL_CB,
     HID_COLORPAGE_WRITERTEXTGRID_CB,
     HID_COLORPAGE_WRITERFIELDSHADINGS_CB,
     HID_COLORPAGE_WRITERIDXSHADINGS_CB,
     HID_COLORPAGE_WRITERDIRECTCURSOR_CB,
     HID_COLORPAGE_WRITERNOTESINDICATOR_CB,
     HID_COLORPAGE_WRITERSCRIPTINDICATOR_CB,
     HID_COLORPAGE_WRITERSECTIONBOUNDARIES_CB,
     HID_COLORPAGE_WRITERPAGEBREAKS_CB,
     HID_COLORPAGE_HTMLSGML_CB,
     HID_COLORPAGE_HTMLCOMMENT_CB,
     HID_COLORPAGE_HTMLKEYWORD_CB,
     HID_COLORPAGE_HTMLUNKNOWN_CB,
     HID_COLORPAGE_CALCGRID_CB,
     HID_COLORPAGE_CALCPAGEBREAK_CB,
     HID_COLORPAGE_CALCPAGEBREAKMANUAL_CB,
     HID_COLORPAGE_CALCPAGEBREAKAUTOMATIC_CB,
     HID_COLORPAGE_CALCDETECTIVE_CB,
     HID_COLORPAGE_CALCDETECTIVEERROR_CB,
     HID_COLORPAGE_CALCREFERENCE_CB,
     HID_COLORPAGE_CALCNOTESBACKGROUND_CB,
     HID_COLORPAGE_DRAWGRID_CB,
     HID_COLORPAGE_DRAWDRAWING_CB,
     HID_COLORPAGE_DRAWFILL_CB,
     HID_COLORPAGE_BASICIDENTIFIER_CB,
     HID_COLORPAGE_BASICCOMMENT_CB,
     HID_COLORPAGE_BASICNUMBER_CB,
     HID_COLORPAGE_BASICSTRING_CB,
     HID_COLORPAGE_BASICOPERATOR_CB,
     HID_COLORPAGE_BASICKEYWORD_CB,
     HID_COLORPAGE_BASICERROR_CB
};

/* -----------------------------2002/06/26 10:48------------------------------

 ---------------------------------------------------------------------------*/

class SvxExtFixedText_Impl : public FixedText
{
private:
    long            m_nGroupHeight;

protected:
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvxExtFixedText_Impl(Window* pParent, const ResId& rResId) :
        FixedText(pParent, rResId), m_nGroupHeight(0) {}

    inline long     GetGroupHeight() { return m_nGroupHeight; }
    inline void     SetGroupHeight( long _nHeight ) { m_nGroupHeight = _nHeight; }
};

/* -----------------------------25.03.2002 15:48------------------------------

 ---------------------------------------------------------------------------*/
class ColorConfigCtrl_Impl;
class ColorConfigWindow_Impl : public Window
{
    friend class ColorConfigCtrl_Impl;
    Window          aGeneralBackWN;
    SvxExtFixedText_Impl    aGeneralFT;
    FixedText       aDocColorFT;
    ColorListBox    aDocColorLB;
    Window          aDocColorWN;
    CheckBox        aDocBoundCB;
    ColorListBox    aDocBoundLB;
    Window          aDocBoundWN;
    FixedText       aAppBackFT;
    ColorListBox    aAppBackLB;
    Window          aAppBackWN;
    CheckBox        aObjBoundCB;
    ColorListBox    aObjBoundLB;
    Window          aObjBoundWN;
    CheckBox        aTableBoundCB;
    ColorListBox    aTableBoundLB;
    Window          aTableBoundWN;
    FixedText       aFontColorFT;
    ColorListBox    aFontColorLB;
    Window          aFontColorWN;
    CheckBox        aLinksCB;
    ColorListBox    aLinksLB;
    Window          aLinksWN;
    CheckBox        aLinksVisitedCB;
    ColorListBox    aLinksVisitedLB;
    Window          aLinksVisitedWN;
    FixedText       aSpellFT;
    ColorListBox    aSpellLB;
    Window          aSpellWN;
    FixedText       aSmarttagsFT;
    ColorListBox    aSmarttagsLB;
    Window          aSmarttagsWN;
    Window          aWriterBackWN;
    SvxExtFixedText_Impl    aWriterFT;
    FixedText       aWrtTextGridFT;
    ColorListBox    aWrtTextGridLB;
    Window          aWrtTextGridWN;
    CheckBox        aWrtFieldCB;
    ColorListBox    aWrtFieldLB;
    Window          aWrtFieldWN;
    CheckBox        aWrtIdxShadingBackCB;
    ColorListBox    aWrtIdxShadingBackLB;
    Window          aWrtIdxShadingBackWN;
    FixedText       aWrtScriptIndicatorFT;
    ColorListBox    aWrtScriptIndicatorLB;
    Window          aWrtScriptIndicatorWN;
    CheckBox        aWrtSectionBoundCB;
    ColorListBox    aWrtSectionBoundLB;
    Window          aWrtSectionBoundWN;
    FixedText       aWrtPageBreaksFT;
    ColorListBox    aWrtPageBreaksLB;
    Window          aWrtPageBreaksWN;
    FixedText       aWrtDirectCrsrFT;
    ColorListBox    aWrtDirectCrsrLB;
    Window          aWrtDirectCrsrWN;
    Window          aHTMLBackWN;
    SvxExtFixedText_Impl    aHTMLFT;
    FixedText       aHTMLSGMLFT;
    ColorListBox    aHTMLSGMLLB;
    Window          aHTMLSGMLWN;
    FixedText       aHTMLCommentFT;
    ColorListBox    aHTMLCommentLB;
    Window          aHTMLCommentWN;
    FixedText       aHTMLKeywdFT;
    ColorListBox    aHTMLKeywdLB;
    Window          aHTMLKeywdWN;
    FixedText       aHTMLUnknownFT;
    ColorListBox    aHTMLUnknownLB;
    Window          aHTMLUnknownWN;
    Window          aCalcBackWN;
    SvxExtFixedText_Impl    aCalcFT;
    FixedText       aCalcGridFT;
    ColorListBox    aCalcGridLB;
    Window          aCalcGridWN;
    FixedText       aCalcPageBreakFT;
    ColorListBox    aCalcPageBreakLB;
    Window          aCalcPageBreakWN;
    FixedText       aCalcPageBreakManualFT;
    ColorListBox    aCalcPageBreakManualLB;
    Window          aCalcPageBreakManualWN;
    FixedText       aCalcPageBreakAutoFT;
    ColorListBox    aCalcPageBreakAutoLB;
    Window          aCalcPageBreakAutoWN;
    FixedText       aCalcDetectiveFT;
    ColorListBox    aCalcDetectiveLB;
    Window          aCalcDetectiveWN;
    FixedText       aCalcDetectiveErrorFT;
    ColorListBox    aCalcDetectiveErrorLB;
    Window          aCalcDetectiveErrorWN;
    FixedText       aCalcReferenceFT;
    ColorListBox    aCalcReferenceLB;
    Window          aCalcReferenceWN;
    FixedText       aCalcNotesBackFT;
    ColorListBox    aCalcNotesBackLB;
    Window          aCalcNotesBackWN;
    Window          aDrawBackWN;
    SvxExtFixedText_Impl    aDrawFT;
    FixedText       aDrawGridFT;
    ColorListBox    aDrawGridLB;
    Window          aDrawGridWN;
    Window          aBasicBackWN;
    SvxExtFixedText_Impl    aBasicFT;
    FixedText       aBasicIdentifierFT;
    ColorListBox    aBasicIdentifierLB;
    Window          aBasicIdentifierWN;
    FixedText       aBasicCommentFT;
    ColorListBox    aBasicCommentLB;
    Window          aBasicCommentWN;
    FixedText       aBasicNumberFT;
    ColorListBox    aBasicNumberLB;
    Window          aBasicNumberWN;
    FixedText       aBasicStringFT;
    ColorListBox    aBasicStringLB;
    Window          aBasicStringWN;
    FixedText       aBasicOperatorFT;
    ColorListBox    aBasicOperatorLB;
    Window          aBasicOperatorWN;
    FixedText       aBasicKeywordFT;
    ColorListBox    aBasicKeywordLB;
    Window          aBasicKeywordWN;
    FixedText       aBasicErrorFT;
    ColorListBox    aBasicErrorLB;
    Window          aBasicErrorWN;
    Window          aSQLBackWN;
    SvxExtFixedText_Impl    aSQLFT;
    FixedText       aSQLIdentifierFT;
    ColorListBox    aSQLIdentifierLB;
    Window          aSQLIdentifierWN;
    FixedText       aSQLNumberFT;
    ColorListBox    aSQLNumberLB;
    Window          aSQLNumberWN;
    FixedText       aSQLStringFT;
    ColorListBox    aSQLStringLB;
    Window          aSQLStringWN;
    FixedText       aSQLOperatorFT;
    ColorListBox    aSQLOperatorLB;
    Window          aSQLOperatorWN;
    FixedText       aSQLKeywordFT;
    ColorListBox    aSQLKeywordLB;
    Window          aSQLKeywordWN;
    FixedText       aSQLParameterFT;
    ColorListBox    aSQLParameterLB;
    Window          aSQLParameterWN;
    FixedText       aSQLCommentFT;
    ColorListBox    aSQLCommentLB;
    Window          aSQLCommentWN;

    ::std::vector< SvxExtFixedText_Impl*>   aChapters;
    ::std::vector< Window* >                aChapterWins;
    ::std::vector< FixedText* >             aFixedTexts;
    ::std::vector< CheckBox* >              aCheckBoxes;
    ::std::vector< ColorListBox* >          aColorBoxes;
    ::std::vector< Window* >                aWindows; // [ColorConfigEntryCount]
    ::std::vector< ::boost::shared_ptr<SvxExtFixedText_Impl> >  m_aExtensionTitles;

    SvtModuleOptions    m_aModuleOptions;

    void            SetNewPosition( sal_Int32 _nFeature, Window* _pWin );

    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    ColorConfigWindow_Impl(Window* pParent, const ResId& rResId);
    ~ColorConfigWindow_Impl();

    inline const SvtModuleOptions&  GetModuleOptions() const { return m_aModuleOptions; }
};

sal_Bool lcl_isGroupVisible( sal_Int32 _nGroup, const SvtModuleOptions& _rModOptions )
{
    sal_Bool bRet = sal_True;

    switch ( _nGroup )
    {
        case GROUP_WRITER :
        case GROUP_HTML :
        {
            bRet = _rModOptions.IsModuleInstalled( SvtModuleOptions::E_SWRITER );
            break;
        }

        case GROUP_CALC :
        {
            bRet = _rModOptions.IsModuleInstalled( SvtModuleOptions::E_SCALC );
            break;
        }

        case GROUP_DRAW :
        {
            bRet = ( _rModOptions.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) ||
                     _rModOptions.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) );
            break;
        }
        case GROUP_SQL :
        {
            bRet = _rModOptions.IsModuleInstalled( SvtModuleOptions::E_SDATABASE );
            break;
        }
    }

    return bRet;
}

sal_Int16 lcl_getGroup( sal_Int32 _nFeature )
{
    if ( _nFeature >= ColorConfigEntryCount )
        return GROUP_COUNT; // feature of an extension

    sal_Int16 nRet = GROUP_UNKNOWN;

    switch ( _nFeature )
    {
        case DOCCOLOR :
        case DOCBOUNDARIES :
        case APPBACKGROUND :
        case OBJECTBOUNDARIES :
        case TABLEBOUNDARIES :
        case FONTCOLOR :
        case LINKS :
        case LINKSVISITED :
        case ANCHOR :
        case SPELL :
        case SMARTTAGS :
        {
            nRet = GROUP_GENERAL;
            break;
        }

        case WRITERTEXTGRID :
        case WRITERFIELDSHADINGS :
        case WRITERIDXSHADINGS :
        case WRITERDIRECTCURSOR :
        case WRITERSCRIPTINDICATOR :
        case WRITERSECTIONBOUNDARIES :
        case WRITERPAGEBREAKS :
        {
            nRet = GROUP_WRITER;
            break;
        }

        case HTMLSGML :
        case HTMLCOMMENT :
        case HTMLKEYWORD :
        case HTMLUNKNOWN :
        {
            nRet = GROUP_HTML;
            break;
        }

        case CALCGRID :
        case CALCPAGEBREAK :
        case CALCPAGEBREAKMANUAL :
        case CALCPAGEBREAKAUTOMATIC :
        case CALCDETECTIVE :
        case CALCDETECTIVEERROR :
        case CALCREFERENCE :
        case CALCNOTESBACKGROUND :
        {
            nRet = GROUP_CALC;
            break;
        }

        case DRAWGRID :
        case DRAWDRAWING :
        case DRAWFILL :
        {
            nRet = GROUP_DRAW;
            break;
        }

        case BASICIDENTIFIER :
        case BASICCOMMENT :
        case BASICNUMBER :
        case BASICSTRING :
        case BASICOPERATOR :
        case BASICKEYWORD :
        case BASICERROR :
        {
            nRet = GROUP_BASIC;
            break;
        }
        case SQLIDENTIFIER :
        case SQLNUMBER:
        case SQLSTRING:
        case SQLOPERATOR:
        case SQLKEYWORD:
        case SQLPARAMETER:
        case SQLCOMMENT:
        {
            nRet = GROUP_SQL;
            break;
        }
    }
    return nRet;
}

/* -----------------------------25.03.2002 17:05------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigWindow_Impl::ColorConfigWindow_Impl(Window* pParent, const ResId& rResId) :
        Window(pParent, rResId),
        aGeneralBackWN(this),
        aGeneralFT(&aGeneralBackWN,  ResId( FT_GENERAL, *rResId.GetResMgr() )),
        aDocColorFT(this, ResId(        FT_DOCCOLOR, *rResId.GetResMgr())),
        aDocColorLB(this, ResId(        LB_DOCCOLOR, *rResId.GetResMgr())),
        aDocColorWN(this, ResId(        WN_DOCCOLOR, *rResId.GetResMgr())),
        aDocBoundCB(this, ResId(        CB_DOCBOUND, *rResId.GetResMgr())),
        aDocBoundLB(this, ResId(        LB_DOCBOUND, *rResId.GetResMgr())),
        aDocBoundWN(this, ResId(      WN_DOCBOUND, *rResId.GetResMgr())),
        aAppBackFT(this, ResId(         FT_APPBACKGROUND, *rResId.GetResMgr())),
        aAppBackLB(this, ResId(         LB_APPBACKGROUND, *rResId.GetResMgr())),
        aAppBackWN(this, ResId(         WN_APPBACKGROUND, *rResId.GetResMgr())),
        aObjBoundCB(this, ResId(        CB_OBJECTBOUNDARIES, *rResId.GetResMgr())),
        aObjBoundLB(this, ResId(        LB_OBJECTBOUNDARIES, *rResId.GetResMgr())),
        aObjBoundWN(this, ResId(        WN_OBJECTBOUNDARIES, *rResId.GetResMgr())),
        aTableBoundCB(this, ResId(      CB_TABLEBOUNDARIES, *rResId.GetResMgr())),
        aTableBoundLB(this, ResId(      LB_TABLEBOUNDARIES, *rResId.GetResMgr())),
        aTableBoundWN(this, ResId(      WN_TABLEBOUNDARIES, *rResId.GetResMgr())),
        aFontColorFT(this, ResId(       FT_FONTCOLOR, *rResId.GetResMgr())),
        aFontColorLB(this, ResId(       LB_FONTCOLOR, *rResId.GetResMgr())),
        aFontColorWN(this, ResId(       WN_FONTCOLOR, *rResId.GetResMgr())),
        aLinksCB(this, ResId(           CB_LINKS, *rResId.GetResMgr())),
        aLinksLB(this, ResId(           LB_LINKS, *rResId.GetResMgr())),
        aLinksWN(this, ResId(           WN_LINKS, *rResId.GetResMgr())),
        aLinksVisitedCB(this, ResId(    CB_LINKSVISITED, *rResId.GetResMgr())),
        aLinksVisitedLB(this, ResId(    LB_LINKSVISITED, *rResId.GetResMgr())),
        aLinksVisitedWN(this, ResId(    WN_LINKSVISITED, *rResId.GetResMgr())),
        aSpellFT(this, ResId(        FT_SPELL, *rResId.GetResMgr())),
        aSpellLB(this, ResId(        LB_SPELL, *rResId.GetResMgr())),
        aSpellWN(this, ResId(        WN_SPELL, *rResId.GetResMgr())),
        aSmarttagsFT(this, ResId(        FT_SMARTTAGS, *rResId.GetResMgr() )),
        aSmarttagsLB(this, ResId(        LB_SMARTTAGS, *rResId.GetResMgr() )),
        aSmarttagsWN(this, ResId(        WN_SMARTTAGS, *rResId.GetResMgr() )),
        aWriterBackWN(this),
        aWriterFT(this, ResId(FT_WRITER, *rResId.GetResMgr())),
        aWrtTextGridFT(this, ResId(     FT_WRITERTEXTGRID, *rResId.GetResMgr())),
        aWrtTextGridLB(this, ResId(     LB_WRITERTEXTGRID, *rResId.GetResMgr())),
        aWrtTextGridWN(this, ResId(     WN_WRITERTEXTGRID, *rResId.GetResMgr())),
        aWrtFieldCB(this, ResId(        CB_WRITERFIELDSHADINGS, *rResId.GetResMgr())),
        aWrtFieldLB(this, ResId(        LB_WRITERFIELDSHADINGS, *rResId.GetResMgr())),
        aWrtFieldWN(this, ResId(        WN_WRITERFIELDSHADINGS, *rResId.GetResMgr())),
        aWrtIdxShadingBackCB(this, ResId(  CB_WRITERIDXSHADINGS, *rResId.GetResMgr())),
        aWrtIdxShadingBackLB(this, ResId(  LB_WRITERIDXSHADINGS, *rResId.GetResMgr())),
        aWrtIdxShadingBackWN(this, ResId(  WN_WRITERIDXSHADINGS, *rResId.GetResMgr())),
        aWrtScriptIndicatorFT(this, ResId(      FT_WRITERSCRIPTINDICATOR, *rResId.GetResMgr())),
        aWrtScriptIndicatorLB(this, ResId(      LB_WRITERSCRIPTINDICATOR, *rResId.GetResMgr())),
        aWrtScriptIndicatorWN(this, ResId(      WN_WRITERSCRIPTINDICATOR, *rResId.GetResMgr())),
        aWrtSectionBoundCB(this, ResId(      CB_WRITERSECTIONBOUNDARIES, *rResId.GetResMgr())),
        aWrtSectionBoundLB(this, ResId(      LB_WRITERSECTIONBOUNDARIES, *rResId.GetResMgr())),
        aWrtSectionBoundWN(this, ResId(      WN_WRITERSECTIONBOUNDARIES, *rResId.GetResMgr())),
        aWrtPageBreaksFT(this, ResId(      FT_WRITERPAGEBREAKS, *rResId.GetResMgr())),
        aWrtPageBreaksLB(this, ResId(      LB_WRITERPAGEBREAKS, *rResId.GetResMgr())),
        aWrtPageBreaksWN(this, ResId(      WN_WRITERPAGEBREAKS, *rResId.GetResMgr())),
        aWrtDirectCrsrFT(this, ResId(      FT_WRITERDIRECTCURSOR, *rResId.GetResMgr())),
        aWrtDirectCrsrLB(this, ResId(      LB_WRITERDIRECTCURSOR, *rResId.GetResMgr())),
        aWrtDirectCrsrWN(this, ResId(      WN_WRITERDIRECTCURSOR, *rResId.GetResMgr())),
        aHTMLBackWN(this),
        aHTMLFT(this, ResId(            FT_HTML, *rResId.GetResMgr())),
        aHTMLSGMLFT(this, ResId(        FT_HTMLSGML, *rResId.GetResMgr())),
        aHTMLSGMLLB(this, ResId(        LB_HTMLSGML, *rResId.GetResMgr())),
        aHTMLSGMLWN(this, ResId(        WN_HTMLSGML, *rResId.GetResMgr())),
        aHTMLCommentFT(this, ResId(     FT_HTMLCOMMENT, *rResId.GetResMgr())),
        aHTMLCommentLB(this, ResId(     LB_HTMLCOMMENT, *rResId.GetResMgr())),
        aHTMLCommentWN(this, ResId(     WN_HTMLCOMMENT, *rResId.GetResMgr())),
        aHTMLKeywdFT(this, ResId(       FT_HTMLKEYWORD, *rResId.GetResMgr())),
        aHTMLKeywdLB(this, ResId(       LB_HTMLKEYWORD, *rResId.GetResMgr())),
        aHTMLKeywdWN(this, ResId(       WN_HTMLKEYWORD, *rResId.GetResMgr())),
        aHTMLUnknownFT(this, ResId(     FT_HTMLUNKNOWN, *rResId.GetResMgr())),
        aHTMLUnknownLB(this, ResId(     LB_HTMLUNKNOWN, *rResId.GetResMgr())),
        aHTMLUnknownWN(this, ResId(     WN_HTMLUNKNOWN, *rResId.GetResMgr())),
        aCalcBackWN(this),
        aCalcFT(this, ResId(    FT_CALC, *rResId.GetResMgr())),
        aCalcGridFT(this, ResId(        FT_CALCGRID, *rResId.GetResMgr())),
        aCalcGridLB(this, ResId(        LB_CALCGRID, *rResId.GetResMgr())),
        aCalcGridWN(this, ResId(        WN_CALCGRID, *rResId.GetResMgr())),
        aCalcPageBreakFT(this, ResId(   FT_CALCPAGEBREAK, *rResId.GetResMgr())),
        aCalcPageBreakLB(this, ResId(   LB_CALCPAGEBREAK, *rResId.GetResMgr())),
        aCalcPageBreakWN(this, ResId(   WN_CALCPAGEBREAK, *rResId.GetResMgr())),
        aCalcPageBreakManualFT(this, ResId(   FT_CALCPAGEBREAKMANUAL, *rResId.GetResMgr())),
        aCalcPageBreakManualLB(this, ResId(   LB_CALCPAGEBREAKMANUAL, *rResId.GetResMgr())),
        aCalcPageBreakManualWN(this, ResId(   WN_CALCPAGEBREAKMANUAL, *rResId.GetResMgr())),
        aCalcPageBreakAutoFT(this, ResId(   FT_CALCPAGEBREAKAUTO, *rResId.GetResMgr())),
        aCalcPageBreakAutoLB(this, ResId(   LB_CALCPAGEBREAKAUTO, *rResId.GetResMgr())),
        aCalcPageBreakAutoWN(this, ResId(   WN_CALCPAGEBREAKAUTO, *rResId.GetResMgr())),
        aCalcDetectiveFT(this, ResId(   FT_CALCDETECTIVE, *rResId.GetResMgr())),
        aCalcDetectiveLB(this, ResId(   LB_CALCDETECTIVE, *rResId.GetResMgr())),
        aCalcDetectiveWN(this, ResId(   WN_CALCDETECTIVE, *rResId.GetResMgr())),
        aCalcDetectiveErrorFT(this, ResId(   FT_CALCDETECTIVEERROR, *rResId.GetResMgr())),
        aCalcDetectiveErrorLB(this, ResId(   LB_CALCDETECTIVEERROR, *rResId.GetResMgr())),
        aCalcDetectiveErrorWN(this, ResId(   WN_CALCDETECTIVEERROR, *rResId.GetResMgr())),
        aCalcReferenceFT(this, ResId(   FT_CALCREFERENCE, *rResId.GetResMgr())),
        aCalcReferenceLB(this, ResId(   LB_CALCREFERENCE, *rResId.GetResMgr())),
        aCalcReferenceWN(this, ResId(   WN_CALCREFERENCE, *rResId.GetResMgr())),
        aCalcNotesBackFT(this, ResId(   FT_CALCNOTESBACKGROUND, *rResId.GetResMgr())),
        aCalcNotesBackLB(this, ResId(   LB_CALCNOTESBACKGROUND, *rResId.GetResMgr())),
        aCalcNotesBackWN(this, ResId(   WN_CALCNOTESBACKGROUND, *rResId.GetResMgr())),
        aDrawBackWN(this),
        aDrawFT(this, ResId(            FT_DRAW, *rResId.GetResMgr())),
        aDrawGridFT(this, ResId(        FT_DRAWGRID, *rResId.GetResMgr())),
        aDrawGridLB(this, ResId(        LB_DRAWGRID, *rResId.GetResMgr())),
        aDrawGridWN(this, ResId(        WN_DRAWGRID, *rResId.GetResMgr())),
        aBasicBackWN(this),
        aBasicFT(this, ResId(            FT_BASIC, *rResId.GetResMgr())),
        aBasicIdentifierFT(this, ResId( FT_BASICIDENTIFIER, *rResId.GetResMgr())),
        aBasicIdentifierLB(this, ResId( LB_BASICIDENTIFIER, *rResId.GetResMgr())),
        aBasicIdentifierWN(this, ResId( WN_BASICIDENTIFIER, *rResId.GetResMgr())),
        aBasicCommentFT(this, ResId( FT_BASICCOMMENT, *rResId.GetResMgr())),
        aBasicCommentLB(this, ResId( LB_BASICCOMMENT, *rResId.GetResMgr())),
        aBasicCommentWN(this, ResId( WN_BASICCOMMENT, *rResId.GetResMgr())),
        aBasicNumberFT(this, ResId( FT_BASICNUMBER, *rResId.GetResMgr())),
        aBasicNumberLB(this, ResId( LB_BASICNUMBER, *rResId.GetResMgr())),
        aBasicNumberWN(this, ResId( WN_BASICNUMBER, *rResId.GetResMgr())),
        aBasicStringFT(this, ResId( FT_BASICSTRING, *rResId.GetResMgr())),
        aBasicStringLB(this, ResId( LB_BASICSTRING, *rResId.GetResMgr())),
        aBasicStringWN(this, ResId( WN_BASICSTRING, *rResId.GetResMgr())),
        aBasicOperatorFT(this, ResId( FT_BASICOPERATOR, *rResId.GetResMgr())),
        aBasicOperatorLB(this, ResId( LB_BASICOPERATOR, *rResId.GetResMgr())),
        aBasicOperatorWN(this, ResId( WN_BASICOPERATOR, *rResId.GetResMgr())),
        aBasicKeywordFT(this, ResId( FT_BASICKEYWORD, *rResId.GetResMgr())),
        aBasicKeywordLB(this, ResId( LB_BASICKEYWORD, *rResId.GetResMgr())),
        aBasicKeywordWN(this, ResId( WN_BASICKEYWORD, *rResId.GetResMgr())),
        aBasicErrorFT(this, ResId( FT_BASICERROR, *rResId.GetResMgr())),
        aBasicErrorLB(this, ResId( LB_BASICERROR, *rResId.GetResMgr())),
        aBasicErrorWN(this, ResId( WN_BASICERROR, *rResId.GetResMgr())),

        aSQLBackWN(this),
        aSQLFT(this, ResId(            FT_SQL_COMMAND, *rResId.GetResMgr())),
        aSQLIdentifierFT(this, ResId( FT_SQLIDENTIFIER, *rResId.GetResMgr())),
        aSQLIdentifierLB(this, ResId( LB_SQLIDENTIFIER, *rResId.GetResMgr())),
        aSQLIdentifierWN(this, ResId( WN_SQLIDENTIFIER, *rResId.GetResMgr())),

        aSQLNumberFT(this, ResId( FT_SQLNUMBER, *rResId.GetResMgr())),
        aSQLNumberLB(this, ResId( LB_SQLNUMBER, *rResId.GetResMgr())),
        aSQLNumberWN(this, ResId( WN_SQLNUMBER, *rResId.GetResMgr())),

        aSQLStringFT(this, ResId( FT_SQLSTRING, *rResId.GetResMgr())),
        aSQLStringLB(this, ResId( LB_SQLSTRING, *rResId.GetResMgr())),
        aSQLStringWN(this, ResId( WN_SQLSTRING, *rResId.GetResMgr())),

        aSQLOperatorFT(this, ResId( FT_SQLOPERATOR, *rResId.GetResMgr())),
        aSQLOperatorLB(this, ResId( LB_SQLOPERATOR, *rResId.GetResMgr())),
        aSQLOperatorWN(this, ResId( WN_SQLOPERATOR, *rResId.GetResMgr())),

        aSQLKeywordFT(this, ResId( FT_SQLKEYWORD, *rResId.GetResMgr())),
        aSQLKeywordLB(this, ResId( LB_SQLKEYWORD, *rResId.GetResMgr())),
        aSQLKeywordWN(this, ResId( WN_SQLKEYWORD, *rResId.GetResMgr())),

        aSQLParameterFT(this, ResId( FT_SQLPARAMETER, *rResId.GetResMgr())),
        aSQLParameterLB(this, ResId( LB_SQLPARAMETER, *rResId.GetResMgr())),
        aSQLParameterWN(this, ResId( WN_SQLPARAMETER, *rResId.GetResMgr())),

        aSQLCommentFT(this, ResId( FT_SQLCOMMENT, *rResId.GetResMgr())),
        aSQLCommentLB(this, ResId( LB_SQLCOMMENT, *rResId.GetResMgr())),
        aSQLCommentWN(this, ResId( WN_SQLCOMMENT, *rResId.GetResMgr()))
{
    aFixedTexts.resize(ColorConfigEntryCount);
    aCheckBoxes.resize(ColorConfigEntryCount);
    aColorBoxes.resize(ColorConfigEntryCount);
    aWindows.resize(ColorConfigEntryCount);

    aFixedTexts[DOCCOLOR         ] = &aDocColorFT;
    aCheckBoxes[DOCBOUNDARIES       ] = &aDocBoundCB             ;
    aFixedTexts[APPBACKGROUND    ] = &aAppBackFT;
    aCheckBoxes[OBJECTBOUNDARIES    ] = &aObjBoundCB             ;
    aCheckBoxes[TABLEBOUNDARIES     ] = &aTableBoundCB           ;
    aFixedTexts[FONTCOLOR           ] = &aFontColorFT            ;
    aCheckBoxes[LINKS               ] = &aLinksCB                ;
    aCheckBoxes[LINKSVISITED        ] = &aLinksVisitedCB         ;
    aFixedTexts[SPELL            ]=& aSpellFT;
    aFixedTexts[SMARTTAGS        ]=& aSmarttagsFT;
    aFixedTexts[WRITERTEXTGRID   ]=& aWrtTextGridFT;
    aCheckBoxes[WRITERFIELDSHADINGS ] = &aWrtFieldCB             ;
    aCheckBoxes[WRITERIDXSHADINGS   ] = &aWrtIdxShadingBackCB       ;
    aFixedTexts[WRITERDIRECTCURSOR  ]=& aWrtDirectCrsrFT;
    aFixedTexts[WRITERSCRIPTINDICATOR   ]=& aWrtScriptIndicatorFT;
    aCheckBoxes[WRITERSECTIONBOUNDARIES ]=& aWrtSectionBoundCB;
    aFixedTexts[HTMLSGML         ]=& aHTMLSGMLFT;
    aFixedTexts[HTMLCOMMENT      ]=& aHTMLCommentFT;
    aFixedTexts[HTMLKEYWORD      ]=& aHTMLKeywdFT;
    aFixedTexts[HTMLUNKNOWN      ]=& aHTMLUnknownFT;
    aFixedTexts[CALCGRID            ] = &aCalcGridFT             ;
    aFixedTexts[CALCPAGEBREAK    ] = &aCalcPageBreakFT;
    aFixedTexts[CALCPAGEBREAKMANUAL    ] = &aCalcPageBreakManualFT;
    aFixedTexts[CALCPAGEBREAKAUTOMATIC ] = &aCalcPageBreakAutoFT;
    aFixedTexts[CALCDETECTIVE    ]=& aCalcDetectiveFT;
    aFixedTexts[CALCDETECTIVEERROR    ]=& aCalcDetectiveErrorFT;
    aFixedTexts[CALCREFERENCE    ]=& aCalcReferenceFT;
    aFixedTexts[CALCNOTESBACKGROUND  ]=& aCalcNotesBackFT;
    aFixedTexts[WRITERPAGEBREAKS] = &aWrtPageBreaksFT;
    aFixedTexts[DRAWGRID            ] = &aDrawGridFT             ;
    aFixedTexts[BASICIDENTIFIER ] = &aBasicIdentifierFT;
    aFixedTexts[BASICCOMMENT    ] = &aBasicCommentFT;
    aFixedTexts[BASICNUMBER     ] = &aBasicNumberFT;
    aFixedTexts[BASICSTRING     ] = &aBasicStringFT;
    aFixedTexts[BASICOPERATOR   ] = &aBasicOperatorFT;
    aFixedTexts[BASICKEYWORD    ] = &aBasicKeywordFT;
    aFixedTexts[BASICERROR      ] = &aBasicErrorFT;
    aFixedTexts[SQLIDENTIFIER   ] = &aSQLIdentifierFT;
    aFixedTexts[SQLNUMBER       ] = &aSQLNumberFT;
    aFixedTexts[SQLSTRING       ] = &aSQLStringFT;
    aFixedTexts[SQLOPERATOR     ] = &aSQLOperatorFT;
    aFixedTexts[SQLKEYWORD      ] = &aSQLKeywordFT;
    aFixedTexts[SQLPARAMETER    ] = &aSQLParameterFT;
    aFixedTexts[SQLCOMMENT      ] = &aSQLCommentFT;

    aColorBoxes[DOCCOLOR            ] = &aDocColorLB             ;
    aColorBoxes[DOCBOUNDARIES       ] = &aDocBoundLB             ;
    aColorBoxes[APPBACKGROUND       ] = &aAppBackLB              ;
    aColorBoxes[OBJECTBOUNDARIES    ] = &aObjBoundLB             ;
    aColorBoxes[TABLEBOUNDARIES     ] = &aTableBoundLB           ;
    aColorBoxes[FONTCOLOR           ] = &aFontColorLB            ;
    aColorBoxes[LINKS               ] = &aLinksLB                ;
    aColorBoxes[LINKSVISITED        ] = &aLinksVisitedLB         ;
    aColorBoxes[SPELL               ] = &aSpellLB             ;
    aColorBoxes[SMARTTAGS           ] = &aSmarttagsLB             ;
    aColorBoxes[WRITERTEXTGRID      ] = &aWrtTextGridLB          ;
    aColorBoxes[WRITERFIELDSHADINGS ] = &aWrtFieldLB             ;
    aColorBoxes[WRITERIDXSHADINGS   ] = &aWrtIdxShadingBackLB       ;
    aColorBoxes[WRITERDIRECTCURSOR  ] = &aWrtDirectCrsrLB           ;
    aColorBoxes[WRITERSCRIPTINDICATOR    ] = &aWrtScriptIndicatorLB           ;
    aColorBoxes[WRITERSECTIONBOUNDARIES  ] = &aWrtSectionBoundLB           ;
    aColorBoxes[WRITERPAGEBREAKS] = &aWrtPageBreaksLB;
    aColorBoxes[HTMLSGML            ] = &aHTMLSGMLLB             ;
    aColorBoxes[HTMLCOMMENT         ] = &aHTMLCommentLB          ;
    aColorBoxes[HTMLKEYWORD         ] = &aHTMLKeywdLB            ;
    aColorBoxes[HTMLUNKNOWN         ] = &aHTMLUnknownLB          ;
    aColorBoxes[CALCGRID            ] = &aCalcGridLB             ;
    aColorBoxes[CALCPAGEBREAK       ] = &aCalcPageBreakLB       ;
    aColorBoxes[CALCPAGEBREAKMANUAL ] = &aCalcPageBreakManualLB       ;
    aColorBoxes[CALCPAGEBREAKAUTOMATIC]= &aCalcPageBreakAutoLB       ;
    aColorBoxes[CALCDETECTIVE       ] = &aCalcDetectiveLB        ;
    aColorBoxes[CALCDETECTIVEERROR  ] = &aCalcDetectiveErrorLB        ;
    aColorBoxes[CALCREFERENCE       ] = &aCalcReferenceLB        ;
    aColorBoxes[CALCNOTESBACKGROUND     ] = &aCalcNotesBackLB            ;
    aColorBoxes[DRAWGRID            ] = &aDrawGridLB             ;
    aColorBoxes[BASICIDENTIFIER     ] = &aBasicIdentifierLB;
    aColorBoxes[BASICCOMMENT        ] = &aBasicCommentLB;
    aColorBoxes[BASICNUMBER         ] = &aBasicNumberLB;
    aColorBoxes[BASICSTRING         ] = &aBasicStringLB;
    aColorBoxes[BASICOPERATOR       ] = &aBasicOperatorLB;
    aColorBoxes[BASICKEYWORD        ] = &aBasicKeywordLB;
    aColorBoxes[BASICERROR          ] = &aBasicErrorLB;
    aColorBoxes[SQLIDENTIFIER       ] = &aSQLIdentifierLB;
    aColorBoxes[SQLNUMBER           ] = &aSQLNumberLB;
    aColorBoxes[SQLSTRING           ] = &aSQLStringLB;
    aColorBoxes[SQLOPERATOR         ] = &aSQLOperatorLB;
    aColorBoxes[SQLKEYWORD          ] = &aSQLKeywordLB;
    aColorBoxes[SQLPARAMETER        ] = &aSQLParameterLB;
    aColorBoxes[SQLCOMMENT          ] = &aSQLCommentLB;

    aWindows[DOCCOLOR            ] = &aDocColorWN             ;
    aWindows[DOCBOUNDARIES       ] = &aDocBoundWN             ;
    aWindows[APPBACKGROUND       ] = &aAppBackWN              ;
    aWindows[OBJECTBOUNDARIES    ] = &aObjBoundWN             ;
    aWindows[TABLEBOUNDARIES     ] = &aTableBoundWN           ;
    aWindows[FONTCOLOR           ] = &aFontColorWN            ;
    aWindows[LINKS               ] = &aLinksWN                ;
    aWindows[LINKSVISITED        ] = &aLinksVisitedWN         ;
    aWindows[SPELL               ] = &aSpellWN             ;
    aWindows[SMARTTAGS           ] = &aSmarttagsWN             ;
    aWindows[WRITERTEXTGRID      ] = &aWrtTextGridWN          ;
    aWindows[WRITERFIELDSHADINGS ] = &aWrtFieldWN             ;
    aWindows[WRITERIDXSHADINGS   ] = &aWrtIdxShadingBackWN       ;
    aWindows[WRITERDIRECTCURSOR  ] = &aWrtDirectCrsrWN           ;
    aWindows[WRITERSCRIPTINDICATOR    ] = &aWrtScriptIndicatorWN           ;
    aWindows[WRITERSECTIONBOUNDARIES  ] = &aWrtSectionBoundWN           ;
    aWindows[WRITERPAGEBREAKS] = &aWrtPageBreaksWN;
    aWindows[HTMLSGML            ] = &aHTMLSGMLWN             ;
    aWindows[HTMLCOMMENT         ] = &aHTMLCommentWN          ;
    aWindows[HTMLKEYWORD         ] = &aHTMLKeywdWN            ;
    aWindows[HTMLUNKNOWN         ] = &aHTMLUnknownWN          ;
    aWindows[CALCGRID            ] = &aCalcGridWN             ;
    aWindows[CALCPAGEBREAK       ] = &aCalcPageBreakWN        ;
    aWindows[CALCPAGEBREAKMANUAL ] = &aCalcPageBreakManualWN        ;
    aWindows[CALCPAGEBREAKAUTOMATIC] = &aCalcPageBreakAutoWN        ;
    aWindows[CALCDETECTIVE       ] = &aCalcDetectiveWN        ;
    aWindows[CALCDETECTIVEERROR  ] = &aCalcDetectiveErrorWN        ;
    aWindows[CALCREFERENCE       ] = &aCalcReferenceWN        ;
    aWindows[CALCNOTESBACKGROUND ] = &aCalcNotesBackWN            ;
    aWindows[DRAWGRID            ] = &aDrawGridWN             ;
    aWindows[BASICIDENTIFIER     ] = &aBasicIdentifierWN;
    aWindows[BASICCOMMENT        ] = &aBasicCommentWN;
    aWindows[BASICNUMBER         ] = &aBasicNumberWN;
    aWindows[BASICSTRING         ] = &aBasicStringWN;
    aWindows[BASICOPERATOR       ] = &aBasicOperatorWN;
    aWindows[BASICKEYWORD        ] = &aBasicKeywordWN;
    aWindows[BASICERROR          ] = &aBasicErrorWN;
    aWindows[SQLIDENTIFIER       ] = &aSQLIdentifierWN;
    aWindows[SQLNUMBER           ] = &aSQLNumberWN;
    aWindows[SQLSTRING           ] = &aSQLStringWN;
    aWindows[SQLOPERATOR         ] = &aSQLOperatorWN;
    aWindows[SQLKEYWORD          ] = &aSQLKeywordWN;
    aWindows[SQLPARAMETER        ] = &aSQLParameterWN;
    aWindows[SQLCOMMENT          ] = &aSQLCommentWN;

    aChapters.push_back(&aGeneralFT); aChapterWins.push_back(&aGeneralBackWN);
    aChapters.push_back(&aWriterFT);  aChapterWins.push_back(&aWriterBackWN);
    aChapters.push_back(&aHTMLFT);    aChapterWins.push_back(&aHTMLBackWN);
    aChapters.push_back(&aCalcFT);    aChapterWins.push_back(&aCalcBackWN);
    aChapters.push_back(&aDrawFT);    aChapterWins.push_back(&aDrawBackWN);
    aChapters.push_back(&aBasicFT);   aChapterWins.push_back(&aBasicBackWN);
    aChapters.push_back(&aSQLFT);   aChapterWins.push_back(&aSQLBackWN);

    // calculate heights of groups which can be hidden
    aChapters[GROUP_WRITER  ]->SetGroupHeight( aChapters[GROUP_HTML]->GetPosPixel().Y() -  aChapters[GROUP_WRITER]->GetPosPixel().Y() );
    aChapters[GROUP_HTML    ]->SetGroupHeight( aChapters[GROUP_CALC]->GetPosPixel().Y() -  aChapters[GROUP_HTML]->GetPosPixel().Y() );
    aChapters[GROUP_CALC    ]->SetGroupHeight( aChapters[GROUP_DRAW]->GetPosPixel().Y() -  aChapters[GROUP_CALC]->GetPosPixel().Y() );
    aChapters[GROUP_DRAW    ]->SetGroupHeight( aChapters[GROUP_BASIC]->GetPosPixel().Y() - aChapters[GROUP_DRAW]->GetPosPixel().Y() );
    aChapters[GROUP_BASIC   ]->SetGroupHeight( aChapters[GROUP_SQL]->GetPosPixel().Y() - aChapters[GROUP_BASIC]->GetPosPixel().Y() );

    ExtendedColorConfig aExtConfig;
    sal_Int32 nExtCount = aExtConfig.GetComponentCount();
    if ( nExtCount )
    {
        // calculate position behind last chapter
        sal_Int32 nLastY = aSQLCommentWN.GetPosPixel().Y() + aSQLCommentWN.GetSizePixel().Height();
        nLastY = nLastY + LogicToPixel( Size( 0, 3 ), MAP_APPFONT ).Height();
        // to calculate the number of lines
        sal_Int32 nHeight = LogicToPixel( Size( 0, _LINE_HEIGHT ), MAP_APPFONT ).Height();
        sal_Int32 nLineNum = nLastY / nHeight;

        Point aFixedPos = LogicToPixel( Point( _FT_XPOS, nLineNum * _LINE_HEIGHT ), MAP_APPFONT );
        Point aLBPos = LogicToPixel( Point( _LB_XPOS, nLineNum * _LINE_HEIGHT ), MAP_APPFONT );
        Size aFixedSize = LogicToPixel( Size( _FT_WIDTH , _FT_HEIGHT ), MAP_APPFONT );
        Size aLBSize = LogicToPixel( Size( _LB_WIDTH , _LB_HEIGHT ), MAP_APPFONT );
        Size aWinSize = LogicToPixel( Size( _WN_WIDTH , _WN_HEIGHT ), MAP_APPFONT );

        for (sal_Int32 j = 0; j < nExtCount; ++j)
        {
            ::rtl::OUString sComponentName = aExtConfig.GetComponentName(j);
            aChapterWins.push_back(new Window(this));
            ::boost::shared_ptr<SvxExtFixedText_Impl> pTitle(new SvxExtFixedText_Impl(this,ResId(FT_SQL_COMMAND, *rResId.GetResMgr())));
            m_aExtensionTitles.push_back(pTitle);
            pTitle->SetPosSizePixel(LogicToPixel( Point( _FT_XPOS, nLineNum * _LINE_HEIGHT ), MAP_APPFONT ),aFixedSize);
            pTitle->SetText(aExtConfig.GetComponentDisplayName(sComponentName));
            aChapters.push_back(pTitle.get());
            ++nLineNum;
            sal_Int32 nColorCount = aExtConfig.GetComponentColorCount(sComponentName);
            for (sal_Int32 i = 0; i < nColorCount; ++i,++nLineNum)
            {
                ExtendedColorConfigValue aColorEntry = aExtConfig.GetComponentColorConfigValue(sComponentName,i);
                FixedText* pFixedText = new FixedText(this,ResId(FT_BASICERROR, *rResId.GetResMgr()));
                pFixedText->SetPosSizePixel(LogicToPixel( Point( _FT_XPOS, nLineNum * _LINE_HEIGHT ), MAP_APPFONT ),aFixedSize);
                pFixedText->SetText(aColorEntry.getDisplayName());
                aFixedTexts.push_back(pFixedText);
                aCheckBoxes.push_back(NULL); // no checkboxes
                ColorListBox* pColorBox = new ColorListBox(this,ResId(LB_BASICERROR, *rResId.GetResMgr()));
                pColorBox->SetPosSizePixel(LogicToPixel( Point( _LB_XPOS, nLineNum * _LINE_HEIGHT ), MAP_APPFONT ),aLBSize);
                aColorBoxes.push_back(pColorBox);
                Window* pWin = new Window(this,ResId(WN_BASICERROR, *rResId.GetResMgr()));
                pWin->SetPosSizePixel(LogicToPixel( Point( _WN_XPOS, nLineNum * _LINE_HEIGHT ), MAP_APPFONT ),aWinSize);
                aWindows.push_back(pWin);
            } // for (sal_Int32 i = 0; i < nExtCount; ++i,++nLineNum)
        }
    }

    FreeResource();

    Color TempColor(COL_TRANSPARENT);
    Wallpaper aTransparentWall(TempColor);
    sal_Int32 nWinWidth = GetSizePixel().Width();
    sal_Int32 nFTHeight = aChapters[0]->GetSizePixel().Height();
    Color aBackColor;
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    sal_Bool bHighContrast = rStyleSettings.GetHighContrastMode();
    if ( bHighContrast )
        aBackColor = rStyleSettings.GetShadowColor();
    else
        aBackColor = Color( COL_LIGHTGRAY);
    sal_Int32 nCount = aChapterWins.size();
    for(sal_Int32 i = 0; i < nCount; ++i)
    {
        if ( lcl_isGroupVisible( i, m_aModuleOptions ) )
        {
            Font aFont = aChapters[i]->GetFont();
            aFont.SetWeight(WEIGHT_BOLD);
            aChapters[i]->SetFont(aFont);
            aChapters[i]->SetBackground(aTransparentWall);
            aChapterWins[i]->SetPosSizePixel( Point(0, aChapters[i]->GetPosPixel().Y()),
                                Size(nWinWidth, nFTHeight));
            aChapterWins[i]->SetBackground(Wallpaper(aBackColor));
            aChapterWins[i]->Show();
        }
        else
        {
            aChapters[i]->Hide();
            aChapterWins[i]->Hide();
        }
    }
    Color aTextColor;
    sal_Bool bSetTextColor = sal_False;
    //#104195# when the window color is the same as the text color it has to be changed
    Color aWinCol = rStyleSettings.GetWindowColor();
    Color aRCheckCol = rStyleSettings.GetRadioCheckTextColor();
    if(aWinCol == aRCheckCol )
    {
        bSetTextColor = sal_True;
        aRCheckCol.Invert();
        //if inversion didn't work (gray) then it's set to black
        if(aRCheckCol == aWinCol)
            aRCheckCol = Color(COL_BLACK);
    } // if(aWinCol == aRCheckCol )
    nCount = aFixedTexts.size();
    sal_Int16 nGroup = GROUP_UNKNOWN;
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        sal_Int16 nNewGroup = lcl_getGroup(i);
        sal_Bool bShow = lcl_isGroupVisible( nNewGroup, m_aModuleOptions );

        if ( nNewGroup > nGroup )
        {
            nGroup = nNewGroup;
            if ( bShow && nGroup >= GROUP_CALC )
            {
                SetNewPosition( i, aChapters[nGroup] );
                SetNewPosition( i, aChapterWins[nGroup] );
            }
        }
        if(aCheckBoxes[i])
        {
            if ( bShow )
                SetNewPosition( i, aCheckBoxes[i] );
            else
                aCheckBoxes[i]->Hide();
            aCheckBoxes[i]->SetBackground(aTransparentWall);
            aCheckBoxes[i]->SetHelpId( aColorCBHids[i] );
            if(bSetTextColor)
                aCheckBoxes[i]->SetTextColor(aRCheckCol);
        }
        if(aFixedTexts[i])
        {
            if ( bShow )
                SetNewPosition( i, aFixedTexts[i] );
            else
                aFixedTexts[i]->Hide();
            aFixedTexts[i]->SetBackground(aTransparentWall);
            if(bSetTextColor)
                aFixedTexts[i]->SetTextColor(aRCheckCol);
        }
        if(aWindows[i])
        {
            if ( bShow )
                SetNewPosition( i, aWindows[i] );
            else
                aWindows[i]->Hide();
            aWindows[i]->SetBorderStyle(WINDOW_BORDER_MONO);
        }

        if ( aColorBoxes[i] )
        {
            if ( bShow )
                SetNewPosition( i, aColorBoxes[i] );
            else
                aColorBoxes[i]->Hide();
        }
    }

    XColorTable aColorTable( SvtPathOptions().GetPalettePath() );
    aColorBoxes[0]->InsertAutomaticEntry();
    for( sal_Int32 i = 0; i < aColorTable.Count(); i++ )
    {
        XColorEntry* pEntry = aColorTable.GetColor(i);
        aColorBoxes[0]->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    aColorBoxes[0]->SetHelpId( aColorLBHids[0] );

    OSL_ENSURE( nCount < sal_Int32(sizeof(aColorLBHids)/sizeof(aColorLBHids[0])), "too few helpIDs for color listboxes" );
    for( sal_Int32 i = 1; i < nCount; i++ )
    {
        if(aColorBoxes[i])
        {
            aColorBoxes[i]->CopyEntries( *aColorBoxes[0] );
            if( i < sal_Int32(sizeof(aColorLBHids)/sizeof(aColorLBHids[0])) )
               aColorBoxes[i]->SetHelpId( aColorLBHids[i] );
        }
    }
}
/* -----------------------------27.03.2002 11:04------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigWindow_Impl::~ColorConfigWindow_Impl()
{
    aChapters.clear();
    ::std::vector< SvxExtFixedText_Impl*>().swap(aChapters);


    sal_Int32 nCount = aFixedTexts.size();
    for (sal_Int32 i = ColorConfigEntryCount; i < nCount; ++i)
    {
        delete aFixedTexts[i];
        delete aCheckBoxes[i];
        delete aColorBoxes[i];
        delete aWindows[i];
    }
    for (sal_uInt32 i = GROUP_COUNT; i < aChapterWins.size(); ++i)
        delete aChapterWins[i];
    aChapterWins.clear();
    ::std::vector< Window*>().swap(aChapterWins);
}
/* -----------------------------2002/06/20 12:48------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfigWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        sal_Bool bHighContrast = rStyleSettings.GetHighContrastMode();
        Color aBackColor( bHighContrast ? COL_TRANSPARENT : COL_LIGHTGRAY);
        ::std::vector< Window* >::iterator aIter = aChapterWins.begin();
        ::std::vector< Window* >::iterator aEnd  = aChapterWins.end();
        for(;aIter != aEnd; ++aIter )
            (*aIter)->SetBackground(Wallpaper(aBackColor));
        SetBackground(Wallpaper(rStyleSettings.GetWindowColor()));
    }
}
/* -----------------------------2002/06/26 10:49------------------------------

 ---------------------------------------------------------------------------*/
void SvxExtFixedText_Impl::DataChanged(const DataChangedEvent& rDCEvt)
{
    FixedText::DataChanged(rDCEvt);
    Color TempColor(COL_TRANSPARENT);
    Wallpaper aTransparentWall(TempColor);
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Font aFont = GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        SetFont(aFont);
        SetBackground(aTransparentWall);
    }
}

void ColorConfigWindow_Impl::SetNewPosition( sal_Int32 _nFeature, Window* _pWin )
{
    DBG_ASSERT( _pWin, "ColorConfigWindow_Impl::SetNewPosition(): no window" );

    // calculate the overall height of the invisible groups
    long nDelta = 0;
    sal_Int16 nGroup = lcl_getGroup( _nFeature ) - 1;
    while ( nGroup > 0 ) // GROUP_GENERAL (0) is always visible
    {
        if ( !lcl_isGroupVisible( nGroup, m_aModuleOptions ) )
            nDelta += aChapters[nGroup]->GetGroupHeight();
        nGroup--;
    }

    // move this window to its new position
    if ( nDelta > 0 )
    {
        Point aPos = _pWin->GetPosPixel();
        aPos.Y() -= nDelta;
        _pWin->SetPosPixel( aPos );
    }
}

/* -----------------------------08.04.2002 17:10------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfigWindow_Impl::Command( const CommandEvent& rCEvt )
{
    GetParent()->Command(rCEvt);
}
/* -----------------------------25.03.2002 10:44------------------------------

 ---------------------------------------------------------------------------*/
class ColorConfigCtrl_Impl : public Control
{
    HeaderBar               aHeaderHB;
    ScrollBar               aVScroll;

    String                  sOn;
    String                  sUIElem;
    String                  sColSetting;
    String                  sPreview;
    ColorConfigWindow_Impl  aScrollWindow;

    EditableColorConfig*            pColorConfig;
    EditableExtendedColorConfig*    pExtColorConfig;

    long            nScrollPos;

    DECL_LINK(ScrollHdl, ScrollBar*);
    DECL_LINK(ClickHdl, CheckBox*);
    DECL_LINK(ColorHdl, ColorListBox*);
    DECL_LINK(ControlFocusHdl, Control*);

    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
public:
    ColorConfigCtrl_Impl(Window* pParent, const ResId& rResId );
    ~ColorConfigCtrl_Impl();

    void SetConfig(EditableColorConfig& rConfig) {pColorConfig = &rConfig;}
    void SetExtendedConfig(EditableExtendedColorConfig& rConfig) {pExtColorConfig = &rConfig;}
    void Update();
    sal_Int32   GetScrollPosition() {return aVScroll.GetThumbPos();}
    void        SetScrollPosition(sal_Int32 nSet)
                    {
                        aVScroll.SetThumbPos(nSet);
                        ScrollHdl(&aVScroll);
                    }
};
/* -----------------------------25.03.2002 17:09------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigCtrl_Impl::ColorConfigCtrl_Impl(
        Window* pParent, const ResId& rResId) :
        Control(pParent, rResId),


        aHeaderHB(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
        aVScroll(this,      ResId(VB_VSCROLL, *rResId.GetResMgr())),
        sOn(                ResId(ST_ON, *rResId.GetResMgr())),
        sUIElem(            ResId(ST_UIELEM, *rResId.GetResMgr())),
        sColSetting(        ResId(ST_COLSET, *rResId.GetResMgr())),
        sPreview(           ResId(ST_PREVIEW, *rResId.GetResMgr())),
        aScrollWindow(this, ResId(WN_SCROLL, *rResId.GetResMgr())),

        pColorConfig(0),
        pExtColorConfig(0),
        nScrollPos(0)
{
    FreeResource();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    aScrollWindow.SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
    aScrollWindow.SetHelpId( HID_OPTIONS_COLORCONFIG_COLORLIST_WIN );
    aVScroll.EnableDrag();
    aVScroll.Show();
    aHeaderHB.SetPosSizePixel(
        Point(0,0),
        Size(GetOutputSizePixel().Width(), aVScroll.GetPosPixel().Y()));
    sal_Int32 nFirstWidth = aScrollWindow.aGeneralFT.GetPosPixel().X();
    sal_Int32 nSecondWidth = aScrollWindow.aColorBoxes[0]->GetPosPixel().X() - nFirstWidth;
    sal_Int32 nThirdWidth = aScrollWindow.aWindows[0]->GetPosPixel().X() - nFirstWidth - nSecondWidth;

    const WinBits nHeadBits = HIB_VCENTER | HIB_FIXED| HIB_FIXEDPOS;
    aHeaderHB.InsertItem( 1, sOn, nFirstWidth, (sal_uInt16)nHeadBits|HIB_CENTER);
    aHeaderHB.InsertItem( 2, sUIElem, nSecondWidth, (sal_uInt16)nHeadBits|HIB_LEFT);
    aHeaderHB.InsertItem( 3, sColSetting, nThirdWidth, (sal_uInt16)nHeadBits|HIB_LEFT);
    aHeaderHB.InsertItem( 4, sPreview,
            aHeaderHB.GetSizePixel().Width() - nFirstWidth - nSecondWidth - nThirdWidth, (sal_uInt16)nHeadBits|HIB_LEFT);
    aHeaderHB.Show();

    aVScroll.SetRangeMin(0);
    sal_Int32 nScrollOffset = aScrollWindow.aColorBoxes[1]->GetPosPixel().Y() - aScrollWindow.aColorBoxes[0]->GetPosPixel().Y();
    sal_Int32 nVisibleEntries = aScrollWindow.GetSizePixel().Height() / nScrollOffset;

    aVScroll.SetRangeMax(aScrollWindow.aCheckBoxes.size() + aScrollWindow.aChapters.size() );
    // static: minus three for ANCHOR, DRAWFILL and DRAWDRAWING
    aVScroll.SetRangeMax( aVScroll.GetRangeMax() - 3 );
    // dynamic: calculate the hidden lines
    long nInvisibleLines = 0;
    sal_Int16 nGroup = GROUP_UNKNOWN;
    sal_Int32 nCount = aScrollWindow.aCheckBoxes.size();
    for ( sal_Int32 i = 0; i < nCount; i++ )
    {
        if ( ANCHOR == i || DRAWFILL == i || DRAWDRAWING == i ) // not used at the moment
            continue;
        sal_Int16 nNewGroup = lcl_getGroup(i);
        sal_Bool bVisible = lcl_isGroupVisible( nNewGroup, aScrollWindow.GetModuleOptions() );
        if ( !bVisible )
            nInvisibleLines++;
        if ( nNewGroup > nGroup )
        {
            nGroup = nNewGroup;
            if ( !bVisible )
                nInvisibleLines++;
        }
    }
    aVScroll.SetRangeMax( aVScroll.GetRangeMax() - nInvisibleLines );

    aVScroll.SetPageSize( nVisibleEntries - 1 );
    aVScroll.SetVisibleSize(nVisibleEntries);
    Link aScrollLink = LINK(this, ColorConfigCtrl_Impl, ScrollHdl);
    aVScroll.SetScrollHdl(aScrollLink);
    aVScroll.SetEndScrollHdl(aScrollLink);

    Link aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    Link aGetFocusLink = LINK(this, ColorConfigCtrl_Impl, ControlFocusHdl);
    for( sal_Int16 i = 0; i < nCount; i++ )
    {
        if(aScrollWindow.aColorBoxes[i])
        {
            aScrollWindow.aColorBoxes[i]->SetSelectHdl(aColorLink);
            aScrollWindow.aColorBoxes[i]->SetGetFocusHdl(aGetFocusLink);
        }
        if(aScrollWindow.aCheckBoxes[i])
        {
            aScrollWindow.aCheckBoxes[i]->SetClickHdl(aCheckLink);
            aScrollWindow.aCheckBoxes[i]->SetGetFocusHdl(aGetFocusLink);
        }
    }
}
/* -----------------------------27.03.2002 10:46------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigCtrl_Impl::~ColorConfigCtrl_Impl()
{
}
/* -----------------------------25.03.2002 17:19------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfigCtrl_Impl::Update()
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );
    sal_Int32 i;
    for( i = 0; i < ColorConfigEntryCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        const ColorConfigValue& rColorEntry = pColorConfig->GetColorValue(ColorConfigEntry(i));
        if(COL_AUTO == (sal_uInt32)rColorEntry.nColor)
        {
            if(aScrollWindow.aColorBoxes[i])
                aScrollWindow.aColorBoxes[i]->SelectEntryPos(0);
            if(aScrollWindow.aWindows[i])
                aScrollWindow.aWindows[i]->SetBackground(
                    Wallpaper(ColorConfig::GetDefaultColor((ColorConfigEntry) i)));
        }
        else
        {
            Color aColor(rColorEntry.nColor);
            if(aScrollWindow.aColorBoxes[i])
                aScrollWindow.aColorBoxes[i]->SelectEntry( aColor );
            if(aScrollWindow.aWindows[i])
                aScrollWindow.aWindows[i]->SetBackground(Wallpaper(aColor));
        }
        if(aScrollWindow.aWindows[i])
            aScrollWindow.aWindows[i]->Invalidate();
        if(aScrollWindow.aCheckBoxes[i])
            aScrollWindow.aCheckBoxes[i]->Check(rColorEntry.bIsVisible);
    } // for( i = 0; i < ColorConfigEntryCount; i++ )


    sal_Int32 nExtCount = pExtColorConfig->GetComponentCount();
    sal_Int32 nCount = aScrollWindow.aCheckBoxes.size();
    sal_Int32 nPos = 0;
    i = ColorConfigEntryCount;
    for (sal_Int32 j = 0; j < nExtCount; ++j)
    {
        ::rtl::OUString sComponentName = pExtColorConfig->GetComponentName(j);
        sal_Int32 nColorCount = pExtColorConfig->GetComponentColorCount(sComponentName);

        for( sal_Int32 k = 0; i < nCount && k < nColorCount; ++i ,++nPos,++k)
        {
            if(aScrollWindow.aColorBoxes[i])
            {
                ExtendedColorConfigValue aColorEntry = pExtColorConfig->GetComponentColorConfigValue(sComponentName,k);
                Color aColor(aColorEntry.getColor());
                if(aColorEntry.getDefaultColor() == aColorEntry.getColor() )
                {
                    aScrollWindow.aColorBoxes[i]->SelectEntryPos(0);
                }
                else
                {
                    aScrollWindow.aColorBoxes[i]->SelectEntry( aColor );
                }
                if(aScrollWindow.aWindows[i])
                    aScrollWindow.aWindows[i]->SetBackground(Wallpaper(aColor));

                if(aScrollWindow.aWindows[i])
                    aScrollWindow.aWindows[i]->Invalidate();
            }
        }
    }
}
/* -----------------------------26.03.2002 12:55------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool lcl_MoveAndShow(Window* pWindow, long nOffset, long nMaxVisible, bool _bShow)
{
    sal_Bool bHide = sal_True;
    if(pWindow)
    {
        Point aPos = pWindow->GetPosPixel();
        aPos.Y() += nOffset;
        pWindow->SetPosPixel(aPos);
        if ( _bShow )
            bHide = aPos.Y() > nMaxVisible || (aPos.Y() + pWindow->GetSizePixel().Height()) < 0;
        pWindow->Show(!bHide);
    }
    return !bHide;
}
IMPL_LINK(ColorConfigCtrl_Impl, ScrollHdl, ScrollBar*, pScrollBar)
{
    aScrollWindow.SetUpdateMode(sal_True);
    sal_Int16 i;
    long nOffset = aScrollWindow.aColorBoxes[1]->GetPosPixel().Y() - aScrollWindow.aColorBoxes[0]->GetPosPixel().Y();
    nOffset *= (nScrollPos - pScrollBar->GetThumbPos());
    nScrollPos = pScrollBar->GetThumbPos();
    const long nWindowHeight = aScrollWindow.GetSizePixel().Height();
    sal_Int16 nFirstVisible = -1;
    sal_Int16 nLastVisible = -1;
    sal_Int32 nCount = aScrollWindow.aFixedTexts.size();
    for( i = 0; i < nCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        Point aPos;
        //controls outside of the view need to be hidden to speed up accessibility tools
        bool bShowCtrl = ( lcl_isGroupVisible(
            lcl_getGroup(i), aScrollWindow.GetModuleOptions() ) != sal_False );
        lcl_MoveAndShow(aScrollWindow.aCheckBoxes[i], nOffset, nWindowHeight, bShowCtrl);
        lcl_MoveAndShow(aScrollWindow.aFixedTexts[i], nOffset, nWindowHeight, bShowCtrl);
        lcl_MoveAndShow(aScrollWindow.aWindows[i]   , nOffset, nWindowHeight, bShowCtrl);
        sal_Bool bShow = lcl_MoveAndShow(aScrollWindow.aColorBoxes[i], nOffset, nWindowHeight, bShowCtrl);
        if(bShow)
        {
            if(nFirstVisible == -1)
                nFirstVisible = i;
            else
                nLastVisible = i;
        }
    }
    //show the one prior to the first visible and the first after the last visble control
    //to enable KEY_TAB travelling

    if(nFirstVisible)
    {
        //skip gaps where no controls exist for the related ColorConfigEntry
        do
            --nFirstVisible;
        while(!aScrollWindow.aCheckBoxes[nFirstVisible] && !aScrollWindow.aColorBoxes[nFirstVisible] && nFirstVisible >= 0);

        if ( lcl_isGroupVisible(
                lcl_getGroup( nFirstVisible ), aScrollWindow.GetModuleOptions() ) != sal_False )
        {
            if(aScrollWindow.aCheckBoxes[nFirstVisible])
                aScrollWindow.aCheckBoxes[nFirstVisible]->Show();
            if(aScrollWindow.aColorBoxes[nFirstVisible])
                aScrollWindow.aColorBoxes[nFirstVisible]->Show();
        }
    }

    if(nLastVisible < nCount - 1)
    {
        nLastVisible++;
        //skip gaps where no controls exist for the related ColorConfigEntry
        while(!aScrollWindow.aCheckBoxes[nLastVisible] && !aScrollWindow.aColorBoxes[nLastVisible] &&
                nLastVisible < nCount - 1 )
            nLastVisible++;
        if ( nLastVisible < nCount )
        {
            if ( lcl_isGroupVisible(
                    lcl_getGroup( nLastVisible ), aScrollWindow.GetModuleOptions() ) != sal_False )
            {
                if(aScrollWindow.aCheckBoxes[nLastVisible])
                    aScrollWindow.aCheckBoxes[nLastVisible]->Show();
                if(aScrollWindow.aColorBoxes[nLastVisible])
                    aScrollWindow.aColorBoxes[nLastVisible]->Show();
            }
        }
    } // if(nLastVisible < nCount - 1)
    sal_Int32 nChapterCount = aScrollWindow.aChapters.size();
    for( i = 0; i < nChapterCount; i++ )
    {
        Point aPos = aScrollWindow.aChapters[i]->GetPosPixel(); aPos.Y() += nOffset; aScrollWindow.aChapters[i]->SetPosPixel(aPos);
        aPos = aScrollWindow.aChapterWins[i]->GetPosPixel(); aPos.Y() += nOffset; aScrollWindow.aChapterWins[i]->SetPosPixel(aPos);
    }
    aScrollWindow.SetUpdateMode(sal_True);
    return 0;
}
/* -----------------------------29.04.2002 17:02------------------------------

 ---------------------------------------------------------------------------*/
long ColorConfigCtrl_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == EVENT_COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        sal_uInt16 nCmd = pCEvt->GetCommand();
        if( COMMAND_WHEEL == nCmd )
        {
            Command(*pCEvt);
            return 1;
        }
    }
    return Control::PreNotify(rNEvt);
}
/* -----------------------------08.04.2002 16:37------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfigCtrl_Impl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {

        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && COMMAND_WHEEL_ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand( rCEvt, 0, &aVScroll );
            }
        }
        break;
        default:
            Control::Command(rCEvt);
    }
}

/* -----------------------------14.12.2005 12:37------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfigCtrl_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
    }
}


/* -----------------------------27.03.2002 11:43------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(ColorConfigCtrl_Impl, ClickHdl, CheckBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );

    for( sal_Int32 i = 0; i < ColorConfigEntryCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        if(aScrollWindow.aCheckBoxes[i] == pBox )
        {
            ColorConfigValue aBoundCol = pColorConfig->GetColorValue(ColorConfigEntry(i));
            aBoundCol.bIsVisible = pBox->IsChecked();
            pColorConfig->SetColorValue(ColorConfigEntry(i), aBoundCol);
            break;
        }
    } // for( sal_Int32 i = 0; i < ColorConfigEntryCount; i++ )
    return 0;
}
/* -----------------------------27.03.2002 11:43------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(ColorConfigCtrl_Impl, ColorHdl, ColorListBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );
    sal_Int32 i = 0;
    for( ; i < ColorConfigEntryCount; i++ )
    {
        if(pBox && aScrollWindow.aColorBoxes[i] == pBox)
        {
            ColorConfigValue aColorEntry = pColorConfig->GetColorValue(ColorConfigEntry(i));
            if(!pBox->GetSelectEntryPos())
            {
                aColorEntry.nColor = COL_AUTO;
                if(aScrollWindow.aWindows[i])
                    aScrollWindow.aWindows[i]->SetBackground(
                        Wallpaper(ColorConfig::GetDefaultColor((ColorConfigEntry) i)));
            }
            else
            {
                Color aColor = pBox->GetSelectEntryColor();     // #i14869# no Color&, 'cause it's a ref to a temp object on the stack!
                aColorEntry.nColor = aColor.GetColor();
                if(aScrollWindow.aWindows[i])
                    aScrollWindow.aWindows[i]->SetBackground(Wallpaper(aColor));
            }
            if(aScrollWindow.aWindows[i])
                aScrollWindow.aWindows[i]->Invalidate();
            pColorConfig->SetColorValue(ColorConfigEntry(i), aColorEntry);
            break;
        }
    } // for( sal_Int32 i = 0; i < ColorConfigEntryCount; i++ )
    sal_Int32 nExtCount = pExtColorConfig->GetComponentCount();
    sal_Int32 nCount = aScrollWindow.aCheckBoxes.size();
    sal_Int32 nPos = 0;
    i = ColorConfigEntryCount;
    for (sal_Int32 j = 0; j < nExtCount; ++j)
    {
        ::rtl::OUString sComponentName = pExtColorConfig->GetComponentName(j);
        sal_Int32 nColorCount = pExtColorConfig->GetComponentColorCount(sComponentName);

        for( sal_Int32 k = 0; i < nCount && k < nColorCount; ++i ,++nPos,++k)
        {
            if(pBox && aScrollWindow.aColorBoxes[i] == pBox)
            {
                ExtendedColorConfigValue aColorEntry = pExtColorConfig->GetComponentColorConfigValue(sComponentName,k);
                Color aColor = pBox->GetSelectEntryColor();     // #i14869# no Color&, 'cause it's a ref to a temp object on the stack!
                aColorEntry.setColor(aColor.GetColor());
                if( !pBox->GetSelectEntryPos() ) // auto color
                {
                    aColorEntry.setColor(aColorEntry.getDefaultColor());
                    aColor.SetColor(aColorEntry.getColor());
                }
                if ( aScrollWindow.aWindows[i] )
                {
                    aScrollWindow.aWindows[i]->SetBackground(Wallpaper(aColor));
                    aScrollWindow.aWindows[i]->Invalidate();
                }
                pExtColorConfig->SetColorValue(sComponentName,aColorEntry);
                break;
            }
        }
    }
    return 0;
}
IMPL_LINK(ColorConfigCtrl_Impl, ControlFocusHdl, Control*, pCtrl)
{
    //determine whether a control is completely visible
    //and make it visible
    Point aCtrlPos(pCtrl->GetPosPixel());
    sal_Int32 nWinHeight = aScrollWindow.GetSizePixel().Height();
    if(0 != (GETFOCUS_TAB & pCtrl->GetGetFocusFlags() &&
        (aCtrlPos.Y() < 0 ||
            nWinHeight < aCtrlPos.Y() + aScrollWindow.aColorBoxes[0]->GetSizePixel().Height())))
    {
        long nThumbPos = aVScroll.GetThumbPos();
        if(nWinHeight < aCtrlPos.Y() + aScrollWindow.aColorBoxes[0]->GetSizePixel().Height())
        {
            //scroll down
            nThumbPos +=2;
        }
        else
        {
            //scroll up
            nThumbPos -= 2;
            if(nThumbPos < 0)
                nThumbPos = 0;
        }
        aVScroll.SetThumbPos(nThumbPos);
        ScrollHdl(&aVScroll);
    }
    return 0;
};
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
SvxColorOptionsTabPage::SvxColorOptionsTabPage(
    Window* pParent, const SfxItemSet& rCoreSet) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_COLORCONFIG ), rCoreSet ),
       aColorSchemeFL(  this, CUI_RES( FL_COLORSCHEME ) ),
       aColorSchemeFT(  this, CUI_RES( FT_COLORSCHEME ) ),
       aColorSchemeLB(  this, CUI_RES( LB_COLORSCHEME ) ),
       aSaveSchemePB(   this, CUI_RES( PB_SAVESCHEME) ),
       aDeleteSchemePB( this, CUI_RES( PB_DELETESCHEME ) ),
       aCustomColorsFL( this, CUI_RES( FL_CUSTOMCOLORS ) ),
       bFillItemSetCalled(sal_False),
       pColorConfig(0),
       pExtColorConfig(0),
       pColorConfigCT(  new ColorConfigCtrl_Impl(this, CUI_RES( CT_COLORCONFIG ) ))
{
    FreeResource();
    aColorSchemeLB.SetSelectHdl(LINK(this, SvxColorOptionsTabPage, SchemeChangedHdl_Impl));
    Link aLk = LINK(this, SvxColorOptionsTabPage, SaveDeleteHdl_Impl );
    aSaveSchemePB.SetClickHdl(aLk);
    aDeleteSchemePB.SetClickHdl(aLk);
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
SvxColorOptionsTabPage::~SvxColorOptionsTabPage()
{
    //when the dialog is cancelled but the color scheme ListBox has been changed these
    //changes need to be undone
    if(!bFillItemSetCalled && aColorSchemeLB.GetSavedValue() != aColorSchemeLB.GetSelectEntryPos())
    {
        rtl::OUString sOldScheme =  aColorSchemeLB.GetEntry(aColorSchemeLB.GetSavedValue());
        if(sOldScheme.getLength())
        {
            pColorConfig->SetCurrentSchemeName(sOldScheme);
            pExtColorConfig->SetCurrentSchemeName(sOldScheme);
        }
    }
    delete pColorConfigCT;
    pColorConfig->ClearModified();
    pColorConfig->EnableBroadcast();
    delete pColorConfig;
    pExtColorConfig->ClearModified();
    pExtColorConfig->EnableBroadcast();
    delete pExtColorConfig;
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
SfxTabPage* SvxColorOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxColorOptionsTabPage( pParent, rAttrSet ) );
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SvxColorOptionsTabPage::FillItemSet( SfxItemSet&  )
{
    bFillItemSetCalled = sal_True;
    if(aColorSchemeLB.GetSavedValue() != aColorSchemeLB.GetSelectEntryPos())
    {
        pColorConfig->SetModified();
        pExtColorConfig->SetModified();
    }
    if(pColorConfig->IsModified())
        pColorConfig->Commit();
    if(pExtColorConfig->IsModified())
        pExtColorConfig->Commit();
    return sal_True;
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
void SvxColorOptionsTabPage::Reset( const SfxItemSet& )
{
    if(pColorConfig)
    {
        pColorConfig->ClearModified();
        pColorConfig->DisableBroadcast();
        delete pColorConfig;
    }
    pColorConfig = new EditableColorConfig;
    pColorConfigCT->SetConfig(*pColorConfig);

    if(pExtColorConfig)
    {
        pExtColorConfig->ClearModified();
        pExtColorConfig->DisableBroadcast();
        delete pExtColorConfig;
    }
    pExtColorConfig = new EditableExtendedColorConfig;
    pColorConfigCT->SetExtendedConfig(*pExtColorConfig);

    String sUser = GetUserData();
    //has to be called always to speed up accessibility tools
    pColorConfigCT->SetScrollPosition(sUser.ToInt32());
    aColorSchemeLB.Clear();
    uno::Sequence< ::rtl::OUString >  aSchemes = pColorConfig->GetSchemeNames();
    const rtl::OUString* pSchemes = aSchemes.getConstArray();
    for(sal_Int32 i = 0; i < aSchemes.getLength(); i++)
        aColorSchemeLB.InsertEntry(pSchemes[i]);
    aColorSchemeLB.SelectEntry(pColorConfig->GetCurrentSchemeName());
    aColorSchemeLB.SaveValue();
    aDeleteSchemePB.Enable( aSchemes.getLength() > 1 );
    UpdateColorConfig();
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
int SvxColorOptionsTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return( LEAVE_PAGE );
}
/* -----------------------------25.03.2002 15:32------------------------------

 ---------------------------------------------------------------------------*/
void SvxColorOptionsTabPage::UpdateColorConfig()
{
    //update the color config control
    pColorConfigCT->Update();
}
/* -----------------------------25.03.2002 15:30------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxColorOptionsTabPage, SchemeChangedHdl_Impl, ListBox*, pBox)
{
    pColorConfig->LoadScheme(pBox->GetSelectEntry());
    pExtColorConfig->LoadScheme(pBox->GetSelectEntry());
    UpdateColorConfig();
    return 0;
}
/* -----------------------------09.04.2002 15:21------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxColorOptionsTabPage, SaveDeleteHdl_Impl, PushButton*, pButton )
{
    if(&aSaveSchemePB == pButton)
    {
        String sName;
        //CHINA001 SvxNameDialog aNameDlg(pButton,
        //CHINA001                     sName,
        //CHINA001                     String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2)));
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( pButton,
                            sName, String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2)) );
        DBG_ASSERT(aNameDlg, "Dialogdiet fail!");//CHINA001
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE1)));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetEditHelpId(HID_OPTIONS_COLORCONFIG_NAME_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute()) //CHINA001 if(RET_OK == aNameDlg.Execute())
        {
            aNameDlg->GetName(sName); //CHINA001 aNameDlg.GetName(sName);
            pColorConfig->AddScheme(sName);
            pExtColorConfig->AddScheme(sName);
            aColorSchemeLB.InsertEntry(sName);
            aColorSchemeLB.SelectEntry(sName);
            aColorSchemeLB.GetSelectHdl().Call(&aColorSchemeLB);
        }
        delete aNameDlg; //add by CHINA001
    }
    else
    {
        DBG_ASSERT(aColorSchemeLB.GetEntryCount() > 1, "don't delete the last scheme");
        QueryBox aQuery(pButton, CUI_RES(RID_SVXQB_DELETE_COLOR_CONFIG));
        aQuery.SetText(String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_DELETE)));
        if(RET_YES == aQuery.Execute())
        {
            rtl::OUString sDeleteScheme(aColorSchemeLB.GetSelectEntry());
            aColorSchemeLB.RemoveEntry(aColorSchemeLB.GetSelectEntryPos());
            aColorSchemeLB.SelectEntryPos(0);
            aColorSchemeLB.GetSelectHdl().Call(&aColorSchemeLB);
            //first select the new scheme and then delete the old one
            pColorConfig->DeleteScheme(sDeleteScheme);
            pExtColorConfig->DeleteScheme(sDeleteScheme);
        }
    }
    aDeleteSchemePB.Enable( aColorSchemeLB.GetEntryCount() > 1 );
    return 0;
}
/* -----------------------------09.04.2002 15:47------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxColorOptionsTabPage, CheckNameHdl_Impl, AbstractSvxNameDialog*, pDialog )
{
    String sName;
    pDialog->GetName(sName);
    return sName.Len() && LISTBOX_ENTRY_NOTFOUND == aColorSchemeLB.GetEntryPos( sName );
}
/* -----------------------------25.04.2002 15:12------------------------------

 ---------------------------------------------------------------------------*/
void SvxColorOptionsTabPage::FillUserData()
{
    SetUserData(String::CreateFromInt32(pColorConfigCT->GetScrollPosition()));
}

