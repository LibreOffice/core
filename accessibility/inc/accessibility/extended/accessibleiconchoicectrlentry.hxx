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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEICONCHOICECTRLENTRY_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEICONCHOICECTRLENTRY_HXX

#include <deque>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

// forward

class SvxIconChoiceCtrlEntry;
class SvtIconChoiceCtrl;


namespace accessibility
{

// class AccessibleIconChoiceCtrlEntry

    typedef ::cppu::WeakAggComponentImplHelper <  css::accessibility::XAccessible
                                                , css::accessibility::XAccessibleContext
                                                , css::accessibility::XAccessibleComponent
                                                , css::accessibility::XAccessibleEventBroadcaster
                                                , css::accessibility::XAccessibleText
                                                , css::accessibility::XAccessibleAction
                                                , css::lang::XServiceInfo
                                                , css::lang::XEventListener > AccessibleIconChoiceCtrlEntry_BASE;

    /** the class AccessibleListBoxEntry represents the class for an accessible object of a listbox entry */
    class AccessibleIconChoiceCtrlEntry :   public ::comphelper::OBaseMutex,
                                       public AccessibleIconChoiceCtrlEntry_BASE,
                                    public ::comphelper::OCommonAccessibleText
    {
    private:
        /** The treelistbox control */
        VclPtr<SvtIconChoiceCtrl>           m_pIconCtrl;
        sal_Int32                           m_nIndex;

    protected:
        /// client id in the AccessibleEventNotifier queue
        sal_uInt32                          m_nClientId;

        css::uno::Reference< css::accessibility::XAccessible > m_xParent;

    private:
        Rectangle               GetBoundingBox_Impl() const;
        Rectangle               GetBoundingBoxOnScreen_Impl() const;
        bool                IsAlive_Impl() const;
        bool                IsShowing_Impl() const;

        Rectangle               GetBoundingBox()
            throw (css::lang::DisposedException,
                   css::uno::RuntimeException);
        Rectangle               GetBoundingBoxOnScreen()
            throw (css::lang::DisposedException,
                   css::uno::RuntimeException);
        void                    EnsureIsAlive() const throw ( css::lang::DisposedException );

    protected:
        virtual ~AccessibleIconChoiceCtrlEntry();
        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL                   disposing() override;

        // OCommonAccessibleText
        virtual OUString                        implGetText() override;
        virtual css::lang::Locale               implGetLocale() override;
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    public:
        /** Ctor()
            @param  _rListBox
                the view control
            @param  _pEntry
                the entry
            @param  _xParent
                is our parent accessible object
        */
        AccessibleIconChoiceCtrlEntry( SvtIconChoiceCtrl& _rIconCtrl,
                                       sal_Int32 _nPos,
                                       const css::uno::Reference< css::accessibility::XAccessible >& _xParent );

        // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw(css::uno::RuntimeException);
        static OUString getImplementationName_Static() throw(css::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

        // XAccessibleComponent
        virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

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

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleAction
        virtual sal_Int32 SAL_CALL getAccessibleActionCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleActionDescription( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    };


}// namespace accessibility


#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEICONCHOICECTRLENTRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
