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
#ifndef INCLUDED_COM_SUN_STAR_UNO_GENFUNC_HXX
#define INCLUDED_COM_SUN_STAR_UNO_GENFUNC_HXX

#include <sal/config.h>

#include <cstddef>

#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/XInterface.hpp>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{


inline void SAL_CALL cpp_acquire( void * pCppI )
{
    static_cast< XInterface * >( pCppI )->acquire();
}

inline void SAL_CALL cpp_release( void * pCppI )
{
    static_cast< XInterface * >( pCppI )->release();
}

inline void * SAL_CALL cpp_queryInterface( void * pCppI, typelib_TypeDescriptionReference * pType )
{
    if (pCppI)
    {
#ifndef __OBJC__
        try
#else
        @try
#endif
        {
            Any aRet( static_cast< XInterface * >( pCppI )->queryInterface(
                * reinterpret_cast< const Type * >( &pType ) ) );
            if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
            {
                XInterface * pRet = static_cast< XInterface * >( aRet.pReserved );
                aRet.pReserved = NULL;
                return pRet;
            }
        }
#ifndef __OBJC__
        catch (RuntimeException &)
#else
        @catch (...)
#endif
        {
        }
    }
    return NULL;
}

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
