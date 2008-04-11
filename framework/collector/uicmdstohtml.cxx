/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uicmdstohtml.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <sal/main.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>

#include <hash_map>
#include <vector>
#include <algorithm>

using namespace rtl;

enum MODULES
{
    MODULE_GLOBAL,
    MODULE_WRITER,
    MODULE_CALC,
    MODULE_DRAWIMPRESS,
    MODULE_CHART,
    MODULE_MATH,
    MODULE_BASIC,
    MODULE_BIBLIO,
    MODULE_BACKINGCOMP,
    MODULE_DBACCESS,
    MODULE_COUNT
};

enum APPMODULES
{
    APPMODULE_BACKINGCOMP,
    APPMODULE_WRITER,
    APPMODULE_WRITERWEB,
    APPMODULE_WRITERGLOBAL,
    APPMODULE_CALC,
    APPMODULE_DRAW,
    APPMODULE_IMPRESS,
    APPMODULE_CHART,
    APPMODULE_BIBLIOGRAPHY,
    APPMODULE_BASICIDE,
    APPMODULE_DBAPP,
    APPMODULE_DBBROWSER,
    APPMODULE_DBQUERY,
    APPMODULE_DBRELATION,
    APPMODULE_DBTABLE,
    APPMODULE_COUNT
};

enum UIELEMENTTYPE
{
    UITYPE_TOOLBAR,
    UITYPE_MENUBAR,
    UITYPE_STATUSBAR,
    UITYPE_ACCELERATOR,
    UITYPE_COUNT
};

struct Projects
{
    const char* pProjectFolder;
    const char* pResPrefix;
    const char* pCSVPrefix;
    bool        bSlotCommands;
    MODULES     eBelongsTo;
};

struct AppModules
{
    const char* pModuleName;
    const char* pModuleIdentifier;
    const char* pProjectFolder;
    const char* pSubFolder;
};

struct UIElementTypeName
{
    const char* Name;
    const char* ShortName;
};

Projects ProjectModule_Mapping[] =
{
    { "sfx2"        , "sfx",    "sfx2",     true,   MODULE_GLOBAL         },
    { "svx"         , "svx",    "svx",      true,   MODULE_GLOBAL         },
    { "svx"         , "ofa",    "ofa",      true,   MODULE_GLOBAL         },
    { "sw"          , "sw",     "sw",       true,   MODULE_WRITER         },
    { "sd"          , "sd",     "sd",       true,   MODULE_DRAWIMPRESS    },
    { "sc"          , "sc",     "sc",       true,   MODULE_CALC           },
    { "sch"         , "sch",    "sch",      true,   MODULE_CHART          },
    { "starmath"    , "sm",     "starmath", true,   MODULE_MATH           },
    { "basctl"      , "basctl", "bastctl",  true,   MODULE_BASIC          },
    { "extensions"  , "bib",    "",         false,  MODULE_BIBLIO         },
    { "framework"   , "fwk",    "",         false,  MODULE_BACKINGCOMP    },
    { "dbaccess"    , "dbu",    "",         false,  MODULE_DBACCESS       },
    { 0             , 0,        "",         false,  MODULE_BASIC          }
};

AppModules AppModules_Mapping[] =
{
    { "Backing Component"       , "startmodule"     ,"framework" , 0                      },
    { "Writer"                  , "swriter"         ,"sw"        , 0                      },
    { "Writer Web"              , "sweb"            ,"sw"        , 0                      },
    { "Writer Global"           , "sglobal"         ,"sw"        , 0                      },
    { "Calc"                    , "scalc"           ,"sc"        , 0                      },
    { "Draw"                    , "sdraw"           ,"sd"        , 0                      },
    { "Impress"                 , "simpress"        ,"sd"        , 0                      },
    { "Chart"                   , "schart"          ,"sch"       , 0                      },
    { "Bibliography"            , "sbibliography"   ,"extensions", "source/bibliography"  },
    { "Basic IDE"               , "basicide"        ,"basctl"    , 0                      },
    { "Database Application"    , "dbapp"           ,"dbaccess"  , 0                      },
    { "Database Browser"        , "dbbrowser"       ,"dbaccess"  , 0                      },
    { "Database Query"          , "dbquery"         ,"dbaccess"  , 0                      },
    { "Database Relation"       , "dbrelation"      ,"dbaccess"  , 0                      },
    { "Database Table"          , "dbtable"         ,"dbaccess"  , 0                      },
    { 0                         , 0                 ,0           , 0                      }
};

UIElementTypeName UIElementTypeFolderName[] =
{
    { "toolbar",    "T"  },
    { "menubar",    "M"  },
    { "statusbar",  "S"  },
    { "accelerator","A"  },
    { 0                  }
};

const char UICONFIGFOLDER[] = "uiconfig";

const char XMLFileExtension[] = ".xcu";
const char* ModuleName[] =
{
    "Global commands",
    "Writer commands",
    "Calc commands",
    "Draw/Impress commands",
    "Chart commands",
    "Math commands",
    "BasicIDE commands",
    "Bibliography commands",
    "Startmodule commands",
    "Database commands",
    0
};

const char* ModuleToXML_Mapping[] =
{
    "GenericCommands",
    "WriterCommands",
    "CalcCommands",
    "DrawImpressCommands",
    "ChartCommands",
    "MathCommands",
    "BasicIDECommands",
    "BibliographyCommands",
    "StartModuleCommands",
    "DbuCommands",
    0
};

struct CommandLabels
{
    unsigned short  nID;
    rtl::OUString   aCommand;
    rtl::OUString   aDefine;
    unsigned long   nModules;
    rtl::OUString   aLabels;
    unsigned long   nProperties;
    rtl::OUString   aUIAvailable;

    CommandLabels() :
    nID( 0 ),
    nModules( 0 ),
    nProperties( 0 )

    {}

    bool CommandLabels::operator< ( const CommandLabels& aCmdLabel ) const
    {
        return ( aCommand.compareTo( aCmdLabel.aCommand ) <= 0 );
    }
};

struct CommandInfo
{
    rtl::OUString   aCommand;
    unsigned long   nAppModules; // bit field for every app module
    unsigned long   nUIElements; // bit field for every ui element type

    bool CommandInfo::operator< ( const CommandInfo& aCmdInfo ) const
    {
        return ( aCommand.compareTo( aCmdInfo.aCommand ) <= 0 );
    }
};

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

typedef std::hash_map< rtl::OUString, CommandInfo, OUStringHashCode, ::std::equal_to< OUString > > CommandToCommandInfoMap;
typedef std::hash_map< int, CommandLabels > CommandIDToLabelsMap;
typedef std::vector< CommandLabels > CommandLabelsVector;
typedef std::vector< CommandInfo > CommandInfoVector;

static CommandIDToLabelsMap moduleMapFiles[MODULE_COUNT];
static CommandLabelsVector moduleCmdVector[MODULE_COUNT];
static CommandToCommandInfoMap commandInfoMap;
static CommandInfoVector commandInfoVector;

bool ExtractVersionNumber( const OUString& rVersion, OUString& rVersionNumber )
{
    bool bCheckNumOnly( false );

    OUStringBuffer aBuf;

    rVersionNumber = OUString();
    for ( int i = 0; i < rVersion.getLength(); i++ )
    {
        if ( rVersion[i] >= sal_Unicode( '0' ) && rVersion[i] <= sal_Unicode( '9' ))
        {
            bCheckNumOnly = true;
            aBuf.append( rVersion[i] );
        }
        else if ( bCheckNumOnly )
            return false;
    }

    rVersionNumber = aBuf.makeStringAndClear();
    return true;
}

bool ReadCSVFile( const OUString& aCVSFileURL, MODULES eModule, const OUString& aProjectName )
{
    osl::File  aCSVFile( aCVSFileURL );
    fprintf(stdout, "############ read csv \"%s\" ... ", ::rtl::OUStringToOString(aCVSFileURL, RTL_TEXTENCODING_UTF8).getStr());
    if ( aCSVFile.open( OpenFlag_Read ) != osl::FileBase::E_None )
    {
        fprintf(stdout, "failed!\n");
        return false;
    }
    fprintf(stdout, "OK!\n");

    {
        sal_Bool            bEOF;
        ::rtl::ByteSequence aCSVLine;
        OUString            aUnoCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

        while ( aCSVFile.isEndOfFile( &bEOF ) == osl::FileBase::E_None && !bEOF )
        {
            aCSVFile.readLine( aCSVLine );

            OString     aLine( (const char *)aCSVLine.getConstArray(), aCSVLine.getLength() );
            OString     aDefine;
            OString     aID;
            OString     aAccelState;
            OString     aMenuState;
            OString     aStatusState;
            OString     aToolbarState;
            OString     aCmdName;
            OString     aSlotName;
            OString     aImageRotationState;
            OString     aImageReflectionState;

            sal_Int32 nIndex = 0;
            aDefine                 = aLine.getToken( 1, ',', nIndex );
            aID                     = aLine.getToken( 0, ',', nIndex );
            aAccelState             = aLine.getToken( 2, ',', nIndex );
            aMenuState              = aLine.getToken( 0, ',', nIndex );
            aStatusState            = aLine.getToken( 0, ',', nIndex );
            aToolbarState           = aLine.getToken( 0, ',', nIndex );
            aImageRotationState     = aLine.getToken( 8, ',', nIndex );
            aImageReflectionState   = aLine.getToken( 0, ',', nIndex );
            aCmdName                = aLine.getToken( 10, ',', nIndex );
            aSlotName               = aLine.getToken( 1, ',', nIndex );

            if ( aCmdName.getLength() == 0 )
                aCmdName = aSlotName;

            int nID = aID.toInt32();

            if ( nID > 5000 && ( aAccelState.equalsIgnoreAsciiCase( "TRUE" ) ||
                                 aMenuState.equalsIgnoreAsciiCase( "TRUE" ) ||
                                 aStatusState.equalsIgnoreAsciiCase( "TRUE" ) ||
                                 aToolbarState.equalsIgnoreAsciiCase( "TRUE" ) ))
            {
                CommandLabels aCmdLabel;

                aCmdLabel.nID = nID;
                aCmdLabel.aCommand += OStringToOUString( aCmdName, RTL_TEXTENCODING_ASCII_US );
                aCmdLabel.nModules |= ( 1 << (unsigned long)( eModule ));
                aCmdLabel.aDefine = OStringToOUString( aDefine, RTL_TEXTENCODING_ASCII_US );

                OUString aUIAvailable;
                if ( aAccelState.equalsIgnoreAsciiCase( "TRUE" ) ||
                     aMenuState.equalsIgnoreAsciiCase( "TRUE" )  ||
                     aToolbarState.equalsIgnoreAsciiCase( "TRUE" ))
                    aUIAvailable = OUString::createFromAscii( "AMT" );
                else if ( aStatusState.equalsIgnoreAsciiCase( "TRUE" ))
                    aUIAvailable = OUString::createFromAscii( "S" );
                aCmdLabel.aUIAvailable = aUIAvailable;

                // Set bitfield
                aCmdLabel.nProperties = (( aImageRotationState.equalsIgnoreAsciiCase( "TRUE" ) ? 1 : 0 ) << 1 );
                aCmdLabel.nProperties |= (( aImageReflectionState.equalsIgnoreAsciiCase( "TRUE" ) ? 1 : 0 ) << 2 );

                moduleMapFiles[int(eModule)].insert( CommandIDToLabelsMap::value_type( nID, aCmdLabel ));
            }
        }

        aCSVFile.close();
    }

    return true;
}

bool ReadXMLFile( const OUString& aFileURL, APPMODULES eAppModule, UIELEMENTTYPE eUIType, const char* pItemTag, const char* pAttributeTag, sal_Int32 nAttributeTagSize )
{
    osl::File  aXMLFile( aFileURL );
    if ( aXMLFile.open( OpenFlag_Read ) != osl::FileBase::E_None )
    {
        fprintf(stdout, "failed!\n");
        return false;
    }

    sal_Bool            bEOF;
    ::rtl::ByteSequence aXMLLine;

    while ( aXMLFile.isEndOfFile( &bEOF ) == osl::FileBase::E_None && !bEOF )
    {
        aXMLFile.readLine( aXMLLine );

        OString aLine( (const char *)aXMLLine.getConstArray(), aXMLLine.getLength() );

        if ( aLine.indexOf( pItemTag ) >= 0 )
        {
            sal_Int32 nIndex = aLine.indexOf( pAttributeTag );
            if (( nIndex >= 0 ) && (( nIndex+nAttributeTagSize+1 ) < aLine.getLength() ))
            {
                sal_Int32 nIndex2 = aLine.indexOf( "\"", nIndex+nAttributeTagSize );
                OString aCmd = aLine.copy( nIndex+nAttributeTagSize, (nIndex2-(nIndex+nAttributeTagSize)) );

                OUString aCmdString = OStringToOUString( aCmd, RTL_TEXTENCODING_ASCII_US );

                CommandToCommandInfoMap::iterator pIter = commandInfoMap.find( aCmdString );
                if ( pIter != commandInfoMap.end() )
                {
                    pIter->second.nAppModules |= ( 1 << eAppModule );
                    pIter->second.nUIElements |= ( 1 << eUIType );
                }
                else
                {
                    CommandInfo aCmdInfo;
                    aCmdInfo.aCommand = aCmdString;
                    aCmdInfo.nAppModules = ( 1 << eAppModule );
                    aCmdInfo.nUIElements = ( 1 << eUIType );
                    commandInfoMap.insert( CommandToCommandInfoMap::value_type( aCmdString, aCmdInfo ));
                }
            }
        }
    }

    return true;
}

bool ReadMenuBarXML( const OUString& aFileURL, APPMODULES eAppModule )
{
    static char MENUITEM_TAG[]      = "<menu:menuitem";
    static char MENUITEM_ID[]       = "menu:id=\"";
    static int  MENUITEM_ID_SIZE    = strlen( MENUITEM_ID );

    return ReadXMLFile( aFileURL, eAppModule, UITYPE_MENUBAR, MENUITEM_TAG, MENUITEM_ID, MENUITEM_ID_SIZE );
}

bool ReadToolBarXML( const OUString& aFileURL, APPMODULES eAppModule )
{
    static char TOOLBARITEM_TAG[]      = "<toolbar:toolbaritem";
    static char TOOLBARITEM_ID[]       = "xlink:href=\"";
    static int  TOOLBARITEM_ID_SIZE    = strlen( TOOLBARITEM_ID );

    return ReadXMLFile( aFileURL, eAppModule, UITYPE_TOOLBAR, TOOLBARITEM_TAG, TOOLBARITEM_ID, TOOLBARITEM_ID_SIZE );
}

bool ReadStatusBarXML( const rtl::OUString& aFileURL, APPMODULES eAppModule )
{
    static char STATUSBARITEM_TAG[]      = "<statusbar:statusbaritem";
    static char STATUSBARITEM_ID[]       = "xlink:href=\"";
    static int  STATUSBARITEM_ID_SIZE    = strlen( STATUSBARITEM_ID );

    return ReadXMLFile( aFileURL, eAppModule, UITYPE_STATUSBAR, STATUSBARITEM_TAG, STATUSBARITEM_ID, STATUSBARITEM_ID_SIZE );
}

bool ReadAcceleratorXML( const rtl::OUString& aFileURL, APPMODULES eAppModule )
{
    static char ACCELERATORITEM_TAG[]      = "<accel:item";
    static char ACCELERATORITEM_ID[]       = "xlink:href=\"";
    static int  ACCELERATORITEM_ID_SIZE    = strlen( ACCELERATORITEM_ID );

    return ReadXMLFile( aFileURL, eAppModule, UITYPE_ACCELERATOR, ACCELERATORITEM_TAG, ACCELERATORITEM_ID, ACCELERATORITEM_ID_SIZE );
}

bool ReadXMLFile( const rtl::OUString& aFileURL, APPMODULES eAppModule, UIELEMENTTYPE eUIElementType )
{
    switch ( eUIElementType )
    {
        case UITYPE_TOOLBAR:
            return ReadToolBarXML( aFileURL, eAppModule );
        case UITYPE_MENUBAR:
            return ReadMenuBarXML( aFileURL, eAppModule );
        case UITYPE_STATUSBAR:
            return ReadStatusBarXML( aFileURL, eAppModule );
        case UITYPE_ACCELERATOR:
            return ReadAcceleratorXML( aFileURL, eAppModule );
        default:
            return false;
    }
}

bool ReadXMLFilesForAppModule( const rtl::OUString& aFolderURL, APPMODULES eAppModule )
{
    osl::Directory aDir( aFolderURL );
    if ( aDir.open() == osl::FileBase::E_None )
    {
        osl::DirectoryItem aFolderItem;
        while ( aDir.getNextItem( aFolderItem ) == osl::FileBase::E_None )
        {
            osl::FileStatus aFileStatus( FileStatusMask_FileName );

            aFolderItem.getFileStatus( aFileStatus );

            OUString aFolderName = aFileStatus.getFileName();
            int i=0;
            while ( i < UITYPE_COUNT )
            {
                if ( aFolderName.equalsAscii( UIElementTypeFolderName[i].Name ))
                {
                    OUStringBuffer aBuf( aFolderURL );
                    aBuf.appendAscii( "/" );
                    aBuf.append( aFolderName );

                    OUString aUIElementFolderURL( aBuf.makeStringAndClear() );
                    osl::Directory aUIElementTypeDir( aUIElementFolderURL );
                    if ( aUIElementTypeDir.open() == osl::FileBase::E_None )
                    {
                        osl::DirectoryItem aItem;
                        while ( aUIElementTypeDir.getNextItem( aItem ) == osl::FileBase::E_None )
                        {
                            osl::FileStatus aFileStatus( FileStatusMask_FileName );

                            aItem.getFileStatus( aFileStatus );
                            if ( aFileStatus.getFileType() == osl_File_Type_Regular )
                            {
                                OUStringBuffer aBuf( aUIElementFolderURL );
                                aBuf.appendAscii( "/" );
                                aBuf.append( aFileStatus.getFileName() );

                                ReadXMLFile( aBuf.makeStringAndClear(), eAppModule, (UIELEMENTTYPE)i );
                            }
                            else if (( aFileStatus.getFileType() == osl_File_Type_Directory ) &&
                                     ( aFileStatus.getFileName().equalsAscii( "en-US" )))
                            {
                                // Accelerators are language-dependent
                                OUStringBuffer aBuf( aUIElementFolderURL );
                                aBuf.appendAscii( "/" );
                                aBuf.append( aFileStatus.getFileName() );
                                aBuf.appendAscii( "/default.xml" );

                                ReadXMLFile( aBuf.makeStringAndClear(), eAppModule, (UIELEMENTTYPE)i );
                            }
                        }
                    }
                    break;
                }
                ++i;
            }
        }
    }

    return true;
}

void SortCommandTable( MODULES eModule )
{
    // copy entries from hash_map to vector
    CommandIDToLabelsMap::iterator pIter = moduleMapFiles[int( eModule)].begin();
    while ( pIter != moduleMapFiles[int( eModule )].end() )
    {
        CommandLabels& rCmdLabels = pIter->second;
        moduleCmdVector[int( eModule )].push_back( rCmdLabels );
        ++pIter;
    }

    CommandLabelsVector::iterator pIterStart = moduleCmdVector[int(eModule)].begin();
    CommandLabelsVector::iterator pIterEnd   = moduleCmdVector[int(eModule)].end();

    std::sort( pIterStart, pIterEnd );
}

void SortCommandInfoVector()
{
    // copy entries from hash_map to vector
    CommandToCommandInfoMap::iterator pIter = commandInfoMap.begin();
    while ( pIter != commandInfoMap.end() )
    {
        CommandInfo& rCmdInfo = pIter->second;
        commandInfoVector.push_back( rCmdInfo );
        ++pIter;
    }

    CommandInfoVector::iterator pIterStart = commandInfoVector.begin();
    CommandInfoVector::iterator pIterEnd   = commandInfoVector.end();

    std::sort( pIterStart, pIterEnd );
}

CommandLabels* RetrieveCommandLabelsFromID( unsigned short nId, MODULES eModule )
{
    CommandIDToLabelsMap::iterator pIter = moduleMapFiles[MODULE_GLOBAL].find( nId );
    if ( pIter != moduleMapFiles[MODULE_GLOBAL].end() )
        return &(pIter->second);
    else if ( eModule != MODULE_GLOBAL )
    {
        CommandIDToLabelsMap::iterator pIter = moduleMapFiles[eModule].find( nId );
        if ( pIter != moduleMapFiles[eModule].end() )
            return &(pIter->second);
    }

    return NULL;
}

static const char ENCODED_AMPERSAND[]  = "&amp;";
static const char ENCODED_LESS[]       = "&lt;";
static const char ENCODED_GREATER[]    = "&gt;";
static const char ENCODED_QUOTE[]      = "&quot;";
static const char ENCODED_APOS[]       = "&apos;";

struct EncodeChars
{
    char  cChar;
    const char* pEncodedChars;
};

EncodeChars EncodeChar_Map[] =
{
    { '&', ENCODED_AMPERSAND    },
    { '<', ENCODED_LESS         },
    { '>', ENCODED_GREATER      },
    { '"', ENCODED_QUOTE        },
    { '\'', ENCODED_APOS        },
    { ' ', 0                    }
};

// Encodes a string to UTF-8 and uses "Built-in entity reference" to
// to escape character data that is not markup!
OString EncodeString( const OUString& aToEncodeStr )
{
    OString aString = OUStringToOString( aToEncodeStr, RTL_TEXTENCODING_UTF8 );

    int     i = 0;
    bool    bMustCopy( sal_False );
    while ( EncodeChar_Map[i].pEncodedChars != 0 )
    {
        OStringBuffer aBuf;
        bool bEncoded( false );

        sal_Int32 nCurIndex = 0;
        sal_Int32 nIndex    = 0;
        while ( nIndex < aString.getLength() )
        {
            nIndex = aString.indexOf( EncodeChar_Map[i].cChar, nIndex );
            if ( nIndex > 0 )
            {
                bEncoded = true;
                if ( nIndex > nCurIndex )
                    aBuf.append( aString.copy( nCurIndex, nIndex-nCurIndex ));
                aBuf.append( EncodeChar_Map[i].pEncodedChars );
                nCurIndex = nIndex+1;
            }
            else if ( nIndex == 0 )
            {
                bEncoded = true;
                aBuf.append( EncodeChar_Map[i].pEncodedChars );
                nCurIndex = nIndex+1;
            }
            else
            {
                if ( bEncoded && nCurIndex < aString.getLength() )
                    aBuf.append( aString.copy( nCurIndex ));
                break;
            }
            ++nIndex;
        }

        if ( bEncoded )
            aString = aBuf.makeStringAndClear();
        ++i;
    }

    return aString;
}

static const char HTMLStart[]        = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n";
static const char HTMLHead[]         = "<html>\n \
 <head>\n \
 \n \
 <meta http-equiv=\"CONTENT-TYPE\"\n \
 content=\"text/html; charset=windows-1252\">\n \
 <title>OpenOffice.org 2.0 : Index of command names</title>\n \
\n \
  <style>\n \
        <!-- \n \
        BODY,DIV,TABLE,THEAD,TBODY,TFOOT,TR,TH,TD,P { font-family:\"Albany\"; font-size:x-small }\n \
         -->\n \
    </style>\n \
\n \
</head>\n \
<body text=\"#000000\">";

static const char HTMLJumpTable[] = "<h1><b>Command tables for OpenOffice.org 2.0<br/>\n \
</b></h1>\n \
<br/>\n \
<p>The following tables list all available commands that are\n \
accessible through the GUI. The command names  are used with the \".uno:\"\n \
protocol scheme to define framework commands. These  command URLs can be\n \
used to dispatch/execute, like \".uno:About\" shows the about box.\n \
Please keep in mind that slot-URLs are deprecated for OpenOffice.org 2.0 and\n \
shouldn't be used anymore.<br/>\n \
<br/>\n \
Abbreviations:<br/>\n \
A = accelerator<br/>\n \
M = menu<br/>\n \
T = toolbox<br/>\n \
S = status bar<br/>\n \
</p>\n \
<br/>\n";

bool WriteLevel( osl::File& rFile, int nLevel )
{
    const char cTab[] = "\t";

    sal_uInt64 nWritten;
    for ( int i = 0; i < nLevel; i++ )
        rFile.write( cTab, strlen( cTab ), nWritten );

    return true;
}

bool WriteJumpTable( osl::File& rFile )
{
    static const char JUMP_ENTRY_1[] = "- ";
    static const char JUMP_LINK_1[]  = "<a href=\"#";
    static const char JUMP_LINK_2[]  = "\">";
    static const char JUMP_LINK_3[]  = "</a>";
    static const char JUMP_ENTRY_2[] = "<br/>\n";

    sal_uInt64 nWritten;

    rFile.write( HTMLJumpTable, strlen( HTMLJumpTable ), nWritten );

    sal_uInt32 i = 0;
    while ( ModuleName[i] != 0 )
    {
        if ( moduleMapFiles[i].size() > 0 )
        {
            OString aTableNameStr( ModuleName[i] );
            OStringBuffer aJumpMark( aTableNameStr );

            // convert jump mark
            for ( sal_Int32 j = 0; j < aJumpMark.getLength(); j++ )
                if ( aJumpMark[j] == ' ' )
                    aJumpMark.setCharAt( j, '_' );

            rFile.write( JUMP_ENTRY_1, strlen( JUMP_ENTRY_1 ), nWritten );
            rFile.write( JUMP_LINK_1, strlen( JUMP_LINK_1 ), nWritten );
            rFile.write( aJumpMark, aJumpMark.getLength(), nWritten );
            rFile.write( JUMP_LINK_2, strlen( JUMP_LINK_2 ), nWritten );
            rFile.write( aTableNameStr, aTableNameStr.getLength(), nWritten );
            rFile.write( JUMP_LINK_3, strlen( JUMP_LINK_3 ), nWritten );
            rFile.write( JUMP_ENTRY_2, strlen( JUMP_ENTRY_2 ), nWritten );
        }
        i++;
    }

    return true;
}

bool WriteHTMLTable( osl::File& rFile, const rtl::OUString& aTableName )
{
    static const char TABLE_NAME1[] = "<h2>";
    static const char TABLE_NAME2[] = "</h2>\n";
    static const char ANCHOR_1[]    = "<a name=\"";
    static const char ANCHOR_2[]    = "\"></a>";
    static const char TABLE_FRAME[] = "<table frame=\"box\" cellspacing=\"0\" cols=\"4\" rules=\"groups\" border=\"1\">\n";
    static const char COLGROUPS1[]  = "<colgroup><col width=\"189\"></colgroup>     <colgroup><col width=\"93\"></colgroup>\n";
    static const char COLGROUPS2[]  = "<colgroup><col width=\"277\"></colgroup>     <colgroup><col width=\"47\"></colgroup>\n";
    static const char TBODY_1[]     = "<tbody>\n\t<tr>\n";
    static const char TBODY_2[]     = "\t</tr>\n</tbody>\n";
    static const char TD_1[]        = "<td width=\"189\" height=\"17\" align=\"left\"><b>Commands</b></td>\n";
    static const char TD_2[]        = "<td width=\"93\" align=\"left\"><b>GUI Elements</b></td>\n";
    static const char TD_3[]        = "<td width=\"277\" align=\"left\"><b>Internal definitions of commands ID</b></td>\n";
    static const char TD_4[]        = "<td width=\"47\" align=\"left\"><b>ID</b></td>\n";

    if ( aTableName.getLength() > 0 )
    {
        sal_uInt64 nWritten;
        OString aTableNameStr = OUStringToOString( aTableName, RTL_TEXTENCODING_ASCII_US );
        OStringBuffer aJumpMark( aTableNameStr );

        // convert jump mark
        for ( sal_Int32 j = 0; j < aJumpMark.getLength(); j++ )
            if ( aJumpMark[j] == ' ' )
                aJumpMark.setCharAt( j, '_' );

        rFile.write( TABLE_NAME1, strlen( TABLE_NAME1 ), nWritten );
        rFile.write( ANCHOR_1, strlen( ANCHOR_1 ), nWritten );
        rFile.write( aJumpMark, aJumpMark.getLength(), nWritten );
        rFile.write( ANCHOR_2, strlen( ANCHOR_2 ), nWritten );
        rFile.write( aTableNameStr, aTableNameStr.getLength(), nWritten );
        rFile.write( TABLE_NAME2, strlen( TABLE_NAME2 ), nWritten );
        rFile.write( TABLE_FRAME, strlen( TABLE_FRAME ), nWritten );
        rFile.write( COLGROUPS1, strlen( COLGROUPS1 ), nWritten );
        rFile.write( COLGROUPS2, strlen( COLGROUPS2 ), nWritten );
        rFile.write( TBODY_1, strlen( TBODY_1 ), nWritten );
        rFile.write( TD_1, strlen( TD_1 ), nWritten );
        rFile.write( TD_2, strlen( TD_2 ), nWritten );
        rFile.write( TD_3, strlen( TD_3 ), nWritten );
        rFile.write( TD_4, strlen( TD_4 ), nWritten );
    }

    return true;
}

bool WriteHTMLTableRow( osl::File& rFile, const CommandLabels& rCmdLabels )
{
    static const char TBODY_1[]     = "<tbody>\n\t<tr>\n";
    static const char TBODY_2[]     = "\t</tr>\n</tbody>\n";
    static const char TD_FIRST[]    = "\t<td height=\"17\" align=\"left\">";
    static const char TD_L[]        = "\t<td align=\"left\">";
    static const char TD_R[]        = "\t<td align=\"right\">";
    static const char TD_CLOSE[]    = "</td>\n";

    OString aCmdStr = OUStringToOString( rCmdLabels.aCommand, RTL_TEXTENCODING_UTF8 );
    OString aUIStr  = OUStringToOString( rCmdLabels.aUIAvailable, RTL_TEXTENCODING_UTF8 );
    OString aDefStr = OUStringToOString( rCmdLabels.aDefine, RTL_TEXTENCODING_UTF8 );
    OString aIDStr  = OString::valueOf( sal_Int32( rCmdLabels.nID ));

    sal_uInt64 nWritten;
    rFile.write( TBODY_1, strlen( TBODY_1 ), nWritten );

    rFile.write( TD_FIRST, strlen( TD_FIRST ), nWritten );
    rFile.write( aCmdStr, aCmdStr.getLength(), nWritten );
    rFile.write( TD_CLOSE, strlen( TD_CLOSE ), nWritten );

    rFile.write( TD_L, strlen( TD_L ), nWritten );
    rFile.write( aUIStr, aUIStr.getLength(), nWritten );
    rFile.write( TD_CLOSE, strlen( TD_CLOSE ), nWritten );

    rFile.write( TD_L, strlen( TD_L ), nWritten );
    rFile.write( aDefStr, aDefStr.getLength(), nWritten );
    rFile.write( TD_CLOSE, strlen( TD_CLOSE ), nWritten );

    rFile.write( TD_R, strlen( TD_R ), nWritten );
    rFile.write( aIDStr, aIDStr.getLength(), nWritten );
    rFile.write( TD_CLOSE, strlen( TD_CLOSE ), nWritten );

    rFile.write( TBODY_2, strlen( TBODY_2 ), nWritten );

    return true;
}

bool WriteHTMLTableClose( osl::File& rFile )
{
    static const char TBODY_CLOSE[]     = "\t</tr>\n</tbody>\n";
    static const char TABLE_CLOSE[]     = "</table>\n";
    static const char BR[]              = "<br/>\n";

    sal_uInt64 nWritten;
    rFile.write( TBODY_CLOSE, strlen( TBODY_CLOSE ), nWritten );
    rFile.write( TABLE_CLOSE, strlen( TABLE_CLOSE ), nWritten );
    rFile.write( BR, strlen( BR ), nWritten );

    return true;
}

bool WriteHTMLFile( const OUString& aOutputDirURL)
{
    OUString aOutputDirectoryURL( aOutputDirURL );
    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
        aOutputDirectoryURL += OUString::createFromAscii( "/" );

    OUString aOutputFileURL( aOutputDirectoryURL );
    aOutputFileURL += OUString::createFromAscii( "commandsReference.html" );

    osl::File aHTMLFile( aOutputFileURL );
    osl::File::RC nRet = aHTMLFile.open( OpenFlag_Create|OpenFlag_Write );
    if ( nRet == osl::File::E_EXIST )
    {
        nRet = aHTMLFile.open( OpenFlag_Write );
        if ( nRet == osl::File::E_None )
            nRet = aHTMLFile.setSize( 0 );
    }

    if ( nRet == osl::FileBase::E_None )
    {
        sal_uInt64 nWritten;

        aHTMLFile.write( HTMLStart, strlen( HTMLStart ), nWritten );
        aHTMLFile.write( HTMLHead, strlen( HTMLHead ), nWritten );

        WriteJumpTable( aHTMLFile );

        int i = 0;
        while ( ModuleToXML_Mapping[i] != 0 )
        {
            SortCommandTable( MODULES( i ));
            if ( moduleCmdVector[i].size() > 0 )
            {
                WriteHTMLTable( aHTMLFile, OUString::createFromAscii( ModuleName[i] ));

                for ( sal_uInt32 j = 0; j < moduleCmdVector[i].size(); j++ )
                {
                    CommandLabels& rCmdLabels = (moduleCmdVector[i])[j];
                    WriteHTMLTableRow( aHTMLFile, rCmdLabels );
                }

                WriteHTMLTableClose( aHTMLFile );
            }
            ++i;
        }
        aHTMLFile.close();
    }

    return true;
}

bool WriteUICommand( osl::File& rFile, const CommandInfo& rCmdInfo )
{
    static const char MODULE_HAS_COMMAND[] = ",1";
    static const char MODULE_NOT_COMMAND[] = ",0";
    static const char NEWLINE[]            = "\n";
    static const char COMMA[]              = ",";

    sal_uInt64 nWritten;

    OString aCommand = OUStringToOString( rCmdInfo.aCommand, RTL_TEXTENCODING_ASCII_US );

    rFile.write( aCommand, aCommand.getLength(), nWritten );
    rFile.write( COMMA, strlen( COMMA ), nWritten );

    if ( rCmdInfo.nUIElements & ( 1<<UITYPE_ACCELERATOR ))
        rFile.write( UIElementTypeFolderName[UITYPE_ACCELERATOR].ShortName, strlen( UIElementTypeFolderName[UITYPE_ACCELERATOR].ShortName ), nWritten );
    if ( rCmdInfo.nUIElements & ( 1<<UITYPE_TOOLBAR))
        rFile.write( UIElementTypeFolderName[UITYPE_TOOLBAR].ShortName, strlen( UIElementTypeFolderName[UITYPE_TOOLBAR].ShortName ), nWritten );
    if ( rCmdInfo.nUIElements & ( 1<<UITYPE_MENUBAR ))
        rFile.write( UIElementTypeFolderName[UITYPE_MENUBAR].ShortName, strlen( UIElementTypeFolderName[UITYPE_MENUBAR].ShortName ), nWritten );
    if ( rCmdInfo.nUIElements & ( 1<<UITYPE_STATUSBAR ))
        rFile.write( UIElementTypeFolderName[UITYPE_STATUSBAR].ShortName, strlen( UIElementTypeFolderName[UITYPE_STATUSBAR].ShortName ), nWritten );

    sal_Int32 nApps = rCmdInfo.nAppModules;
    for ( sal_Int32 i = 0; i < APPMODULE_COUNT; i++ )
    {
        if ( nApps & ( 1 << i ))
            rFile.write( MODULE_HAS_COMMAND, strlen( MODULE_HAS_COMMAND ), nWritten );
        else
            rFile.write( MODULE_NOT_COMMAND, strlen( MODULE_NOT_COMMAND ), nWritten );
    }
    rFile.write( NEWLINE, strlen( NEWLINE ), nWritten );

    return true;
}

bool WriteUICommands( const OUString& aOutputDirURL )
{
    static const char COMMA[]   = ",";
    static const char HEADER[]  = "User interface command, GUI Element";
    static const char NEWLINE[] = "\n";

    OUString aOutputDirectoryURL( aOutputDirURL );
    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
        aOutputDirectoryURL += OUString::createFromAscii( "/" );

    OUString aOutputFileURL( aOutputDirectoryURL );
    aOutputFileURL += OUString::createFromAscii( "UsedCommands.csv" );

    osl::File aCSVFile( aOutputFileURL );
    osl::File::RC nRet = aCSVFile.open( OpenFlag_Create|OpenFlag_Write );
    if ( nRet == osl::File::E_EXIST )
    {
        nRet = aCSVFile.open( OpenFlag_Write );
        if ( nRet == osl::File::E_None )
            nRet = aCSVFile.setSize( 0 );
    }

    if ( nRet == osl::FileBase::E_None )
    {
        sal_uInt64 nWritten;

        aCSVFile.write( HEADER, strlen( HEADER ), nWritten );
        for ( sal_Int32 i = 0; i < APPMODULE_COUNT; i++ )
        {
            aCSVFile.write( COMMA, strlen( COMMA ), nWritten );
            aCSVFile.write( AppModules_Mapping[i].pModuleName, strlen( AppModules_Mapping[i].pModuleName ), nWritten );
        }
        aCSVFile.write( NEWLINE, strlen( NEWLINE ), nWritten );

        for ( sal_uInt32 j = 0; j < commandInfoVector.size(); j++ )
        {
            const CommandInfo& rCmdInfo = commandInfoVector[j];
            WriteUICommand( aCSVFile, rCmdInfo );
        }

        aCSVFile.close();
    }

    return true;
}

bool Convert( sal_Bool        bUseProduct,
              const OUString& aUseRes,
              const OUString& rVersion,
              const OUString& rOutputDirName,
              const OUString& rPlatformName,
              const std::vector< OUString >& rInDirVector,
              const OUString& rErrOutputFileName )
{
    OUString aWorkDir;

    osl_getProcessWorkingDir( &aWorkDir.pData );

    // Try to find xx*.csv file and put all commands into hash table
    for ( int i = 0; i < (int)rInDirVector.size(); i++ )
    {
        OUString aAbsInDirURL;
        OUString aInDirURL;
        OUString aInDir( rInDirVector[i] );

        osl::FileBase::getFileURLFromSystemPath( aInDir, aInDirURL );
        osl::FileBase::getAbsoluteFileURL( aWorkDir, aInDirURL, aAbsInDirURL );
        osl::Directory aDir( aAbsInDirURL );
        if ( aDir.open() == osl::FileBase::E_None )
        {
            osl::DirectoryItem aItem;
            while ( aDir.getNextItem( aItem ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( FileStatusMask_FileName );

                aItem.getFileStatus( aFileStatus );

                int j=0;
                OUString aFileName = aFileStatus.getFileName();

                while ( ProjectModule_Mapping[j].pProjectFolder != 0 &&
                        ProjectModule_Mapping[j].bSlotCommands == true )
                {
                    if ( aFileName.equalsAscii( ProjectModule_Mapping[j].pProjectFolder ))
                    {
                        OUStringBuffer aBuf( aAbsInDirURL );

                        aBuf.appendAscii( "/" );
                        aBuf.append( aFileStatus.getFileName() );
                        aBuf.appendAscii( "/" );
                        aBuf.append( rPlatformName );
                        if ( bUseProduct )
                            aBuf.appendAscii( ".pro" );
                        aBuf.appendAscii( "/misc/xx" );
                        aBuf.appendAscii( ProjectModule_Mapping[j].pCSVPrefix );
                        aBuf.appendAscii( ".csv" );

                        OUString aCSVFileURL( aBuf.makeStringAndClear() );
                        ReadCSVFile( aCSVFileURL, ProjectModule_Mapping[j].eBelongsTo, OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ));
                        break;
                    }
                    ++j;
                }

                j = 0;
                while ( AppModules_Mapping[j].pModuleIdentifier != 0 )
                {
                    if ( aFileName.equalsAscii( AppModules_Mapping[j].pProjectFolder ))
                    {
                        OUStringBuffer aBuf( aAbsInDirURL );

                        aBuf.appendAscii( "/" );
                        aBuf.append( aFileName );
                        aBuf.appendAscii( "/" );
                        if ( AppModules_Mapping[j].pSubFolder != 0 )
                        {
                            aBuf.appendAscii( AppModules_Mapping[j].pSubFolder );
                            aBuf.appendAscii( "/" );
                        }
                        aBuf.appendAscii( UICONFIGFOLDER );
                        aBuf.appendAscii( "/" );
                        aBuf.appendAscii( AppModules_Mapping[j].pModuleIdentifier );

                        OUString aXMLAppModuleFolder( aBuf.makeStringAndClear() );
                        ReadXMLFilesForAppModule( aXMLAppModuleFolder, (APPMODULES)j );
                    }
                    ++j;
                }
            }

            aDir.close();
        }
    }

    OUString aOutDirURL;
    osl::FileBase::getFileURLFromSystemPath( rOutputDirName, aOutDirURL );
    osl::FileBase::getAbsoluteFileURL( aWorkDir, aOutDirURL, aOutDirURL );

    WriteHTMLFile( aOutDirURL );

    SortCommandInfoVector();
    WriteUICommands( aOutDirURL );

    return true;
}

bool GetCommandOption( const ::std::vector< OUString >& rArgs, const OUString& rSwitch, OUString& rParam )
{
    bool        bRet = false;
    OUString    aSwitch( OUString::createFromAscii( "-" ));

    aSwitch += rSwitch;
    for( int i = 0, nCount = rArgs.size(); ( i < nCount ) && !bRet; i++ )
    {
        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            if ( aSwitch.equalsIgnoreAsciiCase( rArgs[ i ] ))
            {
                bRet = true;

                if( i < ( nCount - 1 ) )
                    rParam = rArgs[ i + 1 ];
                else
                    rParam = OUString();
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool GetCommandOptions( const ::std::vector< OUString >& rArgs, const OUString& rSwitch, ::std::vector< OUString >& rParams )
{
    bool bRet = false;

    OUString    aSwitch( OUString::createFromAscii( "-" ));

    aSwitch += rSwitch;
    for( int i = 0, nCount = rArgs.size(); ( i < nCount ); i++ )
    {
        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            if ( aSwitch.equalsIgnoreAsciiCase( rArgs[ i ] ))
            {
                if( i < ( nCount - 1 ) )
                    rParams.push_back( rArgs[ i + 1 ] );
                else
                    rParams.push_back( OUString() );

                break;
            }
        }
    }

    return( rParams.size() > 0 );
}

void ShowUsage()
{
    fprintf( stderr, "Usage: uicmdxml output_dir -r res -v version [-p] -s platform -i input_dir [-i input_dir] [-f errfile]\n" );
    fprintf( stderr, "Options:" );
    fprintf( stderr, "   -r [oo|so]    use resources from ooo, so\n" );
    fprintf( stderr, "   -v            name of the version used, f.e. SRX645, SRC680\n" );
    fprintf( stderr, "   -p            use product version\n" );
    fprintf( stderr, "   -s            name of the system to use, f.e. wntmsci8, unxsols4, unxlngi5\n" );
    fprintf( stderr, "   -i ...        name of directory to be searched for input files [multiple occurence is possible]\n" );
    fprintf( stderr, "   -f            name of file, error output should be written to\n" );
    fprintf( stderr, "Examples:\n" );
    fprintf( stderr, "    uicmd2html /home/out -r so -v SRC680 -p unxlngi5 -i /home/res -f /home/out/log.err\n" );
}

// -----------------------------------------------------------------------

int main ( int argc, char ** argv)
{
    ::std::vector< OUString > aArgs;

    sal_uInt32 nCmds = argc;
    if ( nCmds >= 8 )
    {
        for ( sal_Int32 i=0; i < sal_Int32( nCmds ); i++ )
        {
            OUString aArg = OStringToOUString( argv[i], osl_getThreadTextEncoding() );
            aArgs.push_back( aArg );
        }

        ::std::vector< OUString > aInDirVector;
        OUString                  aErrOutputFileName;
        OUString                  aOutputDirName( aArgs[0] );
        OUString                  aOutDirURL;
        OUString                  aPlatformName;
        OUString                  aVersion;
        OUString                  aUseRes;
        bool                      bUseProduct;
        OUString                  aDummy;

        osl::FileBase::getFileURLFromSystemPath( aOutputDirName, aOutDirURL );
        sal_Int32 j = aOutDirURL.lastIndexOf( '/' );
        if ( j > 0 )
            aOutDirURL = aOutDirURL.copy( 0, j+1 );
        osl::FileBase::getSystemPathFromFileURL( aOutDirURL, aOutputDirName );

        GetCommandOption( aArgs, OUString::createFromAscii( "v" ), aVersion );
        bUseProduct = GetCommandOption( aArgs, OUString::createFromAscii( "p" ), aDummy );
        GetCommandOption( aArgs, OUString::createFromAscii( "s" ), aPlatformName );
        GetCommandOptions( aArgs, OUString::createFromAscii( "i" ), aInDirVector );
        GetCommandOption( aArgs, OUString::createFromAscii( "f" ), aErrOutputFileName );
        GetCommandOption( aArgs, OUString::createFromAscii( "r" ), aUseRes );

        if ( aVersion.getLength() > 0 &&
             aPlatformName.getLength() > 0 &&
             aUseRes.getLength() > 0 &&
             aInDirVector.size() > 0 )
        {
            Convert( bUseProduct, aUseRes, aVersion, aOutputDirName, aPlatformName, aInDirVector, aErrOutputFileName );
        }
        else
        {
            ShowUsage();
            exit( -1 );
        }
    }
    else
    {
        ShowUsage();
        exit( -1 );
    }
}
