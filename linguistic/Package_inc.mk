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

$(eval $(call gb_Package_Package,linguistic_inc,$(SRCDIR)/linguistic/inc))

# add files to the package
# SOURCE_RELPATH/file.ext is the name of the source file (relative to the root give above)
# DESTINATION_RELPATH/file.ext is the destination file (relative to $(OUTDIR))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/lngdllapi.h,linguistic/lngdllapi.h))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/hyphdta.hxx,linguistic/hyphdta.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/lngprophelp.hxx,linguistic/lngprophelp.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/lngprops.hxx,linguistic/lngprops.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/misc.hxx,linguistic/misc.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/spelldta.hxx,linguistic/spelldta.hxx))

# vim: set noet sw=4 ts=4:
