# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,external/stdlibs))

$(eval $(call gb_CustomTarget_register_targets,external/stdlibs,\
	libgcc_s.so.$(gb_SHORTSTDC3) \
	libstdc++.so.$(gb_SHORTSTDCPP3) \
))

$(call gb_CustomTarget_get_workdir,external/stdlibs)/lib%:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),STL,1)
	$(PERL) -w $(SOLARENV)/bin/gccinstlib.pl $(notdir $@) $(dir $@)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
