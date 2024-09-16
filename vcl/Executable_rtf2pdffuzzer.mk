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

$(eval $(call gb_Executable_Executable,rtf2pdffuzzer))

$(eval $(call gb_Executable_use_api,rtf2pdffuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,rtf2pdffuzzer,\
    $(fuzzer_externals) \
    epubgen \
    revenge \
))

$(eval $(call gb_Executable_set_include,rtf2pdffuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,rtf2pdffuzzer,\
    $(fuzzer_writer_libraries) \
    $(fuzzer_core_libraries) \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,rtf2pdffuzzer,\
    $(fuzzer_statics) \
    fuzzer_writer \
))

$(eval $(call gb_Executable_add_exception_objects,rtf2pdffuzzer,\
    vcl/workben/rtf2pdffuzzer \
))

$(eval $(call gb_Executable_add_libs,rtf2pdffuzzer,\
    $(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
