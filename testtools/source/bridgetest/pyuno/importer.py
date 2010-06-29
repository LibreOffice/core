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
import unittest
import uno
import unohelper

from com.sun.star.lang import EventObject,IllegalArgumentException,typeOfIllegalArgumentException
from test.testtools.bridgetest.TestEnum import TWO
from com.sun.star.uno.TypeClass import UNSIGNED_LONG,EXCEPTION
class ImporterTestCase(unittest.TestCase):
      def __init__(self,method,ctx):
          unittest.TestCase.__init__(self,method)
          self.ctx = ctx
          
      def setUp(self):
          self.tobj = self.ctx.ServiceManager.createInstanceWithContext( \
                           "com.sun.star.test.bridge.CppTestObject",self.ctx)

      def testStandard( self ):
          self.failUnless( IllegalArgumentException != None, "none-test" )
          self.failUnlessRaises( IllegalArgumentException, self.tobj.raiseException, 1,"foo",self.tobj)
                 
          self.failUnless( TWO == uno.Enum( "test.testtools.bridgetest.TestEnum","TWO"), "enum" )
          self.failUnless( UNSIGNED_LONG == uno.Enum( "com.sun.star.uno.TypeClass", "UNSIGNED_LONG" ) )
          self.failUnless( typeOfIllegalArgumentException ==
                           uno.Type( "com.sun.star.lang.IllegalArgumentException", EXCEPTION) )

          # should not throw an exception
          e = EventObject()
          e.Source = self.ctx
          e = EventObject( self.ctx )
          e = EventObject( e )

      def testDynamicComponentRegistration( self ):
          ctx = uno.getComponentContext()
          self.failUnless(
              not ("com.sun.star.connection.Acceptor" in ctx.ServiceManager.getAvailableServiceNames()),
              "precondition for dynamic component registration test is not fulfilled" )
          self.failUnless(
              not ("com.sun.star.connection.Connector" in ctx.ServiceManager.getAvailableServiceNames()),
              "precondition for dynamic component registration test is not fulfilled" )
          unohelper.addComponentsToContext(
              ctx , ctx, ("acceptor.uno","connector.uno"), "com.sun.star.loader.SharedLibrary" )
          self.failUnless(
                ("com.sun.star.connection.Acceptor" in ctx.ServiceManager.getAvailableServiceNames()) )
          self.failUnless(
                ("com.sun.star.connection.Connector" in ctx.ServiceManager.getAvailableServiceNames()))

def suite( ctx ):
    suite = unittest.TestSuite()
    suite.addTest(ImporterTestCase("testStandard",ctx))
    suite.addTest(ImporterTestCase("testDynamicComponentRegistration",ctx))
    return suite

