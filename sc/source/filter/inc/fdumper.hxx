/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fdumper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-22 12:07:13 $
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

#ifndef SC_FDUMPER_HXX
#define SC_FDUMPER_HXX

// 1 = Compile with dumper.
#define SCF_INCL_DUMPER 0

// ============================================================================

#if OSL_DEBUG_LEVEL < 2
#undef SCF_INCL_DUMPER
#define SCF_INCL_DUMPER 0
#endif

#if SCF_INCL_DUMPER

// ============================================================================

#include <map>

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

class SvStream;

namespace scf {
namespace dump {

#define SCF_DUMP_ERRSTRING( ascii )     CREATE_STRING( "?err:" ascii )

#define SCF_DUMP_ERR_NOMAP              SCF_DUMP_ERRSTRING( "no-map" )
#define SCF_DUMP_ERR_NONAME             SCF_DUMP_ERRSTRING( "no-name" )
#define SCF_DUMP_ERR_STREAM             SCF_DUMP_ERRSTRING( "stream-error" )

// ============================================================================
// ============================================================================

/** Static helper functions for formatted output to strings. */
class StringHelper
{
public:
    static void         AppendString( String& rStr, const String& rData, xub_StrLen nWidth = 0 );
    static void         AppendToken( String& rStr, const String& rToken, sal_Unicode cSep = ',' );
    static void         TrimString( String& rStr );
    static ByteString   ConvertToUtf8( const String& rStr );

    // append decimal ---------------------------------------------------------

    static void         AppendDec( String& rStr, sal_uInt8  nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_Int8   nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_uInt16 nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_Int16  nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_uInt32 nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_Int32  nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_uInt64 nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, sal_Int64  nData, xub_StrLen nWidth = 0 );
    static void         AppendDec( String& rStr, double     fData, xub_StrLen nWidth = 0 );

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

    // encoded text output ----------------------------------------------------

    static void         AppendCChar( String& rStr, sal_Unicode cChar );
    static void         AppendEncChar( String& rStr, sal_Unicode cChar, xub_StrLen nCount = 1 );
    static void         AppendEncString( String& rStr, const String& rData );

    // string to value conversion ---------------------------------------------

    static bool         ConvertFromDec( sal_Int64& rnData, const String& rData );
    static bool         ConvertFromHex( sal_Int64& rnData, const String& rData );

    static bool         ConvertStringToInt( sal_Int64& rnData, const String& rData );
    static bool         ConvertStringToBool( const String& rData );
};

// ============================================================================
// ============================================================================

/** Base class for all dumper classes. */
class Base : public StringHelper
{
protected:
    virtual             ~Base();
};

// ============================================================================
// ============================================================================

class ConfigBase : public Base
{
public:
    inline explicit     ConfigBase() {}

    void                ReadConfigBlock( SvStream& rStrm );

protected:
    void                ReadConfigBlockContents( SvStream& rStrm );
    void                IgnoreConfigBlockContents( SvStream& rStrm );

    virtual void        ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData ) = 0;

private:
    enum LineType { LINETYPE_DATA, LINETYPE_BEGIN, LINETYPE_END };

    LineType            ReadConfigLine( SvStream& rStrm, String& rKey, String& rData ) const;
    LineType            ReadConfigLine( SvStream& rStrm ) const;
};

// ============================================================================

template< typename KeyType >
struct NamedKey
{
    KeyType             mnKey;
    const sal_Char*     mpcName;
};

typedef sal_Int64 NameListKey;

// ============================================================================

class NameListBase : public ConfigBase
{
public:
    explicit            NameListBase();
    template< typename Type >
    explicit            NameListBase( const NamedKey< Type >* pNames );

    /** Sets a name for the specified key. */
    void                SetName( NameListKey nKey, const String& rName );
    /** Sets names for a list of keys. Must be terminated by an entry with null pointer as name. */
    template< typename Type >
    void                SetNames( const NamedKey< Type >* pNames );

    /** Returns true, if the map contains an entry for the passed key. */
    template< typename Type >
    inline bool         HasName( Type nKey ) const
        { return maMap.count( static_cast< NameListKey >( nKey ) ) != 0; }
    /** Returns the name for the passed key. */
    template< typename Type >
    inline String       GetName( Type nKey ) const
        { return ImplGetName( static_cast< NameListKey >( nKey ) ); }

protected:
    virtual void        ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData );

    /** Derived classes returns the name for the passed key. */
    virtual String      ImplGetName( NameListKey nKey ) const = 0;

protected:
    typedef ::std::map< NameListKey, String > StringMap;
    StringMap           maMap;
};

typedef ScfRef< NameListBase > NameListRef;

// ----------------------------------------------------------------------------

template< typename Type >
NameListBase::NameListBase( const NamedKey< Type >* pNames )
{
    SetNames( pNames );
}

template< typename Type >
void NameListBase::SetNames( const NamedKey< Type >* pNames )
{
    for( const NamedKey< Type >* pName = pNames; pName && pName->mpcName; ++pName )
        SetName( static_cast< NameListKey >( pName->mnKey ), String( pName->mpcName, RTL_TEXTENCODING_UTF8 ) );
}

// ============================================================================

class ConstList : public NameListBase
{
public:
    explicit            ConstList();
    template< typename Type >
    explicit            ConstList( const NamedKey< Type >* pNames );

    /** Sets a default name for unknown keys. */
    inline void         SetDefaultName( const String& rDefName ) { maDefName = rDefName; }

protected:
    virtual void        ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData );

    /** Returns the name for the passed key, or the default name, if key is not contained. */
    virtual String      ImplGetName( NameListKey nKey ) const;

private:
    String              maDefName;
};

// ----------------------------------------------------------------------------

template< typename Type >
ConstList::ConstList( const NamedKey< Type >* pNames ) :
    NameListBase( pNames ),
    maDefName( SCF_DUMP_ERR_NONAME )
{
}

// ============================================================================

class FlagsList : public NameListBase
{
public:
    explicit            FlagsList();
    template< typename Type >
    explicit            FlagsList( const NamedKey< Type >* pNames );

    /** Sets flags to be ignored on output. */
    template< typename Type >
    inline void         SetIgnoreFlags( Type nIgnore )
        { mnIgnore = static_cast< NameListKey >( nIgnore ); }

protected:
    virtual void        ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData );

    /** Returns the name for the passed key, or the default name, if key is not contained. */
    virtual String      ImplGetName( NameListKey nKey ) const;

private:
    NameListKey         mnIgnore;
};

// ----------------------------------------------------------------------------

template< typename Type >
FlagsList::FlagsList( const NamedKey< Type >* pNames ) :
    NameListBase( pNames ),
    mnIgnore( 0 )
{
}

// ============================================================================

class Config : public ConfigBase
{
public:
    inline explicit     Config() {}
    explicit            Config( const String& rFullName );
    explicit            Config( const sal_Char* pcEnvVar, const String& rFileName );

    bool                GetBoolOption( const sal_Char* pcKey, bool bDefault ) const;

    bool                IsDumperEnabled() const;

    NameListRef         GetNameList( const sal_Char* pcMapName ) const;

protected:
    bool                Construct( const String& rFullName );
    bool                Construct( const sal_Char* pcEnvVar, const String& rFileName );

    virtual void        ProcessConfigItem( SvStream& rStrm, const String& rKey, const String& rData );

private:
    template< typename ListType >
    void                ReadNameList( SvStream& rStrm, const String& rName );

private:
    typedef ::std::map< String, String >        ConfigDataMap;
    typedef ::std::map< String, NameListRef >   NameListMap;

    ConfigDataMap       maConfigData;
    NameListMap         maNameLists;
};

typedef ScfRef< Config > ConfigRef;

// ----------------------------------------------------------------------------

template< typename ListType >
void Config::ReadNameList( SvStream& rStrm, const String& rName )
{
    if( rName.Len() > 0 )
    {
        NameListRef& rxList = maNameLists[ rName ];
        if( !rxList || !dynamic_cast< ListType* >( &*rxList ) )
            rxList.reset( new ListType );
        rxList->ReadConfigBlock( rStrm );
    }
}

// ============================================================================
// ============================================================================

class Input : public Base
{
public:
    virtual sal_Size    Size() const = 0;
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
};

typedef ScfRef< Input > InputRef;

// ----------------------------------------------------------------------------

class SvStreamInput : public Input
{
public:
    explicit            SvStreamInput( SvStream& rStrm );
    virtual             ~SvStreamInput();

    virtual sal_Size    Size() const;
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

    inline bool         IsEmpty() const { return maLine.Len() == 0; }
    inline sal_Size     Tell() const { return mrStrm.Tell(); }

    // ------------------------------------------------------------------------

    void                SetPrefix( const String& rPrefix );
    inline const String& GetPrefix() const { return maPrefix; }

    void                IncIndent();
    void                DecIndent();

    void                StartTable( size_t nColCount,
                            xub_StrLen nWidth1, xub_StrLen nWidth2 = 0,
                            xub_StrLen nWidth3 = 0, xub_StrLen nWidth4 = 0 );
    void                StartTable( size_t nColCount,
                            const xub_StrLen* pnColWidths );
    void                Tab();
    void                Tab( size_t nCol );
    void                EndTable();

    void                NewLine();
    void                EmptyLine( sal_Int32 nCount = 1 );

    // ------------------------------------------------------------------------

    void                WriteChar( sal_Unicode cChar, xub_StrLen nCount = 1 );
    void                WriteAscii( const sal_Char* pcStr );
    void                WriteString( const String& rStr );

    template< typename Type >
    inline void         WriteDec( Type nData, xub_StrLen nWidth = 0 )
                            { StringHelper::AppendDec( maLine, nData, nWidth ); }
    template< typename Type >
    inline void         WriteHex( Type nData, bool bPrefix = true )
                            { StringHelper::AppendHex( maLine, nData, bPrefix ); }
    template< typename Type >
    inline void         WriteBin( Type nData, bool bDots = true )
                            { StringHelper::AppendBin( maLine, nData, bDots ); }
    template< typename Type >
    void                WriteName( NameListRef xList, Type nData );

    // ------------------------------------------------------------------------
private:
    typedef ::std::vector< xub_StrLen > StringLenVec;

    SvStream&           mrStrm;
    String              maPrefix;
    ByteString          maPrefixUtf8;
    ByteString          maIndent;
    String              maLine;
    StringLenVec        maColPos;
    size_t              mnCol;
};

typedef ScfRef< Output > OutputRef;

// ----------------------------------------------------------------------------

template< typename Type >
void Output::WriteName( NameListRef xList, Type nData )
{
    if( xList.is() )
        WriteString( xList->GetName( nData ) );
    else
        WriteString( SCF_DUMP_ERR_NOMAP );
}

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
    inline explicit     TableGuard( Output& rOut, size_t nColCount,
                                xub_StrLen nWidth1, xub_StrLen nWidth2 = 0,
                                xub_StrLen nWidth3 = 0, xub_StrLen nWidth4 = 0 ) :
                            mrOut( rOut ) { mrOut.StartTable( nColCount, nWidth1, nWidth2, nWidth3, nWidth4 ); }
    inline explicit     TableGuard( Output& rOut, size_t nColCount,
                                const xub_StrLen* pnColWidths ) :
                            mrOut( rOut ) { mrOut.StartTable( nColCount, pnColWidths ); }
    inline              ~TableGuard() { mrOut.EndTable(); }
    inline void         Tab() { mrOut.Tab(); }
    inline void         Tab( size_t nCol ) { mrOut.Tab( nCol ); }
private:
    Output&             mrOut;
};

// ============================================================================
// ============================================================================

class BaseObject : public Base
{
public:
    inline explicit     BaseObject( const BaseObject& rParent ) { Construct( rParent ); }

    bool                IsValid() const;
    void                Dump();
    void                DumpHeader();
    void                DumpFooter();

    void                StartItem( const sal_Char* pcName );
    void                EndItem();

    void                StartMultiItems();
    void                EndMultiItems();

    // ------------------------------------------------------------------------
protected:
    inline explicit     BaseObject() {}
    void                Construct( ConfigRef xConfig, OutputRef xOut );
    void                Construct( const BaseObject& rParent );
    void                Construct( ConfigRef xConfig, const String& rFileName );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpBody();
    virtual void        ImplDumpFooter();

    // ------------------------------------------------------------------------

    inline Config&      Cfg() { return *mxConfig; }
    inline Output&      Out() { return *mxOut; }

    // ------------------------------------------------------------------------

    template< typename Type >
    void                WriteItem( const sal_Char* pcName, const Type& rData );

    void                WriteEmptyItem( const sal_Char* pcName );
    void                WriteInfoItem( const sal_Char* pcName, const sal_Char* pcData );
    void                WriteInfoItem( const sal_Char* pcName, const String& rData );
    void                WriteStringItem( const sal_Char* pcName, const String& rData );

    template< typename Type >
    void                WriteDecItem( const sal_Char* pcName, Type nData, xub_StrLen nWidth = 0 );
    template< typename Type >
    void                WriteHexItem( const sal_Char* pcName, Type nData, bool bPrefix = true );
    template< typename Type >
    void                WriteBinItem( const sal_Char* pcName, Type nData, bool bDots = true );
    template< typename Type >
    void                WriteHexDecItem( const sal_Char* pcName, Type nData, bool bPrefix = true, xub_StrLen nWidth = 0 );

    template< typename Type >
    void                WriteNameItem( const sal_Char* pcName, NameListRef xList, Type nData, bool bUnknownEmpty = false );
    template< typename Type >
    void                WriteDecItem( const sal_Char* pcName, NameListRef xList, Type nData, xub_StrLen nWidth = 0 );
    template< typename Type >
    void                WriteHexItem( const sal_Char* pcName, NameListRef xList, Type nData, bool bPrefix = true );

private:
    typedef ScfRef< SvStream > SvStreamRef;

    SvStreamRef         mxOwnStrm;
    ConfigRef           mxConfig;
    OutputRef           mxOut;
    sal_Int32           mnItemLevel;
    sal_Int32           mnMultiLevel;
};

// ----------------------------------------------------------------------------

template< typename Type >
void BaseObject::WriteItem( const sal_Char* pcName, const Type& rData )
{
    ItemGuard aItem( *this, pcName );
    (*mxOut) << rData;
}

template< typename Type >
void BaseObject::WriteDecItem( const sal_Char* pcName, Type nData, xub_StrLen nWidth )
{
    ItemGuard aItem( *this, pcName );
    mxOut->WriteDec( nData, nWidth );
}

template< typename Type >
void BaseObject::WriteHexItem( const sal_Char* pcName, Type nData, bool bPrefix )
{
    ItemGuard aItem( *this, pcName );
    mxOut->WriteHex( nData, bPrefix );
}

template< typename Type >
void BaseObject::WriteBinItem( const sal_Char* pcName, Type nData, bool bDots )
{
    ItemGuard aItem( *this, pcName );
    mxOut->WriteBin( nData, bDots );
}

template< typename Type >
void BaseObject::WriteHexDecItem( const sal_Char* pcName, Type nData, bool bPrefix, xub_StrLen nWidth )
{
    ItemGuard aItem( *this, pcName );
    WriteDecItem( pcName, nData, nWidth );
    mxOut->WriteHex( nData, bPrefix );
}

template< typename Type >
void BaseObject::WriteNameItem( const sal_Char* pcName, NameListRef xList, Type nData, bool bUnknownEmpty )
{
    if( !bUnknownEmpty || !xList || xList->HasName( nData ) )
    {
        ItemGuard aItem( *this, pcName );
        mxOut->WriteName( xList, nData );
    }
}

template< typename Type >
void BaseObject::WriteDecItem( const sal_Char* pcName, NameListRef xList, Type nData, xub_StrLen nWidth )
{
    ItemGuard aItem( *this, pcName );
    WriteDecItem( pcName, nData, nWidth );
    mxOut->WriteName( xList, nData );
}

template< typename Type >
void BaseObject::WriteHexItem( const sal_Char* pcName, NameListRef xList, Type nData, bool bPrefix )
{
    ItemGuard aItem( *this, pcName );
    WriteHexItem( pcName, nData, bPrefix );
    mxOut->WriteName( xList, nData );
}

// ----------------------------------------------------------------------------

class ItemGuard : private ScfNoCopy
{
public:
    inline explicit     ItemGuard( BaseObject& rObj, const sal_Char* pcName ) : mrObj( rObj ) { mrObj.StartItem( pcName ); }
    inline              ~ItemGuard() { mrObj.EndItem(); }
private:
    BaseObject&         mrObj;
};

// ----------------------------------------------------------------------------

class MultiItemsGuard : private ScfNoCopy
{
public:
    inline explicit     MultiItemsGuard( BaseObject& rObj ) : mrObj( rObj ) { mrObj.StartMultiItems(); }
    inline              ~MultiItemsGuard() { mrObj.EndMultiItems(); }
private:
    BaseObject&         mrObj;
};

// ============================================================================

class InputObject : public BaseObject
{
public:
    explicit            InputObject( const BaseObject& rParent, InputRef xIn );
    virtual             ~InputObject();

    // ------------------------------------------------------------------------
protected:
    inline explicit     InputObject() {}
    void                Construct( const BaseObject& rParent, InputRef xIn );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpBody();

    inline Input&       In() { return *mxIn; }

    // ------------------------------------------------------------------------

    template< typename Type >
    Type                DumpValue( const sal_Char* pcName );

    template< typename Type >
    Type                DumpDec( const sal_Char* pcName, xub_StrLen nWidth = 0 );
    template< typename Type >
    Type                DumpHex( const sal_Char* pcName, bool bPrefix = true );
    template< typename Type >
    Type                DumpBin( const sal_Char* pcName, bool bDots = true );
    template< typename Type >
    Type                DumpHexDec( const sal_Char* pcName, bool bPrefix = true, xub_StrLen nWidth = 0 );

    template< typename Type >
    Type                DumpName( const sal_Char* pcName, NameListRef xList );
    template< typename Type >
    Type                DumpDec( const sal_Char* pcName, NameListRef xList, xub_StrLen nWidth = 0 );
    template< typename Type >
    Type                DumpHex( const sal_Char* pcName, NameListRef xList, bool bPrefix = true );

    void                DumpBinary( sal_Size nSize, bool bShowOffset = true );

    // ------------------------------------------------------------------------

    void                DumpBinaryStream( bool bShowOffset = true );
    void                DumpTextStream( rtl_TextEncoding eEnc, bool bShowLines = true );

private:
    InputRef            mxIn;
};

// ----------------------------------------------------------------------------

template< typename Type >
Type InputObject::DumpValue( const sal_Char* pcName )
{
    Type aData;
    (*mxIn) >> aData;
    WriteItem( pcName, aData );
    return aData;
}

template< typename Type >
Type InputObject::DumpDec( const sal_Char* pcName, xub_StrLen nWidth )
{
    Type nData;
    (*mxIn) >> nData;
    WriteDecItem( pcName, nData, nWidth );
    return nData;
}

template< typename Type >
Type InputObject::DumpHex( const sal_Char* pcName, bool bPrefix )
{
    Type nData;
    (*mxIn) >> nData;
    WriteHexItem( pcName, nData, bPrefix );
    return nData;
}

template< typename Type >
Type InputObject::DumpBin( const sal_Char* pcName, bool bDots )
{
    Type nData;
    (*mxIn) >> nData;
    WriteBinItem( pcName, nData, bDots );
    return nData;
}

template< typename Type >
Type InputObject::DumpHexDec( const sal_Char* pcName, bool bPrefix, xub_StrLen nWidth )
{
    Type nData;
    (*mxIn) >> nData;
    WriteHexDecItem( pcName, nData, bPrefix, nWidth );
    return nData;
}

template< typename Type >
Type InputObject::DumpName( const sal_Char* pcName, NameListRef xList )
{
    Type nData;
    (*mxIn) >> nData;
    WriteNameItem( pcName, xList, nData );
    return nData;
}

template< typename Type >
Type InputObject::DumpDec( const sal_Char* pcName, NameListRef xList, xub_StrLen nWidth )
{
    Type nData;
    (*mxIn) >> nData;
    WriteDecItem( pcName, xList, nData, nWidth );
    return nData;
}

template< typename Type >
Type InputObject::DumpHex( const sal_Char* pcName, NameListRef xList, bool bPrefix )
{
    Type nData;
    (*mxIn) >> nData;
    WriteHexItem( pcName, xList, nData, bPrefix );
    return nData;
}

// ============================================================================
// ============================================================================

class OleStorageObject : public BaseObject
{
public:
    explicit            OleStorageObject( const OleStorageObject& rParentStrg, const String& rStrgName );
    explicit            OleStorageObject( const BaseObject& rParent, SotStorageRef xRootStrg );
    explicit            OleStorageObject( const BaseObject& rParent, SvStream& rRootStrm );
    virtual             ~OleStorageObject();

    inline SotStorageRef GetStorage() const { return mxStrg; }
    inline const String& GetStrgName() const { return maStrgName; }
    inline const String& GetFullPath() const { return maFullPath; }

protected:
    inline explicit     OleStorageObject() {}
    void                Construct( const BaseObject& rParent, SotStorageRef xStrg, const String& rParentPath );
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrgName );
    void                Construct( const BaseObject& rParent, SvStream& rRootStrm );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpFooter();

private:
    SotStorageRef       mxStrg;
    String              maStrgName;
    String              maFullPath;
};

// ============================================================================

class OleStreamObject : public InputObject
{
public:
    explicit            OleStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );
    virtual             ~OleStreamObject();

    inline SvStream&    GetStream() { return *mxStrm; }
    inline const String& GetStrmName() const { return maStrmName; }
    inline const String& GetFullPath() const { return maFullPath; }

protected:
    inline explicit     OleStreamObject() {}
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpFooter();

private:
    SotStorageStreamRef mxStrm;
    String              maStrmName;
    String              maFullPath;
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif
#endif

