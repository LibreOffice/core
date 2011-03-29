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

$(eval $(call gb_StaticLibrary_StaticLibrary,vclmain))

$(eval $(call gb_StaticLibrary_set_include,vclmain,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,vclmain,\
    vcl/source/salmain/salmain \
))

# HACK for now
# We really should fix the clients of this to link against the static library
# Instead of this evil linking of an object from $(OUTDIR)
define StaticLibrary_salmain_hack
$(call gb_StaticLibrary_get_target,vclmain) : $(OUTDIR)/lib/$(1)
$$(eval $$(call gb_Deliver_add_deliverable,$(OUTDIR)/lib/$(1),$(call gb_CxxObject_get_target,vcl/source/salmain/salmain)))

$(OUTDIR)/lib/$(1) : $(call gb_CxxObject_get_target,vcl/source/salmain/salmain)
    $$(call gb_Deliver_deliver,$$<,$$@)

endef

ifeq ($(OS),WNT)
$(eval $(call StaticLibrary_salmain_hack,salmain.obj))
else
$(eval $(call StaticLibrary_salmain_hack,salmain.o))
endif

# vim: set noet sw=4 ts=4:
