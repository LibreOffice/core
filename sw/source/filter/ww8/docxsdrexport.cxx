/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "docxsdrexport.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/boxitem.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/svditer.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/relationship.hxx>
#include <textboxhelper.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include <fmtcntnt.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtfollowtextflow.hxx>
#include <frmatr.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include "docxattributeoutput.hxx"
#include "docxexportfilter.hxx"
#include <comphelper/flagguard.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <frmfmt.hxx>
#include <IDocumentDrawModelAccess.hxx>

#include <svx/svdtrans.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

using namespace com::sun::star;
using namespace oox;
using namespace sax_fastparser;

namespace
{
uno::Sequence<beans::PropertyValue> lclGetProperty(const uno::Reference<drawing::XShape>& rShape,
                                                   const OUString& rPropName)
{
    uno::Sequence<beans::PropertyValue> aResult;
    uno::Reference<beans::XPropertySet> xPropertySet(rShape, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo;

    if (!xPropertySet.is())
        return aResult;

    xPropSetInfo = xPropertySet->getPropertySetInfo();
    if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(rPropName))
    {
        xPropertySet->getPropertyValue(rPropName) >>= aResult;
    }
    return aResult;
}

OUString lclGetAnchorIdFromGrabBag(const SdrObject* pObj)
{
    OUString aResult;
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pObj)->getUnoShape(),
                                           uno::UNO_QUERY);
    OUString aGrabBagName;
    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY);
    if (xServiceInfo->supportsService(u"com.sun.star.text.TextFrame"_ustr))
        aGrabBagName = "FrameInteropGrabBag";
    else
        aGrabBagName = "InteropGrabBag";
    const uno::Sequence<beans::PropertyValue> propList = lclGetProperty(xShape, aGrabBagName);
    auto pProp
        = std::find_if(propList.begin(), propList.end(),
                       [](const beans::PropertyValue& rProp) { return rProp.Name == "AnchorId"; });
    if (pProp != propList.end())
        pProp->Value >>= aResult;
    return aResult;
}

void lclMovePositionWithRotation(awt::Point& aPos, const Size& rSize, Degree100 nRotation100)
{
    // code from ImplEESdrWriter::ImplFlipBoundingBox (filter/source/msfilter/eschesdo.cxx)
    // TODO: refactor
    // MSO uses left|top of the unrotated object rectangle as position. When you rotate that rectangle
    // around its center and build a snap rectangle S from it, then left|top of S has to be the
    // position used in LO. This method converts LOs aPos to the position used by MSO.

    // rSize has to be size of the logicRect of the object. For calculating the diff, we build a
    // rectangle with left|top A = (-fWidthHalf | -fHeightHalf) and
    // right|top B = (fWidthHalf | -fHeightHalf). The rotation matrix R is here
    //    fcos fsin
    //   -fsin fcos
    // Left of rectangle S = X-coord of R * A, Top of rectangle S = Y-coord of R * B

    // Use nRotation in [0;9000], for to have only one and not four cases.
    if (nRotation100 == 0_deg100)
        return;
    if (nRotation100 < 0_deg100)
        nRotation100 = (36000_deg100 + nRotation100) % 36000_deg100;
    if (nRotation100 % 18000_deg100 == 0_deg100)
        nRotation100 = 0_deg100; // prevents endless loop
    while (nRotation100 > 9000_deg100)
        nRotation100 = 18000_deg100 - (nRotation100 % 18000_deg100);

    double fVal = toRadians(nRotation100);
    double fCos = (nRotation100 == 9000_deg100) ? 0.0 : cos(fVal);
    double fSin = sin(fVal);

    double fWidthHalf = static_cast<double>(rSize.Width()) / 2.0;
    double fHeightHalf = static_cast<double>(rSize.Height()) / 2.0;

    double fXDiff = fSin * fHeightHalf + fCos * fWidthHalf - fWidthHalf;
    double fYDiff = fSin * fWidthHalf + fCos * fHeightHalf - fHeightHalf;

    aPos.X += fXDiff + 0.5;
    aPos.Y += fYDiff + 0.5;
}

/// Determines if the anchor is inside a paragraph.
bool IsAnchorTypeInsideParagraph(const ww8::Frame* pFrame)
{
    const SwFormatAnchor& rAnchor = pFrame->GetFrameFormat().GetAttrSet().GetAnchor();
    return rAnchor.GetAnchorId() != RndStdIds::FLY_AT_PAGE;
}

bool lcl_IsRotateAngleValid(const SdrObject& rObj)
{
    // Some shape types report a rotation angle but are not actually rotated, because all rotation
    // have been incorporated.
    switch (rObj.GetObjIdentifier())
    {
        case SdrObjKind::Group:
        case SdrObjKind::Line:
        case SdrObjKind::PolyLine:
        case SdrObjKind::PathLine:
        case SdrObjKind::PathFill:
            return false;
        default:
            return true;
    }
}

void lcl_calculateMSOBaseRectangle(const SdrObject& rObj, double& rfMSOLeft, double& rfMSORight,
                                   double& rfMSOTop, double& rfMSOBottom,
                                   const bool bIsWord2007Image)
{
    // Word rotates around shape center, LO around left/top. Thus logic rectangle of LO is not
    // directly usable as 'base rectangle'.
    double fCenterX = (rObj.GetSnapRect().Left() + rObj.GetSnapRect().Right()) / 2.0;
    double fCenterY = (rObj.GetSnapRect().Top() + rObj.GetSnapRect().Bottom()) / 2.0;
    double fHalfWidth = rObj.GetLogicRect().getOpenWidth() / 2.0;
    double fHalfHeight = rObj.GetLogicRect().getOpenHeight() / 2.0;

    // MSO swaps width and height depending on rotation angle; exception: Word 2007 (vers 12) never
    // swaps width and height for images.
    double fRotation
        = lcl_IsRotateAngleValid(rObj) ? toDegrees(NormAngle36000(rObj.GetRotateAngle())) : 0.0;
    if (((fRotation > 45.0 && fRotation <= 135.0) || (fRotation > 225.0 && fRotation <= 315.0))
        && !bIsWord2007Image)
    {
        rfMSOLeft = fCenterX - fHalfHeight;
        rfMSORight = fCenterX + fHalfHeight;
        rfMSOTop = fCenterY - fHalfWidth;
        rfMSOBottom = fCenterY + fHalfWidth;
    }
    else
    {
        rfMSOLeft = fCenterX - fHalfWidth;
        rfMSORight = fCenterX + fHalfWidth;
        rfMSOTop = fCenterY - fHalfHeight;
        rfMSOBottom = fCenterY + fHalfHeight;
    }
}

void lcl_calculateRawEffectExtent(sal_Int32& rLeft, sal_Int32& rTop, sal_Int32& rRight,
                                  sal_Int32& rBottom, const SdrObject& rObj,
                                  const bool bUseBoundRect, const bool bIsWord2007Image)
{
    // This method calculates the extent needed, to let Word use the same outer area for the object
    // as LO. Word uses as 'base rectangle' the unrotated shape rectangle, maybe having swapped width
    // and height depending on rotation angle and version of Word.
    double fMSOLeft;
    double fMSORight;
    double fMSOTop;
    double fMSOBottom;
    lcl_calculateMSOBaseRectangle(rObj, fMSOLeft, fMSORight, fMSOTop, fMSOBottom, bIsWord2007Image);

    tools::Rectangle aLORect = bUseBoundRect ? rObj.GetCurrentBoundRect() : rObj.GetSnapRect();
    rLeft = fMSOLeft - aLORect.Left();
    rRight = aLORect.Right() - fMSORight;
    rTop = fMSOTop - aLORect.Top();
    rBottom = aLORect.Bottom() - fMSOBottom;
    // Result values might be negative, e.g for a custom shape 'Arc'.
    return;
}

bool lcl_makeSingleDistAndEffectExtentNonNegative(sal_Int64& rDist, sal_Int32& rExt)
{
    // A negative effectExtent is allowed in OOXML, but Word cannot handle it (bug in Word). It
    // might occur, if the BoundRect in LO is smaller than the base rect in Word.
    // A negative wrap distance from text is allowed in ODF. LO can currently only handle left and
    // right negative values, see bug tdf#141880. Dist must be non-negative in OOXML.
    // We try to compensate Dist vs effectExtent to get similar visual appearance.
    if (rExt >= 0 && rDist >= 0)
        return true;
    if (rExt < 0 && rDist < 0)
    {
        rExt = 0;
        rDist = 0;
        return false;
    }
    if (rDist + static_cast<sal_Int64>(rExt) < 0) // different sign, so no overflow
    {
        rExt = 0;
        rDist = 0;
        return false;
    }
    // rDist + rExt >= 0
    if (rDist < 0)
    {
        rExt += rDist;
        rDist = 0;
    }
    else // rExt < 0
    {
        rDist += rExt;
        rExt = 0;
    }
    return true;
}

bool lcl_makeDistAndExtentNonNegative(sal_Int64& rDistT, sal_Int64& rDistB, sal_Int64& rDistL,
                                      sal_Int64& rDistR, sal_Int32& rLeftExt, sal_Int32& rTopExt,
                                      sal_Int32& rRightExt, sal_Int32& rBottomExt)
{
    bool bLeft = lcl_makeSingleDistAndEffectExtentNonNegative(rDistL, rLeftExt);
    bool bTop = lcl_makeSingleDistAndEffectExtentNonNegative(rDistT, rTopExt);
    bool bRight = lcl_makeSingleDistAndEffectExtentNonNegative(rDistR, rRightExt);
    bool bBottom = lcl_makeSingleDistAndEffectExtentNonNegative(rDistB, rBottomExt);
    return bLeft && bTop && bRight && bBottom;
}

void lcl_makeSingleDistZeroAndExtentNonNegative(sal_Int64& rDist, sal_Int32& rExt)
{
    if (static_cast<double>(rDist) + static_cast<double>(rExt)
        >= static_cast<double>(SAL_MAX_INT32))
        rExt = SAL_MAX_INT32;
    else if (static_cast<double>(rDist) + static_cast<double>(rExt) <= 0)
        rExt = 0;
    else // 0 < rDist + rExt < SAL_MAX_INT32
    {
        rExt = static_cast<sal_Int32>(rDist + rExt);
        if (rExt < 0)
            rExt = 0;
    }
    rDist = 0;
}

void lcl_makeDistZeroAndExtentNonNegative(sal_Int64& rDistT, sal_Int64& rDistB, sal_Int64& rDistL,
                                          sal_Int64& rDistR, sal_Int32& rLeftExt,
                                          sal_Int32& rTopExt, sal_Int32& rRightExt,
                                          sal_Int32& rBottomExt)
{
    lcl_makeSingleDistZeroAndExtentNonNegative(rDistL, rLeftExt);
    lcl_makeSingleDistZeroAndExtentNonNegative(rDistT, rTopExt);
    lcl_makeSingleDistZeroAndExtentNonNegative(rDistR, rRightExt);
    lcl_makeSingleDistZeroAndExtentNonNegative(rDistB, rBottomExt);
}

tools::Polygon lcl_CreateContourPolygon(SdrObject* pSdrObj)
{
    tools::Polygon aContour;
    if (!pSdrObj)
    {
        // use rectangular default
        aContour.Insert(0, Point(0, 0));
        aContour.Insert(1, Point(21600, 0));
        aContour.Insert(2, Point(21600, 21600));
        aContour.Insert(3, Point(0, 21600));
        aContour.Insert(4, Point(0, 0));
        return aContour;
    }

    // Simple version for now: Use ready PolygonFromPolyPolygon().
    // For that we first create a B2DPolyPolygon from the shape, that ideally contains
    // the outline of the shape.
    basegfx::B2DPolyPolygon aPolyPolygon;
    switch (pSdrObj->GetObjIdentifier())
    {
        case SdrObjKind::CustomShape:
        {
            // EnhancedCustomShapeEngine::GetLineGeometry() is not directly usable, because the wrap
            // polygon acts on the untransformed shape in Word. We do here similar as in
            // GetLineGeometry(), but without transformations.
            EnhancedCustomShape2d aCustomShape2d(*static_cast<SdrObjCustomShape*>(pSdrObj));
            rtl::Reference<SdrObject> pLineGeometryObj = aCustomShape2d.CreateLineGeometry();
            if (!pLineGeometryObj)
                break;

            // We might have got other object kinds than SdrPathObj, even groups.
            SdrObjListIter aIter(*pLineGeometryObj, SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                basegfx::B2DPolyPolygon aPP;
                const SdrObject* pNext = aIter.Next();
                if (auto pPathObj = dynamic_cast<const SdrPathObj*>(pNext))
                    aPP = pPathObj->GetPathPoly();
                else
                {
                    rtl::Reference<SdrObject> pNewObj
                        = pLineGeometryObj->ConvertToPolyObj(false, false);
                    SdrPathObj* pPath = dynamic_cast<SdrPathObj*>(pNewObj.get());
                    if (pPath)
                        aPP = pPath->GetPathPoly();
                }
                if (aPP.count())
                    aPolyPolygon.append(aPP);
            }

            if (!aPolyPolygon.count())
                break;

            // Make relative to range 0..21600, 0..21600
            Point aCenter(pSdrObj->GetSnapRect().Center());
            basegfx::B2DHomMatrix aTranslateToOrigin(
                basegfx::utils::createTranslateB2DHomMatrix(-aCenter.X(), -aCenter.Y()));
            aPolyPolygon.transform(aTranslateToOrigin);
            const double fWidth(pSdrObj->GetLogicRect().getOpenWidth());
            double fScaleX = fWidth == 0.0 ? 1.0 : 21600.0 / fWidth;
            const double fHeight(pSdrObj->GetLogicRect().getOpenHeight());
            double fScaleY = fHeight == 0.0 ? 1.0 : 21600.0 / fHeight;
            basegfx::B2DHomMatrix aScale(basegfx::utils::createScaleB2DHomMatrix(fScaleX, fScaleY));
            aPolyPolygon.transform(aScale);

            basegfx::B2DHomMatrix aTranslateToCenter(
                basegfx::utils::createTranslateB2DHomMatrix(10800.0, 10800.0));
            aPolyPolygon.transform(aTranslateToCenter);
            break;
        } // end case OBJ_CUSTOMSHAPE
        case SdrObjKind::Line:
        {
            aContour.Insert(0, Point(0, 0));
            aContour.Insert(1, Point(21600, 21600));
            aContour.Insert(2, Point(0, 0));
            return aContour;
        }
        case SdrObjKind::PathFill:
        case SdrObjKind::PathLine:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::FreehandLine:
        case SdrObjKind::PathPoly:
        case SdrObjKind::PathPolyLine:
            // case OBJ_POLY: FixMe: Creating wrap polygon would work, but export to DML is currently
            // case OBJ_PLIN: disabled for unknown reason; related bug 75254.
            {
                // Includes removing any control points
                rtl::Reference<SdrObject> pNewObj = pSdrObj->ConvertToPolyObj(false, false);
                SdrPathObj* pConverted = dynamic_cast<SdrPathObj*>(pNewObj.get());
                if (!pConverted)
                    break;
                aPolyPolygon = pConverted->GetPathPoly();
                pNewObj.clear();

                // Word adds a line from last to first point. That will cut of indentations from being
                // filled. To prevent this, the wrap polygon is lead along the path back to the first
                // point and so indentation is kept.
                if (!aPolyPolygon.isClosed())
                {
                    basegfx::B2DPolyPolygon aReverse(aPolyPolygon);
                    aReverse.flip();
                    aPolyPolygon.append(aReverse);
                }

                // Make relative to range 0..21600, 0..21600
                Point aCenter(pSdrObj->GetSnapRect().Center());
                basegfx::B2DHomMatrix aTranslateToOrigin(
                    basegfx::utils::createTranslateB2DHomMatrix(-aCenter.X(), -aCenter.Y()));
                aPolyPolygon.transform(aTranslateToOrigin);

                const double fWidth(pSdrObj->GetLogicRect().getOpenWidth());
                double fScaleX = fWidth == 0.0 ? 1.0 : 21600.0 / fWidth;
                const double fHeight(pSdrObj->GetLogicRect().getOpenHeight());
                double fScaleY = fHeight == 0.0 ? 1.0 : 21600.0 / fHeight;
                basegfx::B2DHomMatrix aScale(
                    basegfx::utils::createScaleB2DHomMatrix(fScaleX, fScaleY));
                aPolyPolygon.transform(aScale);

                basegfx::B2DHomMatrix aTranslateToCenter(
                    basegfx::utils::createTranslateB2DHomMatrix(10800.0, 10800.0));
                aPolyPolygon.transform(aTranslateToCenter);
                break;
            }
        case SdrObjKind::NONE:
        default:
            break;
    }

    // Simple version for now: Use ready PolygonFromPolyPolygon()
    const tools::PolyPolygon aToolsPolyPoly(aPolyPolygon);
    aContour = sw::util::PolygonFromPolyPolygon(aToolsPolyPoly);

    // The wrap polygon needs at least two points in OOXML and three points in Word.
    switch (aContour.GetSize())
    {
        case 0:
            // use rectangular default
            aContour.Insert(0, Point(0, 0));
            aContour.Insert(1, Point(21600, 0));
            aContour.Insert(2, Point(21600, 21600));
            aContour.Insert(3, Point(0, 21600));
            aContour.Insert(4, Point(0, 0));
            break;
        case 1:
            aContour.Insert(1, aContour.GetPoint(0));
            aContour.Insert(2, aContour.GetPoint(0));
            break;
        case 2:
            aContour.Insert(2, aContour.GetPoint(0));
            break;
        default:
            break;
    }
    return aContour;
}
} // end anonymous namespace

ExportDataSaveRestore::ExportDataSaveRestore(DocxExport& rExport, SwNodeOffset nStt,
                                             SwNodeOffset nEnd, ww8::Frame const* pParentFrame)
    : m_rExport(rExport)
{
    m_rExport.SaveData(nStt, nEnd);
    m_rExport.m_pParentFrame = pParentFrame;
}

ExportDataSaveRestore::~ExportDataSaveRestore() { m_rExport.RestoreData(); }

/// Holds data used by DocxSdrExport only.
struct DocxSdrExport::Impl
{
private:
    DocxExport& m_rExport;
    sax_fastparser::FSHelperPtr m_pSerializer;
    oox::drawingml::DrawingML* m_pDrawingML;
    const Size* m_pFlyFrameSize;
    bool m_bTextFrameSyntax;
    bool m_bDMLTextFrameSyntax;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pFlyAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pTextboxAttrList;
    OStringBuffer m_aTextFrameStyle;
    bool m_bDrawingOpen;
    bool m_bParagraphSdtOpen;
    bool m_bParagraphHasDrawing; ///Flag for checking drawing in a paragraph.
    rtl::Reference<sax_fastparser::FastAttributeList> m_pFlyFillAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pFlyWrapAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pBodyPrAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pDashLineStyleAttr;
    bool m_bDMLAndVMLDrawingOpen;
    /// List of TextBoxes in this document: they are exported as part of their shape, never alone.
    /// Preserved rotation for TextFrames.
    Degree100 m_nDMLandVMLTextFrameRotation;

public:
    bool m_bFlyFrameGraphic = false;

    Impl(DocxExport& rExport, sax_fastparser::FSHelperPtr pSerializer,
         oox::drawingml::DrawingML* pDrawingML)
        : m_rExport(rExport)
        , m_pSerializer(std::move(pSerializer))
        , m_pDrawingML(pDrawingML)
        , m_pFlyFrameSize(nullptr)
        , m_bTextFrameSyntax(false)
        , m_bDMLTextFrameSyntax(false)
        , m_bDrawingOpen(false)
        , m_bParagraphSdtOpen(false)
        , m_bParagraphHasDrawing(false)
        , m_bDMLAndVMLDrawingOpen(false)
    {
    }

    /// Writes wp wrapper code around an SdrObject, which itself is written using drawingML syntax.

    void textFrameShadow(const SwFrameFormat& rFrameFormat);
    static bool isSupportedDMLShape(const uno::Reference<drawing::XShape>& xShape,
                                    const SdrObject* pSdrObject);

    void setSerializer(const sax_fastparser::FSHelperPtr& pSerializer)
    {
        m_pSerializer = pSerializer;
    }

    const sax_fastparser::FSHelperPtr& getSerializer() const { return m_pSerializer; }

    void setFlyFrameSize(const Size* pFlyFrameSize) { m_pFlyFrameSize = pFlyFrameSize; }

    const Size* getFlyFrameSize() const { return m_pFlyFrameSize; }

    void setTextFrameSyntax(bool bTextFrameSyntax) { m_bTextFrameSyntax = bTextFrameSyntax; }

    bool getTextFrameSyntax() const { return m_bTextFrameSyntax; }

    void setDMLTextFrameSyntax(bool bDMLTextFrameSyntax)
    {
        m_bDMLTextFrameSyntax = bDMLTextFrameSyntax;
    }

    bool getDMLTextFrameSyntax() const { return m_bDMLTextFrameSyntax; }

    void setFlyAttrList(const rtl::Reference<sax_fastparser::FastAttributeList>& pFlyAttrList)
    {
        m_pFlyAttrList = pFlyAttrList;
    }

    rtl::Reference<sax_fastparser::FastAttributeList>& getFlyAttrList() { return m_pFlyAttrList; }

    void
    setTextboxAttrList(const rtl::Reference<sax_fastparser::FastAttributeList>& pTextboxAttrList)
    {
        m_pTextboxAttrList = pTextboxAttrList;
    }

    rtl::Reference<sax_fastparser::FastAttributeList>& getTextboxAttrList()
    {
        return m_pTextboxAttrList;
    }

    OStringBuffer& getTextFrameStyle() { return m_aTextFrameStyle; }

    void setDrawingOpen(bool bDrawingOpen) { m_bDrawingOpen = bDrawingOpen; }

    bool getDrawingOpen() const { return m_bDrawingOpen; }

    void setParagraphSdtOpen(bool bParagraphSdtOpen) { m_bParagraphSdtOpen = bParagraphSdtOpen; }

    bool getParagraphSdtOpen() const { return m_bParagraphSdtOpen; }

    void setDMLAndVMLDrawingOpen(bool bDMLAndVMLDrawingOpen)
    {
        m_bDMLAndVMLDrawingOpen = bDMLAndVMLDrawingOpen;
    }

    bool getDMLAndVMLDrawingOpen() const { return m_bDMLAndVMLDrawingOpen; }

    void setParagraphHasDrawing(bool bParagraphHasDrawing)
    {
        m_bParagraphHasDrawing = bParagraphHasDrawing;
    }

    bool getParagraphHasDrawing() const { return m_bParagraphHasDrawing; }

    rtl::Reference<sax_fastparser::FastAttributeList>& getFlyFillAttrList()
    {
        return m_pFlyFillAttrList;
    }

    void
    setFlyWrapAttrList(rtl::Reference<sax_fastparser::FastAttributeList> const& pFlyWrapAttrList)
    {
        m_pFlyWrapAttrList = pFlyWrapAttrList;
    }

    sax_fastparser::FastAttributeList* getFlyWrapAttrList() const
    {
        return m_pFlyWrapAttrList.get();
    }

    void setBodyPrAttrList(sax_fastparser::FastAttributeList* pBodyPrAttrList)
    {
        m_pBodyPrAttrList = pBodyPrAttrList;
    }

    sax_fastparser::FastAttributeList* getBodyPrAttrList() const { return m_pBodyPrAttrList.get(); }

    rtl::Reference<sax_fastparser::FastAttributeList>& getDashLineStyleAttr()
    {
        return m_pDashLineStyleAttr;
    }

    bool getFlyFrameGraphic() const { return m_bFlyFrameGraphic; }

    oox::drawingml::DrawingML* getDrawingML() const { return m_pDrawingML; }

    DocxExport& getExport() const { return m_rExport; }

    void setDMLandVMLTextFrameRotation(Degree100 nDMLandVMLTextFrameRotation)
    {
        m_nDMLandVMLTextFrameRotation = nDMLandVMLTextFrameRotation;
    }

    Degree100& getDMLandVMLTextFrameRotation() { return m_nDMLandVMLTextFrameRotation; }
};

DocxSdrExport::DocxSdrExport(DocxExport& rExport, const sax_fastparser::FSHelperPtr& pSerializer,
                             oox::drawingml::DrawingML* pDrawingML)
    : m_pImpl(std::make_unique<Impl>(rExport, pSerializer, pDrawingML))
{
}

DocxSdrExport::~DocxSdrExport() = default;

void DocxSdrExport::setSerializer(const sax_fastparser::FSHelperPtr& pSerializer)
{
    m_pImpl->setSerializer(pSerializer);
}

const Size* DocxSdrExport::getFlyFrameSize() const { return m_pImpl->getFlyFrameSize(); }

bool DocxSdrExport::getTextFrameSyntax() const { return m_pImpl->getTextFrameSyntax(); }

bool DocxSdrExport::getDMLTextFrameSyntax() const { return m_pImpl->getDMLTextFrameSyntax(); }

rtl::Reference<sax_fastparser::FastAttributeList>& DocxSdrExport::getFlyAttrList()
{
    return m_pImpl->getFlyAttrList();
}

rtl::Reference<sax_fastparser::FastAttributeList>& DocxSdrExport::getTextboxAttrList()
{
    return m_pImpl->getTextboxAttrList();
}

OStringBuffer& DocxSdrExport::getTextFrameStyle() { return m_pImpl->getTextFrameStyle(); }

bool DocxSdrExport::IsDrawingOpen() const { return m_pImpl->getDrawingOpen(); }

void DocxSdrExport::setParagraphSdtOpen(bool bParagraphSdtOpen)
{
    m_pImpl->setParagraphSdtOpen(bParagraphSdtOpen);
}

bool DocxSdrExport::IsDMLAndVMLDrawingOpen() const { return m_pImpl->getDMLAndVMLDrawingOpen(); }

bool DocxSdrExport::IsParagraphHasDrawing() const { return m_pImpl->getParagraphHasDrawing(); }

void DocxSdrExport::setParagraphHasDrawing(bool bParagraphHasDrawing)
{
    m_pImpl->setParagraphHasDrawing(bParagraphHasDrawing);
}

rtl::Reference<sax_fastparser::FastAttributeList>& DocxSdrExport::getFlyFillAttrList()
{
    return m_pImpl->getFlyFillAttrList();
}

sax_fastparser::FastAttributeList* DocxSdrExport::getBodyPrAttrList()
{
    return m_pImpl->getBodyPrAttrList();
}

rtl::Reference<sax_fastparser::FastAttributeList>& DocxSdrExport::getDashLineStyle()
{
    return m_pImpl->getDashLineStyleAttr();
}

void DocxSdrExport::setFlyWrapAttrList(
    rtl::Reference<sax_fastparser::FastAttributeList> const& pAttrList)
{
    m_pImpl->setFlyWrapAttrList(pAttrList);
}

void DocxSdrExport::startDMLAnchorInline(const SwFrameFormat* pFrameFormat, const Size& rSize)
{
    const SwFormatSurround& rSurround(pFrameFormat->GetSurround());

    // Word uses size excluding right edge. Caller writeDMLDrawing and writeDiagram are changed for
    // now. ToDo: Look whether the other callers give the size this way.
    m_pImpl->setDrawingOpen(true);
    m_pImpl->setParagraphHasDrawing(true);
    m_pImpl->getSerializer()->startElementNS(XML_w, XML_drawing);
    const SdrObject* pObj = pFrameFormat->FindRealSdrObject();

    // LO determines the place needed for the object from wrap type, wrap margin ('distance to text'),
    // object type and anchor type. Word uses dist* for user set margins and effectExtent for place
    // needed for effects like shadow and glow, for fat stroke and for rotation. We map the LO values
    // to values needed by Word so that the appearance is the same as far as possible.
    // All values in Twips, change to EMU is done immediately before writing out.

    bool isAnchor; // true XML_anchor, false XML_inline
    if (m_pImpl->getFlyFrameGraphic())
    {
        isAnchor = false; // make Graphic object inside DMLTextFrame & VMLTextFrame as Inline
    }
    else
    {
        isAnchor = pFrameFormat->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR;
    }

    // tdf#135047: It must be allowed to find in parents too, but default value of bInP parameter
    // for GetLRSpace() and GetULSpace() is true, so no direct setting is required.
    const SvxLRSpaceItem& aLRSpaceItem = pFrameFormat->GetLRSpace();
    const SvxULSpaceItem& aULSpaceItem = pFrameFormat->GetULSpace();
    sal_Int64 nDistT = aULSpaceItem.GetUpper();
    sal_Int64 nDistB = aULSpaceItem.GetLower();
    sal_Int64 nDistL = aLRSpaceItem.GetLeft();
    sal_Int64 nDistR = aLRSpaceItem.GetRight();

    // LibreOffice behaves different for frames and drawing objects, but MS Office treats frames
    // as drawing objects too. Therefore we transform the values from frame so as if they come
    // from a drawing object.
    sal_Int32 nWidthDiff(0);
    sal_Int32 nHeightDiff(0);
    sal_Int32 nPosXDiff(0);
    sal_Int32 nPosYDiff(0);
    sal_Int32 nLeftExt(0);
    sal_Int32 nRightExt(0);
    sal_Int32 nTopExt(0);
    sal_Int32 nBottomExt(0);

    if ((!pObj) || (pObj && (pObj->GetObjIdentifier() == SdrObjKind::SwFlyDrawObjIdentifier)))
    {
        // Frame objects have a restricted shadow and no further effects. They have border instead of
        // stroke. LO includes shadow and border in the object size, but Word not.
        SvxShadowItem aShadowItem = pFrameFormat->GetShadow();
        if (aShadowItem.GetLocation() != SvxShadowLocation::NONE)
        {
            sal_Int32 nShadowWidth(aShadowItem.GetWidth());
            switch (aShadowItem.GetLocation())
            {
                case SvxShadowLocation::TopLeft:
                    nTopExt = nLeftExt = nShadowWidth;
                    nPosXDiff = nLeftExt; // actual move is postponed
                    nPosYDiff = nTopExt;
                    nWidthDiff = -nLeftExt; // actual size extent is postponed
                    nHeightDiff = -nTopExt;
                    break;
                case SvxShadowLocation::TopRight:
                    nTopExt = nRightExt = nShadowWidth;
                    nPosYDiff = nTopExt;
                    nWidthDiff = -nRightExt;
                    nHeightDiff = -nTopExt;
                    break;
                case SvxShadowLocation::BottomLeft:
                    nBottomExt = nLeftExt = nShadowWidth;
                    nPosXDiff = nLeftExt;
                    nWidthDiff = -nLeftExt;
                    nHeightDiff = -nBottomExt;
                    break;
                case SvxShadowLocation::BottomRight:
                    nBottomExt = nRightExt = nShadowWidth;
                    nWidthDiff = -nRightExt;
                    nHeightDiff = -nBottomExt;
                    break;
                case SvxShadowLocation::NONE:
                case SvxShadowLocation::End:
                    break;
            }
        }
        // ToDo: Position refers to outer edge of border in LO, but to center of border in Word.
        // Adaption is missing here. Frames in LO have no stroke but border. The current conversion
        // from border to line treats borders like table borders. That might give wrong values
        // for drawing frames.

        if (pObj && pObj->GetRotateAngle() != 0_deg100)
        {
            Degree100 nRotation = pObj->GetRotateAngle();
            const SwRect aBoundRect(pFrameFormat->FindLayoutRect());
            tools::Long nMSOWidth = rSize.Width();
            tools::Long nMSOHeight = rSize.Height();
            if ((nRotation > 4500_deg100 && nRotation <= 13500_deg100)
                || (nRotation > 22500_deg100 && nRotation <= 31500_deg100))
                std::swap(nMSOWidth, nMSOHeight);
            nBottomExt += (aBoundRect.Height() - 1 - nMSOHeight) / 2;
            nTopExt += (aBoundRect.Height() - 1 - nMSOHeight) / 2;
            nLeftExt += (aBoundRect.Width() - nMSOWidth) / 2;
            nRightExt += (aBoundRect.Width() - nMSOWidth) / 2;
        }
        lcl_makeDistAndExtentNonNegative(nDistT, nDistB, nDistL, nDistR, nLeftExt, nTopExt,
                                         nRightExt, nBottomExt);

        // ToDo: Inline rotated image fails because it would need wrapTight, what is not possible.
        // ToDo: Image plus shadow fails because of wrong shadow direction.
    }
    else // other objects than frames. pObj exists.
    {
        // Word 2007 makes no width-height-swap for images. Detect this situation.
        sal_Int32 nMode = m_pImpl->getExport().getWordCompatibilityModeFromGrabBag();
        bool bIsWord2007Image(nMode > 0 && nMode < 14
                              && pObj->GetObjIdentifier() == SdrObjKind::Graphic);

        // Word cannot handle negative EffectExtent although allowed in OOXML, the 'dist' attributes
        // may not be negative. Take care of that.
        if (isAnchor)
        {
            lcl_calculateRawEffectExtent(nLeftExt, nTopExt, nRightExt, nBottomExt, *pObj, true,
                                         bIsWord2007Image);
            // We have calculated the effectExtent from boundRect, therefore half stroke width is
            // already contained.
            // ToDo: The other half of the stroke width needs to be subtracted from padding.
            //       Where is that?

            // The import has added a difference to dist* in case of contour wrap for to give a
            // rendering nearer to Word. In that case, we need to subtract it on export.
            uno::Any aAny;
            pObj->GetGrabBagItem(aAny);
            comphelper::SequenceAsHashMap aGrabBag(aAny);
            auto it = aGrabBag.find(u"AnchorDistDiff"_ustr);
            if (it != aGrabBag.end())
            {
                comphelper::SequenceAsHashMap aAnchorDistDiff(it->second);
                for (const std::pair<const comphelper::OUStringAndHashCode, uno::Any>& rDiff :
                     aAnchorDistDiff)
                {
                    const OUString& rName = rDiff.first.maString;
                    if (rName == "distTDiff" && rDiff.second.has<sal_Int32>())
                        nDistT -= round(rDiff.second.get<sal_Int32>());
                    else if (rName == "distBDiff" && rDiff.second.has<sal_Int32>())
                        nDistB -= round(rDiff.second.get<sal_Int32>());
                    else if (rName == "distLDiff" && rDiff.second.has<sal_Int32>())
                        nDistL -= rDiff.second.get<sal_Int32>();
                    else if (rName == "distRDiff" && rDiff.second.has<sal_Int32>())
                        nDistR -= rDiff.second.get<sal_Int32>();
                }
            }
            // ToDo: bool bCompansated = ... to be later able to switch from wrapSquare to wrapTight,
            //       if wrapSquare would require negative effectExtent.
            lcl_makeDistAndExtentNonNegative(nDistT, nDistB, nDistL, nDistR, nLeftExt, nTopExt,
                                             nRightExt, nBottomExt);
        }
        else
        {
            lcl_calculateRawEffectExtent(nLeftExt, nTopExt, nRightExt, nBottomExt, *pObj, false,
                                         bIsWord2007Image);
            // nDistT,... contain the needed distances from import or set by user. But Word
            // ignores Dist attributes of inline shapes. So we move all needed distances to
            // effectExtent and force effectExtent to non-negative.
            lcl_makeDistZeroAndExtentNonNegative(nDistT, nDistB, nDistL, nDistR, nLeftExt, nTopExt,
                                                 nRightExt, nBottomExt);
        }
    }

    if (isAnchor)
    {
        rtl::Reference<sax_fastparser::FastAttributeList> attrList
            = sax_fastparser::FastSerializerHelper::createAttrList();

        bool bOpaque = pFrameFormat->GetOpaque().GetValue();
        if (pObj)
        {
            // SdrObjects know their layer, consider that instead of the frame format.
            const IDocumentDrawModelAccess& iDocumentDrawModelAccess
                = pFrameFormat->GetDoc()->getIDocumentDrawModelAccess();
            bOpaque = pObj->GetLayer() != iDocumentDrawModelAccess.GetHellId()
                      && pObj->GetLayer() != iDocumentDrawModelAccess.GetHeaderFooterHellId()
                      && pObj->GetLayer() != iDocumentDrawModelAccess.GetInvisibleHellId();
        }
        attrList->add(XML_behindDoc, bOpaque ? "0" : "1");

        attrList->add(XML_distT, OString::number(TwipsToEMU(nDistT)));
        attrList->add(XML_distB, OString::number(TwipsToEMU(nDistB)));
        attrList->add(XML_distL, OString::number(TwipsToEMU(nDistL)));
        attrList->add(XML_distR, OString::number(TwipsToEMU(nDistR)));

        attrList->add(XML_simplePos, "0");
        attrList->add(XML_locked, "0");

        bool bLclInTabCell = true;
        if (pObj)
        {
            uno::Reference<drawing::XShape> xShape((const_cast<SdrObject*>(pObj)->getUnoShape()),
                                                   uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
            if (xShapeProps.is())
                xShapeProps->getPropertyValue(u"IsFollowingTextFlow"_ustr) >>= bLclInTabCell;
        }

        const SwFormatHoriOrient& rHoriOri(pFrameFormat->GetHoriOrient());
        if (rSurround.GetValue() == text::WrapTextMode_THROUGH
            && rHoriOri.GetRelationOrient() == text::RelOrientation::FRAME)
        {
            // "In front of text" and horizontal positioning relative to Column is ignored on
            // import, add it back here.
            bLclInTabCell = true;
        }

        if (bLclInTabCell)
            attrList->add(XML_layoutInCell, "1");
        else
            attrList->add(XML_layoutInCell, "0");

        bool bAllowOverlap = pFrameFormat->GetWrapInfluenceOnObjPos().GetAllowOverlap();
        attrList->add(XML_allowOverlap, bAllowOverlap ? "1" : "0");

        if (pObj)
            // It seems 0 and 1 have special meaning: just start counting from 2 to avoid issues with that.
            attrList->add(XML_relativeHeight, OString::number(pObj->GetOrdNum() + 2));
        else
            // relativeHeight is mandatory attribute, if value is not present, we must write default value
            attrList->add(XML_relativeHeight, "0");

        if (pObj)
        {
            OUString sAnchorId = lclGetAnchorIdFromGrabBag(pObj);
            if (!sAnchorId.isEmpty())
                attrList->addNS(XML_wp14, XML_anchorId, sAnchorId);
        }

        m_pImpl->getSerializer()->startElementNS(XML_wp, XML_anchor, attrList);

        m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_simplePos, XML_x, "0", XML_y,
                                                  "0"); // required, unused

        // Position is either determined by coordinates aPos or alignment keywords like 'center'.
        // First prepare them.
        const SwFormatVertOrient& rVertOri(pFrameFormat->GetVertOrient());
        awt::Point aPos(rHoriOri.GetPos(), rVertOri.GetPos());

        aPos.X += nPosXDiff; // Make the postponed position move of frames.
        aPos.Y += nPosYDiff;
        if (pObj && lcl_IsRotateAngleValid(*pObj)
            && pObj->GetObjIdentifier() != SdrObjKind::SwFlyDrawObjIdentifier)
            lclMovePositionWithRotation(aPos, rSize, pObj->GetRotateAngle());

        const char* relativeFromH;
        const char* relativeFromV;
        const char* alignH = nullptr;
        const char* alignV = nullptr;
        switch (rVertOri.GetRelationOrient())
        {
            case text::RelOrientation::PAGE_PRINT_AREA:
                relativeFromV = "margin";
                break;
            case text::RelOrientation::PAGE_PRINT_AREA_TOP:
                relativeFromV = "topMargin";
                break;
            case text::RelOrientation::PAGE_PRINT_AREA_BOTTOM:
                relativeFromV = "bottomMargin";
                break;
            case text::RelOrientation::PAGE_FRAME:
                relativeFromV = "page";
                break;
            case text::RelOrientation::FRAME:
                relativeFromV = "paragraph";
                break;
            case text::RelOrientation::TEXT_LINE:
                relativeFromV = "line";
                // Word's "line" is "below the bottom of the line", our TEXT_LINE is
                // "towards top, from the bottom of the line", so invert the vertical position.
                aPos.Y *= -1;
                break;
            default:
                relativeFromV = "line";
                break;
        }
        switch (rVertOri.GetVertOrient())
        {
            case text::VertOrientation::TOP:
            case text::VertOrientation::CHAR_TOP:
            case text::VertOrientation::LINE_TOP:
                if (rVertOri.GetRelationOrient() == text::RelOrientation::TEXT_LINE)
                    alignV = "bottom";
                else
                    alignV = "top";
                break;
            case text::VertOrientation::BOTTOM:
            case text::VertOrientation::CHAR_BOTTOM:
            case text::VertOrientation::LINE_BOTTOM:
                if (rVertOri.GetRelationOrient() == text::RelOrientation::TEXT_LINE)
                    alignV = "top";
                else
                    alignV = "bottom";
                break;
            case text::VertOrientation::CENTER:
            case text::VertOrientation::CHAR_CENTER:
            case text::VertOrientation::LINE_CENTER:
                alignV = "center";
                break;
            default:
                break;
        }
        switch (rHoriOri.GetRelationOrient())
        {
            case text::RelOrientation::PAGE_PRINT_AREA:
                relativeFromH = "margin";
                break;
            case text::RelOrientation::PAGE_FRAME:
                relativeFromH = "page";
                break;
            case text::RelOrientation::CHAR:
                relativeFromH = "character";
                break;
            case text::RelOrientation::PAGE_RIGHT:
                relativeFromH = "rightMargin";
                break;
            case text::RelOrientation::PAGE_LEFT:
                relativeFromH = "leftMargin";
                break;
            case text::RelOrientation::FRAME:
            default:
                relativeFromH = "column";
                break;
        }
        switch (rHoriOri.GetHoriOrient())
        {
            case text::HoriOrientation::LEFT:
                alignH = "left";
                break;
            case text::HoriOrientation::RIGHT:
                alignH = "right";
                break;
            case text::HoriOrientation::CENTER:
                alignH = "center";
                break;
            case text::HoriOrientation::INSIDE:
                alignH = "inside";
                break;
            case text::HoriOrientation::OUTSIDE:
                alignH = "outside";
                break;
            default:
                break;
        }

        // write out horizontal position
        m_pImpl->getSerializer()->startElementNS(XML_wp, XML_positionH, XML_relativeFrom,
                                                 relativeFromH);

        /**
        * Sizes of integral types
        * climits header defines constants with the limits of integral types for the specific system and compiler implementation used.
        * Use of this might cause platform dependent problem like posOffset exceed the limit.
        **/
        const sal_Int64 MAX_INTEGER_VALUE = SAL_MAX_INT32;
        const sal_Int64 MIN_INTEGER_VALUE = SAL_MIN_INT32;

        if (alignH != nullptr)
        {
            m_pImpl->getSerializer()->startElementNS(XML_wp, XML_align);
            m_pImpl->getSerializer()->write(alignH);
            m_pImpl->getSerializer()->endElementNS(XML_wp, XML_align);
        }
        else
        {
            m_pImpl->getSerializer()->startElementNS(XML_wp, XML_posOffset);
            sal_Int64 nPosXEMU = TwipsToEMU(aPos.X);

            /* Absolute Position Offset Value is of type Int. Hence it should not be greater than
             * Maximum value for Int OR Less than the Minimum value for Int.
             * - Maximum value for Int = 2147483647
             * - Minimum value for Int = -2147483648
             *
             * As per ECMA Specification : ECMA-376, Second Edition,
             * Part 1 - Fundamentals And Markup Language Reference[20.4.3.3 ST_PositionOffset (Absolute Position Offset Value)]
             *
             * Please refer : http://www.schemacentral.com/sc/xsd/t-xsd_int.html
             */

            if (nPosXEMU > MAX_INTEGER_VALUE)
            {
                nPosXEMU = MAX_INTEGER_VALUE;
            }
            else if (nPosXEMU < MIN_INTEGER_VALUE)
            {
                nPosXEMU = MIN_INTEGER_VALUE;
            }
            m_pImpl->getSerializer()->write(nPosXEMU);
            m_pImpl->getSerializer()->endElementNS(XML_wp, XML_posOffset);
        }
        m_pImpl->getSerializer()->endElementNS(XML_wp, XML_positionH);

        // write out vertical position
        m_pImpl->getSerializer()->startElementNS(XML_wp, XML_positionV, XML_relativeFrom,
                                                 relativeFromV);
        sal_Int64 nPosYEMU = TwipsToEMU(aPos.Y);

        // tdf#93675, 0 below line/paragraph and/or top line/paragraph with
        // wrap top+bottom or other wraps is affecting the line directly
        // above the anchor line, which seems odd, but a tiny adjustment
        // here to bring the top down convinces msoffice to wrap like us
        if (nPosYEMU == 0
            && (strcmp(relativeFromV, "line") == 0 || strcmp(relativeFromV, "paragraph") == 0)
            && (!alignV || strcmp(alignV, "top") == 0))
        {
            alignV = nullptr;
            nPosYEMU = TwipsToEMU(1);
        }

        if (alignV != nullptr)
        {
            m_pImpl->getSerializer()->startElementNS(XML_wp, XML_align);
            m_pImpl->getSerializer()->write(alignV);
            m_pImpl->getSerializer()->endElementNS(XML_wp, XML_align);
        }
        else
        {
            m_pImpl->getSerializer()->startElementNS(XML_wp, XML_posOffset);
            if (nPosYEMU > MAX_INTEGER_VALUE)
            {
                nPosYEMU = MAX_INTEGER_VALUE;
            }
            else if (nPosYEMU < MIN_INTEGER_VALUE)
            {
                nPosYEMU = MIN_INTEGER_VALUE;
            }
            m_pImpl->getSerializer()->write(nPosYEMU);
            m_pImpl->getSerializer()->endElementNS(XML_wp, XML_posOffset);
        }
        m_pImpl->getSerializer()->endElementNS(XML_wp, XML_positionV);
    }
    else // inline
    {
        // nDist is forced to zero above and ignored by Word anyway, so write 0 directly.
        rtl::Reference<sax_fastparser::FastAttributeList> aAttrList
            = sax_fastparser::FastSerializerHelper::createAttrList();
        aAttrList->add(XML_distT, OString::number(0));
        aAttrList->add(XML_distB, OString::number(0));
        aAttrList->add(XML_distL, OString::number(0));
        aAttrList->add(XML_distR, OString::number(0));
        if (pObj)
        {
            OUString sAnchorId = lclGetAnchorIdFromGrabBag(pObj);
            if (!sAnchorId.isEmpty())
                aAttrList->addNS(XML_wp14, XML_anchorId, sAnchorId);
        }
        m_pImpl->getSerializer()->startElementNS(XML_wp, XML_inline, aAttrList);
    }

    // now the common parts 'extent' and 'effectExtent'
    /**
    * Extent width is of type long ( i.e cx & cy ) as
    *
    * per ECMA-376, Second Edition, Part 1 - Fundamentals And Markup Language Reference
    * [ 20.4.2.7 extent (Drawing Object Size)]
    *
    * cy is of type a:ST_PositiveCoordinate.
    * Minimum inclusive: 0
    * Maximum inclusive: 27273042316900
    *
    * reference : http://www.schemacentral.com/sc/ooxml/e-wp_extent-1.html
    *
    *   Though ECMA mentions the max value as aforementioned. It appears that MSO does not
    *  handle for the same, in fact it actually can handle a max value of int32 i.e
    *   2147483647( MAX_INTEGER_VALUE ).
    *  Therefore changing the following accordingly so that LO sync's up with MSO.
    **/
    sal_uInt64 cx = TwipsToEMU(
        std::clamp(rSize.Width() + nWidthDiff, tools::Long(0), tools::Long(SAL_MAX_INT32)));
    OString aWidth(OString::number(std::min(cx, sal_uInt64(SAL_MAX_INT32))));
    sal_uInt64 cy = TwipsToEMU(
        std::clamp(rSize.Height() + nHeightDiff, tools::Long(0), tools::Long(SAL_MAX_INT32)));
    OString aHeight(OString::number(std::min(cy, sal_uInt64(SAL_MAX_INT32))));

    m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_extent, XML_cx, aWidth, XML_cy, aHeight);

    // XML_effectExtent, includes effects, fat stroke and rotation
    // FixMe: tdf141880. Because LibreOffice currently cannot handle negative vertical margins, they
    // were forced to zero on import. Especially bottom margin of inline anchored rotated objects are
    // affected. If the object was not changed, it would be better to export the original values
    // from grab-Bag. Unfortunately there exists no marker for "not changed", so a heuristic is used
    // here: If current left, top and right margins do not differ more than 1Hmm = 635EMU from the
    // values in grab-Bag, it is likely, that the object was not changed and we restore the values
    // from grab-Bag.
    sal_Int64 nLeftExtEMU = TwipsToEMU(nLeftExt);
    sal_Int64 nTopExtEMU = TwipsToEMU(nTopExt);
    sal_Int64 nRightExtEMU = TwipsToEMU(nRightExt);
    sal_Int64 nBottomExtEMU = TwipsToEMU(nBottomExt);
    if (pObj)
    {
        uno::Any aAny;
        pObj->GetGrabBagItem(aAny);
        comphelper::SequenceAsHashMap aGrabBag(aAny);
        auto it = aGrabBag.find(u"CT_EffectExtent"_ustr);
        if (it != aGrabBag.end())
        {
            comphelper::SequenceAsHashMap aEffectExtent(it->second);
            sal_Int64 nLeftExtGrabBag(0);
            sal_Int64 nTopExtGrabBag(0);
            sal_Int64 nRightExtGrabBag(0);
            sal_Int64 nBottomExtGrabBag(0);
            for (const std::pair<const comphelper::OUStringAndHashCode, uno::Any>& rDirection :
                 aEffectExtent)
            {
                const OUString& rName = rDirection.first.maString;
                if (rName == "l" && rDirection.second.has<sal_Int32>())
                    nLeftExtGrabBag = rDirection.second.get<sal_Int32>();
                else if (rName == "t" && rDirection.second.has<sal_Int32>())
                    nTopExtGrabBag = rDirection.second.get<sal_Int32>();
                else if (rName == "r" && rDirection.second.has<sal_Int32>())
                    nRightExtGrabBag = rDirection.second.get<sal_Int32>();
                else if (rName == "b" && rDirection.second.has<sal_Int32>())
                    nBottomExtGrabBag = rDirection.second.get<sal_Int32>();
            }
            if (abs(nLeftExtEMU - nLeftExtGrabBag) <= 635 && abs(nTopExtEMU - nTopExtGrabBag) <= 635
                && abs(nRightExtEMU - nRightExtGrabBag) <= 635)
            {
                nLeftExtEMU = nLeftExtGrabBag;
                nTopExtEMU = nTopExtGrabBag;
                nRightExtEMU = nRightExtGrabBag;
                nBottomExtEMU = nBottomExtGrabBag;
            }
        }
    }
    m_pImpl->getSerializer()->singleElementNS(
        XML_wp, XML_effectExtent, XML_l, OString::number(nLeftExtEMU), XML_t,
        OString::number(nTopExtEMU), XML_r, OString::number(nRightExtEMU), XML_b,
        OString::number(nBottomExtEMU));

    if (!isAnchor)
        return; // OOXML 'inline' has not wrap type at all

    // XML_anchor has exact one of types wrapNone, wrapSquare, wrapTight, wrapThrough and
    // WrapTopAndBottom. Map our own types to them as far as possible.

    if (rSurround.GetValue() == css::text::WrapTextMode_THROUGH
        || rSurround.GetValue() == css::text::WrapTextMode_THROUGHT)
    {
        m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_wrapNone);
        return;
    }

    if (rSurround.GetValue() == css::text::WrapTextMode_NONE)
    {
        m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_wrapTopAndBottom);
        return;
    }

    // All remaining cases need attribute XML_wrapText
    OUString sWrapType;
    switch (rSurround.GetSurround())
    {
        case text::WrapTextMode_DYNAMIC:
            sWrapType = u"largest"_ustr;
            break;
        case text::WrapTextMode_LEFT:
            sWrapType = u"left"_ustr;
            break;
        case text::WrapTextMode_RIGHT:
            sWrapType = u"right"_ustr;
            break;
        case text::WrapTextMode_PARALLEL:
        default:
            sWrapType = u"bothSides"_ustr;
            break;
    }

    // ToDo: Exclude cases where LibreOffice wrap without contour is different
    // from Word XML_wrapSquare or where direct use of distances not possible and workaround
    // will be done using wrapPolygon.
    // ToDo: handle case Writer frame, where contour can be set in LibreOffice but is not rendered.

    // This case needs no wrapPolygon
    if (!rSurround.IsContour())
    {
        m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_wrapSquare, XML_wrapText, sWrapType);
        return;
    }

    // Contour wrap.
    const sal_Int32 nWrapToken = rSurround.IsOutside() ? XML_wrapTight : XML_wrapThrough;

    // ToDo: cases where wrapPolygon is used as workaround.

    // Own wrap polygon exists only for TextGraphicObject and TextEmbeddedObject. It might be edited
    // by user. If such exists, we use it and we are done.
    if (const SwNoTextNode* pNd = sw::util::GetNoTextNodeFromSwFrameFormat(*pFrameFormat))
    {
        const tools::PolyPolygon* pPolyPoly = pNd->HasContour();
        if (pPolyPoly && pPolyPoly->Count())
        {
            tools::Polygon aPoly
                = sw::util::CorrectWordWrapPolygonForExport(*pPolyPoly, pNd, /*bCorrectCrop=*/true);
            if (aPoly.GetSize() >= 3)
            {
                m_pImpl->getSerializer()->startElementNS(XML_wp, nWrapToken, XML_wrapText,
                                                         sWrapType);
                // ToDo: Test whether XML_edited true or false gives better results.
                m_pImpl->getSerializer()->startElementNS(XML_wp, XML_wrapPolygon, XML_edited, "0");
                m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_start, XML_x,
                                                          OString::number(aPoly[0].X()), XML_y,
                                                          OString::number(aPoly[0].Y()));
                for (sal_uInt16 i = 1; i < aPoly.GetSize(); ++i)
                    m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_lineTo, XML_x,
                                                              OString::number(aPoly[i].X()), XML_y,
                                                              OString::number(aPoly[i].Y()));
                m_pImpl->getSerializer()->endElementNS(XML_wp, XML_wrapPolygon);

                m_pImpl->getSerializer()->endElementNS(XML_wp, nWrapToken);
                return;
            }
        }
    }

    // If this shape comes from ooxml import, there might be a wrap polygon in InteropGrabBag.
    // Wrap polygons can be edited by users in Word. They are independent from changing shape size or
    // rotation. So it is likely, that it is still usable.
    if (pObj)
    {
        uno::Any aAny;
        pObj->GetGrabBagItem(aAny);
        comphelper::SequenceAsHashMap aGrabBag(aAny);
        auto it = aGrabBag.find(u"CT_WrapPath"_ustr);
        if (it != aGrabBag.end())
        {
            m_pImpl->getSerializer()->startElementNS(XML_wp, nWrapToken, XML_wrapText, sWrapType);

            m_pImpl->getSerializer()->startElementNS(XML_wp, XML_wrapPolygon, XML_edited, "0");
            auto aSeqSeq = it->second.get<drawing::PointSequenceSequence>();
            const auto& rPoints = aSeqSeq[0];
            for (auto i = rPoints.begin(); i != rPoints.end(); ++i)
            {
                const awt::Point& rPoint = *i;
                m_pImpl->getSerializer()->singleElementNS(
                    XML_wp, (i == rPoints.begin() ? XML_start : XML_lineTo), XML_x,
                    OString::number(rPoint.X), XML_y, OString::number(rPoint.Y));
            }
            m_pImpl->getSerializer()->endElementNS(XML_wp, XML_wrapPolygon);

            m_pImpl->getSerializer()->endElementNS(XML_wp, nWrapToken);
            return;
        }
    }

    // In this case we likely had an odt document to be exported to docx. ODF does not know the
    // concept of a wrap polygon and LibreOffice has no one internally. So as a workaround, we
    // generate a wrap polygon from the shape geometry.
    tools::Polygon aContour = lcl_CreateContourPolygon(const_cast<SdrObject*>(pObj));

    // lcl_CreateContourPolygon() ensures at least three points
    m_pImpl->getSerializer()->startElementNS(XML_wp, nWrapToken, XML_wrapText, sWrapType);

    // ToDo: Test whether XML_edited true or false gives better results.
    m_pImpl->getSerializer()->startElementNS(XML_wp, XML_wrapPolygon, XML_edited, "0");
    m_pImpl->getSerializer()->singleElementNS(XML_wp, XML_start, XML_x,
                                              OString::number(aContour.GetPoint(0).getX()), XML_y,
                                              OString::number(aContour.GetPoint(0).getY()));
    for (sal_uInt32 i = 1; i < aContour.GetSize(); i++)
        m_pImpl->getSerializer()->singleElementNS(
            XML_wp, XML_lineTo, XML_x, OString::number(aContour.GetPoint(i).getX()), XML_y,
            OString::number(aContour.GetPoint(i).getY()));
    m_pImpl->getSerializer()->endElementNS(XML_wp, XML_wrapPolygon);

    m_pImpl->getSerializer()->endElementNS(XML_wp, nWrapToken);
}

void DocxSdrExport::endDMLAnchorInline(const SwFrameFormat* pFrameFormat)
{
    bool isAnchor;
    if (m_pImpl->getFlyFrameGraphic())
    {
        isAnchor = false; // end Inline Graphic object inside DMLTextFrame
    }
    else
    {
        isAnchor = pFrameFormat->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR;
    }
    m_pImpl->getSerializer()->endElementNS(XML_wp, isAnchor ? XML_anchor : XML_inline);

    m_pImpl->getSerializer()->endElementNS(XML_w, XML_drawing);
    m_pImpl->setDrawingOpen(false);
}

void DocxSdrExport::writeVMLDrawing(const SdrObject* sdrObj, const SwFrameFormat& rFrameFormat)
{
    m_pImpl->getSerializer()->startElementNS(XML_w, XML_pict);
    m_pImpl->getDrawingML()->SetFS(m_pImpl->getSerializer());
    // See WinwordAnchoring::SetAnchoring(), these are not part of the SdrObject, have to be passed around manually.

    SwFormatFollowTextFlow const& rFlow(rFrameFormat.GetFollowTextFlow());
    const SwFormatHoriOrient& rHoriOri = rFrameFormat.GetHoriOrient();
    const SwFormatVertOrient& rVertOri = rFrameFormat.GetVertOrient();
    SwFormatSurround const& rSurround(rFrameFormat.GetSurround());

    rtl::Reference<sax_fastparser::FastAttributeList> pAttrList(docx::SurroundToVMLWrap(rSurround));
    m_pImpl->getExport().VMLExporter().AddSdrObject(
        *sdrObj, rFlow.GetValue(), rHoriOri.GetHoriOrient(), rVertOri.GetVertOrient(),
        rHoriOri.GetRelationOrient(), rVertOri.GetRelationOrient(), pAttrList.get(), true);
    m_pImpl->getSerializer()->endElementNS(XML_w, XML_pict);
}

static bool lcl_isLockedCanvas(const uno::Reference<drawing::XShape>& xShape)
{
    const uno::Sequence<beans::PropertyValue> propList
        = lclGetProperty(xShape, u"InteropGrabBag"_ustr);
    /*
     * Export as Locked Canvas only if the property
     * is in the PropertySet
     */
    return std::any_of(propList.begin(), propList.end(), [](const beans::PropertyValue& rProp) {
        return rProp.Name == "LockedCanvas";
    });
}

void AddExtLst(sax_fastparser::FSHelperPtr const& pFS, DocxExport const& rExport,
               uno::Reference<beans::XPropertySet> const& xShape)
{
    if (xShape->getPropertyValue(u"Decorative"_ustr).get<bool>())
    {
        pFS->startElementNS(XML_a, XML_extLst,
                            // apparently for DOCX the namespace isn't declared on the root
                            FSNS(XML_xmlns, XML_a),
                            rExport.GetFilter().getNamespaceURL(OOX_NS(dml)));
        pFS->startElementNS(XML_a, XML_ext,
                            // Word uses this "URI" which is obviously not a URI
                            XML_uri, "{C183D7F6-B498-43B3-948B-1728B52AA6E4}");
        pFS->singleElementNS(XML_adec, XML_decorative, FSNS(XML_xmlns, XML_adec),
                             "http://schemas.microsoft.com/office/drawing/2017/decorative", XML_val,
                             "1");
        pFS->endElementNS(XML_a, XML_ext);
        pFS->endElementNS(XML_a, XML_extLst);
    }
}

void DocxSdrExport::writeDMLDrawing(const SdrObject* pSdrObject, const SwFrameFormat* pFrameFormat,
                                    int nAnchorId)
{
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pSdrObject)->getUnoShape());
    if (!Impl::isSupportedDMLShape(xShape, pSdrObject))
        return;

    m_pImpl->getExport().DocxAttrOutput().GetSdtEndBefore(pSdrObject);

    sax_fastparser::FSHelperPtr pFS = m_pImpl->getSerializer();
    Size aSize(pSdrObject->GetLogicRect().getOpenWidth(),
               pSdrObject->GetLogicRect().getOpenHeight());
    startDMLAnchorInline(pFrameFormat, aSize);

    rtl::Reference<sax_fastparser::FastAttributeList> pDocPrAttrList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    pDocPrAttrList->add(XML_id, OString::number(nAnchorId));
    pDocPrAttrList->add(XML_name, pSdrObject->GetName());
    if (!pSdrObject->GetTitle().isEmpty())
        pDocPrAttrList->add(XML_title, pSdrObject->GetTitle());
    if (!pSdrObject->GetDescription().isEmpty())
        pDocPrAttrList->add(XML_descr, pSdrObject->GetDescription());
    if (!pSdrObject->IsVisible()
        && pFrameFormat->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)

        pDocPrAttrList->add(XML_hidden, OString::number(1));

    pFS->startElementNS(XML_wp, XML_docPr, pDocPrAttrList);
    OUString sHyperlink = pSdrObject->getHyperlink();
    if (!sHyperlink.isEmpty())
    {
        OUString sRelId = m_pImpl->getExport().GetFilter().addRelation(
            pFS->getOutputStream(), oox::getRelationship(Relationship::HYPERLINK),
            oox::drawingml::URLTransformer().getTransformedString(sHyperlink),
            oox::drawingml::URLTransformer().isExternalURL(sHyperlink));
        pFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId,
                             FSNS(XML_xmlns, XML_a),
                             m_pImpl->getExport().GetFilter().getNamespaceURL(OOX_NS(dml)));
    }
    uno::Reference<beans::XPropertySet> const xShapeProps(xShape, uno::UNO_QUERY_THROW);
    AddExtLst(pFS, m_pImpl->getExport(), xShapeProps);

    pFS->endElementNS(XML_wp, XML_docPr);

    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY_THROW);
    const char* pNamespace = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    if (xServiceInfo->supportsService(u"com.sun.star.drawing.GroupShape"_ustr))
        pNamespace = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
    else if (xServiceInfo->supportsService(u"com.sun.star.drawing.GraphicObjectShape"_ustr))
        pNamespace = "http://schemas.openxmlformats.org/drawingml/2006/picture";
    pFS->startElementNS(XML_a, XML_graphic, FSNS(XML_xmlns, XML_a),
                        m_pImpl->getExport().GetFilter().getNamespaceURL(OOX_NS(dml)));
    pFS->startElementNS(XML_a, XML_graphicData, XML_uri, pNamespace);

    bool bLockedCanvas = lcl_isLockedCanvas(xShape);
    if (bLockedCanvas)
        pFS->startElementNS(
            XML_lc, XML_lockedCanvas, FSNS(XML_xmlns, XML_lc),
            m_pImpl->getExport().GetFilter().getNamespaceURL(OOX_NS(dmlLockedCanvas)));

    m_pImpl->getExport().OutputDML(xShape);

    if (bLockedCanvas)
        pFS->endElementNS(XML_lc, XML_lockedCanvas);
    pFS->endElementNS(XML_a, XML_graphicData);
    pFS->endElementNS(XML_a, XML_graphic);

    // Relative size of the drawing.
    if (pSdrObject->GetRelativeWidth())
    {
        // At the moment drawinglayer objects are always relative from page.
        OUString sValue;
        switch (pSdrObject->GetRelativeWidthRelation())
        {
            case text::RelOrientation::FRAME:
                sValue = "margin";
                break;
            case text::RelOrientation::PAGE_LEFT:
                if (pFrameFormat->GetDoc()->GetPageDesc(0).GetUseOn() == UseOnPage::Mirror)
                    sValue = "outsideMargin";
                else
                    sValue = "leftMargin";
                break;
            case text::RelOrientation::PAGE_RIGHT:
                if (pFrameFormat->GetDoc()->GetPageDesc(0).GetUseOn() == UseOnPage::Mirror)
                    sValue = "insideMargin";
                else
                    sValue = "rightMargin";
                break;
            case text::RelOrientation::PAGE_FRAME:
            default:
                sValue = "page";
                break;
        }
        pFS->startElementNS(XML_wp14, XML_sizeRelH, XML_relativeFrom, sValue);
        pFS->startElementNS(XML_wp14, XML_pctWidth);
        pFS->writeEscaped(
            OUString::number(*pSdrObject->GetRelativeWidth() * 100 * oox::drawingml::PER_PERCENT));
        pFS->endElementNS(XML_wp14, XML_pctWidth);
        pFS->endElementNS(XML_wp14, XML_sizeRelH);
    }
    if (pSdrObject->GetRelativeHeight())
    {
        OUString sValue;
        switch (pSdrObject->GetRelativeHeightRelation())
        {
            case text::RelOrientation::FRAME:
                sValue = "margin";
                break;
            case text::RelOrientation::PAGE_PRINT_AREA:
                sValue = "topMargin";
                break;
            case text::RelOrientation::PAGE_PRINT_AREA_BOTTOM:
                sValue = "bottomMargin";
                break;
            case text::RelOrientation::PAGE_FRAME:
            default:
                sValue = "page";
                break;
        }
        pFS->startElementNS(XML_wp14, XML_sizeRelV, XML_relativeFrom, sValue);
        pFS->startElementNS(XML_wp14, XML_pctHeight);
        pFS->writeEscaped(
            OUString::number(*pSdrObject->GetRelativeHeight() * 100 * oox::drawingml::PER_PERCENT));
        pFS->endElementNS(XML_wp14, XML_pctHeight);
        pFS->endElementNS(XML_wp14, XML_sizeRelV);
    }

    endDMLAnchorInline(pFrameFormat);
}

void DocxSdrExport::Impl::textFrameShadow(const SwFrameFormat& rFrameFormat)
{
    const SvxShadowItem& aShadowItem = rFrameFormat.GetShadow();
    if (aShadowItem.GetLocation() == SvxShadowLocation::NONE)
        return;

    OString aShadowWidth(OString::number(double(aShadowItem.GetWidth()) / 20) + "pt");
    OString aOffset;
    switch (aShadowItem.GetLocation())
    {
        case SvxShadowLocation::TopLeft:
            aOffset = "-" + aShadowWidth + ",-" + aShadowWidth;
            break;
        case SvxShadowLocation::TopRight:
            aOffset = aShadowWidth + ",-" + aShadowWidth;
            break;
        case SvxShadowLocation::BottomLeft:
            aOffset = "-" + aShadowWidth + "," + aShadowWidth;
            break;
        case SvxShadowLocation::BottomRight:
            aOffset = aShadowWidth + "," + aShadowWidth;
            break;
        case SvxShadowLocation::NONE:
        case SvxShadowLocation::End:
            break;
    }
    if (aOffset.isEmpty())
        return;

    OString aShadowColor = msfilter::util::ConvertColor(aShadowItem.GetColor());
    m_pSerializer->singleElementNS(XML_v, XML_shadow, XML_on, "t", XML_color, "#" + aShadowColor,
                                   XML_offset, aOffset);
}

bool DocxSdrExport::Impl::isSupportedDMLShape(const uno::Reference<drawing::XShape>& xShape,
                                              const SdrObject* pSdrObject)
{
    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY_THROW);
    if (xServiceInfo->supportsService(u"com.sun.star.drawing.PolyPolygonShape"_ustr)
        || xServiceInfo->supportsService(u"com.sun.star.drawing.PolyLineShape"_ustr))
        return false;

    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    // For signature line shapes, we don't want DML, just the VML shape.
    if (xServiceInfo->supportsService(u"com.sun.star.drawing.GraphicObjectShape"_ustr))
    {
        bool bIsSignatureLineShape = false;
        xShapeProperties->getPropertyValue(u"IsSignatureLine"_ustr) >>= bIsSignatureLineShape;
        if (bIsSignatureLineShape)
            return false;
    }

    // A FontWork shape with bitmap fill cannot be expressed as a modern 'abc transform'
    // in Word. Only the legacy VML WordArt allows bitmap fill.
    if (pSdrObject->IsTextPath())
    {
        css::drawing::FillStyle eFillStyle = css::drawing::FillStyle_SOLID;
        xShapeProperties->getPropertyValue(u"FillStyle"_ustr) >>= eFillStyle;
        if (eFillStyle == css::drawing::FillStyle_BITMAP)
            return false;
    }
    return true;
}

void DocxSdrExport::writeDMLAndVMLDrawing(const SdrObject* sdrObj,
                                          const SwFrameFormat& rFrameFormat, int nAnchorId)
{
    bool bDMLAndVMLDrawingOpen = m_pImpl->getDMLAndVMLDrawingOpen();
    m_pImpl->setDMLAndVMLDrawingOpen(true);

    // Depending on the shape type, we actually don't write the shape as DML.
    OUString sShapeType;
    ShapeFlag nMirrorFlags = ShapeFlag::NONE;
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(sdrObj)->getUnoShape());

    MSO_SPT eShapeType
        = EscherPropertyContainer::GetCustomShapeType(xShape, nMirrorFlags, sShapeType);

    // In case we are already inside a DML block, then write the shape only as VML, turn out that's allowed to do.
    // A common service created in util to check for VML shapes which are allowed to have textbox in content
    if ((msfilter::util::HasTextBoxContent(eShapeType)) && Impl::isSupportedDMLShape(xShape, sdrObj)
        && (!bDMLAndVMLDrawingOpen || lcl_isLockedCanvas(xShape))) // Locked canvas is OK inside DML
    {
        m_pImpl->getSerializer()->startElementNS(XML_mc, XML_AlternateContent);

        auto pObjGroup = dynamic_cast<const SdrObjGroup*>(sdrObj);
        m_pImpl->getSerializer()->startElementNS(XML_mc, XML_Choice, XML_Requires,
                                                 (pObjGroup ? "wpg" : "wps"));
        writeDMLDrawing(sdrObj, &rFrameFormat, nAnchorId);
        m_pImpl->getSerializer()->endElementNS(XML_mc, XML_Choice);

        m_pImpl->getSerializer()->startElementNS(XML_mc, XML_Fallback);
        writeVMLDrawing(sdrObj, rFrameFormat);
        m_pImpl->getSerializer()->endElementNS(XML_mc, XML_Fallback);

        m_pImpl->getSerializer()->endElementNS(XML_mc, XML_AlternateContent);
    }
    else
        writeVMLDrawing(sdrObj, rFrameFormat);

    m_pImpl->setDMLAndVMLDrawingOpen(bDMLAndVMLDrawingOpen);
}

// Converts ARGB transparency (0..255) to drawingml alpha (opposite, and 0..100000)
static OString lcl_TransparencyToDrawingMlAlpha(const Color& rColor)
{
    if (rColor.IsTransparent())
    {
        sal_Int32 nAlphaPercent = float(rColor.GetAlpha()) / 2.55;
        return OString::number(nAlphaPercent * oox::drawingml::PER_PERCENT);
    }

    return OString();
}

void DocxSdrExport::writeDMLEffectLst(const SwFrameFormat& rFrameFormat)
{
    const SvxShadowItem& aShadowItem = rFrameFormat.GetShadow();

    // Output effects
    if (aShadowItem.GetLocation() == SvxShadowLocation::NONE)
        return;

    // Distance is measured diagonally from corner
    double nShadowDist
        = sqrt(static_cast<double>(aShadowItem.GetWidth()) * aShadowItem.GetWidth() * 2.0);
    OString aShadowDist(OString::number(TwipsToEMU(nShadowDist)));
    OString aShadowColor = msfilter::util::ConvertColor(aShadowItem.GetColor());
    OString aShadowAlpha = lcl_TransparencyToDrawingMlAlpha(aShadowItem.GetColor());
    sal_uInt32 nShadowDir = 0;
    switch (aShadowItem.GetLocation())
    {
        case SvxShadowLocation::TopLeft:
            nShadowDir = 13500000;
            break;
        case SvxShadowLocation::TopRight:
            nShadowDir = 18900000;
            break;
        case SvxShadowLocation::BottomLeft:
            nShadowDir = 8100000;
            break;
        case SvxShadowLocation::BottomRight:
            nShadowDir = 2700000;
            break;
        case SvxShadowLocation::NONE:
        case SvxShadowLocation::End:
            break;
    }
    OString aShadowDir(OString::number(nShadowDir));

    m_pImpl->getSerializer()->startElementNS(XML_a, XML_effectLst);
    m_pImpl->getSerializer()->startElementNS(XML_a, XML_outerShdw, XML_dist, aShadowDist, XML_dir,
                                             aShadowDir);
    if (aShadowAlpha.isEmpty())
        m_pImpl->getSerializer()->singleElementNS(XML_a, XML_srgbClr, XML_val, aShadowColor);
    else
    {
        m_pImpl->getSerializer()->startElementNS(XML_a, XML_srgbClr, XML_val, aShadowColor);
        m_pImpl->getSerializer()->singleElementNS(XML_a, XML_alpha, XML_val, aShadowAlpha);
        m_pImpl->getSerializer()->endElementNS(XML_a, XML_srgbClr);
    }
    m_pImpl->getSerializer()->endElementNS(XML_a, XML_outerShdw);
    m_pImpl->getSerializer()->endElementNS(XML_a, XML_effectLst);
}

void DocxSdrExport::writeDiagram(const SdrObject* sdrObject, const SwFrameFormat& rFrameFormat,
                                 int nDiagramId)
{
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(sdrObject)->getUnoShape(),
                                           uno::UNO_QUERY);

    // write necessary tags to document.xml
    Size aSize(sdrObject->GetSnapRect().getOpenWidth(), sdrObject->GetSnapRect().getOpenHeight());
    startDMLAnchorInline(&rFrameFormat, aSize);

    m_pImpl->getDrawingML()->SetFS(m_pImpl->getSerializer());
    m_pImpl->getDrawingML()->WriteDiagram(xShape, nDiagramId);

    endDMLAnchorInline(&rFrameFormat);
}

void DocxSdrExport::writeOnlyTextOfFrame(ww8::Frame const* pParentFrame)
{
    const SwFrameFormat& rFrameFormat = pParentFrame->GetFrameFormat();
    const SwNodeIndex* pNodeIndex = rFrameFormat.GetContent().GetContentIdx();

    SwNodeOffset nStt = pNodeIndex ? pNodeIndex->GetIndex() + 1 : SwNodeOffset(0);
    SwNodeOffset nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : SwNodeOffset(0);

    //Save data here and restore when out of scope
    ExportDataSaveRestore aDataGuard(m_pImpl->getExport(), nStt, nEnd, pParentFrame);

    m_pImpl->setBodyPrAttrList(sax_fastparser::FastSerializerHelper::createAttrList().get());
    ::comphelper::FlagRestorationGuard const g(m_pImpl->m_bFlyFrameGraphic, true);
    comphelper::ValueRestorationGuard vg(m_pImpl->getExport().m_nTextTyp, TXT_TXTBOX);
    m_pImpl->getExport().WriteText();
}

void DocxSdrExport::writeBoxItemLine(const SvxBoxItem& rBox)
{
    const editeng::SvxBorderLine* pBorderLine = nullptr;

    if (rBox.GetTop())
    {
        pBorderLine = rBox.GetTop();
    }
    else if (rBox.GetLeft())
    {
        pBorderLine = rBox.GetLeft();
    }
    else if (rBox.GetBottom())
    {
        pBorderLine = rBox.GetBottom();
    }
    else if (rBox.GetRight())
    {
        pBorderLine = rBox.GetRight();
    }

    if (!pBorderLine)
    {
        return;
    }

    sax_fastparser::FSHelperPtr pFS = m_pImpl->getSerializer();
    if (pBorderLine->GetWidth() == SvxBorderLineWidth::Hairline)
        pFS->startElementNS(XML_a, XML_ln);
    else
    {
        double fConverted(editeng::ConvertBorderWidthToWord(pBorderLine->GetBorderLineStyle(),
                                                            pBorderLine->GetWidth()));
        OString sWidth(OString::number(TwipsToEMU(fConverted)));
        pFS->startElementNS(XML_a, XML_ln, XML_w, sWidth);
    }

    pFS->startElementNS(XML_a, XML_solidFill);
    OString sColor(msfilter::util::ConvertColor(pBorderLine->GetColor()));
    pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, sColor);
    pFS->endElementNS(XML_a, XML_solidFill);

    if (SvxBorderLineStyle::DASHED == pBorderLine->GetBorderLineStyle()) // Line Style is Dash type
        pFS->singleElementNS(XML_a, XML_prstDash, XML_val, "dash");

    pFS->endElementNS(XML_a, XML_ln);
}

void DocxSdrExport::writeDMLTextFrame(ww8::Frame const* pParentFrame, int nAnchorId,
                                      bool bTextBoxOnly)
{
    bool bDMLAndVMLDrawingOpen = m_pImpl->getDMLAndVMLDrawingOpen();
    m_pImpl->setDMLAndVMLDrawingOpen(IsAnchorTypeInsideParagraph(pParentFrame));

    sax_fastparser::FSHelperPtr pFS = m_pImpl->getSerializer();
    const SwFrameFormat& rFrameFormat = pParentFrame->GetFrameFormat();
    const SwNodeIndex* pNodeIndex = rFrameFormat.GetContent().GetContentIdx();

    SwNodeOffset nStt = pNodeIndex ? pNodeIndex->GetIndex() + 1 : SwNodeOffset(0);
    SwNodeOffset nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : SwNodeOffset(0);

    //Save data here and restore when out of scope
    ExportDataSaveRestore aDataGuard(m_pImpl->getExport(), nStt, nEnd, pParentFrame);

    // When a frame has some low height, but automatically expanded due
    // to lots of contents, this size contains the real size.
    const Size aSize = pParentFrame->GetSize();

    uno::Reference<drawing::XShape> xShape;
    const SdrObject* pSdrObj = rFrameFormat.FindRealSdrObject();
    if (pSdrObj)
        xShape.set(const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo;
    if (xPropertySet.is())
        xPropSetInfo = xPropertySet->getPropertySetInfo();

    m_pImpl->setBodyPrAttrList(sax_fastparser::FastSerializerHelper::createAttrList().get());
    {
        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(u"TextVerticalAdjust"_ustr))
            xPropertySet->getPropertyValue(u"TextVerticalAdjust"_ustr) >>= eAdjust;
        m_pImpl->getBodyPrAttrList()->add(XML_anchor,
                                          oox::drawingml::GetTextVerticalAdjust(eAdjust));
    }

    if (!bTextBoxOnly)
    {
        startDMLAnchorInline(&rFrameFormat, aSize);

        rtl::Reference<sax_fastparser::FastAttributeList> pDocPrAttrList
            = sax_fastparser::FastSerializerHelper::createAttrList();
        pDocPrAttrList->add(XML_id, OString::number(nAnchorId));
        pDocPrAttrList->add(XML_name, rFrameFormat.GetName());

        pFS->startElementNS(XML_wp, XML_docPr, pDocPrAttrList);

        OUString sHyperlink;
        if (xPropertySet.is())
            xPropertySet->getPropertyValue(u"HyperLinkURL"_ustr) >>= sHyperlink;
        if (!sHyperlink.isEmpty())
        {
            OUString sRelId = m_pImpl->getExport().GetFilter().addRelation(
                pFS->getOutputStream(), oox::getRelationship(Relationship::HYPERLINK),
                oox::drawingml::URLTransformer().getTransformedString(sHyperlink),
                oox::drawingml::URLTransformer().isExternalURL(sHyperlink));
            pFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId,
                                 FSNS(XML_xmlns, XML_a),
                                 m_pImpl->getExport().GetFilter().getNamespaceURL(OOX_NS(dml)));
        }

        pFS->endElementNS(XML_wp, XML_docPr);

        pFS->startElementNS(XML_a, XML_graphic, FSNS(XML_xmlns, XML_a),
                            m_pImpl->getExport().GetFilter().getNamespaceURL(OOX_NS(dml)));
        pFS->startElementNS(XML_a, XML_graphicData, XML_uri,
                            "http://schemas.microsoft.com/office/word/2010/wordprocessingShape");
        pFS->startElementNS(XML_wps, XML_wsp);
        pFS->singleElementNS(XML_wps, XML_cNvSpPr, XML_txBox, "1");

        uno::Any aRotation;
        m_pImpl->setDMLandVMLTextFrameRotation(0_deg100);
        if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(u"FrameInteropGrabBag"_ustr))
        {
            uno::Sequence<beans::PropertyValue> propList;
            xPropertySet->getPropertyValue(u"FrameInteropGrabBag"_ustr) >>= propList;
            auto pProp = std::find_if(std::cbegin(propList), std::cend(propList),
                                      [](const beans::PropertyValue& rProp) {
                                          return rProp.Name == "mso-rotation-angle";
                                      });
            if (pProp != std::cend(propList))
                aRotation = pProp->Value;
        }
        sal_Int32 nTmp;
        if (aRotation >>= nTmp)
            m_pImpl->getDMLandVMLTextFrameRotation() = Degree100(nTmp);
        OString sRotation(OString::number(
            oox::drawingml::ExportRotateClockwisify(m_pImpl->getDMLandVMLTextFrameRotation())));
        // Shape properties
        pFS->startElementNS(XML_wps, XML_spPr);
        if (m_pImpl->getDMLandVMLTextFrameRotation())
        {
            pFS->startElementNS(XML_a, XML_xfrm, XML_rot, sRotation);
        }
        else
        {
            pFS->startElementNS(XML_a, XML_xfrm);
        }
        pFS->singleElementNS(XML_a, XML_off, XML_x, "0", XML_y, "0");
        OString aWidth(OString::number(TwipsToEMU(aSize.Width())));
        OString aHeight(OString::number(TwipsToEMU(aSize.Height())));
        pFS->singleElementNS(XML_a, XML_ext, XML_cx, aWidth, XML_cy, aHeight);
        pFS->endElementNS(XML_a, XML_xfrm);
        OUString shapeType = u"rect"_ustr;
        if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(u"FrameInteropGrabBag"_ustr))
        {
            uno::Sequence<beans::PropertyValue> propList;
            xPropertySet->getPropertyValue(u"FrameInteropGrabBag"_ustr) >>= propList;
            auto pProp = std::find_if(std::cbegin(propList), std::cend(propList),
                                      [](const beans::PropertyValue& rProp) {
                                          return rProp.Name == "mso-orig-shape-type";
                                      });
            if (pProp != std::cend(propList))
                pProp->Value >>= shapeType;
        }
        //Empty shapeType will lead to corruption so to avoid that shapeType is set to default i.e. "rect"
        if (shapeType.isEmpty())
            shapeType = "rect";

        pFS->singleElementNS(XML_a, XML_prstGeom, XML_prst, shapeType);
        m_pImpl->setDMLTextFrameSyntax(true);
        m_pImpl->getExport().OutputFormat(pParentFrame->GetFrameFormat(), false, false, true);
        m_pImpl->setDMLTextFrameSyntax(false);
        writeDMLEffectLst(rFrameFormat);
        pFS->endElementNS(XML_wps, XML_spPr);
    }

    //first, loop through ALL of the chained textboxes to identify a unique ID for each chain, and sequence number for each textbox in that chain.
    if (!m_pImpl->getExport().m_bLinkedTextboxesHelperInitialized)
    {
        sal_Int32 nSeq = 0;
        for (auto& rEntry : m_pImpl->getExport().m_aLinkedTextboxesHelper)
        {
            //find the start of a textbox chain: has no PREVIOUS link, but does have NEXT link
            if (rEntry.second.sPrevChain.isEmpty() && !rEntry.second.sNextChain.isEmpty())
            {
                //assign this chain a unique ID and start a new sequence
                nSeq = 0;
                rEntry.second.nId = ++m_pImpl->getExport().m_nLinkedTextboxesChainId;
                rEntry.second.nSeq = nSeq;

                OUString sCheckForBrokenChains = rEntry.first;

                //follow the chain and assign the same id, and incremental sequence numbers.
                auto followChainIter
                    = m_pImpl->getExport().m_aLinkedTextboxesHelper.find(rEntry.second.sNextChain);
                while (followChainIter != m_pImpl->getExport().m_aLinkedTextboxesHelper.end())
                {
                    //verify that the NEXT textbox also points to me as the PREVIOUS.
                    // A broken link indicates a leftover remnant that can be ignored.
                    if (followChainIter->second.sPrevChain != sCheckForBrokenChains)
                        break;

                    followChainIter->second.nId = m_pImpl->getExport().m_nLinkedTextboxesChainId;
                    followChainIter->second.nSeq = ++nSeq;

                    //empty next chain indicates the end of the linked chain.
                    if (followChainIter->second.sNextChain.isEmpty())
                        break;

                    sCheckForBrokenChains = followChainIter->first;
                    followChainIter = m_pImpl->getExport().m_aLinkedTextboxesHelper.find(
                        followChainIter->second.sNextChain);
                }
            }
        }
        m_pImpl->getExport().m_bLinkedTextboxesHelperInitialized = true;
    }

    m_pImpl->getExport().m_pParentFrame = nullptr;
    bool skipTxBxContent = false;
    bool isTxbxLinked = false;

    OUString sLinkChainName;
    if (xPropSetInfo.is())
    {
        if (xPropSetInfo->hasPropertyByName(u"LinkDisplayName"_ustr))
            xPropertySet->getPropertyValue(u"LinkDisplayName"_ustr) >>= sLinkChainName;
        else if (xPropSetInfo->hasPropertyByName(u"ChainName"_ustr))
            xPropertySet->getPropertyValue(u"ChainName"_ustr) >>= sLinkChainName;
    }

    // second, check if THIS textbox is linked and then decide whether to write the tag txbx or linkedTxbx
    auto linkedTextboxesIter = m_pImpl->getExport().m_aLinkedTextboxesHelper.find(sLinkChainName);
    if (linkedTextboxesIter != m_pImpl->getExport().m_aLinkedTextboxesHelper.end())
    {
        if ((linkedTextboxesIter->second.nId != 0) && (linkedTextboxesIter->second.nSeq != 0))
        {
            //not the first in the chain, so write the tag as linkedTxbx
            pFS->singleElementNS(XML_wps, XML_linkedTxbx, XML_id,
                                 OString::number(linkedTextboxesIter->second.nId), XML_seq,
                                 OString::number(linkedTextboxesIter->second.nSeq));
            /* no text content should be added to this tag,
               since the textbox is linked, the entire content
               is written in txbx block
            */
            skipTxBxContent = true;
        }
        else if ((linkedTextboxesIter->second.nId != 0) && (linkedTextboxesIter->second.nSeq == 0))
        {
            /* this is the first textbox in the chaining, we add the text content
               to this block*/
            //since the text box is linked, it needs an id.
            pFS->startElementNS(XML_wps, XML_txbx, XML_id,
                                OString::number(linkedTextboxesIter->second.nId));
            isTxbxLinked = true;
        }
    }

    if (!skipTxBxContent)
    {
        if (!isTxbxLinked)
            pFS->startElementNS(XML_wps, XML_txbx); //text box is not linked, therefore no id.

        pFS->startElementNS(XML_w, XML_txbxContent);

        const SvxFrameDirectionItem& rDirection = rFrameFormat.GetFrameDir();
        if (rDirection.GetValue() == SvxFrameDirection::Vertical_RL_TB)
            m_pImpl->getBodyPrAttrList()->add(XML_vert, "eaVert");
        else if (rDirection.GetValue() == SvxFrameDirection::Vertical_LR_BT)
            m_pImpl->getBodyPrAttrList()->add(XML_vert, "vert270");
        else if (rDirection.GetValue() == SvxFrameDirection::Vertical_LR_TB)
            m_pImpl->getBodyPrAttrList()->add(XML_vert, "mongolianVert");
        else if (rDirection.GetValue() == SvxFrameDirection::Vertical_RL_TB90)
            m_pImpl->getBodyPrAttrList()->add(XML_vert, "vert");
        {
            ::comphelper::FlagRestorationGuard const g(m_pImpl->m_bFlyFrameGraphic, true);
            comphelper::ValueRestorationGuard vg(m_pImpl->getExport().m_nTextTyp, TXT_TXTBOX);
            m_pImpl->getExport().WriteText();
            if (m_pImpl->getParagraphSdtOpen())
            {
                m_pImpl->getExport().DocxAttrOutput().EndParaSdtBlock();
                m_pImpl->setParagraphSdtOpen(false);
            }
        }

        pFS->endElementNS(XML_w, XML_txbxContent);
        pFS->endElementNS(XML_wps, XML_txbx);
    }

    // We need to init padding to 0, if it's not set.
    // In LO the default is 0 and so ins attributes are not set when padding is 0
    // but in MSO the default is 254 / 127, so we need to set 0 padding explicitly
    if (m_pImpl->getBodyPrAttrList())
    {
        if (!m_pImpl->getBodyPrAttrList()->hasAttribute(XML_lIns))
            m_pImpl->getBodyPrAttrList()->add(XML_lIns, OString::number(0));
        if (!m_pImpl->getBodyPrAttrList()->hasAttribute(XML_tIns))
            m_pImpl->getBodyPrAttrList()->add(XML_tIns, OString::number(0));
        if (!m_pImpl->getBodyPrAttrList()->hasAttribute(XML_rIns))
            m_pImpl->getBodyPrAttrList()->add(XML_rIns, OString::number(0));
        if (!m_pImpl->getBodyPrAttrList()->hasAttribute(XML_bIns))
            m_pImpl->getBodyPrAttrList()->add(XML_bIns, OString::number(0));
    }

    rtl::Reference<FastAttributeList> xBodyPrAttrList(m_pImpl->getBodyPrAttrList());
    m_pImpl->setBodyPrAttrList(nullptr);
    if (!bTextBoxOnly)
    {
        pFS->startElementNS(XML_wps, XML_bodyPr, xBodyPrAttrList);
        // AutoSize of the Text Frame.
        const SwFormatFrameSize& rSize = rFrameFormat.GetFrameSize();
        pFS->singleElementNS(
            XML_a,
            (rSize.GetHeightSizeType() == SwFrameSize::Variable ? XML_spAutoFit : XML_noAutofit));
        pFS->endElementNS(XML_wps, XML_bodyPr);

        pFS->endElementNS(XML_wps, XML_wsp);
        pFS->endElementNS(XML_a, XML_graphicData);
        pFS->endElementNS(XML_a, XML_graphic);

        // Relative size of the Text Frame.
        const sal_uInt8 nWidthPercent = rSize.GetWidthPercent();
        if (nWidthPercent && nWidthPercent != SwFormatFrameSize::SYNCED)
        {
            pFS->startElementNS(XML_wp14, XML_sizeRelH, XML_relativeFrom,
                                (rSize.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME
                                     ? "page"
                                     : "margin"));
            pFS->startElementNS(XML_wp14, XML_pctWidth);
            pFS->writeEscaped(OUString::number(nWidthPercent * oox::drawingml::PER_PERCENT));
            pFS->endElementNS(XML_wp14, XML_pctWidth);
            pFS->endElementNS(XML_wp14, XML_sizeRelH);
        }
        const sal_uInt8 nHeightPercent = rSize.GetHeightPercent();
        if (nHeightPercent && nHeightPercent != SwFormatFrameSize::SYNCED)
        {
            pFS->startElementNS(
                XML_wp14, XML_sizeRelV, XML_relativeFrom,
                (rSize.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME ? "page"
                                                                                      : "margin"));
            pFS->startElementNS(XML_wp14, XML_pctHeight);
            pFS->writeEscaped(OUString::number(nHeightPercent * oox::drawingml::PER_PERCENT));
            pFS->endElementNS(XML_wp14, XML_pctHeight);
            pFS->endElementNS(XML_wp14, XML_sizeRelV);
        }

        endDMLAnchorInline(&rFrameFormat);
    }
    m_pImpl->setDMLAndVMLDrawingOpen(bDMLAndVMLDrawingOpen);
}

void DocxSdrExport::writeVMLTextFrame(ww8::Frame const* pParentFrame, bool bTextBoxOnly)
{
    bool bDMLAndVMLDrawingOpen = m_pImpl->getDMLAndVMLDrawingOpen();
    m_pImpl->setDMLAndVMLDrawingOpen(IsAnchorTypeInsideParagraph(pParentFrame));

    sax_fastparser::FSHelperPtr pFS = m_pImpl->getSerializer();
    const SwFrameFormat& rFrameFormat = pParentFrame->GetFrameFormat();
    const SwNodeIndex* pNodeIndex = rFrameFormat.GetContent().GetContentIdx();

    SwNodeOffset nStt = pNodeIndex ? pNodeIndex->GetIndex() + 1 : SwNodeOffset(0);
    SwNodeOffset nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : SwNodeOffset(0);

    //Save data here and restore when out of scope
    ExportDataSaveRestore aDataGuard(m_pImpl->getExport(), nStt, nEnd, pParentFrame);

    // When a frame has some low height, but automatically expanded due
    // to lots of contents, this size contains the real size.
    const Size aSize = pParentFrame->GetSize();
    m_pImpl->setFlyFrameSize(&aSize);

    m_pImpl->setTextFrameSyntax(true);
    m_pImpl->setFlyAttrList(sax_fastparser::FastSerializerHelper::createAttrList());
    m_pImpl->setTextboxAttrList(sax_fastparser::FastSerializerHelper::createAttrList());
    m_pImpl->getTextFrameStyle() = "position:absolute";
    if (!bTextBoxOnly)
    {
        OString sRotation(OString::number(-toDegrees(m_pImpl->getDMLandVMLTextFrameRotation())));
        m_pImpl->getExport().SdrExporter().getTextFrameStyle().append(";rotation:" + sRotation);
    }
    m_pImpl->getExport().OutputFormat(pParentFrame->GetFrameFormat(), false, false, true);
    m_pImpl->getFlyAttrList()->add(XML_style, m_pImpl->getTextFrameStyle().makeStringAndClear());

    const SdrObject* pObject = pParentFrame->GetFrameFormat().FindRealSdrObject();
    if (pObject != nullptr)
    {
        OUString sAnchorId = lclGetAnchorIdFromGrabBag(pObject);
        if (!sAnchorId.isEmpty())
            m_pImpl->getFlyAttrList()->addNS(XML_w14, XML_anchorId, sAnchorId);

        uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pObject)->getUnoShape(),
                                               uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        OUString sHyperlink;
        if (xShapeProps.is())
            xShapeProps->getPropertyValue(u"HyperLinkURL"_ustr) >>= sHyperlink;
        if (!sHyperlink.isEmpty())
            m_pImpl->getFlyAttrList()->add(XML_href, sHyperlink);
    }
    rtl::Reference<FastAttributeList> xFlyAttrList(m_pImpl->getFlyAttrList());
    m_pImpl->getFlyAttrList().clear();
    rtl::Reference<FastAttributeList> xTextboxAttrList(m_pImpl->getTextboxAttrList());
    m_pImpl->getTextboxAttrList().clear();
    m_pImpl->setTextFrameSyntax(false);
    m_pImpl->setFlyFrameSize(nullptr);
    m_pImpl->getExport().m_pParentFrame = nullptr;

    if (!bTextBoxOnly)
    {
        pFS->startElementNS(XML_w, XML_pict);
        pFS->startElementNS(XML_v, XML_rect, xFlyAttrList);
        m_pImpl->textFrameShadow(rFrameFormat);
        if (m_pImpl->getFlyFillAttrList().is())
        {
            rtl::Reference<FastAttributeList> xFlyFillAttrList(m_pImpl->getFlyFillAttrList());
            pFS->singleElementNS(XML_v, XML_fill, xFlyFillAttrList);
        }
        if (m_pImpl->getDashLineStyleAttr().is())
        {
            rtl::Reference<FastAttributeList> xDashLineStyleAttr(m_pImpl->getDashLineStyleAttr());
            pFS->singleElementNS(XML_v, XML_stroke, xDashLineStyleAttr);
        }
        pFS->startElementNS(XML_v, XML_textbox, xTextboxAttrList);
    }
    m_pImpl->getFlyFillAttrList().clear();
    m_pImpl->getDashLineStyleAttr().clear();

    pFS->startElementNS(XML_w, XML_txbxContent);
    {
        ::comphelper::FlagRestorationGuard const g(m_pImpl->m_bFlyFrameGraphic, true);
        comphelper::ValueRestorationGuard vg(m_pImpl->getExport().m_nTextTyp, TXT_TXTBOX);
        m_pImpl->getExport().WriteText();
        if (m_pImpl->getParagraphSdtOpen())
        {
            m_pImpl->getExport().DocxAttrOutput().EndParaSdtBlock();
            m_pImpl->setParagraphSdtOpen(false);
        }
    }
    pFS->endElementNS(XML_w, XML_txbxContent);
    if (!bTextBoxOnly)
    {
        pFS->endElementNS(XML_v, XML_textbox);

        if (m_pImpl->getFlyWrapAttrList())
        {
            rtl::Reference<FastAttributeList> xFlyWrapAttrList(m_pImpl->getFlyWrapAttrList());
            m_pImpl->setFlyWrapAttrList(nullptr);
            pFS->singleElementNS(XML_w10, XML_wrap, xFlyWrapAttrList);
        }

        pFS->endElementNS(XML_v, XML_rect);
        pFS->endElementNS(XML_w, XML_pict);
    }

    m_pImpl->setDMLAndVMLDrawingOpen(bDMLAndVMLDrawingOpen);
}

bool DocxSdrExport::isTextBox(const SwFrameFormat& rFrameFormat)
{
    return SwTextBoxHelper::isTextBox(&rFrameFormat, RES_FLYFRMFMT);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
