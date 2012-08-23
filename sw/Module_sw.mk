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

$(eval $(call gb_Module_Module,sw))

$(eval $(call gb_Module_add_targets,sw,\
    AllLangResTarget_sw \
    Library_msword \
    Library_sw \
    Library_swd \
    Library_swui \
    Package_uiconfig \
))

ifneq ($(DISABLE_SCRIPTING),TRUE)

$(eval $(call gb_Module_add_targets,sw,\
    Library_vbaswobj \
))

endif

$(eval $(call gb_Module_add_check_targets,sw,\
    CppunitTest_sw_swdoc_test \
))

$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_filters_test \
    CppunitTest_sw_macros_test \
    CppunitTest_sw_subsequent_ooxmlexport \
    CppunitTest_sw_subsequent_ooxmlimport \
    CppunitTest_sw_subsequent_ww8export \
    CppunitTest_sw_subsequent_ww8import \
    CppunitTest_sw_subsequent_rtfexport \
    CppunitTest_sw_subsequent_rtfimport \
    CppunitTest_sw_subsequent_odfexport \
    CppunitTest_sw_subsequent_odfimport \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,sw,\
    JunitTest_sw_complex \
    JunitTest_sw_unoapi \
))

# vim: set noet sw=4 ts=4:
