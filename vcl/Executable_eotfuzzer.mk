# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/vcl/commonfuzzer.mk

$(eval $(call gb_Executable_Executable,eotfuzzer))

$(eval $(call gb_Executable_use_api,eotfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,eotfuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,eotfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,eotfuzzer,\
    $(fuzzer_core_libraries) \
))

$(eval $(call gb_Executable_use_static_libraries,eotfuzzer,\
    $(fuzzer_statics) \
))

$(eval $(call gb_Executable_add_exception_objects,eotfuzzer,\
    vcl/workben/eotfuzzer \
))

$(eval $(call gb_Executable_add_libs,eotfuzzer,\
    $(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
