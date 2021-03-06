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

#pragma once

#include <memory>
#include <address.hxx>

class SvNumberFormatter;
class SvStream;
class ScFieldEditEngine;

class ScExportBase
{
protected:

    SvStream&           rStrm;
    ScRange             aRange;
    ScDocument*         pDoc;
    SvNumberFormatter*  pFormatter;
    std::unique_ptr<ScFieldEditEngine>
                        pEditEngine;

public:

                        ScExportBase( SvStream&, ScDocument*, const ScRange& );
    virtual             ~ScExportBase();

                        // Trim borders of hidden Cols/Rows,
                        // return: sal_True if range exists
                        // Start/End/Col/Row must have valid starting values
    bool                TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Get Data Area of a table,
                        // adjust borders of hidden Cols/Rows,
                        // return: sal_True if range exists
    bool                GetDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // table does not exist or is empty
    bool                IsEmptyTable( SCTAB nTab ) const;

    ScFieldEditEngine&  GetEditEngine() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
