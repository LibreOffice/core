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

#include <doctok/resources.hxx>

namespace writerfilter {
namespace doctok {
sal_uInt32 WW8sprmPChgTabsPapx::get_dxaDel_count()
{
    return getS8(0x3);
}

sal_Int16  WW8sprmPChgTabsPapx::get_dxaDel(sal_uInt32 pos)
{
    return getS16(0x4 + pos * 2);
}

sal_uInt32 WW8sprmPChgTabsPapx::get_dxaAdd_count()
{
    return getS8(0x4 + get_dxaDel_count() * 2);
}

sal_Int16 WW8sprmPChgTabsPapx::get_dxaAdd(sal_uInt32 pos)
{
    return getS16(0x4 + get_dxaDel_count() * 2 + 1 + pos * 2);
}

sal_uInt32 WW8sprmPChgTabsPapx::get_tbdAdd_count()
{
    return get_dxaAdd_count();
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmPChgTabsPapx::get_tbdAdd(sal_uInt32 pos)
{
    //wntmsci compiler cannot handle 'too many inlines' ;-)
    writerfilter::Reference<Properties>::Pointer_t pRet( new WW8TBD(this,
                    0x4 + get_dxaDel_count() * 2 + 1 + get_dxaAdd_count() * 2
                    + pos, 1));
    return pRet;
}

writerfilter::Reference<Properties>::Pointer_t
WW8SED::get_sepx()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;
    return pResult;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
