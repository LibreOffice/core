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

#ifndef INCLUDED_EDITENG_SPELLPORTIONS_HXX
#define INCLUDED_EDITENG_SPELLPORTIONS_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/SingleProofreadingError.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <vector>

namespace com{ namespace sun{ namespace star{ namespace linguistic2{
    class XSpellAlternatives;
}}}}

namespace svx{
/** contains a portion of text that has the same language attributes applied
    and belongs to the same script type.
 */
struct SpellPortion
{
    /** contains the text of the portion.
     */
    OUString   sText;
    /** Marks the portion as field, footnote symbol or any other special content that
     should be protected against unintentional deletion.
     */
    bool bIsField;
    /** Marks the portion hidden content that should not be touched by spell checking
        and not be removed like redlines. The creator of the portions has to take care
        for them.
     */
    bool bIsHidden;
    /** contains the language applied to the text. It has to match the script type.
     */
    LanguageType    eLanguage;
    /** for wrong words this reference is filled with the error information otherwise
        it's an empty reference
     */
    css::uno::Reference< css::linguistic2::XSpellAlternatives> xAlternatives;
    /** determines whether the error type is a grammar error
    */
    bool bIsGrammarError;
    /** contains the grammar error information
    */
    css::linguistic2::SingleProofreadingError aGrammarError;
    /** provides access to the grammar checker interface
     */
    css::uno::Reference< css::linguistic2::XProofreader > xGrammarChecker;
    /** marks portion as to-be-ignored. This is a return parameter.
     */
    /** contains the proposed dialog title if the proof reading component provides one.
     */
    OUString   sDialogTitle;

    bool bIgnoreThisError;
    SpellPortion() :
        bIsField(false),
        bIsHidden(false),
        eLanguage(LANGUAGE_DONTKNOW),
        bIsGrammarError(false),
        bIgnoreThisError(false)
        {
            aGrammarError.nErrorStart = aGrammarError.nErrorLength = aGrammarError.nErrorType = 0;
        }
};
typedef std::vector<SpellPortion> SpellPortions;
}//namespace svx
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
