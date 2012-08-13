#*************************************************************************
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
#*************************************************************************
import unittest
import sys
import ddl

from com.sun.star.sdbc.DataType import SMALLINT, INTEGER, BIGINT , DATE, TIME, TIMESTAMP, NUMERIC

def dumpResultSet( rs , count ):
#    for i in range(1, count):
#        sys.stdout.write(meta.getColumnName( i ) + "\t")
    sys.stdout.write( "\n" )
    while rs.next():
        for i in range( 1, count+1):
            sys.stdout.write( rs.getString( i ) + "\t" )
        sys.stdout.write( "\n" )
    rs.beforeFirst()




def suite(ctx,dburl):
    suite = unittest.TestSuite()
    suite.addTest(TestCase("testDatabaseMetaData",ctx,dburl))
    suite.addTest(TestCase("testTypeGuess",ctx,dburl))
    return suite

class TestCase(unittest.TestCase):
      def __init__(self,method,ctx,dburl):
          unittest.TestCase.__init__(self,method)
          self.ctx = ctx
          self.dburl = dburl


      def setUp( self ):
          self.driver = self.ctx.ServiceManager.createInstanceWithContext(
                   'org.openoffice.comp.connectivity.pq.Driver.noext' , self.ctx )
          self.connection = self.driver.connect( self.dburl, () )
          ddl.executeDDLs( self.connection )

      def tearDown( self ):
          self.connection.close()

      def testDatabaseMetaData( self ):
          meta = self.connection.getMetaData()

          rs = meta.getTables( None, "public", "%", () )
#          dumpResultSet( rs, 5)

          rs = meta.getColumns( None, "%", "customer", "%" )
#          dumpResultSet( rs, 18 )

          rs = meta.getPrimaryKeys( None, "public" , "%" )
#          dumpResultSet( rs , 6 )
          rs = meta.getTablePrivileges( None, "public" , "%" )
#          dumpResultSet( rs , 7 )
          rs = meta.getColumns( None, "public" , "customer", "%" )
#          dumpResultSet( rs , 18 )
          rs = meta.getTypeInfo()
#          dumpResultSet(rs, 18)
      while rs.next():
        if rs.getString(1) == "pqsdbc_short":
           self.failUnless( rs.getInt(2) == SMALLINT )
           break
          self.failUnless( not rs.isAfterLast() )  # domain type cannot be found


          rs = meta.getIndexInfo( None, "public" , "customer", False, False )
#          dumpResultSet( rs, 13 )

      def testTypeGuess( self ):
          stmt = self.connection.createStatement()
          rs = stmt.executeQuery( "SELECT sum(amount) FROM orderpos" )
          meta = rs.getMetaData()
          self.failUnless( meta.getColumnType(1) == BIGINT )

          stmt = self.connection.createStatement()
          rs = stmt.executeQuery( "SELECT sum(price) FROM product" )
          meta = rs.getMetaData()
          self.failUnless( meta.getColumnType(1) == NUMERIC )

          rs = stmt.executeQuery( "SELECT max(ttime) FROM firsttable" )
          meta = rs.getMetaData()
          self.failUnless( meta.getColumnType(1) == TIME )

          rs = stmt.executeQuery( "SELECT max(tdate) FROM firsttable" )
          meta = rs.getMetaData()
          self.failUnless( meta.getColumnType(1) == DATE )

          rs = stmt.executeQuery( "SELECT max(ttimestamp) FROM firsttable" )
          meta = rs.getMetaData()
          self.failUnless( meta.getColumnType(1) == TIMESTAMP )
#          rs.next()
#          print rs.getString( 1 )
