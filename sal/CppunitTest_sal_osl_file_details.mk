# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_osl_file_details))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_osl_file_details, \
    sal/qa/osl/file/forbidden \
))

$(eval $(call gb_CppunitTest_add_libs,sal_osl_file_details, \
    $(if $(filter LINUX,$(OS)), \
        -ldl \
        -lrt \
    ) \
    $(if $(filter SOLARIS,$(OS)), \
        -lnsl \
        -lsocket \
    ) \
    $(if $(filter HAIKU,$(OS)), \
        -lnetwork \
    ) \
    $(BACKTRACE_LIBS) \
))

$(eval $(call gb_CppunitTest_set_include,sal_osl_file_details, \
    $$(INCLUDE) \
    -I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_CppunitTest_use_externals,sal_osl_file_details, \
    dtoa \
    zlib \
))

$(eval $(call gb_CppunitTest_use_library_objects,sal_osl_file_details,sal))

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,sal_osl_file_details, \
    Carbon \
    CoreFoundation \
    Foundation \
    $(if $(ENABLE_MACOSX_SANDBOX),Security) \
))
endif

$(eval $(call gb_CppunitTest_use_system_win32_libs,sal_osl_file_details, \
    advapi32 \
    comdlg32 \
    dbghelp \
    mpr \
    ole32 \
    shell32 \
    user32 \
    userenv \
    wer \
    ws2_32 \
))

# vim: set noet sw=4 ts=4:
