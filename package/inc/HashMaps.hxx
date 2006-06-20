/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HashMaps.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:09:46 $
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
#ifndef _HASHMAPS_HXX
#define _HASHMAPS_HXX

#ifndef _ZIP_ENTRY_HXX_
#include <ZipEntry.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#include <hash_map>

struct eqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};

class ZipPackageFolder;
namespace com { namespace sun { namespace star { namespace packages {
class ContentInfo;
} } } }

typedef std::hash_map < rtl::OUString,
                        ZipPackageFolder *,
                        ::rtl::OUStringHash,
                        eqFunc > FolderHash;

typedef std::hash_map < rtl::OUString,
                        vos::ORef < com::sun::star::packages::ContentInfo >,
                        ::rtl::OUStringHash,
                        eqFunc > ContentHash;

typedef std::hash_map < rtl::OUString,
                        ZipEntry,
                        rtl::OUStringHash,
                        eqFunc > EntryHash;

#endif
