/*************************************************************************
 *
 *  $RCSfile: expbase.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


class SvStream;
class ScFieldEditEngine;

class ScExportBase
{
public:
#if defined UNX || defined MAC
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
    BOOL                TrimDataArea( USHORT nTab, USHORT& nStartCol,
                            USHORT& nStartRow, USHORT& nEndCol, USHORT& nEndRow ) const;

                        // Ausgabebereich einer Tabelle ermitteln,
                        // Hidden Cols/Rows an den Raendern beruecksichtigt,
                        // return: TRUE wenn Bereich vorhanden
    BOOL                GetDataArea( USHORT nTab, USHORT& nStartCol,
                            USHORT& nStartRow, USHORT& nEndCol, USHORT& nEndRow ) const;

                        // Tabelle nicht vorhanden oder leer
    BOOL                IsEmptyTable( USHORT nTab ) const;

    ScFieldEditEngine&  GetEditEngine() const;

};


#endif  // SC_EXPBASE_HXX

