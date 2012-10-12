# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,expat))

$(eval $(call gb_ExternalProject_use_unpacked,expat,expat))

ifneq ($(OS),WNT)
$(eval $(call gb_ExternalProject_register_targets,expat,\
	configure \
))
ifeq ($(BUILD_X64),TRUE)
$(eval $(call gb_ExternalProject_register_targets,expat,\
	duplicate_x64_files \
))
endif
endif

$(eval $(call gb_ExternalProject_register_targets,expat,\
	duplicate_unicode_files \
))

$(call gb_ExternalProject_get_state_target,expat,configure) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	&& touch $@

#
# ugly hack number one: we need to compile it twice: 
# with -DXML_UNICODE and without.
# We are copying the source to produce two different objects.
$(call gb_ExternalProject_get_state_target,expat,duplicate_unicode_files) :
	cd $(EXTERNAL_WORKDIR)/lib \
	&& $(GNUCOPY) xmlparse.c unicode_xmlparse.c \
	&& touch $@

#
# ugly hack number two on windows x64 platform: we need to link it twice: 
# with $(LINK_X64_BINARY) and with $(gb_LINK).
# We are copying the source to produce two different objects and libraries.
$(call gb_ExternalProject_get_state_target,expat,duplicate_x64_files) :
	cd $(EXTERNAL_WORKDIR)/lib \
	&& $(GNUCOPY) xmlparse.c xmlparse_x64.c \
	&& $(GNUCOPY) xmltok.c xmltok_x64.c \
	&& $(GNUCOPY) xmlrole.c xmlrole_x64.c \
	&& touch $@


# vim: set noet sw=4 ts=4:
