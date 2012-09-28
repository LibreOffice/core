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

$(eval $(call gb_Module_Module,svtools))

$(eval $(call gb_Module_add_targets,svtools,\
    AllLangResTarget_svt \
    Library_hatchwindowfactory \
    Library_svt \
    Package_inc \
))

ifneq ($(CROSS_COMPILING),YES)
$(eval $(call gb_Module_add_targets,svtools,\
    Executable_bmp \
    Executable_bmpsum \
    Executable_g2g \
))

ifeq ($(GUI),UNX)
$(eval $(call gb_Module_add_targets,svtools,\
    Executable_langsupport \
))
endif

endif

ifneq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_Module_add_check_targets,svtools,\
	CppunitTest_svtools_filters_test \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,svtools,\
    JunitTest_svtools_unoapi \
))

#todo: javapatchres
#todo: jpeg on mac in svtools/util/makefile.mk
#todo: deliver errtxt.src as ehdl.srs
#todo: nooptfiles filter, filterconfigitem, FilterConfigCache, SvFilterOptionsDialog
#todo: map file

# vim: set noet sw=4 ts=4:
