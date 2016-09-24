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

#ifndef INCLUDED_SVL_UNX_INC_CONVERT_HXX
#define INCLUDED_SVL_UNX_INC_CONVERT_HXX

class Convert
{
public:
    static void     Swap( long & nValue )
                    { nValue = OSL_SWAPDWORD( nValue ); }
    static void     Swap( ULONG & nValue )
                    { nValue = OSL_SWAPDWORD( nValue ); }
    static void     Swap( short & nValue )
                    { nValue = OSL_SWAPWORD( nValue ); }
    static void     Swap( USHORT & nValue )
                    { nValue = OSL_SWAPWORD( nValue ); }
    static void     Swap( Point & aPtr )
                    { Swap( aPtr.X() ); Swap( aPtr.Y() ); }
    static void     Swap( Size & aSize )
                    { Swap( aSize.Width() ); Swap( aSize.Height() ); }
    static void     Swap( Rectangle & rRect )
                    { Swap( rRect.Top() ); Swap( rRect.Bottom() );
                      Swap( rRect.Left() ); Swap( rRect.Right() ); }
};

#endif // INCLUDED_SVL_UNX_INC_CONVERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
