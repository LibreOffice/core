# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,rust_uno/rustmaker))

$(call gb_CustomTarget_get_target,rust_uno/rustmaker): \
        $(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp

$(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp: \
        $(call gb_Executable_get_target,rustmaker) \
        $(call gb_Executable_get_runtime_dependencies,rustmaker) \
        $(call gb_UnoApi_get_target,offapi) \
        $(call gb_UnoApi_get_target,udkapi) \
        $(gb_CustomTarget_workdir)/rust_uno/rustmaker/.dir
	rm -fr $(SRCDIR)/rust_uno/src/generated
	mkdir $(SRCDIR)/rust_uno/src/generated
	rm -fr $(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp
	mkdir $(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp
	$(call gb_Helper_abbreviate_dirs, \
        $(call gb_Helper_execute,rustmaker $(if $(verbose),--verbose,) -Ocpp $(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp \
        $(if $(verbose),--verbose,) -Orust $(SRCDIR)/rust_uno/src/generated \
        $(call gb_UnoApi_get_target,offapi) $(call gb_UnoApi_get_target,udkapi)))
	touch $@

#TODO: Add to the general pattern rule from solenv/gbuild/CustomTarget.mk, instead of having to
# repeat its recipe here:
$(call gb_CustomTarget_get_clean_target,rust_uno/rustmaker):
	$(call gb_Output_announce,rust_uno/rustmaker,$(false),CUS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(gb_CustomTarget_workdir)/rust_uno/rustmaker && \
		rm -f $(call gb_CustomTarget_get_target,rust_uno/rustmaker))
	rm -fr $(SRCDIR)/rust_uno/src/generated

# vim: set noet sw=4 ts=4:
