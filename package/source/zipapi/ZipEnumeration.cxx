/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ZipEnumeration.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:28:02 $
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
#include "precompiled_package.hxx"
#ifndef _ZIP_ENUMERATION_HXX
#include <ZipEnumeration.hxx>
#endif

/** Provides an Enumeration over the contents of a Zip file */

ZipEnumeration::ZipEnumeration( EntryHash & rNewEntryHash)
: rEntryHash(rNewEntryHash)
, aIterator(rEntryHash.begin())
{
}
ZipEnumeration::~ZipEnumeration( void )
{
}
sal_Bool SAL_CALL ZipEnumeration::hasMoreElements()
{
    return (aIterator != rEntryHash.end());
}

const ZipEntry* SAL_CALL ZipEnumeration::nextElement()
{
    if (aIterator != rEntryHash.end())
        return &((*aIterator++).second);
    else
        return NULL;
}
