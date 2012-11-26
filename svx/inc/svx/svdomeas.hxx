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



#ifndef _SVDOMEAS_HXX
#define _SVDOMEAS_HXX

#include <svx/svdotext.hxx>
#include "svx/svxdllapi.h"
#include <editeng/measfld.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrOutliner;
struct ImpMeasureRec;
struct ImpMeasurePoly;

namespace sdr { namespace properties {
    class MeasureProperties;
}}

//************************************************************
//   Hilfsklasse SdrMeasureObjGeoData
//************************************************************

class SdrMeasureObjGeoData : public SdrObjGeoData
{
public:
    Point                       aPt1;
    Point                       aPt2;

public:
    SdrMeasureObjGeoData();
    virtual ~SdrMeasureObjGeoData();
};

//************************************************************
//   SdrMeasureObj
//************************************************************

/*************************************************************************/
/* Measure                                                               */
/*************************************************************************/
//                             ___
//    |        100,00km       | 2mm = MeasureHelplineOverhang
//    |<--------------------->|---
//    |                       | 8mm = MeasureLineDist
//    |                       |
//    #=============#         |---
//    #             #         | MeasureHelpline1/2Len
//    #             #=========#---
//    # Zu bemassendes Objekt #
//    #=======================#

class SVX_DLLPUBLIC SdrMeasureObj : public SdrTextObj
{
private:
    // to allow sdr::properties::MeasureProperties access to SetTextDirty()
    friend class sdr::properties::MeasureProperties;
    friend class                SdrMeasureField;

    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    Point                       aPt1; // TTTT convert to B2DPoint
    Point                       aPt2;
    bool                        bTextDirty;

protected:
    void ImpTakeAttr(ImpMeasureRec& rRec) const;
    void TakeRepresentation( XubString& rStr, SdrMeasureFieldKind eMeasureFieldKind ) const;
    void ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const;
    basegfx::B2DPolyPolygon ImpCalcXPoly(const ImpMeasurePoly& rPol) const;
    void ImpEvalDrag(ImpMeasureRec& rRec, const SdrDragStat& rDrag) const;
    void SetTextDirty();
    void UndirtyText() const;

    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

    virtual ~SdrMeasureObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    SdrMeasureObj(
        SdrModel& rSdrModel,
        const Point& rPt1 = Point(),
        const Point& rPt2 = Point());

    virtual bool DoesSupportTextIndentingOnLineWidthChange() const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer(const SdrView& rSdrView) const;

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    virtual bool IsPolygonObject() const;
    virtual sal_uInt32 GetObjectPointCount() const;
    virtual basegfx::B2DPoint GetObjectPoint(sal_uInt32 i) const;
    virtual void SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i);

    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual const Size& GetTextSize() const;
    virtual void TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const;
    virtual basegfx::B2DRange getUnifiedTextRange() const;
    virtual void TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const;
    virtual sal_uInt16 GetOutlinerViewAnchorMode() const;
    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
        bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;
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
//     |        Masszahl       | 2mm
//     |<--------------------->|---
//     |                       | 8mm
//     |                       |
//    Pt1============#        Pt2-- <----Bezugskante (von Pt1 nach Pt2)
//     #             #         |___ <- Ueberstand der Masshilfslinie(n)
//     #             #=========#
//     # Zu bemassendes Objekt #
//     #=======================#
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

#endif //_SVDOMEAS_HXX

