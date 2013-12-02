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

using namespace com::sun::star;

namespace oox { namespace shape {

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

oox::drawingml::ShapePtr WpsContext::getShape()
{
    return mpShape;
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
                OptValue<OUString> oVert = rAttribs.getString(XML_vert);
                if (oVert.has() && oVert.get() == "vert270")
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
                        uno::Reference<beans::XPropertySet> xPropertySet(xTextCursor, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("CharRotation", uno::makeAny(sal_Int16(900)));
                    }
                }

                // Handle inset attributes for Writer textframes.
                sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                boost::optional<sal_Int32> oInsets[4];
                for (size_t i = 0; i < SAL_N_ELEMENTS(aInsets); ++i)
                {
                    OptValue<OUString> oValue = rAttribs.getString(aInsets[i]);
                    if (oValue.has())
                        oInsets[i] = oox::drawingml::GetCoordinate(oValue.get());
                }
                OUString aProps[] = { OUString("LeftBorderDistance"), OUString("TopBorderDistance"), OUString("RightBorderDistance"), OUString("BottomBorderDistance") };
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                for (size_t i = 0; i < SAL_N_ELEMENTS(aProps); ++i)
                    if (oInsets[i])
                        xPropertySet->setPropertyValue(aProps[i], uno::makeAny(*oInsets[i]));
            }
            break;
        case XML_txbx:
            mpShape->setServiceName("com.sun.star.text.TextFrame");
            break;
        default:
            SAL_WARN("oox", "WpsContext::createFastChildContext: unhandled element:" << getBaseToken(nElementToken));
            break;
    }
    return 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
