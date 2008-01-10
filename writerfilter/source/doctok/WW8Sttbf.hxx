/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Sttbf.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:50:46 $
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

#ifndef INCLUDED_WW8_STTBF_HXX
#define INCLUDED_WW8_STTBF_HXX

#ifndef INCLUDED_WW8_STRUCT_BASE_HXX
#include <WW8StructBase.hxx>
#endif

#include <boost/shared_ptr.hpp>
#include <vector>

namespace writerfilter {
namespace doctok
{

using namespace ::std;

/**
   A string table in file.

   The string table contains strings that each can have extra data.
 */
class WW8Sttbf : public WW8StructBase
{
    /// true if strings contain two-byte characters
    bool mbComplex;

    /// the number of entries
    sal_uInt32 mnCount;

    /// the size of the extra data (per string)
    sal_uInt32 mnExtraDataCount;

    /// offsets for the strings
    vector<sal_uInt32> mEntryOffsets;

    /// offsets for the extra data
    vector<sal_uInt32> mExtraOffsets;

    /**
       Return offset of an entry.

       @param nPos    the index of the entry
    */
    sal_uInt32 getEntryOffset(sal_uInt32 nPos) const;

    /**
       Return offset of extra data of an entry.

       @param nPos    the index of the entry
     */
    sal_uInt32 getExtraOffset(sal_uInt32 nPos) const;

public:
    typedef boost::shared_ptr<WW8Sttbf> Pointer_t;

    WW8Sttbf(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount);

    /**
       Return the number of entries.
    */
    sal_uInt32 getEntryCount() const;

    /**
       Return the string of an entry.

       @param nPos     the index of the entry
     */
    rtl::OUString getEntry(sal_uInt32 nPos) const;

    /**
       Return the extra data of an entry.

       @param nPos     the index of the entry
    */
    WW8StructBase::Pointer_t getExtraData(sal_uInt32 nPos);
};
}}
#endif // INCLUDED_WW8_STTBF
