# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# for RSCREVISION
include $(SOLARENV)/inc/minor.mk
# for VERSION
include $(SRCDIR)/jfreereport/version.mk

$(eval $(call gb_ExternalProject_ExternalProject,jfreereport_libformula))

$(eval $(call gb_ExternalProject_use_unpacked,jfreereport_libformula,jfreereport_libformula))

$(eval $(call gb_ExternalProject_register_targets,jfreereport_libformula,\
	build \
))

$(call gb_ExternalProject_get_state_target,jfreereport_libformula,build) :
	cd "$(call gb_UnpackedTarball_get_dir,jfreereport_libformula)" && \
	"$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(RSCREVISION)" \
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
			-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		-Dantcontrib.available="true" \
		-Dbuild.id="10682" \
		-Dproject.revision="$(LIBFORMULA_VERSION)" \
		$(if $(debug),-Dbuild.debug="on") jar && \
	touch $@

# vim: set noet sw=4 ts=4:
