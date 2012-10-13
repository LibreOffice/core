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

$(eval $(call gb_ExternalProject_ExternalProject,apache_commons_lang))

$(eval $(call gb_ExternalProject_use_unpacked,apache_commons_lang,apache_commons_lang))

$(eval $(call gb_ExternalProject_register_targets,apache_commons_lang,\
	build \
))

$(call gb_ExternalProject_get_state_target,apache_commons_lang,build) :
	cd "$(call gb_UnpackedTarball_get_dir,apache_commons_lang)" && \
	ANT_OPTS="$$ANT_OPTS -Dfile.encoding=ISO-8859-1" \
	$(ICECREAM_RUN) "$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(RSCREVISION)" \
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
				-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		$(if $(debug),-Dbuild.debug="on") \
		jar && \
	touch $@

# vim: set noet sw=4 ts=4:
