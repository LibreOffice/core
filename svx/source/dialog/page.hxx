/*************************************************************************
 *
 *  $RCSfile: page.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:53:27 $
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
#ifndef _SVX_PAGE_HXX
#define _SVX_PAGE_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef _SVX_PAGECTRL_HXX //autogen
#include "pagectrl.hxx"
#endif
#ifndef _SVX_FRMDIRLBOX_HXX
#include "frmdirlbox.hxx"
#endif

#include "svxenum.hxx"
#include "flagsdef.hxx" //CHINA001

// enum ------------------------------------------------------------------

//CHINA001 enum SvxModeType
//CHINA001 {
//CHINA001  SVX_PAGE_MODE_STANDARD,
//CHINA001  SVX_PAGE_MODE_CENTER,
//CHINA001  SVX_PAGE_MODE_PRESENTATION
//CHINA001 };

// class SvxPageDescPage -------------------------------------------------

/*  {k:\svx\prototyp\dialog\page.bmp}

    [Beschreibung]
    TabPage zur Einstellung einer Seite (Groesse, Raender, ...)

    [Items]
    <SvxPageItem>:          <SID_ATTR_PAGE>
    <SvxSizeItem>:          <SID_ATTR_SIZE>
    <SvxSizeItem>:          <SID_ATTR_MAXSIZE>
    <SvxULSpaceItem>:       <SID_ATTR_LRSPACE>
    <SvxLRSpaceItem>:       <SID_ATTR_ULSPACE>
    <SfxAllEnumItem>:       <SID_ATTR_PAPERTRAY>
    <SvxPaperBinItem>:      <SID_ATTR_PAPERBIN>
    <SvxBoolItem>:          <SID_ATTR_EXT1>
    <SvxBoolItem>:          <SID_ATTR_EXT2>

    <SfxSetItem>:           <SID_ATTR_HEADERSET>
        <SfxBoolItem>:          <SID_ATTR_ON>
        <SfxBoolItem>:          <SID_ATTR_DYNAMIC>
        <SfxBoolItem>:          <SID_ATTR_SHARED>
        <SvxSizeItem>:          <SID_ATTR_SIZE>
        <SvxULSpaceItem>:       <SID_ATTR_ULSPACE>
        <SvxLRSpaceItem>:       <SID_ATTR_LRSPACE>

    <SfxSetItem>:           <SID_ATTR_FOOTERSET>
        <SfxBoolItem>:          <SID_ATTR_ON>
        <SfxBoolItem>:          <SID_ATTR_DYNAMIC>
        <SfxBoolItem>:          <SID_ATTR_SHARED>
        <SvxSizeItem>:          <SID_ATTR_SIZE>
        <SvxULSpaceItem>:       <SID_ATTR_ULSPACE>
        <SvxLRSpaceItem>:       <SID_ATTR_LRSPACE>
*/

struct  SvxPage_Impl;
typedef USHORT MarginPosition;

class SvxPageDescPage : public SfxTabPage
{
    // paper format
    FixedLine           aPaperSizeFl;
    FixedText           aPaperFormatText;
    ListBox             aPaperSizeBox;

    FixedText           aPaperWidthText;
    MetricField         aPaperWidthEdit;
    FixedText           aPaperHeightText;
    MetricField         aPaperHeightEdit;

    FixedText           aOrientationFT;
    RadioButton         aPortraitBtn;
    RadioButton         aLandscapeBtn;

    SvxPageWindow       aBspWin;

    FixedText                   aTextFlowLbl;
    svx::FrameDirectionListBox  aTextFlowBox;

    FixedText           aPaperTrayLbl;
    ListBox             aPaperTrayBox;

    // Margins
    FixedLine           aMarginFl;
    FixedText           aLeftMarginLbl;
    MetricField         aLeftMarginEdit;
    FixedText           aRightMarginLbl;
    MetricField         aRightMarginEdit;
    FixedText           aTopMarginLbl;
    MetricField         aTopMarginEdit;
    FixedText           aBottomMarginLbl;
    MetricField         aBottomMarginEdit;

    // layout settings
    FixedLine           aLayoutFL;
    ListBox             aLayoutBox;
    FixedText           aPageText;
    ListBox             aNumberFormatBox;
    FixedText           aNumberFormatText;

    FixedLine           aBottomSeparatorFl;

    //Extras Calc
    FixedText           aTblAlignFT;
    CheckBox            aHorzBox;
    CheckBox            aVertBox;

    // Impress and Draw
    CheckBox            aAdaptBox;

    //Register Writer
    CheckBox            aRegisterCB;
    FixedText           aRegisterFT;
    ListBox             aRegisterLB;

    String              sStandardRegister;

    String              aInsideText;
    String              aOutsideText;
    String              aLeftText;
    String              aRightText;
    String              aPrintRangeQueryText;

    long                nFirstLeftMargin;
    long                nFirstRightMargin;
    long                nFirstTopMargin;
    long                nFirstBottomMargin;
    long                nLastLeftMargin;
    long                nLastRightMargin;
    long                nLastTopMargin;
    long                nLastBottomMargin;

    Size                aMaxSize;
    BOOL                bLandscape;
    FASTBOOL            bBorderModified;
    SvxModeType         eMode;
    SvxPaper            ePaperStart;
    SvxPaper            ePaperEnd;

    SvxPage_Impl*       pImpl;

#ifdef _SVX_PAGE_CXX
    void                Init_Impl();
    DECL_LINK(          LayoutHdl_Impl, ListBox* );
    DECL_LINK(          PaperBinHdl_Impl, ListBox* );
    DECL_LINK(          SwapOrientation_Impl, RadioButton* );
    void                SwapFirstValues_Impl( FASTBOOL bSet );
    DECL_LINK(          BorderModify_Impl, MetricField* );
    void                InitHeadFoot_Impl( const SfxItemSet& rSet );
    DECL_LINK(          CenterHdl_Impl, CheckBox* );
    void                UpdateExample_Impl();

    // Papiergroesse
    DECL_LINK(          PaperSizeSelect_Impl, ListBox* );
    DECL_LINK(          PaperSizeModify_Impl, Edit* );

    DECL_LINK(          FrameDirectionModify_Impl, ListBox* );

    // Hintergrund
    void                ResetBackground_Impl( const SfxItemSet& rSet );

    // Grenzwerte
    DECL_LINK(          RangeHdl_Impl, Edit* );
    void                CalcMargin_Impl();

    // Registerhaltigkeit
    DECL_LINK(          RegisterModify, CheckBox * );

    // page direction
    /** Disables vertical page direction entries in the text flow listbox. */
    void                DisableVerticalPageDir();

    bool                IsPrinterRangeOverflow( MetricField& rField, long nFirstMargin,
                                                long nLastMargin, MarginPosition nPos );
    void                CheckMarginEdits( bool _bClear );
    bool                IsMarginOutOfRange();
#endif

    SvxPageDescPage( Window* pParent, const SfxItemSet& rSet );

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rOutSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        FillUserData();

    virtual ~SvxPageDescPage();

    void                SetMode( SvxModeType eMType ) { eMode = eMType; }
    void                SetPaperFormatRanges( SvxPaper eStart, SvxPaper eEnd )
                            { ePaperStart = eStart, ePaperEnd = eEnd; }

    void                SetCollectionList(const List* pList);
    virtual void        PageCreated (SfxAllItemSet aSet); //add CHINA001
};

#endif // #ifndef _SVX_PAGE_HXX

