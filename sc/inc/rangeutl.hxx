/*************************************************************************
 *
 *  $RCSfile: rangeutl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:14:16 $
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

#ifndef SC_RANGEUTL_HXX
#define SC_RANGEUTL_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

//------------------------------------------------------------------------

class SvStream;

class ScArea;
class ScDocument;
class ScRange;
class ScRangeName;
class ScDBCollection;

enum RutlNameScope { RUTL_NONE=0, RUTL_NAMES, RUTL_DBASE };

//------------------------------------------------------------------------

class ScRangeUtil
{
public:
                ScRangeUtil()  {}
                ~ScRangeUtil() {}

    BOOL    MakeArea            ( const String&     rAreaStr,
                                  ScArea&           rArea,
                                  ScDocument*       pDoc,
                                  SCTAB         nTab ) const;

    void    CutPosString        ( const String&     theAreaStr,
                                  String&           thePosStr ) const;

    BOOL    IsAbsTabArea        ( const String&     rAreaStr,
                                  ScDocument*       pDoc,
                                  ScArea***         pppAreas    = 0,
                                  USHORT*           pAreaCount  = 0,
                                  BOOL              bAcceptCellRef = FALSE ) const;

    BOOL    IsAbsArea           ( const String& rAreaStr,
                                  ScDocument*   pDoc,
                                  SCTAB     nTab,
                                  String*       pCompleteStr = 0,
                                  ScRefAddress* pStartPos    = 0,
                                  ScRefAddress* pEndPos      = 0 ) const;

    BOOL    IsRefArea           ( const String& rAreaStr,
                                  ScDocument*   pDoc,
                                  SCTAB     nTab,
                                  String*       pCompleteStr = 0,
                                  ScRefAddress* pPosTripel   = 0 ) const
                                      { return FALSE; }

    BOOL    IsAbsPos            ( const String& rPosStr,
                                  ScDocument*   pDoc,
                                  SCTAB     nTab,
                                  String*       pCompleteStr = 0,
                                  ScRefAddress* pPosTripel   = 0 ) const;

    BOOL    MakeRangeFromName   ( const String& rName,
                                    ScDocument*     pDoc,
                                    SCTAB           nCurTab,
                                    ScRange&        rRange,
                                    RutlNameScope eScope=RUTL_NAMES
                                  ) const;
};

//------------------------------------------------------------------------

class ScArea
{
public:
            ScArea( SCTAB tab      = 0,
                    SCCOL colStart = 0,
                    SCROW rowStart = 0,
                    SCCOL colEnd   = 0,
                    SCROW rowEnd   = 0 );

            ScArea( const ScArea& r );

    void    Clear       ();
    ScArea& operator=   ( const ScArea& r );
    BOOL    operator==  ( const ScArea& r ) const;
    BOOL    operator!=  ( const ScArea& r ) const  { return !( operator==(r) ); }
    void    GetString   ( String& rStr,
                          BOOL bAbsolute = TRUE, ScDocument* pDoc = NULL ) const;

public:
    SCTAB nTab;
    SCCOL nColStart;
    SCROW nRowStart;
    SCCOL nColEnd;
    SCROW nRowEnd;
};

SvStream& operator<< ( SvStream& rStream, const ScArea& rArea );
SvStream& operator>> ( SvStream& rStream, ScArea& rArea );

//------------------------------------------------------------------------

//
//  gibt Bereiche mit Referenz und alle DB-Bereiche zurueck
//

class ScAreaNameIterator
{
private:
    ScRangeName*    pRangeName;
    ScDBCollection* pDBCollection;
    BOOL            bFirstPass;
    USHORT          nPos;
    String          aStrNoName;

public:
            ScAreaNameIterator( ScDocument* pDoc );
            ~ScAreaNameIterator() {}

    BOOL    Next( String& rName, ScRange& rRange );
    BOOL    WasDBName() const   { return !bFirstPass; }
};


#endif // SC_RANGEUTL_HXX

