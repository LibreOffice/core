/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
