/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rsclex.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:34:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <tools/stack.hxx>

#include <hash_set>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>

// a buffer for unique strings
class StringContainer
{
    std::hash_set< rtl::OString, rtl::OStringHash >     m_aStrings;
public:
    StringContainer() {}
    ~StringContainer() {}

    const char* putString( const char* pString );
};


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

/************** O b j e c t s t a c k ************************************/
struct Node {
    Node*   pPrev;
    RSCINST aInst;
    sal_uInt32  nTupelRec;  // Rekursionstiefe fuer Tupel
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
        sal_uInt32  TupelRecCount() const { return pRoot->nTupelRec; }
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
#if defined ( WTC ) || defined ( GCC ) || (_MSC_VER >= 1400)
int yyparse();              // forward Deklaration fuer erzeugte Funktion
#else
yyparse();              // forward Deklaration fuer erzeugte Funktion
#endif
void yyerror( char * );
int  yylex( void );
#endif

class RscTypCont;
class RscFileInst;

extern RscTypCont*              pTC;
extern RscFileInst *            pFI;
extern RscExpression *          pExp;
extern ObjectStack              S;
extern StringContainer*         pStringContainer;
