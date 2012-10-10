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

$(eval $(call gb_ExternalProject_ExternalProject,jfreereport_liblayout))

$(eval $(call gb_ExternalProject_use_unpacked,jfreereport_liblayout,jfreereport_liblayout))

ifneq ($(SYSTEM_APACHE_COMMONS),YES)
$(eval $(call gb_ExternalProject_use_packages,jfreereport_liblayout,\
	apache_commons_logging_inc \
))
endif

$(eval $(call gb_ExternalProject_use_packages,jfreereport_liblayout,\
	jfreereport_sac \
	jfreereport_libbase \
	jfreereport_flute \
	jfreereport_libloader \
	jfreereport_libxml \
	jfreereport_libformula \
	jfreereport_libfonts \
	jfreereport_librepository \
	jfreereport_libserializer \
))

$(eval $(call gb_ExternalProject_register_targets,jfreereport_liblayout,\
	build \
))

$(call gb_ExternalProject_get_state_target,jfreereport_liblayout,build) :
	cd "$(call gb_UnpackedTarball_get_dir,jfreereport_liblayout)" && \
	$(ICECREAM_RUN) "$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(RSCREVISION)" \
		$(if $(filter YES,$(SYSTEM_APACHE_COMMONS))\
			,-Dcommons-logging.jar=$(COMMONS_LOGGING_JAR) \
		)\
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
			-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		-Dantcontrib.available="true" \
		-Dbuild.id="10682" \
		-Dlib=$(OUTDIR)/bin \
		$(if $(debug),-Dbuild.debug="on") jar && \
	touch $@

# vim: set noet sw=4 ts=4:
