/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_FTOOLS_HXX
#define SC_FTOOLS_HXX

#include <vector>
#include <map>
#include <limits>
#include <memory>
#include <tools/string.hxx>
#include <sal/macros.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <oox/helper/helper.hxx>
#include "filter.hxx"
#include "scdllapi.h"

// Common macros ==============================================================

// items and item sets --------------------------------------------------------

/** Expands to the item (with type 'itemtype') with Which-ID 'which'. */
#define GETITEM( itemset, itemtype, which ) \
    static_cast< const itemtype & >( (itemset).Get( which ) )

/** Expands to the value (with type 'valuetype') of the item with Which-ID 'which'. */
#define GETITEMVALUE( itemset, itemtype, which, valuetype ) \
    static_cast< valuetype >( GETITEM( itemset, itemtype, which ).GetValue() )

/** Expands to the value of the SfxBoolItem with Which-ID 'which'. */
#define GETITEMBOOL( itemset, which ) \
    GETITEMVALUE( itemset, SfxBoolItem, which, bool )

// Global static helpers ======================================================

// Value range limit helpers --------------------------------------------------

/** Returns the value, if it is not less than nMin, otherwise nMin. */
template< typename ReturnType, typename Type >
inline ReturnType llimit_cast( Type nValue, ReturnType nMin )
{ return static_cast< ReturnType >( ::std::max< Type >( nValue, nMin ) ); }

/** Returns the value, if it fits into ReturnType, otherwise the minimum value of ReturnType. */
template< typename ReturnType, typename Type >
inline ReturnType llimit_cast( Type nValue )
{ return llimit_cast( nValue, ::std::numeric_limits< ReturnType >::min() ); }

/** Returns the value, if it is not greater than nMax, otherwise nMax. */
template< typename ReturnType, typename Type >
inline ReturnType ulimit_cast( Type nValue, ReturnType nMax )
{ return static_cast< ReturnType >( ::std::min< Type >( nValue, nMax ) ); }

/** Returns the value, if it fits into ReturnType, otherwise the maximum value of ReturnType. */
template< typename ReturnType, typename Type >
inline ReturnType ulimit_cast( Type nValue )
{ return ulimit_cast( nValue, ::std::numeric_limits< ReturnType >::max() ); }

/** Returns the value, if it is not less than nMin and not greater than nMax, otherwise one of the limits. */
template< typename ReturnType, typename Type >
inline ReturnType limit_cast( Type nValue, ReturnType nMin, ReturnType nMax )
{ return static_cast< ReturnType >( ::std::max< Type >( ::std::min< Type >( nValue, nMax ), nMin ) ); }

/** Returns the value, if it fits into ReturnType, otherwise one of the limits of ReturnType. */
template< typename ReturnType, typename Type >
inline ReturnType limit_cast( Type nValue )
{ return limit_cast( nValue, ::std::numeric_limits< ReturnType >::min(), ::std::numeric_limits< ReturnType >::max() ); }

// Read from bitfields --------------------------------------------------------

/** Returns true, if at least one of the bits set in nMask is set in nBitField. */
template< typename Type >
inline bool get_flag( Type nBitField, Type nMask )
{ return (nBitField & nMask) != 0; }

/** Returns nSet, if at least one bit of nMask is set in nBitField, otherwise nUnset. */
template< typename ReturnType, typename Type >
inline ReturnType get_flagvalue( Type nBitField, Type nMask, ReturnType nSet, ReturnType nUnset )
{ return ::get_flag( nBitField, nMask ) ? nSet : nUnset; }

/** Extracts a value from a bit field.
    @descr  Returns in rnRet the data fragment from nBitField, that starts at bit nStartBit
    (0-based, bit 0 is rightmost) with the width of nBitCount. rnRet will be right-aligned (normalized).
    For instance: extract_value( n, 0x4321, 8, 4 ) stores 3 in n (value in bits 8-11). */
template< typename ReturnType, typename Type >
inline ReturnType extract_value( Type nBitField, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{ return static_cast< ReturnType >( ((1UL << nBitCount) - 1) & (nBitField >> nStartBit) ); }

// Write to bitfields ---------------------------------------------------------

/** Sets or clears (according to bSet) all set bits of nMask in rnBitField. */
template< typename Type >
inline void set_flag( Type& rnBitField, Type nMask, bool bSet = true )
{ if( bSet ) rnBitField |= nMask; else rnBitField &= ~nMask; }

/** Inserts a value into a bitfield.
    @descr  Inserts the lower nBitCount bits of nValue into rnBitField, starting
    there at bit nStartBit. Other contents of rnBitField keep unchanged. */
template< typename Type, typename InsertType >
void insert_value( Type& rnBitField, InsertType nValue, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{
    unsigned long nMask = ((1UL << nBitCount) - 1);
    Type nNewValue = static_cast< Type >( nValue & nMask );
    (rnBitField &= ~(nMask << nStartBit)) |= (nNewValue << nStartBit);
}

// ============================================================================

class Color;
class SfxPoolItem;
class SfxItemSet;
class ScStyleSheet;
class ScStyleSheetPool;
class SotStorageRef;
class SotStorageStreamRef;
class SvStream;

/** Contains static methods used anywhere in the filters. */
class ScfTools : boost::noncopyable
{
public:

// *** common methods *** -----------------------------------------------------

    /** Reads a 10-byte-long-double and converts it to double. */
    static double       ReadLongDouble( SvStream& rStrm );
    /** Returns system text encoding for byte string conversion. */
    static rtl_TextEncoding GetSystemTextEncoding();
    /** Returns a string representing the hexadecimal value of nValue. */
    static OUString       GetHexStr( sal_uInt16 nValue );

    /** Mixes RGB components with given transparence.
        @param nTrans  Foreground transparence (0x00 == full nFore ... 0x80 = full nBack). */
    static sal_uInt8    GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans );
    /** Mixes colors with given transparence.
        @param nTrans  Foreground transparence (0x00 == full rFore ... 0x80 = full rBack). */
    static Color        GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt8 nTrans );

// *** conversion of names *** ------------------------------------------------

    /** Converts a string to a valid Calc defined name or database range name.
        @descr  Defined names in Calc may contain letters, digits (*), underscores, periods (*),
        colons (*), question marks, and dollar signs.
        (*) = not allowed at first position. */
    static void         ConvertToScDefinedName( String& rName );

// *** streams and storages *** -----------------------------------------------

    /** Tries to open an existing storage with the specified name in the passed storage (read-only). */
    static SotStorageRef OpenStorageRead( SotStorageRef xStrg, const String& rStrgName );
    /** Creates and opens a storage with the specified name in the passed storage (read/write). */
    static SotStorageRef OpenStorageWrite( SotStorageRef xStrg, const String& rStrgName );

    /** Tries to open an existing stream with the specified name in the passed storage (read-only). */
    static SotStorageStreamRef OpenStorageStreamRead( SotStorageRef xStrg, const String& rStrmName );
    /** Creates and opens a stream with the specified name in the passed storage (read/write). */
    static SotStorageStreamRef OpenStorageStreamWrite( SotStorageRef xStrg, const String& rStrmName );

// *** item handling *** ------------------------------------------------------

    /** Returns true, if the passed item set contains the item.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItem( const SfxItemSet& rItemSet, sal_uInt16 nWhichId, bool bDeep );
    /** Returns true, if the passed item set contains at least one of the items.
        @param pnWhichIds  Zero-terminated array of Which-IDs.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItems( const SfxItemSet& rItemSet, const sal_uInt16* pnWhichIds, bool bDeep );

    /** Puts the item into the passed item set.
        @descr  The item will be put into the item set, if bSkipPoolDef is false,
        or if the item differs from the default pool item.
        @param rItemSet  The destination item set.
        @param rItem  The item to put into the item set.
        @param nWhichId  The Which-ID to set with the item.
        @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
    static void         PutItem(
                            SfxItemSet& rItemSet, const SfxPoolItem& rItem,
                            sal_uInt16 nWhichId, bool bSkipPoolDef );

    /** Puts the item into the passed item set.
        @descr  The item will be put into the item set, if bSkipPoolDef is false,
        or if the item differs from the default pool item.
        @param rItemSet  The destination item set.
        @param rItem  The item to put into the item set.
        @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
    static void         PutItem( SfxItemSet& rItemSet, const SfxPoolItem& rItem, bool bSkipPoolDef );

// *** style sheet handling *** -----------------------------------------------

    /** Creates and returns a cell style sheet and inserts it into the pool.
        @descr  If the style sheet is already in the pool, another unused style name is used.
        @param bForceName  Controls behaviour, if the style already exists:
        true = Old existing style will be renamed; false = New style gets another name. */
    static ScStyleSheet& MakeCellStyleSheet(
                            ScStyleSheetPool& rPool,
                            const OUString& rStyleName, bool bForceName );
    /** Creates and returns a page style sheet and inserts it into the pool.
        @descr  If the style sheet is already in the pool, another unused style name is used.
        @param bForceName  Controls behaviour, if the style already exists:
        true = Old existing style will be renamed; false = New style gets another name. */
    static ScStyleSheet& MakePageStyleSheet(
                            ScStyleSheetPool& rPool,
                            const OUString& rStyleName, bool bForceName );

// *** byte string import operations *** --------------------------------------

    /** Reads and returns a zero terminated byte string and decreases a stream counter. */
    static OString read_zeroTerminated_uInt8s_ToOString(SvStream& rStrm, sal_Int32& rnBytesLeft);
    /** Reads and returns a zero terminated byte string and decreases a stream counter. */
    inline static OUString read_zeroTerminated_uInt8s_ToOUString(SvStream& rStrm, sal_Int32& rnBytesLeft, rtl_TextEncoding eTextEnc)
    {
        return OStringToOUString(read_zeroTerminated_uInt8s_ToOString(rStrm, rnBytesLeft), eTextEnc);
    }

    /** Appends a zero terminated byte string. */
    static void         AppendCString( SvStream& rStrm, String& rString, rtl_TextEncoding eTextEnc );

// *** HTML table names <-> named range names *** -----------------------------

    /** Returns the built-in range name for an HTML document. */
    static const OUString& GetHTMLDocName();
    /** Returns the built-in range name for all HTML tables. */
    static const OUString& GetHTMLTablesName();
    /** Returns the built-in range name for an HTML table, specified by table index. */
    static OUString       GetNameFromHTMLIndex( sal_uInt32 nIndex );
    /** Returns the built-in range name for an HTML table, specified by table name. */
    static String       GetNameFromHTMLName( const String& rTabName );

    /** Returns true, if rSource is the built-in range name for an HTML document. */
    static bool         IsHTMLDocName( const OUString& rSource );
    /** Returns true, if rSource is the built-in range name for all HTML tables. */
    static bool         IsHTMLTablesName( const OUString& rSource );
    /** Converts a built-in range name to an HTML table name.
        @param rSource  The string to be determined.
        @param rName  The HTML table name.
        @return  true, if conversion was successful. */
    static bool         GetHTMLNameFromName( const String& rSource, OUString& rName );

private:
    /** Returns the prefix for table index names. */
    static const String& GetHTMLIndexPrefix();
    /** Returns the prefix for table names. */
    static const String& GetHTMLNamePrefix();
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static items. To enforce this, the default constructor
        is made private */
    ScfTools();
};

// Containers =================================================================

typedef ::std::vector< sal_uInt8 >                  ScfUInt8Vec;
typedef ::std::vector< sal_Int16 >                  ScfInt16Vec;
typedef ::std::vector< sal_uInt16 >                 ScfUInt16Vec;
typedef ::std::vector< sal_Int32 >                  ScfInt32Vec;
typedef ::std::vector< sal_uInt32 >                 ScfUInt32Vec;
typedef ::std::vector< OUString >            ScfStringVec;

// ----------------------------------------------------------------------------

class ScFormatFilterPluginImpl : public ScFormatFilterPlugin
{
public:
    ScFormatFilterPluginImpl();
    virtual ~ScFormatFilterPluginImpl();
    // various import filters
    virtual FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet eSrc = RTL_TEXTENCODING_DONTKNOW );
    virtual FltError ScImportQuattroPro( SfxMedium &rMedium, ScDocument *pDoc );
    virtual FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT );
        // eFormat == EIF_AUTO  -> passender Filter wird automatisch verwendet
        // eFormat == EIF_BIFF5 -> nur Biff5-Stream fuehrt zum Erfolg (auch wenn in einem Excel97-Doc)
        // eFormat == EIF_BIFF8 -> nur Biff8-Stream fuehrt zum Erfolg (nur in Excel97-Docs)
        // eFormat == EIF_BIFF_LE4 -> nur Nicht-Storage-Dateien _koennen_ zum Erfolg fuehren
    virtual FltError ScImportStarCalc10( SvStream&, ScDocument* );
    virtual FltError ScImportDif( SvStream&, ScDocument*, const ScAddress& rInsPos,
                 const CharSet eSrc = RTL_TEXTENCODING_DONTKNOW, sal_uInt32 nDifOption = SC_DIFOPT_EXCEL );
    virtual FltError ScImportRTF( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange );
    virtual FltError ScImportHTML( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange,
                                   double nOutputFactor = 1.0, bool bCalcWidthHeight = true,
                                   SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true );

    virtual ScEEAbsImport *CreateRTFImport( ScDocument* pDoc, const ScRange& rRange );
    virtual ScEEAbsImport *CreateHTMLImport( ScDocument* pDocP, const String& rBaseURL, const ScRange& rRange, bool bCalcWidthHeight );
    virtual String         GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName );

    // various export filters
    virtual FltError ScExportExcel5( SfxMedium&, ScDocument*, ExportFormatExcel eFormat, CharSet eDest );
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScAddress& rOutPos, const CharSet eDest,
                                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL );
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest,
                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL );
    virtual FltError ScExportHTML( SvStream&, const String& rBaseURL, ScDocument*, const ScRange& rRange, const CharSet eDest, bool bAll,
                  const String& rStreamPath, OUString& rNonConvertibleChars );
    virtual FltError ScExportRTF( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest );

    virtual ScOrcusFilters* GetOrcusFilters();
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
