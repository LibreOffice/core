/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ruler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:48:39 $
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
#ifndef _SVX_RULER_HXX
#define _SVX_RULER_HXX

#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#ifndef _RULER_HXX //autogen
#include <svtools/ruler.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvxProtectItem;
class SvxRulerItem;
class SfxBindings;
class SvxLongLRSpaceItem;
class SvxLongULSpaceItem;
class SvxTabStopItem;
class SvxLRSpaceItem;
class SvxPagePosSizeItem;
class SvxColumnItem;
class SfxRectangleItem;
class SvxObjectItem;
class SfxBoolItem;
struct SvxRuler_Impl;

class SVX_DLLPUBLIC SvxRuler: public Ruler, public SfxListener
{
    friend class SvxRulerItem;
    using Window::Notify;

    SvxRulerItem **pCtrlItem;
    SvxLongLRSpaceItem *pLRSpaceItem;  //Linker und Rechter Rand
    SfxRectangleItem *pMinMaxItem; //Maxima fuers Draggen
    SvxLongULSpaceItem *pULSpaceItem;  //Oberer und Unterer Rand
    SvxTabStopItem *pTabStopItem;  //Tabstops
    SvxLRSpaceItem *pParaItem;     //Absaetze
    SvxLRSpaceItem *pParaBorderItem; //Border distance
    SvxPagePosSizeItem *pPagePosItem; //Seitenabstaende zum Lineal
    SvxColumnItem *pColumnItem;    //Spalten
    SvxObjectItem *pObjectItem;    //Objekt
    Window *pEditWin;
    SvxRuler_Impl *pRuler_Imp;
    BOOL bAppSetNullOffset :1;
    BOOL bHorz :1;
    long lLogicNullOffset;      // in logischen Koordinaten
    long lAppNullOffset;        // in logischen Koordinaten
    long lMinFrame;             // minimale Framebreite in Pixel
    long lInitialDragPos;
    USHORT nFlags;
    enum {
        NONE = 0x0000,
        DRAG_OBJECT =  0x0001,
        // letzte Spalte verkleinern, Shift
        DRAG_OBJECT_SIZE_LINEAR = 0x0002,
        DRAG_OBJECT_SIZE_PROPORTIONAL = 0x0004, // proportional, Ctrl
        // nur aktuelle Zeile (Tabelle; Shift-Ctrl)
        DRAG_OBJECT_ACTLINE_ONLY = 0x0008,
        // aktuell gleiche Tastenbelegung
        DRAG_OBJECT_LEFT_INDENT_ONLY = DRAG_OBJECT_SIZE_PROPORTIONAL
        }
    nDragType;
    USHORT nDefTabType;
    USHORT nTabCount;
    USHORT nTabBufSize;
    long lDefTabDist;
    long lTabPos;
    RulerTab *pTabs;            // Tabpositionen in Pixel
    RulerIndent *pIndents;      // Absatzraender in Pixel
    RulerBorder *pBorders;
    USHORT nBorderCount;
    RulerBorder *pObjectBorders;
    SfxBindings *pBindings;
    long nDragOffset;
    long nMaxLeft;
    long nMaxRight;
    BOOL bValid;
    BOOL bListening;
    BOOL bActive;
//#if 0 // _SOLAR__PRIVATE
    void StartListening_Impl();
    long GetCorrectedDragPos( BOOL bLeft = TRUE, BOOL bRight = TRUE );
    void DrawLine_Impl(long &lTabPos, int, BOOL Hori=TRUE);
    USHORT GetObjectBordersOff(USHORT nIdx) const;

    // Seitenr"ander oder umgebender Rahmen
    void UpdateFrame(const SvxLongLRSpaceItem *);
    void UpdateFrame(const SvxLongULSpaceItem *);
    void UpdateFrameMinMax(const SfxRectangleItem *);
    // Absatzeinzuege
    void UpdatePara(const SvxLRSpaceItem *);
    // Border distance
    void UpdateParaBorder(const SvxLRSpaceItem *);
    // Tabs
    void Update(const SvxTabStopItem *);
    // Seitenposition und -breite
    void Update(const SvxPagePosSizeItem *);
    // Spalten
    void Update(const SvxColumnItem *, USHORT nSID);
    // Object Selektion
    void Update(const SvxObjectItem *);
    // Protect
    void Update( const SvxProtectItem* );
    // left-to-right text
    void UpdateTextRTL( const SfxBoolItem* );
    // Absatzeinzuege
    void UpdatePara();
    void UpdateTabs();
    void UpdatePage();
    void UpdateFrame();
    void UpdateColumns();
    void UpdateObject();

    long PixelHAdjust(long lPos, long lPos2) const;
    long PixelVAdjust(long lPos, long lPos2) const;
    long PixelAdjust(long lPos, long lPos2) const;

    long ConvertHPosPixel(long lPos) const;
    long ConvertVPosPixel(long lPos) const;
    long ConvertHSizePixel(long lSize) const;
    long ConvertVSizePixel(long lSize) const;

    long ConvertPosPixel(long lPos) const;
    long ConvertSizePixel(long lSize) const;

    long ConvertHPosLogic(long lPos) const;
    long ConvertVPosLogic(long lPos) const;
    long ConvertHSizeLogic(long lSize) const;
    long ConvertVSizeLogic(long lSize) const;

    long ConvertPosLogic(long lPos) const;
    long ConvertSizeLogic(long lSize) const;

    long GetFirstLineIndent() const;
//  long GetLogicFirstLineIndent() const;
    long GetLeftIndent() const;
//  long GetLogicLeftIndent() const;
    long GetRightIndent() const;
    long GetLogicRightIndent() const;
    long GetPageWidth() const;

    inline long GetLeftFrameMargin() const;
    long GetRightFrameMargin() const;

    void CalcMinMax();

    void EvalModifier();
    void DragMargin1();
    //adjust the left margin either after DragMargin1() or after DragBorders()
    void AdjustMargin1(long lDiff);
    void DragMargin2();
    void DragIndents();
    void DragTabs();
    void DragBorders();
    void DragObjectBorder();

    void ApplyMargins();
    void ApplyIndents();
    void ApplyTabs();
    void ApplyBorders();
    void ApplyObject();

    long GetFrameLeft() const;
    void SetFrameLeft(long);

    long GetLeftMin() const;
    long GetRightMax() const;

    DECL_LINK( TabMenuSelect, Menu * );
    DECL_LINK( MenuSelect, Menu * );
    void PrepareProportional_Impl(RulerType);

    USHORT GetNextVisible(USHORT nColumn);
    USHORT GetPrevVisible(USHORT nColumn);
//#endif

    void Update();

enum UpdateType {
    MOVE_ALL,
    MOVE_LEFT,
    MOVE_RIGHT
    };
    void UpdateParaContents_Impl(long lDiff, UpdateType = MOVE_ALL);
protected:
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    Click();
    virtual long    StartDrag();
    virtual void    Drag();
    virtual void    EndDrag();
    virtual void    ExtraDown();
    virtual void    MouseMove( const MouseEvent& rMEvt );

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );

    // Berechnung der Grenzwerte fuer Objectgrenzen
    // Werte sind bezogen auf die Seite
    virtual BOOL    CalcLimits(long &nMax1, long &nMax2, BOOL bFirst) const;
    BOOL IsActLastColumn(
        BOOL bForceDontConsiderHidden = FALSE, USHORT nAct=USHRT_MAX) const ;
    BOOL IsActFirstColumn(
        BOOL bForceDontConsiderHidden = FALSE, USHORT nAct=USHRT_MAX) const;
    USHORT GetActLeftColumn(
        BOOL bForceDontConsiderHidden = FALSE, USHORT nAct=USHRT_MAX ) const;
    USHORT GetActRightColumn (
        BOOL bForceDontConsiderHidden = FALSE, USHORT nAct=USHRT_MAX ) const;
    long CalcPropMaxRight(USHORT nCol=USHRT_MAX) const;

public:
#define     SVXRULER_SUPPORT_TABS                       0x0001
#define     SVXRULER_SUPPORT_PARAGRAPH_MARGINS          0x0002
#define     SVXRULER_SUPPORT_BORDERS                    0x0004
#define     SVXRULER_SUPPORT_OBJECT                     0x0008
#define     SVXRULER_SUPPORT_SET_NULLOFFSET             0x0010
#define     SVXRULER_SUPPORT_NEGATIVE_MARGINS           0x0020
#define     SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL 0x0040
#define     SVXRULER_SUPPORT_REDUCED_METRIC             0x0080 //shorten the context menu to select metric

    SvxRuler(Window* pParent,
             Window *pEditWin,
             USHORT nRulerFlags,
             SfxBindings &rBindings,
             WinBits nWinStyle = WB_STDRULER);
    ~SvxRuler();

    void SetMinFrameSize(long lSize);
    long GetMinFrameSize() const ;

    USHORT GetRulerFlags() const { return nFlags; }

    void SetDefTabDist(long);
    long GetDefTabDist() const;

    // Setzen / Erfragen NullOffset in logischen Einheiten
    void SetNullOffsetLogic(long lOff = 0);
    long GetNullOffsetLogic() const { return lAppNullOffset; }

    void SetActive(BOOL bOn = TRUE);
    BOOL IsActive() const { return bActive; }

    void ForceUpdate() { Update(); }

    //#i24363# tab stops relative to indent
    void SetTabsRelativeToIndent( BOOL bRel = TRUE );
};

#endif

