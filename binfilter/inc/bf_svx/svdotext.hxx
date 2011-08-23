/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVDOTEXT_HXX
#define _SVDOTEXT_HXX

#include <vcl/field.hxx>

#include <bf_svx/svdoattr.hxx>

#include <bf_svx/svdtrans.hxx> // GeoStat

#include <tools/datetime.hxx>

#include <bf_svx/xtxasit.hxx>

#include <bf_svx/xtextit0.hxx>

#include <vector>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SvxFieldItem;
class ImpSdrObjTextLink;

//************************************************************
//   Defines
//************************************************************

#define SDRUSERDATA_OBJTEXTLINK			(1)

//************************************************************
//   Hilfsklasse SdrTextObjGeoData
//************************************************************

class SdrTextObjGeoData : public SdrObjGeoData
{
public:
    Rectangle					aRect;
    GeoStat						aGeo;
};

//************************************************************
//   Hilfsklasse ImpSdrObjTextLinkUserData
//************************************************************

class ImpSdrObjTextLinkUserData : public SdrObjUserData
{
    friend class				SdrTextObj;
    friend class				ImpSdrObjTextLink;

    SdrTextObj*					pObj;
    String						aFileName;   // Name des referenzierten Dokuments
    String						aFilterName; // ggf. ein Filter
    DateTime					aFileDate0;  // Unnoetiges neuladen vermeiden
    ImpSdrObjTextLink*			pLink;
    rtl_TextEncoding			eCharSet;

public:
    TYPEINFO();
    ImpSdrObjTextLinkUserData(SdrTextObj* pObj1);
    virtual ~ImpSdrObjTextLinkUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const;
    virtual void WriteData(SvStream& rOut);
    virtual void ReadData(SvStream& rIn);
    virtual void AfterRead();
};

//************************************************************
//   SdrTextObj
//************************************************************

class SdrTextObj : public SdrAttrObj
{
    friend class				ImpTextPortionHandler;
    friend class				ImpSdrObjTextLink;
    friend class				ImpSdrObjTextLinkUserData;
    friend class				SdrPowerPointImport; // fuer PowerPointImport
    friend class				SdrExchangeView; // fuer ImpGetDrawOutliner
    friend class				SdrView;         // fuer ImpGetDrawOutliner
    friend class				SdrObjEditView;  // fuer TextEdit
    friend class				SdrMeasureObj;   // fuer ImpGetDrawOutliner
    friend class				SvxMSDffManager; // fuer ImpGetDrawOutliner

protected:
    // Das aRect ist gleichzeig auch das Rect vom RectObj und CircObj.
    // Bei bTextFrame=TRUE wird der Text in dieses Rect hineinformatiert.
    // Bei bTextFrame=FALSE wird der Text am Mittelpunkt des Rect zentriert.
    Rectangle					aRect;

    // Der GeoStat enthaelt den Drehwinkel und einen Shearwinkel
    GeoStat						aGeo;

    // Im pOutlinerParaObject steckt der Text drin
    OutlinerParaObject*			pOutlinerParaObject;

    // Hier merke ich mir die Ausmasse des Textes (n.i.)
    Size						aTextSize;

    // Ein Outliner*, damit
    // 1. das TextObj nicht von mehreren Views gleichzeitig editiert und
    // 2. beim Streamen waerend des Editierens ein Flush() ausgefuehrt
    // werden kann
    SdrOutliner*   				pEdtOutl;

    // Bei Fontwork muss soviel auf's BoundRect draufgerechnet werden
    // damit es ausreichend gross ist.
    Rectangle*					pFormTextBoundRect;

    // Moegliche Werte fuer eTextKind sind:
    //     OBJ_TEXT         normaler Textrahmen
    //     OBJ_TEXTEXT      Textfortsetzungsrahmen
    //     OBJ_TITLETEXT    TitleText fuer StarDraw-Praesentation
    //     OBJ_OUTLINETEXT  OutlineText fuer StarDraw-Praesentation
    // eTextKind hat nur Bedeutung, wenn bTextFrame=TRUE, da es sich sonst
    // um ein beschriftetes Grafikobjekt handelt.
    SdrObjKind					eTextKind;

    // #108784#
    // For text editing in SW Haeder/Footer it is necessary to be
    // able to set an offset for the text edit to allow text editing at the
    // position of the virtual object. This offset is used when setting up
    // and maintaining the OutlinerView.
    Point						maTextEditOffset;
public:
    const Point& GetTextEditOffset() const { return maTextEditOffset; }
    void SetTextEditOffset(const Point& rNew) { maTextEditOffset = rNew; }

protected:
    // Fuer beschriftete Zeichenobjekte ist bTextFrame=FALSE. Der Textblock
    // wird dann hoizontal und vertikal an aRect zentriert. Bei bTextFrame=
    // TRUE wird der Text in aRect hineinformatiert. Der eigentliche Textrahmen
    // ist durch ein SdrRectObj mit bTextFrame=TRUE realisiert.
    BOOL						bTextFrame : 1;
    BOOL						bPortionInfoChecked : 1; // Fuer Optimierung von Textobjekten
    BOOL						bNoShear : 1;            // Obj darf nicht gesheart werden   (->Graf+Ole+TextFrame)
    BOOL						bNoRotate : 1;           // Obj darf nicht gedreht werden    (->Ole)
    BOOL						bNoMirror : 1;           // Obj darf nicht gespiegelt werden (->Ole,TextFrame)
    BOOL						bTextSizeDirty : 1;

    // #101684#
    BOOL						mbInEditMode : 1;	// Is this text obejct in edit mode?

    // Fuer Objekt mit freier Groesse im Draw (Mengentext). Das Flag wird vom
    // der App beim Create gesetzt.
    // Wenn das Objekt dann spaeter in der Breite resized wird, wird
    // AutoGrowWidth abgeschaltet (Hart auf FALSE attributiert). Das Flag wird
    // dann ebenfalls auf FALSE gesetzt, sodass sich das Objekt anschliessend
    // wie ein normales Textobjekt verhaelt.
    // Resize in der Breite kann sein:
    // - Interaktives Resize in Einfach- oder Mehrfachselektion
    // - Positions+Groesse Dialog
    BOOL bDisableAutoWidthOnDragging : 1;

private:
    void ImpCheckMasterCachable();
    // #101029#: Extracted from ImpGetDrawOutliner()
    void ImpInitDrawOutliner( SdrOutliner& rOutl ) const;
    // #101029#: Extracted from Paint()
    SdrOutliner& ImpGetDrawOutliner() const;
    void ImpLinkAnmeldung();
    void ImpLinkAbmeldung();
    ImpSdrObjTextLinkUserData* GetLinkUserData() const;
//	void ImpCheckItemSetChanges(const SfxItemSet& rAttr);

protected:
    FASTBOOL ImpCanConvTextToCurve() const { return pOutlinerParaObject!=NULL && pModel!=NULL && !IsOutlText() && !IsFontwork(); }
    void ImpConvertSetAttrAndLayer(SdrObject* pObj, FASTBOOL bNoSetAttr=FALSE) const;
    SdrObject* ImpConvertMakeObj(const XPolyPolygon& rXPP, FASTBOOL bClosed, FASTBOOL bBezier, FASTBOOL bNoSetAttr=FALSE) const;
    SdrObject* ImpConvertAddText(SdrObject* pObj, FASTBOOL bBezier) const;
    void ImpSetTextStyleSheetListeners();
    void ImpAddTextToBoundRect();
    void ImpJustifyRect(Rectangle& rRect) const;
    void ImpCheckShear();
    void SetTextSizeDirty() { bTextSizeDirty=TRUE; }

    // rAnchorRect ist InOut-Parameter!

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);
    FASTBOOL NbcSetEckenradius(long nRad);
    FASTBOOL NbcSetAutoGrowHeight(FASTBOOL bAuto);
    FASTBOOL NbcSetMinTextFrameHeight(long nHgt);
    FASTBOOL NbcSetMinTextFrameWidth(long nWdt);

    // Konstruktoren fuer beschriftete Zeichenobjekte
    SdrTextObj();
    SdrTextObj(const Rectangle& rNewRect);

    // Konstruktoren fuer Textrahmen
    SdrTextObj(SdrObjKind eNewTextKind);
    SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect);

    virtual ~SdrTextObj();

public:
    TYPEINFO();

    // #101684#
    BOOL IsInEditMode() const { return mbInEditMode; }

    // via eCharSet kann der CharSet der vorliegenden Datei uebergeben werden.
    // Bei RTL_TEXTENCODING_DONTKNOW wird der CharSet der aktuellen Plattform verwendet.
    // Derzeit unterstuetzt wird ASCII und RTF wobei ich die Unterscheidung
    // selbst treffe. rFilterName ist noch ohne Bedeutung und muss leer gelassen
    // werden.
    // I.d.R. muss die App nur SetTextLink() rufen - der Rest geschieht von
    // selbst (SfxLinkManager). Die Methode LoadText() kann ausserdem verwendet
    // werden, um eine Datei in ein Textobjekt zu laden (ohne Verknuepfung).
    // TextLinks koennen nicht editiert werden (allenfalls spaeter mal ReadOnly).
    // Eine Attributierung kann nur am Textrahmen vollzogen werden.
    FASTBOOL IsLinkedText() const { return pPlusData!=NULL && GetLinkUserData()!=NULL; }

    FASTBOOL AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt=TRUE, FASTBOOL bWdt=TRUE) const;
    FASTBOOL NbcAdjustTextFrameWidthAndHeight(FASTBOOL bHgt=TRUE, FASTBOOL bWdt=TRUE);
    FASTBOOL AdjustTextFrameWidthAndHeight(FASTBOOL bHgt=TRUE, FASTBOOL bWdt=TRUE);
    FASTBOOL IsTextFrame() const { return bTextFrame; }
    FASTBOOL IsOutlText() const { return bTextFrame && (eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT); }
    BOOL GetTextKind() const { return eTextKind; }
    FASTBOOL HasText() const { return pEdtOutl==NULL ? pOutlinerParaObject!=NULL : HasEditText(); }
    FASTBOOL HasEditText() const;
    FASTBOOL IsTextEditActive() const { return pEdtOutl!=NULL; }

    /** returns true only if we are in edit mode and the user actually changed anything */
    bool IsRealyEdited(){DBG_BF_ASSERT(0, "STRIP"); return false;}//STRIP001 bool IsRealyEdited() const;

    void SetDisableAutoWidthOnDragging(FASTBOOL bOn) { bDisableAutoWidthOnDragging=bOn; }
    FASTBOOL IsDisableAutoWidthOnDragging() { return bDisableAutoWidthOnDragging; }

    // FitToSize und Fontwork wird bei GetTextSize() nicht berueksichtigt!
    virtual const Size& GetTextSize() const;

    // Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
    // der des EditOutliners) und die PaperSize gesetzt.
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText=FALSE,
        Rectangle* pAnchorRect=NULL, BOOL bLineWidth=TRUE ) const;
    virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;
    const GeoStat& GetGeoStat() const { return aGeo; }
    inline long GetEckenradius() const;
    FASTBOOL IsAutoGrowHeight() const;
    inline long GetMinTextFrameHeight() const;
    inline long GetMaxTextFrameHeight() const;
    FASTBOOL IsAutoGrowWidth() const;
    inline long GetMinTextFrameWidth() const;
    inline long GetMaxTextFrameWidth() const;
    SdrFitToSizeType GetFitToSize() const;

    // Feststellen, ob TextFontwork
    inline FASTBOOL IsFontwork() const;

    // Soll die Fontwork-Kontour versteckt werden?
    inline FASTBOOL IsHideContour() const;

    // Textfluss innerhalb Kontur
    inline FASTBOOL IsContourTextFrame() const;

    // Horizontale Textausrichtung
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    // Vertikale Textausrichtung
    SdrTextVertAdjust GetTextVerticalAdjust() const;

    // Textrahmenabstaende
    inline long GetTextLeftDistance() const;
    inline long GetTextRightDistance() const;
    inline long GetTextUpperDistance() const;
    inline long GetTextLowerDistance() const;
    inline SdrTextAniKind GetTextAniKind() const;
    inline SdrTextAniDirection GetTextAniDirection() const;

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pNewModel);

    // Wird zur Bestimmung des Textankerbereichs benoetigt
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual void operator=(const SdrObject& rObj);
    virtual void TakeContour(XPolyPolygon& rPoly) const;
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual const Rectangle& GetLogicRect() const;
    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(FASTBOOL bVertical=FALSE) const;




    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

    virtual FASTBOOL HasTextEdit() const;
    virtual FASTBOOL BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);
    virtual SdrObject* CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;

    FASTBOOL IsTextAnimated() const { return GetTextAniKind()!=SDRTEXTANI_NONE; }

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    OutlinerParaObject* GetEditOutlinerParaObject() const;

    virtual void NbcReformatText();
    virtual void ReformatText();
    virtual void RestartAnimation(SdrPageView* pPageView) const;

    virtual FASTBOOL CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
        FASTBOOL bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;


    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemSetChanged(const SfxItemSet& rSet);
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);


    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    void SetTextEditOutliner(SdrOutliner* pOutl) { pEdtOutl=pOutl; }

    /** Setup given Outliner equivalently to SdrTextObj::Paint()

        To setup an arbitrary Outliner in the same way as the draw
        outliner on SdrTextObj::Paint(). Among others, the paper size,
        control word and character stretching are initialized, such
        that the formatting should match the screen representation.
        The textual content of the outliner is not touched, i.e. no
        Init() or Clear() is called on the Outliner.

        @param rOutl
        The Outliner to setup.

        @param rPaintRect
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void SetupOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const;

    /** Update given Outliner equivalently to SdrTextObj::Paint()

        Same functionality as in SetupOutlinerFormatting(), except
        that the outliner content is not cleared.

        @param rOutl
        The Outliner to update.

        @param rPaintRect
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void UpdateOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ){DBG_BF_ASSERT(0, "STRIP");}//STRIP001 void UpdateOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const;

    // pre- and postprocessing for objects for saving
    virtual void PreSave();

    void ForceOutlinerParaObject();
    BOOL IsVerticalWriting() const;
    void SetVerticalWriting( BOOL bVertical );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual BOOL TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon);

    // #103836# iterates over the paragraphs of a given SdrObject and removes all
    //			hard set character attributes with the which ids contained in the 
    //			given vector
    void RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds );
};

//************************************************************
//   Inlines
//************************************************************

inline long SdrTextObj::GetEckenradius() const
{
    return ((SdrEckenradiusItem&)(GetItemSet().Get(SDRATTR_ECKENRADIUS))).GetValue();
}

inline long SdrTextObj::GetMinTextFrameHeight() const
{
    return ((SdrTextMinFrameHeightItem&)(GetItemSet().Get(SDRATTR_TEXT_MINFRAMEHEIGHT))).GetValue();
}

inline long SdrTextObj::GetMaxTextFrameHeight() const
{
    return ((SdrTextMaxFrameHeightItem&)(GetItemSet().Get(SDRATTR_TEXT_MAXFRAMEHEIGHT))).GetValue();
}

inline long SdrTextObj::GetMinTextFrameWidth() const
{
    return ((SdrTextMinFrameWidthItem&)(GetItemSet().Get(SDRATTR_TEXT_MINFRAMEWIDTH))).GetValue();
}

inline long SdrTextObj::GetMaxTextFrameWidth() const
{
    return ((SdrTextMaxFrameWidthItem&)(GetItemSet().Get(SDRATTR_TEXT_MAXFRAMEWIDTH))).GetValue();
}

inline FASTBOOL SdrTextObj::IsFontwork() const
{
    return (bTextFrame) ? FALSE // Default ist FALSE
        : ((XFormTextStyleItem&)(GetItemSet().Get(XATTR_FORMTXTSTYLE))).GetValue()!=XFT_NONE;
}

inline FASTBOOL SdrTextObj::IsHideContour() const
{
    return (bTextFrame) ? FALSE // Default ist: Nein, kein HideContour; HideContour nicht bei TextFrames
        : ((XFormTextHideFormItem&)(GetItemSet().Get(XATTR_FORMTXTHIDEFORM))).GetValue();
}

inline FASTBOOL SdrTextObj::IsContourTextFrame() const
{
    return (bTextFrame) ? FALSE // ContourFrame nicht bei normalen TextFrames
        : ((SdrTextContourFrameItem&)(GetItemSet().Get(SDRATTR_TEXT_CONTOURFRAME))).GetValue();
}

inline long SdrTextObj::GetTextLeftDistance() const
{
    return ((SdrTextLeftDistItem&)(GetItemSet().Get(SDRATTR_TEXT_LEFTDIST))).GetValue();
}

inline long SdrTextObj::GetTextRightDistance() const
{
    return ((SdrTextRightDistItem&)(GetItemSet().Get(SDRATTR_TEXT_RIGHTDIST))).GetValue();
}

inline long SdrTextObj::GetTextUpperDistance() const
{
    return ((SdrTextUpperDistItem&)(GetItemSet().Get(SDRATTR_TEXT_UPPERDIST))).GetValue();
}

inline long SdrTextObj::GetTextLowerDistance() const
{
    return ((SdrTextLowerDistItem&)(GetItemSet().Get(SDRATTR_TEXT_LOWERDIST))).GetValue();
}

inline SdrTextAniKind SdrTextObj::GetTextAniKind() const
{
    return ((SdrTextAniKindItem&)(GetItemSet().Get(SDRATTR_TEXT_ANIKIND))).GetValue();
}

inline SdrTextAniDirection SdrTextObj::GetTextAniDirection() const
{
    return ((SdrTextAniDirectionItem&)(GetItemSet().Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
