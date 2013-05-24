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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,basctl))

$(eval $(call gb_AllLangResTarget_add_srs,basctl,\
	basctl/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,basctl/res))

$(eval $(call gb_SrsTarget_set_include,basctl/res,\
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
	-I$(SRCDIR)/basctl/inc \
	-I$(SRCDIR)/basctl/source/inc \
	-I$(SRCDIR)/basctl/source/basicide \
	-I$(SRCDIR)/basctl/source/dlged \
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,basctl/res,\
	basctl/source/basicide/basicprint.src \
	basctl/source/basicide/basidesh.src \
	basctl/source/basicide/brkdlg.src \
	basctl/source/basicide/macrodlg.src \
	basctl/source/basicide/moduldlg.src \
	basctl/source/basicide/moptions.src \
	basctl/source/basicide/objdlg.src \
	basctl/source/dlged/dlgresid.src \
	basctl/source/dlged/managelang.src \
))

# vim: set noet sw=4 ts=4:
