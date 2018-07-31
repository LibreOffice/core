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

#ifndef INCLUDED_SAL_RTL_ALLOC_CACHE_HXX
#define INCLUDED_SAL_RTL_ALLOC_CACHE_HXX

#include <sal/types.h>
#include <rtl/alloc.h>

struct rtl_cache_st
{
    /* properties */
    char                      m_name[RTL_CACHE_NAME_LENGTH + 1];

    sal_Size                  m_type_size;   /* const */

    int  (SAL_CALL * m_constructor)(void * obj, void * userarg); /* const */
    void (SAL_CALL * m_destructor) (void * obj, void * userarg); /* const */
    void *                    m_userarg;
};

#endif // INCLUDED_SAL_RTL_ALLOC_CACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
