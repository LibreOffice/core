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
#include <basegfx/tuple/b2dtuple.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <svx/svdtrans.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/shape.hxx>

#include <optional>

using namespace com::sun::star;

namespace oox::shape
{
WpsContext::WpsContext(ContextHandler2Helper const& rParent, uno::Reference<drawing::XShape> xShape,
                       const drawingml::ShapePtr& pMasterShapePtr,
                       const drawingml::ShapePtr& pShapePtr)
    : ShapeContext(rParent, pMasterShapePtr, pShapePtr)
    , mxShape(std::move(xShape))
{
    mpShapePtr->setWps(true);
}

WpsContext::~WpsContext() = default;

oox::core::ContextHandlerRef WpsContext::onCreateContext(sal_Int32 nElementToken,
                                                         const oox::AttributeList& rAttribs)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_wsp:
        case XML_cNvCnPr:
            break;
        case XML_bodyPr:
            if (mxShape.is())
            {
                uno::Reference<lang::XServiceInfo> xServiceInfo(mxShape, uno::UNO_QUERY);
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                sal_Int32 nVert = rAttribs.getToken(XML_vert, XML_horz);
                if (nVert == XML_eaVert)
                {
                    xPropertySet->setPropertyValue("TextWritingMode",
                                                   uno::makeAny(text::WritingMode_TB_RL));
                }
                else if (nVert != XML_horz)
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
                    const sal_Int32 nRotation = nVert == XML_vert270 ? -270 : -90;
                    if (static_cast<long>(basegfx::rad2deg(fRotate))
                        != NormAngle36000(static_cast<long>(nRotation) * 100) / 100)
                    {
                        comphelper::SequenceAsHashMap aCustomShapeGeometry(
                            xPropertySet->getPropertyValue("CustomShapeGeometry"));
                        aCustomShapeGeometry["TextPreRotateAngle"] <<= nRotation;
                        xPropertySet->setPropertyValue(
                            "CustomShapeGeometry",
                            uno::makeAny(aCustomShapeGeometry.getAsConstPropertyValueList()));
                    }
                }

                if (bool bUpright = rAttribs.getBool(XML_upright, false))
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    xPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc(length + 1);
                    aGrabBag[length].Name = "Upright";
                    aGrabBag[length].Value <<= bUpright;
                    xPropertySet->setPropertyValue("InteropGrabBag", uno::makeAny(aGrabBag));
                }

                if (xServiceInfo.is())
                {
                    // Handle inset attributes for Writer textframes.
                    sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                    std::optional<sal_Int32> oInsets[4];
                    for (std::size_t i = 0; i < SAL_N_ELEMENTS(aInsets); ++i)
                    {
                        OptValue<OUString> oValue = rAttribs.getString(aInsets[i]);
                        if (oValue.has())
                            oInsets[i] = oox::drawingml::GetCoordinate(oValue.get());
                        else
                            // Defaults from the spec: left/right: 91440 EMU, top/bottom: 45720 EMU
                            oInsets[i]
                                = (aInsets[i] == XML_lIns || aInsets[i] == XML_rIns) ? 254 : 127;
                    }
                    const OUString aShapeProps[]
                        = { OUString("TextLeftDistance"), OUString("TextUpperDistance"),
                            OUString("TextRightDistance"), OUString("TextLowerDistance") };
                    for (std::size_t i = 0; i < SAL_N_ELEMENTS(aShapeProps); ++i)
                        if (oInsets[i])
                            xPropertySet->setPropertyValue(aShapeProps[i],
                                                           uno::makeAny(*oInsets[i]));
                }

                // Handle text vertical adjustment inside a text frame
                if (rAttribs.hasAttribute(XML_anchor))
                {
                    drawing::TextVerticalAdjust eAdjust
                        = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                    xPropertySet->setPropertyValue("TextVerticalAdjust", uno::makeAny(eAdjust));
                }

                // Apply character color of the shape to the shape's textbox.
                uno::Reference<text::XText> xText(mxShape, uno::UNO_QUERY);
                uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
                xTextCursor->gotoStart(false);
                xTextCursor->gotoEnd(true);
                const uno::Reference<beans::XPropertyState> xPropertyState(xTextCursor,
                                                                           uno::UNO_QUERY);
                const beans::PropertyState ePropertyState
                    = xPropertyState->getPropertyState("CharColor");
                if (ePropertyState == beans::PropertyState_DEFAULT_VALUE)
                {
                    uno::Reference<beans::XPropertySet> xTextBoxPropertySet(xTextCursor,
                                                                            uno::UNO_QUERY);
                    uno::Any xCharColor = xPropertySet->getPropertyValue("CharColor");
                    Color aColor = COL_AUTO;
                    if (xCharColor >>= aColor)
                    {
                        if (aColor != COL_AUTO)
                            xTextBoxPropertySet->setPropertyValue("CharColor", xCharColor);
                    }
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
                    xPropertySet->setPropertyValue(
                        "FrameIsAutomaticHeight",
                        uno::makeAny(getBaseToken(nElementToken) == XML_spAutoFit));
                else
                    xPropertySet->setPropertyValue(
                        "TextAutoGrowHeight",
                        uno::makeAny(getBaseToken(nElementToken) == XML_spAutoFit));
            }
        }
        break;
        case XML_prstTxWarp:
            if (rAttribs.hasAttribute(XML_prst))
            {
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                if (xPropertySet.is())
                {
                    oox::OptValue<OUString> presetShapeName = rAttribs.getString(XML_prst);
                    const OUString& preset = presetShapeName.get();
                    comphelper::SequenceAsHashMap aCustomShapeGeometry(
                        xPropertySet->getPropertyValue("CustomShapeGeometry"));
                    aCustomShapeGeometry["PresetTextWarp"] <<= preset;
                    xPropertySet->setPropertyValue(
                        "CustomShapeGeometry",
                        uno::makeAny(aCustomShapeGeometry.getAsConstPropertyValueList()));
                }
            }
            break;
        case XML_txbx:
        {
            mpShapePtr->getCustomShapeProperties()->setShapeTypeOverride(true);
            mpShapePtr->setTextBox(true);
            //in case if the textbox is linked, save the attributes
            //for further processing.
            if (rAttribs.hasAttribute(XML_id))
            {
                OptValue<OUString> id = rAttribs.getString(XML_id);
                if (id.has())
                {
                    oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr;
                    linkedTxtBoxAttr.id = id.get().toInt32();
                    mpShapePtr->setTxbxHasLinkedTxtBox(true);
                    mpShapePtr->setLinkedTxbxAttributes(linkedTxtBoxAttr);
                }
            }
            return this;
        }
        break;
        case XML_linkedTxbx:
        {
            //in case if the textbox is linked, save the attributes
            //for further processing.
            mpShapePtr->getCustomShapeProperties()->setShapeTypeOverride(true);
            mpShapePtr->setTextBox(true);
            OptValue<OUString> id = rAttribs.getString(XML_id);
            OptValue<OUString> seq = rAttribs.getString(XML_seq);
            if (id.has() && seq.has())
            {
                oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr;
                linkedTxtBoxAttr.id = id.get().toInt32();
                linkedTxtBoxAttr.seq = seq.get().toInt32();
                mpShapePtr->setTxbxHasLinkedTxtBox(true);
                mpShapePtr->setLinkedTxbxAttributes(linkedTxtBoxAttr);
            }
        }
        break;
        default:
            return ShapeContext::onCreateContext(nElementToken, rAttribs);
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
