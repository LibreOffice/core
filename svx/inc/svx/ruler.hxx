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
#ifndef _SVX_RULER_HXX
#define _SVX_RULER_HXX

#include <vcl/menu.hxx>
#include <svtools/ruler.hxx>
#include <svl/lstner.hxx>
#include "svx/svxdllapi.h"

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

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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

