# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_JunitTest_JunitTest,linguistic_complex))

$(eval $(call gb_JunitTest_set_defs,linguistic_complex,\
	$$(DEFS) \
	-Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/linguistic/qa/complex/linguistic/testdocuments \
))

$(eval $(call gb_JunitTest_use_jars,linguistic_complex,\
	OOoRunner \
	ridl \
	test \
	test-tools \
	unoil \
	jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,linguistic_complex,\
	linguistic/qa/complex/linguistic/HangulHanjaConversion \
	linguistic/qa/complex/linguistic/TestDocument \
))

$(eval $(call gb_JunitTest_add_classes,linguistic_complex,\
	complex.linguistic.HangulHanjaConversion \
))

# vim: set noet sw=4 ts=4:
