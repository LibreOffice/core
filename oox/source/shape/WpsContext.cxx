/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpsContext.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/shapestylecontext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <svx/svdtrans.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include <boost/optional.hpp>

using namespace com::sun::star;

namespace oox
{
namespace shape
{

WpsContext::WpsContext(ContextHandler2Helper& rParent, uno::Reference<drawing::XShape> xShape)
    : ContextHandler2(rParent),
      mxShape(std::move(xShape))
{
    mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.CustomShape"));
    mpShape->setWps(true);
}

WpsContext::~WpsContext() = default;

oox::core::ContextHandlerRef WpsContext::onCreateContext(sal_Int32 nElementToken, const oox::AttributeList& rAttribs)
{
    switch (getBaseToken(nElementToken))
    {
    case XML_wsp:
        break;
    case XML_cNvCnPr:
        break;
    case XML_cNvSpPr:
        break;
    case XML_spPr:
        return new oox::drawingml::ShapePropertiesContext(*this, *mpShape);
        break;
    case XML_style:
        return new oox::drawingml::ShapeStyleContext(*this, *mpShape);
        break;
    case XML_bodyPr:
        if (mxShape.is())
        {
            uno::Reference<lang::XServiceInfo> xServiceInfo(mxShape, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
            OptValue<OUString> oVert = rAttribs.getString(XML_vert);
            if (oVert.has() && oVert.get() == "vert270")
            {
                if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
                {
                    // No support for this in core, work around by char rotation, as we do so for table cells already.
                    uno::Reference<text::XText> xText(mxShape, uno::UNO_QUERY);
                    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
                    xTextCursor->gotoStart(false);
                    xTextCursor->gotoEnd(true);
                    uno::Reference<beans::XPropertyState> xPropertyState(xTextCursor, uno::UNO_QUERY);
                    beans::PropertyState aState = xPropertyState->getPropertyState("CharRotation");
                    if (aState == beans::PropertyState_DEFAULT_VALUE)
                    {
                        uno::Reference<beans::XPropertySet> xTextCursorPropertySet(xTextCursor, uno::UNO_QUERY);
                        xTextCursorPropertySet->setPropertyValue("CharRotation", uno::makeAny(sal_Int16(900)));
                    }
                }
                else
                {
                    // Get the existing rotation of the shape.
                    drawing::HomogenMatrix3 aMatrix;
                    xPropertySet->getPropertyValue("Transformation") >>= aMatrix;
                    basegfx::B2DHomMatrix aTransformation;
                    aTransformation.set(0, 0, aMatrix.Line1.Column1);
                    aTransformation.set(0, 1, aMatrix.Line1.Column2);
                    aTransformation.set(0, 2, aMatrix.Line1.Column3);
                    aTransformation.set(1, 0, aMatrix.Line1.Column1);
                    aTransformation.set(1, 1, aMatrix.Line2.Column2);
                    aTransformation.set(1, 2, aMatrix.Line3.Column3);
                    aTransformation.set(2, 0, aMatrix.Line1.Column1);
                    aTransformation.set(2, 1, aMatrix.Line2.Column2);
                    aTransformation.set(2, 2, aMatrix.Line3.Column3);
                    basegfx::B2DTuple aScale;
                    basegfx::B2DTuple aTranslate;
                    double fRotate = 0;
                    double fShearX = 0;
                    aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

                    // If the text is not rotated the way the shape wants it already, set the angle.
                    const sal_Int32 nRotation = -270;
                    if (static_cast<long>(basegfx::rad2deg(fRotate)) != NormAngle360(static_cast<long>(nRotation) * 100) / 100)
                    {
                        comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
                        aCustomShapeGeometry["TextPreRotateAngle"] = uno::makeAny(nRotation);
                        xPropertySet->setPropertyValue("CustomShapeGeometry", uno::makeAny(aCustomShapeGeometry.getAsConstPropertyValueList()));
                    }
                }
            }

            if (xServiceInfo.is())
            {
                bool bTextFrame = xServiceInfo->supportsService("com.sun.star.text.TextFrame");
                // Handle inset attributes for Writer textframes.
                sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                boost::optional<sal_Int32> oInsets[4];
                for (std::size_t i = 0; i < SAL_N_ELEMENTS(aInsets); ++i)
                {
                    OptValue<OUString> oValue = rAttribs.getString(aInsets[i]);
                    if (oValue.has())
                        oInsets[i] = oox::drawingml::GetCoordinate(oValue.get());
                    else
                        // Defaults from the spec: left/right: 91440 EMU, top/bottom: 45720 EMU
                        oInsets[i] = (aInsets[i] == XML_lIns || aInsets[i] == XML_rIns) ? 254 : 127;
                }
                OUString aProps[] = { OUString("LeftBorderDistance"), OUString("TopBorderDistance"), OUString("RightBorderDistance"), OUString("BottomBorderDistance") };
                OUString aShapeProps[] = { OUString("TextLeftDistance"), OUString("TextUpperDistance"), OUString("TextRightDistance"), OUString("TextLowerDistance") };
                for (std::size_t i = 0; i < SAL_N_ELEMENTS(bTextFrame ? aProps : aShapeProps); ++i)
                    if (oInsets[i])
                        xPropertySet->setPropertyValue((bTextFrame ? aProps : aShapeProps)[i], uno::makeAny(*oInsets[i]));
            }

            // Handle text vertical adjustment inside a text frame
            if (rAttribs.hasAttribute(XML_anchor))
            {
                drawing::TextVerticalAdjust eAdjust = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                xPropertySet->setPropertyValue("TextVerticalAdjust", uno::makeAny(eAdjust));
            }

            // Apply character color of the shape to the shape's textbox.
            uno::Reference<text::XText> xText(mxShape, uno::UNO_QUERY);
            uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
            xTextCursor->gotoStart(false);
            xTextCursor->gotoEnd(true);
            const uno::Reference<beans::XPropertyState> xPropertyState(xTextCursor, uno::UNO_QUERY);
            const beans::PropertyState ePropertyState = xPropertyState->getPropertyState("CharColor");
            if (ePropertyState == beans::PropertyState_DEFAULT_VALUE)
            {
                uno::Reference<beans::XPropertySet> xTextBoxPropertySet(xTextCursor, uno::UNO_QUERY);
                xTextBoxPropertySet->setPropertyValue("CharColor", xPropertySet->getPropertyValue("CharColor"));
            }
            return this;
        }
        break;
    case XML_noAutofit:
    case XML_spAutoFit:
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(mxShape, uno::UNO_QUERY);
        // We can't use oox::drawingml::TextBodyPropertiesContext here, as this
        // is a child context of bodyPr, so the shape is already sent: we need
        // to alter the XShape directly.
        uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
                xPropertySet->setPropertyValue("FrameIsAutomaticHeight", uno::makeAny(getBaseToken(nElementToken) == XML_spAutoFit));
            else
                xPropertySet->setPropertyValue("TextAutoGrowHeight", uno::makeAny(getBaseToken(nElementToken) == XML_spAutoFit));
        }
    }
    break;
    case XML_prstTxWarp:
        if (rAttribs.hasAttribute(XML_prst))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
            oox::OptValue<OUString> presetShapeName = rAttribs.getString(XML_prst);
            const OUString& preset = presetShapeName.get();
            comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
            aCustomShapeGeometry["PresetTextWarp"] = uno::makeAny(preset);
            xPropertySet->setPropertyValue("CustomShapeGeometry", uno::makeAny(aCustomShapeGeometry.getAsConstPropertyValueList()));
        }
        break;
    case XML_txbx:
    {
        mpShape->getCustomShapeProperties()->setShapeTypeOverride(true);
        mpShape->setTextBox(true);
        //in case if the textbox is linked, save the attributes
        //for further processing.
        if (rAttribs.hasAttribute(XML_id))
        {
            OptValue<OUString> id = rAttribs.getString(XML_id);
            if (id.has())
            {
                oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr ;
                linkedTxtBoxAttr.id = id.get().toInt32();
                mpShape->setTxbxHasLinkedTxtBox(true);
                mpShape->setLinkedTxbxAttributes(linkedTxtBoxAttr);
            }
        }
    }
    break;
    case XML_linkedTxbx:
    {
        //in case if the textbox is linked, save the attributes
        //for further processing.
        mpShape->getCustomShapeProperties()->setShapeTypeOverride(true);
        mpShape->setTextBox(true);
        OptValue<OUString> id  = rAttribs.getString(XML_id);
        OptValue<OUString> seq = rAttribs.getString(XML_seq);
        if (id.has() && seq.has())
        {
            oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr ;
            linkedTxtBoxAttr.id  = id.get().toInt32();
            linkedTxtBoxAttr.seq = seq.get().toInt32();
            mpShape->setTxbxHasLinkedTxtBox(true);
            mpShape->setLinkedTxbxAttributes(linkedTxtBoxAttr);
        }
    }
    break;
    default:
        SAL_WARN("oox", "WpsContext::createFastChildContext: unhandled element: " << getBaseToken(nElementToken));
        break;
    }
    return nullptr;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
