/*************************************************************************
#*
#*    $Workfile:        syshelp.cxx  $
#*
#*    class
#*
#*    Beschreibung
#*
#*    Ersterstellung    NP
#*    Letzte Aenderung  $Author: np $
#*                      $Date: 2001-03-09 15:22:59 $
#*    $Revision: 1.1 $
#*
#*    $Logfile:         T:/solar/source/...  $
#*
#*    Copyright (c) 2000, Star Office Gmbh
#*
#**************************************************************************/


#include <syshelp.hxx>


// NOT FULLY DEFINED SERVICES
#include <fstream.h>
#include <string.h>
#include "sistr.hxx"



char C_sSpaceInName[] = "&nbsp;&nbsp;&nbsp;";

void
WriteName( ofstream &       o_rFile,
           const Simstr &   i_rIdlDocuBaseDir,
           const Simstr &   i_rName,
           E_LinkType       i_eLinkType )
{
    if (i_rName.l() == 0)
        return;


    const char * pNameEnd = strstr( i_rName.str(), " in " );

    // No link:
    if ( i_eLinkType == lt_nolink )
    {
        if ( pNameEnd != 0 )
        {
            const char * pStart = i_rName.str();
            o_rFile.write( pStart, pNameEnd -1 - pStart );
            WriteStr( o_rFile, C_sSpaceInName );
            WriteStr( o_rFile, pNameEnd );
        }
        else
        {
            WriteStr( o_rFile, i_rName );
        }
        return;
    }

    if ( i_eLinkType == lt_idl )
    {
        Simstr sPath(i_rName);
        sPath.replace_all('.','/');
        int nNameEnd = sPath.pos_first(' ');
        int nPathStart = sPath.pos_last(' ');
        WriteStr( o_rFile, "<A HREF=\"" );

        if ( nNameEnd > -1 )
        {
            WriteStr( o_rFile, "file:///" );
            WriteStr( o_rFile, i_rIdlDocuBaseDir );
            WriteStr( o_rFile, "/" );
            WriteStr( o_rFile, sPath.str() + 1 + nPathStart );
            WriteStr( o_rFile, "/" );
            o_rFile.write( sPath.str(), nNameEnd );
            WriteStr( o_rFile, ".html\">" );
        }
        else
        {   // Should not be reached:
            WriteStr(o_rFile, i_rName);
            return;
        }
    }
    else if ( i_eLinkType == lt_html )
    {
        int nKomma = i_rName.pos_first(',');
        int nEnd = i_rName.pos_first(' ');
        if ( nKomma > -1 )
        {
            o_rFile.write( i_rName.str(), nKomma );
            WriteStr( o_rFile, ": " );

            WriteStr( o_rFile, "<A HREF=\"" );

            o_rFile.write( i_rName.str(), nKomma );
            WriteStr( o_rFile, ".html#" );
            if ( nEnd > -1 )
                o_rFile.write( i_rName.str() + nKomma + 1, nEnd - nKomma );
            else
                WriteStr( o_rFile, i_rName.str() + nKomma + 1 );
            WriteStr( o_rFile, "\">" );

            o_rFile.write( i_rName.str() + nKomma + 1, nEnd - nKomma );
        }
        else
        {
            WriteStr( o_rFile, "<A HREF=\"" );
            WriteStr( o_rFile, i_rName );
            WriteStr( o_rFile, ".html\">" );

            WriteStr( o_rFile, i_rName );
        }
        WriteStr( o_rFile, "</A>" );
        return;
    }

    if ( pNameEnd != 0 )
    {
        const char * pStart = i_rName.str();
        if ( pNameEnd > pStart )
            o_rFile.write( pStart, pNameEnd - pStart );
        WriteStr( o_rFile, "</A>" );

        WriteStr( o_rFile, C_sSpaceInName );
        WriteStr( o_rFile, pNameEnd );
    }
    else
    {
        WriteStr( o_rFile, i_rName );
        WriteStr( o_rFile, "</A>" );
    }
}


void
WriteStr( ofstream &        o_rFile,
          const char *      i_sStr )
{
    o_rFile.write( i_sStr, strlen(i_sStr) );
}

void
WriteStr( ofstream &          o_rFile,
          const Simstr &      i_sStr )
{
    o_rFile.write( i_sStr.str(), i_sStr.l() );
}

