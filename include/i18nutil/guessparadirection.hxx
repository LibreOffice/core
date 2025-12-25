/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <i18nutil/i18nutildllapi.h>
#include <com/sun/star/text/ScriptHintType.hpp>
#include <rtl/ustring.hxx>
#include <optional>
#include <memory>
#include <vector>

namespace i18nutil
{
enum class ParagraphDirection
{
    Ambiguous,
    LeftToRight,
    RightToLeft
};

I18NUTIL_DLLPUBLIC ParagraphDirection GuessParagraphDirection(const OUString& rText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
