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

#ifndef INCLUDED_OOX_DUMP_DUMPERBASE_HXX
#define INCLUDED_OOX_DUMP_DUMPERBASE_HXX

#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binarystreambase.hxx>
#include <oox/helper/storagebase.hxx>
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>

#define OOX_INCLUDE_DUMPER (OSL_DEBUG_LEVEL > 0)

#if OOX_INCLUDE_DUMPER

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
    namespace io { class XOutputStream; }
    namespace io { class XTextOutputStream2; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {
    class TextInputStream;
}

namespace oox { namespace core {
    class FilterBase;
} }

namespace oox {
namespace dump {


#define OOX_DUMP_UNUSED                     "unused"
#define OOX_DUMP_UNKNOWN                    "?unknown"

#define OOX_DUMP_ERRASCII( ascii )          "?err:" ascii

#define OOX_DUMP_ERR_NOMAP                  "no-map"
#define OOX_DUMP_ERR_NONAME                 "no-name"
#define OOX_DUMP_ERR_STREAM                 "stream-error"

#define OOX_DUMP_DUMPEXT                    ".dump"

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

typedef ::std::pair< OUString, OUString > OUStringPair;

typedef ::std::vector< OUString >         OUStringVector;
typedef ::std::vector< sal_Int64 >        Int64Vector;


/** Static helper functions for system file and stream access. */
class InputOutputHelper
{
public:
    // file names -------------------------------------------------------------

    static OUString convertFileNameToUrl( const OUString& rFileName );
    static sal_Int32    getFileNamePos( const OUString& rFileUrl );
    static OUString getFileNameExtension( const OUString& rFileUrl );

    // input streams ----------------------------------------------------------

    static css::uno::Reference< css::io::XInputStream >
                        openInputStream(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const OUString& rFileName );

    // output streams ---------------------------------------------------------

    static css::uno::Reference< css::io::XOutputStream >
                        openOutputStream(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const OUString& rFileName );

    static css::uno::Reference< css::io::XTextOutputStream2 >
                        openTextOutputStream(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::io::XOutputStream >& rxOutStrm,
                            rtl_TextEncoding eTextEnc );

    static css::uno::Reference< css::io::XTextOutputStream2 >
                        openTextOutputStream(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const OUString& rFileName,
                            rtl_TextEncoding eTextEnc );
};


class BinaryInputStreamRef : public ::oox::BinaryInputStreamRef
{
public:
    BinaryInputStreamRef() {}

    /*implicit*/ BinaryInputStreamRef( BinaryInputStream* pInStrm ) :
                            ::oox::BinaryInputStreamRef( pInStrm ) {}

    /*implicit*/ BinaryInputStreamRef( const css::uno::Reference< css::io::XInputStream >& rxInStrm ) :
                            ::oox::BinaryInputStreamRef( new BinaryXInputStream( rxInStrm, true ) ) {}

    template< typename StreamType >
    /*implicit*/ BinaryInputStreamRef( const std::shared_ptr< StreamType >& rxInStrm ) :
                            ::oox::BinaryInputStreamRef( rxInStrm ) {}
};


/** Specifiers for atomic data types. */
enum DataType
{
    DATATYPE_VOID,              ///< No data type.
    DATATYPE_INT8,              ///< Signed 8-bit integer.
    DATATYPE_UINT8,             ///< Unsigned 8-bit integer.
    DATATYPE_INT16,             ///< Signed 16-bit integer.
    DATATYPE_UINT16,            ///< Unsigned 16-bit integer.
    DATATYPE_INT32,             ///< Signed 32-bit integer.
    DATATYPE_UINT32,            ///< Unsigned 32-bit integer.
    DATATYPE_INT64,             ///< Signed 64-bit integer.
    DATATYPE_UINT64,            ///< Unsigned 64-bit integer.
    DATATYPE_FLOAT,             ///< Floating-point, single precision.
    DATATYPE_DOUBLE             ///< Floating-point, double precision.
};


/** Specifiers for the output format of values. */
enum FormatType
{
    FORMATTYPE_NONE,            ///< No numeric format (e.g. show name only).
    FORMATTYPE_DEC,             ///< Decimal.
    FORMATTYPE_HEX,             ///< Hexadecimal.
    FORMATTYPE_SHORTHEX,        ///< Hexadecimal, as short as possible (no leading zeros).
    FORMATTYPE_BIN,             ///< Binary.
    FORMATTYPE_FIX,             ///< Fixed-point.
    FORMATTYPE_BOOL             ///< Boolean ('true' or 'false').
};


/** Describes the output format of a data item.

    Data items are written in the following format:

    <NAME>=<VALUE>=<NAME-FROM-LIST>

    NAME is the name of the data item. The name is contained in the member
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
    DataType            meDataType;         ///< Data type of the item.
    FormatType          meFmtType;          ///< Output format for the value.
    OUString     maItemName;         ///< Name of the item.
    OUString     maListName;         ///< Name of a name list to be used for this item.

    explicit            ItemFormat();

    void                set( DataType eDataType, FormatType eFmtType, const OUString& rItemName );

    /** Initializes the struct from a vector of strings containing the item format.

        The vector must contain at least 2 strings. The struct is filled from
        the strings in the vector in the following order:
        1) Data type (one of: [u]int8, [u]int16, [u]int32, [u]int64, float, double).
        2) Format type (one of: dec, hex, shorthex, bin, fix, bool, unused, unknown).
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
    OUStringVector      parse( const OUString& rFormatStr );
};


/** Static helper functions for formatted output to strings. */
class StringHelper
{
public:
    // append string to string ------------------------------------------------

    static void         appendChar( OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount = 1 );
    static void         appendString( OUStringBuffer& rStr, const OUString& rData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );

    // append decimal ---------------------------------------------------------

    static void         appendDec( OUStringBuffer& rStr, sal_uInt8  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_Int8   nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_uInt16 nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_Int16  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_uInt32 nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_Int32  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_uInt64 nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, sal_Int64  nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );
    static void         appendDec( OUStringBuffer& rStr, double     fData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' );

    // append hexadecimal -----------------------------------------------------

    static void         appendHex( OUStringBuffer& rStr, sal_uInt8  nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_Int8   nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_Int16  nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_Int32  nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_uInt64 nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, sal_Int64  nData, bool bPrefix = true );
    static void         appendHex( OUStringBuffer& rStr, double     fData, bool bPrefix = true );

    // append shortened hexadecimal -------------------------------------------

    static void         appendShortHex( OUStringBuffer& rStr, sal_uInt8  nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_Int8   nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_uInt16 nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_Int16  nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_uInt32 nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_Int32  nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_uInt64 nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, sal_Int64  nData, bool bPrefix = true );
    static void         appendShortHex( OUStringBuffer& rStr, double     fData, bool bPrefix = true );

    // append binary ----------------------------------------------------------

    static void         appendBin( OUStringBuffer& rStr, sal_uInt8  nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_Int8   nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_uInt16 nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_Int16  nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_uInt32 nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_Int32  nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_uInt64 nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, sal_Int64  nData, bool bDots = true );
    static void         appendBin( OUStringBuffer& rStr, double     fData, bool bDots = true );

    // append fixed-point decimal ---------------------------------------------

    template< typename Type >
    static void         appendFix( OUStringBuffer& rStr, Type nData, sal_Int32 nWidth = 0 );

    // append formatted value -------------------------------------------------

    static void         appendBool( OUStringBuffer& rStr, bool bData );
    template< typename Type >
    static void         appendValue( OUStringBuffer& rStr, Type nData, FormatType eFmtType );

    // encoded text output ----------------------------------------------------

    static void         appendCChar( OUStringBuffer& rStr, sal_Unicode cChar, bool bPrefix = true );
    static void         appendEncChar( OUStringBuffer& rStr, sal_Unicode cChar, sal_Int32 nCount = 1, bool bPrefix = true );
    static void         appendEncString( OUStringBuffer& rStr, const OUString& rData, bool bPrefix = true );

    // token list -------------------------------------------------------------

    static void         appendToken( OUStringBuffer& rStr, const OUString& rToken, sal_Unicode cSep = OOX_DUMP_LISTSEP );

    static void         appendIndex( OUStringBuffer& rStr, const OUString& rIdx );
    static void         appendIndex( OUStringBuffer& rStr, sal_Int64 nIdx );

    static OUString getToken( const OUString& rData, sal_Int32& rnPos, sal_Unicode cSep = OOX_DUMP_LISTSEP );

    /** Encloses the passed string with the passed characters. Uses cOpen, if cClose is NUL. */
    static void         enclose( OUStringBuffer& rStr, sal_Unicode cOpen, sal_Unicode cClose = '\0' );

    // string conversion ------------------------------------------------------

    static OUString trimSpaces( const OUString& rStr );
    static OUString trimTrailingNul( const OUString& rStr );

    static OString convertToUtf8( const OUString& rStr );
    static DataType     convertToDataType( const OUString& rStr );
    static FormatType   convertToFormatType( const OUString& rStr );

    static bool         convertFromDec( sal_Int64& ornData, const OUString& rData );
    static bool         convertFromHex( sal_Int64& ornData, const OUString& rData );

    static bool         convertStringToInt( sal_Int64& ornData, const OUString& rData );
    static bool         convertStringToDouble( double& orfData, const OUString& rData );
    static bool         convertStringToBool( const OUString& rData );

    static OUStringPair convertStringToPair( const OUString& rString, sal_Unicode cSep = '=' );

    // string to list conversion ----------------------------------------------

    static void         convertStringToStringList( OUStringVector& orVec, const OUString& rData, bool bIgnoreEmpty );
    static void         convertStringToIntList( Int64Vector& orVec, const OUString& rData, bool bIgnoreEmpty );
};


template< typename Type >
void StringHelper::appendFix( OUStringBuffer& rStr, Type nData, sal_Int32 nWidth )
{
    appendDec( rStr, static_cast< double >( nData ) / std::pow( 2.0, 4.0 * sizeof( Type ) ), nWidth );
}

template< typename Type >
void StringHelper::appendValue( OUStringBuffer& rStr, Type nData, FormatType eFmtType )
{
    switch( eFmtType )
    {
        case FORMATTYPE_DEC:        appendDec( rStr, nData );       break;
        case FORMATTYPE_HEX:        appendHex( rStr, nData );       break;
        case FORMATTYPE_SHORTHEX:   appendShortHex( rStr, nData );  break;
        case FORMATTYPE_BIN:        appendBin( rStr, nData );       break;
        case FORMATTYPE_FIX:        appendFix( rStr, nData );       break;
        case FORMATTYPE_BOOL:       appendBool( rStr, nData );      break;
        default:;
    }
}


class String : public OUString
{
public:
                 String() {}
    /*implicit*/ String( const OUString& rStr ) : OUString( rStr ) {}
    /*implicit*/ String( const sal_Char* pcStr ) : OUString( OUString::createFromAscii( pcStr ? pcStr : "" ) ) {}
    /*implicit*/ String( sal_Unicode cChar ) : OUString( cChar ) {}

    bool         has() const { return getLength() > 0; }
    OUString operator()( const sal_Char* pcDefault ) const { if( has() ) return *this; return String( pcDefault ); }
};

static const String EMPTY_STRING;


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
    |       |
    |       +---->  FlagsList  ------>  CombiList
    |       |
    |       +---->  UnitConverter
    |
    +---->  SharedConfigData
    |
    +---->  Config
    |
    +---->  Output
    |
    +---->  StorageIterator
    |
    +---->  ObjectBase
            |
            +---->  StorageObjectBase
            |
            +---->  OutputObjectBase
            |       |
            |       +---->  InputObjectBase
            |               |
            |               +---->  BinaryStreamObject
            |               |
            |               +---->  TextStreamObjectBase
            |               |       |
            |               |       +---->  TextStreamObject
            |               |       |
            |               |       +---->  XmlStreamObject
            |               |
            |               +---->  RecordObjectBase
            |                       |
            |                       +---->  SequenceRecordObjectBase
            |
            +---->  DumperBase
 */
class Base
{
public:
    virtual             ~Base();

    bool         isValid() const { return implIsValid(); }
    static bool  isValid( const std::shared_ptr< Base >& rxBase ) { return rxBase.get() && rxBase->isValid(); }

protected:
                        Base() {}

    virtual bool        implIsValid() const = 0;
};


class ConfigItemBase
{
public:
    virtual             ~ConfigItemBase();
    void                readConfigBlock( TextInputStream& rStrm );

protected:
                        ConfigItemBase() {}

    virtual void        implProcessConfigItemStr(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData );

    virtual void        implProcessConfigItemInt(
                            TextInputStream& rStrm,
                            sal_Int64 nKey,
                            const OUString& rData );

    void                readConfigBlockContents(
                            TextInputStream& rStrm );

private:
    enum LineType { LINETYPE_DATA, LINETYPE_END };

    static LineType     readConfigLine(
                            TextInputStream& rStrm,
                            OUString& orKey,
                            OUString& orData );

    void                processConfigItem(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData );
};


class SharedConfigData;
class Config;

class NameListBase;
typedef std::shared_ptr< NameListBase > NameListRef;

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
    typedef ::std::map< sal_Int64, OUString >    OUStringMap;
    typedef OUStringMap::const_iterator                 const_iterator;

public:
    virtual             ~NameListBase();

    /** Sets a name for the specified key. */
    void                setName( sal_Int64 nKey, const String& rName );

    /** Include all names of the passed list. */
    void                includeList( const NameListRef& rxList );

    /** Returns true, if the map contains an entry for the passed key. */
    template< typename Type >
    bool         hasName( Type nKey ) const
        { return maMap.count( static_cast< sal_Int64 >( nKey ) ) != 0; }

    /** Returns the name for the passed key. */
    template< typename Type >
    OUString getName( const Config& rCfg, Type nKey ) const
        { return implGetName( rCfg, static_cast< sal_Int64 >( nKey ) ); }

    /** Returns a display name for the passed double value. */
    OUString getName( const Config& rCfg, double fValue ) const
        { return implGetNameDbl( rCfg, fValue ); }

    /** Returns a map iterator pointing to the first contained name. */
    const_iterator begin() const { return maMap.begin(); }
    /** Returns a map iterator pointing one past the last contained name. */
    const_iterator end() const { return maMap.end(); }

protected:
    explicit            NameListBase( const SharedConfigData& rCfgData ) : mrCfgData( rCfgData ) {}

    virtual bool        implIsValid() const override;

    virtual void        implProcessConfigItemStr(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData ) override;

    virtual void        implProcessConfigItemInt(
                            TextInputStream& rStrm,
                            sal_Int64 nKey,
                            const OUString& rData ) override;

    /** Derived classes set the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const OUString& rName ) = 0;
    /** Derived classes generate and return the name for the passed key. */
    virtual OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const = 0;
    /** Derived classes generate and return the name for the passed double value. */
    virtual OUString implGetNameDbl( const Config& rCfg, double fValue ) const = 0;
    /** Derived classes insert all names and other settings from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList ) = 0;

    /** Inserts the passed name into the internal map. */
    void                insertRawName( sal_Int64 nKey, const OUString& rName );
    /** Returns the name for the passed key, or 0, if nothing found. */
    const OUString* findRawName( sal_Int64 nKey ) const;

private:
    /** Includes name lists, given in a comma separated list of names of the lists. */
    void                include( const OUString& rListKeys );
    /** Excludes names from the list, given in a comma separated list of their keys. */
    void                exclude( const OUString& rKeys );

private:
    OUStringMap         maMap;
    const SharedConfigData& mrCfgData;
};


class ConstList : public NameListBase
{
public:
    explicit            ConstList( const SharedConfigData& rCfgData );

    /** Sets a default name for unknown keys. */
    void                setDefaultName( const String& rDefName ) { maDefName = rDefName; }
    /** Enables or disables automatic quotation of returned names. */
    void                setQuoteNames( bool bQuoteNames ) { mbQuoteNames = bQuoteNames; }

protected:
    virtual void        implProcessConfigItemStr(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData ) override;

    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const OUString& rName ) override;
    /** Returns the name for the passed key, or the default name, if key is not contained. */
    virtual OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const override;
    /** Returns the name for the passed double value. */
    virtual OUString implGetNameDbl( const Config& rCfg, double fValue ) const override;
    /** Inserts all names from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList ) override;

private:
    OUString            maDefName;
    bool                mbQuoteNames;
};


class MultiList : public ConstList
{
public:
    explicit            MultiList( const SharedConfigData& rCfgData );

    void                setNamesFromVec( sal_Int64 nStartKey, const OUStringVector& rNames );

protected:
    virtual void        implProcessConfigItemStr(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData ) override;

    virtual void        implSetName( sal_Int64 nKey, const OUString& rName ) override;

private:
    bool                mbIgnoreEmpty;
};


class FlagsList : public NameListBase
{
public:
    explicit            FlagsList( const SharedConfigData& rCfgData );

    /** Returns the flags to be ignored on output. */
    sal_Int64           getIgnoreFlags() const { return mnIgnore; }
    /** Sets flags to be ignored on output. */
    void                setIgnoreFlags( sal_Int64 nIgnore ) { mnIgnore = nIgnore; }

protected:
    virtual void        implProcessConfigItemStr(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData ) override;

    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const OUString& rName ) override;
    /** Returns the name for the passed key. */
    virtual OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const override;
    /** Returns the name for the passed double value. */
    virtual OUString implGetNameDbl( const Config& rCfg, double fValue ) const override;
    /** Inserts all flags from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList ) override;

private:
    sal_Int64           mnIgnore;
};


class CombiList : public FlagsList
{
public:
    explicit            CombiList( const SharedConfigData& rCfgData );

protected:
    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const OUString& rName ) override;
    /** Returns the name for the passed key. */
    virtual OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const override;
    /** Inserts all flags from the passed list. */
    virtual void        implIncludeList( const NameListBase& rList ) override;

private:
    struct ExtItemFormatKey
    {
        sal_Int64           mnKey;
        ::std::pair< sal_Int64, sal_Int64 >  maFilter;
        explicit            ExtItemFormatKey( sal_Int64 nKey ) : mnKey( nKey ), maFilter( 0, 0 ) {}
        bool                operator<( const ExtItemFormatKey& rRight ) const;

    };
    struct ExtItemFormat : public ItemFormat
    {
        bool                mbShiftValue;
                            ExtItemFormat() : mbShiftValue( true ) {}
    };
    typedef ::std::map< ExtItemFormatKey, ExtItemFormat > ExtItemFormatMap;
    ExtItemFormatMap    maFmtMap;
};


class UnitConverter : public NameListBase
{
public:
    explicit            UnitConverter( const SharedConfigData& rCfgData );

    void                setUnitName( const String& rUnitName ) { maUnitName = rUnitName; }
    void                setFactor( double fFactor ) { mfFactor = fFactor; }

protected:
    /** Sets the name for the passed key. */
    virtual void        implSetName( sal_Int64 nKey, const OUString& rName ) override;
    /** Returns the converted value with appended unit name. */
    virtual OUString implGetName( const Config& rCfg, sal_Int64 nKey ) const override;
    /** Returns the converted value with appended unit name. */
    virtual OUString implGetNameDbl( const Config& rCfg, double fValue ) const override;
    /** Empty implementation. */
    virtual void        implIncludeList( const NameListBase& rList ) override;

private:
    OUString     maUnitName;
    double              mfFactor;
};


class NameListWrapper
{
public:
                 NameListWrapper() {}
    /*implicit*/ NameListWrapper( const OUString& rListName ) : maName( rListName ) {}
    /*implicit*/ NameListWrapper( const sal_Char* pcListName ) : maName( pcListName ) {}
    /*implicit*/ NameListWrapper( const NameListRef& rxList ) : mxList( rxList ) {}

    bool         isEmpty() const { return !mxList && !maName.has(); }
    NameListRef  getNameList( const Config& rCfg ) const;

private:
    String              maName;
    mutable NameListRef mxList;
};

static const NameListWrapper NO_LIST;


class ItemFormatMap
{
private:
    ::std::map< sal_Int64, ItemFormat > maMap;

public:
    ItemFormatMap() : maMap() {}
    explicit     ItemFormatMap( const NameListRef& rxNameList ) { insertFormats( rxNameList ); }

    ::std::map< sal_Int64, ItemFormat >::const_iterator end() const { return maMap.end(); }
    ::std::map< sal_Int64, ItemFormat >::const_iterator find(sal_Int64 nId) const
    {
        return maMap.find(nId);
    }

    void         insertFormats( const NameListRef& rxNameList );
};


class SharedConfigData : public Base, public ConfigItemBase
{
public:
    explicit            SharedConfigData(
                            const OUString& rFileName,
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const StorageRef& rxRootStrg,
                            const OUString& rSysFileName );

    virtual             ~SharedConfigData();

    const css::uno::Reference< css::uno::XComponentContext >& getContext() const { return mxContext; }
    const StorageRef& getRootStorage() const { return mxRootStrg; }
    const OUString& getSysFileName() const { return maSysFileName; }

    void                setOption( const OUString& rKey, const OUString& rData );
    const OUString*      getOption( const OUString& rKey ) const;

    template< typename ListType >
    std::shared_ptr< ListType > createNameList( const OUString& rListName );
    void                setNameList( const OUString& rListName, const NameListRef& rxList );
    void                eraseNameList( const OUString& rListName );
    NameListRef         getNameList( const OUString& rListName ) const;

    bool                isPasswordCancelled() const { return mbPwCancelled; }

protected:
    virtual bool        implIsValid() const override;
    virtual void        implProcessConfigItemStr(
                            TextInputStream& rStrm,
                            const OUString& rKey,
                            const OUString& rData ) override;

private:
    bool                readConfigFile( const OUString& rFileUrl );
    template< typename ListType >
    void                readNameList( TextInputStream& rStrm, const OUString& rListName );
    void                createShortList( const OUString& rData );
    void                createUnitConverter( const OUString& rData );

private:
    typedef ::std::set< OUString >                   ConfigFileSet;
    typedef ::std::map< OUString, OUString >  ConfigDataMap;
    typedef ::std::map< OUString, NameListRef >      NameListMap;

    css::uno::Reference< css::uno::XComponentContext > mxContext;
    StorageRef          mxRootStrg;
    OUString            maSysFileName;
    ConfigFileSet       maConfigFiles;
    ConfigDataMap       maConfigData;
    NameListMap         maNameLists;
    OUString            maConfigPath;
    bool                mbLoaded;
    bool                mbPwCancelled;
};


template< typename ListType >
std::shared_ptr< ListType > SharedConfigData::createNameList( const OUString& rListName )
{
    std::shared_ptr< ListType > xList;
    if( !rListName.isEmpty() )
    {
        xList.reset( new ListType( *this ) );
        setNameList( rListName, xList );
    }
    return xList;
}

template< typename ListType >
void SharedConfigData::readNameList( TextInputStream& rStrm, const OUString& rListName )
{
    NameListRef xList = createNameList< ListType >( rListName );
    if( xList.get() )
        xList->readConfigBlock( rStrm );
}


class Config : public Base
{
public:
    explicit            Config( const Config& rParent );
    explicit            Config(
                            const sal_Char* pcEnvVar,
                            const ::oox::core::FilterBase& rFilter );
    explicit            Config(
                            const sal_Char* pcEnvVar,
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const StorageRef& rxRootStrg,
                            const OUString& rSysFileName );

    virtual             ~Config();

    const css::uno::Reference< css::uno::XComponentContext >& getContext() const { return mxCfgData->getContext(); }
    const StorageRef& getRootStorage() const { return mxCfgData->getRootStorage(); }
    const OUString& getSysFileName() const { return mxCfgData->getSysFileName(); }

    const OUString& getStringOption( const String& rKey, const OUString& rDefault ) const;
    bool                getBoolOption( const String& rKey, bool bDefault ) const;
    template< typename Type >
    Type                getIntOption( const String& rKey, Type nDefault ) const;

    bool                isDumperEnabled() const;
    bool                isImportEnabled() const;

    template< typename ListType >
    std::shared_ptr< ListType > createNameList( const String& rListName );
    void                eraseNameList( const String& rListName );
    NameListRef         getNameList( const String& rListName ) const;

    /** Returns the name for the passed key from the passed name list. */
    template< typename Type >
    OUString     getName( const NameListWrapper& rListWrp, Type nKey ) const;
    /** Returns true, if the passed name list contains an entry for the passed key. */
    template< typename Type >
    bool                hasName( const NameListWrapper& rListWrp, Type nKey ) const;

    bool                isPasswordCancelled() const;

protected:
                        Config() {}
    void                construct( const Config& rParent );
    void                construct(
                            const sal_Char* pcEnvVar,
                            const ::oox::core::FilterBase& rFilter );
    void                construct(
                            const sal_Char* pcEnvVar,
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const StorageRef& rxRootStrg,
                            const OUString& rSysFileName );

    virtual bool        implIsValid() const override;
    const OUString*     implGetOption( const OUString& rKey ) const;
    NameListRef         implGetNameList( const OUString& rListName ) const;

private:
    typedef std::shared_ptr< SharedConfigData > SharedConfigDataRef;
    SharedConfigDataRef mxCfgData;
};

typedef std::shared_ptr< Config > ConfigRef;


template< typename Type >
Type Config::getIntOption( const String& rKey, Type nDefault ) const
{
    sal_Int64 nRawData;
    const OUString* pData = implGetOption( rKey );
    return (pData && StringHelper::convertStringToInt( nRawData, *pData )) ?
        static_cast< Type >( nRawData ) : nDefault;
}

template< typename ListType >
std::shared_ptr< ListType > Config::createNameList( const String& rListName )
{
    return mxCfgData->createNameList< ListType >( rListName );
}

template< typename Type >
OUString Config::getName( const NameListWrapper& rListWrp, Type nKey ) const
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


class Output : public Base
{
public:
    explicit            Output(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const OUString& rFileName );


    void                newLine();
    void                emptyLine( size_t nCount = 1 );

    void                incIndent();
    void                decIndent();

    void                startTable( sal_Int32 nW1 );
    void                startTable( sal_Int32 nW1, sal_Int32 nW2 );
    void                startTable( sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3, sal_Int32 nW4 );
    void                startTable( size_t nColCount, const sal_Int32* pnColWidths );
    void                tab();
    void                tab( size_t nCol );
    void                endTable();

    void                resetItemIndex( sal_Int64 nIdx = 0 );
    void                startItem( const String& rItemName );
    void                contItem();
    void                endItem();

    void                startMultiItems();
    void                endMultiItems();

    void                writeChar( sal_Unicode cChar, sal_Int32 nCount = 1 );
    void                writeAscii( const sal_Char* pcStr );
    void                writeString( const OUString& rStr );
    void                writeArray( const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    void                writeBool( bool bData );
    void                writeDateTime( const css::util::DateTime& rDateTime );

    template< typename Type >
    void                writeDec( Type nData, sal_Int32 nWidth = 0, sal_Unicode cFill = ' ' )
                            { StringHelper::appendDec( maLine, nData, nWidth, cFill ); }
    template< typename Type >
    void                writeHex( Type nData, bool bPrefix = true )
                            { StringHelper::appendHex( maLine, nData, bPrefix ); }
    template< typename Type >
    void                writeShortHex( Type nData, bool bPrefix = true )
                            { StringHelper::appendShortHex( maLine, nData, bPrefix ); }
    template< typename Type >
    void                writeBin( Type nData, bool bDots = true )
                            { StringHelper::appendBin( maLine, nData, bDots ); }
    template< typename Type >
    void                writeFix( Type nData, sal_Int32 nWidth = 0 )
                            { StringHelper::appendFix( maLine, nData, nWidth ); }
    template< typename Type >
    void                writeValue( Type nData, FormatType eFmtType )
                            { StringHelper::appendValue( maLine, nData, eFmtType ); }
    template< typename Type >
    void                writeName( const Config& rCfg, Type nData, const NameListWrapper& rListWrp )
                            { writeString( rCfg.getName( rListWrp, nData ) ); }


protected:
    virtual bool        implIsValid() const override;

private:
    void                writeItemName( const String& rItemName );

private:
    typedef ::std::vector< sal_Int32 > StringLenVec;

    css::uno::Reference< css::io::XTextOutputStream2 > mxStrm;
    OUString            maIndent;
    OUStringBuffer      maLine;
    OUString            maLastItem;
    StringLenVec        maColPos;
    size_t              mnCol;
    size_t              mnItemLevel;
    size_t              mnMultiLevel;
    sal_Int64           mnItemIdx;
    sal_Int32           mnLastItem;
};

typedef std::shared_ptr< Output > OutputRef;


class IndentGuard
{
public:
    explicit            IndentGuard( const OutputRef& rxOut ) : mrOut( *rxOut ) { mrOut.incIndent(); }
                        ~IndentGuard() { mrOut.decIndent(); }
private:
                        IndentGuard( const IndentGuard& ) = delete;
    IndentGuard&        operator=( const IndentGuard& ) = delete;
private:
    Output&             mrOut;
};


class TableGuard
{
public:
    explicit            TableGuard( const OutputRef& rxOut, sal_Int32 nW1 ) :
                            mrOut( *rxOut ) { mrOut.startTable( nW1 ); }
    explicit            TableGuard( const OutputRef& rxOut, sal_Int32 nW1, sal_Int32 nW2 ) :
                            mrOut( *rxOut ) { mrOut.startTable( nW1, nW2 ); }
    explicit            TableGuard( const OutputRef& rxOut, sal_Int32 nW1, sal_Int32 nW2, sal_Int32 nW3, sal_Int32 nW4 ) :
                            mrOut( *rxOut ) { mrOut.startTable( nW1, nW2, nW3, nW4 ); }
    explicit            TableGuard( const OutputRef& rxOut, size_t nColCount,
                                const sal_Int32* pnColWidths ) :
                            mrOut( *rxOut ) { mrOut.startTable( nColCount, pnColWidths ); }
                        ~TableGuard() { mrOut.endTable(); }
    void                tab( size_t nCol ) { mrOut.tab( nCol ); }
private:
                        TableGuard( const TableGuard& ) = delete;
    TableGuard&         operator=( const TableGuard& ) = delete;
private:
    Output&             mrOut;
};


class ItemGuard
{
public:
    explicit            ItemGuard( const OutputRef& rxOut, const String& rName = EMPTY_STRING ) :
                     mrOut( *rxOut ) { mrOut.startItem( rName ); }
                        ~ItemGuard() { mrOut.endItem(); }
    void                cont() { mrOut.contItem(); }
private:
                        ItemGuard( const ItemGuard& ) = delete;
    ItemGuard&          operator=( const ItemGuard& ) = delete;
private:
    Output&             mrOut;
};


class MultiItemsGuard
{
public:
    explicit            MultiItemsGuard( const OutputRef& rxOut ) : mrOut( *rxOut ) { mrOut.startMultiItems(); }
                        ~MultiItemsGuard() { mrOut.endMultiItems(); }
private:
                        MultiItemsGuard( const MultiItemsGuard& ) = delete;
    MultiItemsGuard&    operator=( const MultiItemsGuard& ) = delete;
private:
    Output&             mrOut;
};


class StorageIterator : public Base
{
public:
    explicit            StorageIterator( const StorageRef& rxStrg );
    virtual             ~StorageIterator();

    StorageIterator&    operator++();

    OUString     getName() const;
    bool                isStream() const;
    bool                isStorage() const;

private:
    virtual bool        implIsValid() const override;

private:
    StorageRef          mxStrg;
    OUStringVector      maNames;
    OUStringVector::const_iterator maIt;
};


class ObjectBase : public Base
{
public:
    virtual             ~ObjectBase();

    const css::uno::Reference< css::uno::XComponentContext >&
                        getContext() const { return mxConfig->getContext(); }

    void                dump();


protected:
                        ObjectBase() {}

    void                construct( const ConfigRef& rxConfig );
    void                construct( const ObjectBase& rParent );

    virtual bool        implIsValid() const override;
    virtual void        implDump();


    Config&             cfg() const { return *mxConfig; }

private:
    ConfigRef           mxConfig;
};

typedef std::shared_ptr< ObjectBase > ObjectRef;


class StorageObjectBase : public ObjectBase
{
protected:
                        StorageObjectBase() {}

protected:
    using               ObjectBase::construct;
    void                construct( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath );
    void                construct( const ObjectBase& rParent );

    virtual bool        implIsValid() const override;
    virtual void        implDump() override;

    virtual void        implDumpStream(
                            const css::uno::Reference< css::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );

    virtual void        implDumpStorage(
                            const StorageRef& rxStrg,
                            const OUString& rStrgPath,
                            const OUString& rSysPath );

    virtual void        implDumpBaseStream(
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName );

    void                addPreferredStream( const String& rStrmName );
    void                addPreferredStorage( const String& rStrgPath );

private:
    static OUString     getSysFileName(
                            const OUString& rStrmName,
                            const OUString& rSysOutPath );

    void                extractStream(
                            StorageBase& rStrg,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );
    void                extractStorage(
                            const StorageRef& rxStrg,
                            const OUString& rStrgPath,
                            const OUString& rSysPath );

    void                extractItem(
                            const StorageRef& rxStrg,
                            const OUString& rStrgPath,
                            const OUString& rItemName,
                            const OUString& rSysPath,
                            bool bIsStrg, bool bIsStrm );

private:
    struct PreferredItem
    {
        OUString     maName;
        bool                mbStorage;

        explicit     PreferredItem( const OUString& rName, bool bStorage ) :
                                maName( rName ), mbStorage( bStorage ) {}
    };
    typedef ::std::vector< PreferredItem > PreferredItemVector;

    StorageRef          mxStrg;
    OUString     maSysPath;
    PreferredItemVector maPreferred;
};


class OutputObjectBase : public ObjectBase
{
public:
    virtual             ~OutputObjectBase();


protected:
                        OutputObjectBase() {}

    using               ObjectBase::construct;
    void                construct( const ObjectBase& rParent, const OUString& rSysFileName );
    void                construct( const OutputObjectBase& rParent );

    virtual bool        implIsValid() const override;


    void                writeEmptyItem( const String& rName );
    void                writeInfoItem( const String& rName, const String& rData );
    void                writeCharItem( const String& rName, sal_Unicode cData );
    void                writeStringItem( const String& rName, const OUString& rData );
    void                writeArrayItem( const String& rName, const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    void                writeDateTimeItem( const String& rName, const css::util::DateTime& rDateTime );
    void                writeGuidItem( const String& rName, const OUString& rGuid );

    template< typename Type >
    void                addNameToItem( Type nData, const NameListWrapper& rListWrp );

    template< typename Type >
    void                writeNameItem( const String& rName, Type nData, const NameListWrapper& rListWrp );
    template< typename Type >
    void                writeDecItem( const String& rName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeHexItem( const String& rName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeShortHexItem( const String& rName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeBinItem( const String& rName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeFixItem( const String& rName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeDecBoolItem( const String& rName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                writeValueItem( const String& rName, Type nData, FormatType eFmtType, const NameListWrapper& rListWrp = NO_LIST );

    template< typename Type >
    void                writeValueItem( const ItemFormat& rItemFmt, Type nData );

    template< typename Type >
    void                writeDecPairItem( const String& rName, Type nData1, Type nData2, sal_Unicode cSep = ',' );
    template< typename Type >
    void                writeHexPairItem( const String& rName, Type nData1, Type nData2, sal_Unicode cSep = ',' );

protected:
    OutputRef           mxOut;
    OUString     maSysFileName;
};


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
void OutputObjectBase::writeNameItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeName( cfg(), nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeDecItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeDec( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeHexItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeHex( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeShortHexItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeShortHex( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeBinItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeBin( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeFixItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeFix( nData );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeDecBoolItem( const String& rName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeDec( nData );
    aItem.cont();
    mxOut->writeBool( nData != 0 );
    addNameToItem( nData, rListWrp );
}

template< typename Type >
void OutputObjectBase::writeValueItem( const String& rName, Type nData, FormatType eFmtType, const NameListWrapper& rListWrp )
{
    if( eFmtType == FORMATTYPE_BOOL )
        writeDecBoolItem( rName, nData, rListWrp );
    else
    {
        ItemGuard aItem( mxOut, rName );
        mxOut->writeValue( nData, eFmtType );
        addNameToItem( nData, rListWrp );
    }
}

template< typename Type >
void OutputObjectBase::writeValueItem( const ItemFormat& rItemFmt, Type nData )
{
    OString aNameUtf8 = StringHelper::convertToUtf8( rItemFmt.maItemName );
    writeValueItem( aNameUtf8.getStr(), nData, rItemFmt.meFmtType, rItemFmt.maListName );
}

template< typename Type >
void OutputObjectBase::writeDecPairItem( const String& rName, Type nData1, Type nData2, sal_Unicode cSep )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeDec( nData1 );
    mxOut->writeChar( cSep );
    mxOut->writeDec( nData2 );
}

template< typename Type >
void OutputObjectBase::writeHexPairItem( const String& rName, Type nData1, Type nData2, sal_Unicode cSep )
{
    ItemGuard aItem( mxOut, rName );
    mxOut->writeHex( nData1 );
    mxOut->writeChar( cSep );
    mxOut->writeHex( nData2 );
}


class InputObjectBase : public OutputObjectBase
{
public:
    virtual             ~InputObjectBase();


protected:
                        InputObjectBase() {}

    using               OutputObjectBase::construct;
    void                construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName );
    void                construct( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm );
    void                construct( const InputObjectBase& rParent );

    virtual bool        implIsValid() const override;

    void                skipBlock( sal_Int64 nBytes, bool bShowSize = true );
    void                dumpRawBinary( sal_Int64 nBytes, bool bShowOffset = true, bool bStream = false );

    void                dumpBinary( const String& rName, sal_Int64 nBytes, bool bShowOffset = true );
    void                dumpRemaining( sal_Int64 nBytes );
    void                dumpRemainingTo( sal_Int64 nPos );
    void                dumpRemainingStream();

    void                dumpArray( const String& rName, sal_Int32 nBytes, sal_Unicode cSep = OOX_DUMP_LISTSEP );
    void                dumpUnused( sal_Int32 nBytes ) { dumpArray( OOX_DUMP_UNUSED, nBytes ); }
    void                dumpUnknown( sal_Int32 nBytes ) { dumpArray( OOX_DUMP_UNKNOWN, nBytes ); }

    sal_Unicode         dumpUnicode( const String& rName );

    OUString     dumpCharArray( const String& rName, sal_Int32 nLen, rtl_TextEncoding eTextEnc, bool bHideTrailingNul = false );
    OUString     dumpUnicodeArray( const String& rName, sal_Int32 nLen, bool bHideTrailingNul = false );

    css::util::DateTime dumpFileTime( const String& rName = EMPTY_STRING );
    OUString     dumpGuid( const String& rName = EMPTY_STRING );

    void                dumpItem( const ItemFormat& rItemFmt );

    template< typename Type >
    Type                dumpName( const String& rName, const NameListWrapper& rListWrp );
    template< typename Type >
    Type                dumpDec( const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpHex( const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpBin( const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpFix( const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpBool( const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpValue( const ItemFormat& rItemFmt );

    template< typename Type1, typename Type2 >
    Type1               dumpName( bool bType1, const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpDec( bool bType1, const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpHex( bool bType1, const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpBin( bool bType1, const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpFix( bool bType1, const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpBool( bool bType1, const String& rName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               dumpValue( bool bType1, const ItemFormat& rItemFmt );

    template< typename Type >
    void                dumpDecPair( const String& rName, sal_Unicode cSep = ',' );
    template< typename Type >
    void                dumpHexPair( const String& rName, sal_Unicode cSep = ',' );

protected:
    BinaryInputStreamRef mxStrm;
};


template< typename Type >
Type InputObjectBase::dumpName( const String& rName, const NameListWrapper& rListWrp )
{
    Type nData = mxStrm->readValue<Type>();
    writeNameItem( rName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpDec( const String& rName, const NameListWrapper& rListWrp )
{
    Type nData = mxStrm->readValue<Type>();
    writeDecItem( rName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpHex( const String& rName, const NameListWrapper& rListWrp )
{
    Type nData = mxStrm->readValue<Type>();
    writeHexItem( rName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpBin( const String& rName, const NameListWrapper& rListWrp )
{
    Type nData = mxStrm->readValue<Type>();
    writeBinItem( rName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpFix( const String& rName, const NameListWrapper& rListWrp )
{
    Type nData = mxStrm->readValue<Type>();
    writeFixItem( rName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpBool( const String& rName, const NameListWrapper& rListWrp )
{
    Type nData = mxStrm->readValue<Type>();
    writeDecBoolItem( rName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::dumpValue( const ItemFormat& rItemFmt )
{
    Type nData = mxStrm->readValue<Type>();
    writeValueItem( rItemFmt, nData );
    return nData;
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpName( bool bType1, const String& rName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpName< Type1 >( rName, rListWrp ) : static_cast< Type1 >( dumpName< Type2 >( rName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpDec( bool bType1, const String& rName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpDec< Type1 >( rName, rListWrp ) : static_cast< Type1 >( dumpDec< Type2 >( rName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpHex( bool bType1, const String& rName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpHex< Type1 >( rName, rListWrp ) : static_cast< Type1 >( dumpHex< Type2 >( rName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpBin( bool bType1, const String& rName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpBin< Type1 >( rName, rListWrp ) : static_cast< Type1 >( dumpBin< Type2 >( rName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpFix( bool bType1, const String& rName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpFix< Type1 >( rName, rListWrp ) : static_cast< Type1 >( dumpFix< Type2 >( rName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpBool( bool bType1, const String& rName, const NameListWrapper& rListWrp )
{
    return bType1 ? dumpBool< Type1 >( rName, rListWrp ) : static_cast< Type1 >( dumpBool< Type2 >( rName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::dumpValue( bool bType1, const ItemFormat& rItemFmt )
{
    return bType1 ? dumpValue< Type1 >( rItemFmt ) : static_cast< Type1 >( dumpValue< Type2 >( rItemFmt ) );
}

template< typename Type >
void InputObjectBase::dumpDecPair( const String& rName, sal_Unicode cSep )
{
    Type nData1, nData2;
    *mxStrm >> nData1 >> nData2;
    writeDecPairItem( rName, nData1, nData2, cSep );
}

template< typename Type >
void InputObjectBase::dumpHexPair( const String& rName, sal_Unicode cSep )
{
    Type nData1, nData2;
    *mxStrm >> nData1 >> nData2;
    writeHexPairItem( rName, nData1, nData2, cSep );
}


class BinaryStreamObject : public InputObjectBase
{
public:
    explicit            BinaryStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName );

protected:
    void                dumpBinaryStream( bool bShowOffset = true );

    virtual void        implDump() override;
};


class TextStreamObjectBase : public InputObjectBase
{
protected:
                        TextStreamObjectBase() {}

    using               InputObjectBase::construct;
    void                construct(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            rtl_TextEncoding eTextEnc,
                            const OUString& rSysFileName );
    void                construct(
                            const OutputObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            rtl_TextEncoding eTextEnc );

    virtual bool        implIsValid() const override;
    virtual void        implDump() override;

    virtual void        implDumpText( TextInputStream& rTextStrm ) = 0;

private:
    void                constructTextStrmObj( rtl_TextEncoding eTextEnc );

protected:
    std::shared_ptr< TextInputStream > mxTextStrm;
};


class TextLineStreamObject : public TextStreamObjectBase
{
public:
    explicit            TextLineStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            rtl_TextEncoding eTextEnc,
                            const OUString& rSysFileName );

    explicit            TextLineStreamObject(
                            const OutputObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            rtl_TextEncoding eTextEnc );

protected:
    virtual void        implDumpText( TextInputStream& rTextStrm ) override;
    void        implDumpLine( const OUString& rLine, sal_uInt32 nLine );
};


class XmlStreamObject : public TextStreamObjectBase
{
public:
    explicit            XmlStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName );

protected:
    virtual void        implDumpText( TextInputStream& rTextStrm ) override;
};


class RecordObjectBase : public InputObjectBase
{
protected:
                        RecordObjectBase() {}

    using               InputObjectBase::construct;
    void                construct(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxBaseStrm,
                            const OUString& rSysFileName,
                            const BinaryInputStreamRef& rxRecStrm,
                            const String& rRecNames,
                            const String& rSimpleRecs = EMPTY_STRING );

    sal_Int64           getRecId() const { return mnRecId; }
    sal_Int64           getRecSize() const { return mnRecSize; }
    NameListRef         getRecNames() const { return maRecNames.getNameList( cfg() ); }

    virtual bool        implIsValid() const override;
    virtual void        implDump() override;

    virtual bool        implStartRecord( BinaryInputStream& rBaseStrm, sal_Int64& ornRecPos, sal_Int64& ornRecId, sal_Int64& ornRecSize ) = 0;
    virtual void        implWriteExtHeader();
    virtual void        implDumpRecordBody();

private:
    void                constructRecObjBase(
                            const BinaryInputStreamRef& rxBaseStrm,
                            const String& rRecNames,
                            const String& rSimpleRecs );

    void                writeHeader();

private:
    BinaryInputStreamRef mxBaseStrm;
    NameListWrapper     maRecNames;
    NameListWrapper     maSimpleRecs;
    sal_Int64           mnRecPos;
    sal_Int64           mnRecId;
    sal_Int64           mnRecSize;
    bool                mbShowRecPos;
    bool                mbBinaryOnly;
};


class SequenceRecordObjectBase : public RecordObjectBase
{
protected:
                        SequenceRecordObjectBase() : mxRecData( new StreamDataSequence ) {}

    using               RecordObjectBase::construct;
    void                construct(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxBaseStrm,
                            const OUString& rSysFileName,
                            const String& rRecNames,
                            const String& rSimpleRecs = EMPTY_STRING );

    virtual bool        implStartRecord( BinaryInputStream& rBaseStrm, sal_Int64& ornRecPos, sal_Int64& ornRecId, sal_Int64& ornRecSize ) override;
    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize ) = 0;

private:
    typedef std::shared_ptr< StreamDataSequence > StreamDataSeqRef;
    StreamDataSeqRef    mxRecData;
};


/** Base class for a file dumper. Derived classes implement the implDump()
    function to add functionality.
 */
class DumperBase : public ObjectBase
{
public:
    virtual             ~DumperBase();

    bool                isImportEnabled() const;
    bool                isImportCancelled() const;

protected:
                        DumperBase() {}

    using               ObjectBase::construct;
    void                construct( const ConfigRef& rxConfig );
};


} // namespace dump
} // namespace oox

#define OOX_DUMP_FILE( DumperClassName )            \
do {                                                \
    DumperClassName aDumper( *this );               \
    aDumper.dump();                                 \
    bool bCancelled = aDumper.isImportCancelled();  \
    if( !aDumper.isImportEnabled() || bCancelled )  \
        return aDumper.isValid() && !bCancelled;    \
} while( false )

#else   // OOX_INCLUDE_DUMPER

#define OOX_DUMP_FILE( DumperClassName ) (void)0

#endif  // OOX_INCLUDE_DUMPER
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
