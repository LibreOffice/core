/*************************************************************************
 *
 *  $RCSfile: rschash.hxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rschash.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.6  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.5  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.4  1997/08/27 18:18:10  MM
    neue Headerstruktur


      Rev 1.3   27 Aug 1997 18:18:10   MM
   neue Headerstruktur

      Rev 1.2   21 Nov 1995 19:49:20   TLX
   Neuer Link

      Rev 1.1   04 May 1995 10:17:20   mk
   ^Z am Ende

      Rev 1.0   10 Aug 1992 07:14:22   MM
   Initial revision.

      Rev 1.4   23 Mar 1992 15:23:50   MM

      Rev 1.3   26 Nov 1991 08:56:06   MM

      Rev 1.2   11 Sep 1991 12:20:10   MM

      Rev 1.1   18 Jul 1991 12:53:26   MM
   Unix Anpassung

      Rev 1.0   17 Jun 1991 14:54:46   MM
   Initial revision.

      Rev 1.0   08 Feb 1991 13:42:52   MM
   Initial revision.

**************************************************************************/

#ifndef _RSCHASH_HXX
#define _RSCHASH_HXX

#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

/****************** T Y P E S ********************************************/
/****************** C L A S S E S ****************************************/
/****************** S t r i n g - C o n **********************************/
class StringCon{
private:
    USHORT    nMax;     // sizeof character field
    USHORT    nFill;    // index after last entry
    char *    pField;   // pointer to character field
public:
           StringCon( USHORT nMaxEntries );  // maximum numbers of characters
           ~StringCon();
    char   *Put( const char * pStr ); // put string in field
};

/****************** H a s h - T a b e l **********************************/
class HashTabel{
private:
    USHORT      nMax;                 // size of hash-tabel
    USHORT      nFill;                // elements in hash-tabel
    ULONG       lAsk;                 // Anzahl der Anfragen
    ULONG       lTry;                 // Anzahl der Versuche
protected:
    HASHID  Test_Insert( const void *, BOOL bInsert );
public:
    HashTabel( USHORT nMaxEntries );    // max size of hash-tabel
    ~HashTabel();
    HASHID  Test( const void * );             // test of insert
    HASHID  Insert( const void * pElement );  // insert in hashtabel
    virtual USHORT HashFunc( const void * ){ return( 0 ); };
                                        // get hash value from subclass
    virtual BOOL IsEntry( HASHID ){ return( FALSE ); };
                                        // is field
                                        // nIndex = index in Field
    virtual COMPARE Compare( const void * , HASHID )
        { return( EQUAL ); };           // compare element with entry
};

/****************** H a s h - S t r i n g ********************************/
class HashString : public HashTabel{
private:
    StringCon * paSC;           // container of strings
    char * *    ppStr;          // pointer to char * array
public:
            HashString( USHORT nMaxEntries );   // max size of hash-tabel
            ~HashString();
    USHORT  HashFunc( const void * pElement );  // return hash value
    BOOL    IsEntry( HASHID nIndex );
    COMPARE Compare( const void * pElement, HASHID nIndex );
    HASHID  Insert( const char * pStr );        // test of insert string
    HASHID  Test( const char * pStr );          // insert string
    char *  Get( HASHID nIndex );               // return pointer to string
};

#endif // _RSCHASH_HXX

