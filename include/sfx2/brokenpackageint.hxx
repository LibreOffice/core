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

#ifndef INCLUDED_SFX2_BROKENPACKAGEINT_HXX
#define INCLUDED_SFX2_BROKENPACKAGEINT_HXX

#include <sal/config.h>
#include <rtl/ref.hxx>
#include <sfx2/dllapi.h>

#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::task
{
class XInteractionRequest;
}

class RequestPackageReparation_Impl;
class SFX2_DLLPUBLIC RequestPackageReparation
{
    rtl::Reference<RequestPackageReparation_Impl> mxImpl;

public:
    RequestPackageReparation(const OUString& aName);
    ~RequestPackageReparation();
    RequestPackageReparation(const RequestPackageReparation&) = delete;
    RequestPackageReparation& operator=(const RequestPackageReparation&) = delete;
    bool isApproved() const;
    css::uno::Reference<css::task::XInteractionRequest> GetRequest();
};

class NotifyBrokenPackage_Impl;
class SFX2_DLLPUBLIC NotifyBrokenPackage
{
    rtl::Reference<NotifyBrokenPackage_Impl> mxImpl;

public:
    NotifyBrokenPackage(const OUString& aName);
    ~NotifyBrokenPackage();
    NotifyBrokenPackage(const NotifyBrokenPackage&) = delete;
    NotifyBrokenPackage& operator=(const NotifyBrokenPackage&) = delete;
    css::uno::Reference<css::task::XInteractionRequest> GetRequest();
};

#endif // INCLUDED_SFX2_BROKENPACKAGEINT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
