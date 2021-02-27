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

$(eval $(call gb_Executable_Executable,fodpfuzzer))

$(eval $(call gb_Executable_use_api,fodpfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,fodpfuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,fodpfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,fodpfuzzer,\
    $(fuzzer_draw_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,fodpfuzzer,\
    findsofficepath \
    ulingu \
    fuzzer_draw \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,fodpfuzzer,\
	vcl/workben/fodpfuzzer \
))

$(eval $(call gb_Executable_add_libs,fodpfuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
