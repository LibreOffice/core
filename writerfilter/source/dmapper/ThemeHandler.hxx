/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/PropertyValue.hpp>
#include <i18nlangtag/lang.h>
#include <memory>
#include <oox/drawingml/theme.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter::dmapper
{
class ThemeHandler
{
private:
    oox::drawingml::ThemePtr mpTheme;
    OUString maThemeFontLangEastAsia;
    OUString maThemeFontLangBidi;

public:
    ThemeHandler(oox::drawingml::ThemePtr const& pTheme,
                 const css::uno::Sequence<css::beans::PropertyValue>& rLangProperties);
    OUString getFontNameForTheme(const Id id) const;
    static OUString getStringForTheme(const Id id);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
