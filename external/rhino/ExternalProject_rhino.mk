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
		mkdir $(call gb_UnpackedTarball_get_dir,rhino)/build \
		&& cd $(call gb_UnpackedTarball_get_dir,rhino) \
		&& $(call gb_JavaClassSet_JAVACCOMMAND,$(JAVA_TARGET_VER)) $(gb_JavaClassSet_JAVACDEBUG) \
			-d $(call gb_UnpackedTarball_get_dir,rhino)/build/content \
			@$(SRCDIR)/external/rhino/filelist.txt \
		&& mkdir $(call gb_UnpackedTarball_get_dir,rhino)/build/content/META-INF \
		&& cp $(call gb_UnpackedTarball_get_dir,rhino)/LICENSE.txt \
			$(call gb_UnpackedTarball_get_dir,rhino)/NOTICE-tools.txt \
			$(call gb_UnpackedTarball_get_dir,rhino)/NOTICE.txt \
			$(call gb_UnpackedTarball_get_dir,rhino)/build/content/META-INF/ \
		&& $(gb_Jar_JARCOMMAND) -cf $(call gb_UnpackedTarball_get_dir,rhino)/build/js.jar \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/build/content . \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/commonjs/module/package.html \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/commonjs/module/provider/package.html \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/resources/Messages.properties \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/resources/Messages_en.properties \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/resources/Messages_fr.properties \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/tools/debugger/test.js \
			-C $(call gb_UnpackedTarball_get_dir,rhino)/src \
			org/mozilla/javascript/tools/resources/Messages.properties \
	)
	$(call gb_Trace_EndRange,rhino,EXTERNAL)

# vim: set noet sw=4 ts=4:
