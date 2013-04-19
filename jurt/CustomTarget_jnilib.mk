# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,jurt/util))

$(eval $(call gb_CustomTarget_register_targets,jurt/util,\
	libjpipe.jnilib \
))

# TODO: could this be replaced by defining library jpipe as gb_Library_Bundle?
$(call gb_CustomTarget_get_workdir,jurt/util)/libjpipe.jnilib : $(call gb_Library_get_target,jpipe)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MCB,1)
	cp $< $(dir $@)$(notdir $<) && \
	$(SOLARENV)/bin/macosx-create-bundle $(dir $@)$(notdir $<)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
