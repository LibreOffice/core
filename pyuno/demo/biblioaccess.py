# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************

import uno

from com.sun.star.sdb.CommandType import COMMAND

def main():

    connectionString = "socket,host=localhost,port=2002"

    url = "uno:"+connectionString + ";urp;StarOffice.ComponentContext"

    localCtx = uno.getComponentContext()
    localSmgr = localCtx.ServiceManager
    resolver = localSmgr.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", localCtx)
    ctx = resolver.resolve( url )
    smgr = ctx.ServiceManager

    rowset =smgr.createInstanceWithContext( "com.sun.star.sdb.RowSet", ctx )
    rowset.DataSourceName = "Bibliography"
    rowset.CommandType = COMMAND
    rowset.Command = "SELECT IDENTIFIER, AUTHOR FROM biblio"

    rowset.execute();

    print("Identifier\tAuthor")

    id = rowset.findColumn( "IDENTIFIER" )
    author = rowset.findColumn( "AUTHOR" )
    while rowset.next():
        print(rowset.getString( id ) + "\t" + repr( rowset.getString( author ) ))


    rowset.dispose();

main()
