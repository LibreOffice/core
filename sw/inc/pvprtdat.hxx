/*************************************************************************
 *
 *  $RCSfile: pvprtdat.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
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

#ifndef _PVPRTDAT_HXX
#define _PVPRTDAT_HXX


#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class SwPagePreViewPrtData
{
    ULONG nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;
    BYTE nRow, nCol;
    BOOL bLandscape : 1;
    BOOL bStretch : 1;
public:
    SwPagePreViewPrtData()
        : nLeftSpace(0), nRightSpace(0), nTopSpace(0), nBottomSpace(0),
            nHorzSpace(0), nVertSpace(0), nRow(1), nCol(1),
            bLandscape(0),bStretch(0)
    {}

    ULONG GetLeftSpace() const          { return nLeftSpace; }
    void SetLeftSpace( ULONG n )        { nLeftSpace = n; }

    ULONG GetRightSpace() const         { return nRightSpace; }
    void SetRightSpace( ULONG n )       { nRightSpace = n; }

    ULONG GetTopSpace() const           { return nTopSpace; }
    void SetTopSpace( ULONG n )         { nTopSpace = n; }

    ULONG GetBottomSpace() const        { return nBottomSpace; }
    void SetBottomSpace( ULONG n )      { nBottomSpace = n; }

    ULONG GetHorzSpace() const          { return nHorzSpace; }
    void SetHorzSpace( ULONG n )        { nHorzSpace = n; }

    ULONG GetVertSpace() const          { return nVertSpace; }
    void SetVertSpace( ULONG n )        { nVertSpace = n; }

    BYTE GetRow() const                 { return nRow; }
    void SetRow(BYTE n )                { nRow = n; }

    BYTE GetCol() const                 { return nCol; }
    void SetCol( BYTE n )               { nCol = n; }

    BOOL GetLandscape() const           { return bLandscape; }
    void SetLandscape( BOOL b )         { bLandscape = b; }

    // JP 19.08.98: wird zur Zeit nicht ausgewertet, weil der Font sich
    //              nicht enstprechend stretch laesst.
    BOOL GetStretch() const             { return bStretch; }
    void SetStretch( BOOL b )           { bStretch = b; }
};


#endif


