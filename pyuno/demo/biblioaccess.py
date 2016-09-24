# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

import uno
from com.sun.star.sdb.CommandType import COMMAND

def main():
    connectionString = "socket,host=localhost,port=2002"

    url = "uno:" + connectionString + ";urp;StarOffice.ComponentContext"

    localCtx = uno.getComponentContext()
    localSmgr = localCtx.ServiceManager
    resolver = localSmgr.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", localCtx)
    ctx = resolver.resolve(url)
    smgr = ctx.ServiceManager

    rowset =smgr.createInstanceWithContext("com.sun.star.sdb.RowSet", ctx)
    rowset.DataSourceName = "Bibliography"
    rowset.CommandType = COMMAND
    rowset.Command = "SELECT IDENTIFIER, AUTHOR FROM biblio"

    rowset.execute();

    print("Identifier\tAuthor")

    id = rowset.findColumn("IDENTIFIER")
    author = rowset.findColumn("AUTHOR")
    while rowset.next():
        print(rowset.getString(id) + "\t" + repr(rowset.getString(author)))

    rowset.dispose();

main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
