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

#include "address.hxx"
#include "simplehintids.hxx"

namespace sc {

class RefHint : public SfxSimpleHint
{
public:
    enum Type { Moved, ColumnReordered };

private:
    Type meType;

    RefHint(); // disabled

protected:
    RefHint( Type eType );

public:
    virtual ~RefHint() = 0;

    Type getType() const { return meType; }
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
    const ScRange& getRange() const { return maRange;}

    /**
     * Get the movement vector.
     */
    const ScAddress& getDelta() const { return maMoveDelta;}
};

class RefColReorderHint : public RefHint
{
    const sc::ColReorderMapType& mrColMap;
    SCTAB mnTab;
    SCROW mnRow1;
    SCROW mnRow2;

public:
    RefColReorderHint( const sc::ColReorderMapType& rColMap, SCTAB nTab, SCROW nRow1, SCROW nRow2 );
    virtual ~RefColReorderHint();

    const sc::ColReorderMapType& getColMap() const { return mrColMap;}

    SCTAB getTab() const { return mnTab;}
    SCROW getStartRow() const { return mnRow1;}
    SCROW getEndRow() const { return mnRow2;}
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
