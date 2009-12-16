#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: main.py,v $
#
# $Revision: 1.5 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
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
