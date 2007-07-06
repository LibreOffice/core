/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expbase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 12:38:15 $
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

#ifndef SC_EXPBASE_HXX
#define SC_EXPBASE_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif


class SvStream;
class ScFieldEditEngine;

class ScExportBase
{
public:
#if defined UNX
    static const sal_Char __FAR_DATA sNewLine;
#else
    static const sal_Char __FAR_DATA sNewLine[];
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
                        // return: TRUE wenn Bereich vorhanden
                        // Start/End/Col/Row muessen gueltige Ausgangswerte sein
    BOOL                TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Ausgabebereich einer Tabelle ermitteln,
                        // Hidden Cols/Rows an den Raendern beruecksichtigt,
                        // return: TRUE wenn Bereich vorhanden
    BOOL                GetDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Tabelle nicht vorhanden oder leer
    BOOL                IsEmptyTable( SCTAB nTab ) const;

    ScFieldEditEngine&  GetEditEngine() const;

};


#endif  // SC_EXPBASE_HXX

