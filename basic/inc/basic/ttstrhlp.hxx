/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _BASIC_TTSTRHLP_HXX
#define _BASIC_TTSTRHLP_HXX

#include <tools/string.hxx>

#define CByteString( constAsciiStr ) ByteString( RTL_CONSTASCII_STRINGPARAM ( constAsciiStr ) )
#define CUniString( constAsciiStr ) UniString( RTL_CONSTASCII_USTRINGPARAM ( constAsciiStr ) )
#define Str2Id( Str ) rtl::OUStringToOString( Str, RTL_TEXTENCODING_ASCII_US )
#define Id2Str( Id ) String( rtl::OStringToOUString( Id, RTL_TEXTENCODING_ASCII_US ) )

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
#define UIdString(aID) MakeStringParam(UIdKenn,String(rtl::OStringToOUString( aID, RTL_TEXTENCODING_ASCII_US )))
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

UniString GEN_RES_STR0( sal_uIntPtr nResId );
UniString GEN_RES_STR1( sal_uIntPtr nResId, const String &Text1 );
UniString GEN_RES_STR2( sal_uIntPtr nResId, const String &Text1, const String &Text2 );
UniString GEN_RES_STR3( sal_uIntPtr nResId, const String &Text1, const String &Text2, const String &Text3 );

#define GEN_RES_STR1c( nResId, Text1 ) GEN_RES_STR1( nResId, CUniString(Text1) )
#define GEN_RES_STR2c2( nResId, Text1, Text2 ) GEN_RES_STR2( nResId, Text1, CUniString(Text2) )
#define GEN_RES_STR3c3( nResId, Text1, Text2, Text3 ) GEN_RES_STR3( nResId, Text1, Text2, CUniString(Text3) )

#define IMPL_GEN_RES_STR \
UniString GEN_RES_STR0( sal_uIntPtr nResId ) { return ResString( nResId ); } \
UniString GEN_RES_STR1( sal_uIntPtr nResId, const UniString &Text1 ) { return GEN_RES_STR0( nResId ).Append( ArgString( 1, Text1 ) ); } \
UniString GEN_RES_STR2( sal_uIntPtr nResId, const UniString &Text1, const UniString &Text2 ) { return GEN_RES_STR1( nResId, Text1 ).Append( ArgString( 2, Text2 ) ); } \
UniString GEN_RES_STR3( sal_uIntPtr nResId, const UniString &Text1, const UniString &Text2, const UniString &Text3 ) { return GEN_RES_STR2( nResId, Text1, Text2 ).Append( ArgString( 3, Text3 ) );}

#endif

