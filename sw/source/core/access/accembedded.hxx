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

#include "accnotextframe.hxx"

#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>

class SwAccessibleEmbeddedObject : public   SwAccessibleNoTextFrame
            , public css::accessibility::XAccessibleExtendedAttributes

{
protected:
    virtual ~SwAccessibleEmbeddedObject() override;

public:
    SwAccessibleEmbeddedObject(std::shared_ptr<SwAccessibleMap> const& pInitMap,
                                const SwFlyFrame* pFlyFrame );

    // XInterface

    virtual css::uno::Any SAL_CALL
        queryInterface (const css::uno::Type & rType) override;

    virtual void SAL_CALL
        acquire()
        noexcept override;

    virtual void SAL_CALL
        release()
        noexcept override;

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName() override;

    // Return whether the specified service is supported by this class.
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName) override;

    // Returns a list of all supported services.  In this case that is just
    // the AccessibleContext service.
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XAccessibleExtendedAttributes
        virtual css::uno::Any SAL_CALL getExtendedAttributes() override ;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
