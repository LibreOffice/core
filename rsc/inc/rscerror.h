/*************************************************************************
 *
 *  $RCSfile: rscerror.h,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscerror.h,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.14  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.13  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.12  2000/07/11 17:00:17  th
    Unicode

    Revision 1.11  1997/08/27 18:18:16  MM
    neue Headerstruktur

**************************************************************************/

#ifndef _RSCERROR_H
#define _RSCERROR_H

#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif

/****************** D E F I N I T I O N S ********************************/
/******************* R e t u r n   E r r o r s         *******************/
#define ERR_OK              0xFFFF

#define ERR_ERROR           0x0100
#define ERR_UNKNOWN_METHOD  0x0101  // Return
#define ERR_OPENFILE        0x0102  // Return
#define ERR_NOCHAR          0x0103  // Return
#define ERR_NORSCINST       0x0104  // Return
#define ERR_USAGE           0x0105  //
#define ERR_NOINPUT         0x0106  // Return
#define ERR_UNKNOWNSW       0x0107  //
#define ERR_REFTODEEP       0x0108  // Return
#define ERR_FILEFORMAT      0x0109  // Return
#define ERR_FILESIZE        0x010A  // Return

#define ERR_RSCRANGE        0x0200  // Return
#define ERR_RSCRANGE_OUTDEFSET  (ERR_RSCRANGE +1 )

#define ERR_RSCENUM         0x0210  // Return
#define ERR_RSCFLAG         0x0220  // Return
#define ERR_RSCCONT         0x0240  // Return
#define ERR_CONT_INVALIDPOS     (ERR_RSCCONT +1 ) // Return
#define ERR_CONT_INVALIDTYPE    (ERR_RSCCONT +2 ) // Return

#define ERR_RSCCMPED        0x0250
#define ERR_RSCINST         0x0260
#define ERR_RSCINST_NOVARNAME   (ERR_RSCINST +1 ) // Return
#define ERR_RSCINST_RESERVEDNAME (ERR_RSCINST +2 ) // Return

#define ERR_LEX             0x0270
#define ERR_YACC            0x0280  //
#define ERR_DOUBLEID            (ERR_YACC    +1 ) //
#define ERR_FALSETYPE           (ERR_YACC    +2 ) //
#define ERR_NOVARIABLENAME      (ERR_YACC    +3 ) //
#define ERR_USHORTRANGE         (ERR_YACC    +4 ) //
#define ERR_IDRANGE             (ERR_YACC    +5 ) //
#define ERR_NOCOPYOBJ           (ERR_YACC    +6 ) //
#define ERR_REFNOTALLOWED       (ERR_YACC    +7 ) // Return
#define ERR_DOUBLEDEFINE        (ERR_YACC    +8 ) //
#define ERR_COPYNOTALLOWED      (ERR_YACC    +9 ) //
#define ERR_IDEXPECTED          (ERR_YACC    +10) //
#define ERR_ZERODIVISION        (ERR_YACC    +11) //
#define ERR_PRAGMA              (ERR_YACC    +12) //
#define ERR_DECLAREDEFINE       (ERR_YACC    +13) //
#define ERR_NOTUPELNAME         (ERR_YACC    +14) //
#define ERR_NOTYPE              (ERR_YACC    +15) //

#define ERR_RSCARRAY         0x02A0  // Return
#define ERR_ARRAY_INVALIDINDEX  (ERR_RSCARRAY +1 ) // Return

#define ERR_ERROREND                0x1000

#define ERR_WARNINGSTART        0x1001
#define WRN_LOCALID             (ERR_WARNINGSTART +1 )
#define WRN_GLOBALID            (ERR_WARNINGSTART +2 )
#define WRN_SUBINMEMBER         (ERR_WARNINGSTART +3 )
#define WRN_CONT_NOID           (ERR_WARNINGSTART +4 )
#define WRN_STR_REFNOTFOUND     (ERR_WARNINGSTART +5 )
#define WRN_MGR_REFNOTFOUND     (ERR_WARNINGSTART +6 )
#define WRN_CONT_DOUBLEID       (ERR_WARNINGSTART +7 )

#define ERR_WARNINGEND      0x2000

class ERRTYPE {
    USHORT  nError;
public:
    ERRTYPE()                { nError = ERR_OK; }
    ERRTYPE( USHORT nErr )   { nError = nErr; }
    ERRTYPE( const ERRTYPE & rErr ) { nError = rErr.nError; };
    ERRTYPE& operator = ( const ERRTYPE & rError );
    operator  USHORT() const { return( nError ); }
    BOOL IsError() const     { return( nError <= ERR_ERROREND ); }
    BOOL IsOk() const        { return( !IsError() ); }
    BOOL IsWarning() const   {
        return( nError >= ERR_WARNINGSTART && nError <= ERR_WARNINGEND );
    };
    void    Clear(){ nError = ERR_OK; }
};

/****************** R s c E r r o r **************************************/
class RscId;
class RscTop;

class RscError
{
    FILE *  fListing;

    void WriteError( const ERRTYPE& rError, const char * pMessage );
    void StdLstOut( const char * pStr );
    void ErrorFormat( const ERRTYPE& rError, RscTop * pClass,
                      const RscId & aId );
public:
    USHORT  nErrors;// Anzahl der Fehler
                    RscError(){
                        fListing = NULL;
                        nErrors = 0;
                    };
    void            SetListFile( FILE * fList ){
                        fListing = fList;
                    };
    FILE *          GetListFile(){
                        return fListing;
                    };
    virtual void    StdOut( const char * );
    virtual void    LstOut( const char * );
    virtual void    Error( const ERRTYPE& rError, RscTop* pClass, const RscId &aId,
                           const char * pMessage = NULL );
    // Dieser Fehler sollte nur im Compilermodus auftreten,
    // das Programm wird mit exit() verlassen
    virtual void    FatalError( const ERRTYPE& rError, const RscId &aId,
                                const char * pMessage = NULL );
};

#endif // _RSCERROR_H
