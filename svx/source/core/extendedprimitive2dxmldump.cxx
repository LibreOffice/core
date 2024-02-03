/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <extendedprimitive2dxmldump.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <sdr/primitive2d/sdrcellprimitive.hxx>
#include <tools/XmlWriter.hxx>

namespace svx
{
ExtendedPrimitive2dXmlDump::ExtendedPrimitive2dXmlDump() {}

ExtendedPrimitive2dXmlDump::~ExtendedPrimitive2dXmlDump() {}

bool ExtendedPrimitive2dXmlDump::decomposeAndWrite(
    const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive, ::tools::XmlWriter& rWriter)
{
    sal_uInt32 nId = rPrimitive.getPrimitive2DID();
    if (nId == PRIMITIVE2D_ID_SDRCELLPRIMITIVE2D)
    {
        OUString sCurrentElementTag = idToString(nId);
        rWriter.startElement("sdrCell");
        rWriter.attribute("id", sCurrentElementTag);
        rWriter.attribute("idNumber", nId);

        auto& rSdrCellPrimitive2D
            = dynamic_cast<const drawinglayer::primitive2d::SdrCellPrimitive2D&>(rPrimitive);
        rWriter.attribute("transparenceForShadow",
                          OString::number(rSdrCellPrimitive2D.getTransparenceForShadow()));

        drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
        rPrimitive.get2DDecomposition(aPrimitiveContainer,
                                      drawinglayer::geometry::ViewInformation2D());
        Primitive2dXmlDump::decomposeAndWrite(aPrimitiveContainer, rWriter);
        rWriter.endElement();
        return true;
    }
    return false;
}

} // end namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
