/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpsContext.hxx"
#include "WpgContext.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textbodyproperties.hxx>
#include <tools/helpers.hxx>

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
    if (mpShapePtr)
        mpShapePtr->setWps(true);

    if (const auto pParent = dynamic_cast<const WpgContext*>(&rParent))
        m_bHasWPGParent = pParent->isFullWPGSupport();
    else
        m_bHasWPGParent = false;
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
                // no evaluation of attribute XML_rot, because Word ignores it, as of 2022-07.

                uno::Reference<lang::XServiceInfo> xServiceInfo(mxShape, uno::UNO_QUERY);
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                sal_Int32 nVert = rAttribs.getToken(XML_vert, XML_horz);
                // Values 'wordArtVert' and 'wordArtVertRtl' are not implemented.
                // Map them to other vert values.
                if (nVert == XML_eaVert || nVert == XML_wordArtVertRtl)
                {
                    xPropertySet->setPropertyValue("TextWritingMode",
                                                   uno::Any(text::WritingMode_TB_RL));
                    xPropertySet->setPropertyValue("WritingMode",
                                                   uno::Any(text::WritingMode2::TB_RL));
                }
                else if (nVert == XML_mongolianVert || nVert == XML_wordArtVert)
                {
                    xPropertySet->setPropertyValue("WritingMode",
                                                   uno::Any(text::WritingMode2::TB_LR));
                }
                else if (nVert != XML_horz) // cases XML_vert and XML_vert270
                {
                    // Hack to get same rendering as after the fix for tdf#87924. If shape rotation
                    // plus text direction results in upright text, use horizontal text direction.
                    // Remove hack when frame is able to rotate.

                    // Need transformation matrix since RotateAngle does not contain flip.
                    drawing::HomogenMatrix3 aMatrix;
                    xPropertySet->getPropertyValue("Transformation") >>= aMatrix;
                    basegfx::B2DHomMatrix aTransformation;
                    aTransformation.set(0, 0, aMatrix.Line1.Column1);
                    aTransformation.set(0, 1, aMatrix.Line1.Column2);
                    aTransformation.set(0, 2, aMatrix.Line1.Column3);
                    aTransformation.set(1, 0, aMatrix.Line2.Column1);
                    aTransformation.set(1, 1, aMatrix.Line2.Column2);
                    aTransformation.set(1, 2, aMatrix.Line2.Column3);
                    aTransformation.set(2, 0, aMatrix.Line3.Column1);
                    aTransformation.set(2, 1, aMatrix.Line3.Column2);
                    aTransformation.set(2, 2, aMatrix.Line3.Column3);
                    basegfx::B2DTuple aScale;
                    basegfx::B2DTuple aTranslate;
                    double fRotate = 0;
                    double fShearX = 0;
                    aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
                    auto nRotate(static_cast<sal_uInt16>(NormAngle360(basegfx::rad2deg(fRotate))));
                    if ((nVert == XML_vert && nRotate == 270)
                        || (nVert == XML_vert270 && nRotate == 90))
                    {
                        xPropertySet->setPropertyValue("WritingMode",
                                                       uno::Any(text::WritingMode2::LR_TB));
                        // ToDo: Remember original vert value and remove hack on export.
                    }
                    else if (nVert == XML_vert)
                        xPropertySet->setPropertyValue("WritingMode",
                                                       uno::Any(text::WritingMode2::TB_RL90));
                    else // nVert == XML_vert270
                        xPropertySet->setPropertyValue("WritingMode",
                                                       uno::Any(text::WritingMode2::BT_LR));
                }

                if (bool bUpright = rAttribs.getBool(XML_upright, false))
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    xPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc(length + 1);
                    auto pGrabBag = aGrabBag.getArray();
                    pGrabBag[length].Name = "Upright";
                    pGrabBag[length].Value <<= bUpright;
                    xPropertySet->setPropertyValue("InteropGrabBag", uno::Any(aGrabBag));
                }

                if (xServiceInfo.is())
                {
                    // Handle inset attributes for Writer textframes.
                    sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                    std::optional<sal_Int32> oInsets[4];
                    for (std::size_t i = 0; i < SAL_N_ELEMENTS(aInsets); ++i)
                    {
                        std::optional<OUString> oValue = rAttribs.getString(aInsets[i]);
                        if (oValue.has_value())
                            oInsets[i] = oox::drawingml::GetCoordinate(oValue.value());
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
                            xPropertySet->setPropertyValue(aShapeProps[i], uno::Any(*oInsets[i]));
                }

                // Handle text vertical adjustment inside a text frame
                if (rAttribs.hasAttribute(XML_anchor))
                {
                    drawing::TextVerticalAdjust eAdjust
                        = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                    xPropertySet->setPropertyValue("TextVerticalAdjust", uno::Any(eAdjust));
                }

                // Apply character color of the shape to the shape's textbox.
                uno::Reference<text::XText> xText(mxShape, uno::UNO_QUERY);
                uno::Any xCharColor = xPropertySet->getPropertyValue("CharColor");
                Color aColor = COL_AUTO;
                if ((xCharColor >>= aColor) && aColor != COL_AUTO)
                {
                    // tdf#135923 Apply character color of the shape to the textrun
                    //            when the character color of the textrun is default.
                    // tdf#153791 But only if the run has no background color (shd element in OOXML)
                    if (uno::Reference<container::XEnumerationAccess> paraEnumAccess{
                            xText, uno::UNO_QUERY })
                    {
                        uno::Reference<container::XEnumeration> paraEnum(
                            paraEnumAccess->createEnumeration());

                        while (paraEnum->hasMoreElements())
                        {
                            uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(),
                                                                        uno::UNO_QUERY);
                            uno::Reference<container::XEnumerationAccess> runEnumAccess(
                                xParagraph, uno::UNO_QUERY);
                            if (!runEnumAccess.is())
                                continue;
                            if (uno::Reference<beans::XPropertySet> xParaPropSet{ xParagraph,
                                                                                  uno::UNO_QUERY })
                                if ((xParaPropSet->getPropertyValue("ParaBackColor") >>= aColor)
                                    && aColor != COL_AUTO)
                                    continue;

                            uno::Reference<container::XEnumeration> runEnum
                                = runEnumAccess->createEnumeration();

                            while (runEnum->hasMoreElements())
                            {
                                uno::Reference<text::XTextRange> xRun(runEnum->nextElement(),
                                                                      uno::UNO_QUERY);
                                const uno::Reference<beans::XPropertyState> xRunState(
                                    xRun, uno::UNO_QUERY);
                                if (!xRunState
                                    || xRunState->getPropertyState("CharColor")
                                           == beans::PropertyState_DEFAULT_VALUE)
                                {
                                    uno::Reference<beans::XPropertySet> xRunPropSet(xRun,
                                                                                    uno::UNO_QUERY);
                                    if (!xRunPropSet)
                                        continue;
                                    if ((xRunPropSet->getPropertyValue("CharBackColor") >>= aColor)
                                        && aColor != COL_AUTO)
                                        continue;
                                    if (!(xRunPropSet->getPropertyValue("CharColor") >>= aColor)
                                        || aColor == COL_AUTO)
                                        xRunPropSet->setPropertyValue("CharColor", xCharColor);
                                }
                            }
                        }
                    }
                }

                auto nWrappingType = rAttribs.getToken(XML_wrap, XML_square);
                xPropertySet->setPropertyValue("TextWordWrap",
                                               uno::Any(nWrappingType == XML_square));

                return this;
            }
            else if (m_bHasWPGParent && mpShapePtr)
            {
                // this WPS context has to be inside a WPG shape, so the <BodyPr> element
                // cannot be applied to mxShape member, use mpShape instead, and after the
                // the parent shape finished, apply it for its children.
                mpShapePtr->setWPGChild(true);
                oox::drawingml::TextBodyPtr pTextBody;
                pTextBody.reset(new oox::drawingml::TextBody());

                if (rAttribs.hasAttribute(XML_anchor))
                {
                    drawing::TextVerticalAdjust eAdjust
                        = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                    pTextBody->getTextProperties().meVA = eAdjust;
                }

                sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                for (int i = 0; i < 4; ++i)
                {
                    if (rAttribs.hasAttribute(XML_lIns))
                    {
                        std::optional<OUString> oValue = rAttribs.getString(aInsets[i]);
                        if (oValue.has_value())
                            pTextBody->getTextProperties().moInsets[i]
                                = oox::drawingml::GetCoordinate(oValue.value());
                        else
                            // Defaults from the spec: left/right: 91440 EMU, top/bottom: 45720 EMU
                            pTextBody->getTextProperties().moInsets[i]
                                = (aInsets[i] == XML_lIns || aInsets[i] == XML_rIns) ? 254 : 127;
                    }
                }

                mpShapePtr->setTextBody(pTextBody);
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
                        uno::Any(getBaseToken(nElementToken) == XML_spAutoFit));
                else
                    xPropertySet->setPropertyValue(
                        "TextAutoGrowHeight",
                        uno::Any(getBaseToken(nElementToken) == XML_spAutoFit));
            }
        }
        break;
        case XML_prstTxWarp:
            if (rAttribs.hasAttribute(XML_prst))
            {
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                if (xPropertySet.is())
                {
                    std::optional<OUString> presetShapeName = rAttribs.getString(XML_prst);
                    const OUString& preset = presetShapeName.value();
                    comphelper::SequenceAsHashMap aCustomShapeGeometry(
                        xPropertySet->getPropertyValue("CustomShapeGeometry"));
                    aCustomShapeGeometry["PresetTextWarp"] <<= preset;
                    xPropertySet->setPropertyValue(
                        "CustomShapeGeometry",
                        uno::Any(aCustomShapeGeometry.getAsConstPropertyValueList()));
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
                std::optional<OUString> id = rAttribs.getString(XML_id);
                if (id.has_value())
                {
                    oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr;
                    linkedTxtBoxAttr.id = id.value().toInt32();
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
            std::optional<OUString> id = rAttribs.getString(XML_id);
            std::optional<OUString> seq = rAttribs.getString(XML_seq);
            if (id.has_value() && seq.has_value())
            {
                oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr;
                linkedTxtBoxAttr.id = id.value().toInt32();
                linkedTxtBoxAttr.seq = seq.value().toInt32();
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
