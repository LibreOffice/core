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
import unittest
import ddl
import unohelper
import sys
from com.sun.star.sdbc import SQLException
from com.sun.star.sdbc.DataType import VARCHAR, CHAR, DECIMAL, DOUBLE, BIGINT, NUMERIC
from com.sun.star.sdbc.ColumnValue import NO_NULLS, NULLABLE
from com.sun.star.sdbcx.KeyType import PRIMARY, FOREIGN, UNIQUE
from com.sun.star.sdbc.KeyRule import RESTRICT, CASCADE, NO_ACTION

def suite(ctx,dburl):
    suite = unittest.TestSuite()
    suite.addTest(TestCase("testTables",ctx,dburl))
    suite.addTest(TestCase("testViews",ctx,dburl))
    suite.addTest(TestCase("testKeys",ctx,dburl))
    suite.addTest(TestCase("testUsers",ctx,dburl))
    suite.addTest(TestCase("testIndexes",ctx,dburl))
    return suite

def nullable2Str( v ):
    if v == NO_NULLS:
        return "NOT NULL"
    return ""

def autoIncremtent2Str( v ):
    if v:
        return "auto increment"
    return ""

def dumpColumns( columns ):
    n = columns.getCount()
    print "Name\t type\t prec\t scale\t"
    for i in range( 0, n ):
        col = columns.getByIndex( i )
        print col.Name + "\t "+col.TypeName + "\t " + str(col.Precision) + "\t " + str(col.Scale) + "\t "+\
              str( col.DefaultValue ) + "\t " + str( col.Description ) + "\t " +\
              autoIncremtent2Str( col.IsAutoIncrement ) + "\t " + \
              nullable2Str( col.IsNullable )


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

      def checkDescriptor( self, descriptor, name, typeName, type, prec, scale, defaultValue, desc ):
          self.failUnless( descriptor.Name == name )
          self.failUnless( descriptor.TypeName == typeName )
          self.failUnless( descriptor.Type == type )
          self.failUnless( descriptor.Precision == prec )
          self.failUnless( descriptor.Scale == scale )
#          print descriptor.DefaultValue + " == " + defaultValue
#          self.failUnless( descriptor.DefaultValue == defaultValue )
          self.failUnless( descriptor.Description == desc )


      def testKeys( self ):
          dd = self.driver.getDataDefinitionByConnection( self.connection )
          tables = dd.getTables()
          t = tables.getByName( "public.ordertab" )
          keys = t.getKeys()
          key = keys.getByName( "cust" )
          self.failUnless( key.Name == "cust" )
          self.failUnless( key.Type == FOREIGN )
          self.failUnless( key.ReferencedTable == "public.customer" )
          self.failUnless( key.UpdateRule == RESTRICT )
          self.failUnless( key.DeleteRule == CASCADE )

          keycolumns = keys.getByName( "ordertab_pkey" ).getColumns()
          self.failUnless( keycolumns.getElementNames() == (u"id",) )

          key = keys.getByName( "ordertab_pkey" )
          self.failUnless( key.Name == "ordertab_pkey" )
          self.failUnless( key.Type == PRIMARY )
          self.failUnless( key.UpdateRule == NO_ACTION )
          self.failUnless( key.DeleteRule == NO_ACTION )

          keys = tables.getByName( "public.customer" ).getKeys()
          key = keys.getByName( "customer_dummyserial_key" )
          self.failUnless( key.Name == "customer_dummyserial_key" )
          self.failUnless( key.Type == UNIQUE )
          self.failUnless( key.UpdateRule == NO_ACTION )
          self.failUnless( key.DeleteRule == NO_ACTION )

          keys = tables.getByName( "public.orderpos" ).getKeys()
          keyEnum = keys.createEnumeration()
          while keyEnum.hasMoreElements():
              key = keyEnum.nextElement()
              cols = key.getColumns()
              colEnum = cols.createEnumeration()
              while colEnum.hasMoreElements():
                  col = colEnum.nextElement()

      def testViews( self ):
          dd = self.driver.getDataDefinitionByConnection( self.connection )
          views = dd.getViews()

          v = views.getByName( "public.customer2" )
          self.failUnless( v.Name == "customer2" )
          self.failUnless( v.SchemaName == "public" )
          self.failUnless( v.Command != "" )

      def testIndexes( self ):
          dd = self.driver.getDataDefinitionByConnection( self.connection )
          tables = dd.getTables()
          t = tables.getByName( "public.ordertab" )
          indexes = t.getIndexes()
          index = indexes.getByName( "ordertab_pkey" )

          self.failUnless( index.Name == "ordertab_pkey" )
          self.failUnless( index.IsPrimaryKeyIndex )
          self.failUnless( index.IsUnique )
          self.failUnless( not index.IsClustered )

          columns = index.getColumns()
          self.failUnless( columns.hasByName( "id" ) )

          self.failUnless( columns.getByIndex(0).Name == "id" )

      def checkRenameTable( self, t , tables):
          t.rename( "foo" )
          self.failUnless( tables.hasByName( "public.foo" ) )

          t.rename( "public.foo2" )
          self.failUnless( tables.hasByName( "public.foo2" ) )

          try:
              t.rename( "pqsdbc_test.foo2" )
              self.failUnless( tables.hasByName( "pqsdbc_test.foo2" ) )
              print "looks like a server 8.1 or later (changing a schema succeeded)"
              t.rename( "pqsdbc_test.foo" )
              self.failUnless( tables.hasByName( "pqsdbc_test.foo" ) )
              t.rename( "public.foo2" )
              self.failUnless( tables.hasByName( "public.foo2" ) )
          except SQLException,e:
              if e.Message.find( "support changing" ) >= 0:
                  print "looks like a server prior to 8.1 (changing schema failed with Message [" + e.Message.replace("\n", " ") + "])"
              else:
                  raise e
          tables.dropByName( "public.foo2" )

      def testTables( self ):
          dd = self.driver.getDataDefinitionByConnection( self.connection )
          tables = dd.getTables()
          t = tables.getByName( "public.customer" )
          self.failUnless( t.Name == "customer" )
          self.failUnless( t.SchemaName == "public" )
          self.failUnless( t.Type == "TABLE" )

          cols = t.getColumns()
          self.failUnless( cols.hasByName( 'name' ) )
          self.failUnless( cols.hasByName( 'id' ) )
          col = cols.getByName( "dummyserial" )
#          dumpColumns( cols )
          self.checkDescriptor( cols.getByName( "id" ), "id", "bpchar", CHAR, 8, 0, "", "unique id" )
          self.checkDescriptor( cols.getByName( "name" ), "name", "text", VARCHAR, 0, 0, "", "" )

          dd = cols.createDataDescriptor()
          dd.Name = "foo"
          dd.TypeName = "CHAR"
          dd.Type = CHAR
          dd.Precision = 25
          dd.IsNullable = NULLABLE
          cols.appendByDescriptor( dd )

          dd.Name = "foo2"
          dd.TypeName = "DECIMAL"
          dd.Type = DECIMAL
          dd.Precision = 12
          dd.Scale = 5
          dd.DefaultValue = "2.3423"
          dd.Description = "foo2 description"
          cols.appendByDescriptor( dd )

          dd.Name = "cash"
          dd.TypeName = "MONEY"
          dd.Type = DOUBLE
#          dd.IsNullable = NO_NULLS
          dd.DefaultValue = "'2.42'"
          cols.appendByDescriptor( dd )

          cols.refresh()

          self.checkDescriptor( cols.getByName( "foo"), "foo", "bpchar", CHAR, 25,0,"","")
          self.checkDescriptor(
              cols.getByName( "foo2"), "foo2", "numeric", NUMERIC, 12,5,"2.3423","foo2 description")
#          dumpColumns( cols )

          datadesc = tables.createDataDescriptor()
          datadesc.SchemaName = "public"
          datadesc.Name = "blub"
          datadesc.Description = "This describes blub"

          tables.appendByDescriptor( datadesc )

          # make the appended descriptors known
          tables.refresh()

          t = tables.getByName( "public.blub" )
          self.failUnless( t.Name == "blub" )
          self.failUnless( t.SchemaName == "public" )
          self.failUnless( t.Description == "This describes blub" )

          cols = t.getColumns()
          dd = cols.createDataDescriptor()
          dd.Name = "mytext"
          dd.TypeName = "text"
          dd.Type = VARCHAR
          dd.IsNullable = NO_NULLS
          cols.appendByDescriptor( dd )

          cols.refresh()

          dd.DefaultValue = "'myDefault'"
          dd.Name = "mytext2"
          dd.IsNullable = NULLABLE
          dd.Description = "mytext-Description"
          t.alterColumnByName( "mytext" , dd )

          cols.refresh()

          self.checkDescriptor( cols.getByName( "mytext2" ), "mytext2", "text", VARCHAR, 0,0,"'myDefault'","mytext-Description" )

          t = tables.getByName( "public.customer2" )
          self.checkRenameTable( t,tables )

          t = tables.getByName( "public.blub" )
          self.checkRenameTable( t,tables )



      def testUsers( self ):
          dd = self.driver.getDataDefinitionByConnection( self.connection )
          users = dd.getUsers()
          self.failUnless( "pqsdbc_joe" in users.getElementNames() )
