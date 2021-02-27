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

$(eval $(call gb_Executable_Executable,wmffuzzer))

$(eval $(call gb_Executable_use_api,wmffuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,wmffuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,wmffuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,wmffuzzer,\
    $(fuzzer_core_libraries) \
))

$(eval $(call gb_Executable_use_static_libraries,wmffuzzer,\
    findsofficepath \
    ulingu \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,wmffuzzer,\
	vcl/workben/wmffuzzer \
))

$(eval $(call gb_Executable_add_libs,wmffuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
