/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef OOX_DRAWINGNML_TEXTFONT_HXX
#define OOX_DRAWINGNML_TEXTFONT_HXX

#include <rtl/ustring.hxx>

namespace oox { class AttributeList; }
namespace oox { namespace core { class XmlFilterBase; } }

namespace oox {
namespace drawingml {

// ============================================================================

/** carries a CT_TextFont*/
class TextFont
{
public:
    explicit            TextFont();

    /** Sets attributes from the passed attribute list. */
    void                setAttributes( const AttributeList& rAttribs );

    /** Overwrites this text font with the passed text font, if it is used. */
    void                assignIfUsed( const TextFont& rTextFont );

    /** Returns the font name, pitch, and family; tries to resolve theme
        placeholder names, e.g. '+mj-lt' for the major latin theme font. */
    bool                getFontData(
                            OUString& rFontName,
                            sal_Int16& rnFontPitch,
                            sal_Int16& rnFontFamily,
                            const ::oox::core::XmlFilterBase& rFilter ) const;

private:
    bool                implGetFontData(
                            OUString& rFontName,
                            sal_Int16& rnFontPitch,
                            sal_Int16& rnFontFamily ) const;

private:
    OUString     maTypeface;
    OUString     maPanose;
    sal_Int32           mnPitch;
    sal_Int32           mnCharset;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
