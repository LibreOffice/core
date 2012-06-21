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

#ifndef INCLUDED_WW8_PIECE_TABLE_HXX
#define INCLUDED_WW8_PIECE_TABLE_HXX

#include <sal/types.h>
#include <doctok/WW8Document.hxx>

#include <boost/shared_ptr.hpp>
#include <iostream>

namespace writerfilter {
namespace doctok {

using namespace ::std;

struct Cp;
struct Fc;
class CpAndFc;

/**
   The piece table of a Word document.

   The piece table associates character positions (CP) and File
   character positions (FC). In a FC based view the piece table
   defines intervals of FCs that contain consecutive text.

   CPs and FCs can be complex, i.e. the associated text is stored in
   bytes. Otherwise the text encoding is UTF-16.
 */
class WW8PieceTable
{
public:
    virtual ~WW8PieceTable() {}
    /**
       Shared pointer to piece table
     */
    typedef boost::shared_ptr<WW8PieceTable> Pointer_t;

    /**
       Convert CP to FC.

       @param aCpIn    CP to convert

       @return FC associated with CP
     */
    virtual Fc cp2fc(const Cp & aCpIn) const = 0;

    /**
       Convert FC to CP.

       @param aFcIn    FC to convert

       @return CP associated with FC
     */
    virtual Cp fc2cp(const Fc & aFcIn) const = 0;

    /**
       Check if CP is complex.

       @param  aCp    CP to check
       @retval true   CP is complex
       @retval false  else
     */
    virtual bool isComplex(const Cp & aCp) const = 0;

    /**
       Check if FC is complex.

       @param  aFc    FC to check
       @retval true   FC is complex
       @retval false  else
     */
    virtual bool isComplex(const Fc & aFc) const = 0;

    /**
       Return number of entries.
     */
    virtual sal_uInt32 getCount() const = 0;

    /**
       Return first CP.
     */
    virtual Cp getFirstCp() const = 0;

    /**
       Return first FC.
    */
    virtual Fc getFirstFc() const = 0;

    /**
       Return last CP.
    */
    virtual Cp getLastCp() const = 0;

    /**
        Return last FC.
    */
    virtual Fc getLastFc() const = 0;

    /**
       Return CP at index.

       @param  nIndex    index of CP to return
     */
    virtual Cp getCp(sal_uInt32 nIndex) const = 0;

    /**
       Return FC at index.

       @param nIndex     index of FC to return
    */
    virtual Fc getFc(sal_uInt32 nIndex) const = 0;

    /**
        Create CpAndFc from Cp.

        @param rCp   the Cp

        @return CpAndFc containing rCp and corresponding Fc
    */
    virtual CpAndFc createCpAndFc(const Cp & rCp, PropertyType eType) const = 0;

    /**
        Create CpAndFc from Fc.

        @param rFc   the Fc

        @return CpAndFc containing rFc and corresponding Cp
    */
    virtual CpAndFc createCpAndFc(const Fc & rFc, PropertyType eType) const = 0;

    /**
       Dump piece table.

       @param o        stream to dump to
    */
    virtual void dump(ostream & o) const = 0;
};

/**
   Dump piece table.

   @param o             stream to dump to
   @param rPieceTable   piece table to dump
*/
ostream & operator << (ostream & o, const WW8PieceTable & rPieceTable);
}}

#endif // INCLUDED_WW8_PIECE_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
