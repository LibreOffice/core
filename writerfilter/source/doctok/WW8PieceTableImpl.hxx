/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
