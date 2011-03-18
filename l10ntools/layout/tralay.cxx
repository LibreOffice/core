/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/xml/sax/SAXException.hpp>

#include <osl/file.hxx>
#include <osl/process.h>

#include <l10ntools/vosapp.hxx>

#include "export.hxx"
#include "layoutparse.hxx"
#include "helpmerge.hxx"
#include "xmlparse.hxx"

// Convert a rtl::OUString to a byte string.
#define OUSTRING_CSTR( str ) \
    rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr()

#define STRING( str ) String( str, RTL_TEXTENCODING_UTF8 )
#define BSTRING( str ) ByteString( str, RTL_TEXTENCODING_UTF8 )

using ::rtl::OUString;

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


class TranslateLayout : public Application
{
    ByteString mGid1;
    ByteString mLanguage;
    ByteString mLocalize;
    ByteString mOutput;
    ByteString mProject;
    ByteString mRoot;
    bool mMergeMode;
    std::vector< ByteString > mLanguages;
    std::list< ByteString > mFiles;

public:
    TranslateLayout();
    virtual ~TranslateLayout();
    ByteString GetCommandLineParam( int i );
    ByteString GetOptionArgument( int const i );
    void ExceptionalMain();
    void Main();
    void Merge();
    void MergeLanguage( ByteString const& language );
    void ParseCommandLine();
    void CreateSDF();

    using Application::GetCommandLineParam;
};

static void usage()
{
    fprintf( stderr, "Usage: tralay [OPTION]... XML-FILE\n" );
    fprintf( stderr, "\nOptions:\n" );
    fprintf( stderr, "  -h,--help                  show this help\n" );
    fprintf( stderr, "  -l,--language=LANG         process this language\n" );
    fprintf( stderr, "  -m,--merge=DATABASE.SDF    translation database\n" );
    fprintf( stderr, "\nExamples:\n" );
    fprintf( stderr, "  tralay -l en-US -o localize.sdf zoom.xml   # Extract\n" );
    fprintf( stderr, "  tralay -m localize.sdf -l de -l nl -o out zoom.xml  # Merge/translate\n" );
    exit( 2 );
}

static ByteString ConvertSystemPath( const ByteString& rPath )
{
    if( rPath.CompareTo( ".", 1 ) == 0 )
    {
        OUString sPath( rPath.GetBuffer(), rPath.Len(), RTL_TEXTENCODING_UTF8 );

        ::rtl::OUString curDirPth, sURL;
        osl_getProcessWorkingDir( &curDirPth.pData );

        ::osl::FileBase::getAbsoluteFileURL( curDirPth, sPath, sURL );
        ::osl::FileBase::getSystemPathFromFileURL( sURL, sPath );

        return ByteString( rtl::OUStringToOString( sPath, RTL_TEXTENCODING_UTF8 ) );
    }
    else
    {
        return rPath;
    }
}

ByteString TranslateLayout::GetCommandLineParam( int i )
{
    return ByteString( OUSTRING_CSTR( Application::GetCommandLineParam( sal::static_int_cast< sal_uInt16 >( i ) ) ) );
}

ByteString TranslateLayout::GetOptionArgument( int const i )
{
    if ( i >= GetCommandLineParamCount() )
        usage();
    ByteString arg = GetCommandLineParam( i );
    if ( !arg.CompareTo( "-", 1 ) )
    {
        fprintf( stderr, "Option needs an argument: %s, found: %s\n",
                 GetCommandLineParam( i - 1 ).GetBuffer(),
                 arg.GetBuffer() );
        usage();
    }
    return arg;
 }

void TranslateLayout::ParseCommandLine()
{
    for ( int i = 0; i < GetCommandLineParamCount(); i++ )
    {
        ByteString aParam = GetCommandLineParam( i );
        if ( aParam.Equals( "-h" ) || aParam.Equals( "--help" ) )
            usage();
        else if ( aParam.Equals( "-l" ) || aParam.Equals( "--language" ) )
            mLanguages.push_back ( GetOptionArgument( ++i ) );
        else if ( aParam.Equals( "-m" ) || aParam.Equals( "--merge" ) )
        {
            mMergeMode = true;
            mLocalize = GetOptionArgument( ++i );
        }
        else if ( aParam.Equals( "-o" ) || aParam.Equals( "--output" ) )
            mOutput = ConvertSystemPath( GetOptionArgument( ++i ) );
        else if ( !aParam.CompareTo( "-", 1 ) )
        {
            fprintf( stderr, "error: No such option: %s\n", aParam.GetBuffer() );
            usage();
        }
        else
            mFiles.push_back( ConvertSystemPath( aParam ) );
    }
    if ( !mFiles.size() )
    {
        fprintf( stderr, "error: No XML-FILE found\n" );
        usage();
    }
}

static XMLAttribute*
findAttribute( XMLAttributeList* lst, String const& name )
{
    for ( size_t i = 0; i < lst->size(); i++ )
        if ( (*lst)[ i ]->Equals( name ) )
            return (*lst)[ i ];
    return 0;
}

static XMLAttribute*
translateAttribute( XMLAttributeList* lst,
                    String const& name, String const& translation )
{
    if ( XMLAttribute* a = findAttribute( lst, name ) )
    {
        for ( XMLAttributeList::iterator it = lst->begin(); it < lst->end(); ++it )
        {
            if ( *it == a )
            {
                delete *it;
                *it = new XMLAttribute( name.Copy( 1 ), translation );
                return *it;
            }
        }
    }
    return 0;
}

static void
translateElement( XMLElement* element, ByteString const& lang,
                  ResData* resData, MergeDataFile& mergeData )
{
    XMLAttributeList* attributes = element->GetAttributeList();
    std::vector<XMLAttribute*> interesting( interestingAttributes( attributes ) );


    if( !interesting.empty() )
    {
        std::vector<XMLAttribute*>::iterator i( interesting.begin() );
        ByteString id = BSTRING( (*i++)->GetValue() );
        for ( ; i != interesting.end(); ++i )
        {
            ByteString attributeId = id;
            attributeId += BSTRING ( **i );
            resData->sGId = attributeId;
            resData->sId = element->GetOldref();

            if ( PFormEntrys* entry = mergeData.GetPFormEntrys( resData ) )
            {
                ByteString translation;
                entry->GetText( translation, STRING_TYP_TEXT, lang, true );
                if ( !translation.Len() )
                    translation = BSTRING( ( *i )->GetValue() );
                delete translateAttribute( attributes, **i , STRING( translation ) );
            }
        }
    }
}

static bool is_dir( ByteString const& name )
{
    DirectoryItem aItem;
    OUString sFileURL( name.GetBuffer(), name.Len(), RTL_TEXTENCODING_UTF8 );
    FileBase::getFileURLFromSystemPath( sFileURL, sFileURL );
    if( DirectoryItem::get( sFileURL, aItem ) == FileBase::E_None )
    {
        FileStatus aStatus(FileStatusMask_Type);
        if( aItem.getFileStatus( aStatus ) == FileBase::E_None )
        {
            if( aStatus.getFileType() == FileStatus::Directory )
                return true;
        }
    }
    return false;
}

static void make_directory( ByteString const& name )
{
    OUString sFileURL( name.GetBuffer(), name.Len(), RTL_TEXTENCODING_UTF8 );
    FileBase::getFileURLFromSystemPath( sFileURL, sFileURL );
    Directory::create( sFileURL );
}

static void insertMarker( XMLParentNode *p, ByteString const& file )
{
    if ( XMLChildNodeList* lst = p->GetChildList() )
        if ( !lst->empty() )
        {
            size_t i = 1;
            // Skip newline, if possible.
            if ( lst->size() > 2
                 && (*lst)[ 2 ]->GetNodeType() == XML_NODE_TYPE_DEFAULT )
                i++;
            OUString marker = OUString(RTL_CONSTASCII_USTRINGPARAM("\n    NOTE: This file has been generated automagically by transex3/layout/tralay,\n          from source template: "))
                + STRING( file )
                + OUString(RTL_CONSTASCII_USTRINGPARAM(".\n          Do not edit, changes will be lost.\n"));
            if ( i < lst->size() ) {
                XMLChildNodeList::iterator it = lst->begin();
                ::std::advance( it, i );
                lst->insert( it, new XMLComment( marker, 0 ) );
            } else {
                lst->push_back( new XMLComment( marker, 0 ) );
            }
        }
}

void TranslateLayout::MergeLanguage( ByteString const& language )
{
    ByteString xmlFile = mFiles.front();

    MergeDataFile mergeData( mLocalize, xmlFile,
                             sal_False, RTL_TEXTENCODING_MS_1252 );

    DirEntry aFile( xmlFile );
    SimpleXMLParser aParser;
    LayoutXMLFile* layoutXml = new LayoutXMLFile( mMergeMode );
    if ( !aParser.Execute( aFile.GetFull() , STRING( xmlFile ), layoutXml ) )
    {
        fprintf(stderr, "error: parsing: %s\n", xmlFile.GetBuffer() );
        return;
    }

    layoutXml->Extract();
    insertMarker( layoutXml, xmlFile );

    ResData resData( "", "", "" );
    resData.sResTyp = mProject; /* mGid1 ?? */
    resData.sFilename = xmlFile;

    XMLHashMap* xmlStrings = layoutXml->GetStrings();
    for ( XMLHashMap::iterator i = xmlStrings->begin(); i != xmlStrings->end();
          ++i )
    {
        if ( LangHashMap* languageMap = i->second )
            if ( XMLElement* element = ( *languageMap )[ "en-US" ] )
                translateElement( element, language, &resData, mergeData );
    }

#ifndef WNT
    ByteString outFile = "/dev/stdout";
#else
    ByteString outFile = "\\\\.\\CON";
#endif
    if ( mOutput.Len() )
    {
        outFile = mOutput;
        if ( is_dir( outFile ) )
        {
            ByteString outDir = mOutput;
            outDir.Append( "/" ).Append( language );
            if ( !is_dir( outDir ) )
                make_directory( outDir );
            outFile = outDir;
            outFile.Append( "/" ).Append( xmlFile );
        }
    }
    layoutXml->Write( outFile );
    delete layoutXml;
}

void TranslateLayout::Merge()
{
    if ( mLanguages.size() )
        for ( std::vector<ByteString>::iterator i = mLanguages.begin();
              i != mLanguages.end(); ++i)
            MergeLanguage( *i );
    else
        MergeLanguage( mLanguage );
}

void TranslateLayout::CreateSDF()
{
    ByteString xmlFile = mFiles.front();
#ifndef WNT
    ByteString sdf = "/dev/stdout";
#else
    ByteString sdf = "\\\\.\\CON";
#endif
    if ( mOutput.Len() )
        sdf = mOutput;
    Export::SetLanguages( mLanguages );
    HelpParser::CreateSDF( sdf, mProject, mRoot, xmlFile,
                           new LayoutXMLFile( mMergeMode ), mGid1 );
}

void TranslateLayout::ExceptionalMain()
{
    ParseCommandLine();
    if ( mLanguages.size() )
        mLanguage = mLanguages.front();
    if ( mMergeMode )
        Merge();
    else
        CreateSDF();
}

void TranslateLayout::Main()
{
    try
    {
        ExceptionalMain();
    }
    catch ( xml::sax::SAXException& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        uno::Exception exc;
        if (rExc.WrappedException >>= exc)
        {
            aStr += OString( " >>> " );
            aStr += OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US );
        }
        fprintf( stderr, "error: parsing: '%s'\n", aStr.getStr() );
        OSL_FAIL( aStr.getStr() );
    }
    catch ( uno::Exception& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "error: UNO: '%s'\n", aStr.getStr() );
        OSL_FAIL( aStr.getStr() );
    }
}

TranslateLayout::TranslateLayout()
    : Application()
    , mGid1( "layout" )
    , mLanguage( "en-US" )
    , mLocalize( "localize.sdf" )
    , mOutput()
    , mProject( "layout" )
    , mRoot()
    , mMergeMode( false )
    , mLanguages()
    , mFiles()
{
}

TranslateLayout::~TranslateLayout()
{
}

SAL_IMPLEMENT_MAIN()
{
    TranslateLayout t;
    t.Main();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
