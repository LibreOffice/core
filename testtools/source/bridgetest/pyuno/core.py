#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import pyuno
import uno
import unittest
import exceptions
import types

def suite(ctx):
    suite = unittest.TestSuite()
    suite.addTest(TestCase("testErrors",ctx))
    suite.addTest(TestCase("testBaseTypes",ctx))
    suite.addTest(TestCase("testOutparam",ctx))
    suite.addTest(TestCase("testStruct",ctx))
    suite.addTest(TestCase("testType",ctx))
    suite.addTest(TestCase("testEnum",ctx))
    suite.addTest(TestCase("testBool",ctx))
    suite.addTest(TestCase("testChar",ctx))
    suite.addTest(TestCase("testUnicode",ctx))
    suite.addTest(TestCase("testConstant",ctx))
    suite.addTest(TestCase("testExceptions",ctx))
    suite.addTest(TestCase("testInterface",ctx))
    suite.addTest(TestCase("testByteSequence",ctx))
    suite.addTest(TestCase("testInvoke",ctx))
    return suite

def equalsEps( a,b,eps ):
    if a - eps <= b and a+eps >= b:
       return 1
    return 0

def assign( rData, bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper,\
              nUHyper, fFloat, fDouble, eEnum, rStr, xTest, rAny  ):
    rData.Bool = bBool
    rData.Char = cChar
    rData.Byte = nByte
    rData.Short = nShort
    rData.UShort = nUShort
    rData.Long = nLong
    rData.ULong = nULong
    rData.Hyper = nHyper
    rData.UHyper = nUHyper
    rData.Float = fFloat
    rData.Double = fDouble
    rData.Enum = eEnum
    rData.String = rStr
    rData.Interface = xTest
    rData.Any = rAny


class PythonTransporter:
    def __init__( self ):
        pass
    
    def transportAny( self, arg ):
        return arg
        
class TestCase( unittest.TestCase):

      def __init__(self,method,ctx):
          unittest.TestCase.__init__(self,method)
          self.ctx = ctx

      def setUp(self):
         # the testcomponent from the testtools project
         self.tobj = self.ctx.ServiceManager.createInstanceWithContext( 
                                'com.sun.star.test.bridge.CppTestObject' , self.ctx )

         self.tobj.Bool = 1
         self.tobj.Char = 'h'
         self.tobj.Byte = 43
         self.tobj.Short = -42
         self.tobj.UShort = 44
         self.tobj.Long = 42
         self.tobj.ULong = 41
         self.tobj.Hyper = 46
         self.tobj.UHyper = 47
         self.tobj.Float  = 4.3
         self.tobj.Double = 4.2
         self.tobj.Enum = 4
         self.tobj.String = "yabadabadoo"
         self.tobj.Interface = self.ctx
         self.tobj.Any = self.tobj.String
         mystruct = uno.createUnoStruct( "test.testtools.bridgetest.TestData" )
         assign( mystruct, 1, 'h', 43, -42,44,42,41,46,47,4.3,4.2,4,"yabadabadoo",self.ctx,"yabadabadoo")
         self.tobj.Struct = mystruct

         self.testElement = uno.createUnoStruct( "test.testtools.bridgetest.TestElement" )
         self.testElement.String = "foo"
         self.testElement2 = uno.createUnoStruct( "test.testtools.bridgetest.TestElement" )
         self.testElement2.String = "42"
         self.tobj.Sequence = (self.testElement,self.testElement2)

      def testBaseTypes(self):
          self.assertTrue( 42 == self.tobj.Long , "Long attribute" )
          self.assertTrue( 41 == self.tobj.ULong , "ULong attribute" )
          self.assertTrue( 43 == self.tobj.Byte , "Byte attribute" )
          self.assertTrue( 44 == self.tobj.UShort , "UShort attribute" )
          self.assertTrue( -42 == self.tobj.Short , "Short attribute" )
          self.assertTrue( 46 == self.tobj.Hyper , "Hyper attribute" )
          self.assertTrue( 47 == self.tobj.UHyper , "UHyper attribute" )
          self.assertTrue( self.tobj.Bool , "Bool attribute2" )
          self.assertTrue( "yabadabadoo" == self.tobj.String , "String attribute" )
          self.assertTrue( self.tobj.Sequence[0] == self.testElement , "Sequence test")
          self.assertTrue( self.tobj.Sequence[1] == self.testElement2 , "Sequence2 test")
          self.assertTrue( equalsEps( 4.3,self.tobj.Float,0.0001) , "float test" )
          self.assertTrue( 4.2 == self.tobj.Double , "double test" )
          self.assertTrue( self.ctx == self.tobj.Interface ,
                          "object identity test with C++ object" )
          self.assertTrue( not self.ctx == self.tobj , "object not identical test " )
          self.assertTrue( 42 == self.tobj.transportAny( 42 ), "transportAny long" )
          self.assertTrue( "woo, this is python" == self.tobj.transportAny( "woo, this is python" ), \
                  "string roundtrip via any test"  )
           
      def testEnum( self ):
          e1 = uno.Enum( "com.sun.star.uno.TypeClass" , "LONG" )
          e2 = uno.Enum( "com.sun.star.uno.TypeClass" , "LONG" )
          e3 = uno.Enum( "com.sun.star.uno.TypeClass" , "UNSIGNED_LONG" )
          e4 = uno.Enum( "test.testtools.bridgetest.TestEnum" , "TWO" )
          self.assertTrue( e1 == e2 , "equal enum test" )
          self.assertTrue( not (e1 == e3) , "different enums test" )
          self.assertTrue( self.tobj.transportAny( e3 ) == e3, "enum roundtrip test" )
          self.tobj.Enum = e4
          self.assertTrue( e4 == self.tobj.Enum , "enum assignment failed" )

      def testType(self ):
          t1 = uno.getTypeByName( "com.sun.star.lang.XComponent" )
          t2 = uno.getTypeByName( "com.sun.star.lang.XComponent" )
          t3 = uno.getTypeByName( "com.sun.star.lang.EventObject" )
          self.assertTrue( t1.typeClass == \
              uno.Enum( "com.sun.star.uno.TypeClass", "INTERFACE" ), "typeclass of type test" )
          self.assertTrue( t3.typeClass == \
              uno.Enum( "com.sun.star.uno.TypeClass", "STRUCT" ), "typeclass of type test")
          self.assertTrue( t1 == t2 , "equal type test" )
          self.assertTrue( t1 == t2 , "equal type test" )
          self.assertTrue( t1 == self.tobj.transportAny( t1 ), "type roundtrip test" )

      def testBool( self ):
          self.assertTrue( uno.Bool(1) , "uno.Bool true test" )
          self.assertTrue( not uno.Bool(0) , "uno.Bool false test" )
          self.assertTrue( uno.Bool( "true") , "uno.Bool true1 test" )
          self.assertTrue( not uno.Bool( "false") , "uno.Bool true1 test" )

          self.tobj.Bool = uno.Bool(1)
          self.assertTrue( self.tobj.Bool , "bool true attribute test" )
          self.tobj.Bool = uno.Bool(0)
          self.assertTrue( not self.tobj.Bool , "bool true attribute test" )

          # new boolean semantic 
          self.assertTrue( id( self.tobj.transportAny( True ) ) == id(True)  , "boolean preserve test")
          self.assertTrue( id( self.tobj.transportAny( False ) ) == id(False) , "boolean preserve test" )
          self.assertTrue( id( self.tobj.transportAny(1) ) != id( True ), "boolean preserve test" )
          self.assertTrue( id( self.tobj.transportAny(0) ) != id( False ), "boolean preserve test" )

      def testChar( self ):
          self.tobj.Char = uno.Char( u'h' )
          self.assertTrue( self.tobj.Char == uno.Char( u'h' ), "char type test" )
          self.assertTrue( isinstance( self.tobj.transportAny( uno.Char(u'h') ),uno.Char),"char preserve test" )

      def testStruct( self ):
          mystruct = uno.createUnoStruct( "test.testtools.bridgetest.TestData" )
          assign( mystruct, 1, 'h', 43, -42,44,42,41,46,47,4.3,4.2,4,"yabadabadoo",self.ctx,"yabadabadoo")
          self.tobj.Struct = mystruct
          aSecondStruct = self.tobj.Struct

          self.assertTrue( self.tobj.Struct == mystruct, "struct roundtrip for equality test" )
          self.assertTrue( aSecondStruct == mystruct, "struct roundtrip for equality test2" )
          aSecondStruct.Short = 720
          self.assertTrue( not aSecondStruct == mystruct , "different structs equality test" )
          self.assertTrue( not self.ctx == mystruct , "object is not equal to struct test" )
          self.assertTrue( mystruct == self.tobj.transportAny( mystruct ), "struct roundtrip with any test" )
          my2ndstruct = uno.createUnoStruct( "test.testtools.bridgetest.TestData", \
                      1, 'h', 43, -42,44,42,41,46,47,4.3,4.2,4,"yabadabadoo",self.ctx,"yabadabadoo",())
          self.assertTrue( my2ndstruct == mystruct, "struct non-default ctor test" )
      def testUnicode( self ):
          uni = u'\0148'
          self.tobj.String = uni
          self.assertTrue( uni == self.tobj.String )
          

          self.tobj.String = u'dubidu'
          self.assertTrue( u'dubidu' == self.tobj.String , "unicode comparison test")
          self.assertTrue( 'dubidu' == self.tobj.String , "unicode vs. string comparison test" )

      def testConstant( self ):
          self.assertTrue( uno.getConstantByName( "com.sun.star.beans.PropertyConcept.ATTRIBUTES" ) == 4,\
                                           "constant retrieval test" )

      def testExceptions( self ):
          unoExc = uno.getClass( "com.sun.star.uno.Exception" )
          ioExc = uno.getClass( "com.sun.star.io.IOException" )
          dispExc = uno.getClass( "com.sun.star.lang.DisposedException" )
          wasHere = 0
          try:
                raise ioExc( "huhuh" , self.tobj )
          except unoExc:
                wasHere = 1
                self.assertTrue( wasHere , "exception test 1" )

          wasHere = 0
          try:
                raise ioExc
          except ioExc:
                wasHere = 1
          else:
                self.assertTrue( wasHere, "exception test 2" )

          wasHere = 0
          try:
                raise dispExc
          except ioExc:
                 pass
          except unoExc:
                wasHere = 1
          self.assertTrue(wasHere, "exception test 3")
    
          illegalArg = uno.getClass( "com.sun.star.lang.IllegalArgumentException" )
          wasHere = 0
          try:
                self.tobj.raiseException( 1 , "foo" , self.tobj )
                self.assertTrue( 0 , "exception test 5a" )
          except ioExc:
                self.assertTrue( 0 , "exception test 5b" )
          except illegalArg as i:
                self.assertTrue( 1 == i.ArgumentPosition , "exception member test" )
                self.assertTrue( "foo" == i.Message , "exception member test 2 " )
                wasHere = 1
          else:
                self.assertTrue( 0, "except test 5c" )
                self.assertTrue( wasHere, "illegal argument exception test failed" )
                  
      def testInterface(self):
          clazz = uno.getClass( "com.sun.star.lang.XComponent" )
          self.assertTrue( "com.sun.star.lang.XComponent" == clazz.__pyunointerface__ )
          self.assertTrue( issubclass( clazz, uno.getClass( "com.sun.star.uno.XInterface" ) ) )
          self.tobj.Interface = None
           

      def testOutparam( self):
          # outparameter
          struct, mybool,mychar,mybyte,myshort,myushort,mylong,myulong,myhyper,myuhyper,myfloat,        \
              mydouble,myenum,mystring,myinterface,myany,myseq,my2ndstruct = self.tobj.getValues(       \
              None,None,None,None,None,None,None,None,None,None,                \
              None,None,None,None,None,None,None)
          self.assertTrue(struct == self.tobj.Struct, "outparam 1 test")
          self.assertTrue(self.tobj.Bool, "outparam 2 test")
          self.assertTrue(mychar == self.tobj.Char, "outparam 3 test")
          self.assertTrue(mybyte == self.tobj.Byte, "outparam 4 test")
          self.assertTrue(myshort == self.tobj.Short, "outparam 5 test")
          self.assertTrue(myushort == self.tobj.UShort, "outparam 6 test")
          self.assertTrue(mylong == self.tobj.Long, "outparam 7 test")
          self.assertTrue(myulong == self.tobj.ULong, "outparam 8 test")
          self.assertTrue(myhyper == self.tobj.Hyper, "outparam 9 test")
          self.assertTrue(myuhyper == self.tobj.UHyper, "outparam 10 test")
          self.assertTrue(myfloat == self.tobj.Float, "outparam 11 test")
          self.assertTrue(mydouble == self.tobj.Double, "outparam 12 test")
          self.assertTrue(myenum == self.tobj.Enum, "outparam 13 test")
          self.assertTrue(mystring == self.tobj.String, "outparam 14 test")
          self.assertTrue(myinterface == self.tobj.Interface, "outparam 15 test")
          self.assertTrue(myany == self.tobj.Any, "outparam 16 test")
          self.assertTrue(myseq == self.tobj.Sequence, "outparam 17 test")
          self.assertTrue(my2ndstruct == struct, "outparam 18 test")

# should work, debug on windows, why not    
#    struct, mybool,mychar,mybyte,myshort,myushort,mylong,myulong,myhyper,myuhyper,myfloat,\
#              mydouble,myenum,mystring,myinterface,myany,myseq,my2ndstruct = self.tobj.setValues2( \
#             mybool,mychar,mybyte,myshort,myushort,mylong,myulong,myhyper,myuhyper,myfloat,\
#              mydouble,myenum,mystring,myinterface,myany,myseq,my2ndstruct)
#    self.assertTrue(struct == self.tobj.Struct, "outparam 1 test")
#    self.assertTrue( mybool and self.tobj.Bool, "outparam 2 test")
#    self.assertTrue(mychar == self.tobj.Char, "outparam 3 test")
#    self.assertTrue(mybyte == self.tobj.Byte, "outparam 4 test")
#    self.assertTrue(myshort == self.tobj.Short, "outparam 5 test")
#    self.assertTrue(myushort == self.tobj.UShort, "outparam 6 test")
#    self.assertTrue(mylong == self.tobj.Long, "outparam 7 test")
#    self.assertTrue(myulong == self.tobj.ULong, "outparam 8 test")
#    self.assertTrue(myhyper == self.tobj.Hyper, "outparam 9 test")
#    self.assertTrue(myuhyper == self.tobj.UHyper, "outparam 10 test")
#    self.assertTrue(myfloat == self.tobj.Float, "outparam 11 test")
#    self.assertTrue(mydouble == self.tobj.Double, "outparam 12 test")
#    self.assertTrue(myenum == self.tobj.Enum, "outparam 13 test")
#    self.assertTrue(mystring == self.tobj.String, "outparam 14 test")
#    self.assertTrue(myinterface == self.tobj.Interface, "outparam 15 test")
#    self.assertTrue(myany == self.tobj.Any, "outparam 16 test")
#    self.assertTrue(myseq == self.tobj.Sequence, "outparam 17 test")
#    self.assertTrue(my2ndstruct == struct, "outparam 18 test")

      def testErrors( self ):

          wasHere = 0
          try:
                self.tobj.a = 5
                self.fail("attribute a shouldn't exist")
          except AttributeError:
                 wasHere = 1
          except IllegalArgumentException:
                 wasHere = 1     
          self.assertTrue( wasHere, "wrong attribute test" )

          IllegalArgumentException = uno.getClass("com.sun.star.lang.IllegalArgumentException" )
          RuntimeException = uno.getClass("com.sun.star.uno.RuntimeException" )

# TODO: Remove this once it is done
# wrong number of arguments bug !?
          self.assertRaises( IllegalArgumentException, self.tobj.transportAny, 42, 43 )
          self.assertRaises( IllegalArgumentException, self.tobj.transportAny )
          self.assertRaises( RuntimeException, uno.getClass, "a.b" )
          self.assertRaises( RuntimeException, uno.getClass, "com.sun.star.uno.TypeClass" )

          self.assertRaises( RuntimeException, uno.Enum,  "a" , "b" )
          self.assertRaises( RuntimeException, uno.Enum, "com.sun.star.uno.TypeClass" , "b" )
          self.assertRaises( RuntimeException, uno.Enum, "com.sun.star.uno.XInterface" , "b" )

          tcInterface =uno.Enum( "com.sun.star.uno.TypeClass" , "INTERFACE" )
          self.assertRaises( RuntimeException, uno.Type, "a", tcInterface )
          self.assertRaises( RuntimeException, uno.Type, "com.sun.star.uno.Exception", tcInterface )

          self.assertRaises( (RuntimeException,exceptions.RuntimeError), uno.getTypeByName, "a" )

          self.assertRaises( (RuntimeException), uno.getConstantByName, "a" )
          self.assertRaises( (RuntimeException), uno.getConstantByName, "com.sun.star.uno.XInterface" )

      def testByteSequence( self ):
          s = uno.ByteSequence( b"ab" )
          self.assertTrue( s == uno.ByteSequence( b"ab" ) )
          self.assertTrue( uno.ByteSequence( b"abc" ) == s + uno.ByteSequence( b"c" ) )
          self.assertTrue( uno.ByteSequence( b"abc" ) == s + "c" )
          self.assertTrue( s + "c"  == "abc" )
          self.assertTrue( s == uno.ByteSequence( s ) )
          self.assertTrue( s[0] == 'a' )
          self.assertTrue( s[1] == 'b' )
          

      def testInvoke( self ):
          self.assertTrue( 5 == uno.invoke( self.tobj , "transportAny" , (uno.Any("byte", 5),) ) )
          self.assertTrue( 5 == uno.invoke(
              PythonTransporter(), "transportAny" , (uno.Any( "byte", 5 ),) ) )
          t = uno.getTypeByName( "long" )
          mystruct = uno.createUnoStruct(
              "com.sun.star.beans.PropertyValue", "foo",0,uno.Any(t,2),0 )
          mystruct.Value = uno.Any(t, 1)
          
          
