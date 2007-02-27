/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eeimport.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:56:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_EEIMPORT_HXX
#define SC_EEIMPORT_HXX

#include "global.hxx"

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

class ScDocument;
class ScEEParser;
class ScTabEditEngine;
class SvStream;
class Table;

struct ScEEParseEntry;

class ScEEImport
{
protected:
    ScRange             maRange;
    ScDocument*         mpDoc;
    ScEEParser*         mpParser;
    ScTabEditEngine*    mpEngine;
    Table*              mpRowHeights;

    BOOL                GraphicSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            ScEEParseEntry* );
    void                InsertGraphic( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            ScEEParseEntry* );

public:
                        ScEEImport( ScDocument* pDoc, const ScRange& rRange );
    virtual             ~ScEEImport();

    ULONG               Read( SvStream& rStream, const String& rBaseURL );
    ScRange             GetRange()      { return maRange; }
    virtual void        WriteToDocument( BOOL bSizeColsRows = FALSE,
                                        double nOutputFactor = 1.0 );
};



#endif
