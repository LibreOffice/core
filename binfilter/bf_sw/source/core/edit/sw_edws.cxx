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


#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
namespace binfilter {

/********************************************************
 * Ctor/Dtor
 ********************************************************/

// ctor/dtor


/*N*/ SwEditShell::~SwEditShell() // USED
/*N*/ {
/*N*/ }

/******************************************************************************
 *				   void SwEditShell::StartAllAction()
 ******************************************************************************/


/*N*/ void SwEditShell::StartAllAction()
/*N*/ {
/*N*/ 	ViewShell *pSh = this;
/*N*/ 	do {
/*N*/ 		if( pSh->IsA( TYPE( SwEditShell ) ) )
/*N*/ 			((SwEditShell*)pSh)->StartAction();
/*N*/ 		else
/*?*/ 			pSh->StartAction();
/*N*/ 		pSh = (ViewShell *)pSh->GetNext();
/*N*/ 	} while(pSh != this);
/*N*/ }
/******************************************************************************
 *					void SwEditShell::EndAllAction()
 ******************************************************************************/


/*N*/ void SwEditShell::EndAllAction()
/*N*/ {
/*N*/ 	ViewShell *pSh = this;
/*N*/ 	do {
/*N*/ 		if( pSh->IsA( TYPE( SwEditShell ) ) )
/*N*/ 			((SwEditShell*)pSh)->EndAction();
/*N*/ 		else
/*?*/ 			pSh->EndAction();
/*N*/ 		pSh = (ViewShell *)pSh->GetNext();
/*N*/ 	} while(pSh != this);
/*N*/ }

/******************************************************************************
 *					void SwEditShell::CalcLayout()
 ******************************************************************************/


/*N*/ void SwEditShell::CalcLayout()
/*N*/ {
/*N*/ 	StartAllAction();
/*N*/ 	ViewShell::CalcLayout();
/*N*/ 
/*N*/ 	ViewShell *pSh = this;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if ( pSh->GetWin() )
/*N*/ 			pSh->GetWin()->Invalidate();
/*N*/ 		pSh = (ViewShell*)pSh->GetNext();
/*N*/ 
/*N*/ 	} while ( pSh != this );
/*N*/ 
/*N*/ 	EndAllAction();
/*N*/ }

}
