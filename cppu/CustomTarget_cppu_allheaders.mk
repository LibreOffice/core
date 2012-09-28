# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CustomTarget_CustomTarget,cppu/allheaders))

cppu_allheaders_DIR := $(call gb_CustomTarget_get_workdir,cppu/allheaders)

$(call gb_CustomTarget_get_target,cppu/allheaders) : \
	$(cppu_allheaders_DIR)/cppu_allheaders.hxx

# dependency on Package_inc.mk should ensure this is updated whenever a new public header is added
$(cppu_allheaders_DIR)/cppu_allheaders.hxx : $(SRCDIR)/cppu/CustomTarget_cppu_allheaders.mk \
            | $(cppu_allheaders_DIR)/.dir $(SRCDIR)/cppu/Package_inc.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	printf '// Generated list of all cppu/ includes\n' >  $@
	$(foreach file, $(wildcard $(SRCDIR)/cppu/inc/*.h) $(wildcard $(SRCDIR)/cppu/inc/*.hxx) \
	                $(wildcard $(SRCDIR)/cppu/inc/*/*.h) $(wildcard $(SRCDIR)/cppu/inc/*/*.hxx) \
	                $(wildcard $(SRCDIR)/cppu/inc/*/*/*.h) $(wildcard $(SRCDIR)/cppu/inc/*/*/*.hxx) \
	                $(wildcard $(SRCDIR)/cppu/inc/*/*/*/*.h) $(wildcard $(SRCDIR)/cppu/inc/*/*/*/*.hxx), \
	    $(if $(findstring /win32/, $(file)), printf '#ifdef WNT\n' >> $@ &&) \
	    printf '#include <%s>\n' $(subst $(SRCDIR)/cppu/inc/,,$(file)) >> $@ && \
	    $(if $(findstring /win32/, $(file)), printf '#endif // WNT\n' >> $@ &&) \
	    ) :

# vim: set noet sw=4 ts=4:
