#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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

ifeq ($(strip $(SOLARENV)),)
$(error No environment set)
endif

GBUILDDIR := $(SOLARENV)/gbuild
include $(GBUILDDIR)/gbuild.mk


include $(foreach module,\
	framework \
	sfx2 \
	svl \
	svtools \
	xmloff \
	sw \
	toolkit \
	tools \
,$(SRCDIR)/$(module)/prj/target_module_$(module).mk)

all : $(foreach module,$(gb_Module_ALLMODULES),$(call gb_Module_get_target,$(module)))
	$(call gb_Helper_announce,Completed all modules.)

clean : $(foreach module,$(gb_Module_ALLMODULES),$(call gb_Module_get_clean_target,$(module)))
	$(call gb_Helper_announce,all modules cleaned.)

.DEFAULT_GOAL := all

# vim: set noet sw=4 ts=4:
