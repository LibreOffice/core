/*************************************************************************
 *
 *  $RCSfile: pamtyp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-22 14:09:08 $
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
FASTBOOL GoInDoc( SwPaM&, SwMoveFn);
FASTBOOL GoInSection( SwPaM&, SwMoveFn);
FASTBOOL GoInNode( SwPaM&, SwMoveFn);
FASTBOOL GoInCntnt( SwPaM&, SwMoveFn);
const SwTxtAttr* GetFrwrdTxtHint( const SwpHints&, USHORT&, xub_StrLen );
const SwTxtAttr* GetBkwrdTxtHint( const SwpHints&, USHORT&, xub_StrLen );

FASTBOOL GoNext(SwNode* pNd, SwIndex * pIdx );
FASTBOOL GoPrevious(SwNode* pNd, SwIndex * pIdx);
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

typedef FASTBOOL (*GoNd)( SwNode*, SwIndex* );
typedef SwCntntNode* (*GoNds)( SwNodeIndex*, FASTBOOL );
typedef void (*GoDoc)( SwPosition* );
typedef void (*GoSection)( SwPosition* );
typedef FASTBOOL (SwPosition:: *CmpOp)( const SwPosition& ) const;
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
SwCntntNode* GetNode( SwPaM&, FASTBOOL&, SwMoveFn, FASTBOOL bInReadOnly = FALSE );



#endif
