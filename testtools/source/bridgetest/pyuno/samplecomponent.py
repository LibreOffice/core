#*************************************************************************
#
#   $RCSfile: samplecomponent.py,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obo $ $Date: 2004-11-15 13:06:59 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Ralph Thomas
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Ralph Thomas, Joerg Budischewski
#
#
#
#*************************************************************************
import uno
import unohelper

from com.sun.star.lang import IllegalArgumentException,XServiceInfo
from com.sun.star.uno import RuntimeException
from com.sun.star.beans import UnknownPropertyException
from test.testtools.bridgetest import TestData,XRecursiveCall,XBridgeTestBase

g_ImplementationHelper = unohelper.ImplementationHelper()
g_implName = "org.openoffice.comp.pyuno.PythonTestObject"

g_attribs = "RuntimeException", "Bool", "Char", "Byte", "Short", "UShort", \
            "Long", "ULong", "Hyper", "UHyper", "Float", "Double", "Enum", \
	    "String", "Interface", "Any" , "Sequence" , "Struct"

def assign( rData, bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper,\
	      nUHyper, fFloat, fDouble, eEnum, rStr, xTest, rAny  ):
    rData.Bool = bBool;
    rData.Char = cChar;
    rData.Byte = nByte;
    rData.Short = nShort;
    rData.UShort = nUShort;
    rData.Long = nLong;
    rData.ULong = nULong;
    rData.Hyper = nHyper;
    rData.UHyper = nUHyper;
    rData.Float = fFloat;
    rData.Double = fDouble;
    rData.Enum = eEnum;
    rData.String = rStr;
    rData.Interface = xTest;
    rData.Any = rAny;

class MyRecursiveCall( XRecursiveCall, unohelper.Base ):
      def callRecursivly( xCall, nToCall ):
	  if nToCall:
	     xCall.callRecursivly( self, nToCall -1 )	  
    
class SampleUnoComponent( XBridgeTestBase,XServiceInfo ):
      def __init__(self,ctx):
	  self.__dict__["callid"] = 0
	  self.__dict__["sequenceBroken"] = 0

      def transportAny( self, value ):
	  return value

      def raiseException( self, ArgumentPosition, Message, Context ):
	  raise IllegalArgumentException( Message, Context, ArgumentPosition )
	
      def raiseRuntimeExceptionOneway(self, Message, Context ):
	  raise RuntimeException( Message, Context )

      def setValues( self, bBool, cChar, nByte, nShort, nUShort, nLong,\
		     nULong, nHyper, nUHyper, fFloat, fDouble, eEnum, \
		     aString, xInterface, aAny, aSequence, aStruct ):
         self.__dict__["data"] = TestDataElements( bBool, cChar, nByte, nShort, nUShort, nLong,
	                  nULong, nHyper, nUHyper, fFloat, fDouble, eEnum, aStruct, xInterface,
			  aAny, aSequence )
         self.__dict__["Struct"] = aStruct			  	     

      def setValues2( self, bBool, cChar, nByte, nShort, nUShort, nLong, nULong,\
		      nHyper, nUHyper, fFloat, fDouble, eEnum,		\
		      aString, xInterface, aAny, aSequence, aStruct ):
          self.__dict__["Struct"] = TestData( cChar, nByte, nShort, nUShort, nLong, nULong, nHyper,\
	                                      nUHyper, fFloat, fDouble, eEnum, aStruct, xInterface,\
					      aAny, aSequence )
          self.__dict__["Struct"] = aStruct
	  return bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, nULong, \
	         nHyper, nUHyper, fFloat, fDouble, eEnum, aStruct, xInterface, aAny,	       \
		 (aSequence[1],aSequence[0]), aStruct
      						 
      def getValues(self, a,b,c,d,e,f,g,h, i,j,k,l,m,n):
	  v = self.__dict__["data"]
	  return self.__dict__["Struct"],v.Bool, v.Char, v.Byte, v.Short, v.UShort, v.Long,	\
	         v.ULong, v.Hyper, v.UHyper, v.Float, v.Double, v.Enum, v.String, v.Interface,	\
		 v.Any, v.Sequence, self.__dict__["Struct"]
		 
      def call( self, callid, nWaitMUSEC ):
	  if self.__dict__["callid"] >= callid:
	     self.__dict__["sequenceBroken"] = 1
	  else:
	     self.__dict__["callid"] = callid

      def callOneway( self, nCallId, nWaitMUSEC ):
	  call( nCallId, nWaitMUSEC )
	  
      def sequenceOfCallTestPassed():
	  return self.__dict__["sequenceBroken"]

      def startRecursiveCall( xCall , nToCall ):
	  if nToCall:
	     xCall.callRecursivly( MyRecursiveCall(), nToCall -1 )

      def checkExistence( self, name ):
	  found = 0
	  for x in g_attribs:
	      if x == name:
		 found = 1
		 break
	  if not found:
	     raise UnknownPropertyException( "Property "+name+" is unknown", self )
	     	 
      def __setattr__( self, name, value ):
	  checkExistence( name )
	  self.__dict__[name] = value
	  
      def __getattr__( self, name ):
	  checkExistence( name )
	  return self.__dict__[name]

      def getSupportedServices( self ):
	  return g_ImplementationHelper.getSupportedServices(g_implName)
      def supportsService( self, ServiceName ):
	  return g_ImplementationHelper.supportsService( g_implName, ServiceName )
      def getImplementationName(self):
	  return g_implName


g_ImplementationHelper.addImplementation( \
	SampleUnoComponent,g_implName,("com.sun.star.test.bridge.PythonTestObject",),)
	
#g_ImplementationEntries = \
#    unohelper.ImplementationEntry(				\
#	      "org.openoffice.comp.SamplePythonComponent",	\
#	      ("com.sun.star.test.bridge.PythonTestObject",),	\
#	      SampleUnoComponent)				\
#	   ,

