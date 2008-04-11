/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rsclex.hxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
