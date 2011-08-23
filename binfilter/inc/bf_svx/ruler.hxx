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


#ifndef _RULER_HXX //autogen
#include <bf_svtools/ruler.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <bf_svtools/lstner.hxx>
#endif
class SfxRectangleItem;
class SfxBoolItem;
namespace binfilter {

class SvxProtectItem;
class SvxRulerItem;
class SfxBindings;
class SvxLongLRSpaceItem;
class SvxLongULSpaceItem;
class SvxTabStopItem;
class SvxLRSpaceItem;
class SvxPagePosSizeItem;
class SvxColumnItem;
class SvxObjectItem;
struct SvxRuler_Impl;

class SvxRuler: public Ruler, public SfxListener
{
friend class SvxRulerItem;
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
#if _SOLAR__PRIVATE
    void StartListening_Impl();

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
    void Update(const SvxColumnItem *);
    // Object Selektion
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


    long ConvertHPosPixel(long lPos) const;
    long ConvertVPosPixel(long lPos) const;
    long ConvertHSizePixel(long lSize) const;
    long ConvertVSizePixel(long lSize) const;

    long ConvertPosPixel(long lPos) const;
    long ConvertSizePixel(long lSize) const;



//  long GetLogicFirstLineIndent() const;
//  long GetLogicLeftIndent() const;

    inline long GetLeftFrameMargin() const;
    long GetRightFrameMargin() const;







#endif

    void Update();

enum UpdateType {
    MOVE_ALL,
    MOVE_LEFT,
    MOVE_RIGHT
    };
protected:
    virtual void	MouseMove( const MouseEvent& rMEvt );

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );

    // Berechnung der Grenzwerte fuer Objectgrenzen
    // Werte sind bezogen auf die Seite
    BOOL IsActLastColumn(
        BOOL bForceDontConsiderHidden = FALSE, USHORT nAct=USHRT_MAX) const ;

public:
#define     SVXRULER_SUPPORT_TABS                       0x0001
#define     SVXRULER_SUPPORT_PARAGRAPH_MARGINS          0x0002
#define     SVXRULER_SUPPORT_BORDERS                    0x0004
#define     SVXRULER_SUPPORT_OBJECT                     0x0008
#define     SVXRULER_SUPPORT_SET_NULLOFFSET             0x0010
#define     SVXRULER_SUPPORT_NEGATIVE_MARGINS           0x0020
#define     SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL 0x0040

    SvxRuler(Window* pParent,
             Window *pEditWin,
             USHORT nRulerFlags,
             SfxBindings &rBindings,
             WinBits nWinStyle = WB_STDRULER);
    ~SvxRuler();



    // Setzen / Erfragen NullOffset in logischen Einheiten

    void SetActive(BOOL bOn = TRUE);

    void ForceUpdate() { Update(); }
};

}//end of namespace binfilter
#endif

