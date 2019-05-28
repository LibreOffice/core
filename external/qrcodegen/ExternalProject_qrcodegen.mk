# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,qrcodegen))

$(eval $(call gb_ExternalProject_register_targets,qrcodegen,\
	build \
))

qrcodegen_CPPCLAGS=$(CPPFLAGS)

ifneq (,$(filter ANDROID DRAGONFLY FREEBSD iOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
qrcodegen_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

$(call gb_ExternalProject_get_state_target,qrcodegen,build):
	$(call gb_ExternalProject_run,build,\
		cd cpp && $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
