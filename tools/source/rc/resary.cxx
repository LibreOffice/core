/*************************************************************************
 *
 *  $RCSfile: resary.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2002-05-22 14:44:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _TOOLS_RESARY_CXX

#ifndef _TOOLS_RESARY_HXX
#include <resary.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <rcid.h>
#endif

// =======================================================================

ResStringArray::ResStringArray( const ResId& rResId )
    : Resource ( rResId.SetRT( RSC_STRINGARRAY ) ),
      mpAry    ( 0 ),
      mnSize   ( ReadShortRes() )
{
    if ( mnSize )
    {
        mpAry = new ImplResStringItem*[mnSize];
        for ( USHORT i = 0; i < mnSize; i++ )
        {
            // String laden
            mpAry[i] = new ImplResStringItem( ReadStringRes() );

            // Value laden
            mpAry[i]->mnValue = ReadLongRes();
        }
    }
}

// -----------------------------------------------------------------------

ResStringArray::~ResStringArray()
{
    for ( USHORT i = 0; i < mnSize; i++ )
        delete mpAry[i];
    delete[] mpAry;
}

// -----------------------------------------------------------------------

USHORT ResStringArray::FindIndex( long nValue ) const
{
    for ( USHORT i = 0; i < mnSize; i++ )
    {
        if ( mpAry[i]->mnValue == nValue )
            return i;
    }
    return RESARRAY_INDEX_NOTFOUND;
}
