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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLETEXTHELPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLETEXTHELPER_HXX

#include <config_options.h>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>

namespace com::sun::star::i18n { class XBreakIterator; }
namespace com::sun::star::i18n { class XCharacterClassification; }
namespace com::sun::star::i18n { struct Boundary; }

namespace comphelper
{


    // OCommonAccessibleText

    /** base class encapsulating common functionality for the helper classes implementing
        the XAccessibleText
    */
    class COMPHELPER_DLLPUBLIC OCommonAccessibleText
    {
    private:
        css::uno::Reference < css::i18n::XBreakIterator >             m_xBreakIter;
        css::uno::Reference < css::i18n::XCharacterClassification >   m_xCharClass;

    protected:
        OCommonAccessibleText();
        virtual ~OCommonAccessibleText();

        css::uno::Reference < css::i18n::XBreakIterator > const &            implGetBreakIterator();
        css::uno::Reference < css::i18n::XCharacterClassification > const &  implGetCharacterClassification();
        static bool                      implIsValidBoundary( css::i18n::Boundary const & rBoundary, sal_Int32 nLength );
        static bool                      implIsValidIndex( sal_Int32 nIndex, sal_Int32 nLength );
        static bool                      implIsValidRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex, sal_Int32 nLength );
        static sal_Unicode               implGetCharacter( std::u16string_view rText, sal_Int32 nIndex );
        static OUString                  implGetTextRange( std::u16string_view rText, sal_Int32 nStartIndex, sal_Int32 nEndIndex );
        virtual OUString                 implGetText() = 0;
        virtual css::lang::Locale        implGetLocale() = 0;
        virtual void                     implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) = 0;
        void                             implGetGlyphBoundary( const OUString& rText, css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        bool                             implGetWordBoundary( const OUString& rText, css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        void                             implGetSentenceBoundary( const OUString& rText, css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                     implGetParagraphBoundary( const OUString& rText, css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                     implGetLineBoundary( const OUString& rText, css::i18n::Boundary& rBoundary, sal_Int32 nIndex );

        /** non-virtual versions of the methods

            @throws css::lang::IndexOutOfBoundsException
            @throws css::uno::RuntimeException
        */
        OUString getSelectedText();
        /// @throws css::uno::RuntimeException
        sal_Int32 getSelectionStart();
        /// @throws css::uno::RuntimeException
        sal_Int32 getSelectionEnd();
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::accessibility::TextSegment getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::accessibility::TextSegment getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::accessibility::TextSegment getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType );

    public:

        /** Helper method, that detects the difference between
            two strings and returns the deleted selection and
            the inserted selection if available.

            @returns true if there are differences between the
            two strings and false if both are equal

            @see css::accessibility::AccessibleEventId
                 css::accessibility::TextSegment
        */
        static bool implInitTextChangedEvent(
            std::u16string_view rOldString,
            std::u16string_view rNewString,
            /*out*/ css::uno::Any& rDeleted,
            /*out*/ css::uno::Any& rInserted); // throw()
    };


    // OAccessibleTextHelper


    /** a helper class for implementing an AccessibleExtendedComponent which at the same time
        supports an XAccessibleText interface
    */
    class UNLESS_MERGELIBS_MORE(COMPHELPER_DLLPUBLIC) OAccessibleTextHelper : public cppu::ImplInheritanceHelper<
                                                           OAccessibleExtendedComponentHelper,
                                                           css::accessibility::XAccessibleText>,
                                  public OCommonAccessibleText
    {
    private:
        OAccessibleTextHelper(OAccessibleTextHelper const &) = delete;
        OAccessibleTextHelper(OAccessibleTextHelper &&) = delete;
        void operator =(OAccessibleTextHelper const &) = delete;
        void operator =(OAccessibleTextHelper &&) = delete;

    protected:
        OAccessibleTextHelper();

    public:
        // XAccessibleText
        virtual OUString SAL_CALL getSelectedText() override final;
        virtual sal_Int32 SAL_CALL getSelectionStart() override final;
        virtual sal_Int32 SAL_CALL getSelectionEnd() override final;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override final;
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override final;
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override final;
    };


}   // namespace comphelper


//  OAccessibleTextHelper is a helper class for implementing the
//  XAccessibleText interface.

//  The following methods have a default implementation:

//      getCharacter
//      getSelectedText
//      getSelectionStart
//      getSelectionEnd
//      getTextRange
//      getTextAtIndex
//      getTextBeforeIndex
//      getTextBehindIndex

//  The following methods must be overridden by derived classes:

//      implGetText
//      implGetLocale
//      implGetSelection
//      getCaretPosition
//      setCaretPosition
//      getCharacterAttributes
//      getCharacterBounds
//      getIndexAtPoint
//      setSelection
//      copyText

#endif // INCLUDED_COMPHELPER_ACCESSIBLETEXTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
