#*************************************************************************
#
#   $RCSfile: importer.py,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obo $ $Date: 2004-11-15 13:06:14 $
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

