/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <oox/core/contexthandler.hxx>
#include <oox/core/fragmenthandler2.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <optional>

namespace oox
{
class AttributeList;
}

namespace oox::ppt
{
/** Imported embedded font information */
struct EmbeddedFont
{
    OUString aTypeface;
    OUString aPanose;
    sal_Int32 nPitchFamily = -1;
    sal_Int32 nCharset = -1;

    OUString aRegularID;
    OUString aBoldID;
    OUString aItalicID;
    OUString aBoldItalicID;
};

/** Import context handling the embedded font list (p:embeddedFontLst) */
class EmbeddedFontListContext final : public ::oox::core::FragmentHandler2
{
    std::optional<EmbeddedFont> moCurrentFont;
    EmbeddedFontsHelper maEmbeddedFontHelper;

public:
    EmbeddedFontListContext(oox::core::FragmentHandler2 const& rParent);
    ~EmbeddedFontListContext() override;

    oox::core::ContextHandlerRef onCreateContext(sal_Int32 aElementToken,
                                                 const AttributeList& rAttribs) override;
    void onEndElement() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
