/*************************************************************************
 *
 *  $RCSfile: rsclex.hxx,v $
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

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.18  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.17  2000/07/26 17:13:21  willem.vandorp
    Headers/footers replaced

    Revision 1.16  1999/09/24 13:15:07  hr
    forward decl.

    Revision 1.15  1998/09/24 17:51:52  HR
    GCC braucht richtige Vorwaertsdeklaration


      Rev 1.14   24 Sep 1998 17:51:52   HR
   GCC braucht richtige Vorwaertsdeklaration

      Rev 1.13   27 Aug 1997 18:17:16   MM
   neue Headerstruktur

      Rev 1.12   18 Oct 1996 16:27:12   MM
   dynamische Member

      Rev 1.11   24 Jul 1996 12:49:02   mda
   IRIX

      Rev 1.10   18 Jun 1996 13:38:14   MM
   Header Anpassungen

      Rev 1.9   05 Jun 1996 19:22:40   PL
   Deklarationen fuer SCO

      Rev 1.8   21 Nov 1995 19:49:50   TLX
   Neuer Link

      Rev 1.7   10 Jan 1995 11:23:04   eg
   HP9000 yyerror( const char* )

      Rev 1.6   09 Sep 1994 12:08:06   MH
   Watcom

      Rev 1.5   25 Apr 1994 15:02:20   sv
   HP9000 needs different declarations for yyerror and yylex (extern C)

      Rev 1.4   12 Jan 1994 15:20:44   mm
   Alignementprobleme und Warnings fuer DEC-Alpha beseitigt

      Rev 1.3   21 Sep 1993 10:03:44   mm
   RS6000  needs different prototyps (extern "C")

      Rev 1.2   25 Aug 1993 15:25:02   mm
   Fehler und Warnings beseitigt

      Rev 1.1   23 Dec 1992 14:05:44   mm
   Sprachaenderung

      Rev 1.0   10 Aug 1992 07:22:46   MM
   Initial revision.

**************************************************************************/
#include <tools/stack.hxx>

#define MINBUF          256
#define MAXBUF          256

enum MODE_ENUM { MODE_MODELESS, MODE_APPLICATIONMODAL, MODE_SYSTEMMODAL };

enum JUSTIFY_ENUM { JUST_CENTER, JUST_RIGHT, JUST_LEFT };

enum SHOW_ENUM { SHOW_NORMAL, SHOW_MINIMIZED, SHOW_MAXIMIZED };

enum ENUMHEADER { HEADER_NAME, HEADER_NUMBER };

enum REF_ENUM { TYPE_NOTHING, TYPE_REF, TYPE_COPY };

struct RSCHEADER {
    RscTop *    pClass;
    RscExpType  nName1;
    REF_ENUM    nTyp;
    RscTop *    pRefClass;
    RscExpType  nName2;
};

DECLARE_STACK( RscCharStack, char * )
void PutStringBack( char * pStr );

/************** O b j e c t s t a c k ************************************/
struct Node {
    Node*   pPrev;
    RSCINST aInst;
    USHORT  nTupelRec;  // Rekursionstiefe fuer Tupel
    Node() { pPrev = NULL; nTupelRec = 0; };
};

class ObjectStack {
    private :
        Node* pRoot;
    public :

        ObjectStack ()   { pRoot = NULL; }

        const RSCINST & Top  ()     { return pRoot->aInst; }
        BOOL        IsEmpty()   { return( pRoot == NULL ); }
        void        IncTupelRec() { pRoot->nTupelRec++; }
        void        DecTupelRec() { pRoot->nTupelRec--; }
        USHORT      TupelRecCount() const { return pRoot->nTupelRec; }
        void        Push( RSCINST aInst )
                    {
                        Node* pTmp;

                        pTmp         = pRoot;
                        pRoot        = new Node;
                        pRoot->aInst = aInst;
                        pRoot->pPrev = pTmp;
                    }
        void        Pop()
                    {
                       Node* pTmp;

                       pTmp  = pRoot;
                       pRoot = pTmp->pPrev;
                       delete pTmp;
                    }
};

/****************** F o r w a r d s **************************************/
#if defined( RS6000 )
extern "C" int yyparse();   // forward Deklaration fuer erzeugte Funktion
extern "C" void yyerror( char * );
extern "C" int  yylex( void );
#elif defined( HP9000 ) || defined( SCO ) || defined ( IRIX ) || defined ( SOLARIS )
extern "C" int yyparse();   // forward Deklaration fuer erzeugte Funktion
extern "C" void yyerror( const char * );
extern "C" int  yylex( void );
#else
#if defined ( WTC ) || defined ( GCC )
int yyparse();              // forward Deklaration fuer erzeugte Funktion
#else
yyparse();              // forward Deklaration fuer erzeugte Funktion
#endif
void yyerror( char * );
int  yylex( void );
#endif

class RscTypCont;
class RscFileInst;
class RscCharStack;

extern RscTypCont*              pTC;
extern RscFileInst *            pFI;
extern RscCharStack *           pCS;
extern RscExpression *          pExp;
extern ObjectStack              S;
