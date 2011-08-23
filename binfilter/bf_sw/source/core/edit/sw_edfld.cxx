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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>		// GetCurFld
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _PAM_HXX
#include "pam.hxx"
#endif

#ifndef _DOC_HXX
#include "doc.hxx"
#endif

namespace binfilter {
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::rtl;


// wenn Selektion groesser Max Nodes oder mehr als Max Selektionen
// => keine Attribute
static const USHORT nMaxLookup = 40;

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen zu einer ResId zaehlen
                  wenn 0 alle zaehlen
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: Feldtypen zu einer ResId finden
                  wenn 0 alle finden
 --------------------------------------------------------------------*/

/*--------------------------------------------------------------------
    Beschreibung: Den ersten Typen mit ResId und Namen finden
 --------------------------------------------------------------------*/

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen loeschen
 --------------------------------------------------------------------*/

/*--------------------------------------------------------------------
    Beschreibung: FieldType ueber Name loeschen
 --------------------------------------------------------------------*/



/*************************************************************************
|*
|*					SwEditShell::Insert( SwField )
|*
|*	  Beschreibung	an der Cursorposition ein Feld einfuegen
|*	  Quelle:		vgl. SwEditShell::Insert( String )
|*
*************************************************************************/


/*N*/ inline SwTxtFld *GetDocTxtFld( const SwPosition* pPos )
/*N*/ {
/*N*/ 	SwTxtNode *pNode = pPos->nNode.GetNode().GetTxtNode();
/*N*/ 	if( pNode )
/*N*/ 		return pNode->GetTxtFld( pPos->nContent );
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }

}
