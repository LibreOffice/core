# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,rhino))

$(eval $(call gb_ExternalProject_register_targets,rhino,\
	build \
))

$(call gb_ExternalProject_get_state_target,rhino,build) :
	$(call gb_Trace_StartRange,rhino,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		mkdir $(gb_UnpackedTarball_workdir)/rhino/build \
		&& cd $(gb_UnpackedTarball_workdir)/rhino \
		&& $(call gb_JavaClassSet_JAVACCOMMAND,$(JAVA_TARGET_VER)) $(gb_JavaClassSet_JAVACDEBUG) \
			-d $(gb_UnpackedTarball_workdir)/rhino/build/content \
			@$(SRCDIR)/external/rhino/filelist.txt \
		&& mkdir $(gb_UnpackedTarball_workdir)/rhino/build/content/META-INF \
		&& cp $(gb_UnpackedTarball_workdir)/rhino/LICENSE.txt \
			$(gb_UnpackedTarball_workdir)/rhino/NOTICE-tools.txt \
			$(gb_UnpackedTarball_workdir)/rhino/NOTICE.txt \
			$(gb_UnpackedTarball_workdir)/rhino/build/content/META-INF/ \
		&& $(gb_Jar_JARCOMMAND) -cf $(gb_UnpackedTarball_workdir)/rhino/build/js.jar \
			-C $(gb_UnpackedTarball_workdir)/rhino/build/content . \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/commonjs/module/package.html \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/commonjs/module/provider/package.html \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/resources/Messages.properties \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/resources/Messages_en.properties \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/resources/Messages_fr.properties \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/tools/debugger/test.js \
			-C $(gb_UnpackedTarball_workdir)/rhino/src \
			org/mozilla/javascript/tools/resources/Messages.properties \
	)
	$(call gb_Trace_EndRange,rhino,EXTERNAL)

# vim: set noet sw=4 ts=4:
