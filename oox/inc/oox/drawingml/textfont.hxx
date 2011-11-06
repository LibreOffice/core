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

