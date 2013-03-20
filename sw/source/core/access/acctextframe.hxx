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

#ifndef _ACCTEXTFRAME_HXX
#define _ACCTEXTFRAME_HXX

#include "accframebase.hxx"

class SwFlyFrm;
namespace utl { class AccessibleRelationSetHelper; }
namespace com { namespace star {
    namespace accessibility { struct AccessibleRelation; }
} }

class SwAccessibleTextFrame : public SwAccessibleFrameBase
{
private:
    // #i73249#
    OUString msTitle;
    OUString msDesc;

protected:
    virtual ~SwAccessibleTextFrame();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwAccessibleTextFrame( SwAccessibleMap* pInitMap, const SwFlyFrm* pFlyFrm );

    //=====  XAccessibleContext  ==============================================

    // #i73249# - Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);
    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //====== XTypeProvider ====================================================
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext::getAccessibleRelationSet  ====================

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
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
