/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8PieceTableImpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:48:46 $
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

#ifndef INCLUDED_WW8_PIECE_TABLE_IMPL_HXX
#define INCLUDED_WW8_PIECE_TABLE_IMPL_HXX

#include <vector>
#ifndef INCLUDED_WW8_CP_AND_FC_HXX
#include <WW8CpAndFc.hxx>
#endif
#ifndef INCLUDE_WW8_PIECE_TABLE_HXX
#include <WW8PieceTable.hxx>
#endif

namespace writerfilter {
namespace doctok
{
using namespace ::std;

class WW8PieceTableImpl : public WW8PieceTable
{
    typedef std::vector<CpAndFc> tEntries;
    tEntries mEntries;

    tEntries::const_iterator findCp(const Cp & rCp) const;
    tEntries::const_iterator findFc(const Fc & rFc) const;

    mutable Cp2FcHashMap_t mCp2FcCache;

public:
    WW8PieceTableImpl(WW8Stream & rStream, sal_uInt32 nOffset,
                      sal_uInt32 nCount);

    virtual Fc cp2fc(const Cp & aCpIn) const;
    virtual Cp fc2cp(const Fc & aFcIn) const;
    virtual bool isComplex(const Cp & aCp) const;
    virtual bool isComplex(const Fc & aFc) const;

    virtual sal_uInt32 getCount() const;
    virtual Cp getFirstCp() const;
    virtual Fc getFirstFc() const;
    virtual Cp getLastCp() const;
    virtual Fc getLastFc() const;
    virtual Cp getCp(sal_uInt32 nIndex) const;
    virtual Fc getFc(sal_uInt32 nIndex) const;
    virtual CpAndFc createCpAndFc(const Cp & rCp, PropertyType eType) const;
    virtual CpAndFc createCpAndFc(const Fc & rFc, PropertyType eType) const;

    virtual void dump(ostream & o) const;
};
}}

#endif // INCLUDED_WW8_PIECE_TABLE_IMPL_HXX
