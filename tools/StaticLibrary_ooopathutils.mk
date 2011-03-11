#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_StaticLibrary_StaticLibrary,ooopathutils))

$(eval $(call gb_StaticLibrary_add_package_headers,ooopathutils,tools_inc))

$(eval $(call gb_StaticLibrary_add_exception_objects,ooopathutils,\
    tools/source/misc/pathutils \
))


# HACK for now
# We really should fix the clients of this to link against the static library
# Instead of this evil linking of an object from $(OUTDIR)
define StaticLibrary_ooopathutils_hack
$(call gb_StaticLibrary_get_target,ooopathutils) : $(OUTDIR)/lib/$(1)
$$(eval $$(call gb_Deliver_add_deliverable,$(OUTDIR)/lib/$(1),$(call gb_CxxObject_get_target,tools/source/misc/pathutils)))

$(OUTDIR)/lib/$(1) : $(call gb_CxxObject_get_target,tools/source/misc/pathutils)
	$$(call gb_Deliver_deliver,$$<,$$@)

endef

ifeq ($(OS),WNT)
$(eval $(call StaticLibrary_ooopathutils_hack,pathutils-obj.obj))
else
$(eval $(call StaticLibrary_ooopathutils_hack,pathutils-obj.o))
endif
# vim: set noet sw=4 ts=4:
