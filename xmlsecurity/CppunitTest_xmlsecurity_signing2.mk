# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,xmlsecurity_signing2))

$(eval $(call gb_CppunitTest_add_exception_objects,xmlsecurity_signing2, \
	xmlsecurity/qa/unit/signing/signing2 \
))

$(eval $(call gb_CppunitTest_use_libraries,xmlsecurity_signing2, \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	sax \
	sfx \
	svx \
	subsequenttest \
	test \
	tl \
	unotest \
	utl \
	vcl \
	xmlsecurity \
	xsec_xmlsec \
))

$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_signing2,\
    boost_headers \
    libxml2 \
))

ifneq ($(OS),WNT)
ifneq (,$(ENABLE_NSS))
$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_signing2,\
    nssutil3 \
    nss3 \
))
endif
endif

$(eval $(call gb_CppunitTest_set_include,xmlsecurity_signing2,\
	-I$(SRCDIR)/xmlsecurity/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmlsecurity_signing2))

$(eval $(call gb_CppunitTest_use_ure,xmlsecurity_signing2))
$(eval $(call gb_CppunitTest_use_vcl,xmlsecurity_signing2))

$(eval $(call gb_CppunitTest_use_uiconfigs,xmlsecurity_signing2, \
    svt \
))

$(eval $(call gb_CppunitTest_use_rdb,xmlsecurity_signing2,services))

$(eval $(call gb_CppunitTest_use_configuration,xmlsecurity_signing2))

ifeq ($(ENABLE_POPPLER),TRUE)
$(eval $(call gb_CppunitTest_use_executable,xmlsecurity_signing2,xpdfimport))
endif

# various hacks to make unit test work on Linux more often
ifeq ($(OS),LINUX)
# reset the LD_LIBRARY_PATH for spawned GPG processes
$(call gb_CppunitTest_get_target,xmlsecurity_signing2): \
    EXTRA_ENV_VARS += \
        LIBO_LD_PATH=$$LD_LIBRARY_PATH
endif

$(eval $(call gb_CppunitTest_use_custom_headers,xmlsecurity_signing2,\
    officecfg/registry \
))

ifeq ($(OS),WNT)
# Initializing DocumentSignatureManager will require gpgme-w32spawn.exe in workdir/LinkTarget/Executable
$(eval $(call gb_CppunitTest_use_packages,xmlsecurity_signing2,\
    $(call gb_Helper_optional,GPGMEPP,gpgmepp)\
))
endif

$(eval $(call gb_CppunitTest_add_arguments,xmlsecurity_signing2, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
