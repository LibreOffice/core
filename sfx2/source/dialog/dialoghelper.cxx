/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotools/localedatawrapper.hxx>
#include <sfx2/dialoghelper.hxx>
#include <tools/datetime.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vector>

Size getParagraphPreviewOptimalSize(const OutputDevice& rReference)
{
    return rReference.LogicToPixel(Size(68 , 112), MapMode(MapUnit::MapAppFont));
}

Size getDrawPreviewOptimalSize(const vcl::Window *pReference)
{
    return pReference->LogicToPixel(Size(88, 42), MapMode(MapUnit::MapAppFont));
}

Size getPreviewStripSize(const OutputDevice& rReference)
{
    return rReference.LogicToPixel(Size(70 , 40), MapMode(MapUnit::MapAppFont));
}

Size getPreviewOptionsSize(const OutputDevice& rReference)
{
    return rReference.LogicToPixel(Size(70 , 27), MapMode(MapUnit::MapAppFont));
}

OUString getWidestTime(const LocaleDataWrapper& rWrapper)
{
    Date aDate(22, 12, 2000);
    tools::Time aTime(22, 59, 59);
    DateTime aDateTime(aDate, aTime);
    return formatTime(aDateTime, rWrapper);
}

OUString formatTime(const DateTime& rDateTime, const LocaleDataWrapper& rWrapper)
{
    return rWrapper.getDate(rDateTime) + " "
        + rWrapper.getTime(rDateTime, false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
