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

$(eval $(call gb_Executable_Executable,pdf2fodgfuzzer))

$(eval $(call gb_Executable_use_api,pdf2fodgfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,pdf2fodgfuzzer,\
    $(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,pdf2fodgfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,pdf2fodgfuzzer,\
    $(fuzzer_draw_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,pdf2fodgfuzzer,\
    $(fuzzer_statics) \
    fuzzer_draw \
))

$(eval $(call gb_Executable_add_exception_objects,pdf2fodgfuzzer,\
    vcl/workben/pdf2fodgfuzzer \
))

$(eval $(call gb_Executable_add_libs,pdf2fodgfuzzer,\
    $(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
