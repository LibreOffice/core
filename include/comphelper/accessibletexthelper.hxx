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
        ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator >             m_xBreakIter;
        ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCharacterClassification >   m_xCharClass;

    protected:
        OCommonAccessibleText();
        virtual ~OCommonAccessibleText();

        ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator >             implGetBreakIterator();
        ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCharacterClassification >   implGetCharacterClassification();
        static bool                      implIsValidBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nLength );
        static bool                      implIsValidIndex( sal_Int32 nIndex, sal_Int32 nLength );
        static bool                      implIsValidRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex, sal_Int32 nLength );
        virtual OUString                 implGetText() = 0;
        virtual ::com::sun::star::lang::Locale  implGetLocale() = 0;
        virtual void                     implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) = 0;
        void                             implGetGlyphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        bool                             implGetWordBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        void                             implGetSentenceBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                     implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                     implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );

        /** non-virtual versions of the methods
        */
        sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException);
        OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException);
        OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
        OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    public:

        /** Helper method, that detects the difference between
            two strings and returns the deleted selection and
            the inserted selection if available.

            @returns true if there are differences between the
            two strings and false if both are equal

            @see ::com::sun::star::accessibility::AccessibleEventId
                 ::com::sun::star::accessibility::TextSegment
        */
        static bool implInitTextChangedEvent(
            const OUString& rOldString,
            const OUString& rNewString,
            /*out*/ ::com::sun::star::uno::Any& rDeleted,
            /*out*/ ::com::sun::star::uno::Any& rInserted); // throw()
    };



    // OAccessibleTextHelper


    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessibleText
                                >   OAccessibleTextHelper_Base;

    /** a helper class for implementing an AccessibleExtendedComponent which at the same time
        supports an XAccessibleText interface
    */
    class COMPHELPER_DLLPUBLIC OAccessibleTextHelper : public OAccessibleExtendedComponentHelper,
                                  public OCommonAccessibleText,
                                  public OAccessibleTextHelper_Base
    {
    protected:
        // see the respective base class ctor for an extensive comment on this, please
        OAccessibleTextHelper( IMutex* _pExternalLock );

    public:
        // XInterface
        DECLARE_XINTERFACE( )

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleText
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLETEXTHELPER_HXX




//  OAccessibleTextHelper is a helper class for implementing the
//  XAccessibleText interface.

//  The following methods have a default implementation:

//      getCharacter
//      getCharacterCount
//      getSelectedText
//      getSelectionStart
//      getSelectionEnd
//      getText
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
