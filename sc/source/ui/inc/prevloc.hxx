/*************************************************************************
 *
 *  $RCSfile: prevloc.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2002-02-22 09:55:59 $
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

#ifndef SC_PREVLOC_HXX
#define SC_PREVLOC_HXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif


class Window;
class Point;
class Rectangle;
class ScAddress;
class ScRange;
class ScDocument;

class ScPreviewLocationData
{
    Window*     pWindow;
    ScDocument* pDoc;
    MapMode     aCellMapMode;
    List        aEntries;

    ScAddress   GetCellFromRange( const Size& rOffsetPixel, const ScRange& rRange ) const;
    Rectangle   GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const;

public:
            ScPreviewLocationData( ScDocument* pDocument, Window* pWin );
            ~ScPreviewLocationData();

    void    SetCellMapMode( const MapMode& rMapMode );
    void    Clear();
    void    AddCellRange( const Rectangle& rRect, const ScRange& rRange );
    void    AddColHeaders( const Rectangle& rRect, USHORT nStartCol, USHORT nEndCol );
    void    AddRowHeaders( const Rectangle& rRect, USHORT nStartRow, USHORT nEndRow );
    void    AddHeaderFooter( const Rectangle& rRect );
    void    AddNoteMark( const Rectangle& rRect, const ScAddress& rPos );
    void    AddNoteText( const Rectangle& rRect, const ScAddress& rPos );

    BOOL    GetCell( const Point& rPos, ScAddress& rCellPos, Rectangle& rCellRect ) const;
    BOOL    GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const;
};

#endif
