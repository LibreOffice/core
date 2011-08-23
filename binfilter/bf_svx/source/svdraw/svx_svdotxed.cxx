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

#include "svdotext.hxx"


namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
//  TextEdit
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FASTBOOL SdrTextObj::HasTextEdit() const
/*N*/ {
/*N*/ 	// lt. Anweisung von MB duerfen gelinkte Textobjekte nun doch
/*N*/ 	// geaendert werden (kein automatisches Reload)
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ FASTBOOL SdrTextObj::BegTextEdit(SdrOutliner& rOutl)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }


/*N*/ void SdrTextObj::EndTextEdit(SdrOutliner& rOutl)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ SdrObject* SdrTextObj::CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	return SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer);
/*N*/ }



}
