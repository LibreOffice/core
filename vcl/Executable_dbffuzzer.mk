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

$(eval $(call gb_Executable_Executable,dbffuzzer))

$(eval $(call gb_Executable_use_api,dbffuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,dbffuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,dbffuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,dbffuzzer,\
    $(fuzzer_calc_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,dbffuzzer,\
    findsofficepath \
    ulingu \
    fuzzer_calc \
))

$(eval $(call gb_Executable_add_exception_objects,dbffuzzer,\
	vcl/workben/dbffuzzer \
))

$(eval $(call gb_Executable_add_libs,dbffuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
