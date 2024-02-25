# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,gpgmepp))

$(eval $(call gb_Library_use_unpacked,gpgmepp,gpgmepp))

$(eval $(call gb_Library_use_externals,gpgmepp,\
	libgpg-error \
	libassuan \
))

$(eval $(call gb_LinkTarget_use_external_project,\
	$(call gb_Library_get_linktarget,gpgmepp),gpgmepp,full))

$(eval $(call gb_Library_set_warnings_disabled,gpgmepp))

$(eval $(call gb_Library_set_include,gpgmepp,\
    -I$(call gb_UnpackedTarball_get_dir,gpgmepp)/lang/cpp/src \
    -I$(call gb_UnpackedTarball_get_dir,gpgmepp)/lang/cpp/src/interfaces \
    -I$(call gb_UnpackedTarball_get_dir,gpgmepp) \
    -I$(call gb_UnpackedTarball_get_dir,gpgmepp)/src \
    -I$(call gb_UnpackedTarball_get_dir,gpgmepp)/conf \
    -I$(call gb_UnpackedTarball_get_dir,libgpg-error)/src \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_libs,gpgmepp,\
       ws2_32.lib shell32.lib \
       -LIBPATH:$(call gb_UnpackedTarball_get_dir,gpgmepp)/src/.libs libgpgme.lib \
))

$(eval $(call gb_Library_add_defs,gpgmepp,\
	-DHAVE_CONFIG_H \
	-DBUILDING_GPGMEPP \
	-DDLL_EXPORT \
	-DPIC \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,gpgmepp,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,gpgmepp,\
	UnpackedTarball/gpgmepp/lang/cpp/src/callbacks \
	UnpackedTarball/gpgmepp/lang/cpp/src/configuration \
	UnpackedTarball/gpgmepp/lang/cpp/src/context \
	UnpackedTarball/gpgmepp/lang/cpp/src/context_vanilla \
	UnpackedTarball/gpgmepp/lang/cpp/src/data \
	UnpackedTarball/gpgmepp/lang/cpp/src/decryptionresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/defaultassuantransaction \
	UnpackedTarball/gpgmepp/lang/cpp/src/editinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/encryptionresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/engineinfo \
	UnpackedTarball/gpgmepp/lang/cpp/src/eventloopinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/exception \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgaddexistingsubkeyeditinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgadduserideditinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgagentgetinfoassuantransaction \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpggencardkeyinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgrevokekeyeditinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgsetexpirytimeeditinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgsetownertrusteditinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/gpgsignkeyeditinteractor \
	UnpackedTarball/gpgmepp/lang/cpp/src/importresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/key \
	UnpackedTarball/gpgmepp/lang/cpp/src/keygenerationresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/keylistresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/scdgetinfoassuantransaction \
	UnpackedTarball/gpgmepp/lang/cpp/src/signingresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/statusconsumerassuantransaction \
	UnpackedTarball/gpgmepp/lang/cpp/src/swdbresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/tofuinfo \
	UnpackedTarball/gpgmepp/lang/cpp/src/trustitem \
	UnpackedTarball/gpgmepp/lang/cpp/src/util \
	UnpackedTarball/gpgmepp/lang/cpp/src/verificationresult \
	UnpackedTarball/gpgmepp/lang/cpp/src/vfsmountresult \
))

ifeq ($(COM),MSC)
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_Library_add_cxxflags,gpgmepp, \
    -Wno-c++11-narrowing \
))
endif
endif

# vim: set noet sw=4 ts=4:
