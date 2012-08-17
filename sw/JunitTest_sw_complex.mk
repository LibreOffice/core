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

$(eval $(call gb_JunitTest_JunitTest,sw_complex))

$(eval $(call gb_JunitTest_set_defs,sw_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sw/qa/complex/writer/testdocuments \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sw_complex,\
    sw/qa/complex/accessibility/AccessibleRelationSet \
    sw/qa/complex/checkColor/CheckChangeColor \
    sw/qa/complex/indeterminateState/CheckIndeterminateState \
    sw/qa/complex/writer/CheckBookmarks \
    sw/qa/complex/writer/CheckCrossReferences \
    sw/qa/complex/writer/CheckFlies \
    sw/qa/complex/writer/CheckFields \
    sw/qa/complex/writer/CheckIndexedPropertyValues \
    sw/qa/complex/writer/CheckNamedPropertyValues \
    sw/qa/complex/writer/LoadSaveTest \
    sw/qa/complex/writer/TestDocument \
    sw/qa/complex/writer/TextPortionEnumerationTest \
))

$(eval $(call gb_JunitTest_use_jars,sw_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_classes,sw_complex,\
    complex.accessibility.AccessibleRelationSet \
    complex.checkColor.CheckChangeColor \
    complex.writer.CheckCrossReferences \
    complex.writer.CheckFields\
    complex.writer.CheckFlies \
    complex.writer.CheckIndexedPropertyValues \
    complex.writer.CheckNamedPropertyValues \
    complex.writer.TextPortionEnumerationTest \
))

# FIXME has never worked on windows, hashes are different
ifneq ($(OS),WNT)
$(eval $(call gb_JunitTest_add_classes,sw_complex,\
    complex.writer.CheckBookmarks \
))
endif

# vim: set noet sw=4 ts=4:
