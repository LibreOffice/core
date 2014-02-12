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

#ifndef SC_EXPBASE_HXX
#define SC_EXPBASE_HXX

#include <tools/solar.h>
#include "global.hxx"
#include "address.hxx"


class SvStream;
class ScFieldEditEngine;

class ScExportBase
{
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
    bool                TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Ausgabebereich einer Tabelle ermitteln,
                        // Hidden Cols/Rows an den Raendern beruecksichtigt,
                        // return: sal_True wenn Bereich vorhanden
    bool                GetDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Tabelle nicht vorhanden oder leer
    bool                IsEmptyTable( SCTAB nTab ) const;

    ScFieldEditEngine&  GetEditEngine() const;

};


#endif  // SC_EXPBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
