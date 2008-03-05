/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dumperbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:53:05 $
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

#ifndef OOX_DUMP_DUMPERBASE_HXX
#define OOX_DUMP_DUMPERBASE_HXX

#include <math.h>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "oox/helper/helper.hxx"
#include "oox/helper/storagebase.hxx"
#include "oox/helper/binaryinputstream.hxx"

#define OOX_INCLUDE_DUMPER (OSL_DEBUG_LEVEL > 0)

#if OOX_INCLUDE_DUMPER

namespace com { namespace sun { namespace star {
    namespace util { struct DateTime; }
    namespace io { class XInputStream; }
    namespace io { class XTextInputStream; }
    namespace io { class XOutputStream; }
    namespace io { class XTextOutputStream; }
} } }

namespace oox { class BinaryInputStream; }

namespace oox { namespace core {
    class FilterBase;
} }

namespace oox {
namespace dump {

// ============================================================================

#define OOX_DUMP_UNUSED                     "unused"
#define OOX_DUMP_UNKNOWN                    "?unknown"

#define OOX_DUMP_ERRASCII( ascii )          "?err:" ascii
#define OOX_DUMP_ERRSTRING( ascii )         CREATE_OUSTRING( OOX_DUMP_ERRASCII( ascii ) )

#define OOX_DUMP_ERR_NOMAP                  OOX_DUMP_ERRSTRING( "no-map" )
#define OOX_DUMP_ERR_NONAME                 OOX_DUMP_ERRSTRING( "no-name" )
#define OOX_DUMP_ERR_STREAM                 OOX_DUMP_ERRSTRING( "stream-error" )

const sal_Unicode OOX_DUMP_STRQUOTE         = '\'';
const sal_Unicode OOX_DUMP_FMLASTRQUOTE     = '"';
const sal_Unicode OOX_DUMP_ADDRABS          = '$';
const sal_Unicode OOX_DUMP_R1C1ROW          = 'R';
const sal_Unicode OOX_DUMP_R1C1COL          = 'C';
const sal_Unicode OOX_DUMP_R1C1OPEN         = '[';
const sal_Unicode OOX_DUMP_R1C1CLOSE        = ']';
const sal_Unicode OOX_DUMP_RANGESEP         = ':';
const sal_Unicode OOX_DUMP_BASECLASS        = 'B';
const sal_Unicode OOX_DUMP_FUNCSEP          = ',';
const sal_Unicode OOX_DUMP_LISTSEP          = ',';
const sal_Unicode OOX_DUMP_TABSEP           = '!';
const sal_Unicode OOX_DUMP_ARRAYSEP         = ';';
const sal_Unicode OOX_DUMP_EMPTYVALUE       = '~';
const sal_Unicode OOX_DUMP_CMDPROMPT        = '?';
const sal_Unicode OOX_DUMP_PLACEHOLDER      = '\x01';

typedef ::std::vector< ::rtl::OUString >    OUStringVector;
typedef ::std::vector< sal_Int64 >          Int64Vector;

// ============================================================================
// ============================================================================

/** Static helper functions for system file and stream access. */
class InputOutputHelper
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >      XInputStreamRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextInputStream >  XTextInputStreamRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >     XOutputStreamRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextOutputStream > XTextOutputStreamRef;

public:
    // file names -------------------------------------------------------------

    static ::rtl::OUString convertFileNameToUrl( const ::rtl::OUString& rFileName );
    static sal_Int32    getFileNamePos( const ::rtl::OUString& rFileUrl );
    static ::rtl::OUString getFileNameExtension( const ::rtl::OUString& rFileUrl );

    // input streams ----------------------------------------------------------

    static XInputStreamRef openInputStream( const ::rtl::OUString& rFileName );
    static XTextInputStreamRef openTextInputStream( const XInputStreamRef& rxInStrm, const ::rtl::OUString& rEncoding );
    static XTextInputStreamRef openTextInputStream( const ::rtl::OUString& rFileName, const ::rtl::OUString& rEncoding );

    // output streams ---------------------------------------------------------

    static XOutputStreamRef openOutputStream( const ::rtl::OUString& rFileName );
    static XTextOutputStreamRef openTextOutputStream( const XOutputStreamRef& rxOutStrm, const ::rtl::OUString& rEncoding );
    static XTextOutputStreamRef openTextOutputStream( const ::rtl::OUString& rFileName, const ::rtl::OUString& rEncoding );
};

// ============================================================================
// ============================================================================

/** Specifiers for atomic data types. */
enum DataType
{
    DATATYPE_VOID,              /// No data type.
    DATATYPE_INT8,              /// Signed 8-bit integer.
    DATATYPE_UINT8,             /// Unsigned 8-bit integer.
    DATATYPE_INT16,             /// Signed 16-bit integer.
    DATATYPE_UINT16,            /// Unsigned 16-bit integer.
    DATATYPE_INT32,             /// Signed 32-bit integer.
    DATATYPE_UINT32,            /// Unsigned 32-bit integer.
    DATATYPE_INT64,             /// Signed 64-bit integer.
    DATATYPE_UINT64,            /// Unsigned 64-bit integer.
    DATATYPE_FLOAT,             /// Floating-point, single precision.
    DATATYPE_DOUBLE             /// Floating-point, double precision.
};

// ----------------------------------------------------------------------------

/** Specifiers for the output format of values. */
enum FormatType
{
    FORMATTYPE_NONE,            /// No numeric format (e.g. show name only).
    FORMATTYPE_DEC,             /// Decimal.
    FORMATTYPE_HEX,             /// Hexadecimal.
    FORMATTYPE_BIN,             /// Binary.
    FORMATTYPE_FIX,             /// Fixed-point.
    FORMATTYPE_BOOL             /// Boolean ('true' or 'false').
};

// ----------------------------------------------------------------------------

/** Describes the output format of a data item.

    Data items are written in the following format:

    <NAME>=<VALUE>=<NAME-FROM-LIST>

    NAME is the name of the data item. The name is contained in the members
    maItemName. If the name is empty, only the value is written (without a
    leading equality sign).

    VALUE is the numeric value of the data item. Its format is dependent on the
    output format given in the member meFmtType. If the format type is
    FORMATTYPE_NONE, no value is written.

    NAME-FROM-LIST is a symbolic name for the current value of the data item.
    Various types of name lists produce different names for values, which can
    be used for enumerations or names for single bits in bitfields (see class
    NameListBase and derived classes). The name of the list is given in the
    member maListName. If it is empty, no name is written for the value.
 */
struct ItemFormat
{
    DataType            meDataType;         /// Data type of the item.
    FormatType          meFmtType;          /// Output format for the value.
    ::rtl::OUString     maItemName;         /// Name of the item.
    ::rtl::OUString     maListName;         /// Name of a name list to be used for this item.

    explicit            ItemFormat();

    void                set( DataType eDataType, FormatType eFmtType, const ::rtl::OUString& rItemName );
    void                set( DataType eDataType, FormatType eFmtType, const ::rtl::OUString& rItemName, const ::rtl::OUString& rListName );

    /** Initializes the struct from a vector of strings containing the item format.

        The vector must contain at least 2 strings. The struct is filled from
        the strings in the vector in the following order:
        1) Data type (one of: [u]int8, [u]int16, [u]int32, [u]int64, float, double).
        2) Format type (one of: dec, hex, bin, fix, bool, unused, unknown).
        3) Item name (optional).
        4) Name list name (optional).

        @return  Iterator pointing to the first unhandled string.
     */
    OUStringVector::const_iterator parse( const OUStringVector& rFormatVec );

    /** Initializes the struct from a string containing the item format.

        The string must have the following format:
        DATATYPE,FORMATTYPE[,ITEMNAME[,LISTNAME]]

        DATATYPE is the data type of the item (see above for possible values).
        FORMATTYPE is the format type of the item (see above for possible values).
        ITEMNAME is the name of the item (optional).
        LISTNAME is the name of a name list (optional).

        @return  List containing remaining unhandled format strings.
     */
    OUStringVector      parse( const ::rtl::OUString& rFormatStr );
};

// ============================================================================
// ============================================================================

struct Address
{
    sal_Int32           mnCol;
    sal_Int32           mnRow;
    inline explicit     Address() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit     Address( sal_Int32 nCol, sal_Int32 nRow ) : mnCol( nCol ), mnRow( nRow ) {}
};

// ----------------------------------------------------------------------------

struct Range
{
    Address             maFirst;
    Address             maLast;
    inline explicit     Range() {}
};

// ----------------------------------------------------------------------------

typedef ::std::vector< Range > RangeList;

// ============================================================================

struct TokenAddress : public Address
{
    bool                mbRelCol;
    bool                mbRelRow;
    inline explicit     TokenAddress() : mbRelCol( false ), mbRelRow( false ) {}
};

// ----------------------------------------------------------------------------

struct TokenRange
{
    TokenAddress        maFirst;
    TokenAddress        maLast;
    inline explicit     TokenRange() {}
};

// ============================================================================
// ============================================================================

/** Static helper functions for formatted output to strings. */
class StringHelper
{
public:
    // append string to string ------------------------------------------------

    static void         appendChar( ::rtl::OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount = 1 );
    static void         appendString( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );

    // append decimal ---------------------------------------------------------

    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_uInt8  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_Int8   nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_uInt16 nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_Int16  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_uInt32 nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_Int32  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_uInt64 nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, sal_Int64  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( ::rtl::OUStringBuffer& rStr, double     fData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );

    // append hexadecimal -----------------------------------------------------

    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_uInt8  nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_Int8   nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_Int16  nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_Int32  nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_uInt64 nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, sal_Int64  nData, bool bPrefix = true );
    static void         appendHex( ::rtl::OUStringBuffer& rStr, double     fData, bool bPrefix = true );

    // append shortened hexadecimal -------------------------------------------

    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_uInt8  nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_Int8   nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_Int16  nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_Int32  nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_uInt64 nData, bool bPrefix = true );
    static void         appendShortHex( ::rtl::OUStringBuffer& rStr, sal_Int64  nData, bool bPrefix = true );

    // append binary ----------------------------------------------------------

    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_uInt8  nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_Int8   nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_uInt16 nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_Int16  nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_uInt32 nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_Int32  nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_uInt64 nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, sal_Int64  nData, bool bDots = true );
    static void         appendBin( ::rtl::OUStringBuffer& rStr, double     fData, bool bDots = true );

    // append fixed-point decimal ---------------------------------------------

    template< typename Type >
    static void         appendFix( ::rtl::OUStringBuffer& rStr, Type nData, sal_Int32 nWidth = 0 );

    // append formatted value -------------------------------------------------

    static void         appendBool( ::rtl::OUStringBuffer& rStr, bool bData );
    template< typename Type >
    static void         appendValue( ::rtl::OUStringBuffer& rStr, Type nData, FormatType eFmtType );

    // append columns, rows, addresses ----------------------------------------

    static void         appendAddrCol( ::rtl::OUStringBuffer& rStr, sal_Int32 nCol, bool bRel );
    static void         appendAddrRow( ::rtl::OUStringBuffer& rStr, sal_Int32 nRow, bool bRel );
    static void         appendAddrName( ::rtl::OUStringBuffer& rStr, sal_Unicode cPrefix, sal_Int32 nColRow, bool bRel );

    static void         appendAddress( ::rtl::OUStringBuffer& rStr, const Address& rPos );
    static void         appendRange( ::rtl::OUStringBuffer& rStr, const Range& rRange );
    static void         appendRangeList( ::rtl::OUStringBuffer& rStr, const RangeList& rRanges );

    static void         appendAddress( ::rtl::OUStringBuffer& rStr, const TokenAddress& rPos, bool bR1C1 );
    static void         appendRange( ::rtl::OUStringBuffer& rStr, const TokenRange& rRange, bool bR1C1 );

    // encoded text output ----------------------------------------------------

    static void         appendCChar( ::rtl::OUStringBuffer& rStr, sal_Unicode cChar, bool bPrefix = true );
    static void         appendEncChar( ::rtl::OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount = 1, bool bPrefix = true );
    static void         appendEncString( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rData, bool bPrefix = true );

    // token list -------------------------------------------------------------

    static void         appendToken( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rToken, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    static void         appendToken( ::rtl::OUStringBuffer& rStr, sal_Int64 nToken, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    static void         prependToken( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rToken, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    static void         prependToken( ::rtl::OUStringBuffer& rStr, sal_Int64 nToken, sal_Unicode cSep = OOX_DUMP_LISTSEP );

    static void         appendIndex( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rIdx );
    static void         appendIndex( ::rtl::OUStringBuffer& rStr, sal_Int64 nIdx );
    static void         appendIndexedText( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rData, const ::rtl::OUString& rIdx );
    static void         appendIndexedText( ::rtl::OUStringBuffer& rStr, const ::rtl::OUString& rData, sal_Int64 nIdx );

    static ::rtl::OUString getToken( const ::rtl::OUString& rData, sal_Int32& rnPos, sal_Unicode cSep = OOX_DUMP_LISTSEP );

    /** Encloses the passed string with the passed characters. Uses cOpen, if cClose is NUL. */
    static void         enclose( ::rtl::OUStringBuffer& rStr, sal_Unicode cOpen, sal_Unicode cClose = '\0' );

    // string conversion ------------------------------------------------------

    static ::rtl::OUString trimSpaces( const ::rtl::OUString& rStr );

    static ::rtl::OString convertToUtf8( const ::rtl::OUString& rStr );
    static DataType     convertToDataType( const ::rtl::OUString& rStr );
    static FormatType   convertToFormatType( const ::rtl::OUString& rStr );

    static bool         convertFromDec( sal_Int64& ornData, const ::rtl::OUString& rData );
    static bool         convertFromHex( sal_Int64& ornData, const ::rtl::OUString& rData );

    static bool         convertStringToInt( sal_Int64& ornData, const ::rtl::OUString& rData );
    static bool         convertStringToDouble( double& orfData, const ::rtl::OUString& rData );
    static bool         convertStringToBool( const ::rtl::OUString& rData );

    // string to list conversion ----------------------------------------------

    static void         convertStringToStringList( OUStringVector& orVec, const ::rtl::OUString& rData, bool bIgnoreEmpty );
    static void         convertStringToIntList( Int64Vector& orVec, const ::rtl::OUString& rData, bool bIgnoreEmpty );
};

// ----------------------------------------------------------------------------

template< typename Type >
void StringHelper::appendFix( ::rtl::OUStringBuffer& rStr, Type nData, sal_Int32 nWidth )
{
    appendDec( rStr, static_cast< double >( nData ) / pow( 2.0, 4.0 * sizeof( Type ) ), nWidth );
}

template< typename Type >
void StringHelper::appendValue( ::rtl::OUStringBuffer& rStr, Type nData, FormatType eFmtType )
{
    switch( eFmtType )
    {
        case FORMATTYPE_DEC:        appendDec( rStr, nData );   break;
        case FORMATTYPE_HEX:        appendHex( rStr, nData );   break;
        case FORMATTYPE_BIN:        appendBin( rStr, nData );   break;
        case FORMATTYPE_FIX:        appendFix( rStr, nData );   break;
        case FORMATTYPE_BOOL:       appendBool( rStr, nData );  break;
        default:;
    }
}

// ============================================================================

class StringWrapper
{
public:
    inline              StringWrapper() {}
    inline /*implicit*/ StringWrapper( const ::rtl::OUString& rStr ) : maStr( rStr ) {}
    inline /*implicit*/ StringWrapper( const sal_Char* pcStr ) : maStr( ::rtl::OUString::createFromAscii( pcStr ? pcStr : "" ) ) {}
    inline /*implicit*/ StringWrapper( sal_Unicode cChar ) : maStr( cChar ) {}

    inline bool         isEmpty() const { return maStr.getLength() == 0; }
    inline const ::rtl::OUString& getString() const { return maStr; }

private:
    ::rtl::OUString     maStr;
};

// ============================================================================
// ============================================================================

/** Stack to create a human readable formula string from a UPN token array. */
class FormulaStack
{
public:
    explicit            FormulaStack();

    inline const ::rtl::OUString& getFormulaString() const { return getString( maFmlaStack ); }
    inline const ::rtl::OUString& getClassesString() const { return getString( maClassStack ); }

    void                pushOperand( const StringWrapper& rOp, const ::rtl::OUString& rTokClass );
    void                pushOperand( const StringWrapper& rOp );
    void                pushUnaryOp( const StringWrapper& rLOp, const StringWrapper& rROp );
    void                pushBinaryOp( const StringWrapper& rOp );
    void                pushFuncOp( const StringWrapper& rFunc, const ::rtl::OUString& rTokClass, sal_uInt8 nParamCount );

    inline void         setError() { mbError = true; }
    void                replaceOnTop( const ::rtl::OUString& rOld, const ::rtl::OUString& rNew );

private:
    typedef ::std::stack< ::rtl::OUString > StringStack;

    inline bool         check( bool bCond ) { return (mbError |= !bCond) == false; }

    const ::rtl::OUString& getString( const StringStack& rStack ) const;
    void                pushUnaryOp( StringStack& rStack, const ::rtl::OUString& rLOp, const ::rtl::OUString& rROp );
    void                pushBinaryOp( StringStack& rStack, const ::rtl::OUString& rOp );
    void                pushFuncOp( StringStack& rStack, const ::rtl::OUString& rOp, sal_uInt8 nParamCount );

private:
    StringStack         maFmlaStack;
    StringStack         maClassStack;
    bool                mbError;
};

// ============================================================================
// ============================================================================

class Base;
typedef ::boost::shared_ptr< Base > BaseRef;

/** Base class for all dumper classes.

    Derived classes implement the virtual function implIsValid(). It should
    check all members the other functions rely on. If the function
    implIsValid() returns true, all references and pointers can be used without
    further checking.

    Overview of all classes in this header file based on this Base class:

    Base
    |
    +---->  NameListBase
    |       |
    |       +---->  ConstList  ------>  MultiList
    |       +---->  FlagsList  ------>  CombiList
    |       +---->  UnitConverter
    |
    +---->  SharedConfigData
    +---->  Config
    |
    +---->  Input  ------>  BinaryInput
    +---->  Output
    |
    +---->  StorageIterator
    |
    +---->  ObjectBase
            |
            +---->  RootStorageObjectBase
            |
            +---->  OutputObjectBase
            |       |
            |       +---->  InputObjectBase
            |               |
            |               +---->  InputStreamObject
            |               |
            |               +---->  RecordHeaderImplBase
            |                       |
            |                       +---->  RecordHeaderBase<>
            |
            +---->  DumperBase
 */
class Base
{
public:
    virtual             ~Base();

    inline bool         isValid() const { return implIsValid(); }
    inline static bool  isValid( BaseRef xBase ) { return xBase.get() && xBase->isValid(); }

protected:
    inline explicit     Base() {}

    virtual bool        implIsValid() const = 0;
};

// ============================================================================
// ============================================================================

typedef ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextInputStream > ConfigInputStreamRef;

class ConfigItemBase
{
public:
    virtual             ~ConfigItemBase();
    void                readConfigBlock( const ConfigInputStreamRef& rxStrm );

protected:
    inline explicit     ConfigItemBase() {}

    virtual void        implProcessConfigItemStr(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );

    virtual void        implProcessConfigItemInt(
                            const ConfigInputStreamRef& rxStrm,
                            sal_Int64 nKey,
                            const ::rtl::OUString& rData );

    void                readConfigBlockContents(
                            const ConfigInputStreamRef& rxStrm );

private:
    enum LineType { LINETYPE_DATA, LINETYPE_END };

    LineType            readConfigLine(
                            const ConfigInputStreamRef& rxStrm,
                            ::rtl::OUString& orKey,
                            ::rtl::OUString& orData ) const;

    LineType            readConfigLine(
                            const ConfigInputStreamRef& rxStrm ) const;

    void                processConfigItem(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );
};

// ============================================================================

class SharedConfigData;
class Config;

class NameListBase;
typedef ::boost::shared_ptr< NameListBase > NameListRef;

/** Base class of all classes providing names for specific values (name lists).

    The idea is to provide a unique interfase for all different methods to
    write specific names for any values. This can be enumerations (dedicated
    names for a subset of values), or names for bits in bit fields. Classes
    derived from this base class implement the specific behaviour for the
    desired purpose.
 */
class NameListBase : public Base, public ConfigItemBase
{
public:
    typedef ::std::map< sal_Int64, ::rtl::OUString >    OUStringMap;
    typedef OUStringMap::const_iterator                 const_iterator;

public:
    virtual             ~NameListBase();

    /** Sets a name for the specified key. */
    void                setName( sal_Int64 nKey, const StringWrapper& rNameWrp );

    /** Include all names of the passed list. */
    void                includeList( NameListRef xList );

    /** Returns true, if the map contains an entry for the passed key. */
    template< typename Type >
    inline bool         hasName( Type nKey ) const
        { return maMap.count( static_cast< sal_Int64 >( nKey ) ) != 0; }

    /** Returns the name for the passed key. */
    template< typename Type >
    inline ::rtl::OUString getName( const Config& rCfg, Type nKey ) const
        { return implGetName( rCfg, static_cast< sal_Int64 >( nKey ) ); }

    /** Returns a display name for the passed double value. */
    inline ::rtl::OUString getName( const Config& rCfg, double fValue ) const
        { return implGetNameDbl( rCfg, fValue ); }

    /** Returns a map iterator pointing to the first contained name. */
    inline const_iterator begin() const { return maMap.begin(); }
    /** Returns a map iterator pointing one past the last contained name. */
    inline const_iterator end() const { return maMap.end(); }

protected:
    inline explicit     NameListBase( const SharedConfigData& rCfgData ) : mrCfgData( rCfgData ) {}

    virtual bool        implIsValid() const;

    virtual void        implProcessConfigItemStr(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );

    virtual void        implProcessConfigItemInt(
                            const ConfigInputStreamRef& rxStrm,
                            sal_Int64 nKey,
                            const ::rtl::OUString& rData );

    /** Derived classes set the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const ::rtl::OUString& rName ) = 0;
    /** Derived classes generate and return the name for the passed key. */
    virtual ::rtl::OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const = 0;
    /** Derived classes generate and return the name for the passed double value. */
    virtual ::rtl::OUString implGetNameDbl( const Config& rCfg, double fValue ) const = 0;
    /** Derived classes insert all names and other settings from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList ) = 0;

    /** Inserts the passed name into the internal map. */
    void                insertRawName( sal_Int64 nKey, const ::rtl::OUString& rName );
    /** Returns the name for the passed key, or 0, if nothing found. */
    const ::rtl::OUString* findRawName( sal_Int64 nKey ) const;

private:
    /** Includes name lists, given in a comma separated list of names of the lists. */
    void                include( const ::rtl::OUString& rListKeys );
    /** Excludes names from the list, given in a comma separated list of their keys. */
    void                exclude( const ::rtl::OUString& rKeys );

private:
    OUStringMap         maMap;
    const SharedConfigData& mrCfgData;
};

// ============================================================================

class ConstList : public NameListBase
{
public:
    explicit            ConstList( const SharedConfigData& rCfgData );

    /** Sets a default name for unknown keys. */
    inline void         setDefaultName( const StringWrapper& rDefName ) { maDefName = rDefName.getString(); }
    /** Enables or disables automatic quotation of returned names. */
    inline void         setQuoteNames( bool bQuoteNames ) { mbQuoteNames = bQuoteNames; }

protected:
    virtual void        implProcessConfigItemStr(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );

    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const ::rtl::OUString& rName );
    /** Returns the name for the passed key, or the default name, if key is not contained. */
    virtual ::rtl::OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Returns the name for the passed double value. */
    virtual ::rtl::OUString implGetNameDbl( const Config& rCfg, double fValue ) const;
    /** Inserts all names from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList );

private:
    ::rtl::OUString     maDefName;
    bool                mbQuoteNames;
};

// ============================================================================

class MultiList : public ConstList
{
public:
    explicit            MultiList( const SharedConfigData& rCfgData );

    void                setNamesFromVec( sal_Int64 nStartKey, const OUStringVector& rNames );

protected:
    virtual void        implProcessConfigItemStr(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );

    virtual void        implSetName( sal_Int64 nKey, const ::rtl::OUString& rName );

private:
    void                insertNames( sal_Int64 nStartKey, const ::rtl::OUString& rData );

private:
    bool                mbIgnoreEmpty;
};

// ============================================================================

class FlagsList : public NameListBase
{
public:
    explicit            FlagsList( const SharedConfigData& rCfgData );

    /** Sets flags to be ignored on output. */
    template< typename Type >
    inline void         setIgnoreFlags( Type nIgnore )
                            { mnIgnore = static_cast< sal_Int64 >( nIgnore ); }

protected:
    virtual void        implProcessConfigItemStr(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );

    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const ::rtl::OUString& rName );
    /** Returns the name for the passed key. */
    virtual ::rtl::OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Returns the name for the passed double value. */
    virtual ::rtl::OUString implGetNameDbl( const Config& rCfg, double fValue ) const;
    /** Inserts all flags from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList );

private:
    sal_Int64           mnIgnore;
};

// ============================================================================

class CombiList : public FlagsList
{
public:
    explicit            CombiList( const SharedConfigData& rCfgData );

protected:
    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const ::rtl::OUString& rName );
    /** Returns the name for the passed key. */
    virtual ::rtl::OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Inserts all flags from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList );

private:
    struct ExtItemFormat : public ItemFormat
    {
        bool                mbShiftValue;
        inline explicit     ExtItemFormat() : mbShiftValue( true ) {}
    };
    typedef ::std::map< sal_Int64, ExtItemFormat > ExtItemFormatMap;
    ExtItemFormatMap    maFmtMap;
};

// ============================================================================

class UnitConverter : public NameListBase
{
public:
    explicit            UnitConverter( const SharedConfigData& rCfgData );

    inline void         setUnitName( const StringWrapper& rUnitName ) { maUnitName = rUnitName.getString(); }
    inline void         setFactor( double fFactor ) { mfFactor = fFactor; }

protected:
    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const ::rtl::OUString& rName );
    /** Returns the converted value with appended unit name. */
    virtual ::rtl::OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Returns the converted value with appended unit name. */
    virtual ::rtl::OUString implGetNameDbl( const Config& rCfg, double fValue ) const;
    /** Empty implementation. */
    virtual void        implIncludeList( const NameListBase& rList );

private:
    ::rtl::OUString     maUnitName;
    double              mfFactor;
};

// ============================================================================

class NameListWrapper
{
public:
    inline              NameListWrapper() {}
    inline /*implicit*/ NameListWrapper( const ::rtl::OUString& rListName ) : maNameWrp( rListName ) {}
    inline /*implicit*/ NameListWrapper( const sal_Char* pcListName ) : maNameWrp( pcListName ) {}
    inline /*implicit*/ NameListWrapper( NameListRef xList ) : mxList( xList ) {}

    inline bool         isEmpty() const { return !mxList && maNameWrp.isEmpty(); }
    NameListRef         getNameList( const Config& rCfg ) const;

private:
    StringWrapper       maNameWrp;
    mutable NameListRef mxList;
};

static const NameListWrapper NO_LIST;

// ============================================================================
// ============================================================================

class SharedConfigData : public Base, public ConfigItemBase
{
public:
    explicit            SharedConfigData( const ::rtl::OUString& rFileName );
    virtual             ~SharedConfigData();

    void                setOption( const ::rtl::OUString& rKey, const ::rtl::OUString& rData );
    const ::rtl::OUString* getOption( const ::rtl::OUString& rKey ) const;

    template< typename ListType >
    ::boost::shared_ptr< ListType > createNameList( const ::rtl::OUString& rListName );
    void                setNameList( const ::rtl::OUString& rListName, NameListRef xList );
    void                eraseNameList( const ::rtl::OUString& rListName );
    NameListRef         getNameList( const ::rtl::OUString& rListName ) const;

protected:
    inline explicit     SharedConfigData() : mbLoaded( false ) {}
    void                construct( const ::rtl::OUString& rFileName );

    virtual bool        implIsValid() const;
    virtual void        implProcessConfigItemStr(
                            const ConfigInputStreamRef& rxStrm,
                            const ::rtl::OUString& rKey,
                            const ::rtl::OUString& rData );

private:
    bool                readConfigFile( const ::rtl::OUString& rFileUrl );
    template< typename ListType >
    void                readNameList( const ConfigInputStreamRef& rxStrm, const ::rtl::OUString& rListName );
    void                createShortList( const ::rtl::OUString& rData );
    void                createUnitConverter( const ::rtl::OUString& rData );

private:
    typedef ::std::set< ::rtl::OUString >                   ConfigFileSet;
    typedef ::std::map< ::rtl::OUString, ::rtl::OUString >  ConfigDataMap;
    typedef ::std::map< ::rtl::OUString, NameListRef >      NameListMap;

    ConfigFileSet       maConfigFiles;
    ConfigDataMap       maConfigData;
    NameListMap         maNameLists;
    ::rtl::OUString     maConfigPath;
    bool                mbLoaded;
};

// ----------------------------------------------------------------------------

template< typename ListType >
::boost::shared_ptr< ListType > SharedConfigData::createNameList( const ::rtl::OUString& rListName )
{
    ::boost::shared_ptr< ListType > xList;
    if( rListName.getLength() > 0 )
    {
        xList.reset( new ListType( *this ) );
        setNameList( rListName, xList );
    }
    return xList;
}

template< typename ListType >
void SharedConfigData::readNameList( const ConfigInputStreamRef& rxStrm, const ::rtl::OUString& rListName )
{
    NameListRef xList = createNameList< ListType >( rListName );
    if( xList.get() )
        xList->readConfigBlock( rxStrm );
}

// ============================================================================

class Config : public Base
{
public:
    explicit            Config( const Config& rParent );
    explicit            Config( const ::rtl::OUString& rFileName );
    explicit            Config( const sal_Char* pcEnvVar );
    virtual             ~Config();

    void                setStringOption( const StringWrapper& rKey, const StringWrapper& rData );

    const ::rtl::OUString& getStringOption( const StringWrapper& rKey, const ::rtl::OUString& rDefault ) const;
    bool                getBoolOption( const StringWrapper& rKey, bool bDefault ) const;
    template< typename Type >
    Type                getIntOption( const StringWrapper& rKey, Type nDefault ) const;

    bool                isDumperEnabled() const;
    bool                isImportEnabled() const;

    template< typename ListType >
    ::boost::shared_ptr< ListType > createNameList( const StringWrapper& rListName );
    void                setNameList( const StringWrapper& rListName, NameListRef xList );
    void                eraseNameList( const StringWrapper& rListName );
    NameListRef         getNameList( const StringWrapper& rListName ) const;

    /** Returns the name for the passed key from the passed name list. */
    template< typename Type >
    ::rtl::OUString     getName( const NameListWrapper& rListWrp, Type nKey ) const;
    /** Returns true, if the passed name list contains an entry for the passed key. */
    template< typename Type >
    bool                hasName( const NameListWrapper& rList, Type nKey ) const;

protected:
    inline explicit     Config() {}
    void                construct( const Config& rParent );
    void                construct( const ::rtl::OUString& rFileName );
    void                construct( const sal_Char* pcEnvVar );

    virtual bool        implIsValid() const;
    virtual const ::rtl::OUString* implGetOption( const ::rtl::OUString& rKey ) const;
    virtual NameListRef implGetNameList( const ::rtl::OUString& rListName ) const;

private:
    typedef ::boost::shared_ptr< SharedConfigData > SharedConfigDataRef;
    SharedConfigDataRef mxCfgData;
};

typedef ::boost::shared_ptr< Config > ConfigRef;

// ----------------------------------------------------------------------------

template< typename Type >
Type Config::getIntOption( const StringWrapper& rKey, Type nDefault ) const
{
    sal_Int64 nRawData;
    const ::rtl::OUString* pData = implGetOption( rKey.getString() );
    return (pData && StringHelper::convertStringToInt( nRawData, *pData )) ?
        static_cast< Type >( nRawData ) : nDefault;
}

template< typename ListType >
::boost::shared_ptr< ListType > Config::createNameList( const StringWrapper& rListName )
{
    return mxCfgData->createNameList< ListType >( rListName.getString() );
}

template< typename Type >
::rtl::OUString Config::getName( const NameListWrapper& rListWrp, Type nKey ) const
{
    NameListRef xList = rListWrp.getNameList( *this );
    return xList.get() ? xList->getName( *this, nKey ) : OOX_DUMP_ERR_NOMAP;
}

template< typename Type >
bool Config::hasName( const NameListWrapper& rListWrp, Type nKey ) const
{
    NameListRef xList = rListWrp.getNameList( *this );
    return xList.get() && xList->hasName( nKey );
}

// ============================================================================
// ============================================================================

class Input : public Base
{
public:
    virtual sal_Int64   getSize() const = 0;
    virtual sal_Int64   tell() const = 0;
    virtual void        seek( sal_Int64 nPos ) = 0;
    virtual void        skip( sal_Int32 nBytes ) = 0;
    virtual sal_Int32   read( void* pBuffer, sal_Int32 nBytes ) = 0;

    virtual Input&      operator>>( sal_Int8& rnData ) = 0;
    virtual Input&      operator>>( sal_uInt8& rnData ) = 0;
    virtual Input&      operator>>( sal_Int16& rnData ) = 0;
    virtual Input&      operator>>( sal_uInt16& rnData ) = 0;
    virtual Input&      operator>>( sal_Int32& rnData ) = 0;
    virtual Input&      operator>>( sal_uInt32& rnData ) = 0;
    virtual Input&      operator>>( float& rfData ) = 0;
    virtual Input&      operator>>( double& rfData ) = 0;

    inline bool         isValidPos() const { return tell() < getSize(); }
    template< typename Type >
    inline Type         readValue() { Type nValue; *this >> nValue; return nValue; }

protected:
    virtual bool        implIsValid() const;
};

typedef ::boost::shared_ptr< Input > InputRef;

Input& operator>>( Input& rIn, sal_Int64& rnData );
Input& operator>>( Input& rIn, sal_uInt64& rnData );

// ============================================================================

class BinaryInput : public Input
{
public:
    explicit            BinaryInput( BinaryInputStream& rStrm );

    virtual sal_Int64   getSize() const;
    virtual sal_Int64   tell() const;
    virtual void        seek( sal_Int64 nPos );
    virtual void        skip( sal_Int32 nBytes );
    virtual sal_Int32   read( void* pBuffer, sal_Int32 nBytes );

    virtual BinaryInput& operator>>( sal_Int8& rnData );
    virtual BinaryInput& operator>>( sal_uInt8& rnData );
    virtual BinaryInput& operator>>( sal_Int16& rnData );
    virtual BinaryInput& operator>>( sal_uInt16& rnData );
    virtual BinaryInput& operator>>( sal_Int32& rnData );
    virtual BinaryInput& operator>>( sal_uInt32& rnData );
    virtual BinaryInput& operator>>( float& rfData );
    virtual BinaryInput& operator>>( double& rfData );

protected:
    virtual bool        implIsValid() const;

private:
    BinaryInputStream&  mrStrm;
};

// ============================================================================
// ============================================================================

class Output : public Base
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextOutputStream > XTextOutputStreamRef;

public:
    explicit            Output( const XTextOutputStreamRef& rxStrm );
    explicit            Output( const ::rtl::OUString& rFileName );

    // ------------------------------------------------------------------------

    void                newLine();
    void                emptyLine( size_t nCount = 1 );
    inline ::rtl::OUStringBuffer& getLine() { return maLine; }

    void                incIndent();
    void                decIndent();
    void                resetIndent();

    void                startTable( sal_Int32 nW1 );
    void                startTable( sal_Int32 nW1, sal_Int32 nW2 );
    void                startTable( sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3 );
    void                startTable( sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3, sal_Int32 nW4 );
    void                startTable( size_t nColCount, const sal_Int32* pnColWidths );
    void                tab();
    void                tab( size_t nCol );
    void                endTable();

    void                resetItemIndex( sal_Int64 nIdx = 0 );
    void                startItem( const sal_Char* pcName = 0 );
    void                contItem();
    void                endItem();
    inline const ::rtl::OUString& getLastItemValue() const { return maLastItem; }

    void                startMultiItems();
    void                endMultiItems();

    // ------------------------------------------------------------------------

    void                writeChar( sal_Unicode cChar, sal_Int32 nCount = 1 );
    void                writeAscii( const sal_Char* pcStr );
    void                writeString( const ::rtl::OUString& rStr );
    void                writeArray( const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    void                writeBool( bool bData );
    void                writeColor( sal_Int32 nColor );
    void                writeDateTime( const ::com::sun::star::util::DateTime& rDateTime );
    void                writeColIndex( sal_Int32 nCol );
    void                writeRowIndex( sal_Int32 nRow );
    void                writeColRowRange( sal_Int32 nColRow1, sal_Int32 nColRow2 );
    void                writeColRange( sal_Int32 nCol1, sal_Int32 nCol2 );
    void                writeRowRange( sal_Int32 nRow1, sal_Int32 nRow2 );
    void                writeAddress( const Address& rPos );
    void                writeRange( const Range& rRange );
    void                writeRangeList( const RangeList& rRanges );

    template< typename Type >
    inline void         writeDec( Type nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' )
                            { StringHelper::appendDec( maLine, nData, nWidth, cFill ); }
    template< typename Type >
    inline void         writeHex( Type nData, bool bPrefix = true )
                            { StringHelper::appendHex( maLine, nData, bPrefix ); }
    template< typename Type >
    inline void         writeBin( Type nData, bool bDots = true )
                            { StringHelper::appendBin( maLine, nData, bDots ); }
    template< typename Type >
    inline void         writeFix( Type nData, sal_Int32 nWidth = 0 )
                            { StringHelper::appendFix( maLine, nData, nWidth ); }
    template< typename Type >
    inline void         writeValue( Type nData, FormatType eFmtType )
                            { StringHelper::appendValue( maLine, nData, eFmtType ); }
    template< typename Type >
    inline void         writeName( const Config& rCfg, Type nData, const NameListWrapper& rListWrp )
                            { writeString( rCfg.getName( rListWrp, nData ) ); }

    // ------------------------------------------------------------------------
protected:
    void                construct( const XTextOutputStreamRef& rxStrm );

    virtual bool        implIsValid() const;

private:
    void                writeItemName( const sal_Char* pcName );

private:
    typedef ::std::vector< sal_Int32 > StringLenVec;

    XTextOutputStreamRef mxStrm;
    ::rtl::OUString     maIndent;
    ::rtl::OUStringBuffer maLine;
    ::rtl::OUString     maLastItem;
    StringLenVec        maColPos;
    size_t              mnCol;
    size_t              mnItemLevel;
    size_t              mnMultiLevel;
    sal_Int64           mnItemIdx;
    sal_Int32           mnLastItem;
};

typedef ::boost::shared_ptr< Output > OutputRef;

// ============================================================================

class IndentGuard
{
public:
    inline explicit     IndentGuard( Output& rOut ) : mrOut( rOut ) { mrOut.incIndent(); }
    inline              ~IndentGuard() { mrOut.decIndent(); }
private:
                        IndentGuard( const IndentGuard& );
    IndentGuard&        operator=( const IndentGuard& );
private:
    Output&             mrOut;
};

// ----------------------------------------------------------------------------

class TableGuard
{
public:
    inline explicit     TableGuard( Output& rOut, sal_Int32 nW1 ) :
                            mrOut( rOut ) { mrOut.startTable( nW1 ); }
    inline explicit     TableGuard( Output& rOut, sal_Int32 nW1, sal_Int32 nW2 ) :
                            mrOut( rOut ) { mrOut.startTable( nW1, nW2 ); }
    inline explicit     TableGuard( Output& rOut, sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3 ) :
                            mrOut( rOut ) { mrOut.startTable( nW1, nW2, nW3 ); }
    inline explicit     TableGuard( Output& rOut, sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3, sal_Int32 nW4 ) :
                            mrOut( rOut ) { mrOut.startTable( nW1, nW2, nW3, nW4 ); }
    inline explicit     TableGuard( Output& rOut, size_t nColCount,
                                const sal_Int32* pnColWidths ) :
                            mrOut( rOut ) { mrOut.startTable( nColCount, pnColWidths ); }
    inline              ~TableGuard() { mrOut.endTable(); }
    inline void         tab() { mrOut.tab(); }
    inline void         tab( size_t nCol ) { mrOut.tab( nCol ); }
private:
                        TableGuard( const TableGuard& );
    TableGuard&         operator=( const TableGuard& );
private:
    Output&             mrOut;
};

// ----------------------------------------------------------------------------

class ItemGuard
{
public:
    inline explicit     ItemGuard( Output& rOut, const sal_Char* pcName = 0 ) :
                            mrOut( rOut ) { mrOut.startItem( pcName ); }
    inline              ~ItemGuard() { mrOut.endItem(); }
    inline void         cont() { mrOut.contItem(); }
private:
                        ItemGuard( const ItemGuard& );
    ItemGuard&          operator=( const ItemGuard& );
private:
    Output&             mrOut;
};

// ----------------------------------------------------------------------------

class MultiItemsGuard
{
public:
    inline explicit     MultiItemsGuard( Output& rOut ) : mrOut( rOut ) { mrOut.startMultiItems(); }
    inline              ~MultiItemsGuard() { mrOut.endMultiItems(); }
private:
                        MultiItemsGuard( const MultiItemsGuard& );
    MultiItemsGuard&    operator=( const MultiItemsGuard& );
private:
    Output&             mrOut;
};

// ============================================================================
// ============================================================================

class ObjectBase : public Base
{
public:
    virtual             ~ObjectBase();

    void                dump();

    // ------------------------------------------------------------------------
protected:
    inline explicit     ObjectBase() : mpFilter( 0 ) {}
    void                construct( const ::oox::core::FilterBase& rFilter, ConfigRef xConfig );
    void                construct( const ObjectBase& rParent );

    virtual bool        implIsValid() const;
    virtual ConfigRef   implReconstructConfig();
    virtual void        implDump();

    // ------------------------------------------------------------------------

    void                reconstructConfig();

    inline const ::oox::core::FilterBase& getFilter() const { return *mpFilter; }
    inline Config&      cfg() const { return *mxConfig; }

private:
    const ::oox::core::FilterBase* mpFilter;
    ConfigRef           mxConfig;
};

typedef ::boost::shared_ptr< ObjectBase > ObjectRef;

// ============================================================================
// ============================================================================

class RootStorageObjectBase : public ObjectBase
{
public:
    virtual             ~RootStorageObjectBase();

protected:
    inline explicit     RootStorageObjectBase() {}

    using               ObjectBase::construct;
    void                construct( const ObjectBase& rParent );

    virtual void        implDump();

    virtual void        implDumpStream(
                            BinaryInputStreamRef xStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSystemFileName );

private:
    void                extractStream(
                            StorageBase& rStrg,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSystemFileName );
    void                extractStorage(
                            StorageRef xStrg,
                            const ::rtl::OUString& rSystemPath );
};

typedef ::boost::shared_ptr< RootStorageObjectBase > RootStorageObjectRef;

// ============================================================================

class StorageIterator : public Base
{
public:
    explicit            StorageIterator( StorageRef xStrg );
    virtual             ~StorageIterator();

    size_t              getElementCount() const;

    StorageIterator&    operator++();

    ::rtl::OUString     getName() const;
    bool                isStream() const;
    bool                isStorage() const;

private:
    virtual bool        implIsValid() const;

private:
    StorageRef          mxStrg;
    OUStringVector      maNames;
    OUStringVector::const_iterator maIt;
};

// ============================================================================
// ============================================================================

class OutputObjectBase : public ObjectBase
{
public:
    virtual             ~OutputObjectBase();

    // ------------------------------------------------------------------------
protected:
    inline explicit     OutputObjectBase() {}

    using               ObjectBase::construct;
    void                construct( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName );
    void                construct( const ObjectBase& rParent, OutputRef xOut );
    void                construct( const OutputObjectBase& rParent );

    virtual bool        implIsValid() const;
    virtual OutputRef   implReconstructOutput();

    // ------------------------------------------------------------------------

    void                reconstructOutput();

    inline Output&      out() const { return *mxOut; }

    // ------------------------------------------------------------------------

    void                writeEmptyItem( const sal_Char* pcName );
    void                writeInfoItem( const sal_Char* pcName, const StringWrapper& rData );
    void                writeStringItem( const sal_Char* pcName, const ::rtl::OUString& rData );
    void                writeArrayItem( const sal_Char* pcName, const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    void                writeBoolItem( const sal_Char* pcName, bool bData );
    double              writeRkItem( const sal_Char* pcName, sal_Int32 nRk );
    void                writeColorItem( const sal_Char* pcName, sal_Int32 nColor );
    void                writeDateTimeItem( const sal_Char* pcName, const ::com::sun::star::util::DateTime& rDateTime );
    void                writeGuidItem( const sal_Char* pcName, const ::rtl::OUString& rGuid );
    void                writeColIndexItem( const sal_Char* pcName, sal_Int32 nCol );
    void                writeRowIndexItem( const sal_Char* pcName, sal_Int32 nRow );
    void                writeColRangeItem( const sal_Char* pcName, sal_Int32 nCol1, sal_Int32 nCol2 );
    void                writeRowRangeItem( const sal_Char* pcName, sal_Int32 nRow1, sal_Int32 nRow2 );
    void                writeAddressItem( const sal_Char* pcName, const Address& rPos );
    void                writeRangeItem( const sal_Char* pcName, const Range& rRange );
    void                writeRangeListItem( const sal_Char* pcName, const RangeList& rRanges );
    void                writeTokenAddressItem( const sal_Char* pcName, const TokenAddress& rPos, bool bNameMode );
    void                writeTokenAddress3dItem( const sal_Char* pcName, const ::rtl::OUString& rRef, const TokenAddress& rPos, bool bNameMode );
    void                writeTokenRangeItem( const sal_Char* pcName, const TokenRange& rRange, bool bNameMode );
    void                writeTokenRange3dItem( const sal_Char* pcName, const ::rtl::OUString& rRef, const TokenRange& rRange, bool bNameMode );

    template< typename Type >
    void                addNameToItem( Type nData, const NameListWrapper& rListWrp );

    template< typename Type >
    void                writeNameItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp );
    template< typename Type >
    void                writeDecItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeHexItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeBinItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeFixItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeDecBoolItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeValueItem( const sal_Char* pcName, Type nData, FormatType eFmtType, const NameListWrapper& rListWrp = NO_LIST );

    template< typename Type >
    void                writeValueItem( const ItemFormat& rItemFmt, Type nData );

private:
    OutputRef           mxOut;
};

typedef ::boost::shared_ptr< OutputObjectBase > OutputObjectRef;

// ----------------------------------------------------------------------------

template< typename Type >
void OutputObjectBase::addNameToItem( Type nData, const NameListWrapper& rListWrp )
{
    if( !rListWrp.isEmpty() )
    {
        mxOut->contItem();
        mxOut->writeName( cfg(), nData, rListWrp );
    }
}

template< typename Type >
void OutputObjectBase::writeNameItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeName( cfg(), nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeDecItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeDec( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeHexItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeHex( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeBinItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeBin( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeFixItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeFix( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeDecBoolItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->writeDec( nData );
    aItem.cont();
    mxOut->writeBool( nData != 0 );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeValueItem( const sal_Char* pcName, Type nData, FormatType eFmtType, const NameListWrapper& rListWrp )
{
    if( eFmtType == FORMATTYPE_BOOL )
        writeDecBoolItem( pcName, nData, rListWrp );
    else
    {
        ItemGuard aItem( *mxOut, pcName );
        mxOut->writeValue( nData, eFmtType );
        addNameToItem( nData, rListWrp );
    }
}

template< typename Type >
void OutputObjectBase::writeValueItem( const ItemFormat& rItemFmt, Type nData )
{
    ::rtl::OString aNameUtf8 = StringHelper::convertToUtf8( rItemFmt.maItemName );
    writeValueItem( aNameUtf8.getStr(), nData, rItemFmt.meFmtType, rItemFmt.maListName );
}

// ============================================================================
// ============================================================================

class InputObjectBase : public OutputObjectBase
{
public:
    virtual             ~InputObjectBase();

    // ------------------------------------------------------------------------
protected:
    inline explicit     InputObjectBase() {}

    using               OutputObjectBase::construct;
    void                construct( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, InputRef xIn );
    void                construct( const ObjectBase& rParent, OutputRef xOut, InputRef xIn );
    void                construct( const OutputObjectBase& rParent, InputRef xIn );
    void                construct( const InputObjectBase& rParent );

    virtual bool        implIsValid() const;
    virtual InputRef    implReconstructInput();

    // ------------------------------------------------------------------------

    void                reconstructInput();

    inline Input&       in() const { return *mxIn; }

    // ------------------------------------------------------------------------

    void                skipBlock( sal_Int32 nBytes, bool bShowSize = true );
    void                dumpRawBinary( sal_Int32 nBytes, bool bShowOffset = true, bool bStream = false );

    void                dumpBinary( const sal_Char* pcName, sal_Int32 nBytes, bool bShowOffset = true );
    void                dumpArray( const sal_Char* pcName, sal_Int32 nBytes, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    void                dumpRemaining( sal_Int32 nBytes );
    inline void         dumpUnused( sal_Int32 nBytes ) { dumpArray( OOX_DUMP_UNUSED, nBytes ); }
    inline void         dumpUnknown( sal_Int32 nBytes ) { dumpArray( OOX_DUMP_UNKNOWN, nBytes ); }

    ::rtl::OUString     dumpCharArray( const sal_Char* pcName, sal_Int32 nSize, rtl_TextEncoding eTextEnc );
    ::rtl::OUString     dumpUnicodeArray( const sal_Char* pcName, sal_Int32 nSize );

    double              dumpRk( const sal_Char* pcName = 0 );
    ::rtl::OUString     dumpGuid( const sal_Char* pcName = 0 );
    void                dumpItem( const ItemFormat& rItemFmt );

    template< typename Type >
    Type                dumpName( const sal_Char* pcName, const NameListWrapper& rListWrp );
    template< typename Type >
    Type                dumpDec( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpHex( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpBin( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpFix( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpBool( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpValue( const ItemFormat& rItemFmt );

    template< typename Type1, typename Type2 >
    Type1               dumpName( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpDec( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpHex( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpBin( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpFix( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpBool( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpValue( bool bType1, const ItemFormat& rItemFmt );

private:
    InputRef            mxIn;
};

typedef ::boost::shared_ptr< InputObjectBase > InputObjectRef;

// ----------------------------------------------------------------------------

template< typename Type >
Type InputObjectBase::dumpName( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    writeNameItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpDec( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    writeDecItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpHex( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    writeHexItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpBin( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    writeBinItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpFix( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    writeFixItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpBool( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    writeDecBoolItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpValue( const ItemFormat& rItemFmt )
{
    Type nData;
    *mxIn >> nData;
    writeValueItem( rItemFmt, nData );
    return nData;
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpName( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpName< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( dumpName< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpDec( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpDec< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( dumpDec< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpHex( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpHex< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( dumpHex< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpBin( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpBin< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( dumpBin< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpFix( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpFix< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( dumpFix< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpBool( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpBool< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( dumpBool< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpValue( bool bType1, const ItemFormat& rItemFmt )
{
    return bType1 ? dumpValue< Type1 >( rItemFmt ) : static_cast< Type1 >( dumpValue< Type2 >( rItemFmt ) );
}

// ============================================================================

class InputStreamObject : public InputObjectBase
{
public:
    explicit            InputStreamObject( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, BinaryInputStreamRef xStrm );
    virtual             ~InputStreamObject();

    inline BinaryInputStream& getStream() { return *mxStrm; }
    sal_Int64           getStreamSize() const;

protected:
    inline explicit     InputStreamObject() {}

    using               InputObjectBase::construct;
    void                construct( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, BinaryInputStreamRef xStrm );

    void                dumpBinaryStream( bool bShowOffset = true );

    virtual bool        implIsValid() const;
    virtual void        implDump();

private:
    BinaryInputStreamRef mxStrm;
};

typedef ::boost::shared_ptr< InputStreamObject > InputStreamObjectRef;

// ============================================================================
// ============================================================================

class TextStreamObject : public InputStreamObject
{
public:
    explicit            TextStreamObject( const ObjectBase& rParent,
                            const ::rtl::OUString& rOutFileName,
                            BinaryInputStreamRef xStrm, rtl_TextEncoding eTextEnc );

protected:
    virtual void        implDump();
    virtual void        implDumpLine( const ::rtl::OUString& rLine, sal_uInt32 nLine );

private:
    rtl_TextEncoding    meTextEnc;
};

// ============================================================================

class XmlStreamObject : public TextStreamObject
{
public:
    explicit            XmlStreamObject( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, BinaryInputStreamRef xStrm );

protected:
    virtual void        implDump();
    virtual void        implDumpLine( const ::rtl::OUString& rLine, sal_uInt32 nLine );

private:
    ::rtl::OUString     maIncompleteLine;
};

// ============================================================================
// ============================================================================

struct RecordHeaderConfigInfo
{
    const sal_Char*     mpcTitle;
    const sal_Char*     mpcRecNames;
    const sal_Char*     mpcShowRecPos;
    const sal_Char*     mpcShowRecSize;
    const sal_Char*     mpcShowRecId;
    const sal_Char*     mpcShowRecName;
    const sal_Char*     mpcShowRecBody;
};

// ============================================================================

class RecordHeaderImplBase : public InputObjectBase
{
public:
    inline const sal_Char* getTitle() const { return mpcTitle; }
    inline NameListRef  getRecNames() const { return mxRecNames; }
    inline bool         isShowRecPos() const { return mbShowRecPos; }
    inline bool         isShowRecSize() const { return mbShowRecSize; }
    inline bool         isShowRecId() const { return mbShowRecId; }
    inline bool         isShowRecName() const { return mbShowRecName; }
    inline bool         isShowRecBody() const { return mbShowRecBody; }

protected:
    inline explicit     RecordHeaderImplBase() {}
    virtual             ~RecordHeaderImplBase();

    using               InputObjectBase::construct;
    void                construct( const InputObjectBase& rParent, const RecordHeaderConfigInfo& rCfgInfo );

    virtual bool        implIsValid() const;

private:
    const sal_Char*     mpcTitle;
    NameListRef         mxRecNames;
    bool                mbShowRecPos;
    bool                mbShowRecSize;
    bool                mbShowRecId;
    bool                mbShowRecName;
    bool                mbShowRecBody;
};

// ============================================================================

template< typename RecIdType, typename RecSizeType >
class RecordHeaderBase : public RecordHeaderImplBase
{
public:
    inline bool         isValidHeader() const { return mbValidHeader; }
    inline sal_Int64    getRecPos() const { return mnRecPos; }
    inline RecIdType    getRecId() const { return mnRecId; }
    inline RecSizeType  getRecSize() const { return mnRecSize; }
    inline bool         hasRecName() const { return getRecNames()->hasName( mnRecId ); }

    bool                startNextRecord();

protected:
    using               RecordHeaderImplBase::construct;
    void                construct( const InputObjectBase& rParent, const RecordHeaderConfigInfo& rCfgInfo );

    inline virtual void implDump();
    virtual bool        implReadHeader( sal_Int64& ornRecPos, RecIdType& ornRecId, RecSizeType& ornRecSize ) = 0;
    inline virtual void implWriteExtHeader() {}

private:
    void                writeHeader();

private:
    sal_Int64           mnRecPos;
    RecIdType           mnRecId;
    RecSizeType         mnRecSize;
    bool                mbValidHeader;
};

// ----------------------------------------------------------------------------

template< typename RecIdType, typename RecSizeType >
bool RecordHeaderBase< RecIdType, RecSizeType >::startNextRecord()
{
    dump();
    return mbValidHeader;
}

template< typename RecIdType, typename RecSizeType >
void RecordHeaderBase< RecIdType, RecSizeType >::construct(
        const InputObjectBase& rParent, const RecordHeaderConfigInfo& rCfgInfo )
{
    RecordHeaderImplBase::construct( rParent, rCfgInfo );
    mnRecPos = 0;
    mnRecId = 0;
    mnRecSize = 0;
    mbValidHeader = false;
}

template< typename RecIdType, typename RecSizeType >
inline void RecordHeaderBase< RecIdType, RecSizeType >::implDump()
{
    mbValidHeader = implReadHeader( mnRecPos, mnRecId, mnRecSize );
    if( mbValidHeader )
    {
        writeHeader();
        implWriteExtHeader();
    }
}

template< typename RecIdType, typename RecSizeType >
void RecordHeaderBase< RecIdType, RecSizeType >::writeHeader()
{
    MultiItemsGuard aMultiGuard( out() );
    writeEmptyItem( getTitle() );
    if( isShowRecPos() )  writeHexItem( "pos", static_cast< sal_uInt32 >( mnRecPos ), "CONV-DEC" );
    if( isShowRecSize() ) writeHexItem( "size", mnRecSize, "CONV-DEC" );
    if( isShowRecId() )   writeHexItem( "id", mnRecId );
    if( isShowRecName() ) writeNameItem( "name", mnRecId, getRecNames() );
}

// ============================================================================
// ============================================================================

/** Base class for a file dumper. Derived classes implement the implDump()
    function to add functionality.
 */
class DumperBase : public ObjectBase
{
public:
    virtual             ~DumperBase();

    bool                isImportEnabled() const;

    StorageRef          getRootStorage() const;
    BinaryInputStreamRef getRootStream() const;

protected:
    inline explicit     DumperBase() {}

    using               ObjectBase::construct;
    void                construct( const ::oox::core::FilterBase& rFilter, ConfigRef xConfig );
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#define OOX_DUMP_FILE( DumperClassName )    \
do {                                        \
    DumperClassName aDumper( *this );       \
    aDumper.dump();                         \
    if( !aDumper.isImportEnabled() )        \
        return aDumper.isValid();           \
} while( false )

#else   // OOX_INCLUDE_DUMPER

#define OOX_DUMP_FILE( DumperClassName ) (void)0

#endif  // OOX_INCLUDE_DUMPER
#endif

