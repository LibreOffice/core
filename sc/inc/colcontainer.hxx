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

#ifndef INCLUDED_SC_INC_COLCONTAINER_HXX
#define INCLUDED_SC_INC_COLCONTAINER_HXX


#include "types.hxx"
#include "address.hxx"

#include <vector>


class ScColumn;
class ScDocument;
class ScColContainer
{
public:
    typedef std::vector<ScColumn*> ScColumnVector;
private:
    ScColumnVector    aCols;
    SCCOL             nLast;
    ScDocument*       pDocument;
public:
    ScColContainer( ScDocument* pDoc );
    ScColContainer( ScDocument* pDoc, const size_t nSize );
    ~ScColContainer();

    const ScColumn& operator[] ( const size_t nIndex ) const
    {
        return *aCols[nIndex];
    }

    const ScColumn& operator[] ( SCCOL nIndex ) const
    {
        return ( *this )[ static_cast<const size_t>( nIndex ) ];
    }

    const ScColumn& operator[] ( int nIndex ) const
    {
        return ( *this )[ static_cast<const size_t>( nIndex ) ];
    }

    ScColumn& operator[] ( const size_t nIndex )
    {
        return *aCols[nIndex];
    }

    ScColumn& operator[] ( SCCOL nIndex )
    {
        return ( *this )[ static_cast<const size_t>( nIndex ) ];
    }

    ScColumn& operator[] ( int nIndex )
    {
        return ( *this )[ static_cast<const size_t>( nIndex ) ];
    }

    SCCOL GetLastIndex() const
    {
        return nLast;
    }

    void CreateCol( SCCOL nColIdx, SCTAB nTab );
    void DeleteCol( SCCOL nColIdx );
    bool ColumnExists( SCCOL nColIdx ) const;
    void Clear();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
