/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewutil.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 10:14:04 $
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
#ifndef SC_VIEWUTIL_HXX
#define SC_VIEWUTIL_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class String;
class SfxItemSet;
class SfxBindings;
class SvxFontItem;

class ScChangeAction;
class ScChangeViewSettings;
class ScDocument;
class ScAddress;
class ScRange;
class ScMarkData;

enum ScUpdateMode { SC_UPDATE_ALL, SC_UPDATE_CHANGED, SC_UPDATE_MARKS };

// ---------------------------------------------------------------------------

class ScViewUtil                                // static Methoden
{
public:
    static BOOL ExecuteCharMap( const SvxFontItem&  rOldFont,
                                SvxFontItem&        rNewFont,
                                String&             rString );

    static BOOL IsActionShown( const ScChangeAction& rAction,
                                const ScChangeViewSettings& rSettings,
                                ScDocument& rDocument );

    static void PutItemScript( SfxItemSet& rShellSet, const SfxItemSet& rCoreSet,
                                USHORT nWhichId, USHORT nScript );

    static USHORT GetEffLanguage( ScDocument* pDoc, const ScAddress& rPos );

    static sal_Int32 GetTransliterationType( USHORT nSlotID );

    static bool HasFiltered( const ScRange& rRange, ScDocument* pDoc );
    static void UnmarkFiltered( ScMarkData& rMark, ScDocument* pDoc );

    static void HideDisabledSlot( SfxItemSet& rSet, SfxBindings& rBindings, USHORT nSlotId );
};

// ---------------------------------------------------------------------------

class ScUpdateRect
{
private:
    SCCOL   nOldStartX;
    SCROW   nOldStartY;
    SCCOL   nOldEndX;
    SCROW   nOldEndY;
    SCCOL   nNewStartX;
    SCROW   nNewStartY;
    SCCOL   nNewEndX;
    SCROW   nNewEndY;
    SCCOL   nContX1;
    SCROW   nContY1;
    SCCOL   nContX2;
    SCROW   nContY2;
public:
            ScUpdateRect( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    void    SetNew( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    BOOL    GetDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
    BOOL    GetXorDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, BOOL& rCont );
    void    GetContDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
};




#endif

