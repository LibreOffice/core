/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <BarCodeRender.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>

#include <backend/zint.h>

namespace {
    bool isZIntBitSet(struct zint_symbol *pSymbol, int x, int y)
    {
        // this is very odd; bit-masking !?
        int x_bit = x % 7;
        x /= 7;
        return pSymbol->encoded_data[y][x] & (1 << x_bit);
    }
}

namespace svx {

SdrObjGroup *BarCodeRender(const Rectangle &rLogicRect, const rtl::OUString &rData)
{
    SdrObjGroup *pShape = new SdrObjGroup();

    struct zint_symbol *pSymbol = ZBarcode_Create();

    pSymbol = ZBarcode_Create();

    pSymbol->symbology = BARCODE_QRCODE;

    pSymbol->output_options = 0; // no border
    pSymbol->border_width = 1;
    pSymbol->height = rLogicRect.GetSize().Height();
    pSymbol->option_2 = rLogicRect.GetSize().Width();
    pSymbol->option_1 = -1; // security level
    pSymbol->input_mode = UNICODE_MODE;
    pSymbol->show_hrt = 1; // bool: don't hide text
    pSymbol->option_3 = 0; // PDF417 may need some love here.
    pSymbol->scale = 1.0;

    OString aUtf8(rtl::OUStringToOString(rData, RTL_TEXTENCODING_UTF8));
    if (ZBarcode_Encode(pSymbol, (unsigned char*)aUtf8.getStr(),
                        aUtf8.getLength()))
    {
        SAL_WARN("svx", "Failed to encode string '" << rData << " '" <<
                 (pSymbol->errtxt ? pSymbol->errtxt : "<unknown>") << "'");
    }

    OUString aCaption((const char *)pSymbol->text,
                      pSymbol->text ? strlen((const char *)pSymbol->text) : 0,
                       RTL_TEXTENCODING_UTF8);

    // FIXME: use the pSymbol->errtxt as content if we fail ?
    // or translate that somehow.

    basegfx::B2DPolyPolygon aPolyPoly;
    basegfx::B2DPolygon aPoly;

    Point aP(rLogicRect.TopLeft());
    Size aS(rLogicRect.GetSize() );

    // FIXME: should we use ZBarcode_Render - and walk a list
    // of output rendering primitives instead ? [ easier? ]

    double xscale = aS.Width()/pSymbol->width;
    double yscale = aS.Height()/pSymbol->rows;
    double y = aP.Y();
    for (int row = 0; row < pSymbol->rows; row++)
    {
        for (int i = 0; i < pSymbol->width; i++)
        {
            if (isZIntBitSet(pSymbol, i, row))
            {
                // FIXME: really we want to build larger polygons
                // so we should at least scan horizontally to collate.

                // all this for a rectangle ?
                double xpos = aP.X() + xscale * i;
                aPoly.append(basegfx::B2DPoint(xpos, y));
                aPoly.append(basegfx::B2DPoint(xpos + xscale , y));
                aPoly.append(basegfx::B2DPoint(xpos + xscale , y + yscale));
                aPoly.append(basegfx::B2DPoint(xpos , y + yscale));
                aPoly.setClosed(true);
                aPolyPoly.append(aPoly);

                aPoly.clear();
            }
        }
        y += yscale * pSymbol->row_height[row];
    }

    SdrPathObj* pStroke = new SdrPathObj(OBJ_POLY, aPolyPoly, 0.5);
    pShape->GetSubList()->NbcInsertObject(pStroke);

    ZBarcode_Delete(pSymbol);

    return pShape;
}

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
