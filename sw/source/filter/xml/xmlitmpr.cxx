/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlitmpr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:41:44 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif

using rtl::OUString;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_TOKEN_INVALID;

// TODO: optimize this!

class SvXMLItemMapEntries_impl
{
public:
    SvXMLItemMapEntry* mpEntries;
    sal_uInt16 mnCount;
};

SvXMLItemMapEntries::SvXMLItemMapEntries( SvXMLItemMapEntry* pEntries )
{
    mpImpl = new SvXMLItemMapEntries_impl;
    mpImpl->mpEntries = pEntries;

    mpImpl->mnCount = 0;
    while( pEntries->eLocalName != XML_TOKEN_INVALID )
    {
        pEntries++;
        mpImpl->mnCount++;
    }
}

SvXMLItemMapEntries::~SvXMLItemMapEntries()
{
    delete mpImpl;
}

SvXMLItemMapEntry* SvXMLItemMapEntries::getByName(  sal_uInt16 nNameSpace,
                                                    const OUString& rString,
                                                    SvXMLItemMapEntry* pStartAt /* = NULL */  ) const
{
    SvXMLItemMapEntry* pMap =
        (pStartAt && (pStartAt->eLocalName!=XML_TOKEN_INVALID)) ?
                                           &(pStartAt[1]) : mpImpl->mpEntries;
    while( pMap && (pMap->eLocalName != XML_TOKEN_INVALID) )
    {
        if( pMap->nNameSpace == nNameSpace &&
            IsXMLToken( rString, pMap->eLocalName ) )
            break;
        pMap++;
    }

    return (pMap->eLocalName != XML_TOKEN_INVALID) ? pMap : NULL;
}

SvXMLItemMapEntry* SvXMLItemMapEntries::getByIndex( UINT16 nIndex ) const
{
    return &mpImpl->mpEntries[nIndex];
}

UINT16 SvXMLItemMapEntries::getCount() const
{
    return mpImpl->mnCount;
}

