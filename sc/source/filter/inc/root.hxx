/*************************************************************************
 *
 *  $RCSfile: root.hxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:08:24 $
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

#ifndef _ROOT_HXX
#define _ROOT_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#ifndef _FLTTYPES_HXX
#include "flttypes.hxx"
#endif
#ifndef SC_FILTER_HXX
#include "filter.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif

class SotStorage;
class SvNumberFormatter;
class ScRangeName;
class ScProgress;

class NameBuffer;
class RangeNameBufferWK3;
class ShrfmlaBuffer;
class ExtNameBuff;
class ExtSheetBuffer;
class ExcelToSc;

class XclImpAutoFilterBuffer;
class XclImpPivotCacheList;
class _ScRangeListTabs;

class XclExpChTrTabId;
class XclExpUserBViewList;

class XclObjList;
class XclEscher;
class SfxStyleSheet;
class ExcRecordList;

class XclImpRoot;
class XclExpRoot;

// ---------------------------------------------------------- Excel Imp~/Exp~ -

struct RootData     // -> Inkarnation jeweils im ImportExcel-Objekt!
{
    double              fRowScale;              //  Spaltenbreiten / Zeilenhoehen
    ScDocument*         pDoc;
    ScRangeName*        pScRangeName;

    String              aStandard;              // Schluessel fuer Exc-Standard-Format
    BiffTyp             eDateiTyp;              // feine Differenzierung
    BiffTyp             eHauptDateiTyp;         // grobe Klassifizierung
    ExtSheetBuffer*     pExtSheetBuff;
    NameBuffer*         pTabNameBuff;
    ShrfmlaBuffer*      pShrfmlaBuff;
    ExtNameBuff*        pExtNameBuff;
    ExcelToSc*          pFmlaConverter;
    const CharSet*      pCharset;               // actual charset (im- / export!)

    BOOL                bCellCut;               // bei Ueberlauf max. Cols / Rows
    BOOL                bChartTab;              // Tabelle mit einem einzigen Chart

    // Biff8
    XclImpAutoFilterBuffer* pAutoFilterBuffer;      // ranges for autofilter and advanced filter
    _ScRangeListTabs*       pPrintRanges;
    _ScRangeListTabs*       pPrintTitles;

    // Erweiterungen fuer Export
    XclExpChTrTabId*        pTabId;             // pointer to rec list, do not destroy
    XclExpUserBViewList*    pUserBViewList;     // pointer to rec list, do not destroy

    // Biff8
    XclObjList*         pObjRecs;
    XclEscher*          pEscher;

    BOOL                bWriteVBAStorage;

    XclImpRoot*         pIR;
    XclExpRoot*         pER;

                        RootData( void );       // -> exctools.cxx
                        ~RootData();            // -> exctools.cxx
};

class ExcRoot
{
protected:
    RootData*       pExcRoot;
    inline          ExcRoot( RootData* pNexExcRoot ) : pExcRoot( pNexExcRoot ) {}
    inline          ExcRoot( const ExcRoot& rCopy ) : pExcRoot( rCopy.pExcRoot ) {}
};

// ---------------------------------------------------------- Lotus Imp~/Exp~ -

class LotusRangeList;
class LotusFontBuffer;
class LotAttrTable;


struct LOTUS_ROOT
{
    ScDocument*         pDoc;
    LotusRangeList*     pRangeNames;
    ScRangeName*        pScRangeName;
    CharSet             eCharsetQ;
    Lotus123Typ         eFirstType;
    Lotus123Typ         eActType;
    ScRange             aActRange;
    RangeNameBufferWK3* pRngNmBffWK3;
    LotusFontBuffer*    pFontBuff;
    LotAttrTable*       pAttrTable;
};

extern LOTUS_ROOT*      pLotusRoot; // -> Inkarn. in filter.cxx

// ----------------------------------------------------------------------------

/** List class for sal_uInt16 values.
    @deprecated */
class ScfUInt16List : protected List
{
public:
    inline              ScfUInt16List() : List() {}
    inline              ScfUInt16List( const ScfUInt16List& rCopy ) : List( rCopy ) {}

    inline ScfUInt16List& operator=( const ScfUInt16List& rSource )
                            { List::operator=( rSource ); return *this; }

                        List::Clear;
                        List::Count;
    inline bool         Empty() const   { return List::Count() == 0; }

    inline sal_uInt16   First() { return (sal_uInt16)(sal_uInt32) List::First(); }
    inline sal_uInt16   Last()  { return (sal_uInt16)(sal_uInt32) List::Last(); }
    inline sal_uInt16   Next()  { return (sal_uInt16)(sal_uInt32) List::Next(); }
    inline sal_uInt16   Prev()  { return (sal_uInt16)(sal_uInt32) List::Prev(); }

    inline sal_uInt16   GetValue( ULONG nIndex ) const
                            { return (sal_uInt16)(sal_uInt32) List::GetObject( nIndex ); }
    inline bool         Contains( sal_uInt16 nValue ) const
                            { return List::GetPos( (void*)(sal_uInt32) nValue ) != LIST_ENTRY_NOTFOUND; }

    inline void         Insert( sal_uInt16 nValue, ULONG nIndex )
                            { List::Insert( (void*)(sal_uInt32) nValue, nIndex ); }
    inline void         Append( sal_uInt16 nValue )
                            { List::Insert( (void*)(sal_uInt32) nValue, LIST_APPEND ); }
    inline sal_uInt16   Replace( sal_uInt16 nValue, ULONG nIndex )
                            { return (sal_uInt16)(sal_uInt32) List::Replace( (void*)(sal_uInt32) nValue, nIndex ); }
    inline sal_uInt16   Remove( ULONG nIndex )
                            { return (sal_uInt16)(sal_uInt32) List::Remove( nIndex ); }
};

// ----------------------------------------------------------------------------

/** List class for sal_uInt32 values.
    @deprecated */
class ScfUInt32List : protected List
{
public:
    inline              ScfUInt32List() : List() {}
    inline              ScfUInt32List( const ScfUInt32List& rCopy ) : List( rCopy ) {}

    inline ScfUInt32List& operator=( const ScfUInt32List& rSource )
                            { List::operator=( rSource ); return *this; }

                        List::Clear;
                        List::Count;
    inline bool         Empty() const   { return List::Count() == 0; }

    inline sal_uInt32   First() { return (sal_uInt32) List::First(); }
    inline sal_uInt32   Last()  { return (sal_uInt32) List::Last(); }
    inline sal_uInt32   Next()  { return (sal_uInt32) List::Next(); }
    inline sal_uInt32   Prev()  { return (sal_uInt32) List::Prev(); }

    inline sal_uInt32   GetValue( ULONG nIndex ) const
                            { return (sal_uInt32) List::GetObject( nIndex ); }
    inline bool         Contains( sal_uInt32 nValue ) const
                            { return List::GetPos( (void*) nValue ) != LIST_ENTRY_NOTFOUND; }

    inline void         Insert( sal_uInt32 nValue, ULONG nIndex )
                            { List::Insert( (void*) nValue, nIndex ); }
    inline void         Append( sal_uInt32 nValue )
                            { List::Insert( (void*) nValue, LIST_APPEND ); }
    inline sal_uInt32   Replace( sal_uInt32 nValue, ULONG nIndex )
                            { return (sal_uInt32) List::Replace( (void*) nValue, nIndex ); }
    inline sal_uInt32   Remove( sal_uInt32 nIndex )
                            { return (sal_uInt32) List::Remove( nIndex ); }
};

// ----------------------------------------------------------------------------

#endif

