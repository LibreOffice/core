/*************************************************************************
 *
 *  $RCSfile: rscstr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscstr.hxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.12  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.11  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.10  1997/08/27 18:18:12  MM
    neue Headerstruktur


      Rev 1.9   27 Aug 1997 18:18:12   MM
   neue Headerstruktur

      Rev 1.8   12 Sep 1996 12:44:56   MM
   Eigene Ressourcen definieren

      Rev 1.7   21 Nov 1995 19:49:08   TLX
   Neuer Link

      Rev 1.6   12 Jan 1994 15:18:22   mm
   Alignementprobleme und Warnings fuer DEC-Alpha beseitigt

      Rev 1.5   25 Aug 1993 15:34:30   mm
   Fehler und Warnings beseitigt

      Rev 1.4   04 Feb 1993 17:38:44   mm
   GetClassType Fehler verbessert und keine Inline -ctor.

      Rev 1.3   03 Feb 1993 15:03:08   mm
   GetClassType() implementiert

      Rev 1.2   05 Jan 1993 10:50:44   mm
   Neues Default verhalten

      Rev 1.1   23 Dec 1992 14:02:40   mm
   Sprachaenderung

      Rev 1.0   10 Aug 1992 07:14:14   MM
   Initial revision.

      Rev 1.11   23 Mar 1992 15:24:20   MM

      Rev 1.10   16 Jan 1992 12:48:06   MM
   IsVarDefault
**************************************************************************/

#ifndef _RSCSTR_HXX
#define _RSCSTR_HXX

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif

/******************* R s c S t r i n g ***********************************/
class RscString : public RscTop
{
    RscTop * pRefClass;
    struct RscStringInst {
        char *  pStr;   // Zeiger auf String
        BOOL    bDflt;  // Ist Default
        RscId   aRefId; // ReferenzName
    };
    USHORT  nSize;
public:
                    RscString( HASHID nId, USHORT nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;

    void            SetRefClass( RscTop * pClass )
    {
                        pRefClass = pClass;
                    };
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
                    // Der zulaessige Bereich wird gesetzt
    void            Destroy( const RSCINST & rInst );
    USHORT          Size(){ return nSize; }
    void            SetToDefault( const RSCINST & rInst )
                    {
                        ((RscStringInst*)rInst.pData)->bDflt = TRUE;
                    }
    BOOL            IsDefault( const RSCINST & rInst)
                    {
                        return( ((RscStringInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetString( const RSCINST &, char * pStr );
    ERRTYPE         GetString( const RSCINST &, char ** ppStr );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

#endif // _RSCSTR_HXX
