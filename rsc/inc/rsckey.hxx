/*************************************************************************
 *
 *  $RCSfile: rsckey.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:54 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rsckey.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.6  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.5  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.4  1997/08/27 18:18:06  MM
    neue Headerstruktur


      Rev 1.3   27 Aug 1997 18:18:06   MM
   neue Headerstruktur

      Rev 1.2   21 Nov 1995 19:49:18   TLX
   Neuer Link

      Rev 1.1   16 Feb 1995 19:50:50   MM
   char * -> const char *

      Rev 1.0   10 Aug 1992 07:14:30   MM
   Initial revision.
**************************************************************************/

#ifndef _RSCKEY_HXX
#define _RSCKEY_HXX

/****************** C l a s s   F o r w a r d s **************************/
class RscTop;

#ifndef _RSCALL_H
#include <rscall.h>
#endif

typedef struct {
    HASHID          nName;
    USHORT          nTyp;
    long            yylval;
} KEY_STRUCT;

class RscNameTable {
    BOOL            bSort;      //soll bei jedem einfuegen sortiert werden?
    USHORT          nEntries;   //Anzahl der Eintr„ge
    KEY_STRUCT *    pTable;
public:
            RscNameTable();
            ~RscNameTable();
    void    SetSort( BOOL bSorted = TRUE );
    HASHID  Put( HASHID nName, USHORT nTyp, long nValue );
    HASHID  Put( HASHID nName, USHORT nTyp );
    HASHID  Put( const char * pName, USHORT nTyp, long nValue );
    HASHID  Put( const char * pName, USHORT nTyp );
    HASHID  Put( HASHID nName, USHORT nTyp, RscTop * pClass );
    HASHID  Put( const char * pName, USHORT nTyp, RscTop * pClass );

            // TRUE, wurde gefunden
    BOOL    Get( HASHID nName, KEY_STRUCT * pEle );
};


#endif// _RSCKEY_HXX
