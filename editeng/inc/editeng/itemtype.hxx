/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_ITEMTYPE_HXX
#define _SVX_ITEMTYPE_HXX

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
