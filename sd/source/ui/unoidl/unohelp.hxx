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

namespace sd
{
inline sal_Bool any2bool( const ::com::sun::star::uno::Any& rAny, sal_Bool& rBool )
{
    if( rAny.getValueType() == ::getCppuBooleanType() )
    {
        rBool = *(sal_Bool*)rAny.getValue();
    }
    else
    {
        sal_Int32 nValue = 0;
        if(!(rAny >>= nValue))
            return sal_False;
        rBool = nValue != 0;
    }

    return sal_True;
}

inline void bool2any( sal_Bool bBool, ::com::sun::star::uno::Any& rAny )
{
    rAny.setValue( &bBool, ::getCppuBooleanType() );
}

}

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
