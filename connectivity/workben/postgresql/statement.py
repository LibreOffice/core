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
#   The Initial Developer of the Original Code is: Joerg Budischewski
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Joerg Budischewski
#
#
#
#*************************************************************************
import unohelper
import unittest
import ddl
from com.sun.star.sdbc import SQLException, XArray
from com.sun.star.sdbc.DataType import VARCHAR
from com.sun.star.util import Date
from com.sun.star.util import Time
from com.sun.star.util import DateTime

# todo
class MyArray( unohelper.Base, XArray ):
    def __init__( self, data ):
        self.data = data
    def getBaseType( self ):
        return VARCHAR
    def getBaseTypeName( self ):
        return "varchar"
    def getArray( self, foo ):
        return self.data
    def getArrayAtIndex( self, startIndex, count, foo ):
        return self.data[startIndex:startIndex+count-1]
    def getResultSet( self, foo):
        return None
    def getResultSetAtIndex( self, startIndex, count, foo ):
        return None

def suite(ctx,dburl):
    suite = unittest.TestSuite()
    suite.addTest(TestCase("testRobustness",ctx,dburl))
    suite.addTest(TestCase("testRow",ctx,dburl))
    suite.addTest(TestCase("testNavigation",ctx,dburl))
    suite.addTest(TestCase("testDatabaseMetaData",ctx,dburl))
    suite.addTest(TestCase("testGeneratedResultSet",ctx,dburl))
    suite.addTest(TestCase("testResultSetMetaData",ctx,dburl))
    suite.addTest(TestCase("testArray",ctx,dburl))
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
                   'org.openoffice.comp.connectivity.pq.Driver.noext' , self.ctx )
          self.connection = self.driver.connect( self.dburl, () )
          self.stmt = self.connection.createStatement()
          try:
                self.stmt.executeUpdate( "DROP TABLE firsttable" )
          except SQLException,e:
              pass

          ddls = (
               "BEGIN",
               "CREATE TABLE firsttable (tString text,tInteger integer,tShort smallint,tLong bigint,tFloat real,"+
                       "tDouble double precision,tByteSeq bytea,tBool boolean, tDate date, tTime time, tTimestamp timestamp, tIntArray integer[], tStringArray text[], tSerial serial ) WITH OIDS",
               "INSERT INTO firsttable VALUES ( 'foo', 70000, 12000, 70001, 2.4, 2.45, 'huhu', 'true',  '1999-01-08','04:05:06','1999-01-08 04:05:06', '{2,3,4}', '{\"huhu\",\"hi\"}')",
               "INSERT INTO firsttable VALUES ( 'foo2', 69999, 12001, 70002, -2.4, 2.55, 'huhu', 'false', '1999-01-08','04:05:06','1999-01-08 04:05:06', NULL , '{\"bla\"}' )",
               "INSERT INTO firsttable VALUES ( 'foo2', 69999, 12001, 70002, -2.4, 2.55, 'huhu', null,  '1999-01-08', '04:05:06','1999-01-08 04:05:06', '{}' , '{\"bl ubs\",\"bl\\\\\\\\a}}b\\\\\"a\",\"blub\"}' )",
               "COMMIT" )
          for i in ddls:
              self.stmt.executeUpdate(i)

      def tearDown(self):
          self.stmt.close()
          self.connection.close()

      def testRow(self):
          row = ("foo",70000,12000,70001,2.4,2.45, "huhu", True ,
                 Date(8,1,1999), Time(0,6,5,4),DateTime(0,6,5,4,8,1,1999) )
          row2 = ("foo2",69999,12001,70002,-2.4,2.55, "huhu", False )

          rs = self.stmt.executeQuery( "SELECT * from firsttable" )
          self.failUnless( rs.next() )

          self.failUnless( rs.getString(1) == row[0] )
          self.failUnless( rs.getInt(2) == row[1] )
          self.failUnless( rs.getShort(3) == row[2] )
          self.failUnless( rs.getLong(4) == row[3] )
          self.failUnless( realEquals(rs.getFloat(5), row[4], 0.001))
          self.failUnless( realEquals(rs.getDouble(6), row[5], 0.00001))
          self.failUnless( rs.getBytes(7) == row[6] )
          self.failUnless( rs.getBoolean(8) == row[7] )
          self.failUnless( rs.getDate(9) == row[8] )
          self.failUnless( rs.getTime(10) == row[9] )
          self.failUnless( rs.getTimestamp(11) == row[10] )

          a = rs.getArray(12)
          data = a.getArray( None )
          self.failUnless( len( data ) == 3 )
          self.failUnless( int(data[0] ) == 2 )
          self.failUnless( int(data[1] ) == 3 )
          self.failUnless( int(data[2] ) == 4 )

          self.failUnless( rs.next() )

          self.failUnless( rs.next() )
          data = rs.getArray(13).getArray(None)
          self.failUnless( data[0] == "bl ubs" )
          self.failUnless( data[1] == "bl\\a}}b\"a" ) # check special keys
          self.failUnless( data[2] == "blub" )

          rs.getString(8)
          self.failUnless(  rs.wasNull()  )
          rs.getString(7)
          self.failUnless( not rs.wasNull() )

          self.failUnless( rs.findColumn( "tShort" ) == 3 )
          rs.close()

      def testNavigation( self ):
          rs = self.stmt.executeQuery( "SELECT * from firsttable" )
          self.failUnless( rs.isBeforeFirst() )
          self.failUnless( not rs.isAfterLast() )
          self.failUnless( rs.isBeforeFirst() )

          self.failUnless( rs.next() )
          self.failUnless( rs.isFirst() )
          self.failUnless( not rs.isLast() )
          self.failUnless( not rs.isBeforeFirst() )

          self.failUnless( rs.next() )
          self.failUnless( rs.next() )
          self.failUnless( not rs.next() )
          self.failUnless( rs.isAfterLast() )

          rs.absolute( 1 )
          self.failUnless( rs.isFirst() )

          rs.absolute( 3 )
          self.failUnless( rs.isLast() )

          rs.relative( -1 )
          self.failUnless( rs.getRow() == 2 )

          rs.relative( 1 )
          self.failUnless( rs.getRow() == 3 )

          rs.close()

      def testRobustness( self ):
          rs = self.stmt.executeQuery( "SELECT * from firsttable" )

      self.failUnlessRaises( SQLException, rs.getString , 1 )

      rs.next()
      self.failUnlessRaises( SQLException, rs.getString , 24 )
      self.failUnlessRaises( SQLException, rs.getString , 0 )

      self.connection.close()
      self.failUnlessRaises( SQLException, rs.getString , 1 )
      self.failUnlessRaises( SQLException, self.stmt.executeQuery, "SELECT * from firsttable" )
      rs.close()


      def testDatabaseMetaData( self ):
          meta = self.connection.getMetaData()

          self.failUnless( not meta.isReadOnly() )

      def testGeneratedResultSet( self ):
          self.stmt.executeUpdate(
              "INSERT INTO firsttable VALUES ( 'foo3', 69998, 12001, 70002, -2.4, 2.55, 'huhu2')" )
          #ddl.dumpResultSet( self.stmt.getGeneratedValues() )
          rs = self.stmt.getGeneratedValues()
          self.failUnless( rs.next() )
          self.failUnless( rs.getInt( 14 )  == 4 )

      def testResultSetMetaData( self ):
          rs = self.stmt.executeQuery( "SELECT * from firsttable" )

          # just check, if we get results !
          meta = rs.getMetaData()

          count = meta.getColumnCount()
          for i in range( 1, count+1):
              meta.isNullable( i )
              meta.isCurrency( i )
              meta.isCaseSensitive( i )
              meta.isSearchable( i )
              meta.isSigned( i )
              meta.getColumnDisplaySize( i )
              meta.getColumnName( i )
              meta.getColumnLabel( i )
              meta.getSchemaName( i )
              meta.getPrecision( i )
              meta.getScale( i )
              meta.getTableName( i )
              meta.getColumnTypeName( i )
              meta.getColumnType( i )
              meta.isReadOnly( i )
              meta.isWritable( i )
              meta.isDefinitelyWritable( i )
              meta.getColumnServiceName( i )

      def testArray( self ):
          stmt = self.connection.prepareStatement(
              "INSERT INTO firsttable VALUES ( 'insertedArray', 70000, 12000, 70001, 2.4, 2.45, 'huhu', 'true',  '1999-01-08','04:05:06','1999-01-08 04:05:06', '{2,3,4}', ? )" )
          myarray = ( "a", "\"c", "}d{" )
          stmt.setArray( 1, MyArray( myarray ) )
          stmt.executeUpdate()

          stmt = self.connection.createStatement()
          rs = stmt.executeQuery( "SELECT tStringArray FROM firsttable WHERE tString = 'insertedArray'" )
          rs.next()
          data =  rs.getArray(1).getArray(None)
          self.failUnless( data[0] == myarray[0] )
          self.failUnless( data[1] == myarray[1] )
          self.failUnless( data[2] == myarray[2] )
