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

$(eval $(call gb_Executable_Executable,xlsxfuzzer))

$(eval $(call gb_Executable_use_api,xlsxfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,xlsxfuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,xlsxfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,xlsxfuzzer,\
    $(fuzzer_calc_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,xlsxfuzzer,\
    findsofficepath \
    ulingu \
    fuzzer_calc \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,xlsxfuzzer,\
	vcl/workben/xlsxfuzzer \
))

$(eval $(call gb_Executable_add_libs,xlsxfuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
