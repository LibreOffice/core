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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,rptui))

$(eval $(call gb_AllLangResTarget_set_reslocation,rptui,reportdesign))

$(eval $(call gb_AllLangResTarget_add_srs,rptui,reportdesign/rptui))

$(eval $(call gb_SrsTarget_SrsTarget,reportdesign/rptui))

$(eval $(call gb_SrsTarget_use_packages,reportdesign/rptui,\
	svx_globlmn_hrc \
))

$(eval $(call gb_SrsTarget_set_include,reportdesign/rptui,\
	$$(INCLUDE) \
	-I$(SRCDIR)/reportdesign/source/ui/inc \
	-I$(SRCDIR)/reportdesign/inc \
))


$(eval $(call gb_SrsTarget_add_files,reportdesign/rptui,\
	reportdesign/source/ui/dlg/dlgpage.src	\
	reportdesign/source/ui/dlg/PageNumber.src	\
	reportdesign/source/ui/dlg/DateTime.src	\
	reportdesign/source/ui/dlg/CondFormat.src	\
	reportdesign/source/ui/dlg/Navigator.src	\
	reportdesign/source/ui/dlg/GroupsSorting.src \
	reportdesign/source/ui/inspection/inspection.src \
	reportdesign/source/ui/report/report.src \
))

# vim: set noet sw=4 ts=4:
