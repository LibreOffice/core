/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: op.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:30:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OP_H
#define _OP_H

#include <tools/solar.h>
#include <patattr.hxx>

// OP-Code-Funktionen
class SvStream;
void NI( SvStream &aStream, USHORT nLaenge );
void OP_BOF( SvStream &aStream, USHORT nLaenge );
void OP_EOF( SvStream &aStream, USHORT nLaenge );
void OP_Integer( SvStream &aStream, USHORT nLaenge );
void OP_Number( SvStream &aStream, USHORT nLaenge );
void OP_Label( SvStream &aStream, USHORT nLaenge );
void OP_Text( SvStream &aStream, USHORT nLaenge );      // WK3
void OP_Integer3( SvStream &aStream, USHORT nLaenge );  // WK3
void OP_Number3( SvStream &aStream, USHORT nLaenge );   // WK3
void OP_Formula( SvStream &aStream, USHORT nLaenge );
void OP_Formula3( SvStream &aStream, USHORT nLaenge );  // WK3
void OP_ColumnWidth( SvStream &aStream, USHORT nLaenge );
void OP_NamedRange( SvStream &aStream, USHORT nLaenge );
void OP_SymphNamedRange( SvStream &aStream, USHORT nLaenge );
void OP_Footer( SvStream &aStream, USHORT nLaenge );
void OP_Header( SvStream &aStream, USHORT nLaenge );
void OP_Margins( SvStream &aStream, USHORT nLaenge );
void OP_HiddenCols( SvStream &aStream, USHORT nLaenge );
void OP_Window1( SvStream &aStream, USHORT nLaenge );
void OP_Blank( SvStream &aStream, USHORT nLaenge );
// Lotus 123 bits.
void OP_BOF123( SvStream &aStream, USHORT nLaenge );
void OP_EOF123( SvStream &aStream, USHORT nLaenge );
void OP_Number123( SvStream &aStream, USHORT nLaenge );
void OP_Label123( SvStream &aStream, USHORT nLaenge );
void OP_Formula123( SvStream &aStream, USHORT nLaenge );
void OP_IEEENumber123(SvStream& r, UINT16 n);
void OP_Note123(SvStream &aStream, USHORT nLaenge);
void OP_CreatePattern123(SvStream &aStream, USHORT nLaenge);
void OP_SheetName123( SvStream &rStream, USHORT nLength );
void OP_HorAlign123(BYTE nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_VerAlign123(BYTE nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_ApplyPatternArea123(SvStream& r);

#endif
