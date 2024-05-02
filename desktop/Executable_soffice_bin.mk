# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,soffice_bin))

$(eval $(call gb_Executable_set_include,soffice_bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_add_defs,soffice_bin,\
    $(if $(DISABLE_DYNLOADING),$(if $(SYSTEM_LIBXML),,-DNOTEST_xmlCleanupParser)) \
))

$(eval $(call gb_Executable_use_libraries,soffice_bin,\
    sal \
    sofficeapp \
))

$(eval $(call gb_Executable_add_cobjects,soffice_bin,\
    desktop/source/app/main \
))

ifeq ($(OS),EMSCRIPTEN)
$(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktarget,soffice_bin)) : $(call gb_CustomTarget_get_workdir,static/emscripten_fs_image)/soffice.data.js.link

# don't sort; later can override previous settings!
$(eval $(call gb_Executable_add_prejs,soffice_bin,$(SRCDIR)/static/emscripten/environment.js))
$(eval $(call gb_Executable_add_prejs,soffice_bin,$(call gb_CustomTarget_get_workdir,static/emscripten_fs_image)/soffice.data.js.link))
ifeq ($(ENABLE_QT5),TRUE)
$(eval $(call gb_Executable_add_prejs,soffice_bin,$(SRCDIR)/static/emscripten/soffice_args.js))
endif
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_set_targettype_gui,soffice_bin,NO))

$(eval $(call gb_Executable_add_nativeres,soffice_bin,sofficebin/officeloader))

$(eval $(call gb_Executable_add_default_nativeres,soffice_bin,$(PRODUCTNAME)))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,soffice_bin,\
    /STACK:10000000 \
))

endif

endif

ifeq ($(OS),EMSCRIPTEN)
$(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktarget,soffice_bin)) : $(call gb_StaticLibrary_get_linktarget_target,unoembind)
$(call gb_LinkTarget_get_headers_target,$(call gb_Executable_get_linktarget,soffice_bin)) : $(call gb_StaticLibrary_get_headers_target,unoembind)
$(call gb_LinkTarget__static_lib_dummy_depend,unoembind)

$(eval $(call gb_Executable_add_ldflags,soffice_bin,\
	-s EXPORTED_FUNCTIONS=["_main"$(COMMA)"_libreofficekit_hook"$(COMMA)"_libreofficekit_hook_2"$(COMMA)"_lok_preinit"$(COMMA)"_lok_preinit_2"$(COMMA)"_malloc"$(COMMA)"_free"] -Wl$(COMMA)--whole-archive $(call gb_StaticLibrary_get_target,unoembind) -Wl$(COMMA)--no-whole-archive \
))
ifeq ($(ENABLE_QT6),TRUE)
$(eval $(call gb_Executable_add_ldflags,soffice_bin, \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=soffice_entry \
))
endif

ifneq ($(ENABLE_DBGUTIL),)

$(call gb_Executable_get_linktarget_target,soffice_bin): \
    $(call gb_CustomTarget_get_workdir,static/unoembind)/bindings_uno.js \
    $(SRCDIR)/unotest/source/embindtest/embindtest.js

$(eval $(call gb_Executable_add_ldflags,soffice_bin, \
    --post-js $(call gb_CustomTarget_get_workdir,static/unoembind)/bindings_uno.js \
    --post-js $(SRCDIR)/unotest/source/embindtest/embindtest.js \
))

endif

endif

# vim: set ts=4 sw=4 et:
