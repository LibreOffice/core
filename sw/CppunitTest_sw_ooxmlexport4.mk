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

$(eval $(call sw_ooxmlexport_test,4))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_ooxmlexport4,\
    officecfg/registry \
))

ifeq ($(OS),WNT)
# Initializing DocumentSignatureManager will require gpgme-w32spawn.exe in workdir/LinkTarget/Executable
$(eval $(call gb_CppunitTest_use_packages,sw_ooxmlexport4,\
    $(call gb_Helper_optional,GPGMEPP,gpgmepp)\
))
endif

# vim: set noet sw=4 ts=4:
