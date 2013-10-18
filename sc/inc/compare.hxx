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

#ifndef SC_COMPARE_HXX
#define SC_COMPARE_HXX

#include "queryentry.hxx"

#include "rtl/ustring.hxx"

class ScDocument;

namespace sc {

struct Compare
{
    double nVal[2];
    OUString* pVal[2];
    bool bVal[2];
    bool bEmpty[2];

    bool mbIgnoreCase;

    Compare( OUString* p1, OUString* p2 ) :
        mbIgnoreCase(true)
    {
        pVal[0] = p1;
        pVal[1] = p2;
        bEmpty[0] = false;
        bEmpty[1] = false;
    }
};

struct CompareOptions
{
    ScQueryEntry        aQueryEntry;
    bool                bRegEx;
    bool                bMatchWholeCell;
    bool                bIgnoreCase;

    CompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg );
private:
    // Not implemented, prevent usage.
    CompareOptions();
    CompareOptions( const CompareOptions & );
    CompareOptions& operator=( const CompareOptions & );
};

double CompareFunc( bool bIgnoreCase, const Compare& rComp, CompareOptions* pOptions = NULL );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
