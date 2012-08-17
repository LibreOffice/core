# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_JunitTest_JunitTest,writerfilter_complex))

$(eval $(call gb_JunitTest_set_defs,writerfilter_complex,\
	$$(DEFS) \
	-Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/writerfilter/qa/complex/ooxml/testdocuments \
))

$(eval $(call gb_JunitTest_use_jars,writerfilter_complex,\
	OOoRunner \
	ridl \
	test \
	unoil \
	jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,writerfilter_complex,\
	writerfilter/qa/complex/ooxml/TestDocument \
	writerfilter/qa/complex/ooxml/LoadDocuments \
))

# #i113098# currently fails in non-pro
ifeq ($(gb_PRODUCT),$(true))
$(eval $(call gb_JunitTest_add_classes,writerfilter_complex,\
	complex.ooxml.LoadDocuments \
))
endif

# vim: set noet sw=4 ts=4:
