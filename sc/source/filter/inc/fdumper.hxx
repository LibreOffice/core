/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fdumper.hxx,v $
 * $Revision: 1.8.28.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_FDUMPER_HXX
#define SC_FDUMPER_HXX

#define SCF_INCL_DUMPER (OSL_DEBUG_LEVEL > 0) && 0

#include <map>
// #include <math.h>
#include "ftools.hxx"

#if SCF_INCL_DUMPER

class SvStream;
class SfxMedium;
class SfxObjectShell;
class Color;
class DateTime;

namespace scf {
namespace dump {

#define SCF_DUMP_UNUSED                 "unused"
#define SCF_DUMP_UNKNOWN                "?unknown"

#define SCF_DUMP_ERRASCII( ascii )      "?err:" ascii
#define SCF_DUMP_ERRSTRING( ascii )     CREATE_STRING( SCF_DUMP_ERRASCII( ascii ) )

#define SCF_DUMP_ERR_NOMAP              SCF_DUMP_ERRSTRING( "no-map" )
#define SCF_DUMP_ERR_NONAME             SCF_DUMP_ERRSTRING( "no-name" )
#define SCF_DUMP_ERR_STREAM             SCF_DUMP_ERRSTRING( "stream-error" )

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
    maItemName and maItemNameUtf8. If the name is empty, only the value is
    written (without a leading equality sign).

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
    String              maItemName;         /// Name of the item.
    ByteString          maItemNameUtf8;     /// Name of the item, converted to UTF-8.
    String              maListName;         /// Name of a name list to be used for this item.

    explicit            ItemFormat();

    void                Set( DataType eDataType, FormatType eFmtType, const String& rItemName );
    void                Set( DataType eDataType, FormatType eFmtType, const String& rItemName, const String& rListName );

    /** Initializes the struct from a vector of strings containing the item format.

        The vector must contain at least 2 strings. The struct is filled from
        the strings in the vector in the following order:
        1) Data type (one of: [u]int8, [u]int16, [u]int32, [u]int64, float, double).
        2) Format type (one of: dec, hex, bin, fix, bool, unused, unknown).
        3) Item name (optional).
        4) Name list name (optional).

        @return  Iterator pointing to the first unhandled string.
     */
    ScfStringVec::const_iterator Parse( const ScfStringVec& rFormatVec );

    /** Initializes the struct from a string containing the item format.

        The string must have the following format:
        DATATYPE,FORMATTYPE[,ITEMNAME[,LISTNAME]]

        DATATYPE is the data type of the item (see above for possible values).
        FORMATTYPE is the format type of the item (see above for possible values).
        ITEMNAME is the name of the item (optional).
        LISTNAME is the name of a name list (optional).

        @return  List containing remaining unhandled format strings.
     */
    ScfStringVec        Parse( const String& rFormatStr );
};

// ============================================================================
// ============================================================================

/** Static helper functions for formatted output to strings. */
class StringHelper
{
public:
    // append string to string ------------------------------------------------

    static void         AppendString( String& rStr, const String& rData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );

    // append decimal ---------------------------------------------------------

    static void         AppendDec( String& rStr, sal_uInt8  nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_Int8   nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_uInt16 nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_Int16  nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_uInt32 nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_Int32  nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_uInt64 nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, sal_Int64  nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );
    static void         AppendDec( String& rStr, double     fData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' );

    // append hexadecimal -----------------------------------------------------

    static void         AppendHex( String& rStr, sal_uInt8  nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_Int8   nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_uInt16 nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_Int16  nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_uInt32 nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_Int32  nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_uInt64 nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, sal_Int64  nData, bool bPrefix = true );
    static void         AppendHex( String& rStr, double     fData, bool bPrefix = true );

    // append shortened hexadecimal -------------------------------------------

    static void         AppendShortHex( String& rStr, sal_uInt8  nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_Int8   nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_uInt16 nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_Int16  nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_uInt32 nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_Int32  nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_uInt64 nData, bool bPrefix = true );
    static void         AppendShortHex( String& rStr, sal_Int64  nData, bool bPrefix = true );

    // append binary ----------------------------------------------------------

    static void         AppendBin( String& rStr, sal_uInt8  nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_Int8   nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_uInt16 nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_Int16  nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_uInt32 nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_Int32  nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_uInt64 nData, bool bDots = true );
    static void         AppendBin( String& rStr, sal_Int64  nData, bool bDots = true );
    static void         AppendBin( String& rStr, double     fData, bool bDots = true );

    // append fixed-point decimal ---------------------------------------------

    template< typename Type >
    static void         AppendFix( String& rStr, Type nData, xub_StrLen nWidth = 0 );

    // append formatted value -------------------------------------------------

    static void         AppendBool( String& rStr, bool bData );
    template< typename Type >
    static void         AppendValue( String& rStr, Type nData, FormatType eFmtType );

    // encoded text output ----------------------------------------------------

    static void         AppendCChar( String& rStr, sal_Unicode cChar, bool bPrefix = true );
    static void         AppendEncChar( String& rStr, sal_Unicode cChar, xub_StrLen nCount = 1, bool bPrefix = true );
    static void         AppendEncString( String& rStr, const String& rData, bool bPrefix = true );

    // token list -------------------------------------------------------------

    static void         AppendToken( String& rStr, const String& rToken, sal_Unicode cSep = ',' );
    static void         AppendToken( String& rStr, sal_Int64 nToken, sal_Unicode cSep = ',' );
    static void         PrependToken( String& rStr, const String& rToken, sal_Unicode cSep = ',' );
    static void         PrependToken( String& rStr, sal_Int64 nToken, sal_Unicode cSep = ',' );

    static void         AppendIndex( String& rStr, const String& rIdx );
    static void         AppendIndex( String& rStr, sal_Int64 nIdx );
    static void         AppendIndexedText( String& rStr, const String& rData, const String& rIdx );
    static void         AppendIndexedText( String& rStr, const String& rData, sal_Int64 nIdx );

    static String       GetToken( const String& rData, xub_StrLen& rnPos, sal_Unicode cSep = ',' );

    // quoting and trimming ---------------------------------------------------

    /** Encloses the passed string with the passed characters. Uses cOpen, if cClose is NUL. */
    static void         Enclose( String& rStr, sal_Unicode cOpen, sal_Unicode cClose = '\0' );

    static void         TrimSpaces( String& rStr );
    static void         TrimQuotes( String& rStr );

    // string conversion ------------------------------------------------------

    static ByteString   ConvertToUtf8( const String& rStr );
    static DataType     ConvertToDataType( const String& rStr );
    static FormatType   ConvertToFormatType( const String& rStr );

    static bool         ConvertFromDec( sal_Int64& rnData, const String& rData );
    static bool         ConvertFromHex( sal_Int64& rnData, const String& rData );

    static bool         ConvertStringToInt( sal_Int64& rnData, const String& rData );
    static bool         ConvertStringToDouble( double& rfData, const String& rData );
    static bool         ConvertStringToBool( const String& rData );

    // string to list conversion ----------------------------------------------

    static void         ConvertStringToStringList( ScfStringVec& rVec, const String& rData, bool bIgnoreEmpty );
    static void         ConvertStringToIntList( ScfInt64Vec& rVec, const String& rData, bool bIgnoreEmpty );

    // file names -------------------------------------------------------------

    static String       ConvertFileNameToUrl( const String& rFileName );
    static xub_StrLen   GetFileNamePos( const String& rFileUrl );
};

// ----------------------------------------------------------------------------

template< typename Type >
void StringHelper::AppendFix( String& rStr, Type nData, xub_StrLen nWidth )
{
    AppendDec( rStr, static_cast< double >( nData ) / pow( 2.0, 4.0 * sizeof( Type ) ), nWidth );
}

template< typename Type >
void StringHelper::AppendValue( String& rStr, Type nData, FormatType eFmtType )
{
    switch( eFmtType )
    {
        case FORMATTYPE_DEC:        AppendDec( rStr, nData );   break;
        case FORMATTYPE_HEX:        AppendHex( rStr, nData );   break;
        case FORMATTYPE_BIN:        AppendBin( rStr, nData );   break;
        case FORMATTYPE_FIX:        AppendFix( rStr, nData );   break;
        case FORMATTYPE_BOOL:       AppendBool( rStr, nData );  break;
        default:;
    }
}

// ============================================================================

class StringWrapper
{
public:
    inline              StringWrapper() {}
    inline /*implicit*/ StringWrapper( const String& rStr ) : maStr( rStr ) {}
    inline /*implicit*/ StringWrapper( const sal_Char* pcStr ) : maStr( String::CreateFromAscii( pcStr ? pcStr : "" ) ) {}
    inline /*implicit*/ StringWrapper( sal_Unicode cChar ) : maStr( cChar ) {}

    inline bool         Empty() const { return maStr.Len() == 0; }
    inline const String& GetString() const { return maStr; }

private:
    String              maStr;
};

// ============================================================================
// ============================================================================

class Base;
typedef ScfRef< Base > BaseRef;

/** Base class for all dumper classes.

    Derived classes implement the virtual function ImplIsValid(). It should
    check all members the other functions rely on. If the function
    ImplIsValid() returns true, all references and pointers can be used without
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
    +---->  ConfigCoreData
    +---->  Config
    +---->  CoreData
    |
    +---->  Input  ------>  SvStreamInput
    +---->  Output
    |
    +---->  ObjectBase
            |
            +---->  InputObjectBase
            |       |
            |       +---->  StreamObjectBase
            |       |       |
            |       |       +---->  SvStreamObject
            |       |       +---->  OleStreamObject
            |       |       +---->  WrappedStreamObject
            |       |
            |       +---->  RecordHeaderBase
            |
            +---->  OleStorageObject
            +---->  DumperBase
 */
class Base
{
public:
    virtual             ~Base();

    inline bool         IsValid() const { return ImplIsValid(); }
    inline static bool  IsValid( BaseRef xBase ) { return xBase.is() && xBase->IsValid(); }

protected:
    inline explicit     Base() {}

    virtual bool        ImplIsValid() const = 0;
};

// ============================================================================
// ============================================================================

class ConfigItemBase
{
public:
    virtual             ~ConfigItemBase();
    void                ReadConfigBlock( SvStream& rStrm );

protected:
    inline explicit     ConfigItemBase() {}

    virtual void        ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData );
    virtual void        ImplProcessConfigItemInt( SvStream& rStrm, sal_Int64 nKey, const String& rData );

    void                ReadConfigBlockContents( SvStream& rStrm );
    void                IgnoreConfigBlockContents( SvStream& rStrm );

private:
    enum LineType { LINETYPE_DATA, LINETYPE_BEGIN, LINETYPE_END };

    LineType            ReadConfigLine( SvStream& rStrm, String& rKey, String& rData ) const;
    LineType            ReadConfigLine( SvStream& rStrm ) const;
    void                ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData );
};

// ============================================================================

class ConfigCoreData;
class Config;

class NameListBase;
typedef ScfRef< NameListBase > NameListRef;

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
    typedef ::std::map< sal_Int64, String > StringMap;
    typedef StringMap::const_iterator       const_iterator;

    virtual             ~NameListBase();

    /** Sets a name for the specified key. */
    void                SetName( sal_Int64 nKey, const StringWrapper& rNameWrp );
    /** Include all names of the passed list. */
    void                IncludeList( NameListRef xList );

    /** Returns true, if the map contains an entry for the passed key. */
    template< typename Type >
    inline bool         HasName( Type nKey ) const
        { return maMap.count( static_cast< sal_Int64 >( nKey ) ) != 0; }
    /** Returns the name for the passed key. */
    template< typename Type >
    inline String       GetName( const Config& rCfg, Type nKey ) const
        { return ImplGetName( rCfg, static_cast< sal_Int64 >( nKey ) ); }
    /** Returns a display name for the passed double value. */
    inline String       GetName( const Config& rCfg, double fValue ) const
        { return ImplGetNameDbl( rCfg, fValue ); }

    /** Returns a map iterator pointing to the first contained name. */
    inline const_iterator begin() const { return maMap.begin(); }
    /** Returns a map iterator pointing one past the last contained name. */
    inline const_iterator end() const { return maMap.end(); }

protected:
    inline explicit     NameListBase( const ConfigCoreData& rCoreData ) : mrCoreData( rCoreData ) {}

    virtual bool        ImplIsValid() const;
    virtual void        ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData );
    virtual void        ImplProcessConfigItemInt( SvStream& rStrm, sal_Int64 nKey, const String& rData );

    /** Derived classes set the name for the passed key. */
    virtual void        ImplSetName( sal_Int64 nKey, const String& rName ) = 0;
    /** Derived classes generate and return the name for the passed key. */
    virtual String      ImplGetName( const Config& rCfg, sal_Int64 nKey ) const = 0;
    /** Derived classes generate and return the name for the passed double value. */
    virtual String      ImplGetNameDbl( const Config& rCfg, double fValue ) const = 0;
    /** Derived classes insert all names and other settings from the passed list. */
    virtual void        ImplIncludeList( const NameListBase& rList ) = 0;

    /** Returns the configuration core data, which can be used to access other name lists. */
    inline const ConfigCoreData& GetCoreData() const { return mrCoreData; }
    /** Inserts the passed name into the internal map. */
    void                InsertRawName( sal_Int64 nKey, const String& rName );
    /** Returns the name for the passed key, or 0, if nothing found. */
    const String*       FindRawName( sal_Int64 nKey ) const;

private:
    /** Includes name lists, given in a comma separated list of names of the lists. */
    void                Include( const String& rListKeys );
    /** Excludes names from the list, given by a comma separated list of their keys. */
    void                Exclude( const String& rKeys );

private:
    StringMap           maMap;
    const ConfigCoreData& mrCoreData;
};

// ============================================================================

class ConstList : public NameListBase
{
public:
    explicit            ConstList( const ConfigCoreData& rCoreData );

    /** Sets a default name for unknown keys. */
    inline void         SetDefaultName( const StringWrapper& rDefName ) { maDefName = rDefName.GetString(); }
    /** Enables or disables automatic quotation of returned names. */
    inline void         SetQuoteNames( bool bQuoteNames ) { mbQuoteNames = bQuoteNames; }

protected:
    virtual void        ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData );

    /** Sets the name for the passed key. */
    virtual void        ImplSetName( sal_Int64 nKey, const String& rName );
    /** Returns the name for the passed key, or the default name, if key is not contained. */
    virtual String      ImplGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Returns the name for the passed double value. */
    virtual String      ImplGetNameDbl( const Config& rCfg, double fValue ) const;
    /** Inserts all names from the passed list. */
    virtual void        ImplIncludeList( const NameListBase& rList );

private:
    String              maDefName;
    bool                mbQuoteNames;
};

// ============================================================================

class MultiList : public ConstList
{
public:
    explicit            MultiList( const ConfigCoreData& rCoreData );

    void                SetNamesFromVec( sal_Int64 nStartKey, const ScfStringVec& rNames );

protected:
    virtual void        ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData );
    virtual void        ImplSetName( sal_Int64 nKey, const String& rName );

private:
    void                InsertNames( sal_Int64 nStartKey, const String& rData );

private:
    bool                mbIgnoreEmpty;
};

// ============================================================================

class FlagsList : public NameListBase
{
public:
    explicit            FlagsList( const ConfigCoreData& rCoreData );

    /** Sets flags to be ignored on output. */
    template< typename Type >
    inline void         SetIgnoreFlags( Type nIgnore )
        { mnIgnore = static_cast< sal_Int64 >( nIgnore ); }

protected:
    virtual void        ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData );

    /** Sets the name for the passed key. */
    virtual void        ImplSetName( sal_Int64 nKey, const String& rName );
    /** Returns the name for the passed key. */
    virtual String      ImplGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Returns the name for the passed double value. */
    virtual String      ImplGetNameDbl( const Config& rCfg, double fValue ) const;
    /** Inserts all flags from the passed list. */
    virtual void        ImplIncludeList( const NameListBase& rList );

private:
    sal_Int64           mnIgnore;
};

// ============================================================================

class CombiList : public FlagsList
{
public:
    explicit            CombiList( const ConfigCoreData& rCoreData );

protected:
    /** Sets the name for the passed key. */
    virtual void        ImplSetName( sal_Int64 nKey, const String& rName );
    /** Returns the name for the passed key. */
    virtual String      ImplGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Inserts all flags from the passed list. */
    virtual void        ImplIncludeList( const NameListBase& rList );

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
    explicit            UnitConverter( const ConfigCoreData& rCoreData );

    inline void         SetUnitName( const StringWrapper& rUnitName ) { maUnitName = rUnitName.GetString(); }
    inline void         SetFactor( double fFactor ) { mfFactor = fFactor; }

protected:
    /** Sets the name for the passed key. */
    virtual void        ImplSetName( sal_Int64 nKey, const String& rName );
    /** Returns the converted value with appended unit name. */
    virtual String      ImplGetName( const Config& rCfg, sal_Int64 nKey ) const;
    /** Returns the converted value with appended unit name. */
    virtual String      ImplGetNameDbl( const Config& rCfg, double fValue ) const;
    /** Empty implementation. */
    virtual void        ImplIncludeList( const NameListBase& rList );

private:
    String              maUnitName;
    double              mfFactor;
};

// ============================================================================

class NameListWrapper
{
public:
    inline              NameListWrapper() {}
    inline /*implicit*/ NameListWrapper( const String& rListName ) : maNameWrp( rListName ) {}
    inline /*implicit*/ NameListWrapper( const sal_Char* pcListName ) : maNameWrp( pcListName ) {}
    inline /*implicit*/ NameListWrapper( NameListRef xList ) : mxList( xList ) {}

    inline bool         Empty() const { return !mxList && maNameWrp.Empty(); }
    NameListRef         GetNameList( const Config& rCfg ) const;

private:
    StringWrapper       maNameWrp;
    mutable NameListRef mxList;
};

static const NameListWrapper NO_LIST;

// ============================================================================
// ============================================================================

class ConfigCoreData : public Base, public ConfigItemBase
{
public:
    explicit            ConfigCoreData( const String& rFileName );
    virtual             ~ConfigCoreData();

    void                SetOption( const String& rKey, const String& rData );
    const String*       GetOption( const String& rKey ) const;

    template< typename ListType >
    ScfRef< ListType >  CreateNameList( const String& rListName );
    void                SetNameList( const String& rListName, NameListRef xList );
    void                EraseNameList( const String& rListName );
    NameListRef         GetNameList( const String& rListName ) const;

protected:
    inline explicit     ConfigCoreData() : mbLoaded( false ) {}
    void                Construct( const String& rFileName );

    virtual bool        ImplIsValid() const;
    virtual void        ImplProcessConfigItemStr( SvStream& rStrm, const String& rKey, const String& rData );

private:
    void                ConstructCfgCoreData();

    bool                ReadConfigFile( const String& rFileUrl );
    template< typename ListType >
    void                ReadNameList( SvStream& rStrm, const String& rListName );
    void                CreateShortList( const String& rData );
    void                CreateUnitConverter( const String& rData );

private:
    typedef ::std::map< String, String >        ConfigDataMap;
    typedef ::std::map< String, NameListRef >   NameListMap;

    ConfigDataMap       maConfigData;
    NameListMap         maNameLists;
    String              maConfigPath;
    bool                mbLoaded;
};

// ----------------------------------------------------------------------------

template< typename ListType >
ScfRef< ListType > ConfigCoreData::CreateNameList( const String& rListName )
{
    ScfRef< ListType > xList;
    if( rListName.Len() > 0 )
    {
        xList.reset( new ListType( *this ) );
        SetNameList( rListName, xList );
    }
    return xList;
}

template< typename ListType >
void ConfigCoreData::ReadNameList( SvStream& rStrm, const String& rListName )
{
    NameListRef xList = CreateNameList< ListType >( rListName );
    if( xList.is() )
        xList->ReadConfigBlock( rStrm );
}

// ============================================================================

class Config : public Base
{
public:
    explicit            Config( const Config& rParent );
    explicit            Config( const String& rFileName );
    explicit            Config( const sal_Char* pcEnvVar );
    virtual             ~Config();

    void                SetStringOption( const StringWrapper& rKey, const StringWrapper& rData );

    const String&       GetStringOption( const StringWrapper& rKey, const String& rDefault ) const;
    bool                GetBoolOption( const StringWrapper& rKey, bool bDefault ) const;
    template< typename Type >
    Type                GetIntOption( const StringWrapper& rKey, Type nDefault ) const;

    bool                IsDumperEnabled() const;
    bool                IsImportEnabled() const;

    template< typename ListType >
    ScfRef< ListType >  CreateNameList( const StringWrapper& rListName );
    void                SetNameList( const StringWrapper& rListName, NameListRef xList );
    void                EraseNameList( const StringWrapper& rListName );
    NameListRef         GetNameList( const StringWrapper& rListName ) const;

    /** Returns the name for the passed key from the passed name list. */
    template< typename Type >
    String              GetName( const NameListWrapper& rListWrp, Type nKey ) const;
    /** Returns true, if the passed name list contains an entry for the passed key. */
    template< typename Type >
    bool                HasName( const NameListWrapper& rList, Type nKey ) const;

protected:
    inline explicit     Config() {}
    void                Construct( const Config& rParent );
    void                Construct( const String& rFileName );
    void                Construct( const sal_Char* pcEnvVar );

    virtual bool        ImplIsValid() const;
    virtual const String* ImplGetOption( const String& rKey ) const;
    virtual NameListRef ImplGetNameList( const String& rListName ) const;

private:
    typedef ScfRef< ConfigCoreData > ConfigCoreDataRef;
    ConfigCoreDataRef   mxCoreData;
};

typedef ScfRef< Config > ConfigRef;

// ----------------------------------------------------------------------------

template< typename Type >
Type Config::GetIntOption( const StringWrapper& rKey, Type nDefault ) const
{
    sal_Int64 nRawData;
    const String* pData = ImplGetOption( rKey.GetString() );
    return (pData && StringHelper::ConvertStringToInt( nRawData, *pData )) ?
        static_cast< Type >( nRawData ) : nDefault;
}

template< typename ListType >
ScfRef< ListType > Config::CreateNameList( const StringWrapper& rListName )
{
    return mxCoreData->CreateNameList< ListType >( rListName.GetString() );
}

template< typename Type >
String Config::GetName( const NameListWrapper& rListWrp, Type nKey ) const
{
    NameListRef xList = rListWrp.GetNameList( *this );
    return xList.is() ? xList->GetName( *this, nKey ) : SCF_DUMP_ERR_NOMAP;
}

template< typename Type >
bool Config::HasName( const NameListWrapper& rListWrp, Type nKey ) const
{
    NameListRef xList = rListWrp.GetNameList( *this );
    return xList.is() && xList->HasName( nKey );
}

// ============================================================================
// ============================================================================

class CoreData : public Base
{
public:
    explicit            CoreData( SfxMedium& rMedium, SfxObjectShell* pDocShell );

    inline SfxMedium&   GetMedium() { return mrMedium; }
    inline SfxObjectShell* GetDocShell() { return mpDocShell; }
    SvStream&           GetCoreStream();

protected:
    virtual bool        ImplIsValid() const;

private:
    SfxMedium&          mrMedium;
    SfxObjectShell*     mpDocShell;
};

typedef ScfRef< CoreData > CoreDataRef;

// ============================================================================
// ============================================================================

class Input : public Base
{
public:
    virtual sal_Size    GetSize() const = 0;
    virtual sal_Size    Tell() const = 0;
    virtual void        Seek( sal_Size nPos ) = 0;
    virtual void        SeekRel( sal_sSize nRelPos ) = 0;
    virtual sal_Size    Read( void* pBuffer, sal_Size nSize ) = 0;
    virtual void        ReadLine( String& rLine, rtl_TextEncoding eEnc ) = 0;

    virtual Input&      operator>>( sal_Int8& rnData ) = 0;
    virtual Input&      operator>>( sal_uInt8& rnData ) = 0;
    virtual Input&      operator>>( sal_Int16& rnData ) = 0;
    virtual Input&      operator>>( sal_uInt16& rnData ) = 0;
    virtual Input&      operator>>( sal_Int32& rnData ) = 0;
    virtual Input&      operator>>( sal_uInt32& rnData ) = 0;
    virtual Input&      operator>>( float& rfData ) = 0;
    virtual Input&      operator>>( double& rfData ) = 0;

    inline bool         IsValidPos() const { return Tell() < GetSize(); }
    template< typename Type >
    inline Type         ReadValue() { Type nValue; *this >> nValue; return nValue; }

protected:
    virtual bool        ImplIsValid() const;
};

typedef ScfRef< Input > InputRef;

Input& operator>>( Input& rIn, sal_Int64& rnData );
Input& operator>>( Input& rIn, sal_uInt64& rnData );

// ============================================================================

class SvStreamInput : public Input
{
public:
    explicit            SvStreamInput( SvStream& rStrm );
    virtual             ~SvStreamInput();

    virtual sal_Size    GetSize() const;
    virtual sal_Size    Tell() const;
    virtual void        Seek( sal_Size nPos );
    virtual void        SeekRel( sal_sSize nRelPos );
    virtual sal_Size    Read( void* pBuffer, sal_Size nSize );
    virtual void        ReadLine( String& rLine, rtl_TextEncoding eEnc );

    virtual SvStreamInput& operator>>( sal_Int8& rnData );
    virtual SvStreamInput& operator>>( sal_uInt8& rnData );
    virtual SvStreamInput& operator>>( sal_Int16& rnData );
    virtual SvStreamInput& operator>>( sal_uInt16& rnData );
    virtual SvStreamInput& operator>>( sal_Int32& rnData );
    virtual SvStreamInput& operator>>( sal_uInt32& rnData );
    virtual SvStreamInput& operator>>( float& rfData );
    virtual SvStreamInput& operator>>( double& rfData );

private:
    SvStream&           mrStrm;
};

// ============================================================================
// ============================================================================

class Output : public Base
{
public:
    explicit            Output( SvStream& rStrm );

    // ------------------------------------------------------------------------

    void                NewLine();
    void                EmptyLine( size_t nCount = 1 );
    inline String&      GetLine() { return maLine; }

    void                SetPrefix( const String& rPrefix );
    inline const String& GetPrefix() const { return maPrefix; }

    void                IncIndent();
    void                DecIndent();
    void                ResetIndent();

    void                StartTable( xub_StrLen nW1 );
    void                StartTable( xub_StrLen nW1, xub_StrLen nW2 );
    void                StartTable( xub_StrLen nW1, xub_StrLen nW2, xub_StrLen nW3 );
    void                StartTable( xub_StrLen nW1, xub_StrLen nW2, xub_StrLen nW3, xub_StrLen nW4 );
    void                StartTable( size_t nColCount, const xub_StrLen* pnColWidths );
    void                Tab();
    void                Tab( size_t nCol );
    void                EndTable();

    void                ResetItemIndex( sal_Int64 nIdx = 0 );
    void                StartItem( const sal_Char* pcName = 0 );
    void                ContItem();
    void                EndItem();
    inline const String& GetLastItemValue() const { return maLastItem; }

    void                StartMultiItems();
    void                EndMultiItems();

    // ------------------------------------------------------------------------

    void                WriteChar( sal_Unicode cChar, xub_StrLen nCount = 1 );
    void                WriteAscii( const sal_Char* pcStr );
    void                WriteString( const String& rStr );
    void                WriteArray( const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep = ',' );
    void                WriteBool( bool bData );
    void                WriteColor( const Color& rColor );
    void                WriteDateTime( const DateTime& rDateTime );


    template< typename Type >
    inline void         WriteDec( Type nData, xub_StrLen nWidth = 0, sal_Unicode cFill = ' ' )
                            { StringHelper::AppendDec( maLine, nData, nWidth, cFill ); }
    template< typename Type >
    inline void         WriteHex( Type nData, bool bPrefix = true )
                            { StringHelper::AppendHex( maLine, nData, bPrefix ); }
    template< typename Type >
    inline void         WriteBin( Type nData, bool bDots = true )
                            { StringHelper::AppendBin( maLine, nData, bDots ); }
    template< typename Type >
    inline void         WriteFix( Type nData, xub_StrLen nWidth = 0 )
                            { StringHelper::AppendFix( maLine, nData, nWidth ); }
    template< typename Type >
    inline void         WriteValue( Type nData, FormatType eFmtType )
                            { StringHelper::AppendValue( maLine, nData, eFmtType ); }
    template< typename Type >
    inline void         WriteName( const Config& rCfg, Type nData, const NameListWrapper& rListWrp )
                            { WriteString( rCfg.GetName( rListWrp, nData ) ); }

    // ------------------------------------------------------------------------
protected:
    virtual bool        ImplIsValid() const;

private:
    void                WriteItemName( const sal_Char* pcName );

private:
    typedef ::std::vector< xub_StrLen > StringLenVec;

    SvStream&           mrStrm;
    String              maPrefix;
    ByteString          maPrefixUtf8;
    ByteString          maIndent;
    String              maLine;
    String              maLastItem;
    StringLenVec        maColPos;
    size_t              mnCol;
    size_t              mnItemLevel;
    size_t              mnMultiLevel;
    sal_Int64           mnItemIdx;
    xub_StrLen          mnLastItem;
};

typedef ScfRef< Output > OutputRef;

// ============================================================================

class PrefixGuard : private ScfNoCopy
{
public:
    inline explicit     PrefixGuard( Output& rOut, const String& rPrefix ) :
                            mrOut( rOut ), maOldPrefix( rOut.GetPrefix() ) { mrOut.SetPrefix( rPrefix ); }
    inline              ~PrefixGuard() { mrOut.SetPrefix( maOldPrefix ); }
private:
    Output&             mrOut;
    String              maOldPrefix;
};

// ----------------------------------------------------------------------------

class IndentGuard : private ScfNoCopy
{
public:
    inline explicit     IndentGuard( Output& rOut ) : mrOut( rOut ) { mrOut.IncIndent(); }
    inline              ~IndentGuard() { mrOut.DecIndent(); }
private:
    Output&             mrOut;
};

// ----------------------------------------------------------------------------

class TableGuard : private ScfNoCopy
{
public:
    inline explicit     TableGuard( Output& rOut, xub_StrLen nW1 ) :
                            mrOut( rOut ) { mrOut.StartTable( nW1 ); }
    inline explicit     TableGuard( Output& rOut, xub_StrLen nW1, xub_StrLen nW2 ) :
                            mrOut( rOut ) { mrOut.StartTable( nW1, nW2 ); }
    inline explicit     TableGuard( Output& rOut, xub_StrLen nW1, xub_StrLen nW2, xub_StrLen nW3 ) :
                            mrOut( rOut ) { mrOut.StartTable( nW1, nW2, nW3 ); }
    inline explicit     TableGuard( Output& rOut, xub_StrLen nW1, xub_StrLen nW2, xub_StrLen nW3, xub_StrLen nW4 ) :
                            mrOut( rOut ) { mrOut.StartTable( nW1, nW2, nW3, nW4 ); }
    inline explicit     TableGuard( Output& rOut, size_t nColCount,
                                const xub_StrLen* pnColWidths ) :
                            mrOut( rOut ) { mrOut.StartTable( nColCount, pnColWidths ); }
    inline              ~TableGuard() { mrOut.EndTable(); }
    inline void         Tab() { mrOut.Tab(); }
    inline void         Tab( size_t nCol ) { mrOut.Tab( nCol ); }
private:
    Output&             mrOut;
};

// ----------------------------------------------------------------------------

class ItemGuard : private ScfNoCopy
{
public:
    inline explicit     ItemGuard( Output& rOut, const sal_Char* pcName = 0 ) :
                            mrOut( rOut ) { mrOut.StartItem( pcName ); }
    inline              ~ItemGuard() { mrOut.EndItem(); }
    inline void         Cont() { mrOut.ContItem(); }
private:
    Output&             mrOut;
};

// ----------------------------------------------------------------------------

class MultiItemsGuard : private ScfNoCopy
{
public:
    inline explicit     MultiItemsGuard( Output& rOut ) : mrOut( rOut ) { mrOut.StartMultiItems(); }
    inline              ~MultiItemsGuard() { mrOut.EndMultiItems(); }
private:
    Output&             mrOut;
};

// ============================================================================
// ============================================================================

class ObjectBase : public Base
{
public:
    virtual             ~ObjectBase();

    SfxMedium&          GetCoreMedium() const;
    SvStream&           GetCoreStream() const;

    void                Dump();

    // ------------------------------------------------------------------------
protected:
    inline explicit     ObjectBase() {}
    void                Construct( ConfigRef xConfig, CoreDataRef xCore, OutputRef xOut );
    void                Construct( const ObjectBase& rParent );

    virtual bool        ImplIsValid() const;
    virtual ConfigRef   ImplReconstructConfig();
    virtual OutputRef   ImplReconstructOutput();
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpBody();
    virtual void        ImplDumpFooter();

    // ------------------------------------------------------------------------

    void                ReconstructConfig();
    void                ReconstructOutput();

    inline Config&      Cfg() const { return *mxConfig; }
    inline CoreData&    Core() const { return *mxCore; }
    inline Output&      Out() const { return *mxOut; }

    // ------------------------------------------------------------------------

    void                WriteEmptyItem( const sal_Char* pcName );
    void                WriteInfoItem( const sal_Char* pcName, const StringWrapper& rData );
    void                WriteStringItem( const sal_Char* pcName, const String& rData );
    void                WriteArrayItem( const sal_Char* pcName, const sal_uInt8* pnData, sal_Size nSize, sal_Unicode cSep = ',' );
    void                WriteBoolItem( const sal_Char* pcName, bool bData );
    void                WriteColorItem( const sal_Char* pcName, const Color& rColor );
    void                WriteDateTimeItem( const sal_Char* pcName, const DateTime& rDateTime );
    void                WriteGuidItem( const sal_Char* pcName, const String& rGuid );

    template< typename Type >
    void                AddNameToItem( Type nData, const NameListWrapper& rListWrp );

    template< typename Type >
    void                WriteNameItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp );
    template< typename Type >
    void                WriteDecItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                WriteHexItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                WriteBinItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                WriteFixItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                WriteDecBoolItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    void                WriteValueItem( const sal_Char* pcName, Type nData, FormatType eFmtType, const NameListWrapper& rListWrp = NO_LIST );

    template< typename Type >
    void                WriteValueItem( const ItemFormat& rItemFmt, Type nData );

private:
    ConfigRef           mxConfig;
    CoreDataRef         mxCore;
    OutputRef           mxOut;
};

typedef ScfRef< ObjectBase > ObjectRef;

// ----------------------------------------------------------------------------

template< typename Type >
void ObjectBase::AddNameToItem( Type nData, const NameListWrapper& rListWrp )
{
    if( !rListWrp.Empty() )
    {
        mxOut->ContItem();
        mxOut->WriteName( *mxConfig, nData, rListWrp );
    }
}

template< typename Type >
void ObjectBase::WriteNameItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteName( *mxConfig, nData, rListWrp );
}

template< typename Type >
void ObjectBase::WriteDecItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteDec( nData );
    AddNameToItem( nData, rListWrp );
}

template< typename Type >
void ObjectBase::WriteHexItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteHex( nData );
    AddNameToItem( nData, rListWrp );
}

template< typename Type >
void ObjectBase::WriteBinItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteBin( nData );
    AddNameToItem( nData, rListWrp );
}

template< typename Type >
void ObjectBase::WriteFixItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteFix( nData );
    AddNameToItem( nData, rListWrp );
}

template< typename Type >
void ObjectBase::WriteDecBoolItem( const sal_Char* pcName, Type nData, const NameListWrapper& rListWrp )
{
    ItemGuard aItem( *mxOut, pcName );
    mxOut->WriteDec( nData );
    aItem.Cont();
    mxOut->WriteBool( nData != 0 );
    AddNameToItem( nData, rListWrp );
}

template< typename Type >
void ObjectBase::WriteValueItem( const sal_Char* pcName, Type nData, FormatType eFmtType, const NameListWrapper& rListWrp )
{
    if( eFmtType == FORMATTYPE_BOOL )
        WriteDecBoolItem( pcName, nData, rListWrp );
    else
    {
        ItemGuard aItem( *mxOut, pcName );
        mxOut->WriteValue( nData, eFmtType );
        AddNameToItem( nData, rListWrp );
    }
}

template< typename Type >
void ObjectBase::WriteValueItem( const ItemFormat& rItemFmt, Type nData )
{
    WriteValueItem( rItemFmt.maItemNameUtf8.GetBuffer(),
        nData, rItemFmt.meFmtType, rItemFmt.maListName );
}

// ============================================================================

class InputObjectBase : public ObjectBase
{
public:
    virtual             ~InputObjectBase();

    // ------------------------------------------------------------------------
protected:
    inline explicit     InputObjectBase() {}
    void                Construct( const ObjectBase& rParent, InputRef xIn );
    void                Construct( const InputObjectBase& rParent );

    virtual bool        ImplIsValid() const;
    virtual InputRef    ImplReconstructInput();
    virtual void        ImplDumpBody();

    // ------------------------------------------------------------------------

    void                ReconstructInput();

    inline Input&       In() const { return *mxIn; }

    // ------------------------------------------------------------------------

    void                SkipBlock( sal_Size nSize, bool bShowSize = true );
    void                DumpRawBinary( sal_Size nSize, bool bShowOffset = true, bool bStream = false );

    void                DumpBinary( const sal_Char* pcName, sal_Size nSize, bool bShowOffset = true );
    void                DumpArray( const sal_Char* pcName, sal_Size nSize, sal_Unicode cSep = ',' );
    void                DumpRemaining( sal_Size nSize );
    inline void         DumpUnused( sal_Size nSize ) { DumpArray( SCF_DUMP_UNUSED, nSize ); }
    inline void         DumpUnknown( sal_Size nSize ) { DumpArray( SCF_DUMP_UNKNOWN, nSize ); }

    void                DumpBinaryStream( bool bShowOffset = true );
    void                DumpTextStream( rtl_TextEncoding eEnc, bool bShowLines = true );

    String              DumpGuid( const sal_Char* pcName );
    void                DumpItem( const ItemFormat& rItemFmt );

    template< typename Type >
    Type                DumpName( const sal_Char* pcName, const NameListWrapper& rListWrp );
    template< typename Type >
    Type                DumpDec( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                DumpHex( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                DumpBin( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                DumpFix( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                DumpBool( const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                DumpValue( const ItemFormat& rItemFmt );

    template< typename Type1, typename Type2 >
    Type1               DumpName( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               DumpDec( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               DumpHex( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               DumpBin( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               DumpFix( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               DumpBool( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type1, typename Type2 >
    Type1               DumpValue( bool bType1, const ItemFormat& rItemFmt );

    // ------------------------------------------------------------------------

    using               ObjectBase::Construct;

private:
    InputRef            mxIn;
};

typedef ScfRef< InputObjectBase > InputObjectRef;

// ----------------------------------------------------------------------------

template< typename Type >
Type InputObjectBase::DumpName( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    WriteNameItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::DumpDec( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    WriteDecItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::DumpHex( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    WriteHexItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::DumpBin( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    WriteBinItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::DumpFix( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    WriteFixItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::DumpBool( const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    Type nData;
    *mxIn >> nData;
    WriteDecBoolItem( pcName, nData, rListWrp );
    return nData;
}

template< typename Type >
Type InputObjectBase::DumpValue( const ItemFormat& rItemFmt )
{
    Type nData;
    *mxIn >> nData;
    WriteValueItem( rItemFmt, nData );
    return nData;
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpName( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? DumpName< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( DumpName< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpDec( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? DumpDec< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( DumpDec< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpHex( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? DumpHex< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( DumpHex< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpBin( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? DumpBin< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( DumpBin< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpFix( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? DumpFix< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( DumpFix< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpBool( bool bType1, const sal_Char* pcName, const NameListWrapper& rListWrp )
{
    return bType1 ? DumpBool< Type1 >( pcName, rListWrp ) : static_cast< Type1 >( DumpBool< Type2 >( pcName, rListWrp ) );
}

template< typename Type1, typename Type2 >
Type1 InputObjectBase::DumpValue( bool bType1, const ItemFormat& rItemFmt )
{
    return bType1 ? DumpValue< Type1 >( rItemFmt ) : static_cast< Type1 >( DumpValue< Type2 >( rItemFmt ) );
}

// ============================================================================
// ============================================================================

class StreamObjectBase : public InputObjectBase
{
public:
    virtual             ~StreamObjectBase();

    inline SvStream&    GetStream() { return *mpStrm; }
    inline const String& GetStreamName() const { return maName; }
    inline const String& GetStreamPath() const { return maPath; }
    String              GetFullName() const;
    sal_Size            GetStreamSize() const;

protected:
    inline explicit     StreamObjectBase() : mpStrm( 0 ) {}
    void                Construct( const ObjectBase& rParent, SvStream& rStrm,
                            const String& rPath, const String& rStrmName, InputRef xIn );
    void                Construct( const ObjectBase& rParent, SvStream& rStrm,
                            const String& rPath, const String& rStrmName );
    void                Construct( const ObjectBase& rParent, SvStream& rStrm );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpFooter();
    virtual void        ImplDumpExtendedHeader();

    using               InputObjectBase::Construct;

private:
    void                DumpStreamInfo( bool bExtended );

private:
    SvStream*           mpStrm;
    String              maPath;
    String              maName;
};

typedef ScfRef< StreamObjectBase > StreamObjectRef;

// ============================================================================

class SvStreamObject : public StreamObjectBase
{
public:
    explicit            SvStreamObject( const ObjectBase& rParent, SvStream& rStrm );
    virtual             ~SvStreamObject();

protected:
    inline explicit     SvStreamObject() {}
    void                Construct( const ObjectBase& rParent, SvStream& rStrm );

    using               StreamObjectBase::Construct;
};

typedef ScfRef< SvStreamObject > SvStreamObjectRef;

// ============================================================================

class WrappedStreamObject : public StreamObjectBase
{
public:
    explicit            WrappedStreamObject( const ObjectBase& rParent, StreamObjectRef xStrmObj );
    virtual             ~WrappedStreamObject();

protected:
    inline explicit     WrappedStreamObject() {}
    void                Construct( const ObjectBase& rParent, StreamObjectRef xStrmObj );

    virtual bool        ImplIsValid() const;

    using               StreamObjectBase::Construct;

private:
    StreamObjectRef     mxStrmObj;
};

// ============================================================================
// ============================================================================

struct RecordHeaderConfigInfo
{
    const sal_Char*     mpcRecNames;
    const sal_Char*     mpcShowRecPos;
    const sal_Char*     mpcShowRecSize;
    const sal_Char*     mpcShowRecId;
    const sal_Char*     mpcShowRecName;
    const sal_Char*     mpcShowRecBody;
};

// ----------------------------------------------------------------------------

class RecordHeaderBase : public InputObjectBase
{
public:
    virtual             ~RecordHeaderBase();

    inline NameListRef  GetRecNames() const { return mxRecNames; }
    inline bool         IsShowRecPos() const { return mbShowRecPos; }
    inline bool         IsShowRecSize() const { return mbShowRecSize; }
    inline bool         IsShowRecId() const { return mbShowRecId; }
    inline bool         IsShowRecName() const { return mbShowRecName; }
    inline bool         IsShowRecBody() const { return mbShowRecBody; }

protected:
    inline explicit     RecordHeaderBase() {}
    void                Construct( const InputObjectBase& rParent, const RecordHeaderConfigInfo& rCfgInfo );

    virtual bool        ImplIsValid() const;

    using               InputObjectBase::Construct;

private:
    NameListRef         mxRecNames;
    bool                mbShowRecPos;
    bool                mbShowRecSize;
    bool                mbShowRecId;
    bool                mbShowRecName;
    bool                mbShowRecBody;
};

// ============================================================================
// ============================================================================

class DumperBase : public ObjectBase
{
public:
    virtual             ~DumperBase();

    bool                IsImportEnabled() const;

protected:
    inline explicit     DumperBase() {}
    void                Construct( ConfigRef xConfig, CoreDataRef xCore );
    void                Construct( ConfigRef xConfig, SfxMedium& rMedium, SfxObjectShell* pDocShell );

    using               ObjectBase::Construct;

private:
    typedef ScfRef< SvStream > SvStreamRef;
    SvStreamRef         mxOutStrm;
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif
#endif

