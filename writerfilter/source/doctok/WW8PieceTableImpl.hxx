/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_WW8_PIECE_TABLE_IMPL_HXX
#define INCLUDED_WW8_PIECE_TABLE_IMPL_HXX

#include <vector>
#include <WW8CpAndFc.hxx>
#include <WW8PieceTable.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
