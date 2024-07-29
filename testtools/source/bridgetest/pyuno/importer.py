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
          self.assertTrue( IllegalArgumentException is not None, "none-test" )
          self.assertRaises( IllegalArgumentException, self.tobj.raiseException, 1,"foo",self.tobj)
                 
          self.assertTrue( TWO == uno.Enum( "test.testtools.bridgetest.TestEnum","TWO"), "enum" )
          self.assertTrue( UNSIGNED_LONG == uno.Enum( "com.sun.star.uno.TypeClass", "UNSIGNED_LONG" ) )
          self.assertTrue( typeOfIllegalArgumentException ==
                           uno.Type( "com.sun.star.lang.IllegalArgumentException", EXCEPTION) )

          # should not throw an exception
          e = EventObject()
          e.Source = self.ctx
          e = EventObject( self.ctx )
          e = EventObject( e )

      def testDynamicComponentRegistration( self ):
          ctx = uno.getComponentContext()
          self.assertTrue(
              "com.sun.star.connection.Acceptor" not in ctx.ServiceManager.getAvailableServiceNames(),
              "precondition for dynamic component registration test is not fulfilled" )
          self.assertTrue(
              "com.sun.star.connection.Connector" not in ctx.ServiceManager.getAvailableServiceNames(),
              "precondition for dynamic component registration test is not fulfilled" )
          unohelper.addComponentsToContext(
              ctx , ctx, ("acceptor.uno","connector.uno"), "com.sun.star.loader.SharedLibrary" )
          self.assertTrue(
                ("com.sun.star.connection.Acceptor" in ctx.ServiceManager.getAvailableServiceNames()) )
          self.assertTrue(
                ("com.sun.star.connection.Connector" in ctx.ServiceManager.getAvailableServiceNames()))

def suite( ctx ):
    suite = unittest.TestSuite()
    suite.addTest(ImporterTestCase("testStandard",ctx))
    suite.addTest(ImporterTestCase("testDynamicComponentRegistration",ctx))
    return suite

