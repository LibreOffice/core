/*************************************************************************
 *
 *  $RCSfile: border.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:42:13 $
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
#ifndef _SVX_BORDER_HXX
#define _SVX_BORDER_HXX

// include ---------------------------------------------------------------


#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef SVX_FRMSEL_HXX
#include "frmsel.hxx"
#endif
// forward ---------------------------------------------------------------

class SvxBorderLine;
class XColorTable;

// class SvxBorderTabPage ------------------------------------------------
/*
{k:\svx\prototyp\dialog\border.bmp}
[Beschreibung]
TabPage zur Einstellung der Umrandung (Staerke, Farbe, Schatten, ...)
[Items]
<SvxBoxItem><SID_ATTR_BORDER_OUTER>
<SvxBoxInfoItem><SID_ATTR_BORDER_INNER>
<SvxShadowItem><SID_ATTR_BORDER_SHADOW>
*/

// Border-Modes for paragraphs, textframes and tables
//CHINA001 #define SW_BORDER_MODE_PARA  0x01
//CHINA001 #define SW_BORDER_MODE_TABLE    0x02
//CHINA001 #define SW_BORDER_MODE_FRAME    0x04
#include "flagsdef.hxx"//CHINA001

class SvxBorderTabPage : public SfxTabPage
{
public:
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rCoreAttrs );
    virtual void        Reset( const SfxItemSet& );

    void                HideShadowControls();
    void                SetSWMode(BYTE nSet);
    virtual void        PageCreated (SfxAllItemSet aSet); //add CHINA001
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

private:
    SvxBorderTabPage( Window* pParent, const SfxItemSet& rCoreAttrs );
    ~SvxBorderTabPage();

    // Controls
    FixedLine           aFlBorder;
    FixedText           aDefaultFT;
    ValueSet            aWndPresets;
    FixedText           aUserDefFT;
    svx::FrameSelector  aFrameSel;

    FixedLine           aFlSep1;
    FixedLine           aFlLine;
    FixedText           aStyleFT;
    LineListBox         aLbLineStyle;
    FixedText           aColorFT;
    ColorListBox        aLbLineColor;

    FixedLine           aFlSep2;
    FixedLine           aDistanceFL;
    FixedText           aLeftFT;
    MetricField         aLeftMF;
    FixedText           aRightFT;
    MetricField         aRightMF;
    FixedText           aTopFT;
    MetricField         aTopMF;
    FixedText           aBottomFT;
    MetricField         aBottomMF;
    CheckBox            aSynchronizeCB;

    FixedLine           aFlShadow;
    FixedText           aFtShadowPos;
    ValueSet            aWndShadows;
    FixedText           aFtShadowSize;
    MetricField         aEdShadowSize;
    FixedText           aFtShadowColor;
    ColorListBox        aLbShadowColor;

    //properties - "Merge with next paragraph" in Writer
    FixedLine           aPropertiesFL;
    CheckBox            aMergeWithNextCB;
    // --> collapsing table borders FME 2005-05-27 #i29550#
    CheckBox            aMergeAdjacentBordersCB;
    // <--

    ImageList           aShadowImgLstH;
    ImageList           aShadowImgLst;
    ImageList           aBorderImgLstH;
    ImageList           aBorderImgLst;

    long                nMinValue;  // minimum distance
    BYTE                nSWMode;    // table, textframe, paragraph

    bool                mbHorEnabled;       /// true = Inner horizontal border enabled.
    bool                mbVerEnabled;       /// true = Inner vertical border enabled.
    bool                mbTLBREnabled;      /// true = Top-left to bottom-right border enabled.
    bool                mbBLTREnabled;      /// true = Bottom-left to top-right border enabled.
    bool                mbUseMarginItem;

    static BOOL         bSync;

#ifdef _SVX_BORDER_CXX
    // Handler
    DECL_LINK( SelStyleHdl_Impl, ListBox* pLb );
    DECL_LINK( SelColHdl_Impl, ListBox* pLb );
    DECL_LINK( SelPreHdl_Impl, void* );
    DECL_LINK( SelSdwHdl_Impl, void* );
    DECL_LINK( LinesChanged_Impl, void* );
    DECL_LINK( ModifyDistanceHdl_Impl, MetricField*);
    DECL_LINK( SyncHdl_Impl, CheckBox*);

    USHORT              GetPresetImageId( USHORT nValueSetIdx ) const;
    USHORT              GetPresetStringId( USHORT nValueSetIdx ) const;

    void                FillPresetVS();
    void                FillShadowVS();
    void                FillValueSets();

    // Filler
    void                FillLineListBox_Impl();

    // Setzen von einzelnen Frame-/Core-Linien
    void                ResetFrameLine_Impl( svx::FrameBorderType eBorder,
                                             const SvxBorderLine* pCurLine,
                                             bool bValid );
#endif
};


#endif

