/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pamtyp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:57:49 $
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

#ifndef _PAMTYP_HXX
#define _PAMTYP_HXX

#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
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
SwCntntNode* GoNextNds( SwNodeIndex * pIdx, BOOL );
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
