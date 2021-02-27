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

$(eval $(call gb_Executable_Executable,rtffuzzer))

$(eval $(call gb_Executable_use_api,rtffuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,rtffuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,rtffuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,rtffuzzer,\
    $(fuzzer_writer_libraries) \
    $(fuzzer_core_libraries) \
))

$(eval $(call gb_Executable_use_static_libraries,rtffuzzer,\
    findsofficepath \
    ulingu \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,rtffuzzer,\
	vcl/workben/rtffuzzer \
))

$(eval $(call gb_Executable_add_libs,rtffuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
