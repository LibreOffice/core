/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
                            ::rtl::OUString& rFontName,
                            sal_Int16 rnFontPitch,
                            sal_Int16& rnFontFamily,
                            const ::oox::core::XmlFilterBase& rFilter ) const;

private:
    bool                implGetFontData(
                            ::rtl::OUString& rFontName,
                            sal_Int16 rnFontPitch,
                            sal_Int16& rnFontFamily ) const;

private:
    ::rtl::OUString     maTypeface;
    ::rtl::OUString     maPanose;
    sal_Int32           mnPitch;
    sal_Int32           mnCharset;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

