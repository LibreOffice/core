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

#ifndef INCLUDED_OOX_VML_VMLTEXTBOX_HXX
#define INCLUDED_OOX_VML_VMLTEXTBOX_HXX

#include <cstddef>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <oox/dllapi.h>
#include <oox/helper/helper.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace drawing { class XShape; }
}

namespace oox::vml {

struct ShapeTypeModel;


/// A text paragraph in a textbox.
struct TextParagraphModel
{
    OptValue<OUString> moParaAdjust; ///< Paragraph adjust (left, center, right, etc.)
    OptValue<OUString> moParaStyleName;
};

/** Font settings for a text portion in a textbox. */
struct OOX_DLLPUBLIC TextFontModel
{
    OptValue< OUString > moName;     ///< Font name.
    OptValue< OUString > moNameAsian; ///< Asian font name.
    OptValue< OUString > moNameComplex; ///< Complex font name.
    OptValue< OUString > moColor;    ///< Font color, HTML encoded, sort of.
    OptValue< sal_Int32 > monSize;          ///< Font size in twips.
    OptValue< sal_Int32 > monUnderline;     ///< Single or double underline.
    OptValue< sal_Int32 > monEscapement;    ///< Subscript or superscript.
    OptValue< bool >    mobBold;
    OptValue< bool >    mobItalic;
    OptValue< bool >    mobStrikeout;
    OptValue<sal_Int32> monSpacing;

    explicit            TextFontModel();
};


/** A text portion in a textbox with the same formatting for all characters. */
struct TextPortionModel
{
    TextParagraphModel  maParagraph;
    TextFontModel       maFont;
    OUString     maText;

    explicit            TextPortionModel( const TextParagraphModel& rParagraph, const TextFontModel& rFont, const OUString& rText );
};


/** The textbox contains all text contents and properties. */
class OOX_DLLPUBLIC TextBox
{
public:
    explicit            TextBox(ShapeTypeModel& rTypeModel);

    /** Appends a new text portion to the textbox. */
    void                appendPortion( const TextParagraphModel& rParagraph, const TextFontModel& rFont, const OUString& rText );

    /** Returns the current number of text portions. */
    size_t       getPortionCount() const { return maPortions.size(); }
    /** Returns the font settings of the first text portion. */
    const TextFontModel* getFirstFont() const;
    /** Returns the entire text of all text portions. */
    OUString     getText() const;
    void convert(const css::uno::Reference<css::drawing::XShape>& xShape) const;

    ShapeTypeModel&     mrTypeModel;
    /// Text distance from the border (inset attribute of v:textbox), valid only if set.
    bool borderDistanceSet;
    int borderDistanceLeft, borderDistanceTop, borderDistanceRight, borderDistanceBottom;
    OUString maLayoutFlow;
    OUString msNextTextbox;

private:
    typedef ::std::vector< TextPortionModel > PortionVector;

    PortionVector       maPortions;
};


} // namespace oox::vml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
