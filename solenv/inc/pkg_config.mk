#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: pkg_config.mk,v $
#
# $Revision: 1.7 $
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

.INCLUDE .IGNORE : pkgroot.mk

.IF "$(PKGCONFIG_ROOT)"!=""
PKG_CONFIG=$(PKGCONFIG_ROOT)/bin/pkg-config
.IF "$(OS)"=="SOLARIS" && "$(CPUNAME)"=="SPARC" && "$(CPU)"=="U"
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)/lib/64/pkgconfig
.ELSE
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)/lib/pkgconfig
.ENDIF
.EXPORT : PKG_CONFIG_PATH
PKGCONFIG_PREFIX=--define-variable=prefix=$(PKGCONFIG_ROOT)
.ELSE
PKG_CONFIG*=pkg-config
.IF "$(OS)"=="SOLARIS" && "$(CPUNAME)"=="SPARC" && "$(CPU)"=="U"
PKG_CONFIG_PATH=/usr/lib/64/pkgconfig
.EXPORT : PKG_CONFIG_PATH
.ENDIF
.ENDIF

PKGCONFIG_CFLAGS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --cflags $(PKGCONFIG_MODULES))
PKGCONFIG_LIBS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --libs $(PKGCONFIG_MODULES))
CFLAGS+=$(PKGCONFIG_CFLAGS)
