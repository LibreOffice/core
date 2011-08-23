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

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include <shellio.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <errhdl.hxx>


#include <cmdid.h>
namespace binfilter {


/******************************************************************************
 *	Methode		:	SwDocFac::SwDocFac( SwDoc *pDoc )
 *	Beschreibung:
 *	Erstellt	:	OK 01-24-94 11:32am
 *	Aenderung	:	OK 01-24-94 11:32am
 ******************************************************************************/


/*N*/ SwDocFac::SwDocFac( SwDoc *pDc )
/*N*/ 	: pDoc( pDc )
/*N*/ {
/*N*/ 	if( pDoc )
/*N*/ 		pDoc->AddLink();
/*N*/ }

/******************************************************************************
 *	Methode		:   SwDocFac::~SwDocFac()
 *	Beschreibung:
 *	Erstellt	:	OK 01-24-94 11:33am
 *	Aenderung	:	OK 01-24-94 11:33am
 ******************************************************************************/


/*N*/ SwDocFac::~SwDocFac()
/*N*/ {
/*N*/ 	if( pDoc && !pDoc->RemoveLink() )
/*?*/ 		delete pDoc;
/*N*/ }

/******************************************************************************
 *	Methode		:	SwDoc *SwDocFac::GetDoc()
 *	Beschreibung:	Diese Methode legt immer einen Drucker an.
 *	Erstellt	:	OK 01-24-94 11:34am
 *	Aenderung	:	OK 01-24-94 11:34am
 ******************************************************************************/


/*N*/ SwDoc *SwDocFac::GetDoc()
/*N*/ {
/*N*/ 	if( !pDoc )
/*N*/ 	{
/*N*/ 		pDoc = new SwDoc;
/*N*/ 		pDoc->AddLink();
/*N*/ 	}
/*N*/ 	return pDoc;
/*N*/ }


/******************************************************************************
 *  Erstellt    :   JP 01-27-94 11:37am
 *  Aenderung   :   JP 01-27-94 11:37am
 ******************************************************************************/



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
