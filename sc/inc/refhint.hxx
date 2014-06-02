/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_REFHINT_HXX
#define SC_REFHINT_HXX

#include <address.hxx>
#include <simplehintids.hxx>

namespace sc {

class RefHint : public SfxSimpleHint
{
public:
    enum Type {
        Moved,
        ColumnReordered,
        RowReordered
    };

private:
    Type meType;

    RefHint(); // disabled

protected:
    RefHint( Type eType );

public:
    virtual ~RefHint() = 0;

    Type getType() const;
};

class RefMovedHint : public RefHint
{
    ScRange maRange;
    ScAddress maMoveDelta;

public:

    RefMovedHint( const ScRange& rRange, const ScAddress& rMove );
    virtual ~RefMovedHint();

    /**
     * Get the source range from which the references have moved.
     */
    const ScRange& getRange() const;

    /**
     * Get the movement vector.
     */
    const ScAddress& getDelta() const;
};

class RefColReorderHint : public RefHint
{
    const sc::ColRowReorderMapType& mrColMap;
    SCTAB mnTab;
    SCROW mnRow1;
    SCROW mnRow2;

public:
    RefColReorderHint( const sc::ColRowReorderMapType& rColMap, SCTAB nTab, SCROW nRow1, SCROW nRow2 );
    virtual ~RefColReorderHint();

    const sc::ColRowReorderMapType& getColMap() const;

    SCTAB getTab() const;
    SCROW getStartRow() const;
    SCROW getEndRow() const;
};

class RefRowReorderHint : public RefHint
{
    const sc::ColRowReorderMapType& mrRowMap;
    SCTAB mnTab;
    SCCOL mnCol1;
    SCCOL mnCol2;

public:
    RefRowReorderHint( const sc::ColRowReorderMapType& rRowMap, SCTAB nTab, SCCOL nCol1, SCCOL nCol2 );
    virtual ~RefRowReorderHint();

    const sc::ColRowReorderMapType& getRowMap() const;

    SCTAB getTab() const;
    SCCOL getStartColumn() const;
    SCCOL getEndColumn() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
