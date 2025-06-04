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
"tests bridging python implementations of UNO objects"
import unittest
import uno
import unohelper
import os

from com.sun.star.io import XOutputStream, XInputStream
from com.sun.star.lang import XEventListener

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

          self.assertTrue( 42 == dataInput.readShort() )
          self.assertTrue( 43 == dataInput.readLong() )
          self.assertTrue( self.tobj.transportAny( streamOut ) == streamOut )


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
                  self.assertTrue( "/tmp" == unohelper.fileUrlToSystemPath( "file:///tmp" ) )
                  self.assertTrue( "file:///tmp" == unohelper.systemPathToFileUrl( "/tmp" ))
            else:
                  self.assertTrue( "c:\\temp" == unohelper.fileUrlToSystemPath( "file:///c:/temp" ) )
                  self.assertTrue( "file:///c:/temp" == unohelper.systemPathToFileUrl( "c:\\temp" ) )

            systemPath = unohelper.systemPathToFileUrl( systemPath )
            self.assertTrue( systemPath + "/a" == unohelper.absolutize( systemPath, "a" ) )
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
            self.assertTrue( listener.disposingCalled )

            # check, whether listeners can be removed
            smgr = uno.getComponentContext().ServiceManager.createInstance(
                  "com.sun.star.lang.ServiceManager" )
            listener = EventListener()
            smgr.addEventListener( listener )
            smgr.removeEventListener( listener )
            smgr.dispose()
            self.assertTrue( not listener.disposingCalled )
            
      def testCurrentContext( self ):
            oldContext = uno.getCurrentContext()
            try:
                  uno.setCurrentContext(
                        unohelper.CurrentContext( oldContext,{"My42":42}) )
                  self.assertTrue( uno.getCurrentContext().getValueByName( "My42" ) == 42 )
                  self.assertTrue( uno.getCurrentContext().getValueByName( "My43" ) is None )
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
                                           
