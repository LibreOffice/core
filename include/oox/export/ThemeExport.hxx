/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <oox/dllapi.h>
#include <oox/core/xmlfilterbase.hxx>

namespace model
{
class Theme;
class FontScheme;
}

namespace oox
{
class OOX_DLLPUBLIC ThemeExport
{
private:
    oox::core::XmlFilterBase* mpFilterBase;

public:
    ThemeExport(oox::core::XmlFilterBase* pFilterBase);

    void write(OUString const& rPath, model::Theme const& rTheme);

private:
    static bool writeColorSet(sax_fastparser::FSHelperPtr pFS, model::Theme const& rTheme);
    static bool writeFontScheme(sax_fastparser::FSHelperPtr pFS,
                                model::FontScheme const& rFontScheme);
    static bool writeFormatScheme(sax_fastparser::FSHelperPtr pFS);
};

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
