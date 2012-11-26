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



#ifndef _SVDOTEXT_HXX
#define _SVDOTEXT_HXX

#include <vcl/field.hxx>
#include <svx/svdoattr.hxx>
#include <tools/datetime.hxx>
#include <svx/xtextit0.hxx>
#include "svdtext.hxx"
#include <vector>
#include <boost/shared_ptr.hpp>
#include "svx/svxdllapi.h"
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/svdpagv.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SvxFieldItem;
class ImpSdrObjTextLink;
class EditStatus;

namespace sdr { namespace properties {
    class TextProperties;
}}

namespace drawinglayer { namespace primitive2d {
    class SdrContourTextPrimitive2D;
    class SdrPathTextPrimitive2D;
    class SdrBlockTextPrimitive2D;
    class SdrStretchTextPrimitive2D;
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

//************************************************************
//   Defines
//************************************************************

#define SDRUSERDATA_OBJTEXTLINK         (1)

//************************************************************
//   Hilfsklasse ImpSdrObjTextLinkUserData
//************************************************************

class ImpSdrObjTextLinkUserData : public SdrObjUserData
{
    friend class                SdrTextObj;
    friend class                ImpSdrObjTextLink;

    SdrTextObj*                 pObj;
    String                      aFileName;   // Name des referenzierten Dokuments
    String                      aFilterName; // ggf. ein Filter
    DateTime                    aFileDate0;  // Unnoetiges neuladen vermeiden
    ImpSdrObjTextLink*          pLink;
    rtl_TextEncoding            eCharSet;

public:
    ImpSdrObjTextLinkUserData(SdrTextObj* pObj1);
    virtual ~ImpSdrObjTextLinkUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const;
};

namespace sdr
{
    namespace properties
    {
        class CustomShapeProperties;
        class CellProperties;
    } // end of namespace properties
} // end of namespace sdr

///////////////////////////////////////////////////////
// bTextFrame settings:
//
//     SdrTextObj bTextFrame=false (default)
//       SdrObjCustomShape bTextFrame = true, set in constructor, evtl. changed internally
//         OCustomShape OK
//       SdrEdgeObj bTextFrame=false, derived
//       SdrMeasureObj bTextFrame=false, derived
//       SdrPathObj bTextFrame=false, derived
//       SdrRectObj
//              old constructors:
//                  no textkind->bTextFrame=false
//                  with textkind->bTextFrame=true
//         SdrCaptionObj bTextFrame=true, done in constructor
//         SdrCircObj bTextFrame=false, done in constructor
//         SdrGrafObj bTextFrame=false, done in constructor
//         SdrMediaObj bTextFrame=false, done in constructor
//         SdrOle2Obj bTextFrame=false, done in constructor
//           OOle2Obj OK
//         SdrUnoObj bTextFrame=false, done in constructor
//           DlgEdObj OK
//             DlgEdForm OK
//           OUnoObject OK
//           FmFormObj OK
//       SdrTableObj bTextFrame=false, derived
//
// SdrTextObj used as tooling class (cannot be directly incarnated).
// Derivations from SdrRectObj all have a fixed setting. Thus,
// only the SdrRectObj constructor calls may be a TextFrame object
// with choice. Controlled and compared all calls with current main
// trunk.

class SVX_DLLPUBLIC SdrTextObj : public SdrAttrObj
{
private:
    // Cell needs access to ImpGetDrawOutliner();
    friend class                sdr::table::Cell;
    friend class                sdr::table::SdrTableRtfExporter;
    friend class                sdr::table::SdrTableRTFParser;

    // CustomShapeproperties need to access the "bTextFrame" member:
    friend class sdr::properties::CustomShapeProperties;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

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

    friend class                ImpTextPortionHandler;
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
    // this is the active text
    SdrText*                    mpText;

    // Hier merke ich mir die Ausmasse des Textes (n.i.)
    Size                        aTextSize;

    // Ein Outliner*, damit
    // 1. das TextObj nicht von mehreren Views gleichzeitig editiert und
    // 2. beim Streamen waerend des Editierens ein Flush() ausgefuehrt
    // werden kann
    SdrOutliner*                pEdtOutl;

    // Moegliche Werte fuer eTextKind sind:
    //     OBJ_TEXT         normaler Textrahmen
    //     OBJ_TITLETEXT    TitleText fuer StarDraw-Praesentation
    //     OBJ_OUTLINETEXT  OutlineText fuer StarDraw-Praesentation
    // eTextKind hat nur Bedeutung, wenn bTextFrame=true, da es sich sonst
    // um ein beschriftetes Grafikobjekt handelt.
    SdrObjKind                  eTextKind;

    // #108784#
    // For text editing in SW Haeder/Footer it is necessary to be
    // able to set an offset for the text edit to allow text editing at the
    // position of the virtual object. This offset is used when setting up
    // and maintaining the OutlinerView.
    basegfx::B2DPoint       maTextEditOffset;

public:
    // exclusively for SW to force a offset for the activated TextEdit when
    // not the real but the virtual object is edited. May be removed when SW
    // will be changed to use a better concept that the SdrVirtObjs
    const basegfx::B2DPoint& GetTextEditOffset() const { return maTextEditOffset; }
    void SetTextEditOffset(const basegfx::B2DPoint& rNew) { maTextEditOffset = rNew; }

protected:
    // Fuer beschriftete Zeichenobjekte ist bTextFrame=false. Der Textblock
    // wird dann hoizontal und vertikal an aRect zentriert. Bei bTextFrame=
    // true wird der Text in aRect hineinformatiert. Der eigentliche Textrahmen
    // ist durch ein SdrRectObj mit bTextFrame=true realisiert.
    bool                        bTextFrame : 1;
    bool                        bPortionInfoChecked : 1; // Fuer Optimierung von Textobjekten
    // TTTT: These maybe obsolete
    bool                        bNoShear : 1;            // Obj darf nicht gesheart werden   (->Graf+Ole+TextFrame)
    bool                        bNoRotate : 1;           // Obj darf nicht gedreht werden    (->Ole)
    bool                        bNoMirror : 1;           // Obj darf nicht gespiegelt werden (->Ole,TextFrame)
    bool                        bTextSizeDirty : 1;

    // #101684#
    bool                        mbInEditMode : 1;   // Is this text obejct in edit mode?

    // Fuer Objekt mit freier Groesse im Draw (Mengentext). Das Flag wird vom
    // der App beim Create gesetzt.
    // Wenn das Objekt dann spaeter in der Breite resized wird, wird
    // AutoGrowWidth abgeschaltet (Hart auf false attributiert). Das Flag wird
    // dann ebenfalls auf false gesetzt, sodass sich das Objekt anschliessend
    // wie ein normales Textobjekt verhaelt.
    // Resize in der Breite kann sein:
    // - Interaktives Resize in Einfach- oder Mehrfachselektion
    // - Positions+Groesse Dialog
    bool                        bDisableAutoWidthOnDragging : 1;

    // #111096#
    // Flag for allowing text animation. Default is sal_true.
    bool                        mbTextAnimationAllowed : 1;

    SdrOutliner& ImpGetDrawOutliner() const;

private:
    // #101029#: Extracted from ImpGetDrawOutliner()
    SVX_DLLPRIVATE void ImpInitDrawOutliner( SdrOutliner& rOutl ) const;
    // #101029#: Extracted from Paint()
    SVX_DLLPRIVATE void ImpSetupDrawOutlinerForPaint(
        bool bContourFrame,
                                       SdrOutliner&     rOutliner,
        basegfx::B2DRange& rTextRange,
        basegfx::B2DRange& rAnchorRange,
        basegfx::B2DRange& rPaintRange) const;
    SVX_DLLPRIVATE SdrObject* ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const;
    SVX_DLLPRIVATE void ImpLinkAnmeldung();
    SVX_DLLPRIVATE void ImpLinkAbmeldung();
    SVX_DLLPRIVATE ImpSdrObjTextLinkUserData* GetLinkUserData() const;

protected:
    bool ImpCanConvTextToCurve() const;
    SdrObject* ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier, bool bNoSetAttr = sal_False) const;
    SdrObject* ImpConvertAddText(SdrObject* pObj, bool bBezier) const;
    void ImpSetTextStyleSheetListeners();
    void ImpSetCharStretching(SdrOutliner& rOutliner, const basegfx::B2DRange& rTextRange, const basegfx::B2DRange& rAnchorRange) const;
    void ImpSetTextEditParams() const;
    void SetTextSizeDirty() { bTextSizeDirty=true; }

    void SetEdgeRadius(sal_Int32 nRad);
    bool SetMinTextFrameHeight(sal_Int32 nHgt);
    bool SetMinTextFrameWidth(sal_Int32 nWdt);

    // protected consructor: only a tooling class, do not incarnate
    SdrTextObj(
        SdrModel& rSdrModel,
        const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix(),
        SdrObjKind eNewTextKind = OBJ_TEXT,
        bool bIsTextFrame = false);
    virtual ~SdrTextObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

    // helper for AdjustTextFrameWidthAndHeight which contains the common parts to avoid useless code copying
    basegfx::B2DRange ImpAdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange, bool bHgt, bool bWdt, bool bCheckAnimation) const;

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);
    virtual bool DoesSupportTextIndentingOnLineWidthChange() const;

    // #101684#
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
    void SetTextLink(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet);
    void ReleaseTextLink();
    bool IsLinkedText() const { return (mpPlusData && GetLinkUserData()); }
    bool ReloadLinkedText(bool bForceLoad=false);
    bool LoadText(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet);

    virtual basegfx::B2DRange AdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange, bool bHgt = true, bool bWdt = true) const;
    virtual bool AdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    void ResizeTextAttributes(const Fraction& xFact, const Fraction& yFact);
    bool IsTextFrame() const { return bTextFrame; }
    bool IsOutlText() const { return bTextFrame && (eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT); }
    SdrObjKind GetTextKind() const { return eTextKind; }

    virtual bool HasText() const;
    bool HasEditText() const;
    bool IsTextEditActive() const { return (pEdtOutl != 0); }

    /** returns the currently active text. */
    virtual SdrText* getActiveText() const;

    /** returns the nth available text. */
    virtual SdrText* getText( sal_Int32 nIndex ) const;

    /** returns the number of texts available for this object. */
    virtual sal_Int32 getTextCount() const;

    /** returns true only if we are in edit mode and the user actually changed anything */
    virtual bool IsRealyEdited() const;

    /** changes the current active text */
    virtual void setActiveText( sal_Int32 nIndex );

    /** returns the index of the text that contains the given point or -1 */
    virtual sal_Int32 CheckTextHit(const basegfx::B2DPoint& rPnt) const;

    void SetDisableAutoWidthOnDragging(bool bOn) { bDisableAutoWidthOnDragging=bOn; }
    bool IsDisableAutoWidthOnDragging() { return bDisableAutoWidthOnDragging; }
    void SetText(const String& rStr);
    void SetText(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat);

    // FitToSize und Fontwork wird bei GetTextSize() nicht berueksichtigt!
    virtual const Size& GetTextSize() const;
    void FitFrameToTextSize();

    // Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
    // der des EditOutliners) und die PaperSize gesetzt.
    virtual void TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const;

    // get the text range in unified coordinates (relative to
    // the unit range (0,0,1,1))
    virtual basegfx::B2DRange getUnifiedTextRange() const;

    sal_Int32 GetEdgeRadius() const;
    virtual bool IsAutoGrowHeight() const;
    sal_Int32 GetMinTextFrameHeight() const;
    sal_Int32 GetMaxTextFrameHeight() const;
    virtual bool IsAutoGrowWidth() const;
    sal_Int32 GetMinTextFrameWidth() const;
    sal_Int32 GetMaxTextFrameWidth() const;
    SdrFitToSizeType GetFitToSize() const;

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

    // Textrahmenabstaende
    sal_Int32 GetTextLeftDistance() const;
    sal_Int32 GetTextRightDistance() const;
    sal_Int32 GetTextUpperDistance() const;
    sal_Int32 GetTextLowerDistance() const;
    SdrTextAniKind GetTextAniKind() const;
    SdrTextAniDirection GetTextAniDirection() const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;

    // Wird zur Bestimmung des Textankerbereichs benoetigt
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual Pointer GetCreatePointer(const SdrView& rSdrView) const;

    virtual bool HasTextEdit() const;
    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual void TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const;
    virtual void EndTextEdit(SdrOutliner& rOutl);
    virtual sal_uInt16 GetOutlinerViewAnchorMode() const;

    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
    void SetOutlinerParaObjectForText( OutlinerParaObject* pTextObject, SdrText* pText );
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual OutlinerParaObject* GetEditOutlinerParaObject() const;
    virtual bool HasOutlinerParaObject() const;

    // virtual void NbcReformatText();
    virtual void ReformatText();

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
        bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;

    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

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

        @param rPaintRange
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void SetupOutlinerFormatting( SdrOutliner& rOutl, basegfx::B2DRange& rPaintRange ) const;

    /** Update given Outliner equivalently to SdrTextObj::Paint()

        Same functionality as in SetupOutlinerFormatting(), except
        that the outliner content is not cleared.

        @param rOutl
        The Outliner to update.

        @param rPaintRange
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void UpdateOutlinerFormatting( SdrOutliner& rOutl, basegfx::B2DRange& rPaintRange ) const;
    void ForceOutlinerParaObject();
    virtual bool IsVerticalWriting() const;
    virtual void SetVerticalWriting(bool bVertical);

    /** called from the SdrObjEditView during text edit when the status of the edit outliner changes */
    virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus );

    // #103836# iterates over the paragraphs of a given SdrObject and removes all
    //          hard set character attributes with the which ids contained in the
    //          given vector
    virtual void RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds );

    // #111096#
    // Get necessary data for text scroll animation. ATM base it on a Text-Metafile and a
    // painting rectangle. Rotation is taken from the object.
    GDIMetaFile* GetTextScrollMetaFileAndRange(basegfx::B2DRange& rScrollRange, basegfx::B2DRange& rPaintRange) const;

    // #111096#
    // Access to TextAnimationAllowed flag
    bool IsTextAnimationAllowed() const;
    void SetTextAnimationAllowed(bool bNew);

    // #i8824#
    // Set single item at the local ItemSet. *Does not use* AllowItemChange(),
    // ItemChange(), PostItemChange() and ItemSetChanged() calls.
    void SetObjectItemNoBroadcast(const SfxPoolItem& rItem);

    //////////////////////////////////////////////////////////////////////////////
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
    void impDecomposeStretchTextPrimitive(
        drawinglayer::primitive2d::Primitive2DSequence& rTarget,
        const drawinglayer::primitive2d::SdrStretchTextPrimitive2D& rSdrStretchTextPrimitive,
        const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;

    //////////////////////////////////////////////////////////////////////////////
    // timing generators
    void impGetBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const;
    void impGetScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const;

    /** returns false if the given pointer is 0
        or if the given SdrOutliner contains no text.
        Also checks for one empty paragraph.
    */
    static bool HasTextImpl( const SdrOutliner* pOutliner );

    /** returns a new created and non shared outliner.
        The outliner will not get updated when the SdrModel is changed.
    */
    boost::shared_ptr< SdrOutliner > CreateDrawOutliner();

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOTEXT_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
