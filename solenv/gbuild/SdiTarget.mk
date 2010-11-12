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


# SdiTarget class

gb_SdiTarget_SVIDLTARGET := $(call gb_Executable_get_target,svidl)
gb_SdiTarget_SVIDLAUXDEPS := $(call gb_Library_get_target,tl) $(call gb_Library_get_target,sal)
gb_SdiTarget_SVIDLCOMMAND := $(gb_SdiTarget_SVIDLPRECOMMAND) $(gb_SdiTarget_SVIDLTARGET)

$(call gb_SdiTarget_get_target,%) : $(SRCDIR)/%.sdi | $(gb_SdiTarget_SVIDLTARGET) $(gb_SdiTarget_SVIDLAUXDEPS)
    $(call gb_Helper_announce,$*,$(true),SDI,1)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $(WORKDIR)/inc/$*) $(dir $@))
    $(call gb_Helper_abbreviate_dirs_native,\
        cd $(dir $<) && \
        $(gb_SdiTarget_SVIDLCOMMAND) -quiet \
            $(INCLUDE) \
            -fs$@.hxx \
            -fd$@.ilb \
            -fl$@.lst \
            -fz$@.sid \
            -fx$(EXPORTS) \
            -fm$@ \
            $<)

.PHONY : $(call gb_SdiTarget_get_clean_target,%)
$(call gb_SdiTarget_get_clean_target,%) :
    $(call gb_Helper_announce,$*,$(false),SDI,1)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(foreach ext,.hxx .ilb .lst .sid,\
            $(call gb_SdiTarget_get_target,$*)$(ext)) \
            $(call gb_SdiTarget_get_target,$*))

define gb_SdiTarget_SdiTarget
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := -I$(WORKDIR)/inc/ $$(subst -I. ,-I$$(dir $(SRCDIR)/$(1)) ,$$(SOLARINC))
$(call gb_SdiTarget_get_target,$(1)) : EXPORTS := $(SRCDIR)/$(2).sdi
endef

define gb_SdiTarget_set_include
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

# vim: set noet sw=4 ts=4:
