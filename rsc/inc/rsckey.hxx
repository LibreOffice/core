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
#ifndef _RSCKEY_HXX
#define _RSCKEY_HXX

/****************** C l a s s   F o r w a r d s **************************/
class RscTop;

#include <rscall.h>

typedef struct {
    Atom          nName;
    sal_uInt32          nTyp;
    long            yylval;
} KEY_STRUCT;

class RscNameTable {
    sal_Bool            bSort;      //soll bei jedem einfuegen sortiert werden?
    sal_uInt32          nEntries;   //Anzahl der Eintr�ge
    KEY_STRUCT *    pTable;
public:
            RscNameTable();
            ~RscNameTable();
    void    SetSort( sal_Bool bSorted = sal_True );
    Atom  Put( Atom nName, sal_uInt32 nTyp, long nValue );
    Atom  Put( Atom nName, sal_uInt32 nTyp );
    Atom  Put( const char * pName, sal_uInt32 nTyp, long nValue );
    Atom  Put( const char * pName, sal_uInt32 nTyp );
    Atom  Put( Atom nName, sal_uInt32 nTyp, RscTop * pClass );
    Atom  Put( const char * pName, sal_uInt32 nTyp, RscTop * pClass );

            // sal_True, wurde gefunden
    sal_Bool    Get( Atom nName, KEY_STRUCT * pEle );
};


#endif// _RSCKEY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
