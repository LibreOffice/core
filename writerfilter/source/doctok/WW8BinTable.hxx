/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8BinTable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:44:36 $
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

#ifndef INCLUDED_WW8_BIN_TABLE_HXX
#define INCLUDED_WW8_BIN_TABLE_HXX

#include <boost/shared_ptr.hpp>
#include <string>

#ifndef INCLUDED_WW8_FKP_HXX
#include <WW8FKP.hxx>
#endif

namespace writerfilter {
namespace doctok
{

/**
   A bintable.

   Word uses bintables to associate FC ranges with FKPs. A bintable
   has a list of FCs. At each FC a range begins. The length of the
   range is given by the distance of the according CPs.
 */
class WW8BinTable
{
public:
    virtual ~WW8BinTable() {};
    /**
       Shared pointer to a bintable.
     */
    typedef boost::shared_ptr<WW8BinTable> Pointer_t;

    /**
       Return count of entries.
     */
    virtual sal_uInt32 getEntryCount() const = 0;

    /**
       Return FC from bintable.

       @param nIndex    index in bintable to return FC from
     */
    virtual Fc getFc(sal_uInt32 nIndex) const = 0;

    /**
       Return page number.

       @param nIndex    index in bintable to return page number from
     */
    virtual sal_uInt32 getPageNumber(sal_uInt32 nIndex) const = 0;

    /**
       Return page number associated with FC.

       @param rFc      FC to return page number for
     */
    virtual sal_uInt32 getPageNumber(const Fc & rFc) const = 0;

    /**
       Return string representation of bintable.
     */
    virtual string toString() const = 0;
};
}}

#endif // INCLUDED_WW8_BIN_TABLE_HXX
