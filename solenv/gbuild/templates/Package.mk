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

# MODULE is the name of the module the makefile is located in

# TYPE is an arbitrary name that should denote what is inside this package
# typical names are "inc", "uiconfig", "xml" etc.
# "MODULE_TYPE" is the package name
# the file name of the corresponding makefile should be Package_TYPE.mk
# SOURCE is the name of the root folder of the package that shall be delivered
$(eval $(call gb_Package_Package,MODULE_TYPE,$(SRCDIR)/MODULE/SOURCE))

# add files to the package
# SOURCE_RELPATH/file.ext is the name of the source file (relative to the root give above)
# DESTINATION_RELPATH/file.ext is the destination file (relative to $(OUTDIR))
$(eval $(call gb_Package_add_file,MODULE_TYPE,DESTINATION_RELPATH/file.ext,SOURCE_RELPATH/file.ext))

# vim: set noet sw=4 ts=4:
