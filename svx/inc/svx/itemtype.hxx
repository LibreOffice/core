/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemtype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:58:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_ITEMTYPE_HXX
#define _SVX_ITEMTYPE_HXX

// include ---------------------------------------------------------------

#ifndef _SVXITEMS_HRC
#include <svx/svxitems.hrc>
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
#include <svx/dialmgr.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class Color;
class XColorTable;
class IntlWrapper;
// static and prototypes -------------------------------------------------

static const sal_Unicode cDelim = ',';
static const sal_Unicode cpDelim[] = { ',' , ' ', '\0' };

SVX_DLLPUBLIC String GetSvxString( USHORT nId );
SVX_DLLPUBLIC String GetMetricText( long nVal, SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit, const IntlWrapper * pIntl );
#ifndef SVX_LIGHT
String GetColorString( const Color& rCol );
#endif
SVX_DLLPUBLIC USHORT GetMetricId( SfxMapUnit eUnit );

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

#endif

