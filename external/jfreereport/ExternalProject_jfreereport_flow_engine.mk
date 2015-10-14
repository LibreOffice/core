# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,jfreereport_flow_engine))

$(eval $(call gb_ExternalProject_use_external_projects,jfreereport_flow_engine,\
	jfreereport_liblayout \
))

$(eval $(call gb_ExternalProject_register_targets,jfreereport_flow_engine,\
	build \
))

$(call gb_ExternalProject_get_state_target,jfreereport_flow_engine,build) :
	$(call gb_ExternalProject_run,build,\
		$(ICECREAM_RUN) "$(ANT)" \
			-q \
			-f build.xml \
			-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO)" \
			$(if $(SYSTEM_APACHE_COMMONS),\
				-Dcommons-logging.jar=$(COMMONS_LOGGING_JAR), \
				-Dcommons-logging.jar="$(call gb_UnpackedTarball_get_dir,apache_commons_logging)/target/commons-logging-$(COMMONS_LOGGING_VERSION).jar") \
			-Dlibbase.jar=$(call gb_UnpackedTarball_get_dir,jfreereport_libbase)/dist/libbase-$(LIBBASE_VERSION).jar \
			-Dlibformula.jar=$(call gb_UnpackedTarball_get_dir,jfreereport_libformula)/dist/libformula-$(LIBFORMULA_VERSION).jar \
			-Dliblayout.jar=$(call gb_UnpackedTarball_get_dir,jfreereport_liblayout)/build/lib/liblayout.jar \
			-Dlibloader.jar=$(call gb_UnpackedTarball_get_dir,jfreereport_libloader)/dist/libloader-$(LIBLOADER_VERSION).jar \
			-Dlibserializer.jar=$(call gb_UnpackedTarball_get_dir,jfreereport_libserializer)/dist/libserializer-$(LIBBASE_VERSION).jar \
			-Dlibxml.jar=$(call gb_UnpackedTarball_get_dir,jfreereport_libxml)/dist/libxml-$(LIBXML_VERSION).jar \
			-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
			-Dant.build.javac.target=$(JAVA_TARGET_VER) \
			$(if $(debug),-Dbuild.debug="on") jar \
	)

# vim: set noet sw=4 ts=4:
