/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVX_SPELL_PORTIONS_HXX
#define SVX_SPELL_PORTIONS_HXX

#include <i18npool/lang.h>
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
    rtl::OUString   sText;
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
    /** for wrong words this reference is filled with the error informations otherwise
        it's an empty reference
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives> xAlternatives;
    /** determines whether the error type is a grammar error
    */
    bool bIsGrammarError;
    /** contains the grammar error information
    */
    com::sun::star::linguistic2::SingleProofreadingError aGrammarError;
    /** provides access to the grammar checker interface
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XProofreader > xGrammarChecker;
    /** marks portion as to-be-ignored. This is a return parameter.
     */
    /** contains the proposed dialog title if the proof reading component provides one.
     */
    rtl::OUString   sDialogTitle;

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
