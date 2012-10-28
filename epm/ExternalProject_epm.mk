# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,epm))

$(eval $(call gb_ExternalProject_use_unpacked,epm,epm))

$(eval $(call gb_ExternalProject_register_targets,epm,\
	build \
))

$(call gb_ExternalProject_get_state_target,epm,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure --disable-fltk \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________NONE) \
	&& $(GNUMAKE) \
	&& touch $@

# vim: set noet sw=4 ts=4:
