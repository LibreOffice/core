/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <svtools/txtcmp.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <tools/string.hxx>

class SwpHints;
struct SwPosition;
class SwPaM;
class SwTxtAttr;

// Funktions-Deklarationen fuer die Move/Find-Methoden vom SwPaM

void GoStartDoc( SwPosition*);
void GoEndDoc( SwPosition*);
void GoStartSection( SwPosition*);
void GoEndSection( SwPosition*);
sal_Bool GoInDoc( SwPaM&, SwMoveFn);
sal_Bool GoInSection( SwPaM&, SwMoveFn);
sal_Bool GoInNode( SwPaM&, SwMoveFn);
sal_Bool GoInCntnt( SwPaM&, SwMoveFn);
sal_Bool GoInCntntCells( SwPaM&, SwMoveFn);
sal_Bool GoInCntntSkipHidden( SwPaM&, SwMoveFn);
sal_Bool GoInCntntCellsSkipHidden( SwPaM&, SwMoveFn);
const SwTxtAttr* GetFrwrdTxtHint( const SwpHints&, sal_uInt16&, xub_StrLen );
const SwTxtAttr* GetBkwrdTxtHint( const SwpHints&, sal_uInt16&, xub_StrLen );

sal_Bool GoNext(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode );
sal_Bool GoPrevious(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode );
SW_DLLPUBLIC SwCntntNode* GoNextNds( SwNodeIndex * pIdx, sal_Bool );
SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, sal_Bool );

// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------

sal_Bool GoPrevPara( SwPaM&, SwPosPara);
sal_Bool GoCurrPara( SwPaM&, SwPosPara);
sal_Bool GoNextPara( SwPaM&, SwPosPara);
sal_Bool GoPrevSection( SwPaM&, SwPosSection);
sal_Bool GoCurrSection( SwPaM&, SwPosSection);
sal_Bool GoNextSection( SwPaM&, SwPosSection);


// ------------ Typedefiniton fuer Funktionen ----------------------

typedef sal_Bool (*GoNd)( SwNode*, SwIndex*, sal_uInt16 );
typedef SwCntntNode* (*GoNds)( SwNodeIndex*, sal_Bool );
typedef void (*GoDoc)( SwPosition* );
typedef void (*GoSection)( SwPosition* );
typedef sal_Bool (SwPosition:: *CmpOp)( const SwPosition& ) const;
typedef const SwTxtAttr* (*GetHint)( const SwpHints&, sal_uInt16&, xub_StrLen );
typedef int (utl::TextSearch:: *SearchTxt)( const String&, xub_StrLen*,
                    xub_StrLen*, ::com::sun::star::util::SearchResult* );
typedef void (SwNodes:: *MvSection)( SwNodeIndex * ) const;


struct SwMoveFnCollection
{
    GoNd      fnNd;
    GoNds     fnNds;
    GoDoc     fnDoc;
    GoSection fnSections;
    CmpOp     fnCmpOp;
    GetHint   fnGetHint;
    SearchTxt fnSearch;
    MvSection fnSection;
};

// --------- Funktionsdefinitionen fuers Suchen --------------
SwCntntNode* GetNode( SwPaM&, sal_Bool&, SwMoveFn, sal_Bool bInReadOnly = sal_False );



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
