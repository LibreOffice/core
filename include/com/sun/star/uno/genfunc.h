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
#ifndef INCLUDED_COM_SUN_STAR_UNO_GENFUNC_H
#define INCLUDED_COM_SUN_STAR_UNO_GENFUNC_H

#include "sal/types.h"

typedef struct _typelib_TypeDescriptionReference typelib_TypeDescriptionReference;

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** Function to acquire a C++ interface.

    @param pCppI C++ interface pointer
*/
inline void SAL_CALL cpp_acquire( void * pCppI );
/** Function to release a C++ interface.

    @param pCppI C++ interface pointer
*/
inline void SAL_CALL cpp_release( void * pCppI );
/** Function to query for a C++ interface.

    @param pCppI C++ interface pointer
    @param pType demanded interface type
    @return acquired C++ interface pointer or null
*/
inline void * SAL_CALL cpp_queryInterface( void * pCppI, typelib_TypeDescriptionReference * pType );

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
