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

$(eval $(call gb_Executable_Executable,fods2xlsfuzzer))

$(eval $(call gb_Executable_use_api,fods2xlsfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,fods2xlsfuzzer,\
    $(fuzzer_externals) \
    epubgen \
    revenge \
))

$(eval $(call gb_Executable_set_include,fods2xlsfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,fods2xlsfuzzer,\
    $(fuzzer_calc_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,fods2xlsfuzzer,\
    $(fuzzer_statics) \
    fuzzer_calc \
))

$(eval $(call gb_Executable_add_exception_objects,fods2xlsfuzzer,\
    vcl/workben/fods2xlsfuzzer \
))

$(eval $(call gb_Executable_add_libs,fods2xlsfuzzer,\
    $(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
