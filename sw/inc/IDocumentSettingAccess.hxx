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

#pragma once

#include <sal/types.h>
#include "fldupde.hxx"
#include <i18nlangtag/lang.h>
#include <memory>

class SvxForbiddenCharactersTable;
namespace com::sun::star::i18n
{
struct ForbiddenCharacters;
}
enum class CharCompressType;

enum class DocumentSettingId
{
    // COMPATIBILITY FLAGS START
    PARA_SPACE_MAX,
    PARA_SPACE_MAX_AT_PAGES,

    TAB_COMPAT,

    ADD_FLY_OFFSETS,
    ADD_VERTICAL_FLY_OFFSETS,

    OLD_NUMBERING,

    ADD_EXT_LEADING,
    USE_VIRTUAL_DEVICE,
    USE_HIRES_VIRTUAL_DEVICE,
    OLD_LINE_SPACING,
    ADD_PARA_SPACING_TO_TABLE_CELLS,
    ADD_PARA_LINE_SPACING_TO_TABLE_CELLS,
    USE_FORMER_OBJECT_POS,
    USE_FORMER_TEXT_WRAPPING,
    CONSIDER_WRAP_ON_OBJECT_POSITION,

    IGNORE_FIRST_LINE_INDENT_IN_NUMBERING,
    DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK,
    TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK,
    DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT,
    // tdf#159382: MS Word compatible handling of space between footnote number and text
    NO_GAP_AFTER_NOTE_NUMBER,

    DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE,
    TABLE_ROW_KEEP,
    IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION,
    CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME,

    // tdf#104349 tdf#104668
    MS_WORD_COMP_TRAILING_BLANKS,
    // tdf#128197 MS Word in some modes can have line height based on shape height, not on font
    MS_WORD_COMP_MIN_LINE_HEIGHT_BY_FLY,
    UNIX_FORCE_ZERO_EXT_LEADING,
    TABS_RELATIVE_TO_INDENT,
    PROTECT_FORM,
    // #i89181#
    TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST,
    INVERT_BORDER_SPACING,
    COLLAPSE_EMPTY_CELL_PARA,
    SMALL_CAPS_PERCENTAGE_66,
    TAB_OVERFLOW,
    UNBREAKABLE_NUMBERINGS,
    CLIPPED_PICTURES,
    BACKGROUND_PARA_OVER_DRAWINGS,
    TAB_OVER_MARGIN,
    TAB_OVER_SPACING,
    // MS Word still wraps text around objects with less space than LO would.
    SURROUND_TEXT_WRAP_SMALL,
    PROP_LINE_SPACING_SHRINKS_FIRST_LINE,
    SUBTRACT_FLYS,
    // tdf#112443 disable off-page content positioning
    DISABLE_OFF_PAGE_POSITIONING,
    EMPTY_DB_FIELD_HIDES_PARA,
    // tdf#129448: Auto first-line indent should not be effected by line space
    AUTO_FIRST_LINE_INDENT_DISREGARD_LINE_SPACE,
    HYPHENATE_URLS, ///< tdf#152952
    DO_NOT_BREAK_WRAPPED_TABLES,
    ALLOW_TEXT_AFTER_FLOATING_TABLE_BREAK,
    // tdf#119908 new paragraph justification
    JUSTIFY_LINES_WITH_SHRINKING,
    APPLY_TEXT_ATTR_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH,
    DO_NOT_MIRROR_RTL_DRAW_OBJS,
    // COMPATIBILITY FLAGS END
    BROWSE_MODE,
    HTML_MODE,
    GLOBAL_DOCUMENT,
    GLOBAL_DOCUMENT_SAVE_LINKS,
    LABEL_DOCUMENT,
    PURGE_OLE,
    KERN_ASIAN_PUNCTUATION,
    MATH_BASELINE_ALIGNMENT,
    STYLES_NODEFAULT,
    EMBED_FONTS,
    EMBED_USED_FONTS,
    EMBED_LATIN_SCRIPT_FONTS,
    EMBED_ASIAN_SCRIPT_FONTS,
    EMBED_COMPLEX_SCRIPT_FONTS,
    EMBED_SYSTEM_FONTS,
    APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING,
    CONTINUOUS_ENDNOTES,
    PROTECT_BOOKMARKS,
    PROTECT_FIELDS,
    HEADER_SPACING_BELOW_LAST_PARA,
    FRAME_AUTOWIDTH_WITH_MORE_PARA,
    GUTTER_AT_TOP,
    // footnoteContainer default position is the page end instead of the column end
    // only if "evenly distributed" is set, and "collected at the end" is not set
    FOOTNOTE_IN_COLUMN_TO_PAGEEND,
    // Should we display follow by symbol for numbered paragraph if numbering exists, but "None"?
    NO_NUMBERING_SHOW_FOLLOWBY,
    // drop cap punctuation: smaller dashes, bullet, asterisks, quotation marks etc.
    // by extending the rounding box of the glyph to the baseline
    DROP_CAP_PUNCTUATION,
    // render NBSP as standard-space-width (prettier when justified)
    USE_VARIABLE_WIDTH_NBSP,
    // overlap background shapes if anchored in body
    PAINT_HELL_OVER_HEADER_FOOTER,
};

/** Provides access to settings of a document
 */
class IDocumentSettingAccess
{
public:
    /** Return the specified document setting.

       @param id
       [in] the identifier of the document setting to be returned.
       See above for a list of valid document setting identifiers.

       @returns
       the value of the requested document setting.
    */
    virtual bool get(/*[in]*/ DocumentSettingId id) const = 0;

    /** Set the specified document setting.

        @param id
        [in] the identifier of the document setting to be set.
        See above for a list of valid document setting identifiers.

        @param value
        [in] the new value of the specified document setting.
    */
    virtual void set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value) = 0;

    /** Return the forbidden characters.

       @param nLang
       [in] the language for which the forbidden character list is returned.

       @param bLocaleData
       [in] if set and there is no user defined forbidden character list for
            language lang, the default list for language lang will be returned.

       @returns
       a list of forbidden characters.
    */
    virtual const css::i18n::ForbiddenCharacters*
    getForbiddenCharacters(/*[in]*/ LanguageType nLang, /*[in]*/ bool bLocaleData) const = 0;

    /** Set the forbidden characters.

       @param nLang
       [in] the language for which the forbidden character list should be set.

       @param rForbiddenCharacters
       [in] the new list of forbidden characters for language lang.
    */
    virtual void
    setForbiddenCharacters(/*[in]*/ LanguageType nLang,
                           /*[in]*/ const css::i18n::ForbiddenCharacters& rForbiddenCharacters)
        = 0;

    /** Get the forbidden character table and creates one if necessary.

       @returns
       the forbidden characters table.
    */
    virtual std::shared_ptr<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() = 0;

    /** Get the forbidden character table.

       @returns
       the forbidden characters table.
    */
    virtual const std::shared_ptr<SvxForbiddenCharactersTable>&
    getForbiddenCharacterTable() const = 0;

    /** Get the current link update mode.

       @param bGlobalSettings
       [in] if set, the link update mode is obtained from the module,
            if it is set to GLOBALSETTING

       @returns
       the current link update mode.
    */
    virtual sal_uInt16 getLinkUpdateMode(/*[in]*/ bool bGlobalSettings) const = 0;

    /** Set the current link update mode.

       @param nMode
       [in] the new link update mode.
    */
    virtual void setLinkUpdateMode(/*[in]*/ sal_uInt16 nMode) = 0;

    /** Get the current field update mode.

       @param bGlobalSettings
       [in] if set, the field update mode is obtained from the module,
            if it is set to GLOBALSETTING

       @returns
       the current field update mode.
    */
    virtual SwFieldUpdateFlags getFieldUpdateFlags(/*[in]*/ bool bGlobalSettings) const = 0;

    /** Set the current field update mode.

       @param nMode
       [in] the new field update mode.
    */
    virtual void setFieldUpdateFlags(/*[in]*/ SwFieldUpdateFlags nMode) = 0;

    /** Get the character compression type for Asian characters.

       @returns
       the current character compression mode.
    */
    virtual CharCompressType getCharacterCompressionType() const = 0;

    /** Set the character compression type for Asian characters.

       @param nMode
       [in] the new character compression type.
    */
    virtual void setCharacterCompressionType(/*[in]*/ CharCompressType nType) = 0;

    /** Get the n32DummyCompatibilityOptions1
    */
    virtual sal_uInt32 Getn32DummyCompatibilityOptions1() const = 0;

    /** Set the n32DummyCompatibilityOptions1
    */
    virtual void Setn32DummyCompatibilityOptions1(const sal_uInt32 CompatibilityOptions1) = 0;

    /** Get the n32DummyCompatibilityOptions2
    */
    virtual sal_uInt32 Getn32DummyCompatibilityOptions2() const = 0;

    /** Set the n32DummyCompatibilityOptions2
    */
    virtual void Setn32DummyCompatibilityOptions2(const sal_uInt32 CompatibilityOptions2) = 0;

    virtual sal_Int32 getImagePreferredDPI() = 0;
    virtual void setImagePreferredDPI(sal_Int32 nValue) = 0;

protected:
    virtual ~IDocumentSettingAccess(){};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
