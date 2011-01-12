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

#ifndef OOX_VML_VMLTEXTBOX_HXX
#define OOX_VML_VMLTEXTBOX_HXX

#include <vector>
#include <rtl/ustring.hxx>
#include "oox/helper/helper.hxx"

namespace oox {
namespace vml {

// ============================================================================

/** Font settings for a text portion in a textbox. */
struct TextFontModel
{
    OptValue< ::rtl::OUString > moName;     /// Font name.
    OptValue< ::rtl::OUString > moColor;    /// Font color, HTML encoded, sort of.
    OptValue< sal_Int32 > monSize;          /// Font size in twips.
    OptValue< sal_Int32 > monUnderline;     /// Single or double underline.
    OptValue< sal_Int32 > monEscapement;    /// Subscript or superscript.
    OptValue< bool >    mobBold;
    OptValue< bool >    mobItalic;
    OptValue< bool >    mobStrikeout;

    explicit            TextFontModel();
};

// ============================================================================

/** A text portion in a textbox with the same formatting for all characters. */
struct TextPortionModel
{
    TextFontModel       maFont;
    ::rtl::OUString     maText;

    explicit            TextPortionModel( const TextFontModel& rFont, const ::rtl::OUString& rText );
};

// ============================================================================

/** The textbox contains all text contents and properties. */
class TextBox
{
public:
    explicit            TextBox();

    /** Appends a new text portion to the textbox. */
    void                appendPortion( const TextFontModel& rFont, const ::rtl::OUString& rText );

    /** Returns the current number of text portions. */
    inline size_t       getPortionCount() const { return maPortions.size(); }
    /** Returns the font settings of the first text portion. */
    const TextFontModel* getFirstFont() const;
    /** Returns the entire text of all text portions. */
    ::rtl::OUString     getText() const;

private:
    typedef ::std::vector< TextPortionModel > PortionVector;

    PortionVector       maPortions;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif
