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
    typedef ::cppu::WeakImplHelper< ::com::sun::star::accessibility::XAccessible,
                                     ::com::sun::star::accessibility::XAccessibleContext,
                                     ::com::sun::star::accessibility::XAccessibleComponent,
                                     ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                                     ::com::sun::star::lang::XServiceInfo >  AccessibleImageBulletInterfaceBase;

    /** This class implements the image bullets for the EditEngine/Outliner UAA
     */
    class AccessibleImageBullet : public AccessibleImageBulletInterfaceBase
    {

    public:
        /// Create accessible object for given parent
        AccessibleImageBullet ( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rParent );

        virtual ~AccessibleImageBullet  ();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface (const ::com::sun::star::uno::Type & rType) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int16 SAL_CALL getAccessibleRole() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        /// Maximal length of text returned by getAccessibleDescription()
        enum { MaxDescriptionLen = 40 };
        virtual OUString SAL_CALL getAccessibleDescription() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale() throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XAccessibleComponent
        virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
        void FireEvent(const sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue = ::com::sun::star::uno::Any(), const ::com::sun::star::uno::Any& rOldValue = ::com::sun::star::uno::Any() ) const;

    private:
        AccessibleImageBullet( const AccessibleImageBullet& ) = delete;
        AccessibleImageBullet& operator= ( const AccessibleImageBullet& ) = delete;

        // syntactic sugar for FireEvent
        void GotPropertyEvent( const ::com::sun::star::uno::Any& rNewValue, const sal_Int16 nEventId ) const;
        void LostPropertyEvent( const ::com::sun::star::uno::Any& rOldValue, const sal_Int16 nEventId ) const;

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

        const Point& GetEEOffset() const { return maEEOffset; }

        // the paragraph index in the edit engine (guarded by solar mutex)
        sal_Int32   mnParagraphIndex;

        // our current index in the parent (guarded by solar mutex)
        sal_Int32   mnIndexInParent;

        // the current edit source (guarded by solar mutex)
        SvxEditSource* mpEditSource;

        // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
        Point maEEOffset;

        // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

        /// The shape we're the accessible for (unguarded)
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxParent;

        /// Our listeners (guarded by maMutex)
        int mnNotifierClientId;
    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
