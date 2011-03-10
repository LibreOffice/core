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

#ifndef SC_EXCSCEN_HXX
#define SC_EXCSCEN_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include <tools/solar.h>
#include <tools/string.hxx>

struct RootData;
class XclImpRoot;
class XclImpStream;
class ScDocument;

class ExcScenarioCell
{
private:
    String                      aValue;
public:
    const sal_uInt16                nCol;
    const sal_uInt16                nRow;

    ExcScenarioCell( const UINT16 nC, const UINT16 nR );

    inline void SetValue( const String& rVal ) { aValue = rVal; }

    inline const String& GetValue( void ) const { return aValue; }
};

class ExcScenario
{
public:

    ExcScenario( XclImpStream& rIn, const RootData& rRoot );

    ~ExcScenario();

    void Apply( const XclImpRoot& rRoot, const sal_Bool bLast = false );

protected:

    String* pName;
    String* pComment;
    String* pUserName;
    UINT8 nProtected;
    const UINT16 nTab;
    boost::ptr_vector<ExcScenarioCell> aEntries;
};

struct ExcScenarioList
{
    ExcScenarioList () : nLastScenario(0) {}

    void Apply( const XclImpRoot& rRoot );

    sal_uInt16 nLastScenario;
    boost::ptr_vector<ExcScenario> aEntries;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
