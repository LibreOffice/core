/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backgrnd.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:05:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_BACKGRND_HXX
#define _SVX_BACKGRND_HXX

// include ---------------------------------------------------------------

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SVX_DLG_CTRL_HXX
#include <dlgctrl.hxx>
#endif

// Absatzvorlage
#define PARA_DEST_PARA  0
#define PARA_DEST_CHAR  1

// class SvxBackgroundTabPage --------------------------------------------
/*
{k:\svx\prototyp\dialog\backgrnd.bmp}
    [Beschreibung]
    Mit dieser TabPage kann eine Brush (z.B. fuer die Hintergrundfarbe eines
    Rahmens) eingestellt werden.
    [Items]
    <SvxBrushItem>:     <SID_ATTR_BRUSH>;
*/

//------------------------------------------------------------------------
// forwards:

class BackgroundPreviewImpl;
class SvxOpenGraphicDialog;
struct SvxBackgroundTable_Impl;
struct SvxBackgroundPara_Impl;
struct SvxBackgroundPage_Impl;
class SvxBrushItem;
//------------------------------------------------------------------------

class SvxBackgroundTabPage : public SvxTabPage
{
    using TabPage::DeactivatePage;
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        FillUserData();
    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

    void                ShowSelector(); // Umschalt-ListBox aktivieren
    void                ShowTblControl(); // fuer den Writer (Zellen/Zeilen/Tabelle)
    void                ShowParaControl(BOOL bCharOnly = FALSE); // fuer den Writer (Absatz/Zeichen)
    void                EnableTransparency(BOOL bColor, BOOL bGraphic);
    virtual void        PageCreated (SfxAllItemSet aSet); //add CHINA001
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

private:
    SvxBackgroundTabPage( Window* pParent, const SfxItemSet& rCoreSet );
    ~SvxBackgroundTabPage();

    // Hintergrundfarbe ------------------------------------
    Control                 aBorderWin;
    ValueSet                aBackgroundColorSet;
    FixedLine               aBackgroundColorBox;
    BackgroundPreviewImpl*  pPreviewWin1;
    //color transparency
    FixedText               aColTransFT;
    MetricField             aColTransMF;
    // Hintergrund-Bitmap ----------------------------------
    PushButton              aBtnBrowse;
    CheckBox                aBtnLink;
    CheckBox                aBtnPreview;
    FixedInfo               aFtFile;
    FixedLine               aGbFile;
    RadioButton             aBtnPosition;
    RadioButton             aBtnArea;
    RadioButton             aBtnTile;
    SvxRectCtl              aWndPosition;
    FixedLine               aGbPosition;
    //transparency of graphics
    FixedLine               aGraphTransFL;
    MetricField             aGraphTransMF;

    BackgroundPreviewImpl*  pPreviewWin2;
    // Selektor --------------------------------------------
    FixedText               aSelectTxt;
    ListBox                 aLbSelect;
    const String            aStrBrowse;
    const String            aStrUnlinked;

    // DDListBox fuer Writer -------------------------------

    FixedText               aTblDesc;
    ListBox                 aTblLBox;
    ListBox                 aParaLBox;

    //------------------------------------------------------
    Color       aBgdColor;
    USHORT      nHtmlMode;
    BOOL        bAllowShowSelector  : 1;
    BOOL        bIsGraphicValid     : 1;
    BOOL        bLinkOnly           : 1;
    BOOL        bResized            : 1;
    BOOL        bColTransparency    : 1;
    BOOL        bGraphTransparency  : 1;
    Graphic     aBgdGraphic;
    String      aBgdGraphicPath;
    String      aBgdGraphicFilter;

    SvxBackgroundPage_Impl* pPageImpl;
    SvxOpenGraphicDialog* pImportDlg;

    // Items fuer Sw-Tabelle muessen gesammelt werden
    SvxBackgroundTable_Impl*    pTableBck_Impl;
    // auch fuer die Absatzvorlage
    SvxBackgroundPara_Impl* pParaBck_Impl;

#ifdef _SVX_BACKGRND_CXX
    void                FillColorValueSets_Impl();
    void                ShowColorUI_Impl();
    void                ShowBitmapUI_Impl();
    BOOL                LoadLinkedGraphic_Impl();
    void                RaiseLoadError_Impl();
    void                SetGraphicPosition_Impl( SvxGraphicPosition ePos );
    SvxGraphicPosition  GetGraphicPosition_Impl();
    void                FillControls_Impl(const SvxBrushItem& rBgdAttr,
                                            const String& rUserData);
    BOOL                FillItemSetWithWallpaperItem( SfxItemSet& rCoreSet, USHORT nSlot);
    void                ResetFromWallpaperItem( const SfxItemSet& rSet );

    DECL_LINK( LoadTimerHdl_Impl, Timer* );
    DECL_LINK( SelectHdl_Impl, ListBox* );
    DECL_LINK( BrowseHdl_Impl, PushButton* );
    DECL_LINK( RadioClickHdl_Impl, RadioButton* );
    DECL_LINK( FileClickHdl_Impl, CheckBox* );
    DECL_LINK( BackgroundColorHdl_Impl, ValueSet* );
    DECL_LINK( TblDestinationHdl_Impl, ListBox* );
    DECL_LINK( ParaDestinationHdl_Impl, ListBox* );
#endif
};

#endif // #ifndef _SVX_BACKGRND_HXX


