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

$(eval $(call gb_CustomTarget_CustomTarget,salhelper/allheaders))

salhelper_allheaders_DIR := $(call gb_CustomTarget_get_workdir,salhelper/allheaders)

$(call gb_CustomTarget_get_target,salhelper/allheaders) : \
	$(salhelper_allheaders_DIR)/salhelper_allheaders.hxx

# dependency on Package_inc.mk should ensure this is updated whenever a new public header is added
$(salhelper_allheaders_DIR)/salhelper_allheaders.hxx : \
            $(SRCDIR)/salhelper/CustomTarget_salhelper_allheaders.mk \
            | $(salhelper_allheaders_DIR)/.dir $(SRCDIR)/salhelper/Package_inc.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	printf '// Generated list of all salhelper/ includes\n' >  $@
	$(foreach file, $(wildcard $(SRCDIR)/salhelper/inc/*.h) $(wildcard $(SRCDIR)/salhelper/inc/*.hxx) \
	                $(wildcard $(SRCDIR)/salhelper/inc/*/*.h) $(wildcard $(SRCDIR)/salhelper/inc/*/*.hxx) \
	                $(wildcard $(SRCDIR)/salhelper/inc/*/*/*.h) $(wildcard $(SRCDIR)/salhelper/inc/*/*/*.hxx) \
	                $(wildcard $(SRCDIR)/salhelper/inc/*/*/*/*.h) $(wildcard $(SRCDIR)/salhelper/inc/*/*/*/*.hxx), \
	    $(if $(findstring /win32/, $(file)), printf '#ifdef _WIN32\n' >> $@ &&) \
	    printf '#include <%s>\n' $(subst $(SRCDIR)/salhelper/inc/,,$(file)) >> $@ && \
	    $(if $(findstring /win32/, $(file)), printf '#endif // _WIN32\n' >> $@ &&) \
	    ) :

# vim: set noet sw=4 ts=4:
