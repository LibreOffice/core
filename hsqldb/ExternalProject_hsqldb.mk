# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,hsqldb))

$(eval $(call gb_ExternalProject_use_unpacked,hsqldb,hsqldb))

$(eval $(call gb_ExternalProject_register_targets,hsqldb,\
	build \
))

$(call gb_ExternalProject_get_state_target,hsqldb,build) :
	cd $(EXTERNAL_WORKDIR) && \
	"$(ANT)" \
		-q \
		-f build/build.xml \
		-Dbuild.label="build-libreoffice" \
		$(if $(filter yes,$(JAVACISGCJ))\
			,-Dbuild.compiler=gcj \
			,-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
				-Dant.build.javac.target=$(JAVA_TARGET_VER) \
		) \
		$(if $(debug),-Dbuild.debug="on") \
		jar && \
	touch $@

# vim: set noet sw=4 ts=4:
