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
#ifndef INCLUDED_RSC_INC_RSCKEY_HXX
#define INCLUDED_RSC_INC_RSCKEY_HXX

// class forwards
class RscTop;

#include <rscall.h>

typedef struct
{
    Atom            nName;
    sal_uInt32      nTyp;
    sal_IntPtr      yylval;
} KEY_STRUCT;

class RscNameTable
{
    bool            bSort;      // whether it must be sorted for each insertion
    sal_uInt32      nEntries;   // number of entries
    KEY_STRUCT *    pTable;
public:
            RscNameTable();
            ~RscNameTable();
    void    SetSort( bool bSorted = true );
    Atom    Put( Atom nName, sal_uInt32 nTyp, sal_IntPtr nValue );
    Atom    Put( const char * pName, sal_uInt32 nTyp, sal_IntPtr nValue );
    Atom    Put( const char * pName, sal_uInt32 nTyp );
    void    Put( Atom nName, sal_uInt32 nTyp, RscTop * pClass );

            // if true, it was found
    bool    Get( Atom nName, KEY_STRUCT * pEle );
};

#endif // INCLUDED_RSC_INC_RSCKEY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
