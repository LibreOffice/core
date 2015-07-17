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
#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROLTABLECELL_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROLTABLECELL_HXX

#include <comphelper/accessibletexthelper.hxx>
#include <cppuhelper/implbase2.hxx>
#include "accessibility/extended/AccessibleGridControlBase.hxx"
#include <svtools/accessibletable.hxx>

namespace accessibility
{
    class AccessibleGridControlCell : public AccessibleGridControlBase
    {
    private:
        sal_Int32               m_nRowPos;      // the row number of the table cell
        sal_Int32               m_nColPos;      // the column id of the table cell

    protected:
        // attribute access
        inline sal_Int32    getRowPos( ) const { return m_nRowPos; }
        inline sal_Int32    getColumnPos( ) const { return m_nColPos; }

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus() throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    protected:
        AccessibleGridControlCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            ::svt::table::IAccessibleTable& _rTable,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos,
            ::svt::table::AccessibleTableControlObjType _eType
        );

        virtual ~AccessibleGridControlCell();

    private:
        AccessibleGridControlCell( const AccessibleGridControlCell& ) SAL_DELETED_FUNCTION;
        AccessibleGridControlCell& operator=( const AccessibleGridControlCell& ) SAL_DELETED_FUNCTION;
    };

    typedef ::cppu::ImplHelper2 <   css::accessibility::XAccessibleText
                                ,   css::accessibility::XAccessible
                                >   AccessibleTextHelper_BASE;
    // implementation of a table cell of GridControl
    class AccessibleGridControlTableCell    :public AccessibleGridControlCell
                                        ,public AccessibleTextHelper_BASE
                                        ,public ::comphelper::OCommonAccessibleText
    {
    protected:
        // OCommonAccessibleText
        virtual OUString                        implGetText() SAL_OVERRIDE;
        virtual css::lang::Locale               implGetLocale() SAL_OVERRIDE;
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) SAL_OVERRIDE;
        virtual Rectangle implGetBoundingBox() SAL_OVERRIDE;
        virtual Rectangle implGetBoundingBoxOnScreen() SAL_OVERRIDE;

    public:
        AccessibleGridControlTableCell( const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                                    ::svt::table::IAccessibleTable& _rTable,
                                    sal_Int32 _nRowId,
                                    sal_uInt16 _nColId,
                                    svt::table::AccessibleTableControlObjType  eObjType);

        // XInterface -------------------------------------------------------------

        /** Queries for a new interface. */
        css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
            throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /** Aquires the object (calls acquire() on base class). */
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;

        /** Releases the object (calls release() on base class). */
        virtual void SAL_CALL release() throw () SAL_OVERRIDE;

        /** @return  The index of this object among the parent's children. */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
            throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /** @return
                The name of this class.
        */
        virtual OUString SAL_CALL getImplementationName()
            throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /** @return
                The count of visible children.
        */
        virtual sal_Int32 SAL_CALL getAccessibleChildCount()
            throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /** @return
                The XAccessible interface of the specified child.
        */
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
            getAccessibleChild( sal_Int32 nChildIndex )
                throw ( css::lang::IndexOutOfBoundsException,
                        css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        /** Creates a new AccessibleStateSetHelper and fills it with states of the
            current object.
            @return
                A filled AccessibleStateSetHelper.
        */
        ::utl::AccessibleStateSetHelper* implCreateStateSetHelper() SAL_OVERRIDE;

        // XAccessible ------------------------------------------------------------

        /** @return  The XAccessibleContext interface of this object. */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext()
            throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XAccessibleText
        virtual sal_Int32 SAL_CALL getCaretPosition() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getSelectedText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };
}
#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROLTABLECELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
