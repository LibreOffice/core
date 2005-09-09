#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: importer.py,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 12:26:37 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

