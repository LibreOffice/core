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

#include "TextCursorHelper.hxx"
#include "unobaseclass.hxx"
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;

namespace
{
    class theOTextCursorHelperUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theOTextCursorHelperUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & OTextCursorHelper::getUnoTunnelId()
{
    return theOTextCursorHelperUnoTunnelId::get().getSeq();
}

//XUnoTunnel
sal_Int64 SAL_CALL OTextCursorHelper::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
        throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
        {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
        }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
