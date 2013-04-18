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

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>

class RequestPackageReparation_Impl;
class SFX2_DLLPUBLIC RequestPackageReparation
{
    RequestPackageReparation_Impl* pImp;
public:
    RequestPackageReparation( OUString aName );
    ~RequestPackageReparation();
    sal_Bool    isApproved();
    com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest > GetRequest();
};

class NotifyBrokenPackage_Impl;
class SFX2_DLLPUBLIC NotifyBrokenPackage
{
    NotifyBrokenPackage_Impl* pImp;
public:
    NotifyBrokenPackage( OUString aName );
    ~NotifyBrokenPackage();
    sal_Bool    isAborted();
    com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest > GetRequest();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
