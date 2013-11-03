# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,languagetool))

$(eval $(call gb_ExternalProject_register_targets,languagetool,\
	build \
))

$(eval $(call gb_ExternalProject_use_jars,languagetool,\
	juh \
	jurt \
	ridl \
	unoil \
))

$(call gb_ExternalProject_get_state_target,languagetool,build) :
	cd "$(call gb_UnpackedTarball_get_dir,languagetool)" && \
	$(ICECREAM_RUN) "$(ANT)" \
		-q \
		-f build.xml \
		-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" \
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
			-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		$(if $(debug),-Dbuild.debug="on") \
		-Dext.ooo.juh.lib="$(call gb_Jar_get_target,juh)" \
		-Dext.ooo.jurt.lib="$(call gb_Jar_get_target,jurt)" \
		-Dext.ooo.ridl.lib="$(call gb_Jar_get_target,ridl)" \
		-Dext.ooo.unoil.lib="$(call gb_Jar_get_target,unoil)" \
		dist && \
	touch $@

# vim: set noet sw=4 ts=4:
