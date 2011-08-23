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

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

namespace binfilter {


//------------------ Methoden der CrsrShell ---------------------------

// Parameter fuer das Suchen vom FormatCollections






// Suchen nach Format-Collections


/*M*/ ULONG SwCursor::Find( const SwTxtFmtColl& rFmtColl,
/*M*/                     SwDocPositions nStart, SwDocPositions nEnde, BOOL& bCancel,
/*M*/ 					FindRanges eFndRngs, const SwTxtFmtColl* pReplFmtColl )
/*M*/ {DBG_BF_ASSERT(0, "STRIP");return 0;//STRIP001 
/*M*/ 	// OLE-Benachrichtigung abschalten !!
/*M*/ }



}
