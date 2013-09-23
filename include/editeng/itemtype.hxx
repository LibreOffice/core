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
#ifndef _SVX_ITEMTYPE_HXX
#define _SVX_ITEMTYPE_HXX

#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

// forward ---------------------------------------------------------------
#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <tools/bigint.hxx>
#include <svl/poolitem.hxx>
#include <tools/shl.hxx>
#include "editeng/editengdllapi.h"

class Color;
class IntlWrapper;
// static and prototypes -------------------------------------------------

static const sal_Unicode cpDelim[] = { ',' , ' ', '\0' };

EDITENG_DLLPUBLIC OUString GetSvxString( sal_uInt16 nId );
EDITENG_DLLPUBLIC OUString GetMetricText( long nVal, SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit, const IntlWrapper * pIntl );
OUString GetColorString( const Color& rCol );
EDITENG_DLLPUBLIC sal_uInt16 GetMetricId( SfxMapUnit eUnit );

// -----------------------------------------------------------------------

inline OUString GetBoolString(bool bVal)
{
    return EE_RESSTR(bVal ? RID_SVXITEMS_TRUE : RID_SVXITEMS_FALSE);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
