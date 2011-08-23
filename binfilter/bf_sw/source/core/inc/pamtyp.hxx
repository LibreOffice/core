/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _PAMTYP_HXX
#define _PAMTYP_HXX

#ifndef _TXTCMP_HXX //autogen
#include <bf_svtools/txtcmp.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
namespace binfilter {

class SwpHints;
struct SwPosition;
class SwPaM;
class SwTxtAttr;

// Funktions-Deklarationen fuer die Move/Find-Methoden vom SwPaM

void GoStartDoc( SwPosition*);
void GoEndDoc( SwPosition*);
void GoStartSection( SwPosition*);
void GoEndSection( SwPosition*);
FASTBOOL GoInDoc( SwPaM&, SwMoveFn);
FASTBOOL GoInSection( SwPaM&, SwMoveFn);
FASTBOOL GoInNode( SwPaM&, SwMoveFn);
FASTBOOL GoInCntnt( SwPaM&, SwMoveFn);
FASTBOOL GoInCntntCells( SwPaM&, SwMoveFn);
const SwTxtAttr* GetFrwrdTxtHint( const SwpHints&, USHORT&, xub_StrLen );
const SwTxtAttr* GetBkwrdTxtHint( const SwpHints&, USHORT&, xub_StrLen );

FASTBOOL GoNext(SwNode* pNd, SwIndex * pIdx, USHORT nMode );
FASTBOOL GoPrevious(SwNode* pNd, SwIndex * pIdx, USHORT nMode );
SwCntntNode* GoNextNds( SwNodeIndex * pIdx, FASTBOOL );
SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, FASTBOOL );

// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------

FASTBOOL GoPrevPara( SwPaM&, SwPosPara);
FASTBOOL GoCurrPara( SwPaM&, SwPosPara);
FASTBOOL GoNextPara( SwPaM&, SwPosPara);
FASTBOOL GoPrevSection( SwPaM&, SwPosSection);
FASTBOOL GoCurrSection( SwPaM&, SwPosSection);
FASTBOOL GoNextSection( SwPaM&, SwPosSection);


// ------------ Typedefiniton fuer Funktionen ----------------------

typedef FASTBOOL (*GoNd)( SwNode*, SwIndex*, USHORT );
typedef SwCntntNode* (*GoNds)( SwNodeIndex*, FASTBOOL );
typedef void (*GoDoc)( SwPosition* );
typedef void (*GoSection)( SwPosition* );
typedef FASTBOOL (SwPosition:: *CmpOp)( const SwPosition& ) const;
typedef const SwTxtAttr* (*GetHint)( const SwpHints&, USHORT&, xub_StrLen );
typedef int (::utl::TextSearch:: *SearchTxt)( const String&, xub_StrLen*,
                    xub_StrLen*, ::com::sun::star::util::SearchResult* );
typedef void (SwNodes:: *MvSection)( SwNodeIndex * ) const;


struct SwMoveFnCollection
{
    GoNd	  fnNd;
    GoNds	  fnNds;
    GoDoc	  fnDoc;
    GoSection fnSections;
    CmpOp	  fnCmpOp;
    GetHint   fnGetHint;
    SearchTxt fnSearch;
    MvSection fnSection;
};

// --------- Funktionsdefinitionen fuers Suchen --------------
SwCntntNode* GetNode( SwPaM&, FASTBOOL&, SwMoveFn, FASTBOOL bInReadOnly = FALSE );



} //namespace binfilter
#endif
