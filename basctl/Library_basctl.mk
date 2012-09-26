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

$(eval $(call gb_Library_Library,basctl))

$(eval $(call gb_Library_set_componentfile,basctl,basctl/util/basctl))

$(eval $(call gb_Library_set_include,basctl,\
	-I$(SRCDIR)/basctl/inc \
	-I$(SRCDIR)/basctl/sdi \
	-I$(SRCDIR)/basctl/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/SdiTarget/basctl/sdi \
))

$(eval $(call gb_Library_use_sdk_api,basctl))

$(eval $(call gb_Library_use_libraries,basctl,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	fwe \
	sal \
	sb \
	sfx \
	sot \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	utl \
	ucbhelper \
	vcl \
	xmlscript \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,basctl))

$(eval $(call gb_Library_add_exception_objects,basctl,\
	basctl/source/accessibility/accessibledialogcontrolshape \
	basctl/source/accessibility/accessibledialogwindow \
	basctl/source/basicide/basdoc \
	basctl/source/basicide/basicbox \
	basctl/source/basicide/basicrenderable \
	basctl/source/basicide/baside2b \
	basctl/source/basicide/baside2 \
	basctl/source/basicide/baside3 \
	basctl/source/basicide/basidectrlr \
	basctl/source/basicide/basides1 \
	basctl/source/basicide/basides2 \
	basctl/source/basicide/basides3 \
	basctl/source/basicide/basidesh \
	basctl/source/basicide/basobj2 \
	basctl/source/basicide/basobj3 \
	basctl/source/basicide/bastype2 \
	basctl/source/basicide/bastype3 \
	basctl/source/basicide/bastypes \
	basctl/source/basicide/breakpoint \
	basctl/source/basicide/brkdlg \
	basctl/source/basicide/doceventnotifier \
	basctl/source/basicide/docsignature \
	basctl/source/basicide/documentenumeration \
	basctl/source/basicide/iderdll \
	basctl/source/basicide/layout \
	basctl/source/basicide/linenumberwindow \
	basctl/source/basicide/localizationmgr \
	basctl/source/basicide/macrodlg \
	basctl/source/basicide/moduldl2 \
	basctl/source/basicide/moduldlg \
	basctl/source/basicide/objdlg \
	basctl/source/basicide/register \
	basctl/source/basicide/sbxitem \
	basctl/source/basicide/scriptdocument \
	basctl/source/basicide/tbxctl \
	basctl/source/basicide/unomodel \
	basctl/source/dlged/dlgedclip \
	basctl/source/dlged/dlged \
	basctl/source/dlged/dlgedfac \
	basctl/source/dlged/dlgedfunc \
	basctl/source/dlged/dlgedlist \
	basctl/source/dlged/dlgedmod \
	basctl/source/dlged/dlgedobj \
	basctl/source/dlged/dlgedpage \
	basctl/source/dlged/dlgedview \
	basctl/source/dlged/managelang \
	basctl/source/dlged/propbrw \
))

$(eval $(call gb_Library_add_sdi_headers,basctl,basctl/sdi/basslots))

$(eval $(call gb_SdiTarget_SdiTarget,basctl/sdi/basslots,basctl/sdi/baside))

$(eval $(call gb_SdiTarget_set_include,basctl/sdi/basslots,\
	$$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_libraries,basctl,\
	advapi32 \
	gdi32 \
	shell32 \
))
endif

# vim: set noet sw=4 ts=4:
