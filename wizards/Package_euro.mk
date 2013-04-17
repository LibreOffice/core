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

$(eval $(call gb_Package_Package,wizards_basicsrveuro,$(SRCDIR)/wizards/source/euro))

$(eval $(call gb_Package_set_outdir,wizards_basicsrveuro,$(INSTDIR)))

$(eval $(call gb_Package_add_files,wizards_basicsrveuro,share/basic/Euro,\
	AutoPilotRun.xba \
	Common.xba \
	ConvertRun.xba \
	dialog.xlb \
	DlgConvert.xdl \
	DlgPassword.xdl \
	Hard.xba \
	Init.xba \
	Protect.xba \
	script.xlb \
	Soft.xba \
	Writer.xba \
))

# vim: set noet sw=4 ts=4:
