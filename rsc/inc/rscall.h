/*************************************************************************
 *
 *  $RCSfile: rscall.h,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscall.h,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

**************************************************************************/

#ifndef _RSCALL_H
#define _RSCALL_H

#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCDEF_HXX
#include <rscdef.hxx>
#endif

/******************* T y p e s *******************************************/
typedef char * CLASS_DATA;  // Zeiger auf die Daten einer Klasse

/******************* C l a s s e s   F o r w a r d s *********************/
class RscCompiler;
class HashString;
class RscTop;
class RscTypCont;
class RscIncList;

/******************* G l o b a l   V a r i a b l e s *********************/
extern HashString*  pHS;
extern ByteString*  pStdParType;
extern ByteString*  pStdPar1;
extern ByteString*  pStdPar2;
extern ByteString*  pWinParType;
extern ByteString*  pWinPar1;
extern ByteString*  pWinPar2;
extern USHORT       nRefDeep;
extern USHORT       nRsc_XYMAPMODEId;
extern USHORT       nRsc_WHMAPMODEId;
extern USHORT       nRsc_X;
extern USHORT       nRsc_Y;
extern USHORT       nRsc_WIDTH;
extern USHORT       nRsc_HEIGHT;
extern USHORT       nRsc_DELTALANG;
extern USHORT       nRsc_DELTASYSTEM;
extern USHORT       nRsc_EXTRADATA;

/******************* D e f i n e s ***************************************/

#define HELP_FLAG         0x0001  // Hilfe anzeigen
#define NOPREPRO_FLAG     0x0002  // kein Preprozesor
#define NOSYNTAX_FLAG     0x0004  // keine Syntaxanalyse
#define NOLINK_FLAG       0x0008  // nicht linken
#define NORESFILE_FLAG    0x0010  // keine .res-Datei erzeugen
#define DEFINE_FLAG       0x0020  // es wurde Definitionen angegeben
#define INCLUDE_FLAG      0x0040  // der Include-Pfad wurde erweitert
#define MSCPREPRO_FLAG    0x0080  // spezial Preprozessor
#define PRINTSYNTAX_FLAG  0x0100  // Syntax ausgeben
#define PRELOAD_FLAG      0x0200  // Alle Ressourcen Preloaden
#define SMART_FLAG        0x0400  // abgekuertze Name
#define SRSDEFAULT_FLAG   0x1000  // immer der Default geschrieben
#define NOSYSRESTEST_FLAG 0x2000  // ueberprueft nicht die Richtigkeit von (bmp, ico, cur)


/******************* T y p e s *******************************************/
enum RSCCLASS_TYPE  { RSCCLASS_BOOL, RSCCLASS_STRING, RSCCLASS_NUMBER,
                        RSCCLASS_CONST, RSCCLASS_COMPLEX, RSCCLASS_ENUMARRAY };

typedef void (* VarEnumCallbackProc)( void * pData, RSCCLASS_TYPE, HASHID );

/******************* S t r u c t s ***************************************/
struct RSCINST {
    RscTop *    pClass;
    CLASS_DATA  pData;

                RSCINST(){ pClass = NULL; pData = NULL; }
                RSCINST( RscTop * pCl, CLASS_DATA pClassData ){
                    pClass = pCl;
                    pData = pClassData;
                }
    BOOL        IsInst() const { return( pData != NULL ); }
};

/********************** S U B I N F O S T R U C T ************************/
struct SUBINFO_STRUCT {
    SUBINFO_STRUCT(){ nPos = 0; pClass = NULL; };
    RscId    aId;    // Identifier der Resource
    USHORT   nPos;   // Position der Resource
    RscTop * pClass; // Klasse des Eintrages
};

/******************* F u n c t i o n *************************************/
// Legt Hashtabelle an, legt Strings zur Initialisierung an
void InitRscCompiler();

#endif // _RSCALL_H
