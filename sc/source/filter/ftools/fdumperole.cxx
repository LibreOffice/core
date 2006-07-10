/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fdumperole.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:48:07 $
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

#ifndef SC_FDUMPEROLE_HXX
#include "fdumperole.hxx"
#endif

#if SCF_INCL_DUMPER

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

namespace scf {
namespace dump {

// ============================================================================
// ============================================================================

namespace {

const sal_Int32 OLEPROP_ID_DICTIONARY   = 0;
const sal_Int32 OLEPROP_ID_CODEPAGE     = 1;

const sal_Int32 OLEPROP_TYPE_INT16      = 2;
const sal_Int32 OLEPROP_TYPE_INT32      = 3;
const sal_Int32 OLEPROP_TYPE_FLOAT      = 4;
const sal_Int32 OLEPROP_TYPE_DOUBLE     = 5;
const sal_Int32 OLEPROP_TYPE_DATE       = 7;
const sal_Int32 OLEPROP_TYPE_STRING     = 8;
const sal_Int32 OLEPROP_TYPE_STATUS     = 10;
const sal_Int32 OLEPROP_TYPE_BOOL       = 11;
const sal_Int32 OLEPROP_TYPE_VARIANT    = 12;
const sal_Int32 OLEPROP_TYPE_INT8       = 16;
const sal_Int32 OLEPROP_TYPE_UINT8      = 17;
const sal_Int32 OLEPROP_TYPE_UINT16     = 18;
const sal_Int32 OLEPROP_TYPE_UINT32     = 19;
const sal_Int32 OLEPROP_TYPE_INT64      = 20;
const sal_Int32 OLEPROP_TYPE_UINT64     = 21;
const sal_Int32 OLEPROP_TYPE_STRING8    = 30;
const sal_Int32 OLEPROP_TYPE_STRING16   = 31;
const sal_Int32 OLEPROP_TYPE_FILETIME   = 64;
const sal_Int32 OLEPROP_TYPE_BLOB       = 65;
const sal_Int32 OLEPROP_TYPE_STREAM     = 66;
const sal_Int32 OLEPROP_TYPE_STORAGE    = 67;
const sal_Int32 OLEPROP_TYPE_CLIPFMT    = 71;

const sal_uInt16 CODEPAGE_UNICODE       = 1200;

static const String saGlobalDocPropGuid  = CREATE_STRING( "F29F85E0-4FF9-1068-AB91-08002B27B3D9" );
static const String saBuiltinDocPropGuid = CREATE_STRING( "D5CDD502-2E9C-101B-9397-08002B2CF9AE" );
static const String saCustomDocPropGuid  = CREATE_STRING( "D5CDD505-2E9C-101B-9397-08002B2CF9AE" );

} // namespace

// ============================================================================
// ============================================================================

OleStorageObject::OleStorageObject( const OleStorageObject& rParentStrg, const String& rStrgName )
{
    Construct( rParentStrg, rStrgName );
}

OleStorageObject::OleStorageObject( const ObjectBase& rParent, SotStorageRef xRootStrg )
{
    Construct( rParent, xRootStrg, String::EmptyString() );
}

OleStorageObject::OleStorageObject( const ObjectBase& rParent, SvStream& rRootStrm )
{
    Construct( rParent, rRootStrm );
}

OleStorageObject::OleStorageObject( const ObjectBase& rParent )
{
    Construct( rParent );
}

OleStorageObject::~OleStorageObject()
{
}

void OleStorageObject::Construct( const ObjectBase& rParent, SotStorageRef xStrg, const String& rPath )
{
    ObjectBase::Construct( rParent );

    if( xStrg.Is() && (xStrg->GetError() == ERRCODE_NONE) )
    {
        mxStrg = xStrg;
        maPath = rPath;
        maName = xStrg->GetName();
    }
}

void OleStorageObject::Construct( const OleStorageObject& rParentStrg, const String& rStrgName )
{
    SotStorageRef xStrg = ScfTools::OpenStorageRead( rParentStrg.mxStrg, rStrgName );
    Construct( rParentStrg, xStrg, rParentStrg.GetFullName() );
}

void OleStorageObject::Construct( const ObjectBase& rParent, SvStream& rRootStrm )
{
    SotStorageRef xRootStrg( new SotStorage( rRootStrm ) );
    Construct( rParent, xRootStrg, String::EmptyString() );
}

void OleStorageObject::Construct( const ObjectBase& rParent )
{
    if( rParent.IsValid() )
        Construct( rParent, rParent.GetCoreStream() );
}

String OleStorageObject::GetFullName() const
{
    return String( maPath ).Append( '/' ).Append( maName );
}

bool OleStorageObject::ImplIsValid() const
{
    return mxStrg.Is() && ObjectBase::ImplIsValid();
}

void OleStorageObject::ImplDumpHeader()
{
    Output& rOut = Out();
    rOut.ResetIndent();
    rOut.WriteChar( '+', 2 );
    rOut.WriteChar( '=', 76 );
    rOut.NewLine();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "||" ) );
        WriteEmptyItem( "STORAGE-BEGIN" );
        DumpStorageInfo( true );
        rOut.EmptyLine();
    }
    rOut.EmptyLine();
}

void OleStorageObject::ImplDumpFooter()
{
    Output& rOut = Out();
    rOut.ResetIndent();
    {
        PrefixGuard aPreGuard( rOut, CREATE_STRING( "||" ) );
        rOut.EmptyLine();
        DumpStorageInfo( false );
        WriteEmptyItem( "STORAGE-END" );
    }
    rOut.WriteChar( '+', 2 );
    rOut.WriteChar( '=', 76 );
    rOut.NewLine();
    rOut.EmptyLine();
}

void OleStorageObject::DumpStorageInfo( bool bExtended )
{
    IndentGuard aIndGuard( Out() );
    WriteStringItem( "storage-name", maName );
    WriteStringItem( "full-path", GetFullName() );

    // directory
    if( bExtended )
    {
        SvStorageInfoList aInfoList;
        mxStrg->FillInfoList( &aInfoList );
        WriteDecItem( "directory-size", static_cast< sal_uInt32 >( aInfoList.Count() ) );

        IndentGuard aDirIndGuard( Out() );
        for( ULONG nInfo = 0; nInfo < aInfoList.Count(); ++nInfo )
        {
            MultiItemsGuard aMultiGuard( Out() );
            TableGuard aTabGuard( Out(), 14 );
            SvStorageInfo& rInfo = aInfoList.GetObject( nInfo );
            const sal_Char* pcType = rInfo.IsStream() ? "stream" :
                (rInfo.IsStorage() ? "storage" : "unknown");
            WriteInfoItem( "type", pcType );
            WriteStringItem( "name", rInfo.GetName() );
        }
    }
}

// ----------------------------------------------------------------------------

namespace {

void lclExtractStream( SotStorageStreamRef xInStrm, const String& rFileName )
{
    if( xInStrm.Is() && (xInStrm->GetError() == SVSTREAM_OK) )
    {
        SvFileStream aOutStrm( rFileName, STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC );
        if( aOutStrm.IsOpen() )
        {
            xInStrm->Seek( STREAM_SEEK_TO_BEGIN );
            aOutStrm << *xInStrm;
        }
    }
}

void lclExtractStorage( SotStorageRef xStrg, const String& rDirName )
{
    if( xStrg.Is() && (xStrg->GetError() == ERRCODE_NONE) )
    {
        // create directory in file system
        ::osl::FileBase::RC eRes = ::osl::Directory::create( rDirName );
        if( (eRes == ::osl::FileBase::E_None) || (eRes == ::osl::FileBase::E_EXIST) )
        {
            // process children of the storage
            SvStorageInfoList aInfoList;
            xStrg->FillInfoList( &aInfoList );
            for( ULONG nInfo = 0; nInfo < aInfoList.Count(); ++nInfo )
            {
                SvStorageInfo& rInfo = aInfoList.GetObject( nInfo );

                // encode all characters < 0x20
                String aSubName;
                StringHelper::AppendEncString( aSubName, rInfo.GetName(), false );

                // replace all characters reserved in file system
                static const sal_Unicode spcReserved[] = { '/', '\\', ':', '*', '?', '<', '>', '|', 0 };
                xub_StrLen nPos = 0;
                while( (nPos = aSubName.SearchChar( spcReserved, nPos )) != STRING_NOTFOUND )
                    aSubName.SetChar( nPos, '_' );

                // build full path
                String aFullName = rDirName;
                aFullName.Append( '/' ).Append( aSubName );

                // handle storages and streams
                if( rInfo.IsStorage() )
                {
                    SotStorageRef xSubStrg = ScfTools::OpenStorageRead( xStrg, rInfo.GetName() );
                    lclExtractStorage( xSubStrg, aFullName );
                }
                else if( rInfo.IsStream() )
                {
                    SotStorageStreamRef xSubStrm = ScfTools::OpenStorageStreamRead( xStrg, rInfo.GetName() );
                    lclExtractStream( xSubStrm, aFullName );
                }
            }
        }
    }
}

} // namespace

// ----------------------------------------------------------------------------

void OleStorageObject::ExtractStorageToFileSystem()
{
    if( Cfg().GetBoolOption( "extract-storage-streams", false ) )
    {
        ::rtl::OUString aOUPathName;
        if( ::osl::FileBase::getFileURLFromSystemPath( GetCoreMedium().GetPhysicalName(), aOUPathName ) == ::osl::FileBase::E_None )
        {
            String aFullName = aOUPathName;
            xub_StrLen nSepPos = aFullName.SearchBackward( '/' );
            xub_StrLen nNamePos = (nSepPos == STRING_NOTFOUND) ? 0 : (nSepPos + 1);
            String aDirName = aFullName.Copy( nNamePos );
            aDirName.SearchAndReplaceAll( '.', '_' );
            aDirName.AppendAscii( "_ext" );
            aFullName.Erase( nNamePos ).Append( aDirName );

            SotStorageRef xRootStrg( new SotStorage( GetCoreStream() ) );
            lclExtractStorage( xRootStrg, aFullName );
        }
    }
}

// ============================================================================
// ============================================================================

OleStreamObject::OleStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    Construct( rParentStrg, rStrmName );
}

OleStreamObject::~OleStreamObject()
{
}

void OleStreamObject::Construct( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    mxStrm = ScfTools::OpenStorageStreamRead( rParentStrg.GetStorage(), rStrmName );
    if( mxStrm.Is() && (mxStrm->GetError() == SVSTREAM_OK) )
        StreamObjectBase::Construct( rParentStrg, *mxStrm, rParentStrg.GetFullName(), rStrmName );
}

bool OleStreamObject::ImplIsValid() const
{
    return mxStrm.Is() && StreamObjectBase::ImplIsValid();
}

// ============================================================================

OlePropertyStreamObject::OlePropertyStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    Construct( rParentStrg, rStrmName );
}

void OlePropertyStreamObject::Construct( const OleStorageObject& rParentStrg, const String& rStrmName )
{
    OleStreamObject::Construct( rParentStrg, rStrmName );
}

void OlePropertyStreamObject::InitializeConfig( Config& rCfg )
{
    if( rCfg.IsValid() )
    {
        rCfg.SetStringOption( saGlobalDocPropGuid,  "GlobalDocProp" );
        rCfg.SetStringOption( saBuiltinDocPropGuid, "BuiltinDocProp" );
        rCfg.SetStringOption( saCustomDocPropGuid,  "CustomDocProp" );

        // byte order
        ScfRef< ConstList > xByteOrder = rCfg.CreateNameList< ConstList >( "BYTE-ORDER" );
        xByteOrder->SetName( 0xFEFF, "big-endian" );
        xByteOrder->SetName( 0xFFFE, "little-endian" );

        // OS type
        ScfRef< MultiList > xOleOsType = rCfg.CreateNameList< MultiList >( "OLEPROP-OSTYPE" );
        xOleOsType->SetName( 0, "dos,mac,win32,unix" );

        // base property IDs
        ScfRef< MultiList > xOlePropBaseIds = rCfg.CreateNameList< MultiList >( "OLEPROP-BASEIDS" );
        xOlePropBaseIds->SetName( 0, "dictionary,codepage" );
        xOlePropBaseIds->SetDefaultName( "" );
        xOlePropBaseIds->SetQuoteNames( true );

        // global property IDs
        ScfRef< MultiList > xOlePropGlobalIds = rCfg.CreateNameList< MultiList >( "OLEPROP-GLOBALIDS" );
        xOlePropGlobalIds->IncludeList( xOlePropBaseIds );
        xOlePropGlobalIds->SetName( 2, "title,subject,author,keywords,comments,template,last-author,rev-number" );
        xOlePropGlobalIds->SetName( 10, "edit-time,last-printed,create-time,last-saved,page-count,word-count,char-count,thumbnail,appname,security" );

        // MSO builtin property IDs
        ScfRef< MultiList > xOlePropBuiltinIds = rCfg.CreateNameList< MultiList >( "OLEPROP-BUILTINIDS" );
        xOlePropBuiltinIds->IncludeList( xOlePropBaseIds );
        xOlePropBuiltinIds->SetName( 2, "category,pres-target,byte-count,line-count,para-count,slide-count,note-count,hidden-slide-count" );
        xOlePropBuiltinIds->SetName( 10, "clips,scale-crop,heading-pairs,part-titles,manager,company,links-uptodate" );

        // simple property types
        ScfRef< MultiList > xOlePropTypeSimple = rCfg.CreateNameList< MultiList >( "OLEPROP-TYPE-SIMPLE" );
        xOlePropTypeSimple->SetName( 0, "empty,null,int16,int32,float,double,fixed,date,string8,dispatch" );
        xOlePropTypeSimple->SetName( 10, "error,bool,variant,unknown,decimal,int8,uint8,uint16,uint32" );
        xOlePropTypeSimple->SetName( 20, "int64,uint64,int,uint,void,hresult,ptr,savearray,c-array,userdef" );
        xOlePropTypeSimple->SetName( 30, "string8,string16" );
        xOlePropTypeSimple->SetName( 64, "time-stamp,blob,stream,storage,stream-obj,storage-obj" );
        xOlePropTypeSimple->SetName( 70, "blob-obj,clip-fmt,guid,vers-stream" );
        xOlePropTypeSimple->SetName( 0x0FFF, "str8-blob" );

        // property types with flags
        ScfRef< CombiList > xOlePropType = rCfg.CreateNameList< CombiList >( "OLEPROP-TYPE" );
        xOlePropType->SetName( 0x0FFF, "int32,dec,base-type,OLEPROP-TYPE-SIMPLE" );
        xOlePropType->SetName( 0x1000, "vector" );
        xOlePropType->SetName( 0x2000, "array" );
        xOlePropType->SetName( 0x4000, "byref" );
    }
}

void OlePropertyStreamObject::ImplDumpBody()
{
    Input& rIn = In();
    Output& rOut = Out();

    ScfStringVec aGuidVec;
    ScfUInt32Vec aStartPosVec;

    // dump header
    WriteEmptyItem( "HEADER" );
    {
        IndentGuard aIndGuard( rOut );
        DumpHex< sal_uInt16 >( "byte-order", "BYTE-ORDER" );
        DumpDec< sal_uInt16 >( "version" );
        DumpDec< sal_uInt16 >( "os-minor" );
        DumpDec< sal_uInt16 >( "os-type", "OLEPROP-OSTYPE" );
        DumpGuid( "guid" );
        sal_Int32 nSectCount = DumpDec< sal_Int32 >( "section-count" );

        // dump table of section positions
        {
            TableGuard aTabGuard( rOut, 15, 60 );
            rOut.ResetItemIndex();
            for( sal_Int32 nSectIdx = 0; (nSectIdx < nSectCount) && rIn.IsValidPos(); ++nSectIdx )
            {
                MultiItemsGuard aMultiGuard( rOut );
                WriteEmptyItem( "#section" );
                aGuidVec.push_back( DumpGuid( "guid" ) );
                aStartPosVec.push_back( DumpHex< sal_uInt32 >( "start-pos" ) );
            }
        }
    }
    rOut.EmptyLine();

    // dump sections
    for( size_t nSectIdx = 0; (nSectIdx < aStartPosVec.size()) && rIn.IsValidPos(); ++nSectIdx )
        DumpSection( aGuidVec[ nSectIdx ], aStartPosVec[ nSectIdx ] );
}

void OlePropertyStreamObject::DumpSection( const String& rGuid, sal_uInt32 nStartPos )
{
    Input& rIn = In();
    Output& rOut = Out();

    // property ID names
    mxPropIds = Cfg().CreateNameList< ConstList >( "OLEPROP-IDS" );
    if( rGuid == saGlobalDocPropGuid )
        mxPropIds->IncludeList( Cfg().GetNameList( "OLEPROP-GLOBALIDS" ) );
    else if( rGuid == saBuiltinDocPropGuid )
        mxPropIds->IncludeList( Cfg().GetNameList( "OLEPROP-BUILTINIDS" ) );
    else
        mxPropIds->IncludeList( Cfg().GetNameList( "OLEPROP-BASEIDS" ) );

    // property ID/position map
    typedef ::std::map< sal_Int32, sal_uInt32 > PropertyPosMap;
    PropertyPosMap aPropMap;

    // dump section header line
    WriteSectionHeader( rGuid, nStartPos );

    // seek to section
    IndentGuard aIndGuard( rOut );
    if( StartElement( nStartPos ) )
    {
        // dump section header
        sal_Int32 nSectSize = DumpDec< sal_Int32 >( "size" );
        sal_Int32 nPropCount = DumpDec< sal_Int32 >( "property-count" );

        // dump table of property positions
        {
            TableGuard aTabGuard( rOut, 15, 25 );
            rOut.ResetItemIndex();
            for( sal_Int32 nPropIdx = 0; (nPropIdx < nPropCount) && rIn.IsValidPos(); ++nPropIdx )
            {
                MultiItemsGuard aMultiGuard( rOut );
                WriteEmptyItem( "#property" );
                sal_Int32 nPropId = DumpDec< sal_Int32 >( "id", mxPropIds );
                sal_uInt32 nPropPos = nStartPos + DumpHex< sal_uInt32 >( "start-pos" );
                aPropMap[ nPropId ] = nPropPos;
            }
        }
    }
    rOut.EmptyLine();

    // code page property
    meTextEnc = ScfTools::GetSystemCharSet();
    mbIsUnicode = false;
    PropertyPosMap::iterator aCodePageIt = aPropMap.find( OLEPROP_ID_CODEPAGE );
    if( aCodePageIt != aPropMap.end() )
    {
        DumpCodePageProperty( aCodePageIt->second );
        aPropMap.erase( aCodePageIt );
    }

    // dictionary property
    PropertyPosMap::iterator aDictIt = aPropMap.find( OLEPROP_ID_DICTIONARY );
    if( aDictIt != aPropMap.end() )
    {
        DumpDictionaryProperty( aDictIt->second );
        aPropMap.erase( aDictIt );
    }

    // other properties
    for( PropertyPosMap::const_iterator aIt = aPropMap.begin(), aEnd = aPropMap.end(); aIt != aEnd; ++aIt )
        DumpProperty( aIt->first, aIt->second );

    // remove the user defined list of property ID names
    Cfg().EraseNameList( "OLEPROP-IDS" );
}

void OlePropertyStreamObject::DumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    WritePropertyHeader( nPropId, nStartPos );
    IndentGuard aIndGuard( Out() );
    if( StartElement( nStartPos ) )
        DumpPropertyContents( nPropId );
    Out().EmptyLine();
}

void OlePropertyStreamObject::DumpCodePageProperty( sal_uInt32 nStartPos )
{
    WritePropertyHeader( OLEPROP_ID_CODEPAGE, nStartPos );
    IndentGuard aIndGuard( Out() );
    if( StartElement( nStartPos ) )
    {
        sal_Int32 nType = DumpPropertyType();
        if( nType == OLEPROP_TYPE_INT16 )
        {
            sal_uInt16 nCodePage = DumpDec< sal_uInt16 >( "codepage", "CODEPAGES" );
            rtl_TextEncoding nNewTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
            if( nNewTextEnc != RTL_TEXTENCODING_DONTKNOW )
                meTextEnc = nNewTextEnc;
            mbIsUnicode = nCodePage == CODEPAGE_UNICODE;
        }
        else
            DumpPropertyContents( OLEPROP_ID_CODEPAGE );
    }
    Out().EmptyLine();
}

void OlePropertyStreamObject::DumpDictionaryProperty( sal_uInt32 nStartPos )
{
    WritePropertyHeader( OLEPROP_ID_DICTIONARY, nStartPos );
    IndentGuard aIndGuard( Out() );
    if( StartElement( nStartPos ) )
    {
        sal_Int32 nCount = DumpDec< sal_Int32 >( "count" );
        for( sal_Int32 nIdx = 0; (nIdx < nCount) && In().IsValidPos(); ++nIdx )
        {
            MultiItemsGuard aMultiGuard( Out() );
            TableGuard aTabGuard( Out(), 10, 20 );
            sal_Int32 nId = DumpDec< sal_Int32 >( "id" );
            String aName = DumpString8( "name" );
            if( mxPropIds.is() )
                mxPropIds->SetName( nId, aName );
        }
    }
    Out().EmptyLine();
}

void OlePropertyStreamObject::DumpPropertyContents( sal_Int32 nPropId )
{
    sal_Int32 nType = DumpPropertyType();
    if( ::get_flag< sal_Int32 >( nType, 0x1000 ) ) // vector
    {
        sal_Int32 nBaseType = nType & 0x0FFF;
        sal_Int32 nElemCount = DumpDec< sal_Int32 >( "element-count" );
        for( sal_Int32 nElemIdx = 0; (nElemIdx < nElemCount) && In().IsValidPos(); ++nElemIdx )
        {
            Out().ResetItemIndex( nElemIdx );
            WriteEmptyItem( "#element" );
            IndentGuard aIndGuard( Out() );
            DumpPropertyValue( nPropId, nBaseType );
        }
    }
    else if( !::get_flag< sal_Int32 >( nType, 0x7000 ) )
    {
        DumpPropertyValue( nPropId, nType );
    }
}

void OlePropertyStreamObject::DumpPropertyValue( sal_Int32 nPropId, sal_Int32 nBaseType )
{
    switch( nBaseType )
    {
        case OLEPROP_TYPE_INT16:        DumpDec< sal_Int16 >( "value" );        break;
        case OLEPROP_TYPE_INT32:        DumpDec< sal_Int32 >( "value" );        break;
        case OLEPROP_TYPE_FLOAT:        DumpDec< float >( "value" );            break;
        case OLEPROP_TYPE_DOUBLE:       DumpDec< double >( "value" );           break;
        case OLEPROP_TYPE_DATE:         DumpDec< double >( "date" );            break;
        case OLEPROP_TYPE_STRING:       DumpString8( "value" );                 break;
        case OLEPROP_TYPE_STATUS:       DumpHex< sal_Int32 >( "status" );       break;
        case OLEPROP_TYPE_BOOL:         DumpBool< sal_Int16 >( "value" );       break;
        case OLEPROP_TYPE_VARIANT:      DumpPropertyContents( nPropId );        break;
        case OLEPROP_TYPE_INT8:         DumpDec< sal_Int8 >( "value" );         break;
        case OLEPROP_TYPE_UINT8:        DumpDec< sal_uInt8 >( "value" );        break;
        case OLEPROP_TYPE_UINT16:       DumpDec< sal_uInt16 >( "value" );       break;
        case OLEPROP_TYPE_UINT32:       DumpDec< sal_uInt32 >( "value" );       break;
        case OLEPROP_TYPE_INT64:        DumpDec< sal_Int64 >( "value" );        break;
        case OLEPROP_TYPE_UINT64:       DumpDec< sal_uInt64 >( "value" );       break;
        case OLEPROP_TYPE_STRING8:      DumpString8( "value" );                 break;
        case OLEPROP_TYPE_STRING16:     DumpString16( "value" );                break;
        case OLEPROP_TYPE_FILETIME:     DumpFileTime( "file-time" );            break;
        case OLEPROP_TYPE_BLOB:         DumpBlob( "data" );                     break;
        case OLEPROP_TYPE_STREAM:       DumpString8( "stream-name" );           break;
        case OLEPROP_TYPE_STORAGE:      DumpString8( "storage-name" );          break;
        case OLEPROP_TYPE_CLIPFMT:      DumpBlob( "clip-data" );                break;
    }
}

sal_Int32 OlePropertyStreamObject::DumpPropertyType()
{
    return DumpHex< sal_Int32 >( "type", "OLEPROP-TYPE" );
}

void OlePropertyStreamObject::DumpBlob( const sal_Char* pcName )
{
    sal_Int32 nSize = DumpDec< sal_Int32 >( "data-size" );
    if( nSize > 0 )
        DumpBinary( pcName, nSize );
}

String OlePropertyStreamObject::DumpString8( const sal_Char* pcName )
{
    sal_Int32 nLen = DumpDec< sal_Int32 >( "string-len" );
    return mbIsUnicode ? DumpCharArray16( pcName, nLen ) : DumpCharArray8( pcName, nLen );
}

String OlePropertyStreamObject::DumpCharArray8( const sal_Char* pcName, sal_Int32 nCharCount )
{
    size_t nLen = static_cast< size_t >( ::std::max< sal_Int32 >( ::std::min< sal_Int32 >( nCharCount, 1024 ), 0 ) );
    ::std::vector< sal_Char > aBuffer( nLen + 1, 0 );
    In().Read( &aBuffer.front(), nLen );
    String aData( &aBuffer.front(), meTextEnc );
    WriteStringItem( pcName, aData );
    return aData;
}

String OlePropertyStreamObject::DumpString16( const sal_Char* pcName )
{
    sal_Int32 nLen = DumpDec< sal_Int32 >( "string-len" );
    return DumpCharArray16( pcName, nLen );
}

String OlePropertyStreamObject::DumpCharArray16( const sal_Char* pcName, sal_Int32 nCharCount )
{
    size_t nLen = static_cast< size_t >( ::std::max< sal_Int32 >( ::std::min< sal_Int32 >( nCharCount, 1024 ), 0 ) );
    ::std::vector< sal_Unicode > aBuffer;
    aBuffer.reserve( nLen + 1 );
    for( sal_Size nIdx = 0; nIdx < nLen; ++nIdx )
        aBuffer.push_back( static_cast< sal_Unicode >( In().ReadValue< sal_uInt16 >() ) );
    aBuffer.push_back( 0 );
    String aData( &aBuffer.front() );
    WriteStringItem( pcName, aData );
    if( nLen & 1 ) DumpUnused( 2 ); // always padding to 32bit
    return aData;
}

DateTime OlePropertyStreamObject::DumpFileTime( const sal_Char* pcName )
{
    ItemGuard aItem( Out(), pcName );
    sal_uInt64 nFileTime = DumpDec< sal_uInt64 >( 0 );
#if 0
    // TODO: available from 680m164
    sal_uInt32 nLower = static_cast< sal_uInt32 >( nFileTime );
    sal_uInt32 nUpper = static_cast< sal_uInt32 >( nFileTime >> 32 );
    DateTime aDateTime = DateTime::CreateFromWin32FileDateTime( nLower, nUpper );
#else
    nFileTime /= 100000;
    Date aDate( 1, 1, 1601 );
    aDate += static_cast< sal_uInt32 >( nFileTime / 8640000 );
    Time aTime( static_cast< sal_Int32 >( nFileTime % 8640000 ) );
    DateTime aDateTime( aDate, aTime );
#endif
    aDateTime.ConvertToLocalTime();
    WriteDateTimeItem( 0, aDateTime );
    return aDateTime;
}

bool OlePropertyStreamObject::StartElement( sal_uInt32 nStartPos )
{
    bool bPosOk = nStartPos < In().Size();
    if( bPosOk )
        In().Seek( static_cast< sal_Size >( nStartPos ) );
    else
        WriteInfoItem( "stream-state", SCF_DUMP_ERR_STREAM );
    return bPosOk;
}

void OlePropertyStreamObject::WriteSectionHeader( const String& rGuid, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteEmptyItem( "SECTION" );
    WriteGuidItem( "guid", rGuid );
    WriteHexItem( "pos", nStartPos );
}

void OlePropertyStreamObject::WritePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( Out() );
    WriteEmptyItem( "PROPERTY" );
    WriteDecItem( "id", nPropId, mxPropIds );
    WriteHexItem( "pos", nStartPos );
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif

