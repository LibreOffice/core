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


#ifndef _SVX_ITEMTYPE_HXX
#define _SVX_ITEMTYPE_HXX

// include ---------------------------------------------------------------

#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

// forward ---------------------------------------------------------------
#include <tools/string.hxx>
#include <tools/resid.hxx>
#include <tools/bigint.hxx>
#include <svl/poolitem.hxx>
#include <tools/shl.hxx>
#include "editeng/editengdllapi.h"

class Color;
class IntlWrapper;
// static and prototypes -------------------------------------------------

static const sal_Unicode cDelim = ',';
static const sal_Unicode cpDelim[] = { ',' , ' ', '\0' };

EDITENG_DLLPUBLIC String GetSvxString( sal_uInt16 nId );
EDITENG_DLLPUBLIC String GetMetricText( long nVal, SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit, const IntlWrapper * pIntl );
String GetColorString( const Color& rCol );
EDITENG_DLLPUBLIC sal_uInt16 GetMetricId( SfxMapUnit eUnit );

// -----------------------------------------------------------------------

inline String GetBoolString( sal_Bool bVal )
{
    return String( EditResId( bVal ? RID_SVXITEMS_TRUE : RID_SVXITEMS_FALSE ) );
}

// -----------------------------------------------------------------------

inline long Scale( long nVal, long nMult, long nDiv )
{
    BigInt aVal( nVal );
    aVal *= nMult;
    aVal += nDiv/2;
    aVal /= nDiv;
    return aVal;
}

#endif

