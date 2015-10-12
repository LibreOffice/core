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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCHEADERFOOTER_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCHEADERFOOTER_HXX

#include "acccontext.hxx"

class SwHeaderFrm;
class SwFooterFrm;

class SwAccessibleHeaderFooter : public SwAccessibleContext
{
protected:
    virtual ~SwAccessibleHeaderFooter();

public:
    SwAccessibleHeaderFooter( SwAccessibleMap* pInitMap,
                              const SwHeaderFrm* pHdFrm );
    SwAccessibleHeaderFooter( SwAccessibleMap* pInitMap,
                              const SwFooterFrm* pFtFrm );

    // XAccessibleContext

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object. */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service. */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    // XAccessibleComponent
    sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
