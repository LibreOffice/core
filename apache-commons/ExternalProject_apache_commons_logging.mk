# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,apache_commons_logging))

$(eval $(call gb_ExternalProject_use_unpacked,apache_commons_logging,apache_commons_logging))

# TODO: this should go into RepositoryExternal.mk
ifneq ($(SYSTEM_TOMCAT),YES)
$(eval $(call gb_ExternalProject_use_packages,apache_commons_logging,\
	tomcat_inc \
))
endif

$(eval $(call gb_ExternalProject_register_targets,apache_commons_logging,\
	build \
))

$(call gb_ExternalProject_get_state_target,apache_commons_logging,build) :
	$(call gb_ExternalProject_run,build,\
	ANT_OPTS="$$ANT_OPTS -Dfile.encoding=ISO-8859-1" \
	$(ICECREAM_RUN) "$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" \
		$(if $(filter YES,$(SYSTEM_TOMCAT))\
			,-Dservletapi.jar=$(SERVLETAPI_JAR) \
			,-Dservletapi.jar=$(SOLARVER)/$(INPATH)/bin/servlet-api.jar \
		)\
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
				-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		$(if $(debug),-Dbuild.debug="on") \
		compile build-jar \
	)

# vim: set noet sw=4 ts=4:
