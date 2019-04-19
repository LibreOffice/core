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

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <vector>


namespace pcr
{


    //= IPropertyInfoService

    class SAL_NO_VTABLE IPropertyInfoService
    {
    public:
        virtual sal_Int32                      getPropertyId(const OUString& _rName) const = 0;
        virtual OUString                       getPropertyTranslation(sal_Int32 _nId) const = 0;
        virtual OString                        getPropertyHelpId(sal_Int32 _nId) const = 0;
        virtual sal_Int16                      getPropertyPos(sal_Int32 _nId) const = 0;
        virtual sal_uInt32                     getPropertyUIFlags(sal_Int32 _nId) const = 0;
        virtual std::vector< OUString >      getPropertyEnumRepresentations(sal_Int32 _nId) const = 0;

        virtual ~IPropertyInfoService() { }
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
