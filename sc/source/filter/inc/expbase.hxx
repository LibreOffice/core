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

#ifndef SC_EXPBASE_HXX
#define SC_EXPBASE_HXX

#include <tools/solar.h>
#include "global.hxx"
#include "address.hxx"


class SvStream;
class ScFieldEditEngine;

class ScExportBase
{
public:
#if defined UNX
    static const sal_Char sNewLine;
#else
    static const sal_Char sNewLine[];
#endif

protected:

    SvStream&           rStrm;
    ScRange             aRange;
    ScDocument*         pDoc;
    SvNumberFormatter*  pFormatter;
    ScFieldEditEngine*  pEditEngine;

public:

                        ScExportBase( SvStream&, ScDocument*, const ScRange& );
    virtual             ~ScExportBase();

                        // Hidden Cols/Rows an den Raendern trimmen,
                        // return: sal_True wenn Bereich vorhanden
                        // Start/End/Col/Row muessen gueltige Ausgangswerte sein
    sal_Bool                TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Ausgabebereich einer Tabelle ermitteln,
                        // Hidden Cols/Rows an den Raendern beruecksichtigt,
                        // return: sal_True wenn Bereich vorhanden
    sal_Bool                GetDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Tabelle nicht vorhanden oder leer
    sal_Bool                IsEmptyTable( SCTAB nTab ) const;

    ScFieldEditEngine&  GetEditEngine() const;

};


#endif  // SC_EXPBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
