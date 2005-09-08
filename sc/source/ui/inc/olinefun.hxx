/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olinefun.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:40:48 $
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

#ifndef SC_OLINEFUN_HXX
#define SC_OLINEFUN_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScDocShell;
class ScRange;


// ---------------------------------------------------------------------------

class ScOutlineDocFunc
{
private:
    ScDocShell& rDocShell;

public:
                ScOutlineDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                ~ScOutlineDocFunc() {}

    BOOL        MakeOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi );
    BOOL        RemoveOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi );
    BOOL        RemoveAllOutlines( SCTAB nTab, BOOL bRecord, BOOL bApi );
    BOOL        AutoOutline( const ScRange& rRange, BOOL bRecord, BOOL bApi );

    BOOL        SelectLevel( SCTAB nTab, BOOL bColumns, USHORT nLevel,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL        ShowMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi );
    BOOL        HideMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi );

    BOOL        ShowOutline( SCTAB nTab, BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi );
    BOOL        HideOutline( SCTAB nTab, BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi );
};



#endif

