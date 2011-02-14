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

#ifndef SC_OP_H
#define SC_OP_H

#include <tools/solar.h>
#include <patattr.hxx>

// OP-Code-Funktionen
class SvStream;
void NI( SvStream &aStream, sal_uInt16 nLaenge );
void OP_BOF( SvStream &aStream, sal_uInt16 nLaenge );
void OP_EOF( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Integer( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Number( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Label( SvStream &aStream, sal_uInt16 nLaenge );
//UNUSED2009-05 void OP_Text( SvStream &aStream, sal_uInt16 nLaenge );      // WK3
void OP_Integer3( SvStream &aStream, sal_uInt16 nLaenge );  // WK3
void OP_Number3( SvStream &aStream, sal_uInt16 nLaenge );   // WK3
void OP_Formula( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Formula3( SvStream &aStream, sal_uInt16 nLaenge );  // WK3
void OP_ColumnWidth( SvStream &aStream, sal_uInt16 nLaenge );
void OP_NamedRange( SvStream &aStream, sal_uInt16 nLaenge );
void OP_SymphNamedRange( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Footer( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Header( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Margins( SvStream &aStream, sal_uInt16 nLaenge );
void OP_HiddenCols( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Window1( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Blank( SvStream &aStream, sal_uInt16 nLaenge );
// Lotus 123 bits.
void OP_BOF123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_EOF123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Number123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Label123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Formula123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_IEEENumber123(SvStream& r, sal_uInt16 n);
void OP_Note123(SvStream &aStream, sal_uInt16 nLaenge);
void OP_CreatePattern123(SvStream &aStream, sal_uInt16 nLaenge);
void OP_SheetName123( SvStream &rStream, sal_uInt16 nLength );
void OP_HorAlign123(sal_uInt8 nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_VerAlign123(sal_uInt8 nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_ApplyPatternArea123(SvStream& r);

#endif
