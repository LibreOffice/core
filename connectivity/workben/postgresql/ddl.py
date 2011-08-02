#*************************************************************************
#
#   $RCSfile: ddl.py,v $
#
#   $Revision: 1.1.2.5 $
#
#   last change: $Author: jbu $ $Date: 2007/01/07 13:50:38 $
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
from com.sun.star.sdbc import SQLException
import sys

def dumpResultSet( rs  ):
    meta = rs.getMetaData()
    for i in range(1, meta.getColumnCount()+1):
        sys.stdout.write(meta.getColumnName( i ) + "\t")
    sys.stdout.write( "\n" )
    while rs.next():
        for i in range( 1, meta.getColumnCount()+1):
            sys.stdout.write( rs.getString( i ) + "\t" )
        sys.stdout.write( "\n" )
    rs.beforeFirst()



def executeIgnoringException( stmt, sql ):
    try:
        stmt.executeUpdate(sql)
    except SQLException:
        pass

def cleanGroupsAndUsers( stmt ):
    rs = stmt.executeQuery("SELECT groname FROM pg_group WHERE groname LIKE 'pqsdbc_%'" )
    stmt2 = stmt.getConnection().createStatement()
    while rs.next():
        stmt2.executeUpdate("DROP GROUP " + rs.getString(1) )

    rs.close()
    rs = stmt.executeQuery( "SELECT usename FROM pg_user WHERE usename LIKE 'pqsdbc_%'" )
    while rs.next():
        stmt2.executeUpdate( "DROP USER " + rs.getString(1) )




def executeDDLs( connection ):

    stmt = connection.createStatement()


    executeIgnoringException( stmt, "DROP VIEW customer2" )
    executeIgnoringException( stmt, "DROP TABLE orderpos" )
    executeIgnoringException( stmt, "DROP TABLE ordertab" )
    executeIgnoringException( stmt, "DROP TABLE product" )
    executeIgnoringException( stmt, "DROP TABLE customer" )
    executeIgnoringException( stmt, "DROP TABLE blub" )
    executeIgnoringException( stmt, "DROP TABLE foo" )
    executeIgnoringException( stmt, "DROP TABLE nooid" )
    executeIgnoringException( stmt, "DROP TABLE nooid2" )
    cleanGroupsAndUsers( stmt )
    executeIgnoringException( stmt, "DROP DOMAIN pqsdbc_short" )
    executeIgnoringException( stmt, "DROP DOMAIN pqsdbc_amount" )
    executeIgnoringException( stmt, "DROP SCHEMA pqsdbc_test" )

    ddls = (
        "BEGIN",
    "CREATE DOMAIN pqsdbc_short AS int2",
        "CREATE DOMAIN pqsdbc_amount AS integer",
        "CREATE USER pqsdbc_joe",
        "CREATE USER pqsdbc_susy",
        "CREATE USER pqsdbc_boss",
        "CREATE USER pqsdbc_customer", # technical user (e.g. a webfrontend)
        "CREATE GROUP pqsdbc_employees WITH USER pqsdbc_joe,pqsdbc_susy",
        "CREATE GROUP pqsdbc_admin WITH USER pqsdbc_susy,pqsdbc_boss",
        "CREATE SCHEMA pqsdbc_test",
        "CREATE TABLE customer ( "+
              "id char(8) UNIQUE PRIMARY KEY, "+
              "name text, " +
              "dummySerial serial UNIQUE) WITH OIDS",
        "COMMENT ON TABLE customer IS 'contains customer attributes'",
        "COMMENT ON COLUMN customer.id IS 'unique id'",
        "CREATE TABLE product ("+
             "id char(8) UNIQUE PRIMARY KEY,"+
             "name text,"+
             "price numeric(10,2),"+
             "image bytea) WITH OIDS",

        "CREATE TABLE ordertab ( "+
             "id char(8) UNIQUE PRIMARY KEY,"+
             "customerid char(8) CONSTRAINT cust REFERENCES customer(id) ON DELETE CASCADE ON UPDATE RESTRICT,"+
             "orderdate char(8),"+
             "delivered boolean ) WITH OIDS",
        "CREATE TABLE orderpos ( "+
             "orderid char(8) REFERENCES ordertab(id),"+
              "id char(3),"+
              "productid char(8) REFERENCES product(id),"+
              "amount pqsdbc_amount,"+
              "shortamount pqsdbc_short,"+
              "PRIMARY KEY (orderid,id)) WITH OIDS",
        "CREATE TABLE nooid ("+
             "id char(8) UNIQUE PRIMARY KEY,"+
             "name text) "+
             "WITHOUT OIDS",
        "CREATE TABLE nooid2 ("+
             "id serial UNIQUE PRIMARY KEY,"+
             "name text) "+
             "WITHOUT OIDS",
        "CREATE VIEW customer2 AS SELECT id,name FROM customer",
        "GRANT SELECT ON TABLE customer,product,orderpos,ordertab TO pqsdbc_customer",
        "GRANT SELECT ON TABLE product TO GROUP pqsdbc_employees",
        "GRANT SELECT,UPDATE, INSERT ON TABLE customer TO GROUP pqsdbc_employees",
        "GRANT ALL ON TABLE orderpos,ordertab TO GROUP pqsdbc_employees, GROUP pqsdbc_admin",
        "GRANT ALL ON TABLE customer TO GROUP pqsdbc_admin",  # the admin is allowed to delete customers
        "GRANT ALL ON TABLE product TO pqsdbc_boss",  # only the boss may change the product table
        "INSERT INTO public.customer VALUES ('C1','John Doe')",
        "INSERT INTO \"public\" . \"customer\" VALUES ('C2','Bruce Springsteen')",

        "INSERT INTO \"public\".product VALUES ('PZZ2','Pizza Mista',6.95,'\\003foo\\005')",
        "INSERT INTO product VALUES ('PZZ5','Pizza Funghi',5.95,'\\001foo\\005')",
        "INSERT INTO product VALUES ('PAS1','Lasagne',5.49,NULL)",

        "INSERT INTO ordertab VALUES ( '1', 'C2', '20030403','true')",
        "INSERT INTO ordertab VALUES ( '2', 'C1', '20030402','false')",

        "INSERT INTO orderpos VALUES ( '1','001', 'PZZ2',2,0)",
        "INSERT INTO orderpos VALUES ( '1','002', 'PZZ5',3,-1)",
        "INSERT INTO orderpos VALUES ( '2','001', 'PAS1',5,1)",
        "INSERT INTO orderpos VALUES ( '2','002', 'PZZ2',3,2)",
        "COMMIT" )
    for i in ddls:
        stmt.executeUpdate(i)

    connection.getTables() # force refresh of metadata

    stmt.close()
