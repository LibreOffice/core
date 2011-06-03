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

#ifndef SC_AREASAVE_HXX
#define SC_AREASAVE_HXX

#include "address.hxx"

#include <boost/ptr_container/ptr_vector.hpp>

class ScDocument;
class ScAreaLink;


class ScAreaLinkSaver
{
private:
    ::rtl::OUString aFileName;
    ::rtl::OUString aFilterName;
    ::rtl::OUString aOptions;
    ::rtl::OUString aSourceArea;
    ScRange aDestArea;
    sal_uLong nRefresh;

public:
    ScAreaLinkSaver( const ScAreaLink& rSource );
    ScAreaLinkSaver( const ScAreaLinkSaver& rCopy );
    ~ScAreaLinkSaver();

    bool        IsEqual( const ScAreaLink& rCompare ) const;
    bool        IsEqualSource( const ScAreaLink& rCompare ) const;

    void        WriteToLink( ScAreaLink& rLink ) const;
    void        InsertNewLink( ScDocument* pDoc ) const;
};


class ScAreaLinkSaveCollection
{
    typedef ::boost::ptr_vector<ScAreaLinkSaver> DataType;
    DataType maData;
public:
    ScAreaLinkSaveCollection();
    ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& r );
    ~ScAreaLinkSaveCollection();


    bool        IsEqual( const ScDocument* pDoc ) const;
    void        Restore( ScDocument* pDoc ) const;

    // returns NULL if empty
    static ScAreaLinkSaveCollection* CreateFromDoc( const ScDocument* pDoc );

    const ScAreaLinkSaver* operator[](size_t nIndex) const;
    size_t size() const;
    void clear();
    void push_back(ScAreaLinkSaver* p);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
