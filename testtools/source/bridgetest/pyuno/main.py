#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: main.py,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 12:26:52 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
import uno
import unohelper
import importer
import unittest
import core
import impl
import sys

ctx = uno.getComponentContext()
# needed for the tests 
unohelper.addComponentsToContext(ctx,ctx,("cppobj.uno","bridgetest.uno","streams.uno","servicemgr.uno"),"com.sun.star.loader.SharedLibrary")

unohelper.addComponentsToContext(ctx,ctx,("vnd.openoffice.pymodule:samplecomponent",),"com.sun.star.loader.Python")

runner = unittest.TextTestRunner(sys.stderr,1,2)

suite = unittest.TestSuite()
suite.addTest(importer.suite(ctx))
suite.addTest(core.suite(ctx))
suite.addTest(impl.suite(ctx))

runner.run(suite)
