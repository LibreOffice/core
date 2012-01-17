#**************************************************************
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
#**************************************************************
import uno
import unohelper
import importer
import unittest
import core
import impl
import os
import sys

ctx = uno.getComponentContext()
# needed for the tests 
unohelper.addComponentsToContext(ctx,ctx,(os.environ["FOO"]+"/cppobj.uno",os.environ["FOO"]+"/bridgetest.uno","streams.uno","bootstrap.uno"),"com.sun.star.loader.SharedLibrary")

unohelper.addComponentsToContext(ctx,ctx,("vnd.openoffice.pymodule:samplecomponent",),"com.sun.star.loader.Python")

runner = unittest.TextTestRunner(sys.stderr,1,2)

suite = unittest.TestSuite()
suite.addTest(importer.suite(ctx))
suite.addTest(core.suite(ctx))
suite.addTest(impl.suite(ctx))

runner.run(suite)
