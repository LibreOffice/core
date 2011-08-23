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

#ifndef _SVXITEMS_HRC
#include <bf_svx/svxitems.hrc>
#endif

// forward ---------------------------------------------------------------

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX //autogen
#include <bf_svx/dialmgr.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
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

