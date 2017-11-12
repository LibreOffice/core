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

#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/comphelperdllapi.h>


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
        static sal_Unicode               implGetCharacter( const OUString& rText, sal_Int32 nIndex );
        static OUString                  implGetTextRange( const OUString& rTest, sal_Int32 nStartIndex, sal_Int32 nEndIndex );
        virtual OUString                 implGetText() = 0;
        virtual css::lang::Locale        implGetLocale() = 0;
        virtual void                     implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) = 0;
        void                             implGetGlyphBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        bool                             implGetWordBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        void                             implGetSentenceBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                     implGetParagraphBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                     implGetLineBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex );

        /** non-virtual versions of the methods

            @throws css::lang::IndexOutOfBoundsException
            @throws css::uno::RuntimeException
        */
        OUString SAL_CALL getSelectedText();
        /// @throws css::uno::RuntimeException
        sal_Int32 SAL_CALL getSelectionStart();
        /// @throws css::uno::RuntimeException
        sal_Int32 SAL_CALL getSelectionEnd();
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType );

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
            const OUString& rOldString,
            const OUString& rNewString,
            /*out*/ css::uno::Any& rDeleted,
            /*out*/ css::uno::Any& rInserted); // throw()
    };


    // OAccessibleTextHelper


    typedef ::cppu::ImplHelper1 <   css::accessibility::XAccessibleText
                                >   OAccessibleTextHelper_Base;

    /** a helper class for implementing an AccessibleExtendedComponent which at the same time
        supports an XAccessibleText interface
    */
    class COMPHELPER_DLLPUBLIC OAccessibleTextHelper : public OAccessibleExtendedComponentHelper,
                                  public OCommonAccessibleText,
                                  public OAccessibleTextHelper_Base
    {
    protected:
        OAccessibleTextHelper();

    public:
        // XInterface
        DECLARE_XINTERFACE( )

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleText
        virtual OUString SAL_CALL getSelectedText() override;
        virtual sal_Int32 SAL_CALL getSelectionStart() override;
        virtual sal_Int32 SAL_CALL getSelectionEnd() override;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLETEXTHELPER_HXX


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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
