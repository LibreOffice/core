# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,updatehelper))

$(eval $(call gb_StaticLibrary_use_unpacked,updatehelper,onlineupdate))

$(eval $(call gb_StaticLibrary_set_include,updatehelper,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/inc/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/service \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/common \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/mfbt/double-conversion \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/mozapps/update/common \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/xpcom/base \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,updatehelper,\
	-DMOZ_MAINTENANCE_SERVICE \
	-DNSS3 \
	-DMOZ_VERIFY_MAR_SIGNATURE \
	-DXP_$(if $(filter WNT,$(OS)),WIN,UNIX) \
	$(if $(filter WNT,$(OS)),-U_WIN32_WINNT) \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,updatehelper,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,updatehelper,\
	UnpackedTarball/onlineupdate/mozglue/misc/Printf \
	UnpackedTarball/onlineupdate/onlineupdate/source/update/common/readstrings \
	UnpackedTarball/onlineupdate/toolkit/mozapps/update/common/updatecommon \
))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_generated_exception_objects,updatehelper,\
	UnpackedTarball/onlineupdate/onlineupdate/source/update/common/pathhash \
	UnpackedTarball/onlineupdate/onlineupdate/source/update/common/uachelper \
	UnpackedTarball/onlineupdate/onlineupdate/source/update/common/updatehelper \
	UnpackedTarball/onlineupdate/onlineupdate/source/service/certificatecheck \
	UnpackedTarball/onlineupdate/onlineupdate/source/service/registrycertificates \
	UnpackedTarball/onlineupdate/toolkit/mozapps/update/common/updateutils_win \
))
endif

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,updatehelper,cc))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,updatehelper, \
    UnpackedTarball/onlineupdate/mfbt/double-conversion/double-conversion/bignum \
    UnpackedTarball/onlineupdate/mfbt/double-conversion/double-conversion/bignum-dtoa \
    UnpackedTarball/onlineupdate/mfbt/double-conversion/double-conversion/cached-powers \
    UnpackedTarball/onlineupdate/mfbt/double-conversion/double-conversion/double-to-string \
    UnpackedTarball/onlineupdate/mfbt/double-conversion/double-conversion/fast-dtoa \
    UnpackedTarball/onlineupdate/mfbt/double-conversion/double-conversion/fixed-dtoa \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,updatehelper))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_cxxflags,updatehelper,-Zc:strictStrings-))
endif

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
