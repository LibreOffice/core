/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

// -----------------------------------------------------------------------

extern "C"
{
BOOL WINAPI WIN_Rectangle( HDC hDC, int X1, int Y1, int X2, int Y2 )
{
    return Rectangle( hDC, X1, Y1, X2, Y2 );
}
}

// -----------------------------------------------------------------------

extern "C"
{
BOOL WINAPI WIN_Polygon( HDC hDC, CONST POINT * ppt, int ncnt )
{
    return Polygon( hDC, ppt, ncnt );
}
}

// -----------------------------------------------------------------------

extern "C"
{
BOOL WINAPI WIN_PolyPolygon( HDC hDC, CONST POINT * ppt, LPINT npcnt, int ncnt )
{
    return PolyPolygon( hDC, ppt, npcnt, ncnt );
}
}
