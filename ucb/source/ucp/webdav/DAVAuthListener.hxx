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


#ifndef _DAVAUTHLISTENER_HXX_
#define _DAVAUTHLISTENER_HXX_

#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/XReference.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace http_dav_ucp
{

class DAVAuthListener : public salhelper::SimpleReferenceObject
{
    public:
        virtual int authenticate(
            const ::rtl::OUString & inRealm,
            const ::rtl::OUString & inHostName,
            ::rtl::OUString & inoutUserName,
            ::rtl::OUString & outPassWord,
            sal_Bool bCanUseSystemCredentials,
            sal_Bool bUsePreviousCredentials = sal_True ) = 0;
};

} // namespace http_dav_ucp

#endif // _DAVAUTHLISTENER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
