/*************************************************************************
 *
 *  $RCSfile: ftools.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:37:40 $
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

// ============================================================================

#ifndef SC_FTOOLS_HXX
#define SC_FTOOLS_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COLOR_HXX
#include <tools/color.hxx>
#endif

#include <vector>
#include <stack>
#include <algorithm>
#include <memory>


// Global static helpers ======================================================

/** Expands to the size of a STATIC data array. */
#define STATIC_TABLE_SIZE( array )  (sizeof(array)/sizeof(*(array)))
/** Expands to a pointer behind the last element of a STATIC data array (like STL end()). */
#define STATIC_TABLE_END( array )   ((array)+STATIC_TABLE_SIZE(array))

/** Expands to a temporary String, created from an ASCII character array. */
#define CREATE_STRING( ascii )      String( RTL_CONSTASCII_USTRINGPARAM( ascii ) )
/** Expands to a temporary ::rtl::OUString, created from an ASCII character array. */
#define CREATE_OUSTRING( ascii )    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ascii ) )

// Read from bitfields --------------------------------------------------------

// deprecated
inline bool HasFlag( sal_uInt32 nValue, sal_uInt32 nMask )
{ return (nValue & nMask) != 0; }

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


// Modify values on bit-level -------------------------------------------------

/** Rotates rnValue left by nBits bits. */
template< typename Type >
inline void rotate_left( Type& rnValue, sal_uInt8 nBits )
{
    DBG_ASSERT( nBits < sizeof( Type ) * 8, "rotate_left - overflow" );
    rnValue = static_cast< Type >( (rnValue << nBits) | (rnValue >> (sizeof( Type ) * 8 - nBits)) );
}

/** Rotates the lower nWidth bits of rnValue left by nBits bits. */
template< typename Type >
inline void rotate_left( Type& rnValue, sal_uInt8 nBits, sal_uInt8 nWidth )
{
    DBG_ASSERT( (nBits < nWidth) && (nWidth < sizeof( Type ) * 8), "rotate_left - overflow" );
    Type nMask = static_cast< Type >( (1UL << nWidth) - 1 );
    rnValue = static_cast< Type >(
        ((rnValue << nBits) | ((rnValue & nMask) >> (nWidth - nBits))) & nMask );
}

/** Rotates rnValue right by nBits bits. */
template< typename Type >
inline void rotate_right( Type& rnValue, sal_uInt8 nBits )
{
    DBG_ASSERT( nBits < sizeof( Type ) * 8, "rotate_right - overflow" );
    return static_cast< Type >( (rnValue >> nBits) | (rnValue << (sizeof( Type ) * 8 - nBits)) );
}

/** Rotates the lower nWidth bits of rnValue right by nBits bits. */
template< typename Type >
inline void rotate_right( Type& rnValue, sal_uInt8 nBits, sal_uInt8 nWidth )
{
    DBG_ASSERT( (nBits < nWidth) && (nWidth < sizeof( Type ) * 8), "rotate_right - overflow" );
    Type nMask = static_cast< Type >( (1UL << nWidth) - 1 );
    return static_cast< Type >(
        (((rnValue & nMask) >> nBits) | (rnValue << (nWidth - nBits))) & nMask );
}


// ============================================================================

/** Deriving from this class prevents copy construction. */
class ScfNoCopy
{
private:
                                ScfNoCopy( const ScfNoCopy& );
    ScfNoCopy&                  operator=( const ScfNoCopy& );
protected:
    inline                      ScfNoCopy() {}
};


// ----------------------------------------------------------------------------

/** Deriving from this class prevents construction in general. */
class ScfNoInstance : private ScfNoCopy {};


// ============================================================================

class ScStyleSheet;
class ScStyleSheetPool;
class SvStorage;
class SvStorageStreamRef;
class SvStream;

/** Contains static methods used anywhere in the filters. */
class ScfTools : ScfNoInstance
{
public:

// *** common methods ***

    /** Reads a 10-byte-long-double and converts it to double. */
    static double               ReadLongDouble( SvStream& rStrm );
    /** Returns system charset for byte string conversion. */
    static CharSet              GetSystemCharSet();
    /** Returns a string representing the hexadecimal value of nValue. */
    static String               GetHexStr( sal_uInt16 nValue );

    /** Mixes RGB components with given transparence (0x0000 == full nFore ... 0x8000 = full nBack). */
    static sal_uInt8            GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt16 nTrans );
    /** Mixes colors with given transparence (0x0000 == full rFore ... 0x8000 = full rBack). */
    static Color                GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt16 nTrans );

// *** conversion of names ***

    /** Converts a string to a valid Calc sheet name.
        @descr  Sheet names in Calc may contain letters, digits, underscores, and spaces
        (space characters are not allowed at first position). */
    static void                 ConvertToScSheetName( String& rName );
    /** Converts a string to a valid Calc defined name or database range name.
        @descr  Defined names in Calc may contain letters, digits (*), underscores, periods (*),
        colons (*), question marks, and dollar signs.
        (*) = not allowed at first position. */
    static void                 ConvertToScDefinedName( String& rName );

// *** streams and storages ***

    /** Tries to open the stream with the specified name in the passed storage (read-only). */
    static const SvStorageStreamRef OpenStorageStreamRead( SvStorage* pStorage, const String& rStrmName );
    /** Tries to create or open a stream with the specified name in the passed storage (read/write). */
    static const SvStorageStreamRef OpenStorageStreamWrite( SvStorage* pStorage, const String& rStrmName );

// *** style sheet handling ***

    /** Creates and returns a cell style sheet and inserts it into the pool.
        @descr  If the style sheet is already in the pool, another unused style name is used.
        @param bForceName  Controls behaviour, if the style already exists:
        true = Old existing style will be renamed; false = New style gets another name. */
    static ScStyleSheet&        MakeCellStyleSheet(
                                    ScStyleSheetPool& rPool,
                                    const String& rStyleName, bool bForceName );

// *** byte string import operations ***

    /** Reads and returns a zero terminted byte string. */
    static ByteString           ReadCString( SvStream& rStrm );
    /** Reads and returns a zero terminted byte string. */
    inline static String        ReadCString( SvStream& rStrm, CharSet eSrc )
                                    { return String( ReadCString( rStrm ), eSrc ); }

    /** Reads and returns a zero terminted byte string and decreases a stream counter. */
    static ByteString           ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft );
    /** Reads and returns a zero terminted byte string and decreases a stream counter. */
    inline static String        ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft, CharSet eSrc )
                                    { return String( ReadCString( rStrm, rnBytesLeft ), eSrc ); }

    /** Appends a zero terminted byte string. */
    static void                 AppendCString( SvStream& rStrm, ByteString& rString );
    /** Appends a zero terminted byte string. */
    static void                 AppendCString( SvStream& rStrm, String& rString, CharSet eSrc );

// *** HTML table names <-> named range names ***

    /** Returns the built-in range name for an HTML document. */
    static const String&        GetHTMLDocName();
    /** Returns the built-in range name for all HTML tables. */
    static const String&        GetHTMLTablesName();
    /** Returns the built-in range name for an HTML table, specified by table index. */
    static String               GetNameFromHTMLIndex( sal_uInt32 nIndex );
    /** Returns the built-in range name for an HTML table, specified by table name. */
    static String               GetNameFromHTMLName( const String& rTabName );

    /** Returns true, if rSource is the built-in range name for an HTML document. */
    static bool                 IsHTMLDocName( const String& rSource );
    /** Returns true, if rSource is the built-in range name for all HTML tables. */
    static bool                 IsHTMLTablesName( const String& rSource );
    /** Converts a built-in range name to an HTML table name.
        @param rSource  The string to be determined.
        @param rName  The HTML table name.
        @return  true, if conversion was successful. */
    static bool                 GetHTMLNameFromName( const String& rSource, String& rName );

private:
    /** Returns the prefix for table index names. */
    static const String&        GetHTMLIndexPrefix();
    /** Returns the prefix for table names. */
    static const String&        GetHTMLNamePrefix();
};


// Containers =================================================================

typedef ::std::vector< sal_uInt8 >                  ScfUInt8Vec;
typedef ::std::vector< sal_Int16 >                  ScfInt16Vec;
typedef ::std::vector< sal_uInt16 >                 ScfUInt16Vec;
typedef ::std::vector< sal_Int32 >                  ScfInt32Vec;
typedef ::std::vector< sal_uInt32 >                 ScfUInt32Vec;

typedef ::std::stack< sal_uInt8,  ScfUInt8Vec >     ScfUInt8Stack;
typedef ::std::stack< sal_Int16, ScfInt16Vec >      ScfInt16Stack;
typedef ::std::stack< sal_uInt16, ScfUInt16Vec >    ScfUInt16Stack;
typedef ::std::stack< sal_Int32, ScfInt32Vec >      ScfInt32Stack;
typedef ::std::stack< sal_uInt32, ScfUInt32Vec >    ScfUInt32Stack;


// ----------------------------------------------------------------------------

/** Template for a list that owns the contained objects.
    @descr  This list stores pointers to objects and deletes the objects itself
    on destruction. The Clear() method deletes all objects too. */
template< typename Type > class ScfDelList : ScfNoCopy
{
private:
    mutable List                maList;     /// The base container object.

public:
    inline                      ScfDelList( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 ) :
                                    maList( nInitSize, nResize ) {}
    virtual                     ~ScfDelList();

    inline void                 Insert( Type* pObj, sal_uInt32 nIndex ) { if( pObj ) maList.Insert( pObj, nIndex ); }
    inline void                 Append( Type* pObj )                    { if( pObj ) maList.Insert( pObj, LIST_APPEND ); }
    /** Removes the object without deletion. */
    inline Type*                Remove( sal_uInt32 nIndex )             { return static_cast< Type* >( maList.Remove( nIndex ) ); }
    /** Removes and deletes the object. */
    inline void                 Delete( sal_uInt32 nIndex )             { delete Remove( nIndex ); }
    /** Exchanges the contained object with the passed, returns the old. */
    inline Type*                Exchange( Type* pObj, sal_uInt32 nIndex ){ return static_cast< Type* >( maList.Replace( pObj, nIndex ) ); }
    /** Replaces (deletes) the contained object. */
    inline void                 Replace( Type* pObj, sal_uInt32 nIndex ){ delete Exchange( pObj, nIndex ); }

    inline void                 Clear();
    inline sal_uInt32           Count() const                           { return maList.Count(); }
    inline bool                 Empty() const                           { return maList.Count() == 0; }

    inline Type*                GetCurObject() const                    { return static_cast< Type* >( maList.GetCurObject() ); }
    inline sal_uInt32           GetCurPos() const                       { return maList.GetCurPos(); }
    inline Type*                GetObject( sal_uInt32 nIndex ) const    { return static_cast< Type* >( maList.GetObject( nIndex ) ); }

    inline Type*                First() const                           { return static_cast< Type* >( maList.First() ); }
    inline Type*                Last() const                            { return static_cast< Type* >( maList.Last() ); }
    inline Type*                Next() const                            { return static_cast< Type* >( maList.Next() ); }
    inline Type*                Prev() const                            { return static_cast< Type* >( maList.Prev() ); }
};

template< typename Type > ScfDelList< Type >::~ScfDelList()
{
    Clear();
}

template< typename Type > inline void ScfDelList< Type >::Clear()
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
    inline                      ScfDelStack( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 ) :
                                    ScfDelList< Type >( nInitSize, nResize ) {}

    inline void                 Push( Type* pObj )      { Append( pObj ); }
    /** Removes the top object without deletion. */
    inline Type*                Pop()                   { return Remove( Count() - 1 ); }

    inline Type*                Top() const             { return GetObject( Count() - 1 ); }

                                ScfDelList< Type >::Clear;
                                ScfDelList< Type >::Count;
                                ScfDelList< Type >::Empty;
};


// ----------------------------------------------------------------------------

/** List class for sal_uInt16 values.
    @deprecated */
class ScfUInt16List : protected List
{
public:
    inline                      ScfUInt16List() : List() {}
    inline                      ScfUInt16List( const ScfUInt16List& rCopy ) : List( rCopy ) {}

    inline ScfUInt16List&       operator=( const ScfUInt16List& rSource )
                                        { List::operator=( rSource ); return *this; }

                                List::Clear;
                                List::Count;
    inline bool                 Empty() const   { return List::Count() == 0; }

    inline sal_uInt16           First() { return (sal_uInt16)(sal_uInt32) List::First(); }
    inline sal_uInt16           Last()  { return (sal_uInt16)(sal_uInt32) List::Last(); }
    inline sal_uInt16           Next()  { return (sal_uInt16)(sal_uInt32) List::Next(); }
    inline sal_uInt16           Prev()  { return (sal_uInt16)(sal_uInt32) List::Prev(); }

    inline sal_uInt16           GetValue( sal_uInt32 nIndex ) const
                                        { return (sal_uInt16)(sal_uInt32) List::GetObject( nIndex ); }
    inline bool                 Contains( sal_uInt16 nValue ) const
                                        { return List::GetPos( (void*)(sal_uInt32) nValue ) != LIST_ENTRY_NOTFOUND; }

    inline void                 Insert( sal_uInt16 nValue, sal_uInt32 nIndex )
                                        { List::Insert( (void*)(sal_uInt32) nValue, nIndex ); }
    inline void                 Append( sal_uInt16 nValue )
                                        { List::Insert( (void*)(sal_uInt32) nValue, LIST_APPEND ); }
    inline sal_uInt16           Replace( sal_uInt16 nValue, sal_uInt32 nIndex )
                                        { return (sal_uInt16)(sal_uInt32) List::Replace( (void*)(sal_uInt32) nValue, nIndex ); }
    inline sal_uInt16           Remove( sal_uInt32 nIndex )
                                        { return (sal_uInt16)(sal_uInt32) List::Remove( nIndex ); }
};


// ----------------------------------------------------------------------------

/** List class for sal_uInt32 values.
    @deprecated */
class ScfUInt32List : protected List
{
public:
    inline                      ScfUInt32List() : List() {}
    inline                      ScfUInt32List( const ScfUInt32List& rCopy ) : List( rCopy ) {}

    inline ScfUInt32List&       operator=( const ScfUInt32List& rSource )
                                        { List::operator=( rSource ); return *this; }

                                List::Clear;
                                List::Count;
    inline bool                 Empty() const   { return List::Count() == 0; }

    inline sal_uInt32           First() { return (sal_uInt32) List::First(); }
    inline sal_uInt32           Last()  { return (sal_uInt32) List::Last(); }
    inline sal_uInt32           Next()  { return (sal_uInt32) List::Next(); }
    inline sal_uInt32           Prev()  { return (sal_uInt32) List::Prev(); }

    inline sal_uInt32           GetValue( sal_uInt32 nIndex ) const
                                        { return (sal_uInt32) List::GetObject( nIndex ); }
    inline bool                 Contains( sal_uInt32 nValue ) const
                                        { return List::GetPos( (void*) nValue ) != LIST_ENTRY_NOTFOUND; }

    inline void                 Insert( sal_uInt32 nValue, sal_uInt32 nIndex )
                                        { List::Insert( (void*) nValue, nIndex ); }
    inline void                 Append( sal_uInt32 nValue )
                                        { List::Insert( (void*) nValue, LIST_APPEND ); }
    inline sal_uInt32           Replace( sal_uInt32 nValue, sal_uInt32 nIndex )
                                        { return (sal_uInt32) List::Replace( (void*) nValue, nIndex ); }
    inline sal_uInt32           Remove( sal_uInt32 nIndex )
                                        { return (sal_uInt32) List::Remove( nIndex ); }
};


// ============================================================================

#endif

