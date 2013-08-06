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

#ifndef IDOCUMENTSETTINGACCESS_HXX_INCLUDED
#define IDOCUMENTSETTINGACCESS_HXX_INCLUDED

#include <tools/solar.h>
#include <rtl/ref.hxx>
#include <chcmprse.hxx>
#include <fldupde.hxx>

class SvxForbiddenCharactersTable;
namespace com { namespace sun { namespace star { namespace i18n { struct ForbiddenCharacters; } } } }

 /** Provides access to settings of a document
 */
 class IDocumentSettingAccess
 {
 public:
     enum DocumentSettingId
     {
         // COMPATIBILITY FLAGS START
         PARA_SPACE_MAX,
         PARA_SPACE_MAX_AT_PAGES,

         TAB_COMPAT,

         ADD_FLY_OFFSETS,

         OLD_NUMBERING,

         ADD_EXT_LEADING,
         USE_VIRTUAL_DEVICE,
         USE_HIRES_VIRTUAL_DEVICE,
         OLD_LINE_SPACING,
         ADD_PARA_SPACING_TO_TABLE_CELLS,
         USE_FORMER_OBJECT_POS,
         USE_FORMER_TEXT_WRAPPING,
         CONSIDER_WRAP_ON_OBJECT_POSITION,

         IGNORE_FIRST_LINE_INDENT_IN_NUMBERING,
         DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK,
         DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT,
         OUTLINE_LEVEL_YIELDS_OUTLINE_RULE,

         DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE,
         TABLE_ROW_KEEP,
         IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION,
         CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME,

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
         // MS Word still wraps text around objects with less space than LO would.
         SURROUND_TEXT_WRAP_SMALL,
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
         FLOATTABLE_NOMARGINS,
         EMBED_FONTS,
         EMBED_SYSTEM_FONTS
     };

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
    virtual const com::sun::star::i18n::ForbiddenCharacters*
        getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const = 0;

    /** Set the forbidden characters.

       @param nLang
       [in] the language for which the forbidden character list should be set.

       @param rForbiddenCharacters
       [in] the new list of forbidden characters for language lang.
    */
    virtual void setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang,
                                        /*[in]*/ const com::sun::star::i18n::ForbiddenCharacters& rForbiddenCharacters ) = 0;

    /** Get the forbidden character table and creates one if necessary.

       @returns
       the forbidden characters table.
    */
    virtual rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() = 0;

    /** Get the forbidden character table.

       @returns
       the forbidden characters table.
    */
    virtual const rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() const = 0;

    /** Get the current link update mode.

       @param bGlobalSettings
       [in] if set, the link update mode is obtained from the module,
            if it is set to GLOBALSETTING

       @returns
       the current link update mode.
    */
    virtual sal_uInt16 getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const = 0;

    /** Set the current link update mode.

       @param nMode
       [in] the new link update mode.
    */
    virtual void setLinkUpdateMode( /*[in]*/ sal_uInt16 nMode ) = 0;

    /** Get the current field update mode.

       @param bGlobalSettings
       [in] if set, the field update mode is obtained from the module,
            if it is set to GLOBALSETTING

       @returns
       the current field update mode.
    */
    virtual SwFldUpdateFlags getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const = 0;

    /** Set the current field update mode.

       @param nMode
       [in] the new field update mode.
    */
    virtual void setFieldUpdateFlags( /*[in]*/ SwFldUpdateFlags nMode )  = 0;

    /** Get the character compression type for Asian characters.

       @returns
       the current character compression mode.
    */
    virtual SwCharCompressType getCharacterCompressionType() const = 0;

    /** Set the character compression type for Asian characters.

       @param nMode
       [in] the new character compression type.
    */
    virtual void setCharacterCompressionType( /*[in]*/SwCharCompressType nType ) = 0;

protected:
    virtual ~IDocumentSettingAccess() {};
 };

#endif // IDOCUMENTSETTINGACCESS_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
