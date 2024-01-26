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

ifeq ($(OS)-$(ENABLE_QT5),EMSCRIPTEN-TRUE)
$(eval $(call gb_Executable_add_prejs,soffice_bin,$(SRCDIR)/static/emscripten/soffice_args.js))
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
	-s EXPORTED_FUNCTIONS=["_main"$(COMMA)"_libreofficekit_hook"$(COMMA)"_libreofficekit_hook_2"$(COMMA)"_lok_preinit"$(COMMA)"_lok_preinit_2"] -Wl$(COMMA)--whole-archive $(call gb_StaticLibrary_get_target,unoembind) -Wl$(COMMA)--no-whole-archive \
))
ifeq ($(ENABLE_QT6),TRUE)
$(eval $(call gb_Executable_add_ldflags,soffice_bin, \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=soffice_entry \
))
endif

endif

ifeq ($(OS),MACOSX)
ifeq ($(CPUNAME),X86_64)

# tdf#152524 Add a __objc_fork_ok data section for Mac Intel executable
# This attempts to fix a particularly difficult to debug crash due to
# memory corruption when gpgme tries to fork and exec a gpg command.
# For more background, see the following bug comment:
#   https://bugs.documentfoundation.org/show_bug.cgi?id=152524#c39
# This data section is only added in Mac Intel builds as it is a
# controversial approach and I have not seen any reports of this bug
# on Mac Silicon yet. So limit the fix only to Mac Intel in the hope
# that this controversial approach will automatically phase itself
# out when Apple eventually stops releasing new versions of macOS
# for Mac Intel.
# Note: the Objective C runtime doesn't appear to check the value of
# this data section, but set it to 1 in case that changes in the future.
$(eval $(call gb_Executable_add_ldflags,soffice_bin,\
    -sectcreate __DATA __objc_fork_ok $(SRCDIR)/desktop/util/macos/__objc_fork_ok_data.txt \
))

endif
endif

# vim: set ts=4 sw=4 et:
