# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,jfreereport))

ifneq ($(SOLAR_JAVA),)
ifneq ($(filter JFREEREPORT,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_targets,jfreereport,\
	UnpackedTarball_jfreereport_sac \
	ExternalProject_jfreereport_sac \
	Package_jfreereport_sac \
	UnpackedTarball_jfreereport_libbase \
	ExternalProject_jfreereport_libbase \
	Package_jfreereport_libbase \
	UnpackedTarball_jfreereport_flute \
	ExternalProject_jfreereport_flute \
	Package_jfreereport_flute \
	UnpackedTarball_jfreereport_libloader \
	ExternalProject_jfreereport_libloader \
	Package_jfreereport_libloader \
	UnpackedTarball_jfreereport_libxml \
	ExternalProject_jfreereport_libxml \
	Package_jfreereport_libxml \
	UnpackedTarball_jfreereport_libformula \
	ExternalProject_jfreereport_libformula \
	Package_jfreereport_libformula \
	UnpackedTarball_jfreereport_libfonts \
	ExternalProject_jfreereport_libfonts \
	Package_jfreereport_libfonts \
	ExternalProject_jfreereport_librepository \
	UnpackedTarball_jfreereport_librepository \
	Package_jfreereport_librepository \
	UnpackedTarball_jfreereport_libserializer \
	ExternalProject_jfreereport_libserializer \
	Package_jfreereport_libserializer \
	UnpackedTarball_jfreereport_liblayout \
	ExternalProject_jfreereport_liblayout \
	Package_jfreereport_liblayout \
	UnpackedTarball_jfreereport_flow_engine \
	ExternalProject_jfreereport_flow_engine \
	Package_jfreereport_flow_engine \
	Package_jfreereport_version \
))
endif
endif

# vim: set noet sw=4 ts=4:
