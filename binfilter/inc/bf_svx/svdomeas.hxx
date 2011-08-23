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

#ifndef _SVDOMEAS_HXX
#define _SVDOMEAS_HXX

#ifndef _SVDOTEXT_HXX
#include <bf_svx/svdotext.hxx>
#endif
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrMeasureSetItem;
class SdrOutliner;
struct ImpMeasureRec;
struct ImpMeasurePoly;

//************************************************************
//   Hilfsklasse SdrMeasureObjGeoData
//************************************************************

class SdrMeasureObjGeoData : public SdrTextObjGeoData
{
public:
    Point						aPt1;
    Point						aPt2;

public:
    SdrMeasureObjGeoData();
    virtual ~SdrMeasureObjGeoData();
};

//************************************************************
//   SdrMeasureObj
//************************************************************

class SdrMeasureObj : public SdrTextObj
{
    friend class				SdrMeasureField;

protected:
    Point						aPt1;
    Point						aPt2;
    FASTBOOL					bTextDirty;

protected:
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void ForceDefaultAttr();
    void ImpTakeAttr(ImpMeasureRec& rRec) const;
    void ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const;
    void ImpCalcXPoly(const ImpMeasurePoly& rPol, XPolyPolygon& rXPP) const;
    void SetTextDirty() { bTextDirty=TRUE; SetTextSizeDirty(); if (!bBoundRectDirty) { bBoundRectDirty=TRUE; SetRectsDirty(TRUE); } }
    void UndirtyText() const;


public:
    TYPEINFO();
    SdrMeasureObj();
    SdrMeasureObj(const Point& rPt1, const Point& rPt2);
    virtual ~SdrMeasureObj();

    virtual UINT16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;






    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual long GetRotateAngle() const;
    virtual void RecalcBoundRect();
    virtual void RecalcSnapRect();


    virtual const Point& GetPoint(USHORT i) const;
    virtual void NbcSetPoint(const Point& rPnt, USHORT i);


    virtual FASTBOOL BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);
    virtual const Size& GetTextSize() const;
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText=FALSE,
        Rectangle* pAnchorRect=NULL, BOOL bLineWidth=TRUE ) const;
    virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;
    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual FASTBOOL CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
        FASTBOOL bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;

    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);

    // ItemSet access
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    // pre- and postprocessing for objects for saving
    virtual void PreSave();
    virtual void PostSave();

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Creating:
// ~~~~~~~~~
// Dragging von Bezugspunkt 1 zu Bezugspunkt 2 -> Bezugskante
//
// Die Defaults:
// ~~~~~~~~~~~~~
// Masslinie und Masshilfslinien: Haarlinien solid schwarz
// Pfeile:     2mm x 4mm
// Textgroesse
//                              ___
//     ?       Masszahl       ?2mm
//     ?--------------------->?--
//     ?                      ?8mm
//     ?                      ?
//    Pt1ออออออออออออ?       Pt2-- <----Bezugskante (von Pt1 nach Pt2)
//     ?            ?        ณ___ <- Ueberstand der Masshilfslinie(n)
//     ?            ศอออออออออ?
//     ?Zu bemassendes Objekt ?
//     ศอออออออออออออออออออออออ?
//
// Attribute:
// ~~~~~~~~~~
// 1. Wo steht der Text: mitte, rechts oder links (def=automatik)
// 2. Text oberhalb der Linie oder unterhalb oder Linie unterbrochen durch Text (def=automatik)
// 3. Den Abstand der Masslinie zur Bezugskante (=zum bemassten Objekt).
//    Default=8mm
// 4. Masslinie unterhalb der Bezugskante (default=nein)
// 5. Die Ueberlaenge(n) der Masshilfslinien ueber die Bezugskante (2x, default=0)
// 6. Den Ueberhang der Masshilfslinien ueber die Masslinie (default=2mm)
// 7. Den Abstand der Masshilfslinien zur Bezugskante
//
// Dragging:                    Handle          Shift
// ~~~~~~~~~
// -  Die Bezugspunkte        SolidQuadHdl   nur die Laenge
// 1.+2. Anpacken des Textes
// 3.+4. Hdl am Pfeil (2x)    SolidQuadHdl   nur den Bool
// 5.    Hdl am Endpunkt      CircHdl        beide Laengen?
// 6.+7. Kein Dragging
//
// Offen:
// ~~~~~~
// - Radien (gleich als Typ verankern
//
// Special:
// ~~~~~~~~
// Connecting an max. 2 Objekte
// -> Bei Copy, etc. den entspr. Code der Verbinder verwenden?!?
// wird wohl recht kompliziert werden ...
//
/////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOMEAS_HXX

