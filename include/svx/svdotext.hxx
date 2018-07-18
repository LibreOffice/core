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
#include <svl/style.hxx>
#include <svx/xtextit0.hxx>
#include <svx/svdtext.hxx>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/svdpagv.hxx>
#include <rtl/ref.hxx>
#include <memory>
#include <vector>


//   forward declarations


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
enum class EEAnchorMode;
enum class EETextFormat;

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


//   helper class SdrTextObjGeoData


class SdrTextObjGeoData : public SdrObjGeoData
{
public:
    tools::Rectangle                   aRect;
    GeoStat                     aGeo;
};


//   helper class ImpSdrObjTextLinkUserData


class ImpSdrObjTextLinkUserData : public SdrObjUserData
{
    friend class                SdrTextObj;
    friend class                ImpSdrObjTextLink;

    OUString                    aFileName;   // name of the referenced document
    OUString                    aFilterName; // a filter, if need be
    DateTime                    aFileDate0;  // avoiding unnecessary reload
    tools::SvRef<ImpSdrObjTextLink>
                                pLink;
    rtl_TextEncoding            eCharSet;

public:
    ImpSdrObjTextLinkUserData();
    virtual ~ImpSdrObjTextLinkUserData() override;

    virtual std::unique_ptr<SdrObjUserData> Clone(SdrObject* pObj1) const override;
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
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

private:
    // This method is only allowed for sdr::properties::TextProperties
    SVX_DLLPRIVATE SdrOutliner* GetTextEditOutliner() const
    {
        return pEdtOutl;
    }

    // to allow sdr::properties::TextProperties access to SetPortionInfoChecked()
    // and GetTextEditOutliner()
    friend class sdr::properties::TextProperties;
    friend class sdr::properties::CellProperties;

    friend class                ImpSdrObjTextLink;
    friend class                ImpSdrObjTextLinkUserData;
    friend class                SdrPowerPointImport; // for PowerPointImport
    friend class                SdrExchangeView;     // for ImpGetDrawOutliner
    friend class                SdrView;             // for ImpGetDrawOutliner
    friend class                SdrObjEditView;      // for TextEdit
    friend class                SdrMeasureObj;       // for ImpGetDrawOutliner
    friend class                SvxMSDffManager;     // for ImpGetDrawOutliner
    friend class                SdrObjCustomShape;   // for ImpGetDrawOutliner
    friend class                SdrText;             // for ImpGetDrawOutliner

protected:
    // The "aRect" is also the rect of RectObj and CircObj.
    // When bTextFrame=sal_True the text will be formatted into this rect
    // When bTextFrame=sal_False the text will be centered around its middle
    tools::Rectangle maRect;

    // The GeoStat contains the rotation and shear angles
    GeoStat                     aGeo;

    // this is the active text
    std::unique_ptr<SdrText>    mpText;

    // This contains the dimensions of the text
    Size                        aTextSize;

    // an Outliner*, so that
    // 1. the TextObj won't be edited simultaneously by multiple views, and
    // 2. when streaming while editing Flush() can be done
    SdrOutliner*                pEdtOutl;

    // Possible values for eTextKind are:
    //     OBJ_TEXT         regular text frame
    //     OBJ_TITLETEXT    TitleText for presentations
    //     OBJ_OUTLINETEXT  OutlineText for presentations
    // eTextKind only has meaning when bTextFrame=sal_True, since otherwise
    // we're dealing with a labeled graphical object
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
    bool mbIsUnchainableClone = false;

    // the successor in a chain
    SdrTextObj *mpNextInChain = nullptr;
    SdrTextObj *mpPrevInChain = nullptr;

    // For labeled graphical objects bTextFrame is FALSE. The block of text
    // will then be centered horizontally and vertically on aRect.
    // For bTextFalse=sal_True the text will be formatted into aRect.
    // The actual text frame is realized by an SdrRectObj with
    // bTextFrame=sal_True.
    bool                        bTextFrame : 1;
    bool                        bNoShear : 1;            // disable shearing   (->graphic+Ole+TextFrame)
    bool                        bTextSizeDirty : 1;

    bool                        mbInEditMode : 1;   // Is this text object in edit mode?

    // For objects with free size (flow text). The flag is set by the
    // application on create. If the object width is later resized,
    // AutoGrowWidth will be disabled (set to sal_False). This flag will
    // then also be set to sal_False, so that the object behaves like a
    // normal text object.
    // Width resize can result from:
    // - Interactive Resize in single or multiple selections
    // - position/size dialog
    bool                        bDisableAutoWidthOnDragging : 1;

    // Flag for allowing text animation. Default is sal_true.
    bool                        mbTextAnimationAllowed : 1;

    // flag for preventing recursive onEditOutlinerStatusEvent calls
    bool                        mbInDownScale : 1;

    SdrOutliner& ImpGetDrawOutliner() const;

private:
    // #101029#: Extracted from ImpGetDrawOutliner()
    SVX_DLLPRIVATE void ImpInitDrawOutliner( SdrOutliner& rOutl ) const;
    // #101029#: Extracted from Paint()
    SVX_DLLPRIVATE void ImpSetupDrawOutlinerForPaint( bool bContourFrame,
                                       SdrOutliner&     rOutliner,
                                       tools::Rectangle&       rTextRect,
                                       tools::Rectangle&       rAnchorRect,
                                       tools::Rectangle&       rPaintRect,
                                       Fraction&        aFitXCorrection ) const;
    void ImpAutoFitText( SdrOutliner& rOutliner ) const;
    static void ImpAutoFitText( SdrOutliner& rOutliner, const Size& rShapeSize, bool bIsVerticalWriting );
    SVX_DLLPRIVATE SdrObject* ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const;
    SVX_DLLPRIVATE void ImpRegisterLink();
    SVX_DLLPRIVATE void ImpDeregisterLink();
    SVX_DLLPRIVATE ImpSdrObjTextLinkUserData* GetLinkUserData() const;

    /** Appends the style family to a provided style name */
    static void AppendFamilyToStyleName(OUString& styleName, SfxStyleFamily family);

    /** Reads the style family from a style name to which the family has been appended. */
    static SfxStyleFamily ReadFamilyFromStyleName(const OUString& styleName);

protected:
    bool ImpCanConvTextToCurve() const;
    SdrObject* ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier) const;
    SdrObject* ImpConvertAddText(SdrObject* pObj, bool bBezier) const;
    void ImpSetTextStyleSheetListeners();
    static void ImpSetCharStretching(SdrOutliner& rOutliner, const Size& rTextSize, const Size& rShapeSize, Fraction& rFitXCorrection);
    static void ImpJustifyRect(tools::Rectangle& rRect);
    void ImpCheckShear();
    tools::Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;
    void ImpSetTextEditParams() const;
    void SetTextSizeDirty() { bTextSizeDirty=true; }

    // rAnchorRect is InOut-Parameter!
    void ImpSetContourPolygon( SdrOutliner& rOutliner, tools::Rectangle const & rAnchorRect, bool bLineWidth ) const;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;
    void NbcSetEckenradius(long nRad);

    // #115391# new method for SdrObjCustomShape and SdrTextObj to correctly handle and set
    // SdrTextMinFrameWidthItem and SdrTextMinFrameHeightItem based on all settings, necessities
    // and object sizes
    virtual void AdaptTextMinSize();

    // constructors for labeled graphical objects
    SdrTextObj(SdrModel& rSdrModel);
    SdrTextObj(
        SdrModel& rSdrModel,
        const tools::Rectangle& rNewRect);

    // constructors for text frames
    SdrTextObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewTextKind);
    SdrTextObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewTextKind,
        const tools::Rectangle& rNewRect);

    // protected destructor
    virtual ~SdrTextObj() override;

public:

    bool IsInEditMode() const { return mbInEditMode; }

    // Currently ASCII and RTF are supported and the differentiation is done
    // internally.
    // rFilterName has no meaning and must be empty
    // Normally the application only has to call SetTextLink() - the
    // remainder is automatic (SfxLinkManager).
    // The LoadText() method can also be used to load a file into a text
    // object (without linking.)
    // TextLinks can't be edited (if needed later could be ReadOnly).
    // Setting attributes can only be done on the text frame.
    void SetTextLink(const OUString& rFileName, const OUString& rFilterName);
    void ReleaseTextLink();
    bool IsLinkedText() const { return pPlusData!=nullptr && GetLinkUserData()!=nullptr; }
    bool ReloadLinkedText(bool bForceLoad);
    bool LoadText(const OUString& rFileName, rtl_TextEncoding eCharSet);

    virtual bool AdjustTextFrameWidthAndHeight(tools::Rectangle& rR, bool bHgt = true, bool bWdt = true) const;
    virtual bool NbcAdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    virtual bool AdjustTextFrameWidthAndHeight();
    bool IsTextFrame() const { return bTextFrame; }
    bool IsOutlText() const { return bTextFrame && (eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT); }
    /// returns true if the PPT autofit of text into shape bounds is enabled. implies IsFitToSize()==false!
    bool IsAutoFit() const;
    /// returns true if the old feature for fitting shape content should into shape is enabled. implies IsAutoFit()==false!
    bool IsFitToSize() const;

    // Chaining
    SdrTextObj *GetNextLinkInChain() const;
    void SetNextLinkInChain(SdrTextObj *);
    SdrTextObj *GetPrevLinkInChain() const;
    bool IsChainable() const;
    bool GetPreventChainable() const;
    TextChain *GetTextChain() const;

    SdrObjKind GetTextKind() const { return eTextKind; }

    // #i121917#
    virtual bool HasText() const override;

    bool IsTextEditActive() const { return pEdtOutl != nullptr; }

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
    void NbcSetText(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat);
    void SetText(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat);

    // FitToSize and Fontwork are not taken into account in GetTextSize()!
    virtual const Size& GetTextSize() const;
    void FitFrameToTextSize();
    double GetFontScaleY() const;

    // Simultaneously sets the text into the Outliner (possibly
    // the one of the EditOutliner) and sets the PaperSize.
    virtual void TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText,
        tools::Rectangle* pAnchorRect, bool bLineWidth = true ) const;
    virtual void TakeTextAnchorRect(::tools::Rectangle& rAnchorRect) const;
    const GeoStat& GetGeoStat() const { return aGeo; }

    // get corner radius
    long GetEckenradius() const;
    virtual bool IsAutoGrowHeight() const;
    long GetMinTextFrameHeight() const;
    long GetMaxTextFrameHeight() const;
    virtual bool IsAutoGrowWidth() const;
    long GetMinTextFrameWidth() const;
    long GetMaxTextFrameWidth() const;

    css::drawing::TextFitToSizeType GetFitToSize() const;
    const tools::Rectangle &GetGeoRect() const;

    // check if it's a TextFontwork
    virtual bool IsFontwork() const;

    // should the Fontwork contour be hidden?
    bool IsHideContour() const;

    // text flow within contour
    bool IsContourTextFrame() const;

    SdrTextHorzAdjust GetTextHorizontalAdjust(const SfxItemSet& rSet) const;
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

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

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;

    // needed to determine text anchor area
    virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const;
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
    virtual SdrTextObj* CloneSdrObject(SdrModel& rTargetModel) const override;
    SdrTextObj& operator=(const SdrTextObj& rObj);
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual basegfx::B2DPolyPolygon TakeContour() const override;
    virtual void RecalcSnapRect() override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual const tools::Rectangle& GetLogicRect() const override;
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
    // returns true if TextEditMode started
    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const;
    virtual void EndTextEdit(SdrOutliner& rOutl);
    virtual EEAnchorMode GetOutlinerViewAnchorMode() const;

    virtual void NbcSetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> pTextObject) override;
    void NbcSetOutlinerParaObjectForText( std::unique_ptr<OutlinerParaObject> pTextObject, SdrText* pText );
    virtual OutlinerParaObject* GetOutlinerParaObject() const override;
    std::unique_ptr<OutlinerParaObject> GetEditOutlinerParaObject() const;

    virtual void NbcReformatText() override;
    virtual void ReformatText() override;

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
        bool bEdit, boost::optional<Color>& rpTxtColor, boost::optional<Color>& rpFldColor, OUString& rRet) const;

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
    void SetupOutlinerFormatting( SdrOutliner& rOutl, tools::Rectangle& rPaintRect ) const;

    /** Update given Outliner equivalently to SdrTextObj::Paint()

        Same functionality as in SetupOutlinerFormatting(), except
        that the outliner content is not cleared.

        @param rOutl
        The Outliner to update.

        @param rPaintRect
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void UpdateOutlinerFormatting( SdrOutliner& rOutl, tools::Rectangle& rPaintRect ) const;
    void ForceOutlinerParaObject();
    virtual bool IsVerticalWriting() const;
    virtual void SetVerticalWriting(bool bVertical);

    /** called from the SdrObjEditView during text edit when the status of the edit outliner changes */
    virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus );

    /** called from the SdrObjEditView during text edit when a chain of boxes is to be updated */
    void onChainingEvent();


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
    GDIMetaFile* GetTextScrollMetaFileAndRectangle(tools::Rectangle& rScrollRectangle, tools::Rectangle& rPaintRectangle);

    // Access to TextAnimationAllowed flag
    void SetTextAnimationAllowed(bool bNew);

    // #i8824#
    // Set single item at the local ItemSet. *Does not use* AllowItemChange(),
    // ItemChange(), PostItemChange() and ItemSetChanged() calls.
    void SetObjectItemNoBroadcast(const SfxPoolItem& rItem);

public:

    // text primitive decomposition helpers
    void impDecomposeContourTextPrimitive(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        const drawinglayer::primitive2d::SdrContourTextPrimitive2D& rSdrContourTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposePathTextPrimitive(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        const drawinglayer::primitive2d::SdrPathTextPrimitive2D& rSdrPathTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeBlockTextPrimitive(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        const drawinglayer::primitive2d::SdrBlockTextPrimitive2D& rSdrBlockTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeAutoFitTextPrimitive(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        const drawinglayer::primitive2d::SdrAutoFitTextPrimitive2D& rSdrAutofitTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeStretchTextPrimitive(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        const drawinglayer::primitive2d::SdrStretchTextPrimitive2D& rSdrStretchTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impDecomposeChainedTextPrimitive(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        const drawinglayer::primitive2d::SdrChainedTextPrimitive2D& rSdrChainedTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
    void impHandleChainingEventsDuringDecomposition(SdrOutliner &rOutliner) const;

    // timing generators
    void impGetBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const;
    void impGetScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const;

    // Direct decomposer for text visualization when you already have a prepared
    // Outliner containing all the needed information
    static void impDecomposeBlockTextPrimitiveDirect(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,
        SdrOutliner& rOutliner,
        const basegfx::B2DHomMatrix& rNewTransformA,
        const basegfx::B2DHomMatrix& rNewTransformB,
        const basegfx::B2DRange& rClipRange);

    /** returns false if the given pointer is NULL
        or if the given SdrOutliner contains no text.
        Also checks for one empty paragraph.
    */
    static bool HasTextImpl( SdrOutliner const * pOutliner );

    friend class ::SdrTextObjTest;
};


#endif // INCLUDED_SVX_SVDOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
