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
#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#define SVX_HANGUL_HANJA_CONVERSION_HXX

#include <vcl/window.hxx>
#include <memory>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include "editeng/editengdllapi.h"

//.............................................................................
namespace editeng
{
//.............................................................................

    class HangulHanjaConversion_Impl;

    //=========================================================================
    //= HangulHanjaConversion
    //=========================================================================
    /** encapsulates Hangul-Hanja conversion functionality

        <p>terminology:
            <ul><li>A <b>text <em>portion</em></b> is some (potentially large) piece of text
                which is to be analyzed for convertible sub-strings.</li>
                <li>A <b>text <em>unit</em></b> is a sub string in a text portion, which is
                to be converted as a whole.</li>
            </ul>
            For instance, you could have two independent selections within your document, which are then
            two text portions. A text unit would be single Hangul/Hanja words within a portion, or even
            single Hangul syllabills when "replace by character" is enabled.
        </p>
    */
    class EDITENG_DLLPUBLIC HangulHanjaConversion
    {
        friend class HangulHanjaConversion_Impl;

    public:
        enum ReplacementAction
        {
            eExchange,              // simply exchange one text with another
            eReplacementBracketed,  // keep the original, and put the replacement in brackets after it
            eOriginalBracketed,     // replace the original text, but put it in brackeds after the replacement
            eReplacementAbove,      // keep the original, and put the replacement text as ruby text above it
            eOriginalAbove,         // replace the original text, but put it as ruby text above it
            eReplacementBelow,      // keep the original, and put the replacement text as ruby text below it
            eOriginalBelow          // replace the original text, but put it as ruby text below it
        };

        enum ConversionType             // does not specify direction...
        {
            eConvHangulHanja,           // Korean Hangul/Hanja conversion
            eConvSimplifiedTraditional  // Chinese simplified / Chinese traditional conversion
        };

        // Note: conversion direction for eConvSimplifiedTraditional is
        // specified by source language.
        // This one is for Hangul/Hanja where source and target language
        // are the same.
        enum ConversionDirection
        {
            eHangulToHanja,
            eHanjaToHangul
        };

        enum ConversionFormat
        {
            eSimpleConversion,          // used for simplified / traditional Chinese as well
            eHangulBracketed,
            eHanjaBracketed,
            eRubyHanjaAbove,
            eRubyHanjaBelow,
            eRubyHangulAbove,
            eRubyHangulBelow
        };

    private:
        ::std::auto_ptr< HangulHanjaConversion_Impl >   m_pImpl;

        // used to set initial values of m_pImpl object from saved ones
        static sal_Bool             m_bUseSavedValues;  // defines if the followng two values should be used for initialization
        static sal_Bool             m_bTryBothDirectionsSave;
        static ConversionDirection  m_ePrimaryConversionDirectionSave;

        // Forbidden and not implemented.
        HangulHanjaConversion (const HangulHanjaConversion &);
        HangulHanjaConversion & operator= (const HangulHanjaConversion &);

    public:
        HangulHanjaConversion(
            Window* _pUIParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::lang::Locale& _rSourceLocale,
            const ::com::sun::star::lang::Locale& _rTargetLocale,
            const Font* _pTargetFont,
            sal_Int32 nOptions,
            sal_Bool _bIsInteractive
        );

        virtual ~HangulHanjaConversion( );

        // converts the whole document
        void    ConvertDocument();

        LanguageType    GetSourceLanguage() const;
        LanguageType    GetTargetLanguage() const;
        const Font *    GetTargetFont() const;
        sal_Int32       GetConversionOptions() const;
        sal_Bool        IsInteractive() const;

        // chinese text conversion
        static inline sal_Bool IsSimplified( LanguageType nLang );
        static inline sal_Bool IsTraditional( LanguageType nLang );
        static inline sal_Bool IsChinese( LanguageType nLang );
        static inline sal_Bool IsSimilarChinese( LanguageType nLang1, LanguageType nLang2 );

        // used to specify that the conversion direction states from the
        // last incarnation should be used as
        // initial conversion direction for the next incarnation.
        // (A hack used to transport a state information from
        // one incarnation to the next. Used in Writers text conversion...)
        static void     SetUseSavedConversionDirectionState( sal_Bool bVal );
        static sal_Bool IsUseSavedConversionDirectionState();

    protected:
        /** retrieves the next text portion which is to be analyzed

            <p>pseudo-abstract, needs to be overridden</p>

            @param _rNextPortion
                upon return, this must contain the next text portion
            @param _rLangOfPortion
                upon return, this must contain the language for the found text portion.
                (necessary for Chinese translation since there are 5 language variants
                too look for even if the 'source' language usually is only 'simplified'
                or 'traditional'.)
        */
        virtual void    GetNextPortion(
                ::rtl::OUString& /* [out] */ _rNextPortion,
                LanguageType& /* [out] */ _rLangOfPortion,
                sal_Bool /* [in] */ _bAllowImplicitChangesForNotConvertibleText );

        /** announces a new "current unit"

            <p>This will be called whenever it is necessary to interactively ask the user for
            a conversion. In such a case, a range within the current portion (see <member>GetNextPortion</member>)
            is presented to the user for chosing a substitution. Additionally, this method is called,
            so that derived classes can e.g. highlight this text range in a document view.</p>

            <p>Note that the indexes are relative to the most recent replace action. See
            <member>ReplaceUnit</member> for details.</p>

            @param _nUnitStart
                the start index of the unit

            @param _nUnitEnd
                the start index (exclusively!) of the unit.

            @param _bAllowImplicitChangesForNotConvertibleText
                allows implicit changes other than the text itself for the
                text parts not being convertible.
                Used for chinese translation to attribute all not convertible
                text (e.g. western text, empty paragraphs, spaces, ...) to
                the target language and target font of the conversion.
                This is to ensure that after the conversion any new text entered
                anywhere in the document will have the target language (of course
                CJK Language only) and target font (CJK font only) set.

            @see GetNextPortion
        */
        virtual void    HandleNewUnit( const sal_Int32 _nUnitStart, const sal_Int32 _nUnitEnd );

        /** replaces a text unit within a text portion with a new text

            <p>pseudo-abstract, needs to be overridden</p>

            <p>Note an important thing about the indicies: They are always relative to the <em>previous
            call</em> of ReplaceUnit. This means whe you get a call to ReplaceUnit, and replace some text
            in your document, than you have to remember the document position immediately <em>behind</em>
            the changed text. In a next call to ReplaceUnit, an index of <em>0</em> will denote exactly
            this position behind the previous replacement<br/>
            The reaons for this is that this class here does not know anything about your document structure,
            so after a replacement took place, it's impossible to address anything in the range from the
            beginning of the portion up to the replaced text.<br/>
            In the very first call to ReplaceUnit, an index of <em>0</em> denotes the very first position of
            the current portion.</p>

            <p>If the language of the text to be replaced is different from
            the target language (as given by 'GetTargetLanguage') for example
            when converting simplified Chinese from/to traditional Chinese
            the language attribute of the new text has to be changed as well,
            **and** the font is to be set to the default (document) font for
            that language.</p>

            @param _nUnitStart
                the start index of the range to replace

            @param _nUnitEnd
                the end index (exclusively!) of the range to replace. E.g., an index
                pair (4,5) indicates a range of length 1.

            @param _rOrigText
                the original text to be replaced (as returned by GetNextPortion).
                Since in Chinese conversion the original text is needed as well
                in order to only do the minimal necassry text changes and to keep
                as much attributes as possible this is supplied here as well.

            @param _rReplaceWith
                The replacement text

            @param _rOffsets
                An sequence matching the indices (characters) of _rReplaceWith
                to the indices of the characters in the original text they are
                replacing.
                This is necessary since some portions of the text may get
                converted in portions of different length than the original.
                The sequence will be empty if all conversions in the text are
                of equal length. That is if always the character at index i in
                _rOffsets is replacing the character at index i in the original
                text for all valid index values of i.

            @param _eAction
                replacement action to take

            @param pNewUnitLanguage
                if the replacement unit is required to have a new language that
                is specified here. If the language is to be left unchanged this
                is the 0 pointer.
        */
        virtual void    ReplaceUnit(
                            const sal_Int32 _nUnitStart, const sal_Int32 _nUnitEnd,
                            const ::rtl::OUString& _rOrigText,
                            const ::rtl::OUString& _rReplaceWith,
                            const ::com::sun::star::uno::Sequence< sal_Int32 > &_rOffsets,
                            ReplacementAction _eAction,
                            LanguageType *pNewUnitLanguage
                        );

        /** specifies if rubies are supported by the document implementing
            this class.

            @return
                <TRUE/> if rubies are supported.
        */
        virtual sal_Bool    HasRubySupport() const;
    };

    sal_Bool HangulHanjaConversion::IsSimplified( LanguageType nLang )
    {
        return  nLang == LANGUAGE_CHINESE_SIMPLIFIED ||
                nLang == LANGUAGE_CHINESE_SINGAPORE;
    }

    sal_Bool HangulHanjaConversion::IsTraditional( LanguageType nLang )
    {
        return  nLang == LANGUAGE_CHINESE_TRADITIONAL ||
                nLang == LANGUAGE_CHINESE_HONGKONG ||
                nLang == LANGUAGE_CHINESE_MACAU;
    }

    sal_Bool HangulHanjaConversion::IsChinese( LanguageType nLang )
    {
        return IsTraditional( nLang ) || IsSimplified( nLang );
    }

    sal_Bool HangulHanjaConversion::IsSimilarChinese( LanguageType nLang1, LanguageType nLang2 )
    {
        return (IsTraditional(nLang1) && IsTraditional(nLang2)) ||
               (IsSimplified(nLang1)  && IsSimplified(nLang2));
    }

//.............................................................................
}   // namespace svx
//.............................................................................

#endif // SVX_HANGUL_HANJA_CONVERSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
