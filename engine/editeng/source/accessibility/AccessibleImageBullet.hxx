/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <tools/gen.hxx>
#include <comphelper/OAccessible.hxx>
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
    : public cppu::ImplInheritanceHelper<comphelper::OAccessible, css::lang::XServiceInfo>
{

public:
    /// Create accessible object for given parent
    AccessibleImageBullet(css::uno::Reference<css::accessibility::XAccessible> xParent,
                          sal_Int64 nIndexInParent);

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent() override;
    virtual sal_Int64 getAccessibleIndexInParent() override;
    virtual sal_Int16 getAccessibleRole() override;
    virtual OUString getAccessibleDescription() override;
    virtual OUString getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet() override;
    virtual sal_Int64 getAccessibleStateSet() override;
    virtual css::lang::Locale getLocale() override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService (const OUString& sServiceName) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;

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

    void dispose() override;

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
