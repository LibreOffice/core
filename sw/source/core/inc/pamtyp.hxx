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
BOOL GoInDoc( SwPaM&, SwMoveFn);
BOOL GoInSection( SwPaM&, SwMoveFn);
BOOL GoInNode( SwPaM&, SwMoveFn);
BOOL GoInCntnt( SwPaM&, SwMoveFn);
BOOL GoInCntntCells( SwPaM&, SwMoveFn);
BOOL GoInCntntSkipHidden( SwPaM&, SwMoveFn);
BOOL GoInCntntCellsSkipHidden( SwPaM&, SwMoveFn);
const SwTxtAttr* GetFrwrdTxtHint( const SwpHints&, USHORT&, xub_StrLen );
const SwTxtAttr* GetBkwrdTxtHint( const SwpHints&, USHORT&, xub_StrLen );

BOOL GoNext(SwNode* pNd, SwIndex * pIdx, USHORT nMode );
BOOL GoPrevious(SwNode* pNd, SwIndex * pIdx, USHORT nMode );
SW_DLLPUBLIC SwCntntNode* GoNextNds( SwNodeIndex * pIdx, BOOL );
SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, BOOL );

// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------

BOOL GoPrevPara( SwPaM&, SwPosPara);
BOOL GoCurrPara( SwPaM&, SwPosPara);
BOOL GoNextPara( SwPaM&, SwPosPara);
BOOL GoPrevSection( SwPaM&, SwPosSection);
BOOL GoCurrSection( SwPaM&, SwPosSection);
BOOL GoNextSection( SwPaM&, SwPosSection);


// ------------ Typedefiniton fuer Funktionen ----------------------

typedef BOOL (*GoNd)( SwNode*, SwIndex*, USHORT );
typedef SwCntntNode* (*GoNds)( SwNodeIndex*, BOOL );
typedef void (*GoDoc)( SwPosition* );
typedef void (*GoSection)( SwPosition* );
typedef BOOL (SwPosition:: *CmpOp)( const SwPosition& ) const;
typedef const SwTxtAttr* (*GetHint)( const SwpHints&, USHORT&, xub_StrLen );
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
SwCntntNode* GetNode( SwPaM&, BOOL&, SwMoveFn, BOOL bInReadOnly = FALSE );



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
