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

#pragma once

#include <rtl/ref.hxx>
#include <tools/gen.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

class SvxEditSource;
class SvxTextForwarder;
class SvxViewForwarder;

namespace accessibility
{

/** This class implements the image bullets for the EditEngine/Outliner UAA
 */
class AccessibleImageBullet final
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible, css::lang::XServiceInfo>
{

public:
    /// Create accessible object for given parent
    AccessibleImageBullet ( css::uno::Reference< css::accessibility::XAccessible > xParent );

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent() override;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;
    virtual css::lang::Locale SAL_CALL getLocale() override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
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

    void SAL_CALL dispose() override;

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
    void FireEvent(const sal_Int16 nEventId, const css::uno::Any& rNewValue, const css::uno::Any& rOldValue = css::uno::Any());

private:
    AccessibleImageBullet( const AccessibleImageBullet& ) = delete;
    AccessibleImageBullet& operator= ( const AccessibleImageBullet& ) = delete;

    // maintain state set and send STATE_CHANGE events
    void SetState( const sal_Int64 nStateId );
    void UnSetState( const sal_Int64 nStateId );

    SvxEditSource& GetEditSource() const;

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

    css::awt::Rectangle implGetBounds() override;

    // the paragraph index in the edit engine (guarded by solar mutex)
    sal_Int32   mnParagraphIndex;

    // our current index in the parent (guarded by solar mutex)
    sal_Int32   mnIndexInParent;

    // the current edit source (guarded by solar mutex)
    SvxEditSource* mpEditSource;

    // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
    Point maEEOffset;

    // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
    sal_Int64 mnStateSet = 0;

    /// The shape we're the accessible for (unguarded)
    css::uno::Reference< css::accessibility::XAccessible > mxParent;
};

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
