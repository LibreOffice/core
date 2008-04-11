/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibletexthelper.hxx,v $
 * $Revision: 1.9 $
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

#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX

#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include "comphelper/comphelperdllapi.h"


//..............................................................................
namespace comphelper
{
//..............................................................................

    //==============================================================================
    // OCommonAccessibleText
    //==============================================================================
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
        sal_Bool                                implIsValidBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nLength );
        virtual sal_Bool                        implIsValidIndex( sal_Int32 nIndex, sal_Int32 nLength );
        virtual sal_Bool                        implIsValidRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex, sal_Int32 nLength );
        virtual ::rtl::OUString                 implGetText() = 0;
        virtual ::com::sun::star::lang::Locale  implGetLocale() = 0;
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) = 0;
        virtual void                            implGetGlyphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual sal_Bool                        implGetWordBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                            implGetSentenceBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                            implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                            implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );

        /** non-virtual versions of the methods
        */
        sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException);
        ::rtl::OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException);
        ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
        ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
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
            const rtl::OUString& rOldString,
            const rtl::OUString& rNewString,
            /*out*/ ::com::sun::star::uno::Any& rDeleted,
            /*out*/ ::com::sun::star::uno::Any& rInserted); // throw()
    };


    //==============================================================================
    // OAccessibleTextHelper
    //==============================================================================

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
        OAccessibleTextHelper();
        // see the respective base class ctor for an extensive comment on this, please
        OAccessibleTextHelper( IMutex* _pExternalLock );

    public:
        // XInterface
        DECLARE_XINTERFACE( )

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleText
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    };

//..............................................................................
}   // namespace comphelper
//..............................................................................

#endif // COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX


// -----------------------------------------------------------------------------
//
//  OAccessibleTextHelper is a helper class for implementing the
//  XAccessibleText interface.
//
//  The following methods have a default implementation:
//
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
//
//  The following methods must be overriden by derived classes:
//
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
//
// -----------------------------------------------------------------------------
