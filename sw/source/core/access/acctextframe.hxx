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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCTEXTFRAME_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCTEXTFRAME_HXX

#include "accframebase.hxx"

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SwFlyFrm;
namespace utl { class AccessibleRelationSetHelper; }
namespace com { namespace star {
    namespace accessibility { struct AccessibleRelation; }
} }

class SwAccessibleTextFrame : public SwAccessibleFrameBase,
        public css::accessibility::XAccessibleSelection
{
private:
    // #i73249#
    OUString msTitle;
    OUString msDesc;

protected:
    virtual ~SwAccessibleTextFrame();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwAccessibleTextFrame( SwAccessibleMap* pInitMap, const SwFlyFrm& rFlyFrm );

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;
    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception ) override;

    // XAccessibleContext

    // #i73249# - Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;
    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext::getAccessibleRelationSet

    // text frame may have accessible relations to their
    // predocesor/successor frames

private:
    // helper methods for getAccessibleRelationSet:
    SwFlyFrm* getFlyFrm() const;

    css::accessibility::AccessibleRelation makeRelation(
        sal_Int16 nType, const SwFlyFrm* pFrm );

public:
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
