/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ttstrhlp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:56:19 $
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
#ifndef _BASIC_TTSTRHLP_HXX
#define _BASIC_TTSTRHLP_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#define CByteString( constAsciiStr ) ByteString( RTL_CONSTASCII_STRINGPARAM ( constAsciiStr ) )
#define CUniString( constAsciiStr ) UniString( RTL_CONSTASCII_USTRINGPARAM ( constAsciiStr ) )

#define StartKenn           CUniString("%")
#define EndKenn             CUniString("%")
#define UIdKenn             ( StartKenn.AppendAscii("UId") )
#define MethodKenn          ( StartKenn.AppendAscii("Method") )
#define TypeKenn            ( StartKenn.AppendAscii("RType") )
#define SlotKenn            ( StartKenn.AppendAscii("SlotId") )
#define RcKenn              ( StartKenn.AppendAscii("RCommand") )
#define TabKenn             ( StartKenn.AppendAscii("Tab") )
#define MakeStringParam(Type,aText) ( Type.AppendAscii("=").Append( aText ).Append( EndKenn ) )
#define MakeStringNumber(Type,nNumber)  MakeStringParam (Type, UniString::CreateFromInt32(nNumber))
#define UIdString(aID) MakeStringParam(UIdKenn,aID.GetText())
#define MethodString(nNumber) MakeStringNumber(MethodKenn,nNumber)
#define TypeString(nNumber) MakeStringNumber(TypeKenn,nNumber)
#define SlotString(nNumber) MakeStringNumber(SlotKenn,nNumber)
#define RcString(nNumber) MakeStringNumber(RcKenn,nNumber)
#define TabString(nNumber) MakeStringNumber(TabKenn,nNumber)

#define ResKenn             ( StartKenn.AppendAscii("ResId") )
#define BaseArgKenn         ( StartKenn.AppendAscii("Arg") )
#define ArgKenn(nNumber)    ( BaseArgKenn.Append( UniString::CreateFromInt32(nNumber) ) )
#define ResString(nNumber) MakeStringNumber(ResKenn,nNumber)
#define ArgString(nNumber, aText) MakeStringParam(ArgKenn(nNumber),aText)

UniString GEN_RES_STR0( ULONG nResId );
UniString GEN_RES_STR1( ULONG nResId, const String &Text1 );
UniString GEN_RES_STR2( ULONG nResId, const String &Text1, const String &Text2 );
UniString GEN_RES_STR3( ULONG nResId, const String &Text1, const String &Text2, const String &Text3 );

#define GEN_RES_STR1c( nResId, Text1 ) GEN_RES_STR1( nResId, CUniString(Text1) )
#define GEN_RES_STR2c2( nResId, Text1, Text2 ) GEN_RES_STR2( nResId, Text1, CUniString(Text2) )
#define GEN_RES_STR3c3( nResId, Text1, Text2, Text3 ) GEN_RES_STR3( nResId, Text1, Text2, CUniString(Text3) )

#define IMPL_GEN_RES_STR \
UniString GEN_RES_STR0( ULONG nResId ) { return ResString( nResId ); } \
UniString GEN_RES_STR1( ULONG nResId, const UniString &Text1 ) { return GEN_RES_STR0( nResId ).Append( ArgString( 1, Text1 ) ); } \
UniString GEN_RES_STR2( ULONG nResId, const UniString &Text1, const UniString &Text2 ) { return GEN_RES_STR1( nResId, Text1 ).Append( ArgString( 2, Text2 ) ); } \
UniString GEN_RES_STR3( ULONG nResId, const UniString &Text1, const UniString &Text2, const UniString &Text3 ) { return GEN_RES_STR2( nResId, Text1, Text2 ).Append( ArgString( 3, Text3 ) );}

#endif

