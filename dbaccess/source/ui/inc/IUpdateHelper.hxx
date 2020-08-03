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

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>

namespace dbaui
{
    class SAL_NO_VTABLE IUpdateHelper
    {
    public:
        virtual void updateString(sal_Int32 _nPos, const OUString& _sValue) = 0;
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue) = 0;
        virtual void updateInt(sal_Int32 _nPos, sal_Int32 _nValue) = 0;
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32 sqlType) = 0;
        virtual void updateDate(sal_Int32 _nPos,const css::util::Date& _nValue) = 0;
        virtual void updateTime(sal_Int32 _nPos,const css::util::Time& _nValue) = 0;
        virtual void updateTimestamp(sal_Int32 _nPos,const css::util::DateTime& _nValue) = 0;
        virtual void insertRow() = 0;

    protected:
        ~IUpdateHelper() {}
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
