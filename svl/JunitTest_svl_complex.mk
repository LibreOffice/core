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

$(eval $(call gb_JunitTest_JunitTest,svl_complex))

$(eval $(call gb_JunitTest_set_defs,svl_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/svl/qa/complex/broken_document/test_documents \
))

$(eval $(call gb_JunitTest_add_jars,svl_complex,\
    $(OUTDIR)/bin/OOoRunner.jar \
    $(OUTDIR)/bin/ridl.jar \
    $(OUTDIR)/bin/test.jar \
    $(OUTDIR)/bin/unoil.jar \
    $(OUTDIR)/bin/jurt.jar \
    $(OUTDIR)/bin/ConnectivityTools.jar \
))

$(eval $(call gb_JunitTest_add_sourcefiles,svl_complex,\
    svl/qa/complex/ConfigItems/CheckConfigItems \
    svl/qa/complex/passwordcontainer/PasswordContainerUnitTest \
    svl/qa/complex/passwordcontainer/TestHelper \
    svl/qa/complex/passwordcontainer/Test03 \
    svl/qa/complex/passwordcontainer/Test02 \
    svl/qa/complex/passwordcontainer/Test01 \
    svl/qa/complex/passwordcontainer/PasswordContainerTest \
    svl/qa/complex/passwordcontainer/MasterPasswdHandler \
))

$(eval $(call gb_JunitTest_add_classes,svl_complex,\
    complex.passwordcontainer.PasswordContainerUnitTest \
))

# vim: set noet sw=4 ts=4:
