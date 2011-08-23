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
class SvStream;
class Table;
namespace binfilter {

class ScDocument;
class ScEEParser;
class ScTabEditEngine;

struct ScEEParseEntry;

class ScEEImport
{
protected:
    ScRange 			aRange;
    ScDocument*			pDoc;
    ScEEParser*			pParser;
    ScTabEditEngine*	pEngine;
    Table*				pRowHeights;

    BOOL				GraphicSize( USHORT nCol, USHORT nRow, USHORT nTab,
                            ScEEParseEntry* );
    void				InsertGraphic( USHORT nCol, USHORT nRow, USHORT nTab,
                            ScEEParseEntry* );

public:
                        ScEEImport( ScDocument* pDoc, const ScRange& rRange );
    virtual				~ScEEImport();

    ULONG				Read( SvStream& rStream );
    ScRange				GetRange()		{ return aRange; }
    virtual void		WriteToDocument( BOOL bSizeColsRows = FALSE,
                                        double nOutputFactor = 1.0 );
};



} //namespace binfilter
#endif
