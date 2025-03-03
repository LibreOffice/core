# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,hsqldb))

$(eval $(call gb_ExternalProject_register_targets,hsqldb,\
	build \
))

# ANT_OPTS -Djava.security.manager=allow allows latest Apache Ant 1.10.12 to still run under
# Java 18, where java.lang.System.setSecurityManager (as called from
# org.apache.tools.ant.types.Permissions.setSecurityManager) would otherwise throw an
# UnsupportedOperationException (see <https://openjdk.java.net/jeps/411> "Deprecate the Security
# Manager for Removal"):
$(call gb_ExternalProject_get_state_target,hsqldb,build) :
	$(call gb_Trace_StartRange,hsqldb,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		unset MSYS_NO_PATHCONV && JAVA_HOME=$(JAVA_HOME_FOR_BUILD) \
		$(if $(JDK_SECURITYMANAGER_DISALLOWED_FOR_BUILD), \
		    ANT_OPTS="$$ANT_OPTS -Djava.security.manager=allow") \
		$(ICECREAM_RUN) "$(ANT)" \
			$(if $(verbose),-v,-q) \
			-f build/build.xml \
			-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" \
			-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
			-Dant.build.javac.target=$(JAVA_TARGET_VER) \
			$(if $(debug),-Dbuild.debug="on") \
			jar \
	)
	$(call gb_Trace_EndRange,hsqldb,EXTERNAL)

# vim: set noet sw=4 ts=4:
