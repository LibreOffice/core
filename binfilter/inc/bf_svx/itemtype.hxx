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

// include ---------------------------------------------------------------

#include <bf_svx/svxitems.hrc>

// forward ---------------------------------------------------------------

#include <tools/string.hxx>
#include <tools/resid.hxx>
#include <tools/bigint.hxx>
#include <bf_svx/dialmgr.hxx>
#include <bf_svtools/poolitem.hxx>
#include <tools/shl.hxx>
class Color;
class IntlWrapper;
namespace binfilter {

class XColorTable;
// static and prototypes -------------------------------------------------

static const sal_Unicode cDelim = ',';
static const sal_Unicode cpDelim[] = { ',' , ' ', '\0' };

String GetSvxString( USHORT nId );
String GetMetricText( long nVal, SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit, const ::IntlWrapper * pIntl );
#ifndef SVX_LIGHT
String GetColorString( const Color& rCol );
#endif
USHORT GetMetricId( SfxMapUnit eUnit );

// -----------------------------------------------------------------------

inline String GetBoolString( BOOL bVal )
{
    return String( ResId( bVal ? RID_SVXITEMS_TRUE
                               : RID_SVXITEMS_FALSE, DIALOG_MGR() ) );
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

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
