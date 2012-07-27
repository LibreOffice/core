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
import unohelper

from com.sun.star.task import XJobExecutor

# implement a UNO component by deriving from the standard unohelper.Base class
# and from the interface(s) you want to implement.
class HelloWorldJob( unohelper.Base, XJobExecutor ):
    def __init__( self, ctx ):
        # store the component context for later use
        self.ctx = ctx

    def trigger( self, args ):
        # note: args[0] == "HelloWorld", see below config settings

        # retrieve the desktop object
        desktop = self.ctx.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", self.ctx )

        # get current document model
        model = desktop.getCurrentComponent()

        # access the document's text property
        text = model.Text

        # create a cursor
        cursor = text.createTextCursor()

        # insert the text into the document
        text.insertString( cursor, "Hello World", 0 )

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()

#
g_ImplementationHelper.addImplementation( \
        HelloWorldJob,                        # UNO object class
        "org.openoffice.comp.pyuno.demo.HelloWorld", # implemenation name
        ("com.sun.star.task.Job",),)          # list of implemented services
                                              # (the only service)
