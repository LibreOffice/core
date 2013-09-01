/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_SPELLCHECKCONTEXT_HXX
#define SC_SPELLCHECKCONTEXT_HXX

#include "address.hxx"
#include "editeng/misspellrange.hxx"

#include <boost/unordered_map.hpp>

namespace sc {

struct SpellCheckContext
{
    struct CellPos
    {
        struct Hash
        {
            size_t operator() (const CellPos& rPos) const;
        };

        SCCOL mnCol;
        SCROW mnRow;

        CellPos();
        CellPos(SCCOL nCol, SCROW nRow);

        void setInvalid();
        bool isValid() const;
        void reset();

        bool operator< (const CellPos& r) const;
        bool operator== (const CellPos& r) const;
    };

    typedef boost::unordered_map<CellPos, std::vector<editeng::MisspellRanges>, CellPos::Hash> CellMapType;

    CellPos maPos;
    CellMapType maMisspellCells;

    SpellCheckContext();

    bool isMisspelled( SCCOL nCol, SCROW nRow ) const;
    const std::vector<editeng::MisspellRanges>* getMisspellRanges( SCCOL nCol, SCROW nRow ) const;
    void setMisspellRanges( SCCOL nCol, SCROW nRow, const std::vector<editeng::MisspellRanges>* pRanges );

    void reset();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
