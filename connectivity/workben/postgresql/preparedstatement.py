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
from uno import ByteSequence
from com.sun.star.sdbc import SQLException
from com.sun.star.sdbc.ResultSetConcurrency import UPDATABLE
from com.sun.star.sdbc.DataType import NUMERIC,VARCHAR

def suite(ctx,dburl):
    suite = unittest.TestSuite()
    suite.addTest(TestCase("testQuery",ctx,dburl))
    suite.addTest(TestCase("testGeneratedResultSet",ctx,dburl))
    suite.addTest(TestCase("testUpdateableResultSet",ctx,dburl))
    suite.addTest(TestCase("testQuoteQuote",ctx,dburl))
    return suite

def realEquals( a,b,eps ):
    val = a - b
    if val < 0:
       val = -1. * val
    return val < eps

class TestCase(unittest.TestCase):
      def __init__(self,method,ctx,dburl):
          unittest.TestCase.__init__(self,method)
          self.ctx = ctx
          self.dburl = dburl

      def setUp(self):
          self.driver = self.ctx.ServiceManager.createInstanceWithContext(
                   'org.openoffice.comp.connectivity.pq.Driver.noext', self.ctx )
          self.connection = self.driver.connect( self.dburl, () )
          ddl.executeDDLs( self.connection )

      def testDown( self ):
          self.connection.close()

      def testQuery( self ):

          stmts = "SELECT product.id FROM product WHERE product.price > :lowprice AND product.price < :upprice", \
              "SELECT product.id FROM product WHERE product.price > ? AND product.price < ?" , \
              "SELECT \"product\".\"id\" FROM product WHERE \"product\".\"price\" > :lowprice AND \"product\".\"price\" < :upprice"


          for stmt in stmts:
              prepstmt = self.connection.prepareStatement( stmt )
              prepstmt.setDouble( 1, 5.80 )
              prepstmt.setObjectWithInfo( 2, 7. , NUMERIC, 2)
              prepstmt.setObjectWithInfo( 2, "7.0000", NUMERIC, 2 )
              rs = prepstmt.executeQuery( )
              self.failUnless( rs.getMetaData().getColumnCount() == 1 )
              self.failUnless( rs.getMetaData().getColumnName(1) == "id")
              self.failUnless( prepstmt.getMetaData().getColumnCount() == 1 )
              self.failUnless( prepstmt.getMetaData().getColumnName(1) == "id" )
              self.failUnless( rs.next() )
              self.failUnless( rs.getString( 1 ).strip() == "PZZ2" )
              self.failUnless( rs.next() )
              self.failUnless( rs.getString( 1 ).strip() == "PZZ5" )
              self.failUnless( rs.isLast() )

          prepstmt = self.connection.prepareStatement(
              "SELECT name FROM product WHERE id = ?" )
          prepstmt.setString( 1, 'PZZ2' )
          rs = prepstmt.executeQuery()
          self.failUnless( rs.next() )
          self.failUnless( rs.getString( 1 ) == "Pizza Mista" )
          self.failUnless( rs.isLast() )

          prepstmt = self.connection.prepareStatement(
              "SELECT name FROM product WHERE image = ?" )
          prepstmt.setBytes( 1, ByteSequence( "\001foo\005" ) )
          rs = prepstmt.executeQuery()
          self.failUnless( rs.next() )
          self.failUnless( rs.getString( 1 ) == "Pizza Funghi" )
          self.failUnless( rs.isLast() )

          prepstmt = self.connection.prepareStatement(
              "SELECT * FROM ordertab WHERE delivered = ?" )
          prepstmt.setBoolean( 1 , False )
          rs = prepstmt.executeQuery()
          self.failUnless( rs.next() )
          self.failUnless( rs.getString( 1 ).strip() == "2" )
          self.failUnless( rs.isLast() )

          stmt = self.connection.createStatement()
          rs =  stmt.executeQuery( "SELECT * FROM \"public\".\"customer\"" )

          stmt.executeUpdate( "DELETE FROM product where id='PAS5'" )
          prepstmt =self.connection.prepareStatement(
              "INSERT INTO product VALUES(?,'Ravioli',?,NULL)" );
          prepstmt.setObjectWithInfo( 1, "PAS5" ,VARCHAR,0)
          prepstmt.setObjectWithInfo( 2, "9.223" ,NUMERIC,2)
          prepstmt.executeUpdate()
          rs= stmt.executeQuery( "SELECT price FROM product WHERE id = 'PAS5'" )
          self.failUnless( rs.next() )
          self.failUnless( rs.getString( 1 ).strip() == "9.22" )

          stmt.executeUpdate( "DELETE FROM product where id='PAS5'" )
          prepstmt =self.connection.prepareStatement(
              "INSERT INTO product VALUES('PAS5','Ravioli',?,NULL)" );
          prepstmt.setObjectWithInfo( 1, 9.223,NUMERIC,2 )
          prepstmt.executeUpdate()
          rs= stmt.executeQuery( "SELECT price FROM product WHERE id = 'PAS5'" )
          self.failUnless( rs.next() )
          self.failUnless( rs.getString( 1 ).strip() == "9.22" )

      def testGeneratedResultSet( self ):
          prepstmt = self.connection.prepareStatement(
              "INSERT INTO customer VALUES( ?, ? )" )
          prepstmt.setString( 1, "C3" )
          prepstmt.setString( 2, "Norah Jones" )
          prepstmt.executeUpdate()
          rs = prepstmt.getGeneratedValues()
          self.failUnless( rs.next() )
          self.failUnless( rs.getInt( 3 ) == 3 )

          prepstmt = self.connection.prepareStatement(
              "INSERT INTO public.nooid (id,name) VALUES( ?, ? )" )
          prepstmt.setString( 1, "C3" )
          prepstmt.setString( 2, "Norah Jones" )
          prepstmt.executeUpdate()
          rs = prepstmt.getGeneratedValues()
          self.failUnless( rs.next() )
          self.failUnless( rs.getString(1).rstrip() == "C3" )

          prepstmt = self.connection.prepareStatement(
              "INSERT INTO public.nooid2 (name) VALUES( ? )" )
          prepstmt.setString( 1, "Norah Jones" )
          prepstmt.executeUpdate()
          rs = prepstmt.getGeneratedValues()
          self.failUnless( rs )
          self.failUnless( rs.next() )
          self.failUnless( rs.getString(2) == "Norah Jones" )
          self.failUnless( rs.getString(1) == "1" )

      def testUpdateableResultSet( self ):
          stmt = self.connection.createStatement()
          stmt.ResultSetConcurrency = UPDATABLE
          rs = stmt.executeQuery( "SELECT * FROM orderpos" )
#          ddl.dumpResultSet( rs )
          rs.next()
          rs.deleteRow()
          rs.next()
          rs.updateInt( 4 , 32 )
          rs.updateRow()

          rs.moveToInsertRow()
          rs.updateString( 1 , '2' )
          rs.updateString( 2, '003' )
          rs.updateString( 3, 'PZZ5' )
          rs.updateInt( 4, 22 )
          rs.insertRow()

          rs = stmt.executeQuery( "SELECT * FROM orderpos" )
          rs = stmt.executeQuery( "SELECT * FROM \"public\".\"orderpos\"" )
#          ddl.dumpResultSet( rs )

      def testQuoteQuote( self ):
          stmt = self.connection.prepareStatement( "select 'foo''l'" )
          rs = stmt.executeQuery()
          self.failUnless( rs )
          self.failUnless( rs.next() )
          self.failUnless( rs.getString(1) == "foo'l" )

          stmt = self.connection.prepareStatement( "select 'foo''''l'" )
          rs = stmt.executeQuery()
          self.failUnless( rs )
          self.failUnless( rs.next() )
          self.failUnless( rs.getString(1) == "foo''l" )
