/*************************************************************************
 *
 *  $RCSfile: syshelp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2001-03-12 19:24:51 $
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


#include <syshelp.hxx>


// NOT FULLY DEFINED SERVICES
#include <string.h>
#include "sistr.hxx"



char C_sSpaceInName[] = "&nbsp;&nbsp;&nbsp;";

void
WriteName( std::ofstream &      o_rFile,
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
WriteStr( std::ofstream &       o_rFile,
          const char *      i_sStr )
{
    o_rFile.write( i_sStr, strlen(i_sStr) );
}

void
WriteStr( std::ofstream &         o_rFile,
          const Simstr &      i_sStr )
{
    o_rFile.write( i_sStr.str(), i_sStr.l() );
}

