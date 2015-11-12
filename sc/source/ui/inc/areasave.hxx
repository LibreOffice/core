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

#ifndef INCLUDED_SC_SOURCE_UI_INC_AREASAVE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_AREASAVE_HXX

#include "address.hxx"

#include <vector>

class ScDocument;
class ScAreaLink;

class ScAreaLinkSaver
{
private:
    OUString aFileName;
    OUString aFilterName;
    OUString aOptions;
    OUString aSourceArea;
    ScRange aDestArea;
    sal_uLong nRefresh;

public:
    ScAreaLinkSaver( const ScAreaLink& rSource );
    ScAreaLinkSaver( const ScAreaLinkSaver& rCopy );

    bool        IsEqual( const ScAreaLink& rCompare ) const;
    bool        IsEqualSource( const ScAreaLink& rCompare ) const;

    void        WriteToLink( ScAreaLink& rLink ) const;
    void        InsertNewLink( ScDocument* pDoc );
};

class ScAreaLinkSaveCollection
{
    typedef ::std::vector<ScAreaLinkSaver> DataType;
    DataType maData;
public:
    ScAreaLinkSaveCollection();
    ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& r );
    ~ScAreaLinkSaveCollection();

    bool        IsEqual( const ScDocument* pDoc ) const;
    void Restore( ScDocument* pDoc );

    // returns NULL if empty
    static ScAreaLinkSaveCollection* CreateFromDoc( const ScDocument* pDoc );

    ScAreaLinkSaver& operator[](size_t nIndex);
    const ScAreaLinkSaver& operator[](size_t nIndex) const;
    size_t size() const;
    void push_back(const ScAreaLinkSaver&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
