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

#ifndef INCLUDED_EDITENG_ACCESSIBLEIMAGEBULLET_HXX
#define INCLUDED_EDITENG_ACCESSIBLEIMAGEBULLET_HXX

#include <tools/gen.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <editeng/AccessibleParaManager.hxx>
#include <editeng/unoedsrc.hxx>

namespace accessibility
{
    typedef ::cppu::WeakImplHelper< css::accessibility::XAccessible,
                                     css::accessibility::XAccessibleContext,
                                     css::accessibility::XAccessibleComponent,
                                     css::accessibility::XAccessibleEventBroadcaster,
                                     css::lang::XServiceInfo >  AccessibleImageBulletInterfaceBase;

    /** This class implements the image bullets for the EditEngine/Outliner UAA
     */
    class AccessibleImageBullet : public AccessibleImageBulletInterfaceBase
    {

    public:
        /// Create accessible object for given parent
        AccessibleImageBullet ( const css::uno::Reference< css::accessibility::XAccessible >& rParent );

        virtual ~AccessibleImageBullet  () override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent() override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
        virtual sal_Int16 SAL_CALL getAccessibleRole() override;
        virtual OUString SAL_CALL getAccessibleDescription() override;
        virtual OUString SAL_CALL getAccessibleName() override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet() override;
        virtual css::lang::Locale SAL_CALL getLocale() override;

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

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

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

        /** Set the current index in the accessibility parent

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetIndexInParent( sal_Int32 nIndex );

        /** Set the edit engine offset

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetEEOffset( const Point& rOffset );

        /** Set the EditEngine offset

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetEditSource( SvxEditSource* pEditSource );

        /** Dispose this object

            Notifies and deregisters the listeners, drops all references.
         */
        void Dispose();

        /** Set the current paragraph number

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetParagraphIndex( sal_Int32 nIndex );

        /** Query the current paragraph number (0 - nParas-1)

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Int32 GetParagraphIndex() const { return mnParagraphIndex; }

        /// Calls all Listener objects to tell them the change. Don't hold locks when calling this!
        void FireEvent(const sal_Int16 nEventId, const css::uno::Any& rNewValue, const css::uno::Any& rOldValue = css::uno::Any() ) const;

    private:
        AccessibleImageBullet( const AccessibleImageBullet& ) = delete;
        AccessibleImageBullet& operator= ( const AccessibleImageBullet& ) = delete;

        // maintain state set and send STATE_CHANGE events
        void SetState( const sal_Int16 nStateId );
        void UnSetState( const sal_Int16 nStateId );

        SvxEditSource& GetEditSource() const;

        int getNotifierClientId() const { return mnNotifierClientId; }

        /** Query the SvxTextForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxTextForwarder&   GetTextForwarder() const;

        /** Query the SvxViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxViewForwarder&   GetViewForwarder() const;

        // the paragraph index in the edit engine (guarded by solar mutex)
        sal_Int32   mnParagraphIndex;

        // our current index in the parent (guarded by solar mutex)
        sal_Int32   mnIndexInParent;

        // the current edit source (guarded by solar mutex)
        SvxEditSource* mpEditSource;

        // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
        Point maEEOffset;

        // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
        css::uno::Reference< css::accessibility::XAccessibleStateSet > mxStateSet;

        /// The shape we're the accessible for (unguarded)
        css::uno::Reference< css::accessibility::XAccessible > mxParent;

        /// Our listeners (guarded by maMutex)
        int mnNotifierClientId;
    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
