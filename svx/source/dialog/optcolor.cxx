/*************************************************************************
 *
 *  $RCSfile: optcolor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-29 08:18:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _CTRLBOX_HXX
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _XTABLE_HXX
#include <xtable.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
//CHINA001 #ifndef _SVX_DLG_NAME_HXX
//CHINA001 #include <dlgname.hxx>
//CHINA001 #endif
#include "svxdlg.hxx" //CHINA001
#ifndef _SVX_HELPID_HRC
#include <helpid.hrc>
#endif
#include "svxids.hrc"
#include "dialmgr.hxx"
#include "optcolor.hxx"
#include "dialogs.hrc"
#include "optcolor.hrc"
#include "dlgutil.hxx"

using namespace ::com::sun::star;
using namespace ::svtools;
#define GROUP_COUNT 6
/* -----------------------------2002/06/26 10:48------------------------------

 ---------------------------------------------------------------------------*/
class SvxExtFixedText_Impl : public FixedText
{
protected:
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
public:
    SvxExtFixedText_Impl(Window* pParent, const ResId& rResId) :
        FixedText(pParent, rResId){}
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
    FixedText       aWrtNotesIndicatorFT;
    ColorListBox    aWrtNotesIndicatorLB;
    Window          aWrtNotesIndicatorWN;
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
    SvxExtFixedText_Impl*  aChapters[GROUP_COUNT];
    Window*         aChapterWins[GROUP_COUNT];
    FixedText*      aFixedTexts[ColorConfigEntryCount];
    CheckBox*       aCheckBoxes[ColorConfigEntryCount];
    ColorListBox*   aColorBoxes[ColorConfigEntryCount];
    Window*         aWindows[ColorConfigEntryCount];

    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
public:
    ColorConfigWindow_Impl(Window* pParent, const ResId& rResId);
    ~ColorConfigWindow_Impl();
};
/* -----------------------------25.03.2002 17:05------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigWindow_Impl::ColorConfigWindow_Impl(Window* pParent, const ResId& rResId) :
        Window(pParent, rResId),
        aGeneralBackWN(this),
        aGeneralFT(&aGeneralBackWN,  ResId(        FT_GENERAL              )),
        aDocColorFT(this, ResId(        FT_DOCCOLOR             )),
        aDocColorLB(this, ResId(        LB_DOCCOLOR             )),
        aDocColorWN(this, ResId(        WN_DOCCOLOR             )),
        aDocBoundCB(this, ResId(        CB_DOCBOUND             )),
        aDocBoundLB(this, ResId(        LB_DOCBOUND             )),
        aDocBoundWN(this, ResId(      WN_DOCBOUND             )),
        aAppBackFT(this, ResId(         FT_APPBACKGROUND        )),
        aAppBackLB(this, ResId(         LB_APPBACKGROUND        )),
        aAppBackWN(this, ResId(         WN_APPBACKGROUND        )),
        aObjBoundCB(this, ResId(        CB_OBJECTBOUNDARIES     )),
        aObjBoundLB(this, ResId(        LB_OBJECTBOUNDARIES     )),
        aObjBoundWN(this, ResId(        WN_OBJECTBOUNDARIES     )),
        aTableBoundCB(this, ResId(      CB_TABLEBOUNDARIES      )),
        aTableBoundLB(this, ResId(      LB_TABLEBOUNDARIES      )),
        aTableBoundWN(this, ResId(      WN_TABLEBOUNDARIES      )),
        aFontColorFT(this, ResId(       FT_FONTCOLOR            )),
        aFontColorLB(this, ResId(       LB_FONTCOLOR            )),
        aFontColorWN(this, ResId(       WN_FONTCOLOR            )),
        aLinksCB(this, ResId(           CB_LINKS                )),
        aLinksLB(this, ResId(           LB_LINKS                )),
        aLinksWN(this, ResId(           WN_LINKS                )),
        aLinksVisitedCB(this, ResId(    CB_LINKSVISITED         )),
        aLinksVisitedLB(this, ResId(    LB_LINKSVISITED         )),
        aLinksVisitedWN(this, ResId(    WN_LINKSVISITED         )),
        aSpellFT(this, ResId(        FT_SPELL          )),
        aSpellLB(this, ResId(        LB_SPELL          )),
        aSpellWN(this, ResId(        WN_SPELL          )),
        aWriterBackWN(this),
        aWriterFT(this, ResId(FT_WRITER               )),
        aWrtTextGridFT(this, ResId(     FT_WRITERTEXTGRID       )),
        aWrtTextGridLB(this, ResId(     LB_WRITERTEXTGRID       )),
        aWrtTextGridWN(this, ResId(     WN_WRITERTEXTGRID       )),
        aWrtFieldCB(this, ResId(        CB_WRITERFIELDSHADINGS  )),
        aWrtFieldLB(this, ResId(        LB_WRITERFIELDSHADINGS  )),
        aWrtFieldWN(this, ResId(        WN_WRITERFIELDSHADINGS  )),
        aWrtIdxShadingBackCB(this, ResId(  CB_WRITERIDXSHADINGS          )),
        aWrtIdxShadingBackLB(this, ResId(  LB_WRITERIDXSHADINGS          )),
        aWrtIdxShadingBackWN(this, ResId(  WN_WRITERIDXSHADINGS          )),
        aWrtDirectCrsrFT(this, ResId(      FT_WRITERDIRECTCURSOR         )),
        aWrtDirectCrsrLB(this, ResId(      LB_WRITERDIRECTCURSOR         )),
        aWrtDirectCrsrWN(this, ResId(      WN_WRITERDIRECTCURSOR         )),
        aWrtNotesIndicatorFT(this, ResId(      FT_WRITERNOTESINDICATOR         )),
        aWrtNotesIndicatorLB(this, ResId(      LB_WRITERNOTESINDICATOR         )),
        aWrtNotesIndicatorWN(this, ResId(      WN_WRITERNOTESINDICATOR         )),
        aWrtScriptIndicatorFT(this, ResId(      FT_WRITERSCRIPTINDICATOR         )),
        aWrtScriptIndicatorLB(this, ResId(      LB_WRITERSCRIPTINDICATOR         )),
        aWrtScriptIndicatorWN(this, ResId(      WN_WRITERSCRIPTINDICATOR         )),
        aWrtSectionBoundCB(this, ResId(      CB_WRITERSECTIONBOUNDARIES    )),
        aWrtSectionBoundLB(this, ResId(      LB_WRITERSECTIONBOUNDARIES    )),
        aWrtSectionBoundWN(this, ResId(      WN_WRITERSECTIONBOUNDARIES    )),
        aWrtPageBreaksFT(this, ResId(      FT_WRITERPAGEBREAKS    )),
        aWrtPageBreaksLB(this, ResId(      LB_WRITERPAGEBREAKS    )),
        aWrtPageBreaksWN(this, ResId(      WN_WRITERPAGEBREAKS    )),
        aHTMLBackWN(this),
        aHTMLFT(this, ResId(            FT_HTML                 )),
        aHTMLSGMLFT(this, ResId(        FT_HTMLSGML             )),
        aHTMLSGMLLB(this, ResId(        LB_HTMLSGML             )),
        aHTMLSGMLWN(this, ResId(        WN_HTMLSGML             )),
        aHTMLCommentFT(this, ResId(     FT_HTMLCOMMENT          )),
        aHTMLCommentLB(this, ResId(     LB_HTMLCOMMENT          )),
        aHTMLCommentWN(this, ResId(     WN_HTMLCOMMENT          )),
        aHTMLKeywdFT(this, ResId(       FT_HTMLKEYWORD          )),
        aHTMLKeywdLB(this, ResId(       LB_HTMLKEYWORD          )),
        aHTMLKeywdWN(this, ResId(       WN_HTMLKEYWORD          )),
        aHTMLUnknownFT(this, ResId(     FT_HTMLUNKNOWN          )),
        aHTMLUnknownLB(this, ResId(     LB_HTMLUNKNOWN          )),
        aHTMLUnknownWN(this, ResId(     WN_HTMLUNKNOWN          )),
        aCalcBackWN(this),
        aCalcFT(this, ResId(    FT_CALC                 )),
        aCalcGridFT(this, ResId(        FT_CALCGRID             )),
        aCalcGridLB(this, ResId(        LB_CALCGRID             )),
        aCalcGridWN(this, ResId(        WN_CALCGRID             )),
        aCalcPageBreakFT(this, ResId(   FT_CALCPAGEBREAK   )),
        aCalcPageBreakLB(this, ResId(   LB_CALCPAGEBREAK   )),
        aCalcPageBreakWN(this, ResId(   WN_CALCPAGEBREAK   )),
        aCalcPageBreakManualFT(this, ResId(   FT_CALCPAGEBREAKMANUAL   )),
        aCalcPageBreakManualLB(this, ResId(   LB_CALCPAGEBREAKMANUAL   )),
        aCalcPageBreakManualWN(this, ResId(   WN_CALCPAGEBREAKMANUAL   )),
        aCalcPageBreakAutoFT(this, ResId(   FT_CALCPAGEBREAKAUTO   )),
        aCalcPageBreakAutoLB(this, ResId(   LB_CALCPAGEBREAKAUTO   )),
        aCalcPageBreakAutoWN(this, ResId(   WN_CALCPAGEBREAKAUTO   )),
        aCalcDetectiveFT(this, ResId(   FT_CALCDETECTIVE        )),
        aCalcDetectiveLB(this, ResId(   LB_CALCDETECTIVE        )),
        aCalcDetectiveWN(this, ResId(   WN_CALCDETECTIVE        )),
        aCalcDetectiveErrorFT(this, ResId(   FT_CALCDETECTIVEERROR        )),
        aCalcDetectiveErrorLB(this, ResId(   LB_CALCDETECTIVEERROR        )),
        aCalcDetectiveErrorWN(this, ResId(   WN_CALCDETECTIVEERROR        )),
        aCalcReferenceFT(this, ResId(   FT_CALCREFERENCE        )),
        aCalcReferenceLB(this, ResId(   LB_CALCREFERENCE        )),
        aCalcReferenceWN(this, ResId(   WN_CALCREFERENCE        )),
        aCalcNotesBackFT(this, ResId(   FT_CALCNOTESBACKGROUND      )),
        aCalcNotesBackLB(this, ResId(   LB_CALCNOTESBACKGROUND      )),
        aCalcNotesBackWN(this, ResId(   WN_CALCNOTESBACKGROUND      )),
        aDrawBackWN(this),
        aDrawFT(this, ResId(            FT_DRAW                 )),
        aDrawGridFT(this, ResId(        FT_DRAWGRID             )),
        aDrawGridLB(this, ResId(        LB_DRAWGRID             )),
        aDrawGridWN(this, ResId(        WN_DRAWGRID             )),
        aBasicBackWN(this),
        aBasicFT(this, ResId(            FT_BASIC                 )),
        aBasicIdentifierFT(this, ResId( FT_BASICIDENTIFIER )),
        aBasicIdentifierLB(this, ResId( LB_BASICIDENTIFIER )),
        aBasicIdentifierWN(this, ResId( WN_BASICIDENTIFIER )),
        aBasicCommentFT(this, ResId( FT_BASICCOMMENT    )),
        aBasicCommentLB(this, ResId( LB_BASICCOMMENT    )),
        aBasicCommentWN(this, ResId( WN_BASICCOMMENT    )),
        aBasicNumberFT(this, ResId( FT_BASICNUMBER     )),
        aBasicNumberLB(this, ResId( LB_BASICNUMBER     )),
        aBasicNumberWN(this, ResId( WN_BASICNUMBER     )),
        aBasicStringFT(this, ResId( FT_BASICSTRING     )),
        aBasicStringLB(this, ResId( LB_BASICSTRING     )),
        aBasicStringWN(this, ResId( WN_BASICSTRING     )),
        aBasicOperatorFT(this, ResId( FT_BASICOPERATOR   )),
        aBasicOperatorLB(this, ResId( LB_BASICOPERATOR   )),
        aBasicOperatorWN(this, ResId( WN_BASICOPERATOR   )),
        aBasicKeywordFT(this, ResId( FT_BASICKEYWORD    )),
        aBasicKeywordLB(this, ResId( LB_BASICKEYWORD    )),
        aBasicKeywordWN(this, ResId( WN_BASICKEYWORD    )),
        aBasicErrorFT(this, ResId( FT_BASICERROR        )),
        aBasicErrorLB(this, ResId( LB_BASICERROR        )),
        aBasicErrorWN(this, ResId( WN_BASICERROR        ))
{
    FreeResource();
    long i;
    for( i = 0; i < ColorConfigEntryCount; i++ )
    {
        aCheckBoxes[i] = 0;
        aFixedTexts[i] = 0;
        aColorBoxes[i] = 0;
        aWindows[i] = 0;
    }
    aFixedTexts[DOCCOLOR         ] = &aDocColorFT;
    aCheckBoxes[DOCBOUNDARIES       ] = &aDocBoundCB             ;
    aFixedTexts[APPBACKGROUND    ] = &aAppBackFT;
    aCheckBoxes[OBJECTBOUNDARIES    ] = &aObjBoundCB             ;
    aCheckBoxes[TABLEBOUNDARIES     ] = &aTableBoundCB           ;
    aFixedTexts[FONTCOLOR           ] = &aFontColorFT            ;
    aCheckBoxes[LINKS               ] = &aLinksCB                ;
    aCheckBoxes[LINKSVISITED        ] = &aLinksVisitedCB         ;
    aFixedTexts[SPELL            ]=& aSpellFT;
    aFixedTexts[WRITERTEXTGRID   ]=& aWrtTextGridFT;
    aCheckBoxes[WRITERFIELDSHADINGS ] = &aWrtFieldCB             ;
    aCheckBoxes[WRITERIDXSHADINGS   ] = &aWrtIdxShadingBackCB       ;
    aFixedTexts[WRITERDIRECTCURSOR  ]=& aWrtDirectCrsrFT;
    aFixedTexts[WRITERNOTESINDICATOR    ]=& aWrtNotesIndicatorFT;
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
    aFixedTexts[BASICERROR    ] = &aBasicErrorFT;
    aColorBoxes[DOCCOLOR            ] = &aDocColorLB             ;
    aColorBoxes[DOCBOUNDARIES       ] = &aDocBoundLB             ;
    aColorBoxes[APPBACKGROUND       ] = &aAppBackLB              ;
    aColorBoxes[OBJECTBOUNDARIES    ] = &aObjBoundLB             ;
    aColorBoxes[TABLEBOUNDARIES     ] = &aTableBoundLB           ;
    aColorBoxes[FONTCOLOR           ] = &aFontColorLB            ;
    aColorBoxes[LINKS               ] = &aLinksLB                ;
    aColorBoxes[LINKSVISITED        ] = &aLinksVisitedLB         ;
    aColorBoxes[SPELL               ] = &aSpellLB             ;
    aColorBoxes[WRITERTEXTGRID      ] = &aWrtTextGridLB          ;
    aColorBoxes[WRITERFIELDSHADINGS ] = &aWrtFieldLB             ;
    aColorBoxes[WRITERIDXSHADINGS   ] = &aWrtIdxShadingBackLB       ;
    aColorBoxes[WRITERDIRECTCURSOR  ] = &aWrtDirectCrsrLB           ;
    aColorBoxes[WRITERNOTESINDICATOR     ] = &aWrtNotesIndicatorLB           ;
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
    aColorBoxes[BASICIDENTIFIER    ] = &aBasicIdentifierLB;
    aColorBoxes[BASICCOMMENT       ] = &aBasicCommentLB;
    aColorBoxes[BASICNUMBER        ] = &aBasicNumberLB;
    aColorBoxes[BASICSTRING        ] = &aBasicStringLB;
    aColorBoxes[BASICOPERATOR      ] = &aBasicOperatorLB;
    aColorBoxes[BASICKEYWORD       ] = &aBasicKeywordLB;
    aColorBoxes[BASICERROR       ] = &aBasicErrorLB;
    aWindows[DOCCOLOR            ] = &aDocColorWN             ;
    aWindows[DOCBOUNDARIES       ] = &aDocBoundWN             ;
    aWindows[APPBACKGROUND       ] = &aAppBackWN              ;
    aWindows[OBJECTBOUNDARIES    ] = &aObjBoundWN             ;
    aWindows[TABLEBOUNDARIES     ] = &aTableBoundWN           ;
    aWindows[FONTCOLOR           ] = &aFontColorWN            ;
    aWindows[LINKS               ] = &aLinksWN                ;
    aWindows[LINKSVISITED        ] = &aLinksVisitedWN         ;
    aWindows[SPELL               ] = &aSpellWN             ;
    aWindows[WRITERTEXTGRID      ] = &aWrtTextGridWN          ;
    aWindows[WRITERFIELDSHADINGS ] = &aWrtFieldWN             ;
    aWindows[WRITERIDXSHADINGS   ] = &aWrtIdxShadingBackWN       ;
    aWindows[WRITERDIRECTCURSOR  ] = &aWrtDirectCrsrWN           ;
    aWindows[WRITERNOTESINDICATOR     ] = &aWrtNotesIndicatorWN           ;
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
    aWindows[CALCNOTESBACKGROUND     ] = &aCalcNotesBackWN            ;
    aWindows[DRAWGRID            ] = &aDrawGridWN             ;
    aWindows[BASICIDENTIFIER     ] = &aBasicIdentifierWN;
    aWindows[BASICCOMMENT        ] = &aBasicCommentWN;
    aWindows[BASICNUMBER         ] = &aBasicNumberWN;
    aWindows[BASICSTRING         ] = &aBasicStringWN;
    aWindows[BASICOPERATOR       ] = &aBasicOperatorWN;
    aWindows[BASICKEYWORD        ] = &aBasicKeywordWN;
    aWindows[BASICERROR        ] = &aBasicErrorWN;
    aChapters[0] = &aGeneralFT; aChapterWins[0] = &aGeneralBackWN;
    aChapters[1] = &aWriterFT;  aChapterWins[1] = &aWriterBackWN;
    aChapters[2] = &aHTMLFT;    aChapterWins[2] = &aHTMLBackWN;
    aChapters[3] = &aCalcFT;    aChapterWins[3] = &aCalcBackWN;
    aChapters[4] = &aDrawFT;    aChapterWins[4] = &aDrawBackWN;
    aChapters[5] = &aBasicFT;   aChapterWins[5] = &aBasicBackWN;

    Color TempColor(COL_TRANSPARENT);
    Wallpaper aTransparentWall(TempColor);
    sal_Int32 nWinWidth = GetSizePixel().Width();
    sal_Int32 nFTHeight = aChapters[0]->GetSizePixel().Height();
    sal_Bool bHighContrast = GetDisplayBackground().GetColor().IsDark();
    Color aBackColor;
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if ( bHighContrast )
        aBackColor = rStyleSettings.GetShadowColor();
    else
        aBackColor = Color( COL_LIGHTGRAY);
    for( i = 0; i < GROUP_COUNT; ++i)
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
    Color aTextColor;
    BOOL bSetTextColor = FALSE;
    //#104195# when the window color is the same as the text color it has to be changed
    Color aWinCol = rStyleSettings.GetWindowColor();
    Color aRCheckCol = rStyleSettings.GetRadioCheckTextColor();
    if(aWinCol == aRCheckCol )
    {
        bSetTextColor = TRUE;
        aRCheckCol.Invert();
        //if inversion didn't work (gray) then it's set to black
        if(aRCheckCol == aWinCol)
            aRCheckCol = Color(COL_BLACK);
    }
    for( i = 0; i < ColorConfigEntryCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        if(aCheckBoxes[i])
        {
            aCheckBoxes[i]->SetBackground(aTransparentWall);
            aCheckBoxes[i]->SetHelpId(HID_COLORPAGE_CHECKBOX_START + i);
            if(bSetTextColor)
                aCheckBoxes[i]->SetTextColor(aRCheckCol);
        }
        if(aFixedTexts[i])
        {
            aFixedTexts[i]->SetBackground(aTransparentWall);
            if(bSetTextColor)
                aFixedTexts[i]->SetTextColor(aRCheckCol);
        }
        if(aWindows[i])
            aWindows[i]->SetBorderStyle(WINDOW_BORDER_MONO     );
    }

    XColorTable aColorTable( SvtPathOptions().GetPalettePath() );
    aColorBoxes[0]->InsertAutomaticEntry();
    for( i = 0; i < aColorTable.Count(); i++ )
    {
        XColorEntry* pEntry = aColorTable.Get(i);
        aColorBoxes[0]->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }
    aColorBoxes[0]->SetHelpId(HID_COLORPAGE_LISTBOX_START);
    for( i = 1; i < ColorConfigEntryCount; i++ )
    {
        if(aColorBoxes[i])
        {
            aColorBoxes[i]->CopyEntries( *aColorBoxes[0] );
            aColorBoxes[i]->SetHelpId(HID_COLORPAGE_LISTBOX_START + i);
        }
    }
}
/* -----------------------------27.03.2002 11:04------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigWindow_Impl::~ColorConfigWindow_Impl()
{
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
        for( USHORT i = 0; i < GROUP_COUNT; i++)
            aChapterWins[i]->SetBackground(Wallpaper(aBackColor));
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

    EditableColorConfig* pColorConfig;

    long            nScrollPos;

    DECL_LINK(ScrollHdl, ScrollBar*);
    DECL_LINK(ClickHdl, CheckBox*);
    DECL_LINK(ColorHdl, ColorListBox*);
    DECL_LINK(ControlFocusHdl, Control*);

    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        Command( const CommandEvent& rCEvt );
public:
    ColorConfigCtrl_Impl(Window* pParent, const ResId& rResId );
    ~ColorConfigCtrl_Impl();

    void SetConfig(EditableColorConfig& rConfig) {pColorConfig = &rConfig;}
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
        aVScroll(this,      ResId(VB_VSCROLL)),
        aScrollWindow(this, ResId(WN_SCROLL )),
        sOn(                ResId(ST_ON     )),
        sUIElem(            ResId(ST_UIELEM )),
        sColSetting(        ResId(ST_COLSET )),
        sPreview(           ResId(ST_PREVIEW)),
        nScrollPos(0),
        pColorConfig(0)
{
    FreeResource();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    aScrollWindow.SetBackground(Wallpaper(rStyleSettings.GetWindowColor()));
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
    aHeaderHB.InsertItem( 1, sOn, nFirstWidth, nHeadBits|HIB_CENTER);
    aHeaderHB.InsertItem( 2, sUIElem, nSecondWidth, nHeadBits|HIB_LEFT);
    aHeaderHB.InsertItem( 3, sColSetting, nThirdWidth, nHeadBits|HIB_LEFT);
    aHeaderHB.InsertItem( 4, sPreview,
            aHeaderHB.GetSizePixel().Width() - nFirstWidth - nSecondWidth - nThirdWidth, nHeadBits|HIB_LEFT);
    aHeaderHB.Show();

    aVScroll.SetRangeMin(0);
    sal_Int32 nScrollOffset = aScrollWindow.aColorBoxes[1]->GetPosPixel().Y() - aScrollWindow.aColorBoxes[0]->GetPosPixel().Y();
    sal_Int32 nVisibleEntries = aScrollWindow.GetSizePixel().Height() / nScrollOffset;
    aVScroll.SetRangeMax(ColorConfigEntryCount + GROUP_COUNT );
    //minus two for DRAWFILL and DRAWDRAWING
    aVScroll.SetRangeMax(aVScroll.GetRangeMax() -2);
    aVScroll.SetRangeMax(aVScroll.GetRangeMax() -1);
    aVScroll.SetPageSize( nVisibleEntries - 1 );
    aVScroll.SetVisibleSize(nVisibleEntries);
    Link aScrollLink = LINK(this, ColorConfigCtrl_Impl, ScrollHdl);
    aVScroll.SetScrollHdl(aScrollLink);
    aVScroll.SetEndScrollHdl(aScrollLink);

    Link aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    Link aGetFocusLink = LINK(this, ColorConfigCtrl_Impl, ControlFocusHdl);
    for( sal_Int32 i = 0; i < ColorConfigEntryCount; i++ )
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
    DBG_ASSERT(pColorConfig, "Configuration not set" )
    sal_Int32 i;
    for( i = 0; i < ColorConfigEntryCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        const ColorConfigValue& rColorEntry = pColorConfig->GetColorValue(ColorConfigEntry(i));
        if(COL_AUTO == rColorEntry.nColor)
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
    }
}
/* -----------------------------26.03.2002 12:55------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool lcl_MoveAndShow(Window* pWindow, long nOffset, long nMaxVisible)
{
    BOOL bHide = TRUE;
    if(pWindow)
    {
        Point aPos = pWindow->GetPosPixel();
        aPos.Y() += nOffset;
        pWindow->SetPosPixel(aPos);
        bHide = aPos.Y() > nMaxVisible || (aPos.Y() + pWindow->GetSizePixel().Height()) < 0;
        pWindow->Show(!bHide);
    }
    return !bHide;
}
IMPL_LINK(ColorConfigCtrl_Impl, ScrollHdl, ScrollBar*, pScrollBar)
{
    aScrollWindow.SetUpdateMode(TRUE);
    sal_Int32 i;
    long nOffset = aScrollWindow.aColorBoxes[1]->GetPosPixel().Y() - aScrollWindow.aColorBoxes[0]->GetPosPixel().Y();
    nOffset *= (nScrollPos - pScrollBar->GetThumbPos());
    nScrollPos = pScrollBar->GetThumbPos();
    const long nWindowHeight = aScrollWindow.GetSizePixel().Height();
    long nFirstVisible = -1;
    long nLastVisible = -1;
    for( i = 0; i < ColorConfigEntryCount; i++ )
    {
        if(ANCHOR == i)
            continue;
        Point aPos;
        //controls outside of the view need to be hidden to speed up accessibility tools
        lcl_MoveAndShow(aScrollWindow.aCheckBoxes[i], nOffset, nWindowHeight);
        lcl_MoveAndShow(aScrollWindow.aFixedTexts[i], nOffset, nWindowHeight);
        lcl_MoveAndShow(aScrollWindow.aWindows[i]   , nOffset, nWindowHeight);
        BOOL bShow = lcl_MoveAndShow(aScrollWindow.aColorBoxes[i], nOffset, nWindowHeight);
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

        if(aScrollWindow.aCheckBoxes[nFirstVisible])
            aScrollWindow.aCheckBoxes[nFirstVisible]->Show();
        if(aScrollWindow.aColorBoxes[nFirstVisible])
            aScrollWindow.aColorBoxes[nFirstVisible]->Show();
    }

    if(nLastVisible < ColorConfigEntryCount - 1)
    {
        nLastVisible++;
        //skip gaps where no controls exist for the related ColorConfigEntry
        while(!aScrollWindow.aCheckBoxes[nLastVisible] && !aScrollWindow.aColorBoxes[nLastVisible] &&
                nLastVisible < ColorConfigEntryCount - 1 )
            nLastVisible++;
        if(nLastVisible < ColorConfigEntryCount)
        {
            if(aScrollWindow.aCheckBoxes[nLastVisible])
                aScrollWindow.aCheckBoxes[nLastVisible]->Show();
            if(aScrollWindow.aColorBoxes[nLastVisible])
                aScrollWindow.aColorBoxes[nLastVisible]->Show();
        }
    }
    for( i = 0; i < GROUP_COUNT; i++ )
    {
        Point aPos = aScrollWindow.aChapters[i]->GetPosPixel(); aPos.Y() += nOffset; aScrollWindow.aChapters[i]->SetPosPixel(aPos);
        aPos = aScrollWindow.aChapterWins[i]->GetPosPixel(); aPos.Y() += nOffset; aScrollWindow.aChapterWins[i]->SetPosPixel(aPos);
    }
    aScrollWindow.SetUpdateMode(TRUE);
    return 0;
}
/* -----------------------------29.04.2002 17:02------------------------------

 ---------------------------------------------------------------------------*/
long ColorConfigCtrl_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == EVENT_COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        USHORT nCmd = pCEvt->GetCommand();
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
/* -----------------------------27.03.2002 11:43------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(ColorConfigCtrl_Impl, ClickHdl, CheckBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" )
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
    }
    return 0;
}
/* -----------------------------27.03.2002 11:43------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(ColorConfigCtrl_Impl, ColorHdl, ColorListBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" )
    for( sal_Int32 i = 0; i < ColorConfigEntryCount; i++ )
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
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_COLORCONFIG ), rCoreSet ),
       aColorSchemeFL(  this, ResId( FL_COLORSCHEME ) ),
       aColorSchemeFT(  this, ResId( FT_COLORSCHEME ) ),
       aColorSchemeLB(  this, ResId( LB_COLORSCHEME ) ),
       aSaveSchemePB(   this, ResId( PB_SAVESCHEME) ),
       aDeleteSchemePB( this, ResId( PB_DELETESCHEME ) ),
       aCustomColorsFL( this, ResId( FL_CUSTOMCOLORS ) ),
       pColorConfigCT(  new ColorConfigCtrl_Impl(this, ResId( CT_COLORCONFIG ) )),
       pColorConfig(0),
       bFillItemSetCalled(FALSE)
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
            pColorConfig->SetCurrentSchemeName(sOldScheme);
    }
    delete pColorConfigCT;
    pColorConfig->ClearModified();
    pColorConfig->EnableBroadcast();
    delete pColorConfig;
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
SfxTabPage* SvxColorOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxColorOptionsTabPage( pParent, rAttrSet ) );
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
BOOL SvxColorOptionsTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    bFillItemSetCalled = TRUE;
    if(aColorSchemeLB.GetSavedValue() != aColorSchemeLB.GetSelectEntryPos())
        pColorConfig->SetModified();
    if(pColorConfig->IsModified())
        pColorConfig->Commit();
    return TRUE;
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
void SvxColorOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    if(pColorConfig)
    {
        pColorConfig->ClearModified();
        pColorConfig->DisableBroadcast();
        delete pColorConfig;
    }
    pColorConfig = new EditableColorConfig;
    pColorConfigCT->SetConfig(*pColorConfig);

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
void SvxColorOptionsTabPage::ActivatePage( const SfxItemSet& rSet )
{
}
/* -----------------------------25.03.2002 10:47------------------------------

 ---------------------------------------------------------------------------*/
int SvxColorOptionsTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
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
        //CHINA001                     String(SVX_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2)));
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( pButton,
                            sName, String(SVX_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2)), ResId(RID_SVXDLG_NAME) );
        DBG_ASSERT(aNameDlg, "Dialogdiet fail!");//CHINA001
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(String(SVX_RES(RID_SVXSTR_COLOR_CONFIG_SAVE1)));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetEditHelpId(HID_OPTIONS_COLORCONFIG_NAME_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute()) //CHINA001 if(RET_OK == aNameDlg.Execute())
        {
            aNameDlg->GetName(sName); //CHINA001 aNameDlg.GetName(sName);
            pColorConfig->AddScheme(sName);
            aColorSchemeLB.InsertEntry(sName);
            aColorSchemeLB.SelectEntry(sName);
            aColorSchemeLB.GetSelectHdl().Call(&aColorSchemeLB);
        }
        delete aNameDlg; //add by CHINA001
    }
    else
    {
        DBG_ASSERT(aColorSchemeLB.GetEntryCount() > 1, "don't delete the last scheme")
        QueryBox aQuery(pButton, SVX_RES(RID_SVXQB_DELETE_COLOR_CONFIG));
        aQuery.SetText(String(SVX_RES(RID_SVXSTR_COLOR_CONFIG_DELETE)));
        if(RET_YES == aQuery.Execute())
        {
            rtl::OUString sDeleteScheme(aColorSchemeLB.GetSelectEntry());
            aColorSchemeLB.RemoveEntry(aColorSchemeLB.GetSelectEntryPos());
            aColorSchemeLB.SelectEntryPos(0);
            aColorSchemeLB.GetSelectHdl().Call(&aColorSchemeLB);
            //first select the new scheme and then delete the old one
            pColorConfig->DeleteScheme(sDeleteScheme);
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

