# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

# the .dir is for make 3.81, which ignores trailing /
$(call gb_CustomTarget_get_workdir,%)/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CustomTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),CUS,3)
	touch $@

.PHONY: $(call gb_CustomTarget_get_clean_target,%)
$(call gb_CustomTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CUS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CustomTarget_get_workdir,$*) && \
		rm -f $(call gb_CustomTarget_get_target,$*))

define gb_CustomTarget_CustomTarget
$(eval $(call gb_Module_register_target,$(call gb_CustomTarget_get_target,$(1)),$(call gb_CustomTarget_get_clean_target,$(1))))
$(call gb_CustomTarget_get_target,$(1)) :| $(dir $(call gb_CustomTarget_get_target,$(1))).dir

endef

# vim: set noet sw=4:
