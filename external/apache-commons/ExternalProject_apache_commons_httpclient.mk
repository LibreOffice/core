# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,apache_commons_httpclient))

$(eval $(call gb_ExternalProject_use_external,apache_commons_httpclient,commons-codec))
$(eval $(call gb_ExternalProject_use_external,apache_commons_httpclient,commons-logging))

$(eval $(call gb_ExternalProject_register_targets,apache_commons_httpclient,\
	build \
))

$(call gb_ExternalProject_get_state_target,apache_commons_httpclient,build) :
	$(call gb_ExternalProject_run,build,\
	ANT_OPTS="$$ANT_OPTS -Dfile.encoding=ISO-8859-1" \
	$(ICECREAM_RUN) "$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" \
		-Dcommons-logging.jar="$(call gb_UnpackedTarball_get_dir,apache_commons_logging)/target/commons-logging-1.1.3.jar" \
		-Dcommons-codec.jar="$(call gb_UnpackedTarball_get_dir,apache_commons_codec)/dist/commons-codec-1.6-SNAPSHOT.jar" \
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
				-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		$(if $(debug),-Dbuild.debug="on") \
		dist \
	)

# vim: set noet sw=4 ts=4:
