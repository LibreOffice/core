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

#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
namespace binfilter {


/************************************************************************
 *						   SwDocStat::SwDocStat()
 ************************************************************************/

/*N*/ SwDocStat::SwDocStat() :
/*N*/ 	nTbl(0),
/*N*/ 	nGrf(0),
/*N*/ 	nOLE(0),
/*N*/ 	nPage(1),
/*N*/ 	nPara(1),
/*N*/ 	nWord(0),
/*N*/ 	nChar(0),
/*N*/ 	bModified(TRUE),
/*N*/ 	pInternStat(0)
/*N*/ {}

/************************************************************************
 *						   void SwDocStat::Reset()
 ************************************************************************/

/*N*/ void SwDocStat::Reset()
/*N*/ {
/*N*/ 	nTbl 	= 0;
/*N*/ 	nGrf 	= 0;
/*N*/ 	nOLE 	= 0;
/*N*/ 	nPage   = 1;
/*N*/ 	nPara   = 1;
/*N*/ 	nWord 	= 0;
/*N*/ 	nChar	= 0;
/*N*/ 	bModified = TRUE;
/*N*/ 	pInternStat = 0;
/*N*/ }

}
