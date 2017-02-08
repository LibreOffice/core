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
#ifndef INCLUDED_SC_SOURCE_FILTER_INC_EEIMPORT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_EEIMPORT_HXX

#include "global.hxx"
#include "address.hxx"
#include "filter.hxx"
#include "scdllapi.h"
#include <map>

class ScDocument;
class ScEEParser;
class ScTabEditEngine;
class SvStream;

struct ScEEParseEntry;

typedef std::map<SCROW, long> RowHeightMap;

class ScEEImport : public ScEEAbsImport
{
protected:
    ScRange             maRange;
    ScDocument*         mpDoc;
    std::unique_ptr<ScTabEditEngine>
                        mpEngine;
    std::unique_ptr<ScEEParser>        // needs mpEngine
                        mpParser;      // must reset before mpEngine resets
    RowHeightMap        maRowHeights;

    bool                GraphicSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     ScEEParseEntry* );
    void                InsertGraphic( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                       ScEEParseEntry* );
public:
    ScEEImport( ScDocument* pDoc, const ScRange& rRange );
    virtual ~ScEEImport() override;

    virtual sal_uLong    Read( SvStream& rStream, const OUString& rBaseURL ) override;
    virtual ScRange  GetRange() override { return maRange; }
    virtual void     WriteToDocument( bool bSizeColsRows = false,
                                      double nOutputFactor = 1.0,
                                      SvNumberFormatter* pFormatter = nullptr,
                                      bool bConvertDate = true ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
