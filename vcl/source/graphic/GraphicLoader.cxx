/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/GraphicLoader.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/wmf.hxx>

using namespace css;

namespace vcl
{
namespace graphic
{
Graphic loadFromURL(OUString const& rURL, sal_Int16 nExtWidth, sal_Int16 nExtHeight,
                    sal_Int16 nExtMapMode)
{
    // Define APM Header if goal height and width are defined
    WmfExternal aExtHeader;
    aExtHeader.xExt = nExtWidth;
    aExtHeader.yExt = nExtHeight;
    aExtHeader.mapMode = nExtMapMode;
    WmfExternal* pExtHeader = nullptr;
    if (nExtMapMode > 0)
        pExtHeader = &aExtHeader;

    Graphic aGraphic;

    std::unique_ptr<SvStream> pInputStream;
    pInputStream.reset(utl::UcbStreamHelper::CreateStream(rURL, StreamMode::READ));

    if (pInputStream)
    {
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

        ErrCode nError
            = rFilter.ImportGraphic(aGraphic, rURL, *pInputStream, GRFILTER_FORMAT_DONTKNOW,
                                    nullptr, GraphicFilterImportFlags::NONE, pExtHeader);
        if (nError != ERRCODE_NONE || aGraphic.GetType() == GraphicType::NONE)
            return Graphic();
    }

    return aGraphic;
}

Graphic loadFromURL(OUString const& rURL) { return loadFromURL(rURL, 0, 0, 0); }
}
} // end vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
