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

$(eval $(call gb_Executable_Executable,wksfuzzer))

$(eval $(call gb_Executable_use_api,wksfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,wksfuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,wksfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,wksfuzzer,\
    $(fuzzer_calc_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,wksfuzzer,\
    findsofficepath \
    ulingu \
    fuzzer_calc \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,wksfuzzer,\
	vcl/workben/wksfuzzer \
))

$(eval $(call gb_Executable_add_libs,wksfuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
