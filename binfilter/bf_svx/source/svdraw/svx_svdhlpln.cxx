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

#include "svdhlpln.hxx"
#include "svdio.hxx"



#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

namespace binfilter {

/*?*/ SvStream& operator<<(SvStream& rOut, const SdrHelpLine& rHL)
/*?*/ {
/*?*/ 	SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOHlpLID);
/*?*/ 	rOut<<UINT16(rHL.eKind);
/*?*/ 	rOut<<rHL.aPos;
/*?*/ 	return rOut;
/*?*/ }

/*N*/ SvStream& operator>>(SvStream& rIn, SdrHelpLine& rHL)
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rIn,STREAM_READ);
/*N*/ 	UINT16 nDum;
/*N*/ 	rIn>>nDum;
/*N*/ 	rHL.eKind=(SdrHelpLineKind)nDum;
/*N*/ 	rIn>>rHL.aPos;
/*N*/ 	return rIn;
/*N*/ }

/*N*/ void SdrHelpLineList::Clear()
/*N*/ {
/*N*/ 	USHORT nAnz=GetCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		delete GetObject(i);
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ }

/*N*/ void SdrHelpLineList::operator=(const SdrHelpLineList& rSrcList)
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	USHORT nAnz=rSrcList.GetCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		Insert(rSrcList[i]);
/*N*/ 	}
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrHelpLineList& rHLL)
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOHLstID);
/*N*/ 	USHORT nAnz=rHLL.GetCount();
/*N*/ 	rOut<<nAnz;
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		rOut<<rHLL[i];
/*N*/ 	}
/*N*/ 	return rOut;
/*N*/ }

/*N*/ SvStream& operator>>(SvStream& rIn, SdrHelpLineList& rHLL)
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rIn,STREAM_READ);
/*N*/ 	rHLL.Clear();
/*N*/ 	USHORT nAnz;
/*N*/ 	rIn>>nAnz;
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		SdrHelpLine* pHL=new SdrHelpLine;
/*N*/ 		rIn>>*pHL;
/*N*/ 		rHLL.aList.Insert(pHL,CONTAINER_APPEND);
/*N*/ 	}
/*N*/ 	return rIn;
/*N*/ }

}
