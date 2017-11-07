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

#ifndef INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLEICONCHOICECTRLENTRY_HXX
#define INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLEICONCHOICECTRLENTRY_HXX

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
#include <cppuhelper/compbase8.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

// forward

class SvxIconChoiceCtrlEntry;
class SvtIconChoiceCtrl;

namespace accessibility
{

// class AccessibleIconChoiceCtrlEntry

    typedef ::cppu::WeakAggComponentImplHelper8< css::accessibility::XAccessible
                                                , css::accessibility::XAccessibleContext
                                                , css::accessibility::XAccessibleComponent
                                                , css::accessibility::XAccessibleEventBroadcaster
                                                , css::accessibility::XAccessibleText
                                                , css::accessibility::XAccessibleAction
                                                , css::lang::XServiceInfo
                                                , css::lang::XEventListener > AccessibleIconChoiceCtrlEntry_BASE;

    /** the class AccessibleListBoxEntry represents the class for an accessible object of a listbox entry */
    class AccessibleIconChoiceCtrlEntry final : public ::cppu::BaseMutex,
                                                public AccessibleIconChoiceCtrlEntry_BASE,
                                                public ::comphelper::OCommonAccessibleText
    {
        /** The treelistbox control */
        VclPtr<SvtIconChoiceCtrl>           m_pIconCtrl;
        sal_Int32                           m_nIndex;

        /// client id in the AccessibleEventNotifier queue
        sal_uInt32                          m_nClientId;

        css::uno::Reference< css::accessibility::XAccessible > m_xParent;

        tools::Rectangle               GetBoundingBox_Impl() const;
        tools::Rectangle               GetBoundingBoxOnScreen_Impl() const;
        bool                IsAlive_Impl() const;
        bool                IsShowing_Impl() const;

        /// @throws css::lang::DisposedException
        /// @throws css::uno::RuntimeException
        tools::Rectangle               GetBoundingBox();
        /// @throws css::lang::DisposedException
        /// @throws css::uno::RuntimeException
        tools::Rectangle               GetBoundingBoxOnScreen();
        /// @throws css::lang::DisposedException
        void                    EnsureIsAlive() const;

        virtual ~AccessibleIconChoiceCtrlEntry() override;
        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL                   disposing() override;

        // OCommonAccessibleText
        virtual OUString                        implGetText() override;
        virtual css::lang::Locale               implGetLocale() override;
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    public:
        /** Ctor()
            @param  _rIconCtrl
                the icon control
            @param  _nPos
                the entry number
            @param  _xParent
                is our parent accessible object
        */
        AccessibleIconChoiceCtrlEntry( SvtIconChoiceCtrl& _rIconCtrl,
                                       sal_Int32 _nPos,
                                       const css::uno::Reference< css::accessibility::XAccessible >& _xParent );

        // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
        virtual css::lang::Locale SAL_CALL getLocale(  ) override;

        // XAccessibleComponent
        virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
        virtual css::awt::Point SAL_CALL getLocation(  ) override;
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
        virtual css::awt::Size SAL_CALL getSize(  ) override;
        virtual void SAL_CALL grabFocus(  ) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) override;

        // XAccessibleText
        virtual sal_Int32 SAL_CALL getCaretPosition() override;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
        virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
        virtual sal_Int32 SAL_CALL getCharacterCount() override;
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
        virtual OUString SAL_CALL getSelectedText() override;
        virtual sal_Int32 SAL_CALL getSelectionStart() override;
        virtual sal_Int32 SAL_CALL getSelectionEnd() override;
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
        virtual OUString SAL_CALL getText() override;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

        // XAccessibleAction
        virtual sal_Int32 SAL_CALL getAccessibleActionCount(  ) override;
        virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex ) override;
        virtual OUString SAL_CALL getAccessibleActionDescription( sal_Int32 nIndex ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;
    };


}// namespace accessibility


#endif // INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLEICONCHOICECTRLENTRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
