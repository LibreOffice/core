/*************************************************************************
 *
 *  $RCSfile: clientTest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-13 15:29:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <windows.h>
#include <comdef.h>
#include <tchar.h>
#include <atlbase.h>
extern CComModule _Module;
#include<atlcom.h>
#include<atlimpl.cpp>

#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <oletest/XCallback.hpp>
#include <rtl/process.h>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/string.h>
#pragma hdrstop

#include "axhost.hxx"

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;
using namespace cppu;
using namespace rtl;
HRESULT doTest();
HRESULT doTest2( Reference<XInvocation> &);
Reference<XInvocation> getComObject(OUString& );

HRESULT InitializeParameter();
void printResultVariantArray( VARIANT & var);
void printVariant( VARIANT & var);
void printSequence( Sequence<Any>& val);


Reference< XMultiServiceFactory > objectFactory;// OleObjectFactory;


int __cdecl _tmain( int argc, _TCHAR * argv[] )
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL)))
    {
        _tprintf(_T("CoInitialize failed \n"));
        return -1;
    }


    _Module.Init( ObjectMap, GetModuleHandle( NULL));

    if( FAILED(hr=doTest()))
    {
        _com_error err( hr);
        const TCHAR * errMsg= err.ErrorMessage();
        MessageBox( NULL, errMsg, "Test failed", MB_ICONERROR);
    }


    _Module.Term();
    CoUninitialize();
    return 0;
}

Reference<XMultiServiceFactory> getMultiServiceFactory()
{
    static Reference< XMultiServiceFactory > factory;
    if( ! objectFactory.is() )
    {
        Reference<XInterface> xint= createRegistryServiceFactory( OUString(L"applicat.rdb"));
        factory= Reference<XMultiServiceFactory>( xint, UNO_QUERY);
    }
    return factory;
}

Reference<XInvocation> getComObject( OUString progId)
{
    HRESULT hr= S_OK;
    Reference< XInvocation > ret;
//  Reference<XMultiServiceFactory> fac;
    if(  ! objectFactory.is())
    {   Reference<XMultiServiceFactory> mgr= getMultiServiceFactory();
        Reference< XInterface > xInt= mgr->createInstance( OUString(L"com.sun.star.bridge.OleObjectFactory"));
        objectFactory= Reference<XMultiServiceFactory>::query(  xInt);
    }

    if( objectFactory.is())
    {
        Reference<XInterface> xIntAx= objectFactory->createInstance( progId.getStr());
        if( xIntAx.is() )
        {
            Reference< XInvocation > xInv( xIntAx, UNO_QUERY);
            ret= xInv;
        }
    }
    return ret;
}

Reference<XInvocation> convertComObject( IUnknown* pUnk)
{
    Reference< XMultiServiceFactory > mgr= getMultiServiceFactory();
    Reference< XInterface > xIntSupplier= mgr->createInstance(OUString(L"com.sun.star.bridge.OleBridgeSupplier2"));
    Reference< XBridgeSupplier2 > xSuppl( xIntSupplier, UNO_QUERY);

    Any any;
    CComVariant var( pUnk);
    any <<= ( sal_uInt32)&var;
    sal_uInt8 arId[16];
    rtl_getGlobalProcessId( arId);
    Any target= xSuppl->createBridge( any, Sequence<sal_Int8>( (sal_Int8*)arId, 16), OLE, UNO );

    Reference<XInvocation> ret;
    target>>= ret;
    return ret;
}

HRESULT doTest()
{
    HRESULT hr= S_OK;
    USES_CONVERSION;
    Reference<XInvocation> inv= getComObject( L"AxTestComponents.Basic");
    Sequence< sal_Int16> seqIndices;
    Sequence<Any> seqOut;

    Any aAny;
    Any anyOut;
    char buff[256];
    Any seqAny;

    //###################################################################################
    //  in parameter
    //###################################################################################
//  aAny <<= ( sal_Int8) 127;
//  inv->invoke( OUString(L"inByte"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  aAny <<= ( sal_Int16) 0xffff;
//  inv->invoke( OUString(L"inShort"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

//  aAny <<= ( sal_Int32) 1234567;
//  inv->invoke( OUString(L"inLong"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  aAny <<= OUString(L" this is clientTest.exe");
//  inv->invoke( OUString(L"inString"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  aAny <<= ( float) 3.14;
//  inv->invoke( OUString(L"inFloat"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  aAny <<= ( double) 3.145;
//  inv->invoke( OUString(L"inDouble"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  aAny <<= OUString( L" A string in an any");
//  inv->invoke( OUString(L"inVariant"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  OUString arStr[]= {L"string0", L"string1", L"string2"};
//  Sequence<OUString> seq( arStr, 3);
//  Any arAny[1];
//  arAny[0] <<= seq;
//  inv->invoke( OUString(L"inArray"), Sequence< Any > ( arAny, 1), seqIndices, seqOut);
//
//  // same again but this time Sequence<Any> with the Anys containing strings.
//  OUString arStr1[]= {L"string0", L"string1", L"string2"};
//  Any arAnyStr1[3];
//  arAnyStr1[0]<<= arStr1[0];
//  arAnyStr1[1]<<= arStr1[1];
//  arAnyStr1[2]<<= arStr1[2];
//  Sequence<Any> seq_1( arAnyStr1, 3);
//  Any arAny_1[1];
//  arAny_1[0] <<= seq_1;
//  inv->invoke( OUString(L"inArray"), Sequence< Any > ( arAny_1, 1), seqIndices, seqOut);

//  Reference < XInvocation > inv2= getComObject(L"AxTestComponents.Basic");
//  Any anyVal;
//  anyVal <<= OUString(L"this is the value of prpString");
//  inv2->setValue( OUString(L"prpString"), anyVal);
//  aAny <<= inv2;
//  inv->invoke( OUString(L"inObject"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  //###################################################################################
//  //  out parameter
//  //###################################################################################
    // outByte
//  aAny= Any();
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outByte"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " out Byte: %d", *( char*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // outShort
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outShort"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " out Short: %d", *( sal_Int16*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // outLong
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outLong"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " out Long: %d", *( sal_Int32*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // outString
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outString"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " out String: %S", (*( OUString*)anyOut.getValue()).getStr());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // outFloat
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outFloat"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " out float: %f", *( float*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // outDouble
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outDouble"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " out double: %g", *( double*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);

    // outVariant
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outVariant"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  if( anyOut.getValueTypeClass() == TypeClass_STRING)
//  {
//      OUString s;
//      anyOut >>= s;
//      sprintf( buff, " out string ( variant): %S", s.getStr());
//      MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//  }
//
//  // outArray
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outArray"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  Sequence<Any> seqOutValue;
//  anyOut >>= seqOutValue;
//
//  // we assume that the Sequence contains Anys of strings
//  OUString usMessage;
//  for( int i=0; i < seqOutValue.getLength(); i++)
//  {
//      OUString stemp;
//      if( seqOutValue[i] >>= stemp)
//      {
//          usMessage += OUString(L"\n");
//          usMessage += stemp;
//      }
//  }
//  MessageBox( NULL, W2T( usMessage.getStr()), _T("Test Program"), MB_OK);
//
//  // outObject
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  inv->invoke(  OUString(L"outObject"), Sequence< Any > ( &aAny, 1 ), seqIndices, seqOut);
//  Reference<XInvocation> invOut;
//  if( seqOut[0]>>=invOut)
//  {
//      Any val=    invOut->getValue( L"prpString");
//
//      if( val.getValueTypeClass() == TypeClass_STRING)
//      {
//          OUString s;
//          val>>=s;
//          MessageBox( NULL,W2T( s.getStr()), _T("Test Program"), MB_OK);
//      }
//  }

//  //###################################################################################
//  //  in/out parameter
//  //###################################################################################
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= ( sal_Int8) 127;
//  inv->invoke( OUString(L"inoutByte"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " in out Byte: %d", *( char*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);

    // in out short
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= ( sal_Int16) 1111;
//  inv->invoke( OUString(L"inoutShort"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " in out Short: %d", *( sal_Int16*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  //in out long
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= ( sal_Int32) 111111;
//  inv->invoke( OUString(L"inoutLong"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, "inout Long: %d", *( sal_Int32*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  //in out string
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= OUString(L" this is clientTest.exe");
//  inv->invoke( OUString(L"inoutString"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " inout String: %S", (*( OUString*)anyOut.getValue()).getStr());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  //in out float
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= ( float) 3.14;
//  inv->invoke( OUString(L"inoutFloat"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " inout float: %f", *( float*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // in out double
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= ( double) 3.145;
//  inv->invoke( OUString(L"inoutDouble"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  sprintf( buff, " inout double: %g", *( double*)anyOut.getValue());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  // in out VARIANT
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  aAny <<= OUString( L" A string in an any");
//  inv->invoke( OUString(L"inoutVariant"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  anyOut <<= seqOut[0];
//  if( anyOut.getValueTypeClass() == TypeClass_STRING)
//  {
//      OUString s;
//      anyOut >>= s;
//      sprintf( buff, " in out string ( variant): %S", s.getStr());
//      MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//  }

//  // in out Array
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  OUString arStr2[]= {L"string0", L"string1", L"string2"};
//  Sequence<OUString> seq2( arStr2, 3);
//  Any arAny2[1];
//  arAny2[0] <<= seq2;
//  inv->invoke( OUString(L"inoutArray"), Sequence< Any > ( arAny2, 1), seqIndices, seqOut);
//  Sequence<Any> seqOutValue2;
//  seqOut[0]>>= seqOutValue2;
//
//  // we assume that the Sequence contains Anys of strings
//  OUString usMessage2;
//  for(int  i2=0; i2 < seqOutValue2.getLength(); i2++)
//  {
//      OUString stemp;
//      if( seqOutValue2[i2] >>= stemp)
//      {
//          usMessage2 += OUString(L"\n");
//          usMessage2 += stemp;
//      }
//  }
//  MessageBox( NULL, W2T( usMessage2.getStr()), _T("Test Program"), MB_OK);

//
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  Reference < XInvocation > inv3= getComObject(L"AxTestComponents.Basic");
//  Any anyVal2;
//  anyVal2 <<= OUString(L"this is the value of prpString");
//  inv3->setValue( OUString(L"prpString"), anyVal2);
//  aAny <<= inv3;
//  inv->invoke( OUString(L"inoutObject"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//  Reference<XInvocation> invOut2;
//  if( seqOut[0]>>=invOut2)
//  {
//      Any val=    invOut2->getValue( L"prpString");
//
//      if( val.getValueTypeClass() == TypeClass_STRING)
//      {
//          OUString s;
//          val>>=s;
//          MessageBox( NULL,W2T( s.getStr()), _T("Test Program"), MB_OK);
//      }
//  }

    //###################################################################################
    //  mixed parameter
    //###################################################################################
//  // mixed1
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  Sequence< Any > params(12);
//  sal_Int8 aByte=111;
//  OUString aString(L" a string in a VARIANT");
//  Any param[12];
//  param[0] <<= aByte;
//  param[2] <<= aByte; //in out
//  param[3] <<= aString; // in String
//  param[5] <<= aString; // in out string
//
//  OUString arStr3[]= {L"string0", L"string1", L"string2"};
//  Sequence<OUString> seq3( arStr3,3);
//  param[6] <<= seq3;  // in Array
//  param[8] <<= seq3;  // in ou Array
//
//  Reference < XInvocation > inv4= getComObject(L"AxTestComponents.Basic");
//  Any anyVal3;
//  anyVal3 <<= OUString(L"this is the value of prpString");
//  inv4->setValue( OUString(L"prpString"), anyVal3);
//  param[9] <<= inv4; // in dispatch
//  param[11] <<= inv4;
//  inv->invoke( OUString(L"mixed1"), Sequence< Any > ( param, 12),seqIndices, seqOut);
//
//  for( int i3=0; i3<seqOut.getLength();i3++)
//  {
//      Any any;
//      any <<= seqOut[i3];
//
//  }
//  sal_Int8 outChar= *( char*)seqOut[0].getValue();
//  sal_Int8 inoutChar= *( char*)seqOut[1].getValue();
//  sprintf( buff, " out Byte: %d \n in out Byte %d", outChar, inoutChar );
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  OUString outString( *(OUString*)seqOut[2].getValue());
//  OUString inoutString( *(OUString*)seqOut[3].getValue());
//  sprintf( buff, "out string: %S \n in out string: %S", outString.getStr(), inoutString.getStr());
//  MessageBox( NULL, A2T( buff), _T("Test Program"), MB_OK);
//
//  Sequence< Any > outSeq;
//  seqOut[4] >>= outSeq;
//  OUString usMessage3;
//  for(int  i4=0; i4 < outSeq.getLength(); i4++)
//  {
//      OUString stemp;
//      if( outSeq[i4] >>= stemp)
//      {
//          usMessage3 += OUString(L"\n");
//          usMessage3 += stemp;
//      }
//  }
//  MessageBox( NULL, W2T( usMessage3.getStr()), _T("Test Program. out Sequence"), MB_OK);
//
//  seqOut[5] >>= outSeq;
//  OUString usMessage4;
//  for(int  i5=0; i5 < outSeq.getLength(); i5++)
//  {
//      OUString stemp;
//      if( outSeq[i5] >>= stemp)
//      {
//          usMessage4 += OUString(L"\n");
//          usMessage4 += stemp;
//      }
//  }
//  MessageBox( NULL, W2T( usMessage3.getStr()), _T("Test Program. in out Sequence"), MB_OK);
//
//  Reference<XInvocation> invOut3;
//  seqOut[6] >>= invOut3;
//  if( seqOut[0]>>=invOut3)
//  {
//      Any val=    invOut3->getValue( L"prpString");
//
//      if( val.getValueTypeClass() == TypeClass_STRING)
//      {
//          OUString s;
//          val>>=s;
//          MessageBox( NULL,W2T( s.getStr()), _T("Test Program, out object"), MB_OK);
//      }
//  }
//
//  Reference<XInvocation> invOut4;
//  seqOut[6] >>= invOut4;
//  if( seqOut[0]>>=invOut4)
//  {
//      Any val=    invOut4->getValue( L"prpString");
//
//      if( val.getValueTypeClass() == TypeClass_STRING)
//      {
//          OUString s;
//          val>>=s;
//          MessageBox( NULL,W2T( s.getStr()), _T("Test Program, in out object"), MB_OK);
//      }
//  }

    //###################################################################################
    //  in Sequences
    //###################################################################################
    // inSequenceLong
//  seqIndices.realloc( 0);
//  seqOut.realloc(0);
//  sal_Int32 arLong[]={ 1,2,3};
//  Sequence< sal_Int32 > seqLong( arLong, 3);

    //  seqLongAny <<= seqLong; // 1 dimension
    //      seqLongAny<<= seq; // 2 dimensions
    //  seqLongAny<<= seq1;
    //  seqLongAny<<= Sequence<sal_Int32>( arLong, 3);

//  seqAny<<= seqLong;
//  inv->invoke( OUString(L"inSequenceLong"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    // inSequenceShort
//  sal_Int16 arShort[]={1,2,3};
//  Sequence<sal_Int16> seqShort(arShort, 3);
//  seqAny<<= seqShort;
//  inv->invoke( OUString(L"inSequenceShort"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    // inSequenceByte
//  sal_Int8 arByte[]={1,2,3};
//  Sequence<sal_Int8> seqByte(arByte, 3);
//  seqAny<<= seqByte;
//  inv->invoke( OUString(L"inSequenceByte"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    // inSequenceString
//  OUString arString[]={L"string one", L"string two", L"string three"};
//  Sequence<OUString> seqString( arString, 3);
//  seqAny<<= seqString;
//  inv->invoke( OUString(L"inSequenceString"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    // inSequenceFloat
//  float arFloat[]={3.14, 31.4, 314.};
//  Sequence<float> seqFloat( arFloat, 3);
//  seqAny<<=seqFloat;
//  inv->invoke( OUString(L"inSequenceFloat"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    // inSequenceDouble
//  double arDouble[]={3.14, 31.4, 314.};
//  Sequence<double> seqDouble( arDouble, 3);
//  seqAny<<=seqDouble;
//  inv->invoke( OUString(L"inSequenceDouble"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);


    // inSequenceObject
//  Any anyVala;
//  OUString sVal;
//  Sequence<Reference<XInvocation> > seqObj(3);
//  seqObj[0]=  getComObject(L"AxTestComponents.Basic");
//  sVal= L"this is the property value of prpString (1)";
//  anyVala<<= sVal;
//  seqObj[0]->setValue( OUString( L"prpString"), anyVala);
//
//  seqObj[1]=  getComObject(L"AxTestComponents.Basic");
//  sVal= L"this is the property valuef of prpString (2)";
//  anyVala<<= sVal;
//  seqObj[1]->setValue( OUString( L"prpString"), anyVala);
//
//  seqObj[2]=  getComObject(L"AxTestComponents.Basic");
//  sVal= L"this is the property value of prpString (3)";
//  anyVala<<= sVal;
//  seqObj[2]->setValue( OUString( L"prpString"), anyVala);
//
//  seqAny<<=seqObj;
//  inv->invoke( OUString(L"inSequenceObject"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    //###################################################################################
    //  out Sequences
    //###################################################################################

    // outSequenceByte

//  inv->invoke( OUString(L"outSequenceByte"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());
//
//
//  // outSequenceShort
//  inv->invoke( OUString(L"outSequenceShort"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence(*(Sequence<Any>*)seqOut[0].getValue());
//
//
//  // outSequenceLong
//  inv->invoke( OUString(L"outSequenceLong"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence(*(Sequence<Any>*)seqOut[0].getValue());
//
//  // outSequenceString
//  inv->invoke( OUString(L"outSequenceString"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence(*(Sequence<Any>*)seqOut[0].getValue());
//
//  // outSequenceFloat
//  inv->invoke( OUString(L"outSequenceFloat"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence(  *(Sequence<Any>*)seqOut[0].getValue());
//
//
//  //outSequenceDouble
//  inv->invoke( OUString(L"outSequenceDouble"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence(*(Sequence<Any>*)seqOut[0].getValue());

    //outSequenceObject
//  inv->invoke( OUString(L"outSequenceObject"),Sequence< Any > (), seqIndices, seqOut);
//  printSequence(  *(Sequence<Any>*)seqOut[0].getValue());

    // outSequenceVariant (see outArray)
    //###################################################################################
    //  in out Sequences
    //###################################################################################
    // inoutSequenceByte
//  sal_Int8 arByte1[]={1,2,3};
//  Sequence<sal_Int8> seqByteIO(arByte1, 3);
//  seqAny<<= seqByteIO;
//  inv->invoke( OUString(L"inoutSequenceByte"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());
//
//  // inoutSequenceShort
//  sal_Int16 arShort1[]={1,2,3};
//  Sequence<sal_Int16> seqShortIO(arShort1, 3);
//  seqAny<<= seqShortIO;
//  inv->invoke( OUString(L"inoutSequenceShort"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());
//
//
//  // inoutSequenceLong
//  sal_Int32 arLong1[]={ 1,2,3};
//  Sequence< sal_Int32 > seqLongIO( arLong1, 3);
//  seqAny<<= seqLongIO;
//  inv->invoke( OUString(L"inoutSequenceLong"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());
//
//  // inoutSequenceString
//  OUString arString1[]={L"string one", L"string two", L"string three"};
//  Sequence<OUString> seqStringIO( arString1, 3);
//  seqAny<<= seqStringIO;
//  inv->invoke( OUString(L"inoutSequenceString"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());
//
//  // inoutSequenceFloat
//  float arFloat1[]={3.14, 31.4, 314.};
//  Sequence<float> seqFloatIO( arFloat1, 3);
//  seqAny<<=seqFloatIO;
//  inv->invoke( OUString(L"inoutSequenceFloat"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());
//
//  // inoutSequenceDouble
//  double arDouble1[]={3.14, 31.4, 314.};
//  Sequence<double> seqDoubleIO( arDouble1, 3);
//  seqAny<<=seqDoubleIO;
//  inv->invoke( OUString(L"inoutSequenceDouble"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());


    // inoutSequenceObject
//  Any anyValb;
//  OUString sVala;
//  Sequence<Reference<XInvocation> > seqObjIO(3);
//  seqObjIO[0]=    getComObject(L"AxTestComponents.Basic");
//  sVala= L"this is the property value of prpString (1)";
//  anyValb<<= sVala;
//  seqObjIO[0]->setValue( OUString( L"prpString"), anyValb);
//
//  seqObjIO[1]=    getComObject(L"AxTestComponents.Basic");
//  sVala= L"this is the property valuef of prpString (2)";
//  anyValb<<= sVala;
//  seqObjIO[1]->setValue( OUString( L"prpString"), anyValb);
//
//  seqObjIO[2]=    getComObject(L"AxTestComponents.Basic");
//  sVala= L"this is the property value of prpString (3)";
//  anyValb<<= sVala;
//  seqObjIO[2]->setValue( OUString( L"prpString"), anyValb);
//
//  seqAny<<=seqObjIO;
//  inv->invoke( OUString(L"inoutSequenceObject"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//  printSequence( *(Sequence<Any>*)seqOut[0].getValue());

    //###################################################################################
    //  in multi Sequences
    //###################################################################################
    // inMulDimArrayLong
//  sal_Int32 arLongi[]={1,2,3};
//  sal_Int32 arLongi2[]={4,5,6,7};
//  sal_Int32 arLongi3[]={8,9,10,11,12};
//
//  Sequence<sal_Int32> seqLongi1( arLongi, 3);
//  Sequence<sal_Int32> seqLongi2( arLongi2, 4);
//  Sequence<sal_Int32> seqLongi3( arLongi3, 5);
//
//  Sequence< Sequence< sal_Int32 > > seq2i(3);
//  seq2i[0]= seqLongi1;
//  seq2i[1]= seqLongi2;
//  seq2i[2]= seqLongi3;
//  seqAny<<= seq2i;
//  // dimension length 3,5
//  inv->invoke( OUString(L"inMulDimArrayLong"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

//  //inMulDimArrayVariant
//  inv->invoke( OUString(L"inMulDimArrayVariant"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//
//  //inMulDimArrayLong2
//  sal_Int32 arLongii1[]={1,2,3};
//  sal_Int32 arLongii2[]={4,5,6,7};
//  sal_Int32 arLongii3[]={8,9,10,11,12};
//  sal_Int32 arLongii4[]={13,14,15,16};
//  sal_Int32 arLongii5[]={17,18,19};
//
//  Sequence<sal_Int32> seqLongii1( arLongii1, 3);
//  Sequence<sal_Int32> seqLongii2( arLongii2, 4);
//  Sequence<sal_Int32> seqLongii3( arLongii3, 5);
//  Sequence<sal_Int32> seqLongii4( arLongii4, 4);
//  Sequence<sal_Int32> seqLongii5( arLongii5, 3);
//
//  Sequence< Sequence< sal_Int32 > > seq2ii(3);
//  Sequence< Sequence< sal_Int32> > seq2ii2(2);
//  seq2ii[0]= seqLongii1;
//  seq2ii[1]= seqLongii2;
//  seq2ii[2]= seqLongii3;
//
//  seq2ii2[0]= seqLongii4;
//  seq2ii2[1]= seqLongii5;
//
//  Sequence< Sequence< Sequence< sal_Int32> > >  seq3ii(2);
//  seq3ii[0]=seq2ii;
//  seq3ii[1]=seq2ii2;
//  seqAny<<= seq3ii;
//  inv->invoke( OUString(L"inMulDimArrayLong2"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//
//  // inMulDimArrayByte2
//  sal_Int8 arByteii1[]={1,2,3};
//  sal_Int8 arByteii2[]={4,5,6,7};
//  sal_Int8 arByteii3[]={8,9,10,11,12};
//  sal_Int8 arByteii4[]={13,14,15,16};
//  sal_Int8 arByteii5[]={17,18,19};
//
//  Sequence<sal_Int8> seqByteii1( arByteii1, 3);
//  Sequence<sal_Int8> seqByteii2( arByteii2, 4);
//  Sequence<sal_Int8> seqByteii3( arByteii3, 5);
//  Sequence<sal_Int8> seqByteii4( arByteii4, 4);
//  Sequence<sal_Int8> seqByteii5( arByteii5, 3);
//
//  Sequence< Sequence< sal_Int8 > > seq2Byteii(3);
//  Sequence< Sequence< sal_Int8> > seq2Byteii2(2);
//  seq2Byteii[0]= seqByteii1;
//  seq2Byteii[1]= seqByteii2;
//  seq2Byteii[2]= seqByteii3;
//
//  seq2Byteii2[0]= seqByteii4;
//  seq2Byteii2[1]= seqByteii5;
//
//  Sequence< Sequence< Sequence< sal_Int8> > >  seq3Byteii(2);
//  seq3Byteii[0]=seq2Byteii;
//  seq3Byteii[1]=seq2Byteii2;
//  seqAny<<= seq3Byteii;
//  inv->invoke( OUString(L"inMulDimArrayByte2"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//
//
//  // prpByte
//  Any prpAny;
//  sal_Int8 aByte1= 111;
//  prpAny<<= aByte1;
//  inv->setValue(OUString(L"prpByte"), prpAny);
//  prpAny= Any();
//  aByte1=0;
//  prpAny= inv->getValue(OUString(L"prpByte"));
//  prpAny>>= aByte1;
    // prpShort

    // prpLong
    //###################################################################################
    //###################################################################################
    //###################################################################################
    //  Tests with a MFC ActiveX control, ( pure dispinterface)
    //###################################################################################

    HostWin* pWin= new HostWin( L"MFCCONTROL.MfcControlCtrl.1");
    CComPtr<IUnknown> spUnk= pWin->GetHostedControl();
    inv= convertComObject( spUnk.p);
    //###################################################################################
    //  in parameter
    //###################################################################################
    // unsigned char is not supported by MFC
    //  aAny <<= ( sal_Int8) 127;
    //  inv->invoke( OUString(L"inByte"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    aAny <<= ( sal_Int16) 0xffff;
    aAny= inv->invoke( OUString(L"inShort"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

//
    aAny <<= ( sal_Int32) 1234567;
    aAny=inv->invoke( OUString(L"inLong"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    sal_Int32 retLong= *(sal_Int32*)aAny.getValue();

    OUString str_1(L" this is clientTest.exe");
    aAny <<= str_1;
    aAny=inv->invoke( OUString(L"inString"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    aAny>>= str_1;

    aAny <<= ( float) 3.14;
    aAny=inv->invoke( OUString(L"inFloat"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    aAny <<= ( double) 3.145;
    aAny=inv->invoke( OUString(L"inDouble"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    aAny <<= OUString( L" A string in an any");
    aAny=inv->invoke( OUString(L"inVariant"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    // Tests with Sequences later.
    //  OUString arStr4[]= {L"string0", L"string1", L"string2"};
    //  Sequence<OUString> seqStr4( arStr4, 3);
    //  aAny <<= seqStr4;
    //  inv->invoke( OUString(L"inArray"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);


    Reference < XInvocation > inv5= getComObject(L"AxTestComponents.Basic");
    Any anyVal4;
    anyVal4 <<= OUString(L"this is the value of prpString");
    inv5->setValue( OUString(L"prpString"), anyVal4);
    aAny <<= inv5;
    aAny=inv->invoke( OUString(L"inObject"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    //###################################################################################
    //  in parameter
    //###################################################################################


    delete pWin;
    return hr;

}


void printSequence( Sequence<Any>& val)
{

//  typelib_TypeDescription* desc;
//  val.getValueTypeDescription( &desc);
//  typelib_typedescription_release( desc);

    USES_CONVERSION;
    char buff[1024];
    buff[0]=0;
    char tmpBuf[1024];
    tmpBuf[0]=0;
    sal_Int32 i;

    for( i=0; i< val.getLength(); i++)
    {
        Any& elem= val[i];
        switch ( elem.getValueTypeClass())
        {
        case TypeClass_BYTE:
             sprintf( tmpBuf, "sal_Int8 %d \n", *(sal_Int8*)elem.getValue());
             break;
        case TypeClass_SHORT:
             sprintf( tmpBuf, "sal_Int16 %d \n", *(sal_Int16*)elem.getValue());
             break;
        case TypeClass_LONG:
             sprintf( tmpBuf, "sal_Int32 %d \n", *(sal_Int32*)elem.getValue());
             break;
        case TypeClass_DOUBLE:
             sprintf( tmpBuf, "double %f \n", *(double*)elem.getValue());
             break;
        case TypeClass_FLOAT:
             sprintf( tmpBuf, "float %f \n", *(float*)elem.getValue());
             break;
        case TypeClass_STRING:
             sprintf( tmpBuf, "%S \n", (*(OUString*)elem.getValue()).getStr());
             break;
        case TypeClass_INTERFACE:
            {
            // we assume that the interface is XInvocation of a AxTestControls.Basic component.
            Reference<XInvocation> inv;
            elem>>= inv;
            if( inv.is())
            {
                Any prpVal= inv->getValue( OUString( L"prpString"));
                sprintf( tmpBuf, "Property prpString: %S \n", (*(OUString*)prpVal.getValue()).getStr());
            }
            break;
            }
        default:break;
        }
        strcat( buff, tmpBuf);

    }

    MessageBox( NULL, A2T(buff), _T("clientTest: printing Sequence elements"), MB_OK);
}

//VARIANT VT_UI1

