#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: pkg_config.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:08:23 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

.INCLUDE .IGNORE : pkgroot.mk

.IF "$(PKGCONFIG_ROOT)"!=""
PKG_CONFIG=$(PKGCONFIG_ROOT)$/bin$/pkg-config
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)$/lib$/pkgconfig
.EXPORT : PKG_CONFIG_PATH
PKGCONFIG_PREFIX=--define-variable=prefix=$(PKGCONFIG_ROOT)
.ELSE           # "$(OS)"=="SOLARIS"
PKG_CONFIG*=pkg-config
.ENDIF          # "$(OS)"=="SOLARIS"

PKGCONFIG_CFLAGS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --cflags $(PKGCONFIG_MODULES))
PKGCONFIG_LIBS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --libs $(PKGCONFIG_MODULES))
CFLAGS+=$(PKGCONFIG_CFLAGS)
