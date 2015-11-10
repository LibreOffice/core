/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SVDOTEXT_HXX
#define INCLUDED_SVX_SVDOTEXT_HXX

#include <vcl/field.hxx>
#include <svx/itextprovider.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdtrans.hxx>
#include <tools/datetime.hxx>
#include <rsc/rscsfx.hxx>
#include <svx/xtextit0.hxx>
#include <svx/svdtext.hxx>
#include <vector>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/svdpagv.hxx>


//   Vorausdeklarationen


class OutlinerParaObject;
class OverflowingText;
class SdrOutliner;
class SdrTextObj;
class SdrTextObjTest;
class SvxFieldItem;
class ImpSdrObjTextLink;
class EditStatus;
class TextChain;
class TextChainFlow;


namespace sdr { namespace properties {
    class TextProperties;
}}

namespace drawinglayer { namespace primitive2d {
    class SdrContourTextPrimitive2D;
    class SdrPathTextPrimitive2D;
    class SdrBlockTextPrimitive2D;
    class SdrAutoFitTextPrimitive2D;
    class SdrStretchTextPrimitive2D;
    class SdrChainedTextPrimitive2D;
}}

namespace drawinglayer { namespace animation {
    class AnimationEntryList;
}}

namespace drawinglayer { namespace geometry {
    class ViewInformation2D;
}}

namespace sdr { namespace table {
    class Cell;
    class SdrTableRtfExporter;
    class SdrTableRTFParser;
}}


//   Defines


#define SDRUSERDATA_OBJTEXTLINK         (1)


//   Hilfsklasse SdrTextObjGeoData


class SdrTextObjGeoData : public SdrObjGeoData
{
public:
    Rectangle                   aRect;
    GeoStat                     aGeo;
};


//   Hilfsklasse ImpSdrObjTextLinkUserData


class ImpSdrObjTextLinkUserData : public SdrObjUserData
{
    friend class                SdrTextObj;
    friend class                ImpSdrObjTextLink;

    SdrTextObj*                 pObj;
    OUString                    aFileName;   // Name des referenzierten Dokuments
    OUString                    aFilterName; // ggf. ein Filter
    DateTime                    aFileDate0;  // Unnoetiges neuladen vermeiden
    ImpSdrObjTextLink*          pLink;
    rtl_TextEncoding            eCharSet;

public:
    ImpSdrObjTextLinkUserData(SdrTextObj* pObj1);
    virtual ~ImpSdrObjTextLinkUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const override;
};

namespace sdr
{
    namespace properties
    {
        class CustomShapeProperties;
        class CellProperties;
    } // end of namespace properties
} // end of namespace sdr


//   SdrTextObj


class SVX_DLLPUBLIC SdrTextObj : public SdrAttrObj, public svx::ITextProvider
{
private:
    // Cell needs access to ImpGetDrawOutliner();

    friend class                sdr::table::Cell;
    friend class                sdr::table::SdrTableRtfExporter;
    friend class                sdr::table::SdrTableRTFParser;

    friend class                TextChain;
    friend class                TextChainFlow;
    friend class                EditingTextChainFlow;


    // CustomShapeproperties need to access the "bTextFrame" member:
    friend class sdr::properties::CustomShapeProperties;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

private:
    // This method is only allowed for sdr::properties::TextProperties
    SVX_DLLPRIVATE SdrOutliner* GetTextEditOutliner() const
    {
        return pEdtOutl;
    }

    // This method is only allowed for sdr::properties::TextProperties
    SVX_DLLPRIVATE void SetPortionInfoChecked(bool bNew)
    {
        bPortionInfoChecked = bNew;
    }

    // to allow sdr::properties::TextProperties access to SetPortionInfoChecked()
    // and GetTextEditOutliner()
    friend class sdr::properties::TextProperties;
    friend class sdr::properties::CellProperties;

    friend class                ImpSdrObjTextLink;
    friend class                ImpSdrObjTextLinkUserData;
    friend class                SdrPowerPointImport; // fuer PowerPointImport
    friend class                SdrExchangeView; // fuer ImpGetDrawOutliner
    friend class                SdrView;         // fuer ImpGetDrawOutliner
    friend class                SdrObjEditView;  // fuer TextEdit
    friend class                SdrMeasureObj;   // fuer ImpGetDrawOutliner
    friend class                SvxMSDffManager; // fuer ImpGetDrawOutliner
    friend class                SdrObjCustomShape;// fuer ImpGetDrawOutliner
    friend class                SdrText;        // fuer ImpGetDrawOutliner

protected:
    // Das aRect ist gleichzeig auch das Rect vom RectObj und CircObj.
    // Bei bTextFrame=sal_True wird der Text in dieses Rect hineinformatiert.
    // Bei bTextFrame=sal_False wird der Text am Mittelpunkt des Rect zentriert.
    Rectangle maRect;

    // Der GeoStat enthaelt den Drehwinkel und einen Shearwinkel
    GeoStat                     aGeo;

    // this is the active text
    SdrText*                    mpText;

    // Hier merke ich mir die Ausmasse des Textes (n.i.)
    Size                        aTextSize;

    // Ein Outliner*, damit
    // 1. das TextObj nicht von mehreren Views gleichzeitig editiert und
    // 2. beim Streamen waerend des Editierens ein Flush() ausgefuehrt
    // werden kann
    SdrOutliner*                pEdtOutl;

    // Bei Fontwork muss soviel auf's BoundRect draufgerechnet werden
    // damit es ausreichend gross ist.
    Rectangle*                  pFormTextBoundRect;

    // Moegliche Werte fuer eTextKind sind:
    //     OBJ_TEXT         normaler Textrahmen
    //     OBJ_TEXTEXT      Textfortsetzungsrahmen
    //     OBJ_TITLETEXT    TitleText fuer StarDraw-Praesentation
    //     OBJ_OUTLINETEXT  OutlineText fuer StarDraw-Praesentation
    // eTextKind hat nur Bedeutung, wenn bTextFrame=sal_True, da es sich sonst
    // um ein beschriftetes Grafikobjekt handelt.
    SdrObjKind                  eTextKind;

    // For text editing in SW Header/Footer it is necessary to be
    // able to set an offset for the text edit to allow text editing at the
    // position of the virtual object. This offset is used when setting up
    // and maintaining the OutlinerView.
    Point                       maTextEditOffset;

    virtual SdrObject* getFullDragClone() const override;

public:
    const Point& GetTextEditOffset() const { return maTextEditOffset; }
    void SetTextEditOffset(const Point& rNew) { maTextEditOffset = rNew; }

protected:
    OverflowingText *mpOverflowingText = nullptr;
    bool mbIsUnchainableClone = false;

    // the successor in a chain
    SdrTextObj *mpNextInChain = nullptr;
    SdrTextObj *mpPrevInChain = nullptr;

    // indicating the for its text to be chained to another text box
    bool mbToBeChained : 1;

    // Fuer beschriftete Zeichenobjekte ist bTextFrame=FALSE. Der Textblock
    // wird dann hoizontal und vertikal an aRect zentriert. Bei bTextFrame=
    // sal_True wird der Text in aRect hineinformatiert. Der eigentliche Textrahmen
    // ist durch ein SdrRectObj mit bTextFrame=sal_True realisiert.
    bool                        bTextFrame : 1;
    bool                        bPortionInfoChecked : 1; // Fuer Optimierung von Textobjekten
    bool                        bNoShear : 1;            // Obj darf nicht gesheart werden   (->Graf+Ole+TextFrame)
    bool                        bNoRotate : 1;           // Obj darf nicht gedreht werden    (->Ole)
    bool                        bNoMirror : 1;           // Obj darf nicht gespiegelt werden (->Ole,TextFrame)
    bool                        bTextSizeDirty : 1;

    bool                        mbInEditMode : 1;   // Is this text object in edit mode?

    // Fuer Objekt mit freier Groesse im Draw (Mengentext). Das Flag wird vom
    // der App beim Create gesetzt.
    // Wenn das Objekt dann spaeter in der Breite resized wird, wird
    // AutoGrowWidth abgeschaltet (Hart auf sal_False attributiert). Das Flag wird
    // dann ebenfalls auf sal_False gesetzt, sodass sich das Objekt anschliessend
    // wie ein normales Textobjekt verhaelt.
    // Resize in der Breite kann sein:
    // - Interaktives Resize in Einfach- oder Mehrfachselektion
    // - Positions+Groesse Dialog
    bool                        bDisableAutoWidthOnDragging : 1;

    // Allow text suppression
    bool                        mbTextHidden : 1;

    // Flag for allowing text animation. Default is sal_true.
    bool                        mbTextAnimationAllowed : 1;

    // flag for preventing recursive onEditOutlinerStatusEvent calls
    bool                        mbInDownScale : 1;

    SdrOutliner& ImpGetDrawOutliner() const;

private:
    SVX_DLLPRIVATE void ImpCheckMasterCachable();
    // #101029#: Extracted from ImpGetDrawOutliner()
    SVX_DLLPRIVATE void ImpInitDrawOutliner( SdrOutliner& rOutl ) const;
    // #101029#: Extracted from Paint()
    SVX_DLLPRIVATE void ImpSetupDrawOutlinerForPaint( bool bContourFrame,
                                       SdrOutliner&     rOutliner,
                                       Rectangle&       rTextRect,
                                       Rectangle&       rAnchorRect,
                                       Rectangle&       rPaintRect,
                                       Fraction&        aFitXKorreg ) const;
    void ImpAutoFitText( SdrOutliner& rOutliner ) const;
    static void ImpAutoFitText( SdrOutliner& rOutliner, const Size& rShapeSize, bool bIsVerticalWriting );
    SVX_DLLPRIVATE SdrObject* ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const;
    SVX_DLLPRIVATE void ImpLinkAnmeldung();
    SVX_DLLPRIVATE void ImpLinkAbmeldung();
    SVX_DLLPRIVATE ImpSdrObjTextLinkUserData* GetLinkUserData() const;
//  void ImpCheckItemSetChanges(const SfxItemSet& rAttr);

    /** Appends the style family to a provided style name */
    static void AppendFamilyToStyleName(OUString& styleName, SfxStyleFamily family);

    /** Reads the style family from a style name to which the family has been appended. */
    static SfxStyleFamily ReadFamilyFromStyleName(const OUString& styleName);

protected:
    bool ImpCanConvTextToCurve() const;
    SdrObject* ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier, bool bNoSetAttr = false) const;
    SdrObject* ImpConvertAddText(SdrObject* pObj, bool bBezier) const;
    void ImpSetTextStyleSheetListeners();
    static void ImpSetCharStretching(SdrOutliner& rOutliner, const Size& rTextSize, const Size& rShapeSize, Fraction& rFitXKorreg);
    static void ImpJustifyRect(Rectangle& rRect);
    void ImpCheckShear();
    Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;
    void ImpSetTextEditParams() const;
    void SetTextSizeDirty() { bTextSizeDirty=true; }

    // rAnchorRect ist InOut-Parameter!
    void ImpSetContourPolygon( SdrOutliner& rOutliner, Rectangle& rAnchorRect, bool bLineWidth ) const;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;
    bool NbcSetEckenradius(long nRad);

    // #115391# new method for SdrObjCustomShape and SdrTextObj to correctly handle and set
    // SdrTextMinFrameWidthItem and SdrTextMinFrameHeightItem based on all settings, necessities
    // and object sizes
    virtual void AdaptTextMinSize();

    // Konstruktoren fuer beschriftete Zeichenobjekte
    SdrTextObj();
    SdrTextObj(const Rectangle& rNewRect);

    // Konstruktoren fuer Textrahmen
    SdrTextObj(SdrObjKind eNewTextKind);
    SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect);

    virtual ~SdrTextObj();

public:
    TYPEINFO_OVERRIDE();

    bool IsInEditMode() const { return mbInEditMode; }

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
    void SetTextLink(const OUString& rFileName, const OUString& rFilterName, rtl_TextEncoding eCharSet);
    void ReleaseTextLink();
    bool IsLinkedText() const { return pPlusData!=nullptr && GetLinkUserData()!=nullptr; }
    bool ReloadLinkedText(bool bForceLoad = false);
    bool LoadText(const OUString& rFileName, const OUString& rFilterName, rtl_TextEncoding eCharSet);

    virtual bool AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt = true, bool bWdt = true) const;
    virtual bool NbcAdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    virtual bool AdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    bool IsTextFrame() const { return bTextFrame; }
    bool IsOutlText() const { return bTextFrame && (eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT); }
    /// returns true if the PPT autofit of text into shape bounds is enabled. implies IsFitToSize()==false!
    bool IsAutoFit() const;
    /// returns true if the old feature for fitting shape content should into shape is enabled. implies IsAutoFit()==false!
    bool IsFitToSize() const;

    // Chaining
    bool IsToBeChained() const;
    SdrTextObj *GetNextLinkInChain() const;
    void SetNextLinkInChain(SdrTextObj *);
    SdrTextObj *GetPrevLinkInChain() const;
    bool IsChainable() const;
    void SetPreventChainable();
    bool GetPreventChainable() const;
    TextChain *GetTextChain() const;

    SdrObjKind GetTextKind() const { return eTextKind; }

    // #i121917#
    virtual bool HasText() const override;

    bool HasEditText() const;
    bool IsTextEditActive() const { return (pEdtOutl != nullptr); }

    /** returns the currently active text. */
    virtual SdrText* getActiveText() const;

    /** returns the nth available text. */
    virtual SdrText* getText( sal_Int32 nIndex ) const override;

    /** returns the number of texts available for this object. */
    virtual sal_Int32 getTextCount() const override;

    /** returns true only if we are in edit mode and the user actually changed anything */
    virtual bool IsReallyEdited() const;

    /** changes the current active text */
    virtual void setActiveText( sal_Int32 nIndex );

    /** returns the index of the text that contains the given point or -1 */
    virtual sal_Int32 CheckTextHit(const Point& rPnt) const;

    void SetDisableAutoWidthOnDragging(bool bOn) { bDisableAutoWidthOnDragging=bOn; }
    void NbcSetText(const OUString& rStr);
    void SetText(const OUString& rStr);
    void NbcSetText(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat);
    void SetText(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat);

    // FitToSize und Fontwork wird bei GetTextSize() nicht berueksichtigt!
    virtual const Size& GetTextSize() const;
    void FitFrameToTextSize();

    // Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
    // der des EditOutliners) und die PaperSize gesetzt.
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText = false,
        Rectangle* pAnchorRect=nullptr, bool bLineWidth = true ) const;
    virtual void TakeTextAnchorRect(::Rectangle& rAnchorRect) const;
    const GeoStat& GetGeoStat() const { return aGeo; }

    long GetEckenradius() const;
    virtual bool IsAutoGrowHeight() const;
    long GetMinTextFrameHeight() const;
    long GetMaxTextFrameHeight() const;
    virtual bool IsAutoGrowWidth() const;
    long GetMinTextFrameWidth() const;
    long GetMaxTextFrameWidth() const;

    SdrFitToSizeType GetFitToSize() const;
    const Rectangle &GetGeoRect() const;

    // Feststellen, ob TextFontwork
    virtual bool IsFontwork() const;

    // Soll die Fontwork-Kontour versteckt werden?
    bool IsHideContour() const;

    // Textfluss innerhalb Kontur
    bool IsContourTextFrame() const;

    // Horizontale Textausrichtung
    SdrTextHorzAdjust GetTextHorizontalAdjust(const SfxItemSet& rSet) const;
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    // Vertikale Textausrichtung
    SdrTextVertAdjust GetTextVerticalAdjust(const SfxItemSet& rSet) const;
    SdrTextVertAdjust GetTextVerticalAdjust() const;

    /** Left inner spacing to borders  */
    long GetTextLeftDistance() const;
    /** Right inner spacing to borders  */
    long GetTextRightDistance() const;
    /** Top inner spacing to borders */
    long GetTextUpperDistance() const;
    /** Bottom inner spacing to borders */
    long GetTextLowerDistance() const;
    SdrTextAniKind GetTextAniKind() const;
    SdrTextAniDirection GetTextAniDirection() const;

    virtual void SetPage(SdrPage* pNewPage) override;
    virtual void SetModel(SdrModel* pNewModel) override;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;

    // Wird zur Bestimmung des Textankerbereichs benoetigt
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
    virtual SdrTextObj* Clone() const override;
    SdrTextObj& operator=(const SdrTextObj& rObj);
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual basegfx::B2DPolyPolygon TakeContour() const override;
    virtual void RecalcSnapRect() override;
    virtual void NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;
    virtual const Rectangle& GetLogicRect() const override;
    virtual long GetRotateAngle() const override;
    virtual long GetShearAngle(bool bVertical = false) const override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    virtual Pointer GetCreatePointer() const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;

    virtual bool HasTextEdit() const override;
    virtual bool BegTextEdit(SdrOutliner& rOutl) override;
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const;
    virtual void EndTextEdit(SdrOutliner& rOutl) override;
    virtual sal_uInt16 GetOutlinerViewAnchorMode() const;

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject) override;
    void NbcSetOutlinerParaObjectForText( OutlinerParaObject* pTextObject, SdrText* pText );
    virtual OutlinerParaObject* GetOutlinerParaObject() const override;
    virtual OutlinerParaObject* GetEditOutlinerParaObject() const;

    virtual void NbcReformatText() override;
    virtual void ReformatText() override;

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
        bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, OUString& rRet) const;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

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
    void UpdateOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const;
    void ForceOutlinerParaObject();
    virtual bool IsVerticalWriting() const;
    virtual void SetVerticalWriting(bool bVertical);

    /** called from the SdrObjEditView during text edit when the status of the edit outliner changes */
    virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus );

    /** called from the SdrObjEditView during text edit when a chain of boxes is to be updated */
    virtual void onChainingEvent();




    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.


    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;

    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    // #103836# iterates over the paragraphs of a given SdrObject and removes all
    //          hard set character attributes with the which ids contained in the
    //          given vector
    void RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds );

    // Get necessary data for text scroll animation. ATM base it on a Text-Metafile and a
    // painting rectangle. Rotation is taken from the object.
    GDIMetaFile* GetTextScrollMetaFileAndRectangle(Rectangle& rScrollRectangle, Rectangle& rPaintRectangle);

    // Access to TextAnimationAllowed flag
    void SetTextAnimationAllowed(bool bNew);

    // #i8824#
    // Set single item at the local ItemSet. *Does not use* AllowItemChange(),
    // ItemChange(), PostItemChange() and ItemSetChanged() calls.
    void SetObjectItemNoBroadcast(const SfxPoolItem& rItem);

public:

    // text primitive decomposition helpers
    void impDecomposeContourTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrContourTextPrimitive2D& rSdrContourTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposePathTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrPathTextPrimitive2D& rSdrPathTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeBlockTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrBlockTextPrimitive2D& rSdrBlockTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeAutoFitTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrAutoFitTextPrimitive2D& rSdrAutofitTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeStretchTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrStretchTextPrimitive2D& rSdrStretchTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeChainedTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrChainedTextPrimitive2D& rSdrChainedTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impHandleChainingEventsDuringDecomposition(SdrOutliner &rOutliner) const;


    // timing generators
    void impGetBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const;
    void impGetScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const;

    /** returns false if the given pointer is NULL
        or if the given SdrOutliner contains no text.
        Also checks for one empty paragraph.
    */
    static bool HasTextImpl( SdrOutliner* pOutliner );

    friend class ::SdrTextObjTest;
};



#endif // INCLUDED_SVX_SVDOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
