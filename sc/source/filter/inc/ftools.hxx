/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftools.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-19 13:23:44 $
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

#ifndef SC_FTOOLS_HXX
#define SC_FTOOLS_HXX

#include <vector>
#include <limits>
#include <memory>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

// Common macros ==============================================================

/** Expands to the size of a STATIC data array. */
#define STATIC_TABLE_SIZE( array )  (sizeof(array)/sizeof(*(array)))
/** Expands to a pointer behind the last element of a STATIC data array (like STL end()). */
#define STATIC_TABLE_END( array )   ((array)+STATIC_TABLE_SIZE(array))

/** Expands to a temporary String, created from an ASCII character array. */
#define CREATE_STRING( ascii )      String( RTL_CONSTASCII_USTRINGPARAM( ascii ) )
/** Expands to a temporary ::rtl::OUString, created from an ASCII character array. */
#define CREATE_OUSTRING( ascii )    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ascii ) )

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
template< typename Type, typename ReturnType >
inline ReturnType get_flagvalue( Type nBitField, Type nMask, ReturnType nSet, ReturnType nUnset )
{ return ::get_flag( nBitField, nMask ) ? nSet : nUnset; }

/** Extracts a value from a bit field.
    @descr  Returns in rnRet the data fragment from nBitField, that starts at bit nStartBit
    (0-based, bit 0 is rightmost) with the width of nBitCount. rnRet will be right-aligned (normalized).
    For instance: extract_value( n, 0x4321, 8, 4 ) stores 3 in n (value in bits 8-11). */
template< typename Type, typename ReturnType >
inline void extract_value( ReturnType& rnRet, Type nBitField, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{ rnRet = static_cast< ReturnType >( ((1UL << nBitCount) - 1) & (nBitField >> nStartBit) ); }

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

/** Deriving from this class prevents copy construction. */
class ScfNoCopy
{
private:
                        ScfNoCopy( const ScfNoCopy& );
    ScfNoCopy&          operator=( const ScfNoCopy& );
protected:
    inline              ScfNoCopy() {}
};

// ----------------------------------------------------------------------------

/** Deriving from this class prevents construction in general. */
class ScfNoInstance : private ScfNoCopy {};

// ============================================================================

/** Simple shared pointer (NOT thread-save, but faster than boost::shared_ptr). */
template< typename Type >
class ScfRef
{
    template< typename Type2 > friend class ScfRef;

public:
    typedef Type        element_type;
    typedef ScfRef      this_type;

    inline explicit     ScfRef( element_type* pObj = 0 ) { eat( pObj ); }
    inline /*implicit*/ ScfRef( const this_type& rRef ) { eat( rRef.mpObj, rRef.mpnCount ); }
    template< typename Type2 >
    inline /*implicit*/ ScfRef( const ScfRef< Type2 >& rRef ) { eat( rRef.mpObj, rRef.mpnCount ); }
    inline              ~ScfRef() { rel(); }

    inline void         reset( element_type* pObj = 0 ) { rel(); eat( pObj ); }
    inline this_type&   operator=( const this_type& rRef ) { if( this != &rRef ) { rel(); eat( rRef.mpObj, rRef.mpnCount ); } return *this; }
    template< typename Type2 >
    inline this_type&   operator=( const ScfRef< Type2 >& rRef ) { rel(); eat( rRef.mpObj, rRef.mpnCount ); return *this; }

    inline element_type* get() const { return mpObj; }
    inline bool         is() const { return mpObj != 0; }

    inline element_type* operator->() const { return mpObj; }
    inline element_type& operator*() const { return *mpObj; }

    inline bool         operator!() const { return mpObj == 0; }

private:
    inline void         eat( element_type* pObj, size_t* pnCount = 0 ) { mpObj = pObj; mpnCount = mpObj ? (pnCount ? pnCount : new size_t( 0 )) : 0; if( mpnCount ) ++*mpnCount; }
    inline void         rel() { if( mpnCount && !--*mpnCount ) { DELETEZ( mpObj ); DELETEZ( mpnCount ); } }

private:
    Type*               mpObj;
    size_t*             mpnCount;
};

template< typename Type >
inline bool operator==( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() == rxRef2.get();
}

template< typename Type >
inline bool operator!=( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() != rxRef2.get();
}

template< typename Type >
inline bool operator<( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() < rxRef2.get();
}

template< typename Type >
inline bool operator>( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() > rxRef2.get();
}

template< typename Type >
inline bool operator<=( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() <= rxRef2.get();
}

template< typename Type >
inline bool operator>=( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() >= rxRef2.get();
}

// ============================================================================

class Color;
class SfxPoolItem;
class SfxItemSet;
class ScStyleSheet;
class ScStyleSheetPool;
class SotStorage;
class SotStorageRef;
class SotStorageStreamRef;
class SvStream;

/** Contains static methods used anywhere in the filters. */
class ScfTools : ScfNoInstance
{
public:

// *** common methods *** -----------------------------------------------------

    /** Reads a 10-byte-long-double and converts it to double. */
    static double       ReadLongDouble( SvStream& rStrm );
    /** Returns system text encoding for byte string conversion. */
    static rtl_TextEncoding GetSystemTextEncoding();
    /** Returns a string representing the hexadecimal value of nValue. */
    static String       GetHexStr( sal_uInt16 nValue );

    /** Mixes RGB components with given transparence.
        @param nTrans  Foreground transparence (0x0000 == full nFore ... 0x8000 = full nBack). */
    static sal_uInt8    GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt16 nTrans );
    /** Mixes colors with given transparence.
        @param nTrans  Foreground transparence (0x0000 == full rFore ... 0x8000 = full rBack). */
    static Color        GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt16 nTrans );

// *** conversion of names *** ------------------------------------------------

    /** Converts a string to a valid Calc sheet name.
        @descr  Sheet names in Calc may contain letters, digits, underscores, and spaces
        (space characters are not allowed at first position). */
    static void         ConvertToScSheetName( String& rName );
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
    static bool         CheckItem( const SfxItemSet& rItemSet, USHORT nWhichId, bool bDeep );
    /** Returns true, if the passed item set contains at least one of the items.
        @param pnWhichIds  Zero-terminated array of Which-IDs.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItems( const SfxItemSet& rItemSet, const USHORT* pnWhichIds, bool bDeep );

    /** Puts the item into the passed item set.
        @descr  The item will be put into the item set, if bSkipPoolDef is false,
        or if the item differs from the default pool item.
        @param rItemSet  The destination item set.
        @param rItem  The item to put into the item set.
        @param nWhichId  The Which-ID to set with the item.
        @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
    static void         PutItem(
                            SfxItemSet& rItemSet, const SfxPoolItem& rItem,
                            USHORT nWhichId, bool bSkipPoolDef );

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
                            const String& rStyleName, bool bForceName );
    /** Creates and returns a page style sheet and inserts it into the pool.
        @descr  If the style sheet is already in the pool, another unused style name is used.
        @param bForceName  Controls behaviour, if the style already exists:
        true = Old existing style will be renamed; false = New style gets another name. */
    static ScStyleSheet& MakePageStyleSheet(
                            ScStyleSheetPool& rPool,
                            const String& rStyleName, bool bForceName );

// *** byte string import operations *** --------------------------------------

    /** Reads and returns a zero terminted byte string. */
    static ByteString   ReadCString( SvStream& rStrm );
    /** Reads and returns a zero terminted byte string. */
    inline static String ReadCString( SvStream& rStrm, rtl_TextEncoding eTextEnc )
                            { return String( ReadCString( rStrm ), eTextEnc ); }

    /** Reads and returns a zero terminted byte string and decreases a stream counter. */
    static ByteString   ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft );
    /** Reads and returns a zero terminted byte string and decreases a stream counter. */
    inline static String ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft, rtl_TextEncoding eTextEnc )
                            { return String( ReadCString( rStrm, rnBytesLeft ), eTextEnc ); }

    /** Appends a zero terminted byte string. */
    static void         AppendCString( SvStream& rStrm, ByteString& rString );
    /** Appends a zero terminted byte string. */
    static void         AppendCString( SvStream& rStrm, String& rString, rtl_TextEncoding eTextEnc );

// *** HTML table names <-> named range names *** -----------------------------

    /** Returns the built-in range name for an HTML document. */
    static const String& GetHTMLDocName();
    /** Returns the built-in range name for all HTML tables. */
    static const String& GetHTMLTablesName();
    /** Returns the built-in range name for an HTML table, specified by table index. */
    static String       GetNameFromHTMLIndex( sal_uInt32 nIndex );
    /** Returns the built-in range name for an HTML table, specified by table name. */
    static String       GetNameFromHTMLName( const String& rTabName );

    /** Returns true, if rSource is the built-in range name for an HTML document. */
    static bool         IsHTMLDocName( const String& rSource );
    /** Returns true, if rSource is the built-in range name for all HTML tables. */
    static bool         IsHTMLTablesName( const String& rSource );
    /** Converts a built-in range name to an HTML table name.
        @param rSource  The string to be determined.
        @param rName  The HTML table name.
        @return  true, if conversion was successful. */
    static bool         GetHTMLNameFromName( const String& rSource, String& rName );

private:
    /** Returns the prefix for table index names. */
    static const String& GetHTMLIndexPrefix();
    /** Returns the prefix for table names. */
    static const String& GetHTMLNamePrefix();
};

// Containers =================================================================

typedef ::std::vector< sal_uInt8 >                  ScfUInt8Vec;
typedef ::std::vector< sal_Int16 >                  ScfInt16Vec;
typedef ::std::vector< sal_uInt16 >                 ScfUInt16Vec;
typedef ::std::vector< sal_Int32 >                  ScfInt32Vec;
typedef ::std::vector< sal_uInt32 >                 ScfUInt32Vec;
typedef ::std::vector< sal_Int64 >                  ScfInt64Vec;
typedef ::std::vector< sal_uInt64 >                 ScfUInt64Vec;
typedef ::std::vector< String >                     ScfStringVec;

// ----------------------------------------------------------------------------

/** Template for a list that owns the contained objects.
    @descr  This list stores pointers to objects and deletes the objects itself
    on destruction. The Clear() method deletes all objects too. */
template< typename Type > class ScfDelList
{
public:
    inline explicit     ScfDelList( USHORT nInitSize = 16, USHORT nResize = 16 ) :
                            maList( nInitSize, nResize ) {}
    /** Creates a deep copy of the passed list (copy-constructs all contained objects). */
    inline explicit     ScfDelList( const ScfDelList& rSrc ) { *this = rSrc; }
    virtual             ~ScfDelList();

    /** Creates a deep copy of the passed list (copy-constructs all contained objects). */
    ScfDelList&         operator=( const ScfDelList& rSrc );

    inline void         Insert( Type* pObj, ULONG nIndex )      { if( pObj ) maList.Insert( pObj, nIndex ); }
    inline void         Append( Type* pObj )                    { if( pObj ) maList.Insert( pObj, LIST_APPEND ); }
    /** Removes the object without deletion. */
    inline Type*        Remove( ULONG nIndex )                  { return static_cast< Type* >( maList.Remove( nIndex ) ); }
    /** Removes and deletes the object. */
    inline void         Delete( ULONG nIndex )                  { delete Remove( nIndex ); }
    /** Exchanges the contained object with the passed, returns the old. */
    inline Type*        Exchange( Type* pObj, ULONG nIndex )    { return static_cast< Type* >( maList.Replace( pObj, nIndex ) ); }
    /** Replaces (deletes) the contained object. */
    inline void         Replace( Type* pObj, ULONG nIndex )     { delete Exchange( pObj, nIndex ); }

    void                Clear();
    inline ULONG        Count() const                           { return maList.Count(); }
    inline bool         Empty() const                           { return maList.Count() == 0; }

    inline Type*        GetCurObject() const                    { return static_cast< Type* >( maList.GetCurObject() ); }
    inline ULONG        GetCurPos() const                       { return maList.GetCurPos(); }
    inline Type*        GetObject( sal_uInt32 nIndex ) const    { return static_cast< Type* >( maList.GetObject( nIndex ) ); }

    inline Type*        First() const                           { return static_cast< Type* >( maList.First() ); }
    inline Type*        Last() const                            { return static_cast< Type* >( maList.Last() ); }
    inline Type*        Next() const                            { return static_cast< Type* >( maList.Next() ); }
    inline Type*        Prev() const                            { return static_cast< Type* >( maList.Prev() ); }

private:
    mutable List        maList;     /// The base container object.
};

template< typename Type > ScfDelList< Type >& ScfDelList< Type >::operator=( const ScfDelList& rSrc )
{
    Clear();
    for( const Type* pObj = rSrc.First(); pObj; pObj = rSrc.Next() )
        Append( new Type( *pObj ) );
    return *this;
}

template< typename Type > ScfDelList< Type >::~ScfDelList()
{
    Clear();
}

template< typename Type > void ScfDelList< Type >::Clear()
{
    for( Type* pObj = First(); pObj; pObj = Next() )
        delete pObj;
    maList.Clear();
}

// ----------------------------------------------------------------------------

/** Template for a stack that owns the contained objects.
    @descr  This stack stores pointers to objects and deletes the objects
    itself on destruction. The Clear() method deletes all objects too.
    The Pop() method removes the top object from stack without deletion. */
template< typename Type >
class ScfDelStack : private ScfDelList< Type >
{
public:
    inline              ScfDelStack( USHORT nInitSize = 16, USHORT nResize = 16 ) :
                            ScfDelList< Type >( nInitSize, nResize ) {}

    inline void         Push( Type* pObj )      { Append( pObj ); }
    /** Removes the top object without deletion. */
    inline Type*        Pop()                   { return Remove( Count() - 1 ); }

    inline Type*        Top() const             { return GetObject( Count() - 1 ); }

                        ScfDelList< Type >::Clear;
                        ScfDelList< Type >::Count;
                        ScfDelList< Type >::Empty;
};

// ============================================================================

#endif

