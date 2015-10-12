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
        public ::com::sun::star::accessibility::XAccessibleSelection
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

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;
    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XAccessibleContext

    // #i73249# - Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleContext::getAccessibleRelationSet

    // text frame may have accessible relations to their
    // predocesor/successor frames

private:
    // helper methods for getAccessibleRelationSet:
    SwFlyFrm* getFlyFrm() const;

    com::sun::star::accessibility::AccessibleRelation makeRelation(
        sal_Int16 nType, const SwFlyFrm* pFrm );

public:
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
