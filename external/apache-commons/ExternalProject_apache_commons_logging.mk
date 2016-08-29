# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,apache_commons_logging))

$(eval $(call gb_ExternalProject_register_targets,apache_commons_logging,\
	build \
))

$(call gb_ExternalProject_get_state_target,apache_commons_logging,build) :
	$(call gb_ExternalProject_run,build,\
	ANT_OPTS="$$ANT_OPTS -Dfile.encoding=ISO-8859-1" \
	$(ICECREAM_RUN) "$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO)" \
		-Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER) \
		$(if $(filter yes,$(JAVACISGCJ)),\
			-Dbuild.compiler=gcj \
		) \
		$(if $(debug),-Dcompile.debug="true",-Dcompile.debug="false") \
		compile build-jar \
	)

# vim: set noet sw=4 ts=4:
