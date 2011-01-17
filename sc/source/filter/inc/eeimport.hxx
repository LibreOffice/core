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
#ifndef SC_EEIMPORT_HXX
#define SC_EEIMPORT_HXX

#include "global.hxx"
#include "address.hxx"
#include "filter.hxx"
#include "scdllapi.h"

class ScDocument;
class ScEEParser;
class ScTabEditEngine;
class SvStream;
class Table;

struct ScEEParseEntry;

class ScEEImport : public ScEEAbsImport
{
protected:
    ScRange             maRange;
    ScDocument*         mpDoc;
    ScEEParser*         mpParser;
    ScTabEditEngine*    mpEngine;
    Table*              mpRowHeights;

    sal_Bool                GraphicSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     ScEEParseEntry* );
    void                InsertGraphic( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                       ScEEParseEntry* );
public:
    ScEEImport( ScDocument* pDoc, const ScRange& rRange );
    virtual ~ScEEImport();

    virtual sal_uLong    Read( SvStream& rStream, const String& rBaseURL );
    virtual ScRange  GetRange() { return maRange; }
    virtual void     WriteToDocument( sal_Bool bSizeColsRows = sal_False,
                                      double nOutputFactor = 1.0,
                                      SvNumberFormatter* pFormatter = NULL,
                                      bool bConvertDate = true );
};

#endif
