/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resary.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:16:34 $
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
#ifndef _TOOLS_RESARY_HXX
#define _TOOLS_RESARY_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif

// ---------------------
// - ImplResStringItem -
// ---------------------

struct ImplResStringItem
{
    XubString   maStr;
    long        mnValue;

//#if 0 // _SOLAR__PRIVATE
#ifdef _TOOLS_RESARY_CXX
                ImplResStringItem( const XubString& rStr ) :
                    maStr( rStr ) {}
#endif
//#endif
};

// ------------------
// - ResStringArray -
// ------------------

#define RESARRAY_INDEX_NOTFOUND             ((USHORT)0xFFFF)

class TOOLS_DLLPUBLIC ResStringArray : public Resource
{
private:
    ImplResStringItem** mpAry;
    sal_uInt32          mnSize;

public:
                        ResStringArray( const ResId& rResId );
                        ~ResStringArray();

    const XubString&    GetString( sal_uInt32 nIndex ) const
                            { return mpAry[nIndex]->maStr; }
    long                GetValue( sal_uInt32 nIndex ) const
                            { return mpAry[nIndex]->mnValue; }
    sal_uInt32          Count() const { return mnSize; }

    sal_uInt32          FindIndex( long nValue ) const;

private:
                        ResStringArray( const ResStringArray& );
    ResStringArray&     operator=( const ResStringArray& );
};

#endif  // _TOOLS_RESARY_HXX
