#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
"tests bridging python implementations of UNO objects"
import unittest
import uno
import unohelper
import os
import sys

from com.sun.star.io import XOutputStream, XInputStream, typeOfXOutputStream, typeOfXInputStream
from com.sun.star.lang import XTypeProvider, typeOfXTypeProvider, XEventListener
from com.sun.star.uno import XCurrentContext

class SequenceOutputStream( unohelper.Base, XOutputStream ):
      def __init__( self ):
          self.s = uno.ByteSequence("")
          self.closed = 0
          
      def closeOutput(self):
          self.closed = 1

      def writeBytes( self, seq ):
          self.s = self.s + seq

      def flush( self ):
          pass

      def getSequence( self ):
          return self.s
          
                  
class SequenceInputStream( XInputStream, unohelper.Base ):
      def __init__( self, seq ):
          self.s = seq
          self.nIndex = 0
          self.closed = 0
          
      def closeInput( self):
          self.closed = 1
          self.s = None
          
      def skipBytes( self, nByteCount ):
          if( nByteCount + self.nIndex > len(self.s) ):
              nByteCount = len(self.s) - self.nIndex
          self.nIndex += nByteCount

      def readBytes( self, retSeq, nByteCount ):
          nRet = 0
          if( self.nIndex + nByteCount > len(self.s) ):
              nRet = len(self.s) - self.nIndex
          else:
              nRet = nByteCount
          retSeq = uno.ByteSequence(self.s.value[self.nIndex : self.nIndex + nRet ])
          self.nIndex = self.nIndex + nRet
          return nRet, retSeq
          
      def readSomeBytes( self, retSeq , nByteCount ):
          #as we never block !
          return readBytes( retSeq, nByteCount )
          
      def available( self ):
          return len( self.s ) - self.nIndex

class SequenceInputStream2( SequenceInputStream ):
      def __init__( self, seq ):
            SequenceInputStream.__init__( self, seq )
            
class TestCase(unittest.TestCase):
      def __init__(self,method,ctx):
          unittest.TestCase.__init__(self,method)
          self.ctx = ctx

      def setUp(self):
          self.tobj = self.ctx.ServiceManager.createInstanceWithContext( \
                           "com.sun.star.test.bridge.CppTestObject",self.ctx)
          self.pipe = self.ctx.ServiceManager.createInstanceWithContext( \
                           "com.sun.star.io.Pipe" , self.ctx )
                           
      def testStandard( self ):
          dataOut = self.ctx.ServiceManager.createInstanceWithContext( \
                        "com.sun.star.io.DataOutputStream", self.ctx )
          streamOut = SequenceOutputStream()
          dataOut.setOutputStream( streamOut )
          dataOut.writeShort( 42 )
          dataOut.writeLong( 43 )
          dataOut.closeOutput()

          dataInput = self.ctx.ServiceManager.createInstanceWithContext( \
                   "com.sun.star.io.DataInputStream", self.ctx )
          
          dataInput.setInputStream( SequenceInputStream2( streamOut.getSequence() ) )

          self.failUnless( 42 == dataInput.readShort() )
          self.failUnless( 43 == dataInput.readLong() )
          self.failUnless( self.tobj.transportAny( streamOut ) == streamOut )


class NullDevice:
      def write( self, string ):
            pass
      

class EventListener( unohelper.Base, XEventListener ):
    def __init__( self ):
        self.disposingCalled = False
        
    def disposing( self , eventObject ):
        self.disposingCalled = True

class TestHelperCase( unittest.TestCase ):

      def __init__(self,method):
            unittest.TestCase.__init__(self,method)

      def testUrlHelper( self ):
            systemPath = os.getcwd()
            if systemPath.startswith( "/" ):
                  self.failUnless( "/tmp" == unohelper.fileUrlToSystemPath( "file:///tmp" ) )
                  self.failUnless( "file:///tmp" == unohelper.systemPathToFileUrl( "/tmp" ))
            else:
                  self.failUnless( "c:\\temp" == unohelper.fileUrlToSystemPath( "file:///c:/temp" ) )
                  self.failUnless( "file:///c:/temp" == unohelper.systemPathToFileUrl( "c:\\temp" ) )

            systemPath = unohelper.systemPathToFileUrl( systemPath )
            self.failUnless( systemPath + "/a" == unohelper.absolutize( systemPath, "a" ) )
      def testInspect( self ):
            dev = NullDevice()
#            dev = sys.stdout
            unohelper.inspect( uno.getComponentContext() , dev )
            unohelper.inspect( uno.getComponentContext().ServiceManager , dev )
            unohelper.inspect( uno.getTypeByName( "com.sun.star.lang.XComponent" ) , dev )

      def testListener( self ):
            smgr = uno.getComponentContext().ServiceManager.createInstance(
                  "com.sun.star.lang.ServiceManager" )

            # check, whether listeners 
            listener = EventListener()
            smgr.addEventListener( listener )
            smgr.dispose()
            self.failUnless( listener.disposingCalled )

            # check, whether listeners can be removed
            smgr = uno.getComponentContext().ServiceManager.createInstance(
                  "com.sun.star.lang.ServiceManager" )
            listener = EventListener()
            smgr.addEventListener( listener )
            smgr.removeEventListener( listener )
            smgr.dispose()
            self.failUnless( not listener.disposingCalled )
            
      def testCurrentContext( self ):
            oldContext = uno.getCurrentContext()
            try:
                  uno.setCurrentContext(
                        unohelper.CurrentContext( oldContext,{"My42":42}) )
                  self.failUnless( 42 == uno.getCurrentContext().getValueByName( "My42" ) )
                  self.failUnless( None == uno.getCurrentContext().getValueByName( "My43" ) )
            finally:
                  uno.setCurrentContext( oldContext )
          
            

def suite( ctx ):
    suite = unittest.TestSuite()
    suite.addTest(TestCase("testStandard",ctx))
    suite.addTest(TestHelperCase( "testUrlHelper" ))
    suite.addTest(TestHelperCase( "testInspect" ))
    suite.addTest(TestHelperCase( "testListener" ) )
    suite.addTest(TestHelperCase( "testCurrentContext" ) )
    return suite
                                           
