/*************************************************************************
 *
 *  $RCSfile: uicmdstohtml.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-29 15:00:02 $
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

struct Projects
{
    const char* pProjectFolder;
    const char* pResPrefix;
    const char* pCSVPrefix;
    bool        bSlotCommands;
    MODULES     eBelongsTo;
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
    { "offmgr"      , "ofa",    "",         false,  MODULE_BACKINGCOMP    },
    { "dbaccess"    , "dbu",    "",         false,  MODULE_DBACCESS       },
    { 0             , 0,        "",         false,  MODULE_BASIC          }
};

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


struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

typedef std::hash_map< int, CommandLabels > CommandIDToLabelsMap;
typedef std::vector< CommandLabels > CommandLabelsVector;

//typedef std::hash_map< OUString, CommandLabels, OUStringHashCode, ::std::equal_to< OUString > > CommandToLabelsMap;

static CommandIDToLabelsMap moduleMapFiles[MODULE_COUNT];
static CommandLabelsVector moduleCmdVector[MODULE_COUNT];

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

bool WriteLevel( osl::File& rFile, int nLevel )
{
    const char cTab[] = "\t";

    sal_uInt64 nWritten;
    for ( int i = 0; i < nLevel; i++ )
        rFile.write( cTab, strlen( cTab ), nWritten );

    return true;
}

bool WriteSeparator( osl::File& rFile, int nLevel )
{
    static const char MenuSeparator[] = "<menu:menuseparator/>\n";

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuSeparator, strlen( MenuSeparator ), nWritten );

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
                    j++;
                }
            }

            aDir.close();
        }
    }

    OUString aOutDirURL;
    osl::FileBase::getFileURLFromSystemPath( rOutputDirName, aOutDirURL );
    osl::FileBase::getAbsoluteFileURL( aWorkDir, aOutDirURL, aOutDirURL );

    WriteHTMLFile( aOutDirURL );

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
