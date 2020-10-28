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

#include <com/sun/star/awt/XWindow.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/weld.hxx>

using namespace css;

namespace vcl::graphic
{
Graphic loadFromURL(OUString const& rURL, weld::Window* pParentWin)
{
    Graphic aGraphic;

    std::unique_ptr<SvStream> pInputStream = utl::UcbStreamHelper::CreateStream(
        rURL, StreamMode::READ, pParentWin ? pParentWin->GetXWindow() : nullptr);

    if (pInputStream)
    {
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

        ErrCode nError
            = rFilter.ImportGraphic(aGraphic, rURL, *pInputStream, GRFILTER_FORMAT_DONTKNOW,
                                    nullptr, GraphicFilterImportFlags::NONE,
                                    /*pExtHeader*/ static_cast<WmfExternal const*>(nullptr));
        if (nError != ERRCODE_NONE || aGraphic.GetType() == GraphicType::NONE)
            return Graphic();
    }

    return aGraphic;
}
} // end vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
