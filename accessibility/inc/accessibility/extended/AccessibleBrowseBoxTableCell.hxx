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
#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXTABLECELL_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXTABLECELL_HXX

#include "accessibility/extended/accessiblebrowseboxcell.hxx"
#include <comphelper/accessibletexthelper.hxx>
#include <cppuhelper/implbase.hxx>

namespace accessibility
{
    typedef ::cppu::ImplHelper  <   css::accessibility::XAccessibleText
                                ,   css::accessibility::XAccessible
                                >   AccessibleTextHelper_BASE;

    // implementation of a table cell of BrowseBox
    class AccessibleBrowseBoxTableCell  :public AccessibleBrowseBoxCell
                                        ,public AccessibleTextHelper_BASE
                                        ,public ::comphelper::OCommonAccessibleText
    {
    private:
        sal_Int32   m_nOffset;

    protected:
        // OCommonAccessibleText
        virtual OUString                        implGetText() override;
        virtual css::lang::Locale               implGetLocale() override;
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    public:
        AccessibleBrowseBoxTableCell( const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                                    ::svt::IAccessibleTableProvider& _rBrowseBox,
                                    const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                                    sal_Int32 _nRowId,
                                    sal_uInt16 _nColId,
                                    sal_Int32 _nOffset );

        void    nameChanged( const OUString& rNewName, const OUString& rOldName );

        // XInterface -------------------------------------------------------------

        /** Queries for a new interface. */
        css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
            throw ( css::uno::RuntimeException, std::exception ) override;

        /** Aquires the object (calls acquire() on base class). */
        virtual void SAL_CALL acquire() throw () override;

        /** Releases the object (calls release() on base class). */
        virtual void SAL_CALL release() throw () override;

        // XEventListener
        using AccessibleBrowseBoxBase::disposing;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
            throw(css::uno::RuntimeException, std::exception) override;

        /** @return  The index of this object among the parent's children. */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
            throw ( css::uno::RuntimeException, std::exception ) override;

        /** @return
                The name of this class.
        */
        virtual OUString SAL_CALL getImplementationName()
            throw ( css::uno::RuntimeException, std::exception ) override;

        /** @return
                The count of visible children.
        */
        virtual sal_Int32 SAL_CALL getAccessibleChildCount()
            throw ( css::uno::RuntimeException, std::exception ) override;

        /** @return
                The XAccessible interface of the specified child.
        */
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
            getAccessibleChild( sal_Int32 nChildIndex )
                throw ( css::lang::IndexOutOfBoundsException,
                        css::uno::RuntimeException, std::exception ) override;

        /** Creates a new AccessibleStateSetHelper and fills it with states of the
            current object.
            @return
                A filled AccessibleStateSetHelper.
        */
        ::utl::AccessibleStateSetHelper* implCreateStateSetHelper() override;

        // XAccessible ------------------------------------------------------------

        /** @return  The XAccessibleContext interface of this object. */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext()
            throw ( css::uno::RuntimeException, std::exception ) override;

        // XAccessibleText
        virtual sal_Int32 SAL_CALL getCaretPosition() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getSelectedText() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXTABLECELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
