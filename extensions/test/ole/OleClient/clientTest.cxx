/*************************************************************************
 *
 *  $RCSfile: clientTest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:55 $
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


CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace cppu;
using namespace rtl;
HRESULT doTest();
HRESULT doTest2( Reference<XInvocation> &);
Reference<XInvocation> getComObject(OUString& );

HRESULT InitializeParameter();
void printResultVariantArray( VARIANT & var);
void printVariant( VARIANT & var);


Reference< XMultiServiceFactory > objectFactory;


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


Reference<XInvocation> getComObject( OUString progId)
{
    HRESULT hr= S_OK;
    Reference< XInvocation > ret;
//  Reference<XMultiServiceFactory> fac;
    if(  ! objectFactory.is())
    {
        Reference< XMultiServiceFactory > mgr= createRegistryServiceFactory( OUString(L"applicat.rdb"));
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
//      CComPtr<IUnknown> spUnkMgr;
//  Reference< XInvocation > ret;
//  Reference< XMultiServiceFactory > mgr= createRegistryServiceFactory( OUString(L"applicat.rdb"));
//  Reference< XInterface > xInt= mgr->createInstance(OUString(L"com.sun.star.bridge.OleObjectFactory"));
//  Reference< XMultiServiceFactory > fac( xInt, UNO_QUERY);
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
    //  Sequences
    //###################################################################################

    seqIndices.realloc( 0);
    seqOut.realloc(0);
    sal_Int32 arLong[]={ 1,2,3};
    Any seqLongAny;
    seqLongAny<<= Sequence<sal_Int32>( arLong, 3);
    inv->invoke( OUString(L"inSequenceLong"),Sequence< Any > ( &seqLongAny, 1), seqIndices, seqOut);

    return hr;
}
