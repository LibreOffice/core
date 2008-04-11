#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: productversion.mk,v $
#
# $Revision: 1.27 $
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
.IF "$(OS)"=="LINUX"
PKGREV          = $(BUILD)
.ELSE
PKGREV          = $(LAST_MINOR:s/m//:s/s/./)
.ENDIF

ABSLOCALOUT:=$(shell @cd $(PRJ) && pwd)$/$(ROUT)

# create desktop-integration subfolder on linux
.IF "$(OS)"=="LINUX"
# rpm seems to require absolute paths here :(
PKGDIR:=$(ABSLOCALOUT)$/bin$/desktop-integration
RPMMACROS= \
    --define "_rpmdir $(PKGDIR)" \
    --define "_rpmfilename %%{{NAME}}-%%{{VERSION}}-%%{{RELEASE}}.%%{{ARCH}}.rpm"
.ELSE
PKGDIR=$(BIN)
.ENDIF

PRODUCTLIST = openoffice.org broffice.org

# default values to minimize maintainance effort 
PRODUCTVERSION = 3.0
PKGVERSION = $(PRODUCTVERSION)
# gnome-vfs treats everything behind the last '.' as an icon extension, 
# even though the "icon_filename" in '.keys' is specified as filename 
# without extension. Since it also does not know how to handle "2-writer"
# type icons :-), we are stripping all '.' for now.
# ToDo: maybe we use a product major later ??
ICONVERSION = $(PRODUCTVERSION:s/.//g)
# UNIXWRAPPERNAME variable is used to generate the common desktop files below share/xdg;
# the default values get replaced by make_installer.pl at (core0x) packaging time;
# another wrapper name can be forced by --with-unix-wrapper configure option
# which is need by other distributors, see http://www.openoffice.org/issues/show_bug.cgi?id=75366
UNIXWRAPPERNAME *= '$${{UNIXPRODUCTNAME}}$${{PRODUCTVERSION}}'

PRODUCTNAME.openoffice.org = OpenOffice.org
PRODUCTVERSION.openoffice.org = $(PRODUCTVERSION)
PKGVERSION.openoffice.org = $(PKGVERSION)
UNIXFILENAME.openoffice.org = $(PRODUCTNAME.openoffice.org:l)$(PRODUCTVERSION.openoffice.org)
ICONPREFIX.openoffice.org = $(UNIXFILENAME.openoffice.org:s/.//g)

PRODUCTNAME.broffice.org = BrOffice.org
PRODUCTVERSION.broffice.org = $(PRODUCTVERSION)
PKGVERSION.broffice.org = $(PKGVERSION)
UNIXFILENAME.broffice.org = $(PRODUCTNAME.broffice.org:l)$(PRODUCTVERSION.broffice.org)
ICONPREFIX.broffice.org = $(UNIXFILENAME.broffice.org:s/.//g)


