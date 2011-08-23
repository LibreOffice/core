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

#include <hintids.hxx>
#include <errhdl.hxx>
#include <fmthbsh.hxx>
namespace binfilter {


/*************************************************************************
|*
|*    class SwFmtHardBlank
|*
|*    Beschreibung      Dokument 1.20
|*    Ersterstellung    JP 23.11.90
|*    Letzte Aenderung  JP 20.02.91
|*
*************************************************************************/

/*N*/ SwFmtHardBlank::SwFmtHardBlank( sal_Unicode cCh, BOOL bCheck )
/*N*/ 	: SfxPoolItem( RES_TXTATR_HARDBLANK ),
/*N*/ 	cChar( cCh )
/*N*/ {
/*N*/ 	ASSERT( !bCheck || (' ' != cCh && '-' != cCh),
/*N*/ 			"Invalid character for the HardBlank attribute - "
/*N*/ 			"must be a normal unicode character" );
/*N*/ }



/*************************************************************************
|*
|*    class SwFmtSoftHyph
|*
|*    Beschreibung      Dokument 1.20
|*    Ersterstellung    JP 23.11.90
|*    Letzte Aenderung  JP 20.02.91
|*
*************************************************************************/

/*N*/ SwFmtSoftHyph::SwFmtSoftHyph()
/*N*/ 	: SfxPoolItem( RES_TXTATR_SOFTHYPH )
/*N*/ {
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
