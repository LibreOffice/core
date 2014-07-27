/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpsContext.hxx"
#include <oox/drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/shapestylecontext.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <oox/drawingml/drawingmltypes.hxx>

using namespace com::sun::star;

namespace oox
{
namespace shape
{

WpsContext::WpsContext(ContextHandler2Helper& rParent, uno::Reference<drawing::XShape> xShape)
    : ContextHandler2(rParent),
      mxShape(xShape)
{
    mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.CustomShape"));
    mpShape->setWps(true);
}

WpsContext::~WpsContext()
{
}

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
                    comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
                    aCustomShapeGeometry["TextPreRotateAngle"] = uno::makeAny(sal_Int32(-270));
                    xPropertySet->setPropertyValue("CustomShapeGeometry", uno::makeAny(aCustomShapeGeometry.getAsConstPropertyValueList()));
                }
            }

            if (xServiceInfo.is())
            {
                bool bTextFrame = xServiceInfo->supportsService("com.sun.star.text.TextFrame");
                // Handle inset attributes for Writer textframes.
                sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                boost::optional<sal_Int32> oInsets[4];
                for (size_t i = 0; i < SAL_N_ELEMENTS(aInsets); ++i)
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
                for (size_t i = 0; i < SAL_N_ELEMENTS(bTextFrame ? aProps : aShapeProps); ++i)
                    if (oInsets[i])
                        xPropertySet->setPropertyValue((bTextFrame ? aProps : aShapeProps)[i], uno::makeAny(*oInsets[i]));
            }

            // Handle text vertical adjustment inside a text frame
            if (rAttribs.hasAttribute(XML_anchor))
            {
                drawing::TextVerticalAdjust eAdjust = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                xPropertySet->setPropertyValue("TextVerticalAdjust", uno::makeAny(eAdjust));
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
            OUString preset = presetShapeName.get();
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
    return 0;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
