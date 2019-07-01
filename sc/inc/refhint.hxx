/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_REFHINT_HXX
#define INCLUDED_SC_INC_REFHINT_HXX

#include "types.hxx"
#include <svl/hint.hxx>

namespace sc {

struct RefUpdateContext;
class ColRowReorderMapType;

class RefHint : public SfxHint
{
public:
    enum Type {
        ColumnReordered,
        RowReordered,
        StartListening,
        StopListening
    };

private:
    Type const meType;

protected:
    RefHint( Type eType );

public:
    RefHint() = delete;
    virtual ~RefHint() override = 0;

    RefHint(RefHint const &) = default;
    RefHint(RefHint &&) = default;
    RefHint & operator =(RefHint const &) = delete;
    RefHint & operator =(RefHint &&) = delete;

    Type getType() const;
};

class RefColReorderHint : public RefHint
{
    const sc::ColRowReorderMapType& mrColMap;
    SCTAB const mnTab;
    SCROW const mnRow1;
    SCROW const mnRow2;

public:
    RefColReorderHint( const sc::ColRowReorderMapType& rColMap, SCTAB nTab, SCROW nRow1, SCROW nRow2 );
    virtual ~RefColReorderHint() override;

    RefColReorderHint(RefColReorderHint const &) = default;
    RefColReorderHint(RefColReorderHint &&) = default;
    RefColReorderHint & operator =(RefColReorderHint const &) = delete; // due to mrColMap
    RefColReorderHint & operator =(RefColReorderHint &&) = delete; // due to mrColMap

    const sc::ColRowReorderMapType& getColMap() const;

    SCTAB getTab() const;
    SCROW getStartRow() const;
    SCROW getEndRow() const;
};

class RefRowReorderHint : public RefHint
{
    const sc::ColRowReorderMapType& mrRowMap;
    SCTAB const mnTab;
    SCCOL const mnCol1;
    SCCOL const mnCol2;

public:
    RefRowReorderHint( const sc::ColRowReorderMapType& rRowMap, SCTAB nTab, SCCOL nCol1, SCCOL nCol2 );
    virtual ~RefRowReorderHint() override;

    RefRowReorderHint(RefRowReorderHint const &) = default;
    RefRowReorderHint(RefRowReorderHint &&) = default;
    RefRowReorderHint & operator =(RefRowReorderHint const &) = delete; // due to mrRowMap
    RefRowReorderHint & operator =(RefRowReorderHint &&) = delete; // due to mrRowMap

    const sc::ColRowReorderMapType& getRowMap() const;

    SCTAB getTab() const;
    SCCOL getStartColumn() const;
    SCCOL getEndColumn() const;
};

class RefStartListeningHint : public RefHint
{
public:
    RefStartListeningHint();
    virtual ~RefStartListeningHint() override;
};

class RefStopListeningHint : public RefHint
{
public:
    RefStopListeningHint();
    virtual ~RefStopListeningHint() override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
