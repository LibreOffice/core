#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

$(eval $(call gb_JunitTest_JunitTest,toolkit_complex))

$(eval $(call gb_JunitTest_add_jars,toolkit_complex,\
    $(OUTDIR)/bin/OOoRunner.jar \
    $(OUTDIR)/bin/ridl.jar \
    $(OUTDIR)/bin/test.jar \
    $(OUTDIR)/bin/unoil.jar \
))

$(eval $(call gb_JunitTest_add_sourcefiles,toolkit_complex,\
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleComponent \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleContext \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleEventBroadcaster \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleExtendedComponent \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleText \
    toolkit/qa/complex/toolkit/Assert \
    toolkit/qa/complex/toolkit/awtgrid/GridDataListener \
    toolkit/qa/complex/toolkit/awtgrid/TMutableGridDataModel \
    toolkit/qa/complex/toolkit/awtgrid/DummyColumn \
    toolkit/qa/complex/toolkit/GridControl \
    toolkit/qa/complex/toolkit/UnitConversion \
))

$(eval $(call gb_JunitTest_add_classes,toolkit_complex,\
    complex.toolkit.GridControl \
    complex.toolkit.UnitConversion \
))

# vim: set noet sw=4 ts=4:
